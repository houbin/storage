#include "../include/storage.h"
#include "YST_userdef.h"
#include "../include/JVNSDKDef.h"
#include "../include/JVNSDK.h"
#include "../include/JvClient.h"

namespace storage
{

void YST_ClientConnectCallBack(int nLocalChannel, unsigned char uchType, char *pMsg, int nPWData)
{
    Log(logger, "enter connect yst client connect callback");

    switch(uchType)
    {
        case JVN_CCONNECTTYPE_CONNOK:
            Log(logger, "connect ok");
            break;
        
        case JVN_CCONNECTTYPE_DISOK:
            Log(logger, "disconnect ok");
            break;

        case JVN_CCONNECTTYPE_RECONN:
            Log(logger, "reconnect");
            break;

        case JVN_CCONNECTTYPE_CONNERR:
            Log(logger, "connect failed");

            /* TODO: do something */
            break;
        
        case JVN_CCONNECTTYPE_NOCONN:
            Log(logger, "no connect");
            break;
        
        case JVN_CCONNECTTYPE_DISCONNE:
            Log(logger, "error to disconnect");
            break;

        case JVN_CCONNECTTYPE_SSTOP:
            Log(logger, "stop");
            break;

        case JVN_CCONNECTTYPE_DISF:
            Log(logger, "disconnect failed");
            break;

        default:
            assert("invalid type" == NULL);
            break;
    }

    return;
}

void YST_ClientNormalDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, int nWidth, int nHeight)
{
    int32_t ret;
    Log(logger, "enter yst client normal data callback");
    
    uint32_t stream_id = (uint64_t)nLocalChannel;

    StreamTransferClient *transfer_client = NULL;
    ret = transfer_client_manager.find(stream_id, &transfer_client);
    if (ret < 0)
    {
        free(pBuffer);
        return;
    }

    transfer_client->


    

    return;
}

void YST_ClientCheckResultCallBack(int nLocalChannel, unsigned char *pBuffer, int nSize)
{
    Log(logger, "enter yst client check result callback");

    return;
}

void YST_ClientChatDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize)
{
    Log(logger, "enter yst client chat data callback");

    return;
}

void YST_ClientTextDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize)
{
    Log(logger, "enter yst client text data callback");

    return;
}

void YST_ClientDownloadCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, 
                                    int nFileLen)
{
    Log(logger, "enter yst client download callback");

    return;
}

void YST_ClientPlayDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, 
                                    int nWidth, int nHeight, int nTotalFrame)
{
    Log(logger, "enter yst client play data callback");

    return;
}


}
