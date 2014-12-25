#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "udp_service.h"
#include "../grpc/grpc.h"
#include "../grpc/storage_json.h"

int RecvUdp(grpc_t *grpc, void *buffer, int len, int *timeout)
{
    int fd = 0;
    int ret = 0;
    struct pollfd pfd;
    Logger *logger = NULL;
    bool if_broadcast = false;

    UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;
    fd = user_info->fd;
    logger = user_info->logger;
    if_broadcast = user_info->if_broadcast;

    Log(logger, "calling poll");
    
    pfd.fd = fd;
    pfd.events = POLLIN | POLLERR | POLLNVAL | POLLHUP;
    ret = poll(&pfd, 1, -1);
    if (ret < 0)
    {
        Log(logger, "poll error - %s", strerror(errno));
        return -errno;
    }

    Log(logger, "poll got %d, pfd.revents is %d", pfd.revents);

    if (pfd.revents & (POLLERR | POLLNVAL | POLLHUP))
    {
        Log(logger, "pdf.revents is POLLERR or POLLNVAL or POLLHUP");
        return -1;
    }

    int addr_len = sizeof(user_info->from_addr);
    ret = recvfrom(fd, buffer, len, 0, (struct sockaddr *)&(user_info->from_addr), (socklen_t *)&addr_len);
    if (ret < 0)
    {
        Log(logger, "recvfrom error - %s", strerror(errno));
        return -errno;
    }

    if (!if_broadcast)
    {
        user_info->send_addr = &(user_info->from_addr);
        assert(user_info->send_addr != NULL);
    }

    Log(logger, "recvfrom ret %d", ret);
    return ret;
}

int SendUdp(grpc_t *grpc, void *buffer, int len)
{
    int ret = 0;
    int fd = 0;
    int addr_len = 0;
    Logger *logger = NULL;
    struct sockaddr_in *send_addr = NULL;
    UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;

    assert(user_info != NULL);
    assert(user_info->fd > 0);
    assert(user_info->logger != NULL);
    assert(user_info->send_addr != NULL);
    
    fd = user_info->fd;
    logger = user_info->logger;
    send_addr = user_info->send_addr;

    Log(logger, "SendResponse start");

    addr_len = sizeof(*send_addr);
    ret = sendto(fd, buffer, len, 0, (struct sockaddr *)send_addr, addr_len);
    if (ret < 0)
    {
        Log(logger, "sendto error - %s", strerror(errno));
        return -errno;
    }

    return ret;
}


namespace util
{

UDP_SERVICE::UDP_SERVICE(Logger *logger, struct sockaddr_in &in_addr, struct sockaddr_in *out_addr)
: if_broadcast_(false), done_(false)
{
    assert(logger != NULL);
    logger_ = logger;
    
    memcpy(&recv_addr_, &in_addr, sizeof(struct sockaddr_in));
    send_addr_ = out_addr;

    if ((send_addr_ != NULL) 
        && (send_addr_->sin_addr.s_addr == htonl(INADDR_BROADCAST)))
    {
        if_broadcast_ = true;
        assert(send_addr_ != NULL);
    }
}

int32_t UDP_SERVICE::Bind()
{
    int optval = 1;
    int32_t ret = 0;;

    Log(logger_, "bind");

    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0)
    {
        Log(logger_, "create socket error: %s", strerror(errno));
        return -errno;
    }

    ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret < 0)
    {
        Log(logger_, "setsockopt reuseaddr error: %s", strerror(errno));
        return -errno;
    }

    if (if_broadcast_)
    {
        ret = setsockopt(fd_, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
        if (ret < 0)
        {
            Log(logger_, "setsockopt broadcast error: %s", strerror(errno));
            return -errno;
        }
    }

    ret = bind(fd_, (struct sockaddr *)&recv_addr_, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        Log(logger_, "bind error: %s", strerror(errno));
        return -errno;
    }

    return 0;
}

int32_t UDP_SERVICE::Start()
{
    Log(logger_, "start");

    Create();

    return 0;
}


void *UDP_SERVICE::Entry()
{
    int ret;
    grpcInitParam_t init_param;
    UserDefInfo user_info;

    user_info.fd = this->fd_;
    user_info.logger = this->logger_;
    user_info.send_addr = this->send_addr_;
    user_info.if_broadcast = this->if_broadcast_;
    memset(&(user_info.from_addr), 0, sizeof(user_info.from_addr));
    user_info.udp_service = this;

    memset(&init_param, 0, sizeof(init_param));
    init_param.userdef = &user_info;
    init_param.fptr_net.recv = RecvUdp;
    init_param.fptr_net.send = SendUdp;
    init_param.methodList = storage_json_methodList;
    
    grpc_t *grpc = grpc_new();
    grpc_init(grpc, &init_param);
    grpc_s_account_clear(grpc);

    Log(logger_, "entry start");

    // TODO add account manage
    
    while(!done_)
    {
        Log(logger_, "grpc_s_serve start");

        ret = grpc_s_serve(grpc);
        if (ret < 0)
        {
            Log(logger_, "grpc_s_serve return error, ret is %d", ret);
            break;
        }

        Log(logger_, "grpc_s_serve end");
        grpc_end(grpc);
    }

    if (fd_ > 0)
    {
        close(fd_);
        fd_ = -1;
    }

    Log(logger_, "entry end");
    return 0;
}

void UDP_SERVICE::Stop()
{
    done_ = true;

    Log(logger_, "stop");

    if (fd_ > 0)
    {
        ::close(fd_);
        fd_ = -1;
    }

    if (IsStarted())
    {
        Join();
    }

    done_ = false;
}

}

