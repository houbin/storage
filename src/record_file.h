#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

namespace storage
{

class RecordFile
{
private:
    uint8_t disk_id_; //所属的磁盘id
    uint16_t number_; // 文件编号
    uint64_t firstIFrameTime_; //文件中第一个关键帧的时间
    uint64_t endIFrameTime_; // 文件中最后一个关键帧的时间

public:
    
};

}
#endif
