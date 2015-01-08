#include "stream_transfer_client.h"
#include "../include/JVNSDKDef.h"
#include "../include/JVNSDK.h"
#include "../include/JvClient.h"
#include "yst_userdef.h"
#include "stream_info.h"

namespace storage
{

StreamTransferClient::StreamTransferClient(Logger *logger)
: logger_(logger)
{
    
}

int32_t StreamTransferClient::Start()
{
    Log(logger_, "start");

    JVC_Connect(0, stream_id, stream_info_.stream_server_channel_id_, stream_info_.stream_server_ip_,
        stream_info_.stream_server_port_, stream_info_.user_name_, stream_info_.user_passwd_,
        -1, "", true, 1, false, NULL, false);

    return 0;
}

void StreamTransferClient::Stop()
{
    Log(logger_, "stop, stream id is %d", stream_info.stream_id);

    JVC_DisConnect(stream_info_.stream_id);

    return;
}

void StreamTransferClient::Wait()
{
    Log(logger_, "wait, stream id is %d", stream_info_.stream_id);

    return;
}

}
