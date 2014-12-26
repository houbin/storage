#ifndef STORAGE_GRPC_USERDEF_H_
#define STORAGE_GRPC_USERDEF_H_

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

extern int RecvUdp(grpc_t *grpc, void *buffer, int len, int *timeout);
extern int SendUdp(grpc_t *grpc, void *buffer, int len);

}

#endif
