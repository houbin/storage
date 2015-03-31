#ifndef STORAGE_STORE_CLIENT_CENTER_H_
#define STORAGE_STORE_CLIENT_CENTER_H_

#include <vector>
#include <map>
#include "../include/errcode.h"
#include "../include/storage_api.h"
#include "../util/logger.h"
#include "../util/rwlock.h"
#include "store_types.h"
#include "config_opts.h"
#include "id_center.h"
#include "free_file_table.h"
#include "store_client.h"

using namespace std;
using namespace util;

namespace storage
{

class RecordFile;

typedef struct recycle_item
{
    RecordFile *record_file;
    StoreClient *store_client;
}RecycleItem;

class C_Recycle;
class StoreClientCenter
{
private:
    Logger *logger_;

    /* used for add/delete/find client */
    RWLock rwlock_;
    vector<StoreClient*> clients_;
    map<string, StoreClient*> client_search_map_;
    
    Mutex recycle_mutex_;
    // use end time of record file as key
    multimap<UTime, RecycleItem> recycle_map_;
    map<RecordFile*, multimap<UTime, RecycleItem>::iterator> recycle_item_search_map_;

    C_Recycle *recycle_event_;

    int32_t FindStoreClientUnlocked(string stream_info, StoreClient **client);
    int32_t GetStoreClientUnlocked(int32_t id, StoreClient **client);
    int32_t RemoveStoreClient(StoreClient *client);

public:
    Mutex timer_lock;
    SafeTimer timer;

    StoreClientCenter(Logger *logger);

    int32_t Init();
    int32_t Open(int flags, int32_t id, string &stream_info);
    int32_t Close(int32_t id, int flag);
    int32_t AddStoreClient(string &stream_info, StoreClient **client);

    int32_t FindStoreClient(string stream_info, StoreClient **client);
    int32_t GetStoreClient(int32_t id, StoreClient **client);

    int32_t WriteFrame(int32_t id, FRAME_INFO_T *frame);
    int32_t SeekRead(int32_t id, UTime &stamp);
    int32_t ReadFrame(int32_t id, FRAME_INFO_T *frame);

    int32_t ListRecordFragments(int32_t id, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue);

    int32_t UpdateRecordFileInRecycleQueue(StoreClient *store_client, RecordFile *record_file);
    int32_t AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file);
    int32_t RemoveFromRecycleQueue(RecordFile *record_file);
    int32_t StartRecycle();
    int32_t Recycle();

    void Shutdown();

    int32_t DumpClientSearchMap();
};

class C_Recycle : public Context
{
public:
    StoreClientCenter *store_client_center_;

    C_Recycle(StoreClientCenter *store_client_center)
    : store_client_center_(store_client_center)
    {
    
    }

    void Finish(int r)
    {
        store_client_center_->Recycle();
        return;
    }
};

}

#endif

