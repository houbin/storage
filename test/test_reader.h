#ifndef TEST_READER_H_
#define TEST_READER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../include/storage_api.h"
#include "../util/thread.h"

using namespace util;
using namespace std;

class FrameReader : public Thread
{
private:
    uint32_t id_;

    char stream_info_[64];

    int32_t op_id_;

public:
    FrameReader(uint32_t id) : id_(id), op_id_(-1)
    {
        memset(stream_info_, 0, 64);
        snprintf(stream_info_, 63, "stream_info_%03d", id_);
    }

    void Start();

    int32_t GenerateListRangeTime(UTIME_T &start, UTIME_T &end);
    int32_t CheckFrame(FRAME_INFO_T *frame);
    int32_t PareFrame();
    int32_t ReadFrame();

    void *Entry();
    void Shutdown();
};

#endif
