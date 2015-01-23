#include "id_center.h"
#include "config_opts.h"

namespace storage
{

IdCenter::IdManager(Logger *logger)
: logger_(logger), mutex_("IdCenter::lock"), next_id_(0)
{

}

int32_t IdCenter::ApplyForId(string key_info, int flags, uint32_t *id)
{
    Log(logger_, "apply for id, stream info is %s, size is %d", stream_info, size);
    assert(flags == 0 || flags == 1);

    Mutex::Locker lock(mutex_);

    if (flags == 1)
    {
        map<string, uint32_t>::iterator iter = write_key_info_.find(key_info);
        if (iter != write_key_info_.end())
        {
            Log(logger_, "write operation, key info exist, key info is %s", key_info.c_str());
            return -ERR_DOUBLE_WRITE;
        }
    }

    map<uint32_t, string>::iterator iter = id_map_.find(next_id_);
    while(iter != id_map_.end())
    {
        next_id_++;
        iter = id_map_.find(next_id_);
    }

    *id = next_id_;
    next_id_++;

    id_map_.insert(make_pair(*id, key_info));

    if (flags == 1)
    {
        write_key_info_.insert(make_pair(key_info, *id));
    }

    return 0;
}

int32_t IdCenter::ReleaseId(uint32_t id)
{
    Log(logger_, "release id, id is %d")

    Mutex::Locker lock(mutex_);

    map<uint32_t, string>::iterator iter = id_map_.find(id);
    if (iter != id_map_.end())
    {
        Log(logger_, "find id %d, and its stream info is %s. Delete it", id, iter->second.c_str());

        map<string, uint32_t>::iterator key_info_iter = write_key_info_.find(iter->second);
        if (key_info_iter != write_key_info_.end())
        {
            write_key_info_.erase(key_info_iter);
        }
        
        id_map_.erase(iter);
    }
    else
    {
        Log(logger_, "not find id %d", id);
    }

    return 0;
}

int32_t IdCenter::GetStreamInfoFromId(uint32_t id, string &key_info)
{
    Log(logger_, "get stream info from id %d", id);

    Mutex::Locker lock(mutex_);

    map<uint32_t, string>::iterator iter = id_map_.find(id);
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

}
