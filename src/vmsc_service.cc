#include "vmsc_service.h"

namespace storage
{

VmscService::VmscService(Logger *logger, struct sockaddr_in &addr)
: logger_(logger), UDP_SERVICE(logger, addr, NULL)
{

}

int32_t VmscService::AddRecordRequest(StreamInfo &stream_info)
{
    return stream_map_.AddRecordRequest(stream_info);
}

int32_t VmscService::RemoveRecordRequest(StreamInfo &stream_info)
{
    return stream_map_.RemoveRecordRequest(stream_info);
}

}
