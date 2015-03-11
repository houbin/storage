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

int FrameWriter::WriteOFrame()
{
    seq_++;

    char stream_info[64] = {0};
    snprintf(stream_info, 63, "stream_info_id_%03d", id_);

    int32_t ret;
    FRAME_INFO_T frame = {0};
    struct timeval now;

    gettimeofday(&now, NULL);
    frame.frame_time.seconds = now.tv_sec;
    frame.frame_time.nseconds = now.tv_usec * 1000;
    frame.stamp = seq_;
    frame.type = JVN_DATA_O;
    frame.size = 100;
    frame.buffer = (char *)malloc(100);
    memset(frame.buffer, 'o', 100);

    fprintf(stderr, "stream %s, seq_ is %d, O frame\n", stream_info, seq_);

    ret = storage_write(op_id_, &frame);
    if (ret != 0)
    {
        fprintf(stderr, "storage write return %d\n", ret);
        assert(ret == 0);
        return 0; 
    }

    free(frame.buffer);
    frame.buffer = NULL;

    usleep(30 * 1000);

    return 0;
}

int FrameWriter::WriteFrame()
{
    seq_++;

    char stream_info[64] = {0};
    snprintf(stream_info, 63, "stream_info_id_%03d", id_);

    int32_t ret;
    FRAME_INFO_T frame = {0};
    struct timeval now;

    gettimeofday(&now, NULL);
    frame.frame_time.seconds = now.tv_sec;
    frame.frame_time.nseconds = now.tv_usec * 1000;
    frame.stamp = seq_;

    int rand_number = rand();
    if (seq_ % 30 == 1)
    {
        frame.type = JVN_DATA_I;
        
        int len = rand_number % (200 * 1024) + 10 * 1024;
        frame.size = len;
        frame.buffer = (char *)malloc(frame.size);
        memset(frame.buffer, 'i', frame.size);

        fprintf(stderr, "stream %s, seq_ is %d, i frame\n", stream_info, seq_);
    }
    else
    {
        frame.type = JVN_DATA_B;

        int len = rand_number % (30 * 1024) + 30;
        frame.size = len;
        frame.buffer = (char *)malloc(frame.size);
        memset(frame.buffer, 'b', frame.size);

        fprintf(stderr, "stream %s, seq_ is %d, b or p frame\n", stream_info, seq_);
    }

    ret = storage_write(op_id_, &frame);
    if (ret != 0)
    {
        fprintf(stderr, "storage write return %d\n", ret);
        assert(ret == 0);
        return 0; 
    }

    free(frame.buffer);
    frame.buffer = NULL;

    usleep(30 * 1000);

    return 0;
}

void *FrameWriter::Entry()
{
    int ret = 0;
    int type = 0;
    int length = 0;
    uint32_t temp_op_id = 0;
    
    char stream_info[64] = {0};
    snprintf(stream_info, 63, "stream_info_id_%03d", id_);

    for (int i = 0; i < 10000; i++)
    {
        if (op_id_ < 0)
        {
            ret = storage_open(stream_info, 64, 1, &temp_op_id);
            assert (ret == 0);
            op_id_ = (int32_t)temp_op_id;
            fprintf(stderr, "storage open id %d\n", op_id_);

            WriteOFrame();

            continue;
        }

        WriteFrame();

        if (i % 1200 == 1199)
        {
            fprintf(stderr, "storage close id %d\n", op_id_);
            storage_close(op_id_);
            op_id_ = -1;
            sleep(10);
        }
    }

    //test if write index file when not close
    sleep(100);

    if (op_id_ >= 0)
    {
        storage_close(op_id_);
        op_id_ = -1;
    }

    return 0;
}

void FrameWriter::Shutdown()
{
    fprintf(stderr, "shutdown %d\n", id_);
    
    Join();

    return;
}

