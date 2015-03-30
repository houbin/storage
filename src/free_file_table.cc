#include <stdlib.h>
#include "free_file_table.h"
#include "config_opts.h"
#include "storage.h"
#include "index_file.h"

namespace storage
{

FreeFileTable::FreeFileTable(Logger *logger)
: logger_(logger), mutex_("FreeFileTable::Lock")
{

}

uint32_t FreeFileTable::CountRecordFiles()
{
    uint32_t sum = 0;

    map<string, DiskInfo*>::iterator iter = disk_free_file_info_.begin();
    for (; iter != disk_free_file_info_.end(); iter++)
    {
        DiskInfo *disk_info = iter->second;
        
        sum += disk_info->free_file_queue.size();
    }

    return sum;
}

int32_t FreeFileTable::TryRecycle()
{
    uint32_t sum_files = 0;
    uint32_t write_stream_counts = 0;

    sum_files = CountRecordFiles();
    write_stream_counts = stream_to_disk_map_.size();

    if (sum_files <= write_stream_counts * 2)
    {
        store_client_center->StartRecycle();
        return 0;
    }

    return 0;
}

int32_t FreeFileTable::Put(RecordFile *record_file)
{
    assert(record_file != NULL);
    LOG_DEBUG(logger_, "put free record file %srecord_%05d", record_file->base_name_.c_str(), record_file->number_);

    int32_t ret;
    DiskInfo *disk_info = NULL;

    Mutex::Locker lock(mutex_);
    string disk_base_name(record_file->base_name_);
    map<string, DiskInfo*>::iterator iter = disk_free_file_info_.find(disk_base_name);
    if (iter == disk_free_file_info_.end())
    {
        disk_info = new DiskInfo;
        assert(disk_info != NULL);
        disk_free_file_info_.insert(make_pair(disk_base_name, disk_info));
        LOG_DEBUG(logger_, "not found disk info");
    }
    else
    {
        disk_info = iter->second;
        LOG_DEBUG(logger_, "found disk info");
    }

    assert(disk_info != NULL);

    IndexFile *index_file = NULL;
    ret = index_file_manager->Find(record_file->base_name_, &index_file);
    assert(ret == 0);
    assert(index_file != NULL);

    struct RecordFileInfo record_file_info = {0};
    memset(&record_file_info, 0, sizeof(RecordFileInfo));
    int32_t write_offset = record_file->number_ * sizeof(struct RecordFileInfo);
    index_file->Write(write_offset, (char *)&record_file_info, sizeof(struct RecordFileInfo));
    record_file->Clear();

    disk_info->free_file_queue.push_back(record_file);
    cond_.Signal();

    return 0;
}

int32_t FreeFileTable::Get(string stream_info, RecordFile **record_file)
{
    assert(record_file != NULL);
    LOG_DEBUG(logger_, "get free record file, stream info [%s]", stream_info.c_str());

    int32_t ret;

    Mutex::Locker lock(mutex_);
    map<string, string>::iterator stream_iter = stream_to_disk_map_.find(stream_info);
    if (stream_iter != stream_to_disk_map_.end())
    {
        string disk_str(stream_iter->second);
        map<string, DiskInfo*>::iterator disk_iter = disk_free_file_info_.find(disk_str);
        assert(disk_iter != disk_free_file_info_.end());

        DiskInfo *disk_info = disk_iter->second;
        assert(disk_info != NULL);
        if (!disk_info->free_file_queue.empty())
        {
            *record_file = disk_info->free_file_queue.front();
            disk_info->free_file_queue.pop_front();
            LOG_DEBUG(logger_, "get free record file %srecord_%05d", (*record_file)->base_name_.c_str(), (*record_file)->number_);

            goto end;
        }
        else
        {
            // stream migrated to other disk
            disk_info->writing_streams.erase(stream_info);
        }
    }

    ret = GetNewDiskFreeFile(stream_info, record_file);
    assert(ret == 0);

end:
    TryRecycle();
    return 0;
}

int32_t FreeFileTable::GetNewDiskFreeFile(string stream_info, RecordFile **record_file)
{
    assert(record_file != NULL);
    LOG_DEBUG(logger_, "get new disk free file, stream info %s", stream_info.c_str());

    DiskInfo *valid_disk_info = NULL;
    string disk_str;
    while (true)
    {
        map<string, DiskInfo*>::iterator iter = disk_free_file_info_.begin();
        for (; iter != disk_free_file_info_.end(); iter++)
        {
            DiskInfo *disk_info = iter->second;
            assert(disk_info != NULL);

            uint32_t stream_counts = disk_info->writing_streams.size();
            if (stream_counts >= kMaxStreamsPerDisk)
            {
                continue;
            }

            if (disk_info->free_file_queue.empty())
            {
                continue;
            }

            disk_str = iter->first;
            valid_disk_info = disk_info;
            break;
        }

        if (valid_disk_info == NULL)
        {
            LOG_DEBUG(logger_, "no useful disk, start recycle");
            mutex_.Unlock();
            store_client_center->StartRecycle();
            mutex_.Lock();
            cond_.Wait(mutex_);
            continue;
        }
        else
        {
            LOG_DEBUG(logger_, "find useful disk, disk_str %s", disk_str.c_str());
            break;
        }
    }

    // get record file and update disk info
    *record_file = valid_disk_info->free_file_queue.front();
    valid_disk_info->free_file_queue.pop_front();
    valid_disk_info->writing_streams.insert(stream_info);

    map<string, string>::iterator iter = stream_to_disk_map_.find(stream_info);
    if (iter != stream_to_disk_map_.end())
    {
        iter->second = disk_str;
    }
    else
    {
        stream_to_disk_map_.insert(make_pair(stream_info, disk_str));
    }

    LOG_DEBUG(logger_, "get new disk record file %srecord_%05d", (*record_file)->base_name_.c_str(), (*record_file)->number_);
    return 0;
}

int32_t FreeFileTable::Close(string stream_info)
{
    LOG_DEBUG(logger_, "close stream %s", stream_info.c_str());

    string disk_base_name;

    Mutex::Locker lock(mutex_);
    map<string, string>::iterator iter = stream_to_disk_map_.find(stream_info);
    if (iter != stream_to_disk_map_.end())
    {
        disk_base_name = iter->second;
        stream_to_disk_map_.erase(iter);

        map<string, DiskInfo*>::iterator iter = disk_free_file_info_.find(disk_base_name);
        if (iter != disk_free_file_info_.end())
        {
            DiskInfo *disk_info = iter->second;
            disk_info->writing_streams.erase(stream_info);
        }
    }

    return 0;
}

int32_t FreeFileTable::Shutdown()
{
    LOG_DEBUG(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    while(!disk_free_file_info_.empty())
    {
        map<string, DiskInfo*>::iterator iter = disk_free_file_info_.begin();

        DiskInfo *disk_info = iter->second;
        while(!disk_info->free_file_queue.empty())
        {
            RecordFile *record_file = disk_info->free_file_queue.front();
            disk_info->free_file_queue.pop_front();
            if (record_file != NULL)
            {
                delete record_file;
                record_file = NULL;
            }
        }
        
        disk_free_file_info_.erase(iter);
        delete disk_info;
        disk_info = NULL;
    }
    
    return 0;
}

}

