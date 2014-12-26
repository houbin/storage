#include <assert.h>
#include "stream_manager.h"
#include "errcode.h"
#include "../util/mutex.h"
#include "../grpc/grpc.h"
#include "../grpc/storage_json.h"
#include "grpc_userdef.h"

using namespace util;

namespace storage
{

StreamManager::StreamManager(Logger *logger) 
: logger_(logger), 
  mutex_("StreamManager::mutex_"),
  stop_(false),
  request_mutex_("StreamManager::request_mutex_"),
  prerecord_thread(this)
{

}

int32_t StreamManager::EnqueueRecordRequest(StreamInfo &stream_info)
{
    Log(logger_, "AddRecordRequest");

    Mutex::Locker locker(request_mutex_);

    set<StreamInfo>::iterator iter = record_requests_.find(stream_info);
    if (iter != record_requests_.end())
    {
        record_requests_.erase(iter);
    }

    record_requests_.insert(stream_info);
    request_cond_.Signal();
    return 0;
}

StreamInfo StreamManager::DequeueRecordRequest()
{
    StreamInfo stream_info;
    Log(logger_, "RemoveRecordRequest");

    if (!record_requests_.empty())
    {
        set<StreamInfo>::iterator iter = record_requests_.begin();
        stream_info = *iter;
        record_requests_.erase(iter);
    }
    
    return stream_info;
}

void StreamManager::PrerecordEntry()
{
    request_mutex_.Lock();
    Log(logger_, "PrerecordEntry");

    while(!stop_)
    {
        while(!record_requests_.empty())
        {
            StreamInfo stream_info = DequeueRecordRequest();
            request_mutex_.Unlock();

            stream_info.PrintToLogFile(logger_);
/*
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0)
            {
                Log(logger_, "create socket error, error is %s", strerror(errno));
                return -errno;
            }

            grpcInitParam_t init_param;
            UserDefInfo user_info;
            user_info.logger = this->logger_;
            memcpy(&(user_info.send_addr), &(stream_info.))
*/
            
        }
        
    }
}

}

