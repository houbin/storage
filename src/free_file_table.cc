#include "free_file_table.h"

namespace storage
{

FreeFileTable::FreeFileTable(Logger *logger)
: logger_(logger), mutex_("FreeFileTable::Lock"), stop_(false)
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

int32_t FreeFileTable::Put(RecordFile *record_file)
{
    assert(record_file != NULL);
    Log(logger_, "put record file %p", record_file);

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        return -1;
    }

    free_file_queue_.push_back(record_file);

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
