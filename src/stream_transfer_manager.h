#ifndef STORAGE_STREAM_TRANSFER_MANAGER_H_
#define STORAGE_STREAM_TRANSFER_MANAGER_H_

#include "../util/logger.h"
#include "../util/mutex.h"
#include "stream_transfer_client.h"

using namespace util;

class StreamTransferManager
{
private:
    Logger *logger_;
    Mutex mutex_;

    map<uint64_t, StreamTransferClient*> stream_transfer_clients_;

public:
    StreamTransferManager(Logger *logger);
    
    Find(uint64_t stream_id);
    Insert(uint64_t stream_id, StreamTransferClient *transfer_client);
    Erase(uint64_t stream_id);
};


#endif
