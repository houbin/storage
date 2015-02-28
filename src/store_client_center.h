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

class RecordFileMap
{
private:
    Logger *logger_;

    RWLock rwlock_;
    map<UTime, RecordFile*> record_file_map_;
    map<RecordFile*, map<UTime, RecordFile*>::iterator> file_search_map_;

    bool stop_;

public:
    RecordFileMap(Logger *logger);
    bool IsEmpty();

    int32_t GetRecordFile(UTime &time, RecordFile **record_file);
    int32_t GetNextRecordFile(RecordFile *current_record_file, RecordFile **next_record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t PutRecordFile(UTime &time, RecordFile *record_file);
    int32_t EraseRecordFile(RecordFile *record_file);

    void Shutdown();
};

class StoreClient;
class C_WriteIndexTick;

class RecordWriter : public Thread
{
private:
    Logger *logger_;
    StoreClient *store_client_;

    Mutex queue_mutex_;
    Cond queue_cond_;
    deque<WriteOp*> write_op_queue_;

    string buffer_;
    BufferTimes buffer_times_;

    FRAME_INFO_T *current_o_frame_;

    C_WriteIndexTick *write_index_event_;

    Mutex last_record_file_mutex_;

    bool stop_;

public:
    RecordWriter(Logger *logger, StoreClient *client);

    int32_t Enqueue(WriteOp *write_op);
    int32_t Dequeue(WriteOp **write_op);

    int32_t EncodeHeader(char *buffer, FRAME_INFO_T *frame);
    int32_t EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame);
    int32_t UpdateBufferTimes(uint32_t type, UTime &time);
    int32_t WriteBuffer(RecordFile *record_file, uint32_t write_length);
    int32_t BuildRecordFileIndex(RecordFile *record_file, char *record_file_info_buffer, uint32_t record_file_info_length,
                        char *record_frag_info_buffer, uint32_t record_frag_info_length, uint32_t *record_frag_info_number);

    int32_t WriteRecordFileIndex(RecordFile *record_file, int r);
    int32_t ResetWriteIndexEvent(RecordFile *record_file, uint32_t after_seconds);

    void Start();
    void *Entry();
    void Stop();

    void Shutdown();
};

class RecordReader
{
private:
    StoreClient *store_client_;
    RecordFile *record_file_;
    uint32_t read_offset_;
    FRAME_INFO_T current_o_frame_;

public:
    RecordReader(StoreClient *store_client);
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
    map<uint32_t, RecordReader*> record_readers_;

public:
    StoreClient(Logger *logger, string stream_info);

    bool IsRecordFileEmpty();
    string GetStreamInfo();

    int32_t OpenWrite(uint32_t id);
    int32_t EnqueueFrame(FRAME_INFO_T *frame);
    int32_t CloseWrite(uint32_t id);

    int32_t OpenRead(uint32_t id);
    int32_t SeekRead(uint32_t id, UTime &stamp);
    int32_t ReadFrame(uint32_t id, FRAME_INFO_T *frame);
    int32_t CloseRead(uint32_t id);

    int32_t GetFreeFile(UTime &time, RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t GetRecordFile(UTime &stamp, RecordFile **record_file);
    int32_t PutRecordFile(UTime &stamp, RecordFile *record_file);
    int32_t RecycleRecordFile(RecordFile *record_file);

    int32_t WriteRecordFileIndex(RecordFile *record_file, int r);


    void Shutdown();
};

class C_WriteIndexTick: public Context
{
    StoreClient *client_;
    RecordFile *record_file_;
public:

    C_WriteIndexTick(StoreClient *client, RecordFile *record_file)
    : client_(client), record_file_(record_file)
    {}

    void Finish(int r)
    {
        int32_t ret;
        ret = client_->WriteRecordFileIndex(record_file_, r);
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
    deque<RecycleItem> recycle_queue_;

public:
    Mutex timer_lock;
    SafeTimer timer;

    StoreClientCenter(Logger *logger);

    int32_t Open(int flags, uint32_t id, string &stream_info);
    int32_t Close(uint32_t id, int flag);
    int32_t AddStoreClient(string &stream_info, StoreClient **client);

    int32_t GetStoreClient(uint32_t id, StoreClient **client);
    int32_t FindStoreClient(string stream_info, StoreClient **client);
    int32_t RemoveStoreClient(StoreClient *client);

    int32_t WriteFrame(uint32_t id, FRAME_INFO_T *frame);
    int32_t SeekRead(uint32_t id, UTime &stamp);
    int32_t ReadFrame(uint32_t id, FRAME_INFO_T *frame);

    int32_t AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file);
    int32_t StartRecycle();
    int32_t Recycle();

    void Shutdown();
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

