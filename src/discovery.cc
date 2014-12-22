#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "discovery.h"
#include "../util/errcode.h"
#include "../grpc/grpc.h"
#include "../grpc/storage_json.h"

int RecvDiscovery(grpc_t *grpc, void *buffer, int len, int *timeout)
{
	int ret = 0;
	struct pollfd pfd;
	UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;
	int discovery_fd = user_info->discovery_fd;
	Logger *logger = user_info->logger;

	Log(logger, "calling poll");
	
	pfd.fd = user_info->discovery_fd;
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
	ret = recvfrom(discovery_fd, buffer, len, 0, (struct sockaddr *)&(user_info->from_addr), (socklen_t *)&addr_len);
	if (ret < 0)
	{
		Log(logger, "recvfrom error - %s", strerror(errno));
		return -errno;
	}

	Log(logger, "recvfrom ret %d", ret);
	return ret;
}

int SendResponse(grpc_t *grpc, void *buffer, int len)
{
	int ret = 0;
	int addr_len = 0;
	int discovery_fd;
	Logger *logger = NULL;
	struct sockaddr_in *send_addr = NULL;
	UserDefInfo *user_info = (UserDefInfo *)grpc->userdef;

	assert(user_info != NULL);
	assert(user_info->discovery_fd > 0);
	assert(user_info->logger != NULL);
	assert(user_info->send_addr != NULL);
	
	logger = user_info->logger;
	discovery_fd = user_info->discovery_fd;
	send_addr = user_info->send_addr;

	Log(logger, "SendResponse start");

	addr_len = sizeof(*send_addr);
	ret = sendto(discovery_fd, buffer, len, 0, (struct sockaddr *)send_addr, addr_len);
	if (ret < 0)
	{
		Log(logger, "sendto error - %s", strerror(errno));
		return -errno;
	}

	return ret;
}

namespace storage
{

Discovery::Discovery(Logger *logger): logger_(logger), done_(false)
{
	memset(&send_addr_, 0, sizeof(struct sockaddr_in));
	send_addr_.sin_family = AF_INET;
	send_addr_.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	send_addr_.sin_port = htons(DISCOVERY_SEND_PORT);
}

int32_t Discovery::Start()
{
	Log(logger_, "start");

	Create();
	
	return 0;
}

int32_t Discovery::Bind()
{
	int ret;
	int optval = 1;
	struct sockaddr_in discovery_addr = {0};

	discovery_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (discovery_fd_ < 0)
	{
		Log(logger_, "create socket error: %s", strerror(errno));
		return -1;
	}

	ret = setsockopt(discovery_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (ret < 0)
	{
		Log(logger_, "setsockopt reuseaddr error: %s", strerror(errno));
		return -1;
	}

	ret = setsockopt(discovery_fd_, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
	if (ret < 0)
	{
		Log(logger_, "setsocket broadcast error: %s", strerror(errno));
		return -1;
	}

	memset(&discovery_addr, 0, sizeof(discovery_addr));
	discovery_addr.sin_family = AF_INET;
	discovery_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	discovery_addr.sin_port = htons(DISCOVERY_RECV_PORT);

	ret = bind(discovery_fd_, (struct sockaddr *)&discovery_addr, sizeof(discovery_addr));
	if (ret < 0)
	{
		Log(logger_, "bind error: %s", strerror(errno));
		return -1;
	}

	return 0;
}


void *Discovery::Entry()
{
	int ret;
	grpcInitParam_t init_param;
	UserDefInfo user_info;

	user_info.discovery_fd = this->discovery_fd_;
	user_info.logger = this->logger_;
	user_info.send_addr = &(this->send_addr_);
	memset(&(user_info.from_addr), 0, sizeof(user_info.from_addr));

	memset(&init_param, 0, sizeof(init_param));
	init_param.userdef = &user_info;
	init_param.fptr_net.recv = RecvDiscovery;
	init_param.fptr_net.send = SendResponse;
	init_param.methodList = storage_json_methodList;
	
	grpc_t *grpc = grpc_new();
	grpc_init(grpc, &init_param);
	grpc_s_account_clear(grpc);

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

	if (discovery_fd_ > 0)
	{
		close(discovery_fd_);
		discovery_fd_ = -1;
	}

	Log(logger_, "discovery entry end");
	return 0;
}

void Discovery::Stop()
{
	done_ = true;

	Log(logger_, "Discovery stop");

	if (discovery_fd_ > 0)
	{
		::close(discovery_fd_);
		discovery_fd_ = -1;
	}

	if (IsStarted())
	{
		Join();
	}

	done_ = false;
}

}

