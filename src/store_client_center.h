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
    bool stop_;

    map<UTime, RecordFile*> record_file_map_;

public:
    RecordFileMap(Logger logger)
    int32_t GetRecordFile(UTime time, RecordFile **record_file);
    int32_t GetLastRecordFile(RecordFile **record_file);
    int32_t PushBackRecordFile(UTime time, RecordFile *record_file);

    void Shutdown();
};

class StoreClient
{
private:
    Logger *logger_;

    string stream_info_;

    FreeFileTable *free_file_table_;
    RecordFileMap *record_file_map_;

    class Writer : public Thread
    {
    private:
        Logger *logger;
        StoreClient *store_client_;

        Mutex queue_mutex_;
        Cond queue_cond_;
        deque<WriteOp*> write_op_queue_;

        string buffer_;
        BufferTimes buffer_times_;

        FRAME_INFO_T *current_o_frame;

        bool stop_;

    public:
        Writer(Logger *logger, RWLock &rwlock);

        int32_t EncodeHeader(char *buffer, FRAME_INFO_T *frame);
        int32_t EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame);
        
        int32_t Enqueue(WriteOp *write_op);
        int32_t Dequeue(WriteOp **write_op);

        int32_t UpdateBufferTimes(uint32_t type, UTime time);
        int32_t WriteBuffer(uint32_t write_length, RecordFile *record_file);

        void Entry();

        void Stop();
    };

    uint32_t write_fd;
    Writer writer;

public:
    StoreClient(Logger *logger, string stream_info);

    int32_t Open(int flags, uint32_t id);
    int32_t EnqueueFrame(FRAME_INFO_T *frame);

    int32_t GetFreeFile(UTime &time, RecordFile **record_file);
};

class StoreClientCenter
{
private:
    Logger *logger_;

    /* only used for add or delete client */
    Mutex mutex_;

    IdCenter *id_center_;

    /* use id as index */
    vector<StoreClient*> clients_;

    map<string, StoreClient*> client_search_map_;

public:
    StoreClientCenter(Logger *logger);

    int32_t OpenStoreClient(uint32_t id);
    int32_t GetStoreClient(uint32_t id, StoreClient **client);
    int32_t EraseStoreClient(uint32_t id);

};

}

#endif
