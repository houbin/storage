#include "stream_transfer_manager.h"

namespace storage
{

StreamTransferClientManager::StreamTransferClientManager(Logger *logger)
: logger_(logger), mutex_(StreamTransferClientManager::Lock), shutdown_(false)
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
    return 0;
}

int32_t StreamTransferClientManager::Shutdown()
{
    Log(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    shutdown_ = true;
    
    while(!transfer_clients_.empty())
    {
        map<uint64_t, StreamTransferClient*>::iterator iter = transfer_clients_.begin();
        if (iter != transfer_clients_.end())
        {
            StreamTransferClient *transfer_client = iter->second;
            assert(transfer_client == NULL);

            transfer_client->Stop();
            transfer_client->Wait();
            transfer_clients_.Erase(iter);
            delete transfer_client;
        }
    }

    JVC_ReleaseSDK();

    return 0;
}

int32_t StreamTransferClientManager::Find(uint64_t stream_id, StreamTransferClient **transfer_client)
{
    Mutex::Locker lock(mutex_);
    Log(logger_, "find stream %d", stream_id);

    map<uint64_t, StreamTransferClient*>::iterator iter = transfer_clients_.find(stream_id);
    if (iter == transfer_clients_.end())
    {
        Log(logger_, "not find stream %d", stream_id);
        return -1;
    }

    *transfer_client = iter->second;
    Log(logger_, "find stream %d end", stream_id);

    return 0;
}

int32_t StreamTransferClientManager::Insert(uint64_t stream_id, StreamTransferClient *transfer_client)
{
    Log(logger_, "insert stream %d", stream_id);
    assert(transfer_client != NULL);

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        delete transfer_client;
        return 0;
    }

    transfer_clients_.insert(make_pair(stream_id, transfer_client));
    transfer_client->Start();

    Log(logger_, "insert stream %d end", stream_id);

    return 0;
}

int32_t StreamTransferClientManager::Erase(uint64_t stream_id)
{
    int32_t ret = 0;
    StreamTransferClient *transfer_client;

    Log(logger_, "erase stream %d", stream_id);

    Mutex::Locker lock(mutex_);
    ret = transfer_clients_.find(stream_id, &transfer_client);
    if (ret < 0)
    {
        Log(logger_, "not find stream %d", stream_id);
    }
    else
    {
        transfer_clients_.Erase(stream_id);
        transfer_client->Stop();
        transfer_client->Wait();
        delete transfer_client;
    }

    Log(logger_, "erase stream %d end", stream_id);

    return 0;
}

}
