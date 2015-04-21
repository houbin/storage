//Need user to modify this file.
#include "watchdog.h"
#include "../util/grpc_udp_service.h"
#include "../src/watchdog.h"

using namespace util;
using namespace watchdog;

int USERDEF_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp)
{
    int32_t ret = 0;
    WatchDog *dog = NULL;

    GrpcUdpUserDefInfo *user_info = (GrpcUdpUserDefInfo *)grpc->userdef;
    assert(user_info != NULL);

    dog = (WatchDog *)user_info->parent;
    assert(dog != NULL);

    ret = dog->HandleBeacon(req->rebootStr, req->pid, req->lostThreshold);
    if (ret != 0)
    {
        grpc_s_set_error(grpc, ret, "handle beacon msg error");
        return ret;
    }

    return 0;
}

int USERDEF_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp)
{
    int32_t ret = 0;
    WatchDog *dog = NULL;

    GrpcUdpUserDefInfo *user_info = (GrpcUdpUserDefInfo *)grpc->userdef;
    assert(user_info != NULL);

    dog = (WatchDog *)user_info->parent;
    assert(dog != NULL);

    ret = dog->HandleLegalClose(req->rebootStr);
    if (ret != 0)
    {
        grpc_s_set_error(grpc, ret, "handle legal close error");
        return ret;
    }

	return 0;
}
