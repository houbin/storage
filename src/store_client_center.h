#ifndef STORAGE_STORE_CLIENT_CENTER_H_
#define STORAGE_STORE_CLIENT_CENTER_H_

#include <vector>
#include <deque>
#include "../include/errcode.h"
#include "../include/storage_api.h"
#include "../util/utime.h"
#include "../util/logger.h"
#include "../util/thread.h"
#include "../util/rwlock.h"
#include "store_types.h"
#include "config_opts.h"
#include "id_center.h"
#include "free_file_table.h"
#include "record_file.h"

using namespace std;
using namespace util;

namespace storage
{

class StoreClient;
class RecordFileMap
{
private:
    Logger *logger_;

    StoreClient *store_client_;
    RWLock rwlock_;
    map<UTime, RecordFile*> record_file_map_;
    map<RecordFile*, map<UTime, RecordFile*>::iterator> file_search_map_;

    int32_t FindStampRecordFile(UTime &stamp, RecordFile **record_file);
    int32_t SelectFragInfoWithStartTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectFragInfoWithStartAndEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectFragInfoWithEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &end, deque<FRAGMENT_INFO_T> &select_frag_info);
    int32_t SelectAllFragInfo(deque<FRAGMENT_INFO_T> &all_frag_info, deque<FRAGMENT_INFO_T> &select_frag_info);

    void Dump();

public:
    RecordFileMap(Logger *logger, StoreClient *store_client);

    bool Empty();

    /* these functions just get or put record file */
    int32_t PutRecordFile(UTime &time, RecordFile *record_file);
    int32_t ListRecordFragments(UTime &time, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);
    int32_t GetLastRecordFile(RecordFile **record_file);

    /* these functions change states of record file */
    int32_t OpenWriteRecordFile(RecordFile **record_file);
    int32_t AllocWriteRecordFile(UTime &stamp, RecordFile **record_file);
    int32_t FinishWriteRecordFile(RecordFile *record_file);
    int32_t EraseRecordFile(RecordFile *record_file);
    int32_t SeekStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset);
    int32_t FinishReadRecordFile(RecordFile *record_file);

    void Shutdown();
};

class StoreClient;
class C_WriteIndexTick;

class RecordWriter : public Thread
{
private:
    Logger *logger_;
    RecordFileMap *file_map_;

    Mutex queue_mutex_;
    Cond queue_cond_;
    deque<WriteOp*> write_op_queue_;

    Mutex record_file_mutex_;
    RecordFile *record_file_;

    char *buffer_;
    uint32_t buffer_write_offset_;
    BufferTimes buffer_times_;

    FRAME_INFO_T *current_o_frame_;

    C_WriteIndexTick *write_index_event_;

    bool stop_;

public:
    RecordWriter(Logger *logger, RecordFileMap *file_map_);

    int32_t Enqueue(WriteOp *write_op);
    int32_t Dequeue(WriteOp **write_op);

    int32_t EncodeFrameHeader(char *buffer, FRAME_INFO_T *frame);
    int32_t EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame, char *temp_buffer);
    int32_t UpdateBufferTimes(uint32_t type, UTime &time);
    int32_t WriteBuffer(uint32_t write_length);
    int32_t BuildRecordFileIndex(RecordFile *record_file, char *record_file_info_buffer, uint32_t record_file_info_length,
                        char *record_frag_info_buffer, uint32_t record_frag_info_length, uint32_t *record_frag_info_number);

    int32_t WriteRecordFileIndex(int r);
    int32_t ResetWriteIndexEvent(uint32_t after_seconds);
    int32_t DoWriteIndexEvent(bool again);

    void Start();
    void *Entry();
    void Stop();

    void Shutdown();
};

class RecordReader
{
private:
    Logger *logger_;
    RecordFileMap *file_map_;
    RecordFile *record_file_;

    uint32_t read_offset_;
    uint32_t read_end_offset_;

    FRAME_INFO_T current_o_frame_;

public:
    RecordReader(Logger *logger, RecordFileMap *file_map);
    int32_t Seek(UTime &stamp);
    int32_t ReadFrame(FRAME_INFO_T *frame_info);
    int32_t Close();

    void Shutdown();
};

class StoreClient
{
private:
    Logger *logger_;

    string stream_info_;
    RecordFileMap record_file_map_;

    RecordWriter writer;

    Mutex reader_mutex_;
    map<int32_t, RecordReader*> record_readers_;

public:
    StoreClient(Logger *logger, string stream_info);

    bool IsRecordFileEmpty();
    string GetStreamInfo();

    int32_t OpenWrite(int32_t id);
    int32_t EnqueueFrame(FRAME_INFO_T *frame);
    int32_t CloseWrite(int32_t id);

    int32_t OpenRead(int32_t id);
    int32_t SeekRead(int32_t id, UTime &stamp);
    int32_t ReadFrame(int32_t id, FRAME_INFO_T *frame);
    int32_t CloseRead(int32_t id);

    int32_t GetFreeFile(RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t SeekReaderStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset);
    int32_t PutRecordFile(UTime &stamp, RecordFile *record_file);
    int32_t RecycleRecordFile(RecordFile *record_file);

    int32_t WriteRecordFileIndex(RecordFile *record_file, int r);

    int32_t ListRecordFragments(UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);

    void Shutdown();
};

class C_WriteIndexTick: public Context
{
    RecordWriter *writer_;
public:

    C_WriteIndexTick(RecordWriter *writer)
    : writer_(writer)
    {}

    void Finish(int r)
    {
        int32_t ret;
        ret = writer_->WriteRecordFileIndex(r);
        return;
    }
};

typedef struct recycle_item
{
    RecordFile *record_file;
    StoreClient *store_client;
}RecycleItem;

class C_Recycle;
class StoreClientCenter
{
private:
    Logger *logger_;

    /* used for add/delete/find client */
    RWLock rwlock_;
    vector<StoreClient*> clients_;
    map<string, StoreClient*> client_search_map_;
    
    Mutex recycle_mutex_;
    // use recycle seq of record file as key
    map<uint64_t, RecycleItem> recycle_queue_;

public:
    Mutex timer_lock;
    SafeTimer timer;

    StoreClientCenter(Logger *logger);

    int32_t Init();
    int32_t Open(int flags, int32_t id, string &stream_info);
    int32_t Close(int32_t id, int flag);
    int32_t AddStoreClient(string &stream_info, StoreClient **client);

    int32_t GetStoreClient(int32_t id, StoreClient **client);
    int32_t FindStoreClient(string stream_info, StoreClient **client);
    int32_t RemoveStoreClient(StoreClient *client);

    int32_t WriteFrame(int32_t id, FRAME_INFO_T *frame);
    int32_t SeekRead(int32_t id, UTime &stamp);
    int32_t ReadFrame(int32_t id, FRAME_INFO_T *frame);

    int32_t ListRecordFragments(int32_t id, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);

    int32_t AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file);
    int32_t Recycle();

    void Shutdown();

    int32_t DumpClientSearchMap();
};

class C_Recycle : public Context
{
public:
    StoreClientCenter *store_client_center_;

    C_Recycle(StoreClientCenter *store_client_center)
    : store_client_center_(store_client_center)
    {
    
    }

    void Finish(int r)
    {
        store_client_center_->Recycle();
        return;
    }
};

}

#endif

