#ifndef STORAGE_STREAM_MANAGER_H_
#define STORAGE_STREAM_MANAGER_H_

#include <stdint.h>
#include <map>
#include <set>
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/thread.h"
#include "stream.h"
#include "stream_info.h"

using namespace util;

namespace storage {

#define BUFFER_LENGTH (1 * 1024 * 1024)
class StreamManager
{
private:
    Logger *logger_;
    Mutex mutex_;
    bool stop_;

    Mutex request_mutex_;
    Cond request_cond_; 
    set<StreamInfo> record_requests_;

    struct PreRecordThread : public Thread
    {
        StreamManager *manager_;
        PreRecordThread(StreamManager *manager) : manager_(manager) { }
        void *Entry()
        {
            manager_->PrerecordEntry();
            return 0;
        }
    } prerecord_thread;

    map<StreamInfo, uint32_t> recording_streams;

public:
    StreamManager(Logger *logger);

    int32_t EnqueueRecordRequest(StreamInfo &stream_info);
    StreamInfo DequeueRecordRequest();

    void PrerecordEntry();
};

}

#endif
