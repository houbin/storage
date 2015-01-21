#ifndef STORAGE_STREAM_OP_H_
#define STORAGE_STREAM_OP_H_

#include "stream_info.h"

namespace storage
{

enum StreamOpType{
    STREAM_OP_ADD = 0,
    STREAM_OP_DEL = 1
};

class StreamOp
{
private:
    uint64_t op_seq_;
    StreamInfo *stream_info_;
    StreamOpType stream_op_type_;
    
    // no copying allowed
    StreamOp(const StreamOp&);
    void operator=(const StreamOp&);
public:
    StreamOP(uint64_t op_seq_, StreamInfo *stream_info, StreamOpType stream_op_type)
    : op_seq_(op_seq), stream_info_(stream_info), stream_op_type_(stream_op_type)
    {
    }
};

}

#endif
