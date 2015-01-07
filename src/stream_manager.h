#ifndef STORAGE_STREAM_MANAGER_H_
#define STORAGE_STREAM_MANAGER_H_

#include <stdint.h>
#include <map>
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/thread.h"
#include "stream.h"
#include "stream_info.h"
#include "stream_id_manager.h"
#include "../grpc/storage_json.h"

using namespace util;

namespace storage {

#define BUFFER_LENGTH (1 * 1024 * 1024)
class StreamManager
{
private:
    Logger *logger_;

    Mutex_ mutex;
    Cond cond_;
    
    map<StreamInfo, uint64_t> stream_infos_;
    map<uint64_t, StreamTransferClient> stream_transfer_clients_;

    StreamIdManager stream_id_manager_;
    
    bool stop_;

public:
    StreamManager(Logger *logger, uint32_t max_stream_numbers);

    void Start();
    void *Entry();
    void Shutdown();
};

}

#endif

