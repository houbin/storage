#ifndef STORAGE_STREAM_TRANSFER_MANAGER_H_
#define STORAGE_STREAM_TRANSFER_MANAGER_H_

#include <list>
#include "../util/logger.h"
#include "../util/mutex.h"
#include "../util/timer.h"
#include "stream_transfer_client.h"
#include "stream_id_manager.h"

using namespace std;
using namespace util;

class StreamTransferClientManager
{
private:
    Logger *logger_;
    Mutex mutex_;

    /* stream info -> client */
    map<string, StreamTransferClient*> transfer_clients_;

    bool shutdown_;

    SafeTimer timer;

public:
    StreamTransferClientManager(Logger *logger);

    int32_t Init();
    int32_t Shutdown();
    
    int32_t Insert(string key_info);
    int32_t Erase(string key_info);

    int32_t Find(string key_info, StreamTransferClient **transfer_client);
    int32_t RecycleRecordFiles(list<RecordFile*> &free_file_list, bool force_recycle);

    int32_t Open(string key_info, int flags);
};

#endif
