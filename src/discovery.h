#ifndef STORAGE_DISCOVERY_H_
#define STORAGE_DISCOVERY_H_

#include <assert.h>
#include <netinet/in.h>
#include "../util/logger.h"
#include "../util/thread.h"
#include "../grpc/grpc.h"

using namespace util;

typedef struct user_def_info
{
	int discovery_fd;
	Logger *logger;
	struct sockaddr_in *send_addr;
	struct sockaddr_in from_addr;
} UserDefInfo;

namespace storage
{
#define DISCOVERY_RECV_PORT 9001
#define DISCOVERY_SEND_PORT 9002

class Discovery : public Thread
{
private:
	Logger *logger_;
	int discovery_fd_;
	struct sockaddr_in send_addr_;
	bool done_;
	
public:
	Discovery(Logger *logger);
	int32_t Start();
	int32_t Bind();
	void *Entry();
	void Stop();
};

}


#endif
