#ifndef STORAGE_FREE_FILE_TABLE_H_
#define STORAGE_FREE_FILE_TABLE_H_

#include <deque>
#include "../util/cond.h"
#include "../util/mutex.h"
#include "../util/logger.h"
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

public:
    FreeFileTable(Logger *logger, StreamTransferClientManager *transfer_client_manager);

    int32_t Put(RecordFile *record_file);
    int32_t Get(RecrodFile **record_file);
};

}

#endif
