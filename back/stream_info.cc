#include "stream_info.h"
#include <string.h>

namespace storage
{

StreamInfo::StreamInfo(char *stream_type,
           char *sid,
           char *protocol,
           bool main_stream,
           bool sub_stream,
           char *main_stream_rstp_addr, 
           char *sub_stream_rstp_addr, 
           char *ip,
           int port,
           char *name,
           int channelcnt,
           int channel_id, 
           char *user_name,
           char *user_passwd,
           char *stream_server_ip,
           int stream_server_port,
           int stream_server_channel_id)
{
    memset(stream_type_, 0, sizeof(stream_type_));
    memset(sid_, 0, sizeof(sid_));
    memset(protocol_, 0, sizeof(protocol_));
    main_stream_ = 0;
    sub_stream_ = 0;
    memset(main_stream_rstp_addr_, 0, sizeof(main_stream_rstp_addr_));
    memset(sub_stream_rstp_addr_, 0, sizeof(sub_stream_rstp_addr_));
    memset(ip_, 0, sizeof(ip_));
    memset(port_, 0, sizeof(port_));
    memset(name_, 0, sizeof(name_));
    channelcnt_ = 0;
    channel_id_ = 0;
    memset(user_name_, 0, sizeof(user_name_));
    memset(user_passwd_, 0, sizeof(user_passwd_));
    memset(stream_server_ip_, 0, sizeof(stream_server_ip_));
    stream_server_port_ = 0;
    stream_server_channel_id_ = 0;
    stream_server_data_services_port_ = 0;

    strncpy(stream_type_, stream_type, sizeof(stream_type_) - 1);
    strncpy(sid_, sid, sizeof(sid_) - 1);
    strncpy(protocol_, protocol, sizeof(protocol_) - 1);
    main_stream_ = main_stream;
    sub_stream_ = sub_stream;
    strncpy(main_stream_rstp_addr_, main_stream_rstp_addr, sizeof(main_stream_rstp_addr_) - 1);
    strncpy(sub_stream_rstp_addr_, sub_stream_rstp_addr, sizeof(sub_stream_rstp_addr_) - 1);
    strncpy(ip_, ip, sizeof(ip_) - 1);
    port_ = port;
    strncpy(name_, name, sizeof(name_) - 1);
    channelcnt_ = channelcnt;
    channel_id_ = channel_id;
    strncpy(user_name_, user_name, sizeof(user_name_) - 1);
    strncpy(user_passwd_, user_passwd, sizeof(user_passwd_) - 1);
    strncpy(stream_server_ip_, stream_server_ip, sizeof(stream_server_ip_) - 1);
    stream_server_port_ = stream_server_port;
    stream_server_channel_id_ = stream_server_channel_id;
}

bool StreamInfo::operator<(const StreamInfo &info) const
{
    if (-1 == (memcmp(sid_, info.sid_, sizeof(sid_))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void StreamInfo::PrintToLogFile(Logger *logger)
{
    Log(logger, "stream_info prints: stream_type: %s, sid: %s, protocol: %s, main_stream: %d, sub_stream: %d, "
                "main_stream_rstp_addr: %s, sub_stream_rstp_addr: %s, ip: %s, port: %d, name: %s, channelcnt: %d, channel_id: %d, "
                "user_name: %s, user_passwd: %s, stream_server_ip: %s, stream_server_port: %d, stream_server_channel_id: %d, "
                "stream_server_data_services_port_: %d",
                stream_type_,
                sid_,
                protocol_,
                main_stream_,
                sub_stream_,
                main_stream_rstp_addr_,
                sub_stream_rstp_addr_,
                ip_,
                port_,
                name_,
                channelcnt_,
                channel_id_,
                user_name_,
                user_passwd_,
                stream_server_ip_,
                stream_server_port_,
                stream_server_channel_id_,
                stream_server_data_services_port_);

    return;
}

}

