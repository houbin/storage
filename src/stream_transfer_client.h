#ifndef STORAGE_STREAM_TRANSFER_CLIENT_H_
#define STORAGE_STREAM_TRANSFER_CLIENT_H_

#include <stdbool.h>
#include "../util/logger.h"
#include "../util/utime.h"
#include "stream_info.h"
#include "record_file.h"
#include "../include/JVNSDKDef.h"
#include "../include/JvCDefines.h"

namespace storage
{

/* client分为三个状态
 * 1. start。当vms client添加录像配置，client进入start状态。
 * 2. stop。当vms client去掉录像的配置，client进入stop状态，此时还可以查找录像，但是已经不接收录像。
 * 3. shutdown。当对应stream的录像文件已经被回收完毕，则需要把此stream对应的client删掉.
 *
 * */

const static uint32_t start_code = 1234567890;
class StreamTransferClient
{
private:
    Logger *logger;
    Mutex *mutex_;
    StreamInfo stream_info_;

    map<UTime, RecordFile*> record_file_map_;
    bool stop_;

    unsigned char *write_buffer_;
    uint32_t write_offset_;

    unsigned char *header_; // 用于存放到i帧的头部
    int header_size_;

public:
    StreamTransferClient(Logger *logger, StreamInfo &stream_info);

    int32_t GetStreamInfo(StreamInfo **stream_info);

    int32_t Start();
    bool IsStop();
    void Stop(); //只是停止，还能查找录像
    bool ShouldShutdown();
    int32_t Shutdown(); // 关闭

    int32_t Insert(RecordFile *record_file);
    int32_t RecycleExpiredRecordFiles(list<RecordFile*> &expired_file_list, UTime expired_deadline);

    int32_t Store(unsigned char type, unsigned char *buffer, int size, int width, int height);
};

}

#endif
