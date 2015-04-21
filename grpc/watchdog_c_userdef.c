//Need user to modify this file.
#include "watchdog.h"


int USERDEF_RESP_watchdog_send_beacon_to_watchdog(grpc_t *grpc)
{
	PARAM_RESP_watchdog_send_beacon_to_watchdog resp;
	__NULL_FUNC_DBG__();
	CLIENT_RESP_watchdog_send_beacon_to_watchdog(grpc, &resp);
	//TODO: What ever you want

	return 0;
}

int USERDEF_RESP_watchdog_send_legalclose_to_watchdog(grpc_t *grpc)
{
	PARAM_RESP_watchdog_send_legalclose_to_watchdog resp;
	__NULL_FUNC_DBG__();
	CLIENT_RESP_watchdog_send_legalclose_to_watchdog(grpc, &resp);
	//TODO: What ever you want

	return 0;
}


//Method List
grpcMethod_t watchdog_methodList_c[] = {
		{"send_beacon_to_watchdog"       , USERDEF_RESP_watchdog_send_beacon_to_watchdog   , GRPC_USER_LEVEL_Administrator   },
		{"send_legalclose_to_watchdog"   , USERDEF_RESP_watchdog_send_legalclose_to_watchdog, GRPC_USER_LEVEL_Administrator   },
		{NULL, NULL, (GRPCUserLevel_e)0}
};

