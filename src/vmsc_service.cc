#include "vmsc_service.h"

namespace storage
{

VmscService::VmscService(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr, StreamManager *stream_manager)
: logger_(logger), UDP_SERVICE(logger, recv_addr, send_addr, manager), 
{

}

int32_t VmscService::EnqueueRecordRequest(StreamInfo &stream_info)
{
    return stream_map_.EnqueueRecordRequest(stream_info);
}

}
