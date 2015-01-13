#include "free_file_table.h"

namespace storage
{

FreeFileTable::FreeFileTable(Logger *logger, StreamTransferClientManager *transfer_client_manager)
: logger_(logger), mutex_(FreeFileTable::Lock), transfer_client_manager_(transfer_client_manager), stop_(false)
{

}

int32_t FreeFileTable::Put(RecordFile *record_file)
{
    assert(record_file != NULL);

    Log(logger_, "put record file, ptr is %p", record_file);

    Mutex::Locker lock(mutex_);

    if(stop_)
    {
        delete record_file;
        return 0;
    }

    free_file_queue_.push_back(record_file);
    cond.Signal();

    return 0;
}

int32_t FreeFileTable::Get(RecordFile **record_file)
{
    assert(record_file != NULL);

    Log(logger_, "get record file");

    Mutex::Locker lock(mutex_);
    while(free_file_queue_.empty() && !stop_)
    {
        cond.Wait(mutex_);
    }

    if (stop_)
    {
        return -1;
    }

    *record_file = free_file_queue_.front();
    free_file_queue_.pop_front();

    return 0;
}

}
