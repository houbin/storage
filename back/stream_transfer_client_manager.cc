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
        map<string, StreamTransferClient*>::iterator iter = transfer_clients_.begin();
        StreamTransferClient *transfer_client = iter->second;
        assert(transfer_client != NULL);
        transfer_client->Shutdown();
        transfer_clients_.erase(iter);
        delete transfer_client;
    }

    timer.Shutdown();

    return 0;
}

int32_t StreamTransferClientManager::Find(string key_info, StreamTransferClient **transfer_client)
{
    Log(logger_, "find stream info %s", key_info.c_str());

    Mutex::Locker lock(mutex_);

    if(shutdown_)
    {
        Log(logger_, "should't come here");
        return -ERR_SHUTDOWN;
    }

    map<string, StreamTransferClient*>::iterator iter = transfer_clients_.find(key_info);
    if (iter == transfer_clients_.end())
    {
        Log(logger_, "can't find stream %s", key_info.c_str());
        return -ERR_RECODE_NOT_FOUND;
    }

    *transfer_client = iter->second;
    Log(logger_, "find stream %s end", key_info.c_str());

    return 0;
}

int32_t StreamTransferClientManager::Erase(string key_info)
{
    StreamTransferClient *client = NULL;
    Log(logger_, "Erase stream, stream info is %p", key_info);

    map<string, StreamTransferClient*>::iterator iter = transfer_clients_.find(key_info);
    if (iter != transfer_clients_.end())
    {
        Log(logger_, "find stream info %p, stream id %d, erase it", stream_info, stream_id);
        client = iter->second;
        client->Stop();
        delete client;

        transfer_clients_.erase(iter);
    }
    else
    {
        Log(logger_, "not find stream , and stream info is %p", key_info.c_str());
    }
    
    return 0;
}

/* force_recycle用于表示是否要强制回收文件。若要强制回收，则回收等于视频流数个文件 */
int32_t RecycleRecordFiles(list<RecordFile*> &free_file_list, bool force_recycle)
{
    int32_t ret;
    Log(logger_, "recycle record file");

    mutex_.Lock();

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

int32_t StreamTransferClientManager::Open(string key_info, int flags)
{
    Log(logger_, "open, key info is %s, flags is %d", key_info.c_str(), flags);

    Mutex::Locker lock(mutex_);

    map<string, StreamTransferClient*>::iterator iter = transfer_clients_.find(key_info);
    if (iter == transfer_clients_.end())
    {
        StreamTransferClient *transfer_client = new StreamTransferClient(logger_, temp);
        assert(transfer_client != NULL);

        pair<map<string, StreamTransferClient*>::iterator, bool> ret;
        ret = transfer_clients_.insert(make_pair(key_info, transfer_client));
        assert(ret->second == false);
        transfer_client->Start();

        iter = ret->first;
    }

    if (flags == 0)
    {
        /* TODO */
    }

    return 0;
}

}
