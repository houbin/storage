#ifndef STORAGE_YST_USERDEF_H_
#define STORAGE_YST_USERDEF_H_

namespace storage
{

void YST_ClientConnectCallBack(int nLocalChannel, unsigned char uchType, char *pMsg, int nPWData);
void YST_ClientNormalDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, nWidth, int nHeight);
void YST_ClientCheckResultCallBack(int nLocalChannel, unsigned char *pBuffer, int nSize);
void YST_ClientChatDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize);
void YST_ClientTextDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize);
void YST_ClientDownloadCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, int nFileLen);
void YST_ClientPlayDataCallBack(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer, int nSize, 
                                    int nWidth, int nHeight, int nTotalFrame);
}

#endif
