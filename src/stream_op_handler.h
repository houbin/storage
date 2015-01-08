#ifndef STORAGE_STREAM_OP_HANDLER_H_
#define STORAGE_STREAM_OP_HANDLER_H_

#include <stdint.h>
#include <map>
#include <deque>
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/thread.h"
#include "stream.h"
#include "stream_info.h"
#include "stream_id_manager.h"
#include "../grpc/storage_json.h"
#include "stream_op.h"

using namespace util;

namespace storage {

#define BUFFER_LENGTH (1 * 1024 * 1024)
#define MAX_STREAM_NUMBERS 512

class StreamOpHandler: public Thread
{
private:
    Logger *logger_;

    Mutex_ mutex_;
    Cond cond_;
    
    deque<StreamOp> stream_op_queue_;
    map<StreamInfo, uint64_t> stream_infos_;
    map<uint64_t, StreamTransferClient*> stream_transfer_clients_;

    StreamIdManager stream_id_manager_;
    
    bool stop_;

public:
    StreamOpHandler(Logger *logger);

    int32_t EnqueueStreamOp(StreamOp *stream_op);
    StreamOp DequeStreamOp();

    int32_t HandleStreamAdd(StreamInfo *stream_info);
    int32_t HandleStreamDel(StreamInfo *stream_info);

    void Init();
    void Start();
    void *Entry();
    void Stop();
};

}

#endif

