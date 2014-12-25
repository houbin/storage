#ifndef STORAGE_STREAM_MANAGER_H_
#define STORAGE_STREAM_MANAGER_H_

#include <stdint.h>
#include <map>
#include <set>
#include "stream.h"
#include "stream_addr.h"

namespace storage {

class StreamInfoSet
{
private:
    Mutex mutex_;
    Logger *logger_;
    set<StreamInfo> record_requests_;

public:
    StreamInfoSet(Logger *logger);

    int32_t Add(StreamInfo *info);
    int32_t Remove(StreamInfo *info);
};

class StreamInfoMap
{
private:
    Mutex Mutex_;

    /* stream info -> stream id */
    map<StreamInfo, uint32_t> info_map_; 

public:
    int32_t Add(StreamInfo *info, uint32_t stream_id);
    int32_t Remove(StreamInfo *info);
};

#define BUFFER_LENGTH (1 * 1024 * 1024)
class StreamManager
{
private:
    Logger *logger_;
    Mutex mutex_;

    Mutex 
    StreamInfoSet unrecorded_streams;
    StreamInfoMap recording_streams;

public:
    StreamManager(Logger *logger) : logger_(logger), mutex_("StreamManager::lock") { }

    int32_t AddRecordRequest(StreamInfo *stream_info);

    int32_t Remove(uint16_t stream_id);
    int32_t Find(uint16_t stream_id);
    int32_t IsStreamExist(uint16_t stream_id);
};

}

#endif
