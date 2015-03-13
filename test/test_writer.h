#ifndef STORAGE_TEST_H_
#define STORAGE_TEST_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../include/storage_api.h"
#include "../util/thread.h"
#include "test_types.h"

using namespace std;
using namespace util;

class FrameWriter : public Thread
{
private:
    uint32_t id_;
    uint64_t seq_;

    char stream_info_[64];

    int32_t op_id_;
    
public:
    FrameWriter(uint32_t id) : id_(id), seq_(0), op_id_(-1) 
    {
        memset(stream_info_, 0, 64);
        snprintf(stream_info_, 63, "stream_info_%03d", id_);
    }

    void Start();

    int FillFrame(char *buffer, int frame_buffer_length);
    int WriteOFrame();
    int WriteFrame();

    void *Entry();
    void Shutdown();
};

#endif 

