#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "grpc_userdef.h"

namespace storage
{

int RecvUdp(grpc_t *grpc, void *buffer, int len, int *timeout)
{
    int fd = 0;
    int ret = 0;
    struct pollfd pfd;
    Logger *logger = NULL;
    bool if_use_recv_addr_to_send = false;

    UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;
    fd = user_info->fd;
    logger = user_info->logger;
    if_use_recv_addr_to_send = user_info->if_use_recv_addr_to_send;

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

    int addr_len = sizeof(user_info->recv_addr);
    ret = recvfrom(fd, buffer, len, 0, (struct sockaddr *)&(user_info->recv_addr), (socklen_t *)&addr_len);
    if (ret < 0)
    {
        Log(logger, "recvfrom error - %s", strerror(errno));
        return -errno;
    }

    if (if_use_recv_addr_to_send)
    {
        memcpy(&(user_info->send_addr), &(user_info->recv_addr), addr_len);
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
    UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;

    assert(user_info != NULL);
    assert(user_info->fd > 0);
    assert(user_info->logger != NULL);
    
    fd = user_info->fd;
    logger = user_info->logger;

    Log(logger, "SendResponse start");

    addr_len = sizeof(user_info->send_addr);
    ret = sendto(fd, buffer, len, 0, (struct sockaddr *)&(user_info->send_addr), addr_len);
    if (ret < 0)
    {
        Log(logger, "sendto error - %s", strerror(errno));
        return -errno;
    }

    return ret;
}

}

