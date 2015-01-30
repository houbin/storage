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
#include "config_opts.h"
#include "id_center.h"
#include "free_file_table.h"

using namespace std;
using namespace util;

namespace storage
{

typedef struct write_op_
{
    FRAME_INFO_T *frame_info;
}WriteOp;

typedef struct buffer_times
{
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
}BufferTimes;

class RecordFileMap
{
private:
    Logger *logger_;

    RWLock rwlock_;
    map<UTime, RecordFile*> record_file_map_;

    bool stop_;

public:
    RecordFileMap(Logger logger);
    int32_t GetRecordFile(UTime time, RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t PushBackRecordFile(UTime time, RecordFile *record_file);

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
        ret = client_->writer.WriteRecordFileIndex(record_file_, r);
        return;
    }
};

class RecordWriter : public Thread
{
private:
    Logger *logger;
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
    int32_t UpdateBufferTimes(uint32_t type, UTime time);
    int32_t WriteBuffer(RecordFile *record_file, uint32_t write_length);
    int32_t BuildRecordFileIndex(RecordFile *record_file, char *record_file_info_buffer, uint32_t record_file_info_length,
                        char *record_frag_info_buffer, uint32_t record_frag_info_length, uint32_t *record_frag_info_number);

    int32_t WriteRecordFileIndex(RecordFile *record_file, int r);
    int32_t ResetWriteIndexEvent(RecordFile *record_file, uint32_t after_seconds);
    int32_t CancelWriteIndexEvent();

    void Start();
    void Entry();
    void Stop();
};

class StoreClient
{
private:
    Logger *logger_;

    string stream_info_;
    RecordFileMap record_file_map_;

    RecordWriter writer;

public:
    StoreClient(Logger *logger, string stream_info);

    int32_t Open(int flags, uint32_t id);
    int32_t EnqueueFrame(FRAME_INFO_T *frame);

    int32_t GetFreeFile(UTime &time, RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);

    int32_t OpenWrite(uint32_t id);
    int32_t OpenRead(uint32_t id);

    int32_t CloseWrite(uint32_t id);
    int32_t CloseRead(uint32_t id);
};

typedef struct recycle_item
{
    RecordFile *record_file;
    StoreClient *store_client;
}RecycleItem;

class C_Recycle : public Context
{
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
}

class StoreClientCenter
{
private:
    Logger *logger_;

    /* only used for add or delete client */
    RWLock rwlock_;
    vector<StoreClient*> clients_; // 以操作id为下标，用于快速查找对应的client
    map<string, StoreClient*> client_search_map_;
    
    Mutex recycle_mutex_;
    deque<RecycleItem> recycle_queue_;

public:
    Mutex timer_lock;
    SafeTimer timer;

    StoreClientCenter(Logger *logger);

    int32_t OpenStoreClient(int flags, uint32_t id, string &stream_info);
    int32_t GetStoreClient(uint32_t id, StoreClient **client);
    int32_t FindStoreClient(string stream_info, StoreClient **client);
    int32_t CloseStoreClient(uint32_t id, int flag);

    int32_t WriteFrame(uint32_t id, FRAME_INFO_T *frame);

    int32_t AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file);
    int32_t StartRecycle();
    int32_t Recycle();
};

}

#endif

