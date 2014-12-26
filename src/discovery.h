#ifndef STORAGE_DISCOVERY_H_
#define STORAGE_DISCOVERY_H_

#include "udp_service.h"

using namespace util;

namespace storage
{
#define DISCOVERY_RECV_PORT 9001
#define DISCOVERY_SEND_PORT 9002

class Discovery : public UDP_SERVICE
{
public:
    Discovery(Logger *logger, struct sockaddr_in &in_addr, struct sockaddr_in &out_addr)
    : UDP_SERVICE(logger, in_addr, out_addr) { }
};

}


#endif
