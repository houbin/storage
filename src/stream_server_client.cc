#include "stream_server_client.h"
#include "../grpc/storage_json.h"
#include "../grpc/grpc.h"
#include "../util/errcode.h"

namespace storage
{

StreamServerClient::StreamServerClient(Logger *logger, StreamManager *stream_manager)
: logger_(logger), mutex_("StreamServerClient::Locker"), stop_(false), stream_manager_(stream_manager)
{

}

int32_t StreamServerClient::EnqueueStreamOp(StreamOp *stream_op)
{
    if (stop_)
    {
        Log(logger_, "have been stopped");
        safe_free(stream_op->stream_info);
        safe_free(stream_op);
        
        return 0;
    }

    Mutex::Locker lock(mutex_);

    stream_op_queue_.push_back(stream_op);
    
    cond.Signal();

    return 0;
}

StreamOp* StreamServerClient::DequeueStreamOp()
{
    /* make sure locked */
    assert(!stream_op_queue_.empty());

    StreamOp *stream_op = stream_op_queue_.front();
    stream_op_queue_.pop_front();
    
    return stream_op;
}

int32_t StreamServerClient::Start()
{
    Log(logger_, "Start");

    Create();

    return 0;
}

int32_t StreamServerClient::MallocStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req)
{
    StreamInfo stream_info;

    req->type = (char *)malloc(sizeof(stream_info.stream_type_));
    req->sid = (char *)malloc(sizeof(stream_info.sid_));
    req->protocol = (char *)malloc(sizeof(stream_info.protocol_));
    req->mainstream = (char *)malloc(sizeof(stream_info.main_stream_rstp_addr_));
    req->substream = (char *)malloc(sizeof(stream_info.sub_stream_rstp_addr_));
    req->ip = (char *)malloc(sizeof(stream_info.ip_));
    return 0;
}

int32_t StreamServerClient::FreeStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req)
{
    safe_free(req->type);
    safe_free(req->sid);
    safe_free(req->protocol);
    safe_free(req->mainstream);
    safe_free(req->substream);
    safe_free(req->ip);

    return 0;
}

int32_t StreamServerClient::FillStreamServiceReqParam(PARAM_REQ_storage_json_stream_get_service *req, StreamInfo *stream_info)
{
    Log(logger_, "FillStreamServiceReqParam");

    memcpy(req->type, stream_info->stream_type_, sizeof(stream_info->stream_type_));
    memcpy(req->sid, stream_info->sid_, sizeof(stream_info->sid_));
    memcpy(req->protocol, stream_info->protocol_, sizeof(stream_info->protocol_));
    req->bMainStream = stream_info->main_stream_;
    req->bSubStream = stream_info->sub_stream_;
    memcpy(req->mainstream, stream_info->main_stream_rstp_addr_, sizeof(stream_info->main_stream_rstp_addr_));
    memcpy(req->substream, stream_info->sub_stream_rstp_addr_, sizeof(stream_info->sub_stream_rstp_addr_));
    memcpy(req->ip, stream_info->ip_, sizeof(stream_info->ip_));
    req_->port = stream_info->port_;
    req->channelcnt = stream_info->channelcnt_;
    req->channelid = stream_info->channel_id_;
    
    return 0;
}
}

void *StreamServerClient::Entry()
{
    Log(logger_, "entry");

    int ret = 0;
    int optval = 1;
    int fd = -1;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        Log(logger_, "create socket error, error is %s", strerror(errno));
        assert(fd >= 0);
    }
    
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret < 0)
    {
        Log(logger_, "setsockopt SO_REUSEADDR error, error is %s", strerror(errno));
        assert(ret >= 0);
    }

    /* malloc req */
    PARAM_REQ_storage_json_stream_get_service req;
    MallocStreamServiceReq(&req);

    /* new grpc */
    grpc_t *grpc = grpc_new();

    mutex_.Lock();
    Log(logger_, "PrerecordEntry");

    while(true)
    {
        while(!stream_op_queue_.empty())
        {
            StreamOp *stream_op = DequeueStreamOp();
            if (stop_)
            {
                safe_free(stream_op->stream_info);
                safe_free(stream_op);
                continue;
            }

            mutex_.Unlock();

            Stream_info *stream_info = stream_op->stream_info;
            stream_info->PrintToLogFile(logger_);

            if ((strlen(sstream_info->stream_server_ip_) == 0) 
                || (stream_op->StreamOpType == STREAM_OP_DEL))
            {
                Log(logger_, "stream_server_ip is NULL or stream op: del, enqueue to stream manager");
                stream_manager_->EnqueueStreamOp(stream_op);
                mutex_.Lock();
                continue;
            }

            UserDefInfo user_info;
            user_info.logger = logger_;
            user_info.fd = fd;
            struct in_addr temp;
            ret = inet_pton(AF_INET, stream_info->stream_server_ip_, (void *)&temp);
            if (ret != 1)
            {
                Log(logger_, "stream_server_ip_ is %s", stream_info->stream_server_ip_);
                mutex_.Lock();
                continue;
            }

            user_info.send_addr.sin_family = AF_INET;
            user_info.send_addr.sin_addr.s_addr = temp.s_addr;
            user_info.send_addr.sin_port = htons(stream_info->stream_server_port_);
            memset(&(user_info.recv_addr), 0, sizeof(user_info.recv_addr));
            user_info.if_use_recv_addr_to_send = false;
            user_info.parent = this;

            grpcInitParam_t init_param;
            init_param.userdef = &user_info;
            init_param.fptr_net.recv = RecvUdp;
            init_param.fptr_net.send = SendUdp;

            grpc_init(grpc, &init_param);
            grpc->timeout_milliseconds = 5000;

            /* fill */
            FillStreamServiceReqParam(&req, stream_info);

            PARAM_RESP_storage_json_stream_get_service resp;

            ret = CLIENT_storage_json_stream_get_service(grpc, &req, &resp);
            if (ret == 0)
            {
                Log(logger_, "get stream server service channel id %d, resp.port is %d", resp.channelid, resp.port);

                stream_info->stream_server_channel_id_ = resp.channelid;
                stream_info->stream_server_data_services_port_ = resp.port;
                stream_manager->EnqueueStreamOp(stream_op);
            }
            else
            {
                Log(logger_, "get stream server service error, ret is %d", ret);
            }

            /* ignore the situation that get stream services failed */

            grpc_end(grpc);
            mutex_.Lock();
        }
        
        if (stop_)
        {
            break;
        }

        cond_.Wait(mutex_);
    }

    mutex_.Unlock();
    FreeStreamServiceReq(&req);
    grpc_delete(grpc);
    close(fd);

    return ;
}

int32_t StreamServerClient::Stop()
{
    Log(logger_, "stop");

    mutex_.Lock();
    stop_ = true;
    cond_.Signal();
    mutex.Unlock();

    return 0;
}

}
