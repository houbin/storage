#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include "../include/storage_api.h"
#include "storage_test.h"

void FrameWriter::Start()
{
    Create();

    return;
}

int FrameWriter::MakeFrame(int *type, int *length, FRAME_INFO_T *frame)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    frame->frame_time.seconds = now.tv_sec;
    frame->frame_time.nseconds = now.tv_usec * 1000;
    frame->stamp = seq_;

    int rand_number = rand();

    if (seq_ == 0)
    {
        frame->type = JVN_DATA_O;
        frame->size = 100;
        frame->buffer = (char *)malloc(100);
        memset(frame->buffer, 'o', 100);
    }
    else if (seq_ % 30 == 2)
    {
        frame->type = JVN_DATA_I;
        
        int len = rand_number % (200 * 1024) + 10 * 1024;
        frame->size = len;
        frame->buffer = (char *)malloc(frame->size);
        memset(frame->buffer, 'i', frame->size);
    }
    else
    {
        frame->type = JVN_DATA_B;

        int len = rand_number % (30 * 1024) + 30;
        frame->size = len;
        frame->buffer = (char *)malloc(frame->size);
        memset(frame->buffer, 'b', frame->size);
    }

    seq_++;
    usleep(30 * 1000);

    return 0;
}

void *FrameWriter::Entry()
{
    int ret = 0;
    int type = 0;
    int length = 0;
    uint32_t temp_op_id = 0;
    FRAME_INFO_T frame = {0};
    
    char stream_info[64] = {0};
    snprintf(stream_info, 63, "stream_info_id_%d", id_);

    for (int i = 0; i < 10000; i++)
    {
        memset(&frame, 0, sizeof(FRAME_INFO_T));
        if (op_id_ < 0)
        {
            ret = storage_open(stream_info, 64, 1, &temp_op_id);
            assert (ret == 0);
            op_id_ = (int32_t)temp_op_id;
            fprintf(stderr, "storage open id %d\n", op_id_);
        }

        MakeFrame(&type, &length, &frame);

        ret = storage_write(op_id_, &frame);
        if (ret != 0)
        {
            fprintf(stderr, "storage write return %d\n", ret);
            return 0; 
        }

        if (frame.buffer != NULL)
        {
            free(frame.buffer);
            frame.buffer = NULL;
        }

        if (i % 123 == 122)
        {
            fprintf(stderr, "storage close id %d", op_id_);
            storage_close(op_id_);
            op_id_ = -1;
            sleep(10);
        }
    }

    return 0;
}

void FrameWriter::Shutdown()
{
    fprintf(stderr, "shutdown %d\n", id_);
    
    Join();

    return;
}

