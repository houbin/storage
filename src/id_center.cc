#include "id_center.h"
#include "../include/errcode.h"
#include "config_opts.h"
#include "storage.h"

namespace storage
{

IdCenter::IdCenter(Logger *logger)
: logger_(logger), mutex_("IdCenter::lock"), next_id_(0)
{

}

int32_t IdCenter::ApplyForId(string key_info, int flags, int32_t *id)
{
    Log(logger_, "apply for id, stream info is %s, flags is %d", key_info.c_str(), flags);
    assert(flags == 0 || flags == 1);

    Mutex::Locker lock(mutex_);

    if (flags == 1)
    {
        /* write legal judgement */
        map<string, int32_t>::iterator iter = write_key_info_.find(key_info);
        if (iter != write_key_info_.end())
        {
            Log(logger_, "write operation, key info exist, key info is %s", key_info.c_str());
            return -ERR_DOUBLE_WRITE;
        }

        uint32_t write_counts = write_key_info_.size();
        if (write_counts >= MAX_WRITE_STREAM_COUNTS)
        {
            return -ERR_REACH_WRITE_THREHOLD;
        }
    }
    else
    {
        /* read legal judgement */
        uint32_t read_counts = read_id_map_.size();
        if (read_counts >= MAX_READ_STREAM_COUNTS)
        {
            return -ERR_REACH_READ_THREHOLD;
        }

        StoreClient *store_client = NULL;
        int32_t ret = 0;
        ret = store_client_center->FindStoreClient(key_info, &store_client);
        if (ret != 0)
        {
            return -ERR_STREAM_NOT_WRITE;
        }
    }

    /* get id */
    while(true)
    {
        map<int32_t, string>::iterator iter = id_map_.find(next_id_);
        *id = next_id_;
        next_id_ = (next_id_ + 1) % MAX_STREAM_COUNTS;
        if (iter == id_map_.end())
        {
            break;
        }
    }

    id_map_.insert(make_pair(*id, key_info));

    if (flags == 1)
    {
        write_key_info_.insert(make_pair(key_info, *id));
    }
    else
    {
        read_id_map_.insert(make_pair(*id, key_info));
    }

    return 0;
}

int32_t IdCenter::ReleaseId(int32_t id)
{
    LOG_INFO(logger_, "release id, id %d", id);

    Mutex::Locker lock(mutex_);

    map<int32_t, string>::iterator iter = id_map_.find(id);
    if (iter != id_map_.end())
    {
        Log(logger_, "find id %d, and its stream info is %s. Delete it", id, iter->second.c_str());

        map<string, int32_t>::iterator key_info_iter = write_key_info_.find(iter->second);
        if (key_info_iter != write_key_info_.end())
        {
            write_key_info_.erase(key_info_iter);
        }

        map<int32_t, string>::iterator read_iter = read_id_map_.find(id);
        if (read_iter != read_id_map_.end())
        {
            read_id_map_.erase(read_iter);
        }
        
        id_map_.erase(iter);
    }
    else
    {
        Log(logger_, "not find id %d", id);
    }

    return 0;
}

int32_t IdCenter::GetStreamInfoFromId(int32_t id, string &key_info)
{
    Log(logger_, "get stream info from id %d", id);

    Mutex::Locker lock(mutex_);

    map<int32_t, string>::iterator iter = id_map_.find(id);
    if (iter != id_map_.end())
    {
        Log(logger_, "find id %d, key info is %s", id, iter->second.c_str());
        key_info = iter->second;
    }
    else
    {
        Log(logger_, "not find id %d", id);
        return -ERR_ITEM_NOT_FOUND;
    }

    return 0;
}

int32_t IdCenter::GetFlag(int32_t id, int &flag)
{
    Log(logger_, "get flag, id is %d", id);

    Mutex::Locker lock(mutex_);
    map<int32_t, string>::iterator iter = id_map_.find(id);
    if (iter == id_map_.end())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    map<int32_t, string>::iterator read_iter = read_id_map_.find(id);
    if (read_iter != read_id_map_.end())
    {
        flag = 0;
    }
    else
    {
        flag = 1;
    }

    return 0;
}

void IdCenter::Shutdown()
{
    return;
}

}

