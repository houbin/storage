#ifndef STORAGE_VMSC_SERVICE_H_
#define STORAGE_VMSC_SERVICE_H_

#include "../util/udp_service.h"
#include "stream_manager.h"

namespace storage
{

class VmscService : public UDP_SERVICE
{
private:
    StreamManager *stream_manager_;

public:
    VmscService(Logger *logger, struct sockaddr_in &in_addr, StreamManager *stream_manager);
    
    int32_t AddRecordRequest(StreamInfo &stream_info);
    int32_t RemoveRecordRequest(StreamInfo &stream_info);
};
}

#endif
