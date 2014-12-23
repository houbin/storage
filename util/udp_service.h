#ifndef STORAGE_UDP_SERVICE_H_
#define STORAGE_UDP_SERVICE_H_

#include <stdbool.h>
#include <netinet/in.h>
#include "thread.h"
#include "logger.h"

using namespace util;

typedef struct user_def_info
{
    int fd;
    Logger *logger;
    struct sockaddr_in *send_addr;
    struct sockaddr_in from_addr;
    bool if_broadcast;
} UserDefInfo;

namespace util
{

class UDP_SERVICE : public Thread
{
private:
    int fd_;
    Logger *logger_;
    struct sockaddr_in recv_addr_;
    struct sockaddr_in *send_addr_;
    bool if_broadcast_;
    bool done_;

public:
    UDP_SERVICE(Logger *logger, struct sockaddr_in &recv_addr, struct sockaddr_in *send_addr);

    int32_t Bind();
    int32_t Start();
    void *Entry();
    void Stop();
};

}

#endif

