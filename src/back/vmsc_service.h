#ifndef STORAGE_VMSC_SERVICE_H_
#define STORAGE_VMSC_SERVICE_H_

#include "../util/mutex.h"
#include "udp_service.h"
#include "stream_server_client.h"

namespace storage
{

class VmscService : public UDP_SERVICE
{
private:
    Mutex mutex_;
    uint64_t seq_;
    StreamServerClient *stream_server_client_;

public:
    VmscService(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr, StreamServerClient *stream_server_client);
    
    int32_t EnqueueRecordRequest(StreamInfo *stream_info);
};
}

#endif
