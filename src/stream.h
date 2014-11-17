#ifndef STORAGE_STREAM_H_
#define STORAGE_STREAM_H_

#include "stream_types.h"
#include "stream_addr.h"

namespace storage
{

class Stream
{
private:
    Mutex mutex_;
    StreamAddr addr_;
    char *write_buffer_; // 1M写缓冲区
    char *read_buffer_;  // 1M读缓冲区
    map<uint64_t, RecordFile> record_file_; // 以时间为key，对数据文件进行排序

    // no copying
    void operator=(Stream &);
public:
    Stream() : addr_(0), write_buffer_(NULL), read_buffer_(NULL) { }
    Stream(const Stream &stream)
    {
        addr_.Copy(stream.addr_);
        write_buffer = stream.write_buffer;
        read_buffer = stream.read_buffer;
    }

    void CopyAddr(const StreamAddr &addr)
    {
        addr_.Copy(addr);
        return;
    }

    int32_t Write(Frame frame);

};


}
#endif
