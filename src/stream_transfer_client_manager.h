#ifndef STORAGE_STREAM_TRANSFER_MANAGER_H_
#define STORAGE_STREAM_TRANSFER_MANAGER_H_

#include "../util/logger.h"
#include "../util/mutex.h"
#include "stream_transfer_client.h"

using namespace util;

class StreamTransferClientManager
{
private:
    Logger *logger_;
    Mutex mutex_;

    map<uint32_t, StreamTransferClient*> transfer_clients_;

    bool shutdown_;

public:
    StreamTransferClientManager(Logger *logger);

    int32_t Init();
    int32_t Shutdown();
    
    int32_t Find(uint32_t stream_id, StreamTransferClient *transfer_client);
    int32_t Insert(uint32_t stream_id, StreamTransferClient *transfer_client);
    int32_t Erase(uint32_t stream_id);
};

#endif
