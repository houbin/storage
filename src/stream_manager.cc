#include <assert.h>
#include "stream_manager.h"
#include "errcode.h"
#include "../util/mutex.h"
#include "../grpc/grpc.h"
#include "../grpc/storage_json.h"
#include "grpc_userdef.h"
#include "stream_id_manager.h"

using namespace util;

namespace storage
{

StreamManager::StreamManager(Logger *logger, uint32_t max_stream_numbers) 
: logger_(logger), 
  stream_id_manager_(logger_, max_stream_numbers),
  stop_record_request_(false),
  request_mutex_("StreamManager::request_mutex_"),
  prerecord_thread(this),
  stop_recording_(false),
  recording_mutex_("StreamManager::recording_mutex_"),
  recording_thread(this)
{

}

void StreamManager::Start()
{
    Log(logger_, "start");

    prerecord_thread.Create();
    recording_thread.Create();

    return;
}

void StreamManager::Shutdown()
{
    request_mutex_.Lock();
    stop_record_request_ = true;
    request_cond_.Signal();
    request_mutex_.Unlock();

    recording_mutex_.Lock();
    stop_recording_ = true;
    recording_cond_.Signal();
    recording_mutex_.Unlock();

    return;
}

void StreamManager::Wait()
{
    prerecord_thread.Join();
    recording_thread.Join();
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

int32_t StreamManager::EnqueueRecordingStream(StreamInfo &stream_info)
{
    Log(logger_, "EnqueueRecordingStream");

    Mutex::Locker locker(recording_mutex_);
    
    map<StreamInfo, uint32_t>::iterator iter = recording_streams_.find(stream_info);
    if (iter != recording_streams_.end())
    {
        Log(logger_, "this stream_info already exist, stream_id is %d", iter->second);
        return 0;
    }

    /* apply for stream id */
    int32_t free_id = stream_id_manager_.ApplyForFreeId();
    if (free_id < 0)
    {
        Log(logger_, "stream_id_manager_ apply for free id error");
        return -1;
    }

    /* TODO: save stream_info -> stream_id to db */

    recording_streams_.insert(make_pair(stream_info, free_id));
    stream_info.PrintToLogFile(logger_);
    Log(logger_, "free_id is %d", free_id);
    recording_cond_.Signal();

    return 0;
}

int32_t StreamManager::MallocStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req)
{
    StreamInfo stream_info;

    req->type = (char *)malloc(sizeof(stream_info.stream_type_));
    req->sid = (char *)malloc(sizeof(stream_info.sid_));
    req->protocol = (char *)malloc(sizeof(stream_info.protocol_));
    req->mainstream = (char *)malloc(sizeof(stream_info.main_stream_rstp_addr_));
    req->substream = (char *)malloc(sizeof(stream_info.sub_stream_rstp_addr_));
    req->ip = (char *)malloc(sizeof(stream_info.ip_));
    req->port = (char *)malloc(sizeof(stream_info.port_));

    return 0;
}

int32_t StreamManager::FreeStreamServiceReq(PARAM_REQ_storage_json_stream_get_service *req)
{
    safe_free(req->type);
    safe_free(req->sid);
    safe_free(req->protocol);
    safe_free(req->mainstream);
    safe_free(req->substream);
    safe_free(req->ip);
    safe_free(req->port);

    return 0;
}

int32_t StreamManager::FillStreamServiceReqParam(PARAM_REQ_storage_json_stream_get_service *req, StreamInfo &stream_info)
{
    Log(logger_, "FillStreamServiceReqParam");

    memcpy(req->type, stream_info.stream_type_, sizeof(stream_info.stream_type_));
    memcpy(req->sid, stream_info.sid_, sizeof(stream_info.sid_));
    memcpy(req->protocol, stream_info.protocol_, sizeof(stream_info.protocol_));
    req->bMainStream = stream_info.main_stream_;
    req->bSubStream = stream_info.sub_stream_;
    memcpy(req->mainstream, stream_info.main_stream_rstp_addr_, sizeof(stream_info.main_stream_rstp_addr_));
    memcpy(req->substream, stream_info.sub_stream_rstp_addr_, sizeof(stream_info.sub_stream_rstp_addr_));
    memcpy(req->ip, stream_info.ip_, sizeof(stream_info.ip_));
    memcpy(req->port, stream_info.port_, sizeof(stream_info.port_));
    req->channelcnt = stream_info.channelcnt_;
    req->channelid = stream_info.channel_id_;
    
    return 0;
}

/* entry of thread that get stream server services */
void StreamManager::PreRecordEntry()
{
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

    request_mutex_.Lock();
    Log(logger_, "PrerecordEntry");

    while(!stop_record_request_)
    {
        while(!record_requests_.empty())
        {
            StreamInfo stream_info = DequeueRecordRequest();
            request_mutex_.Unlock();

            stream_info.PrintToLogFile(logger_);

            /* if stop */
            if (stop_record_request_)
            {
                request_mutex_.Lock();
                continue;
            }

            /* stream_server_ip为空，则直接放入recording_streams_ */
            if (strlen(stream_info.stream_server_ip_) == 0)
            {
                Log(logger_, "stream_info.stream_server_ip is NULL, enqueue to recording streams");
                EnqueueRecordingStream(stream_info);
                request_mutex_.Lock();
                continue;
            }

            UserDefInfo user_info;
            user_info.logger = logger_;
            user_info.fd = fd;
            struct in_addr temp;
            ret = inet_pton(AF_INET, stream_info.stream_server_ip_, (void *)&temp);
            if (ret != 1)
            {
                Log(logger_, "stream_server_ip_ is %s", stream_info.stream_server_ip_);
                request_mutex_.Lock();
                continue;
            }

            user_info.send_addr.sin_family = AF_INET;
            user_info.send_addr.sin_addr.s_addr = temp.s_addr;
            user_info.send_addr.sin_port = htons(stream_info.stream_server_port_);
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
            if ((ret == 0) && (resp.port != NULL))
            {
                Log(logger_, "get stream server service channel id %d, resp.port is %s", resp.channelid, resp.port);

                stream_info.stream_server_channel_id_ = resp.channelid;
                stream_info.stream_server_data_services_port_ = atoi(resp.port);
                EnqueueRecordingStream(stream_info);
            }
            else
            {
                Log(logger_, "get stream server service error, ret is %d, resp.port is %p", ret, resp.port);
            }

            /* ignore the situation that get stream services failed */

            grpc_end(grpc);
            request_mutex_.Lock();
        }
        
        if (stop_record_request_)
        {
            break;
        }

        request_cond_.Wait(request_mutex_);
    }

    request_mutex_.Unlock();
    FreeStreamServiceReq(&req);
    grpc_delete(grpc);
    close(fd);

    return ;
}

void StreamManager::RecordingEntry()
{
    Log(logger_, "RecordingEntry");

    recording_mutex_.Lock();
    while(!stop_recording_)
    {
        while (!recording_streams_.empty())
        {
            map<StreamInfo, uint32_t>::iterator iter = recording_streams_.begin();
            StreamInfo stream_info = iter->first;
            uint32_t stream_id = iter->second;

            Log(logger_, "stream_id is %d", stream_id);
            recording_streams_.erase(iter);
            recording_mutex_.Unlock();

            /* TODO recording stream to disk */
            
            recording_mutex_.Lock();
        }

        recording_cond_.Wait(recording_mutex_);
    }

    recording_mutex_.Unlock();
    return;
}

}

