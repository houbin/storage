#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "../include/storage_api.h"

#define JVN_DATA_I           0x01//视频I帧
#define JVN_DATA_B           0x02//视频B帧
#define JVN_DATA_P           0x03//视频P帧
#define JVN_DATA_A           0x04//音频
#define JVN_DATA_S           0x05//帧尺寸
#define JVN_DATA_OK          0x06//视频帧收到确认
#define JVN_DATA_DANDP       0x07//下载或回放收到确认
#define JVN_DATA_O           0x08//其他自定义数据
#define JVN_DATA_SKIP        0x09//视频S帧
#define JVN_DATA_SPEED 0x64//主控码率
#define JVN_DATA_HEAD        0x66//视频频解码头，该数据出现的同时将清空缓存

uint64_t seq = 0;

int make_frame(int *type, int *length, char **buffer)
{
    FRAME_INFO_T *frame = NULL;
    
    frame = (FRAME_INFO_T *)malloc(sizeof(FRAME_INFO_T));
    if (frame == NULL)
    {
        return -ENOMEM;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    frame->frame_time.seconds = now.tv_sec;
    frame->frame_time.nseconds = now.tv_usec * 1000;
    frame->stamp = seq;

    int rand_number = rand();

    if (seq == 0)
    {
        frame->type = JVN_DATA_O;
        frame->size = 100;
        frame->buffer = (char *)malloc(100);
        memset(frame->buffer, 'o', 100);
    }
    else if (seq % 40 == 2)
    {
        frame->type = JVN_DATA_I;
        
        int len = rand_number % (900 * 1024) + 100 * 1024;
        frame->size = len;
        frame->buffer = (char *)malloc(frame->size);
        memset(frame->buffer, 'i', frame->size);
    }
    else if (seq % 4 == 0)
    {
        frame->type = JVN_DATA_B;
        

        int len = rand_number % (90 * 1024) + 10 * 1024;
        frame->size = len;
        frame->buffer = (char *)malloc(frame->size);
        memset(frame->buffer, 'b', frame->size);
    }
    else
    {
        frame->type = JVN_DATA_P;

        int len = rand_number % (80 * 1024) + 10 * 1024;
        frame->size = len;
        frame->buffer = (char *)malloc(frame->size);
        memset(frame->buffer, 'p', frame->size);
    }

    seq++;

    *buffer = (char *)frame;
    return 0;
}

int main()
{
    int ret = 0;
    uint32_t  write_id = 0;
    char stream_info[64] = {0};

    strncpy(stream_info, "stream info aaaa", 64);

    storage_init();
    storage_open(stream_info, 64, 1, &write_id);

    int i = 0;
    for (i = 0; i < 100; i++)
    {
        int type;
        int length;
        char *frame = NULL;

        make_frame(&type, &length, &frame);
        storage_write(write_id, (FRAME_INFO_T*)frame);
        usleep(40000);
    }

    storage_close(write_id);
    storage_shutdown();
    return 0;
}
