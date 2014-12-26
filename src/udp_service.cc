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

namespace storage
{

UDP_SERVICE::UDP_SERVICE(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in &send_addr)
{
    logger_ = logger;
    memcpy(&recv_addr_, &recv_addr, sizeof(recv_addr_));
    memcpy(&send_addr_, &send_addr, sizeof(send_addr_));

    struct sockaddr_in temp;
    memset(&temp, 0, sizeof(struct sockaddr_in));
    if (memcmp(&send_addr_, &temp, sizeof(struct sockaddr_in)) == 0)
    {
        if_use_recv_addr_to_send = true;
    }
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
    memcpy(&(user_info.send_addr), &(this->send_addr_), sizeof(user_info.send_addr));
    memcpy(&(user_info.recv_addr), &(this->recv_addr_), sizeof(user_info.recv_addr));
    user_info.if_use_recv_addr_to_send = this->if_use_recv_addr_to_send_;
    user_info.parent = (void *)this;

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

