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

StreamOpHandler::StreamManager(Logger *logger)
: logger_(logger), mutex_("StreamOpHandler::Lock"),
  stream_id_manager_(logger_, MAX_STREAM_NUMBERS), stop_(false)
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
        return;
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

    /* adjust stream_infos_ */
    {
        map<StreamInfo, uint64_t>::iterator iter = stream_infos_.find(stream_info);
        if (iter != stream_infos_.end())
        {
            stream_id = iter->second;
        }
        else
        {
            ret = stream_id_manager_.ApplyForFreeId(&stream_id);
            assert(ret == 0);
            
            // TODO store <stream_info, stream_id> to db
            
            stream_infos_.insert(make_pair(*stream_info, stream_id));
        }
    }

    Log(logger_, "stream id is %"PRIu64"", stream_id);

    /* adjust stream_transfer_clients_ */
    {
        map<uint64_t, StreamTransferClient*>::iterator iter = stream_transfer_clients_.find(stream_id);
        if (iter == stream_transfer_clients_.end())
        {
            StreamTransferClient *stream_transfer_client = new StreamTransferClient();
            assert(stream_transfer_client != NULL);

            stream_transfer_clients_.insert(make_pair(stream_id, stream_transfer_client));
            stream_transfer_client.Start();
        }
    }

    Log(logger_, "handle stream add end");

    return 0;
}

int32_t StreamOpHandler::HandleStreamDel(StreamInfo *stream_info)
{
    int32_t ret;
    uint64_t stream_id;

    Log(logger_, "handle stream del");

    /* adjust stream_infos_ */
    {
        map<StreamInfo, uint64_t>::iterator iter = stream_infos_.find(*stream_info);
        if (iter != stream_infos_.end())
        {
            stream_id = iter->second;
            stream_infos_.erase(iter);
        }
        else
        {
            Log(logger_, "not find this stream, stream sid is %d", stream_info->sid_);
            return 0;
        }
    }

    Log(logger_, "stream id is %"PRIu64"", stream_id);

    /* adjust stream_transfer_clients_ */
    {
        map<uint64_t, StreamTransferClient*>::iterator iter = stream_transfer_clients_.find(stream_id);
        if (iter != stream_transfer_clients_.end())
        {
            StreamTransferClient  *stream_transfer_client = iter->second;
            assert(stream_transfer_client != NULL);

            stream_transfer_client->Stop();
            stream_transfer_client->Wait();

            delete stream_transfer_client;
            stream_transfer_clients_.erase(iter);
        }
    }

    Log(logger_, "handle stream del end");
    return 0;
}

void StreamOpHandler::Init()
{
    int ret;

    Log(logger_, "init");

    ret = JVC_InitSDK(-1);
    assert(ret == 0);

    JVC_RegisterCallBack(YST_ClientConnectCallBack,
                         YST_ClientNormalDataCallBack,
                         YST_ClientCheckResultCallBack,
                         YST_ClientChatDataCallBack,
                         YST_ClientTextDataCallBack,
                         YST_ClientDownloadCallBack,
                         YST_ClientPlayDataCallBack
                         );

    JVC_EnableLog(TRUE);
    return;
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
            mutex_.unlock();

            assert (stream_op != NULL);
            assert(stream_op->stream_info != NULL);

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

            safe_free(stream_op->stream_info);
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
    
    return 0;
}

void StreamOpHandler::Stop()
{
    mutex_.Lock();
    stop_ = true;
    cond.Signal();
    mutex_.Unlock();

    JVC_ReleaseSDK();

    return;
}

}

