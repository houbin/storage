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
           char *port,
           char *name,
           int channelcnt,
           int channel_id, 
           char *user_name,
           char *user_passwd,
           char *stream_server_ip,
           int stream_server_port,
           int stream_server_channel_id)
{
    memcpy(stream_type_, stream_type, sizeof(stream_type_));
    memcpy(sid_, sid, sizeof(sid_));
    memcpy(protocol_, protocol, sizeof(protocol_));
    main_stream_ = main_stream;
    sub_stream_ = sub_stream;
    memcpy(main_stream_rstp_addr_, main_stream_rstp_addr, sizeof(main_stream_rstp_addr_));
    memcpy(sub_stream_rstp_addr_, sub_stream_rstp_addr, sizeof(sub_stream_rstp_addr_));
    memcpy(ip_, ip, sizeof(ip_));
    memcpy(port_, port, sizeof(port_));
    memcpy(name_, name, sizeof(name_));
    channelcnt_ = channelcnt;
    channel_id_ = channel_id;
    memcpy(user_name_, user_name, sizeof(user_name_));
    memcpy(user_passwd_, user_passwd, sizeof(user_passwd_));
    memcpy(stream_server_ip_, stream_server_ip, sizeof(stream_server_ip_));
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
    Log(logger, "stream_info prints: stream_type: %s, sid: %s, protocol: %s, main_stream: %d, sub_stream: %d, \
                main_stream_rstp_addr: %s, sub_stream_rstp_addr: %s, ip: %s, port: %s, name: %s, channelcnt: %d, channel_id: %d, \
                user_name: %s, user_passwd: %s, stream_server_ip: %s, stream_server_port: %d, stream_server_channel_id: %d",
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
                stream_server_channel_id_);

    return;
}

}

