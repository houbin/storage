#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

#include <string>
#include "stream_map.h"

namespace storage
{
#define BUFFER_COUNT_LIMIT 256
#define RECORD_FILE_SIZE (BUFFER_COUNT_LIMIT * BUFFER_LENGTH)

using std::string;
class RecordFile
{
private:
    string mount_point_;
    uint16_t number_; // 文件编号
    uint64_t firstIFrameTime_; //文件中第一个关键帧的时间
    uint64_t endIFrameTime_; // 文件中最后一个关键帧的时间
public:
    
};

}
#endif
