#ifndef STORAGE_STREAM_TYPES_H_
#define STORAGE_STREAM_TYPES_H_

#include "../include/storage_api.h"
#include "../util/utime.h"

using namespace std;
using namespace util;

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
/* 这里定义的是硬盘上index file中的组织方式 */

/* 录像文件信息 */
struct RecordFileInfo
{
    uint32_t length;
    uint32_t crc;
    char stream_info[64];
    bool locked;
    char state_;
    uint16_t record_fragment_counts;
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
    uint32_t record_offset;
    char padding[16];
}; 

/* 录像片段信息 */
struct RecordFragmentInfo
{
    uint32_t length;
    uint32_t crc;
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
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

#define JVN_DATA_I           0x01//视频I帧
#define JVN_DATA_B           0x02//视频B帧
#define JVN_DATA_P           0x03//视频P帧
#define JVN_DATA_A           0x04//音频
#define JVN_DATA_S           0x05//帧尺寸
#define JVN_DATA_OK          0x06//视频帧收到确认
#define JVN_DATA_DANDP       0x07//下载或回放收到确认
#define JVN_DATA_O           0x08//其他自定义数据
#define JVN_DATA_SKIP        0x09//视频S帧
#define JVN_DATA_SPEED		 0x64//主控码率
#define JVN_DATA_HEAD        0x66//视频解码头，该数据出现的同时将清空缓存

}

#endif
