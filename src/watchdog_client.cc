#include "../grpc/grpc.h"
#include "../util/grpc_udp_client.h"
#include "../grpc/watchdog.h"
#include "watchdog_client.h"

namespace storage
{

WatchDogClient::WatchDogClient(Logger *logger, struct sockaddr_in &server_addr, string reboot_path, pid_t pid, int lost_threshold)
: GrpcUdpClient(logger, server_addr, watchdog_methodList_c, SendBeaconToWatchDog, SendCloseToWatchDog), logger_(logger), server_addr_(server_addr), 
reboot_path_(reboot_path), pid_(pid), lost_threshold_(lost_threshold)
{

}

void WatchDogClient::Init()
{
    Create();

    LOG_INFO(logger_, "watchdog client init ok");
    return;
}

int WatchDogClient::SendBeaconToWatchDog(grpc_t *grpc)
{
    assert(grpc != NULL);

    int ret;
    Logger *logger = NULL;
    GrpcUdpUserDefInfo *user_info = (GrpcUdpUserDefInfo *)grpc->userdef;
    assert(user_info != NULL);

    WatchDogClient *dog_client = (WatchDogClient *)user_info->parent;
    assert(dog_client != NULL);
    logger = user_info->logger;

    PARAM_REQ_watchdog_send_beacon_to_watchdog req;
    PARAM_RESP_watchdog_send_beacon_to_watchdog resp;

    req.rebootStr = grpc_strdup(grpc, dog_client->reboot_path_.c_str());
    req.pid = dog_client->pid_;
    req.lostThreshold = dog_client->lost_threshold_;

    ret = CLIENT_watchdog_send_beacon_to_watchdog(grpc, &req, &resp);
    if (ret < 0)
    {
        LOG_INFO(logger, "send beacon error, and recv error, error code %d, error msg %s", grpc->error.errcode, grpc->error.message);
    }
    else
    {
        LOG_INFO(logger, "send beacon ok");
    }

    sleep((dog_client->lost_threshold_ >= 30) ? dog_client->lost_threshold_ / 6 : 2);

    return ret;
}

int WatchDogClient::SendCloseToWatchDog(grpc_t *grpc)
{
    assert(grpc != NULL);

    int ret;
    Logger *logger = NULL;
    GrpcUdpUserDefInfo *user_info = (GrpcUdpUserDefInfo *)grpc->userdef;
    assert(user_info != NULL);

    WatchDogClient *dog_client = (WatchDogClient *)user_info->parent;
    assert(dog_client != NULL);

    logger = user_info->logger;

    PARAM_REQ_watchdog_send_legalclose_to_watchdog req;
    PARAM_RESP_watchdog_send_legalclose_to_watchdog resp;

    req.rebootStr = grpc_strdup(grpc, dog_client->reboot_path_.c_str());
    
    ret = CLIENT_watchdog_send_legalclose_to_watchdog(grpc, &req, &resp);

    LOG_INFO(logger, "send and recv legal close ok");

    return 0;
}

void WatchDogClient::Shutdown()
{
    Stop();

    return;
}

}
