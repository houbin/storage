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
#include "test_writer.h"
#include "../util/coding.h"
#include "../util/crc32c.h"

void FrameWriter::Start()
{
    Create();

    return;
}

/*
 * frame buffer format如下所示
 *   |  stream info    |      seq      |    padding                           |   crc   |
 *   |  64字节         |      4字节    |    填充0, 长度为帧长度减去72字节     |   4字节 |
 * */
int FrameWriter::FillFrame(char *buffer, int frame_buffer_length)
{
    int padding_length = 0;
    char *temp = buffer;

    if (frame_buffer_length <= 72)
    {
        assert(frame_buffer_length > 72);
    }

    // zero buffer
    memset(buffer, 0, frame_buffer_length);

    memcpy(temp, stream_info_, 64);
    temp += 64;

    EncodeFixed32(temp, seq_);
    temp += 4;

    padding_length = frame_buffer_length - 72;
    temp += padding_length;

    uint32_t crc_length = frame_buffer_length - 4;
    uint32_t crc = crc32c::Value(buffer, crc_length);
    EncodeFixed32(temp, crc);
    //EncodeFixed32(temp, 0);
    temp += 4;

    return 0;
}

int FrameWriter::WriteOFrame()
{
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

    //fprintf(stderr, "stream %s, seq_ is %d, O frame\n", stream_info_, seq_);
    FillFrame(frame.buffer, 100);

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

        //fprintf(stderr, "stream %s, seq_ is %d, i frame\n", stream_info_, seq_);
    }
    else
    {
        frame.type = JVN_DATA_B;

        int len = rand_number % (30 * 1024) + 200;
        frame.size = len;
        frame.buffer = (char *)malloc(frame.size);
        //fprintf(stderr, "stream %s, seq_ is %d, b or p frame\n", stream_info_, seq_);
    }

    FillFrame(frame.buffer, frame.size);

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
    int32_t temp_op_id = 0;
    
    for (int i = 0; i < 1000000; i++)
    {
        if (op_id_ < 0)
        {
            ret = storage_open(stream_info_, 64, 1, &temp_op_id);
            assert (ret == 0);
            op_id_ = temp_op_id;
            fprintf(stderr, "storage open id %d\n", op_id_);

            WriteOFrame();
            seq_++;

            continue;
        }

        WriteFrame();
        seq_++;

        if (i % 1234 == 1233)
        {
            fprintf(stderr, "storage close write id %d, i is %d\n", op_id_, i);
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
    //fprintf(stderr, "Writer shutdown %d\n", id_);
    
    Join();

    return;
}

