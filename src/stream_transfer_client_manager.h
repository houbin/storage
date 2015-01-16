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

    map<uint32_t, StreamTransferClient*> transfer_clients_;
    map<StreamInfo, uint32_t> stream_id_map_;

    uint32_t next_apply_stream_id_;
    uint32_t active_client_counts_;

    bool shutdown_;

    SafeTimer timer;

public:
    StreamTransferClientManager(Logger *logger);

    int32_t Init();
    int32_t Shutdown();
    
    int32_t Insert(StreamInfo *stream_info);
    int32_t Erase(StreamInfo *stream_info);

    int32_t Find(uint32_t stream_id, StreamTransferClient **transfer_client);
    int32_t RecycleRecordFiles(list<RecordFile*> &free_file_list, bool force_recycle);
};

#endif
