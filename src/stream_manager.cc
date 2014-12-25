#include "stream_manager.h"
#include "errcode.h"
#include "../util/mutex.h"

namespace storage
{

StreamInfoSet::StreamInfoSet(Logger *logger)
: logger_(logger), mutex_("StreamInfoSet::mutex_")
{
    
}

int32_t StreamInfoSet::Add(StreamInfo *stream_info)
{
    Log(logger_, "add stream, sid is %s", stream_info->sid);

    Mutex::Locker locker(mutex_);
    set<StreamInfo>::iterator iter = record_requests_.find(*stream_info);
    if (iter != record_requests_.end())
    {
        record_requests_.erase(iter);
    }

    record_requests_.insert(*stream_info);
    return 0;
}

int32_t StreamInfoSet::Remove(StreamInfo *stream_info)
{
    Log(logger_, "remove stream, sid is %s", stream_info_.sid);

    Mutex::Locker locker(mutex_);
    set<StreamInfo>::iterator iter = record_requests_.find(*stream_info);
    if (iter != record_requests_.end())
    {
        record_requests_.erase(iter);
    }

    return 0;
}

StreamManager::StreamManager(Logger *logger)
: logger_(logger), mutex_("StreamManager::mutex_")
{
    
}

int32_t StreamManager::AddRecordRequest(StreamInfo &stream_info)
{
    int32_t ret = 0;
    ret = unrecorded_streams.Add(stream_info);
    if (ret != 0)
    {
        return ret;
    }

    
}



}
