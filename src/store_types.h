#ifndef STORAGE_STREAM_TYPES_H_
#define STORAGE_STREAM_TYPES_H_

#include "stdlib.h"
#include "../include/storage_api.h"
#include "../util/utime.h"

using namespace std;
using namespace util;

#define safe_free(ptr) do{if(ptr){free(ptr);ptr = NULL;}}while(0)

namespace storage
{

typedef struct write_op_
{
    FRAME_INFO_T *frame_info;
}WriteOp;

typedef struct buffer_times
{
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
}BufferTimes;

#pragma pack(1)

struct RecordFileInfo
{
    uint32_t length;
    uint32_t crc;
    char stream_info[64];
    bool locked;
    uint16_t record_fragment_counts;
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
    uint32_t record_offset;
    char padding[17];
}; 

struct RecordFragmentInfo
{
    uint32_t length;
    uint32_t crc;
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;

    uint32_t start_offset;
    uint32_t end_offset;
};
#pragma pack()

inline bool operator>(const UTIME_T &x, const UTIME_T &y)
{
    return ((x.seconds > y.seconds) || ((x.seconds == y.seconds) && (x.nseconds > y.nseconds)));
}

inline bool operator<=(const UTIME_T &x, const UTIME_T &y)
{
    return !(operator>(x, y));
}

inline bool operator<(const UTIME_T &x, const UTIME_T &y)
{
    return ((x.seconds < y.seconds) || ((x.seconds == y.seconds) && (x.nseconds < y.seconds)));
}

inline bool operator>=(const UTIME_T &x, const UTIME_T &y)
{
    return !(operator<(x,y));
}

inline bool operator==(const UTIME_T &x, const UTIME_T &y)
{
    return ((x.seconds == y.seconds) && (x.nseconds == y.nseconds));
}

inline bool operator!=(const UTIME_T &x, const UTIME_T &y)
{
    return !(operator==(x, y));
}


inline bool operator>(const UTIME_T &x, const UTime &y)
{
    return ((x.seconds > y.tv_sec) || ((x.seconds == y.tv_sec) && (x.nseconds > y.tv_nsec)));
}

inline bool operator<=(const UTIME_T &x, const UTime &y)
{
    return !(operator>(x, y));
}

inline bool operator<(const UTIME_T &x, const UTime &y)
{
    return ((x.seconds < y.tv_sec) || ((x.seconds == y.tv_sec) && (x.nseconds < y.tv_nsec)));
}

inline bool operator>=(const UTIME_T &x, const UTime &y)
{
    return !(operator<(x,y));
}

inline bool operator==(const UTIME_T &x, const UTime &y)
{
    return ((x.seconds == y.tv_sec) && (x.nseconds == y.tv_nsec));
}

inline bool operator!=(const UTIME_T &x, const UTime &y)
{
    return !(operator==(x, y));
}

#define JVN_DATA_I           0x01
#define JVN_DATA_B           0x02
#define JVN_DATA_P           0x03
#define JVN_DATA_A           0x04
#define JVN_DATA_S           0x05
#define JVN_DATA_OK          0x06
#define JVN_DATA_DANDP       0x07
#define JVN_DATA_O           0x08
#define JVN_DATA_SKIP        0x09
#define JVN_DATA_SPEED		 0x64
#define JVN_DATA_HEAD        0x66

}

#endif

