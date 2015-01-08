#ifndef STORAGE_STREAM_SERVER_CLIENT_H_
#define STORAGE_STREAM_SERVER_CLIENT_H_

#include <deque>
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/logger.h"
#include "stream_op.h"
#include "stream_op_handler.h"

using namespace std;

namespace storage
{

class StreamServerClient : public Thread
{
private:
    Logger *logger_;

    Mutex mutex_;
    Cond cond_;
    deque<StreamOp *> stream_op_queue_;
    bool stop_;

    StreamManager *stream_manager_;

public:
    StreamServerClient(Logger *logger, StreamManager *stream_manager_);
    int32_t EnqueueStreamOp(StreamOp *stream_op);
    StreamOp *DequeueStreamOp();

    int32_t MallocStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req);
    int32_t FreeStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req);
    int32_t FillStreamServiceReqParam(PARAM_REQ_storage_json_stream_get_service *req, StreamInfo *stream_info);

    int32_t Start();
    virtual void *Entry();
    int32_t Stop();
};

}


#endif
