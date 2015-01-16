#include "stream_transfer_client_manager.h"
#include "stream_transfer_client.h"
#include "../util/clock.h"

#define MAX_RECORD_STREAM_COUNTS 500
namespace storage
{

StreamTransferClientManager::StreamTransferClientManager(Logger *logger)
: logger_(logger), mutex_(StreamTransferClientManager::Lock), next_apply_stream_id_(0),
active_client_counts_(0), shutdown_(false), timer(logger)
{

}

int32_t StreamTransferClientManager::Init()
{
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
    timer.Init();
    return 0;
}

int32_t StreamTransferClientManager::Shutdown()
{
    Log(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    shutdown_ = true;
    
    while(!transfer_clients_.empty())
    {
        map<uint32_t, StreamTransferClient*>::iterator iter = transfer_clients_.begin();
        StreamTransferClient *transfer_client = iter->second;
        assert(transfer_client == NULL);
        if (!transfer_client->IsStop())
        {
            transfer_client->Stop();
        }
        transfer_client->Shutdown();
        transfer_clients_.erase(iter);
        delete transfer_client;
    }

    JVC_ReleaseSDK();
    timer.Shutdown();

    return 0;
}

int32_t StreamTransferClientManager::Insert(StreamInfo *stream_info)
{
    uint32_t stream_id;

    assert(stream_info != NULL);
    Log(logger_, "insert stream info, stream info is %p", stream_info);

    StreamInfo temp = *stream_info;
    delete stream_info;
    stream_info = NULL;

    Mutex::Locker lock(mutex_);
    if (shutdown_)
    {
        return 0;
    }

    if (active_client_counts_ >= MAX_RECORD_STREAM_COUNTS)
    {
        return -ERR_REACH_MAX_STREAM;
    }

    {
        map<StreamInfo, uint32_t>::iterator iter = stream_id_map_.find(temp);
        if (iter != stream_id_map_.end())
        {
            Log(logger_, "find stream info, stream_id is %d", iter->second);
            stream_id = iter->second;
        }
        else
        {
            /* find a unused stream id */
            map<uint32_t, StreamTransferClient*>::iterator iter = transfer_clients_.find(next_apply_stream_id_);
            while (iter != transfer_clients_.end())
            {
                next_apply_stream_id_++;
                iter = transfer_clients_.find(next_apply_stream_id_);
            }

            stream_id = next_apply_stream_id_++;
            stream_id_map_.insert(make_pair(temp, stream_id));
            
            // TODO 存放 stream_info -> stream id到db中，状态设置为active
            Log(logger_, "apply stream id %d", stream_id);
        }
    }
    
    {
        map<uint32_t, StreamTransferClient*>::iterator iter = transfer_clients_.find(stream_id);
        if (iter != transfer_clients_.end())
        {
            Log(logger_, "find stream id %d in transfer clients", stream_id);
            StreamTransferClient *transfer_client = iter->second;
            assert(transfer_client != NULL);
            if (transfer_client->IsStop())
            {
                transfer_client->Start();
                active_client_counts_++;
            }
        }
        else
        {
            Log(logger, "not find stream id %d in transfer clients", stream_id);
            StreamTransferClient *transfer_client = new StreamTransferClient(logger_, temp);
            assert(transfer_client != NULL);
            transfer_clients_.insert(make_pair(stream_id, transfer_client));
            transfer_client->Start();
            active_client_counts_++;
        }
    }

    {
        // TODO store config to db
    }

    Log(logger_, "insert stream info, stream info is %p, stream id is %d", stream_info, stream_id);

    return 0;
}

int32_t StreamTransferClientManager::Remove(StreamInfo *stream_info)
{
    StreamTransferClient *transfer_client = NULL;
    uint32_t stream_id;

    assert(stream_info != NULL);
    Log(logger_, "remove stream info %p", stream_info);

    StreamInfo temp = *stream_info;
    delete stream_info;
    stream_info = NULL;

    Mutex::Locker lock(mutex_);
    if (shutdown_)
    {
        return 0;
    }

    {
        map<StreamInfo, uint32_t>::iterator iter = stream_id_map_.find(temp);
        if (iter != stream_id_map_.end())
        {
            // 设置 stream info -> stream id 在db中配置为inactive
            stream_id = iter->second; 
        }
        else
        {
            return 0;
        }
    }

    {
        map<uint32_t, StreamTransferClient*>::iterator iter = transfer_clients_.find(stream_id);
        if (iter != transfer_clients_.end())
        {
            Log(logger_, "find stream info %p, stream id %d, stop it", stream_info, stream_id);
            transfer_client = iter->second;
            transfer_client->Stop();
            active_client_counts_--;
        }
        else
        {
            Log(logger_, "find stream info %p, stream id %d", stream_info, stream_id);
        }
    }

    Log(logger_, "erase stream %d end", stream_id);

    return 0;
}

int32_t StreamTransferClientManager::Find(uint64_t stream_id, StreamTransferClient **transfer_client)
{
    Log(logger_, "find stream %d", stream_id);

    Mutex::Locker lock(mutex_);

    if(shutdown_)
    {
        Log(logger_, "should't come here");
        return 0;
    }

    map<uint64_t, StreamTransferClient*>::iterator iter = transfer_clients_.find(stream_id);
    if (iter == transfer_clients_.end())
    {
        Log(logger_, "can't find stream %d", stream_id);
        return -1;
    }

    *transfer_client = iter->second;
    Log(logger_, "find stream %d end", stream_id);

    return 0;
}

/* force_recycle用于表示是否要强制回收文件。若要强制回收，则回收等于视频流数个文件 */
int32_t RecycleRecordFiles(list<RecordFile*> &free_file_list, bool force_recycle)
{
    int32_t ret;
    Log(logger_, "recycle record file");

    Mutex::Locker lock(mutex_);

    UTime now = GetClockNow();
    UTime hold_time(10 * 24 * 3600, 0);
    UTime hold_deadline = now - hold_time;

    /* 回收过期文件 */
    map<uint32_t, StreamTransferClient*>::iterator iter = transfer_clients_.begin();
    while(iter != transfer_clients_.end())
    {
        StreamTransferClient *transfer_client = iter->second;
        assert(transfer_client != NULL);

        ret = transfer_client->RecycleExpiredRecordFiles(free_file_list, hold_deadline);
        assert(ret == 0);

        if (transfer_client->ShouldShutdown())
        {
            //TODO 删除db中stream info -> stream id的配置
            //
            map<uint32_t, StreamTransferClient*>::iterator to_erase = iter;
            iter++;
            transfer_clients_.erase(to_erase);

            StreamInfo *stream_info = NULL;
            ret = transfer_client->GetStreamInfo(&stream_info);
            assert(ret == 0);
            stream_id_map_.erase(*stream_info);

            delete transfer_client;
            transfer_client = NULL;
        }
        else
        {
            iter++;
        }
    }

    // TODO: 带有强制回收标志，需要强制回收视频流数个文件

    return 0;
}

}
