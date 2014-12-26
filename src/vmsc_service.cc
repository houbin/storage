#include "vmsc_service.h"

namespace storage
{

VmscService::VmscService(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr, StreamManager *stream_manager)
: UDP_SERVICE(logger, recv_addr, send_addr), stream_manager_(stream_manager)
{

}

int32_t VmscService::EnqueueRecordRequest(StreamInfo &stream_info)
{
    return stream_manager_->EnqueueRecordRequest(stream_info);
}

}
