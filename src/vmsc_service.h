#ifndef STORAGE_VMSC_SERVICE_H_
#define STORAGE_VMSC_SERVICE_H_

#include "udp_service.h"
#include "stream_manager.h"

namespace storage
{

class VmscService : public UDP_SERVICE
{
private:
    StreamManager *stream_manager_;

public:
    VmscService(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr, StreamManager *stream_manager);
    
    int32_t EnqueueRecordRequest(StreamInfo &stream_info);
};
}

#endif
