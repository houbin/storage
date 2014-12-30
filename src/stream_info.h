#ifndef STORAGE_STREAM_INFO_H_
#define STORAGE_STREAM_INFO_H_   

#include <string>
#include "../util/logger.h"

using namespace util;

namespace storage
{

class StreamInfo
{
public:
    char stream_type_[32];
    char sid_[16]; // 云视通id or 设备的id
    char protocol_[16];
    bool main_stream_;
    bool sub_stream_;
    char main_stream_rstp_addr_[64];
    char sub_stream_rstp_addr_[64];
    char ip_[16]; //ipc
    char port_[8]; // ipc
    char name_[64]; // ipc
    int channelcnt_;
    int channel_id_;
    char user_name_[32];
    char user_passwd_[32];
    char stream_server_ip_[16];
    int  stream_server_port_;
    int stream_server_channel_id_;
    int stream_server_data_services_port_;

    StreamInfo() {}

    StreamInfo(char *stream_type, 
               char *sid, 
               char *protocol, 
               bool main_stream_, 
               bool sub_stream_, 
               char *main_stream_rstp_addr, 
               char *sub_stream_rstp_addr, 
               char *ip, 
               char *port, 
               char *name, 
               int channelcnt,
               int channel_id, 
               char *user_name,
               char *user_passwd, 
               char *stream_server_ip, 
               int stream_server_port,
               int stream_server_channel_id);

    bool operator<(const StreamInfo &info) const;
    void PrintToLogFile(Logger *logger);
};


}
#endif
