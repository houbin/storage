#include <assert.h>
#include "vmsc_service.h"
#include "stream_op.h"

namespace storage
{

VmscService::VmscService(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr, StreamServerClient *stream_server_client)
: mutex_("VmsService::Lock"), seq_(0), UDP_SERVICE(logger, recv_addr, send_addr), stream_server_client_(stream_server_client)
{

}

int32_t VmscService::EnqueueRecordRequest(StreamInfo *stream_info)
{
    assert(stream_info != NULL);

    Mutex::Locker locker(mutex_);

    seq_++;
    StreamOp *stream_op = new StreamOp(seq_, stream_info, STREAM_OP_ADD);

    return stream_server_client_->EnqueueStreamOp(stream_op);
}

}
