#include <errno.h>
#include "grpc_udp_client.h"

namespace util
{

GrpcUdpClient::GrpcUdpClient(Logger *logger, struct sockaddr_in &server_addr, grpcMethod_t *method_list_c, 
ClientFuncPtr func, ClientFuncPtr post_func, int recv_timeout_milliseconds)
: logger_(logger), server_addr_(server_addr), method_list_c_(method_list_c), client_func_ptr_(func), client_post_func_ptr_(post_func),
recv_timeout_milliseconds_(recv_timeout_milliseconds)
{

}

int GrpcUdpClient::RecvByUdp(grpc_t *grpc, void *buffer, int len, int *timeout_milliseconds)
{
    int fd = 0;
    int ret = 0;
    Logger *logger = NULL;
    GrpcUdpUserDefInfo *user_info = NULL;

    LOG_DEBUG(logger, "wait to recv msg from watchdog server");
    user_info = (GrpcUdpUserDefInfo *)grpc->userdef;

    fd = user_info->fd;
    logger = user_info->logger;

    
    socklen_t addr_len = sizeof(struct sockaddr);
    ret = recvfrom(fd, buffer, len, 0, (struct sockaddr *)&user_info->server_addr, &addr_len);

    LOG_INFO(logger, "recvfrom watchdog server ok, ret %d", ret);

    return ret;
}

int GrpcUdpClient::SendByUdp(grpc_t *grpc, void *buffer, int len)
{
    int ret;
    Logger *logger = NULL;

    GrpcUdpUserDefInfo *user_info = (GrpcUdpUserDefInfo *)grpc->userdef;
    assert(user_info != NULL);

    logger = user_info->logger;

    LOG_INFO(logger, "recvfrom watchdog server ok");

    ret = sendto(user_info->fd, buffer, len, 0, (struct sockaddr *)&user_info->server_addr, sizeof(struct sockaddr));

    LOG_INFO(logger, "send to watchdog server, ret %d", ret);

    return ret;
}

int32_t GrpcUdpClient::Connect()
{
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0)
    {
        LOG_INFO(logger_, "create socket error, and error msg %s", strerror(errno));
        return -errno;
    }

    LOG_INFO(logger_, "grpc udp client connect ok");

    return 0;
}

int32_t GrpcUdpClient::Start()
{
    Create();

    return 0;
}

void *GrpcUdpClient::Entry()
{
    int ret;
    GrpcUdpUserDefInfo user_info = {0};

    Connect();

    user_info.fd = fd_;
    user_info.logger = logger_;
    memcpy(&(user_info.server_addr), &server_addr_, sizeof(&server_addr_));
    user_info.recv_timeout_milliseconds = recv_timeout_milliseconds_;
    user_info.parent = (void *)this;

    grpcInitParam_t grpc_init_param;
    memset(&grpc_init_param, 0, sizeof(grpcInitParam_t));
    grpc_init_param.userdef = &user_info;
    grpc_init_param.fptr_net.recv = RecvByUdp;
    grpc_init_param.fptr_net.send = SendByUdp;
    grpc_init_param.methodList_c = method_list_c_;

    grpc_t *grpc = grpc_new();
    grpc_init(grpc, &grpc_init_param);

    grpcUser_t user;
    strcpy(user.name, "abc");
    strcpy(user.passwd, "123");
    grpc_c_account_set(grpc, &user);

    while (!stop_)
    {
        // client function
        ret = (*client_func_ptr_)(grpc);

        grpc_end(grpc);
    }

    // post function
    ret = (*client_post_func_ptr_)(grpc);
    grpc_end(grpc);

    grpc_delete(grpc);

    if (fd_ >= 0)
    {
        close(fd_);
        fd_ = -1;
    }

    return 0;
}

void GrpcUdpClient::Stop()
{
    stop_ = true;

    if (fd_ > 0)
    {
        ::close(fd_);
        fd_ = -1;
    }

    if (IsStarted())
    {
        Join();
    }

    stop_ = false;

    return;
}

}

