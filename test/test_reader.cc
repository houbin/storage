#include "../include/storage_api.h"
#include "test_reader.h"
#include "../util/crc32c.h"
#include "../util/coding.h"
#include "../include/storage.h"

#define ERR_STREAM_INFO_CHECk_FAILED 9000
#define ERR_CRC_CHECK_FAILED 9001

void FrameReader::Start()
{
    Create();

    return;
}

int32_t FrameReader::GenerateListRangeTime(UTIME_T &start, UTIME_T &end)
{
    int min;
    int max;
    struct timeval now;

    /* 生成过去2小时的一段时间 */
    int a = rand() % 1800;
    int b = rand() % 1800;

    if (a > b)
    {
        min = b;
        max = a;
    }
    else
    {
        min = a;
        max = b;
    }

    gettimeofday(&now, NULL);

    start.seconds = now.tv_sec - max;
    start.nseconds = now.tv_usec * 1000;
    end.seconds = now.tv_sec;
    end.nseconds = now.tv_usec * 1000;

    return 0;
}

int32_t FrameReader::CheckFrame(FRAME_INFO_T *frame_info)
{
    int ret = 0;
    uint32_t length = frame_info->size;
    char *buffer = frame_info->buffer;

    uint32_t actual_crc = crc32c::Value(buffer, length - 4);

    ret = memcmp(buffer, stream_info_, 64);
    if (ret != 0)
    {
        return -ERR_STREAM_INFO_CHECk_FAILED;
    }
    buffer += 64;

    int seq = DecodeFixed32(buffer);
    buffer += 4;

    buffer += length - 72;

    uint32_t expect_crc = DecodeFixed32(buffer);

    if (actual_crc != expect_crc)
    {
        fprintf(stderr, "id %d, frame type %d, time of frame %d.%d, seq %d\n", id_, frame_info->type, frame_info->frame_time.seconds,
                frame_info->frame_time.nseconds, seq);
        return -ERR_CRC_CHECK_FAILED;
    }


    return 0;
}

void *FrameReader::Entry()
{
    int i = 0;
    int32_t ret;
    UTIME_T start;
    UTIME_T end;

    srand((int)time(0));

    FRAME_INFO_T frame_info = {0};
    frame_info.buffer = (char *)malloc(kBlockSize);

    for (; i < 1000000; i++)
    {
        frame_info.type = 0;
        frame_info.frame_time.seconds = 0;
        frame_info.frame_time.nseconds = 0;
        frame_info.stamp = 0;
        frame_info.size = 0;
        memset(frame_info.buffer, 0, kBlockSize);

        if (op_id_ < 0)
        {
            int32_t temp;
            ret = storage_open(stream_info_, 64, 0, &temp);
            if (ret == -ERR_ITEM_NOT_FOUND)
            {
                sleep(10);
                continue;
            }

            op_id_ = temp;
        }

        GenerateListRangeTime(start, end);
        FRAGMENT_INFO_T *frag_buffer = NULL;
        uint32_t count = 0;
        fprintf(stderr, "list record frag from %d.%d to %d.%d\n", start.seconds, start.nseconds, end.seconds, end.nseconds);
        ret = storage_list_record_fragments(op_id_, &start, &end, &frag_buffer, &count);
        if (ret != 0 || count == 0)
        {
            fprintf(stderr, "[list frag]: error, ret is %d, count is %d\n", ret, count);
            sleep(10);
            fprintf(stderr, "\n\n");
            continue;
        }

        fprintf(stderr, "[list frag]: ok, count is %d\n", count);

        for (int seq  = 0; seq < count; seq++)
        {
            FRAGMENT_INFO_T temp = frag_buffer[seq];

            fprintf(stderr, "    fragment %d, frag start time %d.%d, frag end time %d.%d\n",
            seq, temp.start_time.seconds, temp.start_time.nseconds, temp.end_time.seconds, temp.end_time.nseconds);
        }

        /* pick one frag */
        int rand_number = rand() % count;
        FRAGMENT_INFO_T rand_frag = frag_buffer[rand_number];

        int read_rand_offset = rand() % (rand_frag.end_time.seconds - rand_frag.start_time.seconds);

        UTIME_T read_start_time;
        read_start_time.seconds = rand_frag.start_time.seconds + read_rand_offset;
        read_start_time.nseconds = rand_frag.start_time.nseconds;

        ret = storage_seek(op_id_, &read_start_time);
        if (ret != 0)
        {
            fprintf(stderr, "[seek]: error, seek time %d.%d, ret is %d\n", read_start_time.seconds, read_start_time.nseconds, ret);
            
            goto FreeResource;
        }

        fprintf(stderr, "[seek]: ok, frag start is %d.%d, seek is %d.%d, seek ret is %d\n", 
            rand_frag.start_time.seconds, rand_frag.start_time.nseconds, read_start_time.seconds, read_start_time.nseconds, ret);

        // read 200 frame
        for (int j = 0; j < 200; j++)
        {
            ret = storage_read(op_id_, &frame_info);
            if (ret != 0)
            {
                fprintf(stderr, "[read], error, read ret is %d\n", ret);
                
                goto FreeResource;
            }

            /* check frame */
            ret = CheckFrame(&frame_info);
            if (ret != 0)
            {
                fprintf(stderr, "check frame error, ret is %d\n", ret);

                goto FreeResource;
            }
        }

FreeResource:
        if (i % 1234 == 1233)
        {
            storage_close(op_id_);
            op_id_ = -1;
            sleep(10);
        }
        fprintf(stderr, "\n\n");
        sleep(10);
        continue;
    }
}

void FrameReader::Shutdown()
{
    fprintf(stderr, "Reader shutdown %d\n", id_);

    Join();
    return;
}

