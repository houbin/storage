#ifndef WATCHDOG_CLIENT_H_
#define WATCHDOG_CLIENT_H_

#include <string>
#include "../util/grpc_udp_client.h"

using namespace std;
using namespace util;

namespace storage
{

class WatchDogClient : public GrpcUdpClient
{
public:
    Logger *logger_;

    struct sockaddr_in server_addr_;

    string reboot_path_;
    pid_t pid_;
    int lost_threshold_;

    bool stop_;

public:
    WatchDogClient(Logger *logger, struct sockaddr_in &server_addr, string reboot_path, pid_t pid, int lost_threshold);
    
    void Init();

    static int SendBeaconToWatchDog(grpc_t *grpc);
    static int SendCloseToWatchDog(grpc_t *grpc);

    void Shutdown();
};

}

#endif
