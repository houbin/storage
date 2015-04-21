// This file is generated auto. Do not modify it anytime.
#include "watchdog.h"

int CLIENT_REQ_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req)
{
	__GENERATE_FUNC_DEBUG__();

	grpc->root = cJSON_CreateObject();
	cJSON_AddStringToObject(grpc->root, "method", "send_beacon_to_watchdog");

	{
		cJSON *param = cJSON_CreateObject();
		cJSON_AddItemToObject(grpc->root, "param", param);
		if ((*req).rebootStr)
		{
			cJSON_AddStringToObject(param, "rebootStr", (*req).rebootStr);
		}
		cJSON_AddNumberToObject(param, "pid", (*req).pid);
		cJSON_AddNumberToObject(param, "lostThreshold", (*req).lostThreshold);
	}
	grpc_c_send(grpc);

	return 0;
}

int CLIENT_RESP_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp)
{
	__GENERATE_FUNC_DEBUG__();

	memset(resp, 0, sizeof(PARAM_RESP_watchdog_send_beacon_to_watchdog));
	if (grpc->error.errcode != 0)
		return grpc->error.errcode;
	return 0;
}

int CLIENT_watchdog_send_beacon_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_beacon_to_watchdog *req, PARAM_RESP_watchdog_send_beacon_to_watchdog *resp)
{
	int ret;
	__GENERATE_FUNC_DEBUG__();

	CLIENT_REQ_watchdog_send_beacon_to_watchdog(grpc, req);
	ret = grpc_c_recv(grpc);
	if (0 == ret)
	{
		ret = CLIENT_RESP_watchdog_send_beacon_to_watchdog(grpc, resp);
	}

	return ret;
}

int SERVER_watchdog_send_beacon_to_watchdog(grpc_t *grpc)
{
	int ret;
	PARAM_REQ_watchdog_send_beacon_to_watchdog req;
	PARAM_RESP_watchdog_send_beacon_to_watchdog resp;
	__GENERATE_FUNC_DEBUG__();
	memset(&req, 0, sizeof(req));
	memset(&resp, 0, sizeof(resp));

	{
		cJSON *param = cJSON_GetObjectItem(grpc->root, "param");
		if (param)
		{
			req.rebootStr = grpc_strdup(grpc, cJSON_GetObjectValueString(param, "rebootStr"));
			req.pid = cJSON_GetObjectValueInt(param, "pid");
			req.lostThreshold = cJSON_GetObjectValueInt(param, "lostThreshold");
		}
	}
	ret = USERDEF_watchdog_send_beacon_to_watchdog(grpc, &req, &resp);
	if (ret != 0)
		return ret;

	return 0;
}
int CLIENT_REQ_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req)
{
	__GENERATE_FUNC_DEBUG__();

	grpc->root = cJSON_CreateObject();
	cJSON_AddStringToObject(grpc->root, "method", "send_legalclose_to_watchdog");

	{
		cJSON *param = cJSON_CreateObject();
		cJSON_AddItemToObject(grpc->root, "param", param);
		if ((*req).rebootStr)
		{
			cJSON_AddStringToObject(param, "rebootStr", (*req).rebootStr);
		}
	}
	grpc_c_send(grpc);

	return 0;
}

int CLIENT_RESP_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp)
{
	__GENERATE_FUNC_DEBUG__();

	memset(resp, 0, sizeof(PARAM_RESP_watchdog_send_legalclose_to_watchdog));
	if (grpc->error.errcode != 0)
		return grpc->error.errcode;
	return 0;
}

int CLIENT_watchdog_send_legalclose_to_watchdog(grpc_t *grpc, PARAM_REQ_watchdog_send_legalclose_to_watchdog *req, PARAM_RESP_watchdog_send_legalclose_to_watchdog *resp)
{
	int ret;
	__GENERATE_FUNC_DEBUG__();

	CLIENT_REQ_watchdog_send_legalclose_to_watchdog(grpc, req);
	ret = grpc_c_recv(grpc);
	if (0 == ret)
	{
		ret = CLIENT_RESP_watchdog_send_legalclose_to_watchdog(grpc, resp);
	}

	return ret;
}

int SERVER_watchdog_send_legalclose_to_watchdog(grpc_t *grpc)
{
	int ret;
	PARAM_REQ_watchdog_send_legalclose_to_watchdog req;
	PARAM_RESP_watchdog_send_legalclose_to_watchdog resp;
	__GENERATE_FUNC_DEBUG__();
	memset(&req, 0, sizeof(req));
	memset(&resp, 0, sizeof(resp));

	{
		cJSON *param = cJSON_GetObjectItem(grpc->root, "param");
		if (param)
		{
			req.rebootStr = grpc_strdup(grpc, cJSON_GetObjectValueString(param, "rebootStr"));
		}
	}
	ret = USERDEF_watchdog_send_legalclose_to_watchdog(grpc, &req, &resp);
	if (ret != 0)
		return ret;

	return 0;
}


//Method List
grpcMethod_t watchdog_methodList_s[] = {
		{"send_beacon_to_watchdog"       , SERVER_watchdog_send_beacon_to_watchdog         , GRPC_USER_LEVEL_Administrator   },
		{"send_legalclose_to_watchdog"   , SERVER_watchdog_send_legalclose_to_watchdog     , GRPC_USER_LEVEL_Administrator   },
		{NULL, NULL, (GRPCUserLevel_e)0}
};

