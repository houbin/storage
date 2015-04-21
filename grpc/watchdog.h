// This file is generated auto. Do not modify it anytime.
#ifndef _watchdog_H_
#define _watchdog_H_

#include "grpc.h"


#ifdef __cplusplus
extern "C" {
#endif

extern grpcMethod_t watchdog_methodList_s[];

extern grpcMethod_t watchdog_methodList_c[];

//--- send_beacon_to_watchdog definition ----

typedef struct{
	char *rebootStr; ///usr/bin/JCVR
	int pid;
	int lostThreshold;
} PARAM_REQ_watchdog_send_beacon_to_watchdog;

typedef struct{
	int idle;
} PARAM_RESP_watchdog_send_beacon_to_watchdog;
int USERDEF_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp);
int CLIENT_REQ_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req);
int CLIENT_RESP_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp);
int CLIENT_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp);

//--- send_legalclose_to_watchdog definition ----

typedef struct{
	char *rebootStr; ///usr/bin/JCVR
} PARAM_REQ_watchdog_send_legalclose_to_watchdog;

typedef struct{
	int idle;
} PARAM_RESP_watchdog_send_legalclose_to_watchdog;
int USERDEF_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp);
int CLIENT_REQ_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req);
int CLIENT_RESP_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp);
int CLIENT_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp);
#ifdef __cplusplus
}
#endif
#endif
