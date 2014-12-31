#ifndef STORAGE_GRPC_USERDEF_H_
#define STORAGE_GRPC_USERDEF_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../util/logger.h"
#include "../grpc/grpc.h"

using namespace util;
namespace storage
{

typedef struct user_define_info
{
    Logger *logger;

    int fd; /* udp socket fd */
    struct sockaddr_in send_addr;
    struct sockaddr_in recv_addr;

    bool if_use_recv_addr_to_send;

    void *parent;
}UserDefInfo;

int RecvUdp(grpc_t *grpc, void *buffer, int len, int timeout);
int SendUdp(grpc_t *grpc, void *buffer, int len);

}

#endif
