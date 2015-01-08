#ifndef STORAGE_STREAM_TRANSFER_CLIENT_H_
#define STORAGE_STREAM_TRANSFER_CLIENT_H_

#include <stdbool.h>
#include "../util/logger.h"
#include "stream_info.h"

class StreamTransferClient
{
private:
    Logger *logger;
    StreamInfo stream_info_;

public:
    StreamTransferClient(Logger *logger, StreamInfo &stream_info);

    int32_t Start();
    void Stop();
    void Wait();
};

#endif
