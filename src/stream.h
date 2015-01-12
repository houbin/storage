#ifndef STORAGE_STREAM_H_
#define STORAGE_STREAM_H_

#include <map>
#include <set>
#include "stream_types.h"
#include "stream_info.h"
#include "record_file.h"

using namespace std;
namespace storage
{

class Stream
{
private:
    Mutex mutex_;

    uint16_t stream_id_;

    uint16_t write_offset_;
    char *write_buffer_; // 写缓冲区

    uint16_t file_write_count_;
    map<uint64_t, RecordFile> record_file_; // 以时间为key，对数据文件进行排序

    // no copying
    void operator=(Stream &);
public:
    Stream() : mutex_("Stream"), stream_id_(0), write_offset_(0), write_buffer_(NULL), read_offset_(0), read_buffer_(NULL) { }

    int32_t Write(Frame frame);
};

}
#endif

