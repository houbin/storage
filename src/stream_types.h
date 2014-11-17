#ifndef STORAGE_STREAM_TYPES_H_
#define STORAGE_STREAM_TYPES_H_

namespace storage
{

struct Frame
{
    uint32_t start_code;
    uint32_t type;
    uint32_t width;
    uint32_t height;
    uint64_t timestamp;
    char *data;
    uint32_t size;
};


}

#endif
