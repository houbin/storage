#ifndef STORAGE_FREE_FILE_TABLE_H_
#define STORAGE_FREE_FILE_TABLE_H_

#include <deque>
#include "../util/cond.h"
#include "../util/mutex.h"
#include "../util/logger.h"
#inlcude "../util/context.h"
#include "../util/timer.h"
#include "record_file.h"
#include "stream_transfer_client_manager.h"

using namespace std;
using namespace util;

namespace storage
{

class FreeFileTable
{
private:
    Logger *logger_;
    Mutex *mutex_;
    Cond *cond_;
    deque<RecordFile*> free_file_queue_;
    bool stop_;

    /* use for recycling record files */
    StreamTransferClientManager *transfer_client_manager_;

    /* 定时器处理函数，用于定时回收record file */
    class C_Recycle : public Context
    {
    private:
        FreeFileTable *free_file_table_;
    public:
        C_Recycle(FreeFileTable *free_file_table) : free_file_table_(free_file_table) {}

        void Finish(int r)
        {
            free_file_table_->Recycle();
            return;
        }
    };

public:
    FreeFileTable(Logger *logger, StreamTransferClientManager *transfer_client_manager);

    int32_t Put(RecordFile *record_file);
    int32_t Get(RecrodFile **record_file);

    int32_t Recycle();
    int32_t Shutdown();
};

}

#endif
