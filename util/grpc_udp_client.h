#ifndef GRPC_UDP_CLIENT_H_
#define GRPC_UDP_CLIENT_H_

#include <stdbool.h>
#include <logger.h>
#include <thread.h>
#include <netinet/in.h>
#include "../grpc/grpc.h"
#include "../grpc/watchdog.h"

namespace util
{

typedef int (*ClientFuncPtr)(grpc_t *grpc);

typedef struct grpc_udp_user_define_info
{
    Logger *logger;
    int fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in from_addr;
    int recv_timeout_milliseconds;
    void *parent;
}GrpcUdpUserDefInfo;

class GrpcUdpClient : public Thread
{
private:
    Logger *logger_;

    int fd_;
    struct sockaddr_in server_addr_;
    grpcMethod_t *method_list_c_;

    ClientFuncPtr client_func_ptr_;
    ClientFuncPtr client_post_func_ptr_;

    int recv_timeout_milliseconds_;

    bool stop_;

public:
    GrpcUdpClient(Logger *logger, struct sockaddr_in &server_addr, grpcMethod_t *method_list_c, ClientFuncPtr func,
                        ClientFuncPtr post_func, int recv_timeout_milliseconds = -1);

    static int RecvByUdp(grpc_t *grpc, void *buffer, int len, int *timeout_milliseconds);
    static int SendByUdp(grpc_t *grpc, void *buffer, int len);

    int32_t Connect();
    int32_t Start();
    void *Entry();
    void Stop();
};

}

#endif 
