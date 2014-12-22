//Need user to modify this file.
#include <stdbool.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include "storage_json.h"

int USERDEF_storage_json_account_get_users(grpc_t *grpc, PARAM_REQ_storage_json_account_get_users *req, PARAM_RESP_storage_json_account_get_users *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_account_add_user(grpc_t *grpc, PARAM_REQ_storage_json_account_add_user *req, PARAM_RESP_storage_json_account_add_user *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_account_del_user(grpc_t *grpc, PARAM_REQ_storage_json_account_del_user *req, PARAM_RESP_storage_json_account_del_user *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_account_modify_user(grpc_t *grpc, PARAM_REQ_storage_json_account_modify_user *req, PARAM_RESP_storage_json_account_modify_user *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_channel_get(grpc_t *grpc, PARAM_REQ_storage_json_channel_get *req, PARAM_RESP_storage_json_channel_get *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_channel_add(grpc_t *grpc, PARAM_REQ_storage_json_channel_add *req, PARAM_RESP_storage_json_channel_add *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_channel_remove(grpc_t *grpc, PARAM_REQ_storage_json_channel_remove *req, PARAM_RESP_storage_json_channel_remove *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_channel_modify(grpc_t *grpc, PARAM_REQ_storage_json_channel_modify *req, PARAM_RESP_storage_json_channel_modify *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

#define MAX_STR_LEN 64
typedef struct ifinfo
{
	char name[MAX_STR_LEN + 1];
	char ip[MAX_STR_LEN + 1];
	char mac[MAX_STR_LEN + 1];
}IF_INFO;

#define MAX_INTERFACES 16    /* 最大接口数 */
struct ifreq buf[MAX_INTERFACES];
struct ifconf ifc;
int USERDEF_storage_json_get_first_up_interface(IF_INFO *if_info)
{
	bool found = false;
	int fd = 0;
	int ret = 0;
	int if_len = 0;

	if (if_info == NULL)
	{
		return -1;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;

	/* 获取接口列表 */
	ret = ioctl(fd, SIOCGIFCONF, (char *)&ifc);
	if (ret < 0)
	{
		return -1;
	}

	if_len = ifc.ifc_len / sizeof(struct ifreq);

	int i = 0;
	for(i = 0; i < if_len; i++)
	{
		char name_temp[MAX_STR_LEN + 1] = {0};
		strncpy(name_temp, buf[i].ifr_name, MAX_STR_LEN);

		ret = ioctl(fd, SIOCGIFFLAGS, (char *)&buf[i]);
		if (ret < 0)
		{
			return -1;
		}

		if (buf[i].ifr_flags & IFF_LOOPBACK)
		{
			continue;
		}

		if ((buf[i].ifr_flags & IFF_UP) && (buf[i].ifr_flags & IFF_RUNNING))
		{
			ret = ioctl(fd, SIOCGIFADDR, (char *)&buf[i]);
			if (ret < 0)
			{
				return -1;
			}
			
			strncpy(if_info->name, name_temp, MAX_STR_LEN);
			strncpy(if_info->ip, (char*)inet_ntoa(((struct sockaddr_in*)(&buf[i].ifr_addr))->sin_addr), MAX_STR_LEN);

			ret = ioctl(fd, SIOCGIFHWADDR, (char *)&buf[i]);
			if (ret < 0)
			{
				return -1;
			}

			snprintf(if_info->mac, MAX_STR_LEN, "N%02x%02x%02x%02x%02x%02x", 
	                    (unsigned char) buf[i].ifr_hwaddr.sa_data[0],
						(unsigned char) buf[i].ifr_hwaddr.sa_data[1],
						(unsigned char) buf[i].ifr_hwaddr.sa_data[2],
	                    (unsigned char) buf[i].ifr_hwaddr.sa_data[3],
					    (unsigned char) buf[i].ifr_hwaddr.sa_data[4],
					    (unsigned char) buf[i].ifr_hwaddr.sa_data[5]);

			found = true;
			break;
		}
	}

	if (found)
		return 0;
	else
		return -1;
}

int USERDEF_storage_json_broadcast_discovery(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_discovery *req, PARAM_RESP_storage_json_broadcast_discovery *resp)
{
	int ret = 0;
	IF_INFO if_info = {0};

	ret = USERDEF_storage_json_get_first_up_interface(&if_info);
	if (ret < 0)
	{
		resp->sid = grpc_strdup(grpc, "N000000000000");
		resp->ip = grpc_strdup(grpc, "0.0.0.0");
		resp->port = grpc_strdup(grpc, "0000");
	}
	else
	{
		resp->sid = grpc_strdup(grpc, if_info.mac);
		resp->ip = grpc_strdup(grpc, if_info.ip);
		resp->port = grpc_strdup(grpc, "0000");
	}

	resp->type = grpc_strdup(grpc, "storage");

	return 0;
}

int USERDEF_storage_json_broadcast_get_inet(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_get_inet *req, PARAM_RESP_storage_json_broadcast_get_inet *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_broadcast_eth_set(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_eth_set *req, PARAM_RESP_storage_json_broadcast_eth_set *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_broadcast_ppp_set(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_ppp_set *req, PARAM_RESP_storage_json_broadcast_ppp_set *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_broadcast_wifi_connect(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_wifi_connect *req, PARAM_RESP_storage_json_broadcast_wifi_connect *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_broadcast_wifi_list_ap(grpc_t *grpc, PARAM_REQ_storage_json_broadcast_wifi_list_ap *req, PARAM_RESP_storage_json_broadcast_wifi_list_ap *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_storage_get_info(grpc_t *grpc, PARAM_REQ_storage_json_storage_get_info *req, PARAM_RESP_storage_json_storage_get_info *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_storage_start_intial_disk(grpc_t *grpc, PARAM_REQ_storage_json_storage_start_intial_disk *req, PARAM_RESP_storage_json_storage_start_intial_disk *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_storage_get_disk_initial_rate(grpc_t *grpc, PARAM_REQ_storage_json_storage_get_disk_initial_rate *req, PARAM_RESP_storage_json_storage_get_disk_initial_rate *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_storage_get_record_fragments(grpc_t *grpc, PARAM_REQ_storage_json_storage_get_record_fragments *req, PARAM_RESP_storage_json_storage_get_record_fragments *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}

int USERDEF_storage_json_stream_get_service(grpc_t *grpc, PARAM_REQ_storage_json_stream_get_service *req, PARAM_RESP_storage_json_stream_get_service *resp)
{
#if 1
	__NULL_FUNC_DBG__();
	grpc_s_set_error(grpc, GRPC_ERR_METHOD_NOT_IMPLEMENTED, "Method not implemented");
	return GRPC_ERR_METHOD_NOT_IMPLEMENTED;
#else
	__NULL_FUNC_DBG__();
	int cnt = 1;
	int i;
	resp->users_cnt = cnt;
	resp->users = grpc_malloc(grpc, cnt * sizeof(*resp->users));
	for (i=0;i<cnt;i++)
	{
		resp->users[i].name = grpc_strdup(grpc, "username");
	}

	//grpc_set_error(grpc, 0, );
#endif

	return 0;
}
