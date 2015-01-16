#include <assert.h>
#include "stream_op_handler.h"
#include "errcode.h"
#include "../util/mutex.h"
#include "../grpc/grpc.h"
#include "../grpc/storage_json.h"
#include "grpc_userdef.h"
#include "stream_id_manager.h"
#include "stream_op.h"

using namespace util;

namespace storage
{

StreamOpHandler::StreamManager(Logger *logger, StreamTransferClientManager *client_manager)
: logger_(logger), mutex_("StreamOpHandler::Lock"), client_manager_(client_manager), stop_(false)
{

}

int32_t StreamOpHandler::EnqueueStreamOp(StreamOp *stream_op)
{
    Log(logger_, "equeue stream op");

    assert(stream_op != NULL);
    assert(stream_op->stream_op_type == STREAM_OP_ADD);

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        safe_free(stream_op->stream_info);
        safe_free(stream_op);
        return 0;
    }

    stream_op_queue_.push_back(stream_op);
    cond_.Signal();

    return 0;
}

/* when call deque, we should lock outside */
StreamOp *StreamOpHandler::DequeueStreamOp()
{
    Log(logger_, "dequeue stream op");

    assert(!stream_op_queue_.empty());
    
    StreamOp *temp = stream_op_queue_.front();
    stream_op_queue_.pop_front();

    return temp;
}

int32_t StreamOpHandler::HandleStreamAdd(StreamInfo *stream_info)
{
    int32_t ret;
    uint64_t stream_id = 0;

    assert(stream_info != NULL);
    Log(logger_, "handle stream add, stream_info is %p", stream_info);

    client_manager_.Insert(stream_info);

    return 0;
}

int32_t StreamOpHandler::HandleStreamDel(StreamInfo *stream_info)
{
    assert(stream_info != NULL);
    Log(logger_, "handle stream del, stream_info is %p", stream_info);

    int32_t ret = client_manager_.Remove(stream_info);
    assert(ret == 0);

    return 0;
}

void StreamOpHandler::Start()
{
    Log(logger_, "start");

    Create();

    return;
}

void *StreamOpHandler::Entry()
{
    int32_t ret;
    Log(logger_, "entry");

    mutex_.lock();

    while (true)
    {
        while (!stream_op_queue_.empty())
        {
            StreamOp *stream_op = DequeueStreamOp();

            assert (stream_op != NULL);
            assert(stream_op->stream_info != NULL);
            if (stop_)
            {
                safe_free(stream_op->stream_info);
                safe_free(stream_op);
                continue;
            }

            mutex_.Unlock();

            switch (stream_op->stream_op_type)
            {
                case STREAM_OP_ADD:
                    ret = HandleStreamAdd(stream_op->stream_info);
                    break;
                
                case STREAM_OP_DEL:
                    ret = HandleStreamDel(stream_op->stream_info);
                    break;

                default:
                    assert("error stream op" != NULL);
            }

            safe_free(stream_op);
            assert(ret == 0);
            mutex_.Lock();
        }

        if (stop_)
        {
            break;
        }

        cond_.Wait(mutex_);
    }

    mutex_.Unlock();
    
    Log(logger_, "entry end");

    return 0;
}

void StreamOpHandler::Stop()
{
    mutex_.Lock();
    stop_ = true;
    cond.Signal();
    mutex_.Unlock();

    return;
}

}

