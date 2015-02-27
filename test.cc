#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "util/utime.h"

using namespace std;
using namespace util;

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

int main()
{
    int record_file_info_len = sizeof(struct RecordFileInfo);
    int record_frag_info_len = sizeof(struct RecordFragmentInfo);

    printf("file info len is %d\n", record_file_info_len);
    printf("frag info len is %d\n", record_frag_info_len);
    
    return 0;
}
