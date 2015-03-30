#include "store_client_center.h"
#include "storage.h"
#include "store_types.h"

namespace storage
{

uint64_t kOpSeq = 0;

StoreClientCenter::StoreClientCenter(Logger *logger)
: logger_(logger), rwlock_("StoreClientCenter::RWLock"), recycle_mutex_("StoreClientCenter::RecycleMutex"),
recycle_event_(NULL), timer_lock("StoreClientCenter::timer_lock"), timer(logger_, timer_lock)
{
    clients_.resize(MAX_STREAM_COUNTS, 0);
}

int32_t StoreClientCenter::Init()
{
    {
        Mutex::Locker lock(timer_lock);
        timer.Init();
    }

    return 0;
}

int32_t StoreClientCenter::Open(int flag, int32_t id, string &stream_info)
{
    assert(flag == 0 || flag == 1);

    int32_t ret;
    StoreClient *client = NULL;

    if (flag == 0)
    {
        ret = FindStoreClient(stream_info, &client);
        if (ret != 0)
        {
            LOG_WARN(logger_, "find store client error, stream info %s, ret %d", stream_info.c_str(), ret);
            return -ERR_ITEM_NOT_FOUND;
        }

        {
            RWLock::WRLocker lock(rwlock_);
            clients_[id] = client;
        }

        ret = client->OpenRead(id);
        LOG_DEBUG(logger_, "open read ok, id %d, flag %d, stream info %s", id, flag, stream_info.c_str());
    }
    else
    {
        ret = FindStoreClient(stream_info, &client);
        if (ret != 0)
        {
            client = new StoreClient(logger_, stream_info);
            assert(client != NULL);

            {
                RWLock::WRLocker lock(rwlock_);
                clients_[id] = client;
                client_search_map_.insert(make_pair(stream_info, client));
            }

            LOG_DEBUG(logger_, "no store client, so new store client, id %d, flag %d, stream info %s", id, flag, stream_info.c_str());
        }
        else
        {
            RWLock::WRLocker lock(rwlock_);
            clients_[id] = client;
        }

        ret = client->OpenWrite(id);
        LOG_DEBUG(logger_, "open write ok, id %d, flag %d, stream info %s", id, flag, stream_info.c_str());
    }

    return ret;
}

int32_t StoreClientCenter::AddStoreClient(string &stream_info, StoreClient **client)
{
    LOG_DEBUG(logger_, "add store client, stream info %s", stream_info.c_str());

    *client = new StoreClient(logger_, stream_info);
    assert(*client != NULL);

    RWLock::WRLocker lock(rwlock_);
    client_search_map_.insert(make_pair(stream_info, *client));

    return 0;
}

int32_t StoreClientCenter::GetStoreClient(int32_t id, StoreClient **client)
{
    assert(client != NULL);

    if (id >= MAX_STREAM_COUNTS)
    {
        LOG_WARN(logger_, "id %d exceed max stream id", id);
        return -ERR_ITEM_NOT_FOUND;
    }

    RWLock::RDLocker lock(rwlock_);
    *client = clients_[id];
    if (*client == NULL)
    {
        LOG_WARN(logger_, "store client is NULL, id %d not exist", id);
        return -ERR_ITEM_NOT_FOUND;
    }

    LOG_DEBUG(logger_, "get store client ok, id is %d", id);

    return 0;
}

int32_t StoreClientCenter::FindStoreClient(string stream_info, StoreClient **client)
{
    assert(client != NULL);

    RWLock::RDLocker lock(rwlock_);
    map<string, StoreClient*>::iterator iter = client_search_map_.find(stream_info);
    if (iter == client_search_map_.end())
    {
        LOG_WARN(logger_, "find stream info error, stream info [%s]", stream_info.c_str());
        return -ERR_ITEM_NOT_FOUND;
    }

    *client = iter->second;
    LOG_DEBUG(logger_, "find store client ok, stream info is %s", stream_info.c_str());

    return 0;
}

int32_t StoreClientCenter::RemoveStoreClient(StoreClient *client)
{
    assert(client != NULL);
    
    RWLock::WRLocker lock(rwlock_);

    string stream_info = client->GetStreamInfo();
    map<string, StoreClient*>::iterator map_iter = client_search_map_.find(stream_info);
    assert(map_iter != client_search_map_.end());
    client_search_map_.erase(map_iter);

    client->Shutdown();
    delete client;
    client = NULL;

    LOG_DEBUG(logger_, "remove store client ok, client is %p", client);
    return 0;
}

int32_t StoreClientCenter::Close(int32_t id, int flag)
{
    assert(flag == 0 || flag == 1);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        LOG_WARN(logger_, "get store client error, id %d, ret %d", id, ret);
        return ret;
    }

    if (flag == 0)
    {
        ret = client->CloseRead(id);
        if (ret != 0)
        {
            LOG_ERROR(logger_, "close read error, id %d, ret %d", id, ret);
        }
    }
    else
    {
        ret = client->CloseWrite(id);
        if (ret != 0)
        {
            LOG_ERROR(logger_, "close write error, id %d, ret %d", id, ret);
        }
    }

    {
        RWLock::WRLocker lock(rwlock_);
        clients_[id] = NULL;
    }

    if (ret == 0)
        LOG_DEBUG(logger_, "close store client ok, id %d, ret %d", id, ret);

    return ret;
}

int32_t StoreClientCenter::WriteFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    LOG_TRACE(logger_, "write frame, id %d, frame %p, buffer size %d", id, frame, frame->size);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        LOG_WARN(logger_, "get store client error, id %d, ret %d", id, ret);
        return ret;
    }

    return client->EnqueueFrame(frame);
}

int32_t StoreClientCenter::SeekRead(int32_t id, UTime &stamp)
{
    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        LOG_WARN(logger_, "get store client error, id %d, ret %d", id, ret);
        return ret;
    }

    ret = client->SeekRead(id, stamp);
    if (ret != 0)
    {
        LOG_WARN(logger_, "seek frame error, id %d, ret %d", id, ret);
        return ret;
    }

    LOG_DEBUG(logger_, "seek read ok, id %d, stamp %d.%d", id, stamp.tv_sec, stamp.tv_nsec);
    return 0;
}

int32_t StoreClientCenter::ReadFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        LOG_WARN(logger_, "get store client error, id %d, ret %d", id, ret);
        return ret;
    }

    ret = client->ReadFrame(id, frame);
    if (ret != 0)
    {
        LOG_WARN(logger_, "read frame error, id %d, ret %d", id, ret);
        return ret;
    }

    LOG_TRACE(logger_, "read frame ok, id %d", id);
    return 0;
}

int32_t StoreClientCenter::ListRecordFragments(int32_t id, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    Log(logger_, "list record fragments, id is %d, start time is %d.%d, end time is %d.%d", 
        id, start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);

    int32_t ret;
    StoreClient *store_client = NULL;

    ret = GetStoreClient(id, &store_client);
    if (ret != 0)
    {
        LOG_WARN(logger_, "get store client error, id %d, start time %d.%d, end time %d.%d, ret %d", 
                    id, start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec, ret);
        return ret;
    }

    ret = store_client->ListRecordFragments(start, end, frag_info_queue);
    if (ret != 0)
    {
        LOG_WARN(logger_, "list record fragments error, id %d, start time %d.%d, end time %d.%d, ret %d",
                    id, start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec, ret);
        return ret;
    }

    LOG_DEBUG(logger_, "list record fragments ok, id %d, start time %d.%d, end time %d.%d, ret %d",
                id, start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec, ret);
    return 0;
}

int32_t StoreClientCenter::UpdateRecordFileInRecycleQueue(StoreClient *store_client, RecordFile *record_file)
{
    assert(store_client != NULL);
    assert(record_file != NULL);

    int32_t ret;

    ret = RemoveFromRecycleQueue(record_file);
    assert (ret == 0);

    ret = AddToRecycleQueue(store_client, record_file);
    assert(ret == 0);

    return 0;
}

int32_t StoreClientCenter::AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file)
{
    assert(store_client != NULL);
    assert(record_file != NULL);

    Mutex::Locker lock(recycle_mutex_);
    LOG_DEBUG(logger_, "add to recycle queue, store_client is %p, record_file is %p", store_client, record_file);

    UTime end_time = record_file->end_time_;

    RecycleItem recycle_item;
    recycle_item.store_client = store_client;
    recycle_item.record_file = record_file;

    multimap<UTime, RecycleItem>::iterator insert_iter = recycle_map_.insert(make_pair(end_time, recycle_item));

    pair<map<RecordFile*, multimap<UTime, RecycleItem>::iterator>::iterator, bool> ret;
    ret = recycle_item_search_map_.insert(make_pair(record_file, insert_iter));
    assert(ret.second == true);

    return 0;
}

int32_t StoreClientCenter::RemoveFromRecycleQueue(RecordFile *record_file)
{
    Mutex::Locker lock(recycle_mutex_);

    map<RecordFile*, multimap<UTime, RecycleItem>::iterator>::iterator search_iter = recycle_item_search_map_.find(record_file);
    assert(search_iter != recycle_item_search_map_.end());

    multimap<UTime, RecycleItem>::iterator iter = search_iter->second;
    assert(iter != recycle_map_.end());

    recycle_map_.erase(iter);
    recycle_item_search_map_.erase(search_iter);
    
    return 0;
}

int32_t StoreClientCenter::StartRecycle()
{
    Mutex::Locker lock(timer_lock);
    
    if (recycle_event_ != NULL)
    {
        return 0;
    }

    recycle_event_ = new C_Recycle(this);
    assert(recycle_event_ != NULL);

    timer.AddEventAfter(0, recycle_event_);

    return 0;
}

int32_t StoreClientCenter::Recycle()
{
    uint32_t recycle_count = 0;
    int32_t ret = 0;

    Mutex::Locker lock(recycle_mutex_);
    multimap<UTime, RecycleItem>::iterator iter = recycle_map_.begin();
    while(iter != recycle_map_.end() && recycle_count < kFilesPerRecycle)
    {
        RecycleItem recycle_item = iter->second;
        RecordFile *record_file = recycle_item.record_file;
        StoreClient *store_client = recycle_item.store_client;

        assert(record_file != NULL);
        assert(store_client != NULL);

        ret = store_client->RecycleRecordFile(record_file);
        if (ret == -ERR_RECORD_FILE_BUSY)
        {
            LOG_DEBUG(logger_, "recycle continue, record file %srecord_%05d is using", record_file->base_name_.c_str(), record_file->number_);
            iter++;
            continue;
        }

        assert(ret == 0);

        if (store_client->IsRecordFileEmpty())
        {
            ret = store_client_center->RemoveStoreClient(store_client);
            assert(ret == 0);
            LOG_DEBUG(logger_, "remove store client %d", store_client);
        }

        multimap<UTime, RecycleItem>::iterator del_iter = iter;
        iter++;
        recycle_map_.erase(del_iter);
        recycle_item_search_map_.erase(record_file);

        /* add to free file table */
        free_file_table->Put(record_file);
        recycle_count++;
    }

    assert(iter != recycle_map_.end());

    recycle_event_ = NULL;
    return 0;
}

void StoreClientCenter::Shutdown()
{
    {
        RWLock::WRLocker lock(rwlock_);
        map<string, StoreClient*>::iterator iter = client_search_map_.begin();
        for(; iter != client_search_map_.end(); iter++)
        {
            StoreClient *store_client = iter->second;
            assert(store_client != NULL);
            store_client->Shutdown();
            delete store_client;
            iter->second = NULL;
        }
    }

    {
        Mutex::Locker lock(timer_lock);
        timer.Shutdown();
    }

    return;
}

int32_t StoreClientCenter::DumpClientSearchMap()
{
    RWLock::RDLocker lock(rwlock_);

    map<string, StoreClient*>::iterator iter = client_search_map_.begin();
    for (; iter != client_search_map_.end(); iter++)
    {
        string temp = iter->first;
        StoreClient *store_client = iter->second;
        string stream_info = store_client->GetStreamInfo();
        fprintf(stderr, "stream info is %s, storeclient is %p, stream info of store client is %s\n", temp.c_str(), store_client,
            stream_info.c_str());
    }

    return 0;
}

}

