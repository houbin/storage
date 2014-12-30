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
#include "stream_id_manager.h"
#include "../grpc/storage_json.h"

using namespace util;

namespace storage {

#define BUFFER_LENGTH (1 * 1024 * 1024)
class StreamManager
{
private:
    Logger *logger_;

    /* stream id manager, using bitmap */
    StreamIdManager stream_id_manager_;
    
    /* vms client config stream record requests */
    bool stop_record_request_;
    Mutex request_mutex_;
    Cond request_cond_; 
    set<StreamInfo> record_requests_;
    struct PreRecordThread : public Thread
    {
        StreamManager *manager_;
        PreRecordThread(StreamManager *manager) : manager_(manager) { }
        void *Entry()
        {
            manager_->PreRecordEntry();
            return 0;
        }
    } prerecord_thread;

    /* recording streams */
    bool stop_recording_;
    Mutex recording_mutex_;
    Cond recording_cond_;
    map<StreamInfo, uint32_t> recording_streams_;
    struct RecordingThread : public Thread
    {
        StreamManager *manager_;
        RecordingThread(StreamManager *stream_manager) : manager_(stream_manager) {}
        void *Entry()
        {
            manager_->RecordingEntry();
            return 0;
        }
    }recording_thread;

public:
    StreamManager(Logger *logger, uint32_t stream_counts);

    void Start();
    void Wait();
    void Shutdown();

    int32_t MallocStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req);
    int32_t FreeStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req);
    int32_t FillStreamServiceReqParam(PARAM_REQ_storage_json_stream_get_service *req, StreamInfo &stream_info);
    
    /* enqueue and dequeue stream record requests */
    int32_t EnqueueRecordRequest(StreamInfo &stream_info);
    StreamInfo DequeueRecordRequest();

    /* enqueue and dequeue recording streams */
    int32_t EnqueueRecordingStream(StreamInfo &stream_info);

    /* entry of thread that get stream server services */
    void PreRecordEntry();

    /* entry of thread that start recording streams */
    void RecordingEntry();
};

}

#endif

