#include "free_file_table.h"

namespace storage
{

FreeFileTable::FreeFileTable(Logger *logger, StreamTransferClientManager *transfer_client_manager)
: logger_(logger), mutex_(FreeFileTable::Lock), transfer_client_manager_(transfer_client_manager), stop_(false)
{

}

int32_t FreeFileTable::Get(RecordFile **record_file)
{
    assert(record_file != NULL);

    Log(logger_, "get record file");

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        return -1;
    }

    *record_file = free_file_queue_.front();
    free_file_queue_.pop_front();

    return 0;
}

#define FREE_FILE_RECYCLE_SECONDS (24*3600)
int32_t FreeFileTable::Recycle()
{
    list<RecordFile*> free_file_list;
    
    transfer_client_manager_->RecycleRecordFiles(free_file_list, false);

    mutex_.Lock();
    while (!free_file_list.empty())
    {
        RecordFile *record_file = free_file_list.front();
        assert(record_file != NULL);
        free_file_list.pop_front();
        free_file_queue_.push_back(record_file);
    }
    mutex_.Unlock();

    transfer_client_manager_->timer.AddEventAfter(FREE_FILE_RECYCLE_SECONDS, new C_Recycle(this));

    return 0;
}

int32_t FreeFileTable::Shutdown()
{
    Log(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    stop_ = true;
    while(!free_file_queue_.empty())
    {
        RecordFile *record_file = free_file_queue_.front();
        free_file_queue_.pop_front();
        delete record_file;
    }

    return 0;
}

}
