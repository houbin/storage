#ifndef STORAGE_STREAM_TRANSFER_CLIENT_H_
#define STORAGE_STREAM_TRANSFER_CLIENT_H_

#include <stdbool.h>
#include "../util/logger.h"
#include "../util/utime.h"
#include "stream_info.h"
#include "record_file.h"

class StreamTransferClient
{
private:
    Logger *logger;
    Mutex *mutex_;
    StreamInfo stream_info_;

    map<UTime, RecordFile*> record_file_map_;

public:
    StreamTransferClient(Logger *logger, StreamInfo &stream_info);

    int32_t Start();
    void Stop();
    void Wait();

    int32_t Insert(RecordFile *record_file);
};

#endif
