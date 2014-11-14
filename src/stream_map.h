#ifndef STORAGE_STREAM_MAP_
#define STORAGE_STREAM_MAP_

#include <stdint.h>

class RecordFile
{
private:
    uint8_t disk_id_; //所属的磁盘id
    uint16_t number_; // 文件编号
    uint64_t firstIFrameTime; //文件中第一个关键帧的时间
    uint64_t endIFrameTime; // 文件中最后一个关键帧的时间

public:
    
};

class Stream
{
private:
    uint16_t stream_id;
    map<uint64_t, RecordFile> record_file_; // 所有视频数据的存放文件

public:
    Stream()
};

class StreamMap
{
private:
    map<uint16_t, 
};
