#ifndef STORAGE_STREAM_ID_MANAGER_H_
#define STORAGE_STREAM_ID_MANAGER_H_

#include "../util/mutex.h"
#include "../util/bitmapper.h"
#include "../util/logger.h"

using namespace util;

namespace storage
{

class StreamIdManager
{
private:
    Logger *logger_;
    Mutex lock_;
    bitmapper bitmap_;

public:
    StreamIdManager(Logger *logger, uint32_t max_stream_numbers)
    : logger_(logger), lock_("StreamIdManager::lock"), bitmap_((max_stream_numbers / 32) * 4)
    {

    }

    int32_t ApplyForFreeId();
    int32_t ReleaseId(uint32_t id);
};

}

#endif
