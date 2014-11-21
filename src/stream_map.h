#ifndef STORAGE_STREAM_MAP_
#define STORAGE_STREAM_MAP_

#include <stdint.h>
#include "stream.h"

namespace storage {

#define BUFFER_LENGTH (1 * 1024 * 1024)
class StreamMap
{
private:
    Mutex mutex_;
    map<StreamAddr, Stream> streams_map_; // stream addr -> stream
    Logger *logger_;

public:
    StreamMap(Logger *logger) : next_index(0), logger_(logger) { }

    int32_t Open(StreamAddr *addr, bool createIfMissing, Stream *stream);

    int32_t AddStream(uint16_t* stream_id);
    int32_t DelStream(uint16_t stream_id);
    int32_t FindStream(uint16_t stream_id);
    int32_t IsStreamExist(uint16_t stream_id);
};
}

#endif
