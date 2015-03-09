#ifndef STORAGE_TEST_H_
#define STORAGE_TEST_H_

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

#include <stdint.h>
#include "../include/storage_api.h"
#include "../util/thread.h"

using namespace std;
using namespace util;

class FrameWriter : public Thread
{
private:
    uint32_t id_;
    uint64_t seq_;

    int32_t op_id_;
    
public:
    FrameWriter(uint32_t id) : id_(id), seq_(0), op_id_(-1) {}

    void Start();
    int MakeFrame(int *type, int *length, FRAME_INFO_T *buffer);

    void *Entry();
    void Shutdown();
};


#endif 
