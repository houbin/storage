#ifndef STORAGE_STREAM_MANAGER_H_
#define STORAGE_STREAM_MANAGER_H_

#include <stdint.h>
#include <map>
#include <set>
#include "../util/cond.h"
#include "stream.h"
#include "stream_addr.h"

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
    set<StreamInfo> unrecorded_streams;

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
    int32_t DequeueRecordRequest(StreamInfo &stream_info);

    void PrerecordEntry();
};

}

#endif
