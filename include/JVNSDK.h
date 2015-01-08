#ifndef JVNSDK_H
#define JVNSDK_H

#include "JVNSDKDef.h"
#ifndef WIN32
	#ifdef __cplusplus
		#define JVCLIENT_API extern "C"
	#else
		#define JVCLIENT_API
	#endif
#else
#define JVCLIENT_API extern "C" __declspec(dllexport)
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                                      分控端接口                                                     //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************接口清单********************************************************
JVNC_InitSDK -----------------01 初始化SDK资源
JVNC_ReleaseSDK --------------02 释放SDK资源，必须最后被调用
JVNC_RegisterCallBack --------03 设置分控端回调函数
JVNC_Connect -----------------04 连接某通道网络服务
JVNC_DisConnect --------------05 断开某通道服务连接
JVNC_SendData ----------------06 发送数据
JVNC_EnableLog ---------------07 设置写出错日志是否有效
JVNC_SetLanguage -------------08 设置日志/提示信息语言(英文/中文) 
JVNC_TCPConnect --------------09 方式连接某通道网络服务
JVNC_GetPartnerInfo ----------10 获取伙伴节点信息
JVNC_RegisterRateCallBack ----11 注册缓冲进度回调函数
JVNC_StartLANSerchServer -----12 开启服务可以搜索局域网中维设备
JVNC_StopLANSerchServer ------13 停止搜索服务
JVNC_LANSerchDevice ----------14 搜索局域网中维设备
JVNC_SetLocalFilePath --------15 自定义本地文件存储路径，包括日志，生成的其他关键文件等
JVNC_SetDomainName -----------16 设置新的域名，系统将从其获取服务器列表
JVNC_WANGetChannelCount ------17 通过外网获取某个云视通号码所具有的通道总数
JVNC_StartBroadcastServer ----18 开启自定义广播服务
JVNC_StopBroadcastServer -----19 停止自定义广播服务
JVNC_BroadcastOnce -----------20 发送广播消息
JVNC_ClearBuffer -------------21 清理本地缓存

JVNC_EnableHelp---------------22 启用/停用快速链接服务(云视通小助手使用)
JVNC_SetHelpYSTNO-------------23 设置对某些云视通号码的辅助支持
JVNC_GetHelpYSTNO-------------24 获取对某些云视通号码的辅助支持
JVNC_EnableLANTool------------25 开启服务可以搜索配置局域网中的设备
JVNC_LANToolDevice------------26 搜索局域网中的可配置设备
JVNC_SendCMD------------------27 向主控端发送一些特殊命令
JVNC_AddFSIpSection-----------28 增加优先发送广播的IP组
JVNC_MOLANSerchDevice --------29 手机搜索局域网中维设备
JVNC_RegisterCommonCallBack---30 手机专用回调注册
JVNC_QueryPosition------------31 当前区域查询
JVNC_GetExamItem--------------32 获取需要检测的项目
JVNC_ExamItem-----------------33 检测项目
JVNC_GetDemo------------------34 获取演示点
JVNC_HelperRemove-------------35 删除助手内号码
JVNC_HelpQuery----------------36 查询助手是否连接成功
JVNC_QueryDevice--------------37 查询设备是不是已经配置成功(手机搜索)
JVNC_SetNet3G-----------------38 设置当前网络环境为3G或一般网络
JVNC_WANGetChannelCountByNickName--39 根据别名通过外网查询通道数
JVNC_GetPositionID------------41 查询地区
*******************************************************************************************************/


/****************************************************************************
*名称  : JVNC_InitSDK
*功能  : 初始化SDK资源，必须被第一个调用 
*参数  : [IN] nLocalStartPort 本地连接使用的起始端口 <0时默认9200
*返回值: TRUE     成功
         FALSE    失败
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_InitSDK(int nLocStartPort);
#else
	JVCLIENT_API bool __stdcall	JVNC_InitSDK(int nLocStartPort);
#endif

/****************************************************************************
*名称  : JVNC_ReleaseSDK
*功能  : 释放SDK资源，必须最后被调用 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_ReleaseSDK();
#else
	JVCLIENT_API void __stdcall	JVNC_ReleaseSDK();
#endif

/****************************************************************************
*名称  : JVNC_RegisterSCallBack
*功能  : 设置分控端回调函数 
*参数  : [IN] ConnectCallBack   与主控连接状况回调函数
*返回值: 无
*其他  : 分控端回调函数包括：
             与主控端通信状态函数；      (连接状态)
			 实时监控处理                (收到实时监控数据)
		     录像检索结果处理函数；      (收到录像检索结果)
			 语音聊天/文本聊天函数       (远程语音和文本聊天)
			 远程下载函数；              (远程下载数据)
			 远程回放函数；              (远程回放数据)
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void  JVNC_RegisterCallBack(FUNC_CCONNECT_CALLBACK ConnectCallBack,
													FUNC_CNORMALDATA_CALLBACK NormalData,
													FUNC_CCHECKRESULT_CALLBACK CheckResult,
													FUNC_CCHATDATA_CALLBACK ChatData,
													FUNC_CTEXTDATA_CALLBACK TextData,
													FUNC_CDOWNLOAD_CALLBACK DownLoad,
													FUNC_CPLAYDATA_CALLBACK PlayData);
#else
	JVCLIENT_API void __stdcall	JVNC_RegisterCallBack(FUNC_CCONNECT_CALLBACK ConnectCallBack,
													FUNC_CNORMALDATA_CALLBACK NormalData,
													FUNC_CCHECKRESULT_CALLBACK CheckResult,
													FUNC_CCHATDATA_CALLBACK ChatData,
													FUNC_CTEXTDATA_CALLBACK TextData,
													FUNC_CDOWNLOAD_CALLBACK DownLoad,
													FUNC_CPLAYDATA_CALLBACK PlayData);
#endif

/****************************************************************************
*名称  : JVNC_Connect
*功能  : 连接某通道网络服务
*参数  : 
		[IN] nType 新添加的连接类型,0 IP  1 号码 2 昵称 3只TCP 4 号码 + 端口
		[IN] nLocalChannel 本地通道号 >=1
         [IN] nChannel      服务通道号 >=1
		 [IN] pchServerIP   当nYSTNO<0时，该参数指通道服务IP；当nYSTNO>=0时无效.
		 [IN] nServerPort   当nYSTNO<0时，该参数指通道服务port；当nYSTNO>=0时无效.
		 [IN] pchPassName   用户名
		 [IN] pchPassWord   密码
		 [IN] nYSTNO        云视通号码，不使用时置-1
		 [IN] chGroup       编组号，形如"A" "AAAA" 使用云视通号码时有效
		 [IN] bLocalTry     是否进行内网探测
		 [IN] nTURNType     YST方式时有效,转发功能类型(禁用转发\启用转发(默认值)\仅用转发)
*返回值: TRUE  成功
         FALSE 失败
*其他  : nLocalChannel <= -2 且 nChannel = -1 可连接服务端的特殊通道，
         可避开视频数据，用于收发普通数据
*****************************************************************************/
#ifndef WIN32
	#ifdef MOBILE_CLIENT
	JVCLIENT_API void JVNC_Connect(int nType,int nLocalChannel,int nChannel,
		                          char *pchServerIP,int nServerPort,
		                          char *pchPassName,char *pchPassWord,
		                          int nYSTNO,char chGroup[4],
		                          int bLocalTry,
		                          int nTURNType,
		                          int bCache,
								  int nConnectType,
								  char *chNickName,
								  int bSendRequestVido,
								  int bVIP);
    #else
	JVCLIENT_API void JVNC_Connect(int nType,int nLocalChannel,int nChannel,
											char *pchServerIP,int nServerPort,
											char *pchPassName,char *pchPassWord,
											int nYSTNO,char chGroup[4],
											int bLocalTry,
											int nTURNType,
											int bCache,
											char *chNickName,
											int bVIP);
    #endif
#else
	JVCLIENT_API void __stdcall	JVNC_Connect(int nType,int nLocalChannel,int nChannel,
											char *pchServerIP,int nServerPort,
											char *pchPassName,char *pchPassWord,
											int nYSTNO,char chGroup[4],
											int bLocalTry,
											int nTURNType,
											int bCache,
											char *chNickName,
											int bVIP);
#endif

/****************************************************************************
*名称  : JVNC_DisConnect
*功能  : 断开某通道服务连接 
*参数  : [IN] nLocalChannel 服务通道号 >=1
*返回值: 无
*其他  : 
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_DisConnect(int nLocalChannel);
#else
	JVCLIENT_API void __stdcall	JVNC_DisConnect(int nLocalChannel);
#endif

/****************************************************************************
*名称  : JVNC_SendData
*功能  : 发送数据 
*参数  : [IN] nLocalChannel   本地通道号 >=1
         [IN] uchType          数据类型：各种请求；各种控制；各种应答
         [IN] pBuffer         待发数据内容
		 [IN] nSize           待发数据长度
*返回值: 无
*其他  : 向通道连接的主控发送数据
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_SendData(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer,int nSize);
#else
	JVCLIENT_API void __stdcall	JVNC_SendData(int nLocalChannel, unsigned char uchType, void *pBuffer,int nSize);
#endif

/****************************************************************************
*名称  : JVNS_EnableLog
*功能  : 设置写出错日志是否有效 
*参数  : [IN] bEnable  TRUE:出错时写日志；FALSE:不写任何日志
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_EnableLog(int bEnable);
#else
	JVCLIENT_API void __stdcall	JVNC_EnableLog(bool bEnable);
#endif

/****************************************************************************
*名称  : JVNC_SetLanguage
*功能  : 设置日志/提示信息语言(英文/中文) 
*参数  : [IN] nLgType  JVNS_LANGUAGE_ENGLISH/JVNS_LANGUAGE_CHINESE
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_SetLanguage(int nLgType);
#else
	JVCLIENT_API void __stdcall	JVNC_SetLanguage(int nLgType);
#endif

/****************************************************************************
*名称  : JVNC_TCPConnect
*功能  : TCP方式连接某通道网络服务
*参数  : [IN] nLocalChannel 本地通道号 >=1
         [IN] nChannel      服务通道号 >=1
		 [IN] pchServerIP   当nYSTNO<0时，该参数指通道服务IP；当nYSTNO>=0时无效.
		 [IN] nServerPort   当nYSTNO<0时，该参数指通道服务port；当nYSTNO>=0时无效.
		 [IN] pchPassName   用户名
		 [IN] pchPassWord   密码
		 [IN] nYSTNO        云视通号码，不使用时置-1
		 [IN] chGroup       编组号，形如"A" "AAAA" 使用云视通号码时有效
		 [IN] bLocalTry     是否进行内网探测
		 [IN] nConnectType  连接方式：TYPE_PC_TCP/TYPE_MO_TCP
		 [IN] nTURNType     转发功能类型(禁用转发\启用转发\仅用转发)
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_TCPConnect(int nLocalChannel,int nChannel,
									char *pchServerIP,int nServerPort,
									char *pchPassName,char *pchPassWord,
									int nYSTNO,char chGroup[4],
									int bLocalTry,
									int nConnectType,
									int nTURNType);
#else
	JVCLIENT_API void __stdcall	JVNC_TCPConnect(int nLocalChannel,int nChannel,
												char *pchServerIP,int nServerPort,
												char *pchPassName,char *pchPassWord,
												int nYSTNO,char chGroup[4],
												BOOL bLocalTry,
												int nConnectType,
												int nTURNType);
#endif


/****************************************************************************
*名称  : JVNC_GetPartnerInfo
*功能  : 获取伙伴节点信息 
*参数  : [IN] nLocalChannel   本地通道号 >=1
         [OUT] pMsg   信息内容
		              (是否多播(1)+在线总个数(4)+已连接总数(4)+[IP(16) + port(4)+连接状态(1)+下载速度(4)+下载总量(4)+上传总量(4)]
					                +[...]...)
		 [OUT] nSize  信息总长度
*返回值: 无
*其他  : 调用频率严禁太高，否则会影响视频处理速度；
         频繁程度度不能低于1秒，最好在2秒以上或更长时间，时间越长影响越小。
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_GetPartnerInfo(int nLocalChannel, char *pMsg, int *nSize);
#else
	JVCLIENT_API void __stdcall	JVNC_GetPartnerInfo(int nLocalChannel, char *pMsg, int &nSize);
#endif

#ifndef WIN32
	JVCLIENT_API void JVNC_RegisterRateCallBack(FUNC_CBUFRATE_CALLBACK BufRate);
#else
	JVCLIENT_API void __stdcall	JVNC_RegisterRateCallBack(FUNC_CBUFRATE_CALLBACK BufRate);
#endif

/****************************************************************************
*名称  : JVNC_StartLANSerchServer
*功能  : 开启服务可以搜索局域网中维设备
*参数  : [IN] nLPort      本地服务端口，<0时为默认9400
         [IN] nServerPort 设备端服务端口，<=0时为默认9103,建议统一用默认值与服务端匹配
		 [IN] LANSData    搜索结果回调函数
		 [IN] nType			0 只搜索本网段 1 启用ping 可以跨网段 2 发送整个C网
*返回值: TRUE/FALSE
*其他  : 开启了搜索服务才可以接收搜索结果，搜索条件通过JVNC_LANSerchDevice接口指定
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_StartLANSerchServer(int nLPort, int nServerPort, FUNC_CLANSDATA_CALLBACK LANSData,int nType);
#else
	JVCLIENT_API bool __stdcall	JVNC_StartLANSerchServer(int nLPort, int nServerPort, FUNC_CLANSDATA_CALLBACK LANSData,int nType);
#endif

/****************************************************************************
*名称  : JVNC_StopLANSerchServer
*功能  : 停止搜索服务 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_StopLANSerchServer();
#else
	JVCLIENT_API void __stdcall	JVNC_StopLANSerchServer();
#endif

/****************************************************************************
*名称  : JVNC_LANSerchDevice
*功能  : 搜索局域网中维设备 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
         [IN] nCardType   搜索某型号的设备，>0有效,当nYSTNO>0时该参数无效
		 [IN] chDeviceName搜索某个别名的设备，strlen>0有效，当nYSTNO>0时无效
		 [IN] nVariety    搜索某个类别的设备，1板卡;2DVR;3IPC;>0有效,当nYSTNO>0时该参数无效
		 [IN] nTimeOut    本地搜索有效时间，单位毫秒。超过该时间的结果将被舍弃，
		                  超时时间到达后回调函数中将得到超时提示作为搜索结束标志。
						  如果不想使用SDK超时处理可以置为0，此时结果强全部返回给调用者。
						  
						  同网段的设备一般能在<500的时间内搜索到；
						  设备搜索建议至少设置2000，确保搜索的网段全面；
						  即插即用搜索可根据需要长或短，越短越可能遗漏不同网段的设备；
*返回值: 无
*其他  : 当两参数同时为0时，将搜索局域网中所有中维设备
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API bool JVNC_LANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence);
#else
	JVCLIENT_API bool __stdcall	JVNC_LANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence );
#endif

/****************************************************************************
*名称  : JVNC_SetLocalFilePath
*功能  : 自定义本地文件存储路径，包括日志，生成的其他关键文件等 
*参数  : [IN] chLocalPath  路径 形如："C:\\jovision"  其中jovision是文件夹
*返回值: 无
*其他  : 参数使用内存拷贝时请注意初始化，字符串需以'\0'结束
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SetLocalFilePath(char chLocalPath[256]);
#else
	JVCLIENT_API bool __stdcall	JVNC_SetLocalFilePath(char chLocalPath[256]);
#endif

/****************************************************************************
*名称  : JVNC_SetDomainName 
*功能  : 设置新的域名，系统将从其获取服务器列表
*参数  : [IN]  pchDomainName     域名
[IN]  pchPathName       域名下的文件路径名 形如："/down/YSTOEM/yst0.txt"
*返回值: TRUE  成功
FALSE 失败
*其他  : 系统初始化(JVNS_InitSDK)完后设置
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SetDomainName(char *pchDomainName,char *pchPathName);
#else
	JVCLIENT_API bool __stdcall	JVNC_SetDomainName(char *pchDomainName,char *pchPathName);
#endif

/****************************************************************************
*名称  : JVNC_WANGetChannelCount
*功能  : 通过外网获取某个云视通号码所具有的通道总数
*参数  : [IN]  chGroup   编组号
         [IN]  nYstNO    云视通号码
		 [IN]  nTimeOutS 等待超时时间(秒)
*返回值: >0  成功,通道数
         -1 失败，原因未知
		 -2 失败，号码未上线
		 -3 失败，主控版本较旧，不支持该查询
*其他  : 系统初始化(JVNS_InitSDK)完后 可独立调用
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_WANGetChannelCount(char chGroup[4], int nYSTNO, int nTimeOutS);
#else
	JVCLIENT_API int __stdcall	JVNC_WANGetChannelCount(char chGroup[4], int nYSTNO, int nTimeOutS);
#endif

/****************************************************************************
*名称  : JVNC_WANGetChannelCountByNickName
*功能  : 通过外网获取某个云视通号码所具有的通道总数
*参数  : [IN]  chNickName   别名 支持6-32字节UTF8字符串，禁止使用其他字符集格式的字符串,字符串中必须包含一个特殊字符，建议@ & * _
		 [IN]  nTimeOutS 等待超时时间(秒)
*返回值: >0  成功,通道数
         0  失败，参数有误
		 -1 失败，号码未上线
		 -2 失败，主控版本较旧，不支持该查询
		 -3 失败，其他原因
		 -4 失败，别名不存在
*其他  : 系统初始化(JVN_InitSDK)完后 可独立调用
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_WANGetChannelCountByNickName(char *chNickName, int nTimeOutS);
#else
	JVCLIENT_API int __stdcall	JVNC_WANGetChannelCountByNickName(char *chNickName, int nTimeOutS);
#endif

/****************************************************************************
*名称  : JVNC_StartBroadcastServer
*功能  : 开启自定义广播服务 
*参数  : [IN] nLPort      本地服务端口，<0时为默认9500
         [IN] nServerPort 设备端服务端口，<=0时为默认9106,建议统一用默认值与服务端匹配
		 [IN] BroadcastData  广播结果回调函数
*返回值: TRUE/FALSE
*其他  : 开启了广播服务才可以接收广播结果，广播内容通过JVNC_BroadcastOnce接口指定；
         端口设置请一定注意和设备搜索相关端口作区别，否则数据将异常；
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_StartBroadcastServer(int nLPort, int nServerPort, FUNC_CBCDATA_CALLBACK BCData);
#else
	JVCLIENT_API bool __stdcall	JVNC_StartBroadcastServer(int nLPort, int nServerPort, FUNC_CBCDATA_CALLBACK BCData);
#endif

/****************************************************************************
*名称  : JVNC_StopBroadcastServer
*功能  : 停止自定义广播服务 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_StopBroadcastServer();
#else
	JVCLIENT_API void __stdcall	JVNC_StopBroadcastServer();
#endif

/****************************************************************************
*名称  : JVNC_BroadcastOnce
*功能  : 发送广播消息 
*参数  : [IN] nBCID       广播ID,由调用者定义,用于在回调函数中匹配区分本次广播
         [IN] pBuffer     广播净载数据,数据不允许超出10k,否则将舍弃多余部分
         [IN] nSize       广播净载数据长度
		 [IN] nTimeOut    本次广播有效时间，单位毫秒。超过该时间的结果将被舍弃，
		                  超时时间到达后回调函数中将得到超时提示作为搜索结束标志。
						  如果不想使用SDK超时处理可以置为0，此时结果强全部返回给调用者。
*返回值: 无
*其他  : 目前该功能暂不支持并发广播，即并发调用时最后一次广播将覆盖之前的广播；
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_BroadcastOnce(int nBCID, unsigned char *pBuffer, int nSize, int nTimeOut);
#else
	JVCLIENT_API BOOL __stdcall JVNC_BroadcastOnce(int nBCID, BYTE *pBuffer, int nSize, int nTimeOut);
#endif

/****************************************************************************
*名称  : JVNC_ClearBuffer
*功能  : 清空本地缓存 
*参数  : [IN] nLocalChannel 服务通道号 >=1
*返回值: 无
*其他  : 仅对普通模式链接有效，多播连接不允许从本地清空
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVNC_ClearBuffer(int nLocalChannel);
#else
	JVCLIENT_API void __stdcall	JVNC_ClearBuffer(int nLocalChannel);
#endif 

/****************************************************************************
*名称  : JVNC_SetLocalNetCard
*功能  : 设置哪一张网卡 eth0,eth1,... for linux or 0, 1, 2,...for win  
*参数  : [IN] strCardName   网卡
*返回值: 成功 TRUE ,失败 FALSE
*日期  : 2013 5
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SetLocalNetCard(char* strCardName);
#else
	JVCLIENT_API BOOL __stdcall JVNC_SetLocalNetCard(char* strCardName);
#endif

/****************************************************************************
*名称  : JVNC_EnableHelp
*功能  : 启用/停用快速链接服务
*参数  : [IN] bEnable TRUE开启/FALSE关闭
         [IN] nType  当前使用者是谁，当bEnable为TRUE时有效
		             1 当前使用者是云视通小助手(独立进程)
		             2 当前使用者是云视通客户端，支持独立进程的小助手
		             3 当前使用者是云视通客户端，不支持独立进程的小助手
		[IN] nMaxLimit 链接的最大数
*返回值: 无
*其他  : 启用该功能后，网络SDK会对设定的号码进行连接提速等优化；
		 启用该功能后，网络SDK会支持小助手和客户端之间进行交互；
		 如果分控端支持小助手进程，则用小助手端使用nType=1，客户端使用nType=2即可；
		 如果客户端不支持小助手进程，则客户端使用nType=3即可，比如手机客户端；
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_EnableHelp(int bEnable, int nType,int nMaxLimit);
#else
	JVCLIENT_API BOOL __stdcall JVNC_EnableHelp(BOOL bEnable, int nType);
#endif

/****************************************************************************
*名称  : JVNC_SetHelpYSTNO
*功能  : 设置对某些云视通号码的辅助支持
*参数  : [IN] pBuffer 云视通号码集合，由STBASEYSTNO结构组成；比如有两个号码
                      STBASEYSTNO st1,STBASEYSTNO st1;
					  pBuffer的内容就是:
					  memcpy(bBuffer, &st1, sizeof(STBASEYSTNO));
					  memcpy(&bBuffer[sizeof(STBASEYSTNO)], &st2, sizeof(STBASEYSTNO));
         [IN] nSize   pBuffer的实际有效长度；如果是两个号码则为：
		              2*sizeof(STBASEYSTNO);
					  
*返回值: 无
*其他  : 云视通小助手端使用；
         客户端不支持小助手时客户端使用；

         添加后，网络SDK会对这些云视通号码进行连接提速等优化；
		 这是初始设置，程序运行中客户端也会有些新的号码，
		 会动态添加到内部；
		 STBASEYSTNOS 云视通号码,STYSTNO定义参看JVNSDKDef.h
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SetHelpYSTNO(unsigned char *pBuffer, int nSize);
#else
	JVCLIENT_API BOOL __stdcall JVNC_SetHelpYSTNO(BYTE *pBuffer, int nSize);
#endif

/****************************************************************************
*名称  : JVNC_GetHelpYSTNO
*功能  : 获取当前已知的云视通号码清单
*参数  : [IN/OUT] pBuffer 由调用者开辟内存；
                          返回云视通号码集合，由STBASEYSTNO结构组成；比如有两个号码
                          STBASEYSTNO st1,STBASEYSTNO st1;
                          pBuffer的内容就是:
                          memcpy(bBuffer, &st1, sizeof(STBASEYSTNO));
                          memcpy(&bBuffer[sizeof(STBASEYSTNO)], &st2, sizeof(STBASEYSTNO));
         [IN/OUT] nSize   调用时传入的是pBuffer的实际开辟长度，
		                  调用后返回的是pBuffer的实际有效长度；如果是两个号码则为：
                          2*sizeof(STBASEYSTNO);
*返回值: -1  错误，参数有误，pBuffer为空或是大小不足以存储结果；
          0  服务未开启
          1  成功
*其他  : 云视通小助手端使用；
         客户端不支持小助手时客户端使用；

         这是程序运行中已知的所有号码，即小助手会对这些号码进行连接优化支持；
		 该接口仅用于查询，由于内部会自动添加，查询结果不会长期有效；
		 STBASEYSTNOS 云视通号码,STYSTNO定义参看JVNSDKDef.h
*****************************************************************************/
#ifndef WIN32 
	JVCLIENT_API int JVNC_GetHelpYSTNO(unsigned char *pBuffer, int *nSize);
#else
	JVCLIENT_API int __stdcall JVNC_GetHelpYSTNO(BYTE *pBuffer, int &nSize);
#endif

/****************************************************************************
*名称  : JVNC_GetYSTStatus
*功能  : 获取某个云视通号码的在线状态
*参数  : [IN] chGroup  云视通号码的编组号；
         [IN] nYSTNO   云视通号码
		 [IN] nTimeOut 超时时间(秒)，建议>=2秒
*返回值: -1  错误，参数有误，chGroup为空或是nYSTNO<=0；
          0  本地查询太频繁，稍后重试
          1  号码在线
		  2  号码不在线
		  3  查询失败，还不能判定号码是否在线
*其他  : 1.注意，该函数目前仅限用于手机,pc端暂不允许使用；
         2.该函数对同一个号码不允许频繁调用，间隔>=10s;
		 3.该函数对不同号码不允许频繁调用，间隔>=1s;
*****************************************************************************/
#ifndef WIN32 
	JVCLIENT_API int JVNC_GetYSTStatus(char chGroup[4], int nYSTNO, int nTimeOut);
#else
	JVCLIENT_API int __stdcall JVNC_GetYSTStatus(char chGroup[4], int nYSTNO, int nTimeOut);
#endif

/****************************************************************************
*名称  : JVNC_EnableLANTool
*功能  : 开启服务可以搜索配置局域网中的设备 
*参数  : [IN] nEnable     1开启 0关闭
         [IN] nLPort      本地服务端口，<0时为默认9600
         [IN] nServerPort 设备端服务端口，<=0时为默认9104,建议统一用默认值与服务端匹配
		 [IN] LANTData    搜索结果回调函数
*返回值: 0失败/1成功
*其他  : 开启了搜索服务才可以接收搜索结果，搜索条件通过JVNC_LANToolDevice接口指定
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_EnableLANTool(int nEnable, int nLPort, int nServerPort, FUNC_CLANTDATA_CALLBACK LANTData);
#else
	JVCLIENT_API int __stdcall	JVNC_EnableLANTool(int nEnable, int nLPort, int nServerPort, FUNC_CLANTDATA_CALLBACK LANTData);
#endif

/****************************************************************************
*名称  : JVNC_LANToolDevice
*功能  : 搜索局域网中的可配置设备 
*参数  : [IN] chPName     用户名
		 [IN] chPWord     密码，使用用户名密码可提高IPC的安全性，即配置也是需要权限的
         [IN] nTimeOut    本地搜索有效时间，单位毫秒。超过该时间的结果将被舍弃，
		                  超时时间到达后回调函数中将得到超时提示作为搜索结束标志。
						  如果不想使用SDK超时处理可以置为0，此时结果强全部返回给调用者。
						  
						  同网段的设备一般能在<500的时间内搜索到；
						  设备搜索建议至少设置2000，确保搜索的网段全面；
						  即插即用搜索可根据需要长或短，越短越可能遗漏不同网段的设备；
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_LANToolDevice(char chPName[256], char chPWord[256], int nTimeOut);
#else
	JVCLIENT_API int __stdcall	JVNC_LANToolDevice(char chPName[256], char chPWord[256], int nTimeOut);
#endif

/****************************************************************************
*名称  : JVNC_SendCMD
*功能  : 向主控端发送一些特殊命令 
*参数  : [IN] nLocalChannel   本地通道号 >=1
		 [IN] uchType         数据类型
		 [IN] pBuffer         待发数据内容
		 [IN] nSize           待发数据长度
*返回值: 0  发送失败
         1  发送成功
		 2  对方不支持该命令
*其他  : 仅对普通模式链接有效；
         当前支持的有:只发关键帧命令JVN_CMD_ONLYI
		              和恢复满帧命令JVN_CMD_FULL
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SendCMD(int nLocalChannel, unsigned char uchType, unsigned char  *pBuffer, int nSize);
#else
	JVCLIENT_API int __stdcall	JVNC_SendCMD(int nLocalChannel, unsigned char  uchType, unsigned char  *pBuffer, int nSize);
#endif



/****************************************************************************
*名称  : JVC_AddFSIpSection
*功能  : 增加自定义IP段，以供优先搜索 ,优先搜索指定的IP段 first search
*参数  : [IN] pStartIp		  IPSECTION数组地址
		 [IN] nSize           IP段数*sizeof(IPSECTION)
		 [IN] bEnablePing     暂停、继续 ping线程 
*返回值: 0，成功 -1 失败
*其他  : 无
*****************************************************************************/
#ifndef WIN32
 	JVCLIENT_API int JVNC_AddFSIpSection( const IPSECTION * pStartIp, int nSize ,int bEnablePing );
#else
 	JVCLIENT_API int __stdcall	JVNC_AddFSIpSection( const IPSECTION * pStartIp, int nSize ,BOOL bEnablePing );
 
#endif

/****************************************************************************
*名称  : JVNC_MOLANSerchDevice
*功能  : 手机搜索局域网中维设备
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
         [IN] nCardType   搜索某型号的设备，>0有效,当nYSTNO>0时该参数无效
		 [IN] chDeviceName搜索某个别名的设备，strlen>0有效，当nYSTNO>0时无效
		 [IN] nVariety    搜索某个类别的设备，1板卡;2DVR;3IPC;>0有效,当nYSTNO>0时该参数无效
		 [IN] nTimeOut    本地搜索有效时间，单位毫秒。超过该时间的结果将被舍弃，
		                  超时时间到达后回调函数中将得到超时提示作为搜索结束标志。
						  如果不想使用SDK超时处理可以置为0，此时结果强全部返回给调用者。

						  同网段的设备一般能在<500的时间内搜索到；
						  设备搜索建议至少设置2000，确保搜索的网段全面；
						  即插即用搜索可根据需要长或短，越短越可能遗漏不同网段的设备；
		 [IN] unFrequence (单位为s )执行ping 网关的频率，默认请填入30s搜索一次，嵌入式设备可根据需要更改>20&&<24*3600有效.

*返回值: 无
*其他  : 当两参数同时为0时，将搜索局域网中所有中维设备
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int  JVNC_MOLANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence);
#else
	JVCLIENT_API bool __stdcall	JVNC_MOLANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence);
#endif

/****************************************************************************
*名称  : JVNC_RegisterCommonCallBack
*功能  : 云视通库与应用层数据交互 回调注册
*参数  : 回调函数

*返回值: 无
*****************************************************************************/

#ifndef WIN32
	JVCLIENT_API void JVNC_RegisterCommonCallBack(FUNC_COMM_DATA_CALLBACK pfWriteReadDataCallBack);
#else
	JVCLIENT_API void __stdcall JVNC_RegisterCommonCallBack(FUNC_COMM_DATA_CALLBACK pfWriteReadDataCallBack);
#endif

/****************************************************************************
*名称  : JVNC_SetNet3G
*功能  : 设置当前的网络模式是否是3G模式
*参数  : [IN]	bIs3GNet  TRUE 3G网络(跑流量的网) FALSE 正常的网络
*返回值: 0 网络未初始化 1 成功
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int  JVNC_SetNet3G(int bIs3GNet);
#else
JVCLIENT_API int __stdcall	JVNC_SetNet3G(bool bIs3GNet);
#endif

/****************************************************************************
*名称  : JVNC_GetDemo
*功能  : 获取演示点
*参数  : [OUT] pBuff		  存放查询的号码的列表,内存由应用层创建分配 编组 : 4 BYTE 号码 : 4 BYTE 通道数 : 1 BYTE
         [IN] nBuffSize       创建的内存大小

*返回值: 正数 演示点的数量，-2 系统未初始化 -1 为内存太小，0 可能是网络库还没有获取到
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVNC_GetDemo(unsigned char* pBuff,int nBuffSize);
#else
JVCLIENT_API int __stdcall JVNC_GetDemo(BYTE* pBuff,int nBuffSize);
#endif

/****************************************************************************
*名称  : JVNC_HelperRemove
*功能  : 删除助手内的号码 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效

*返回值: 无
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API void JVNC_HelperRemove(char* pGroup,int nYST);
#else
JVCLIENT_API void __stdcall JVNC_HelperRemove(char* pGroup,int nYST);
#endif
/****************************************************************************
*名称  : JVNC_HelpQuery
*功能  : 查询助手的号码连接状态 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
		 [OUT] nCount     助手的数量

*返回值: -1 未初始化 0 未连接 1 连接 内网 2 转发连接 3 连接 外网
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVNC_HelpQuery(char* pGroup,int nYST,int &nCount);
#else
JVCLIENT_API int __stdcall JVNC_HelpQuery(char* pGroup,int nYST,int &nCount);
#endif


/****************************************************************************
*名称  : JVNC_QueryDevice
*功能  : 查询号码是否已经搜索出来 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
         [IN] nTimeOut    超时时间单位毫秒
		 [IN] callBack    回调函数

*返回值: 0 错误 1 成功 等待回调
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVNC_QueryDevice(char* pGroup,int nYST,int nTimeOut,FUNC_DEVICE_CALLBACK callBack);
#else
JVCLIENT_API int __stdcall JVNC_QueryDevice(char* pGroup,int nYST,int nTimeOut,FUNC_DEVICE_CALLBACK callBack);
#endif



/****************************************************************************
*名称  : JVNC_QueryPosition
*功能  : 查询当前的地址编号 
*参数  : [IN] callBack     回调函数，查询后由此函数返回
 
*返回值: 0 错误 1 成功 等待回调
*****************************************************************************/
#ifndef WIN32
JVCLIENT_API int JVNC_QueryPosition(FUNC_POSITION_CALLBACK callBack);
#else
JVCLIENT_API int __stdcall JVNC_QueryPosition(FUNC_POSITION_CALLBACK callBack);
#endif

/****************************************************************************
*名称  : JVNC_GetExamItem
*功能  : 获取诊断项目
*参数  : [IN,OUT]	pExamItem  需要诊断的项目列表4BYTE 长度 +文本 [+ 4BYTE + 文本...]
[IN]	nSize          结果回调函数		 
*返回值: int 可以检测项的数目 ==0 没有检测项 <0 项目列表缓存太小
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int  JVNC_GetExamItem(char *pExamItem,int nSize);
#else
JVCLIENT_API int __stdcall	JVNC_GetExamItem(char *pExamItem,int nSize);
#endif


/****************************************************************************
*名称  : JVNC_ExamItem
*功能  : 诊断某一项
*参数  : [IN]	nExamType  诊断类型 ：-1 全部诊断 其他根据返回的已知类型诊断
		 [IN]	pUserInfo          诊断时用户填写的信息		 
		 [IN]	callBackExam          结果回调函数		 
*返回值: 无
*****************************************************************************/
#ifndef WIN32
JVCLIENT_API int  JVNC_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#else
JVCLIENT_API int __stdcall	JVNC_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#endif



/****************************************************************************
*名称  : JVNC_SetPlayMode
*功能  :  设置播放模式
*参数  : [IN] nPlayMode //0,流畅性优先,占用内存大，1，实时性优先,占用内存小
*返回值:  0，成功 -1 失败
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_SetPlayMode(int nPlayMode);
#else
	JVCLIENT_API int __stdcall	JVNC_SetPlayMode(int nPlayMode);
#endif




/****************************************************************************
*名称  : JVNC_GetPositionID
*功能  : 查询当前所在的区域
*参数  : nGetType 查询类型 1 先通过第三方查询 2先通过afdvr查询
*返回值: 0 国内  1 国外 -1 出错，未知
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVNC_GetPositionID(int nGetType);
#else
	JVCLIENT_API int __stdcall JVNC_GetPositionID(int nGetType);
#endif


#ifndef WIN32
	//#include "JVNSDKDef.h"
	#ifdef __cplusplus
		#define JVSERVER_API extern "C"
	#else
		#define JVSERVER_API
	#endif
#else
	#define JVSERVER_API extern "C" __declspec(dllexport)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                                      主控端接口                                                     //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************接口清单********************************************************
JVNS_InitSDK -----------------01 初始化SDK资源
JVNS_ReleaseSDK---------------02 释放SDK资源
JVNS_RegisterCallBack --------03 设置主控端回调函数
JVNS_ActiveYSTNO -------------04 激活云视通号码
JVNS_InitYST -----------------05 初始化云视通服务
JVNS_StartChannel ------------06 开启某通道网络服务
JVNS_StopChannel -------------07 停止某通道所有服务
JVNS_SendData ----------------08 发送数据(通道群发)
JVNS_SendDataTo --------------09 发送数据(定向发送)
JVNS_SendChatData ------------10 发送聊天信息(语音聊天和文本聊天)
JVNS_EnableLog ---------------11 设置写出错日志是否有效
JVNS_RegCard -----------------12 注册产品
JVNS_SetLanguage -------------13 设置日志/提示信息语言(英文/中文)
JVNS_GetLanguage -------------14 获取日志/提示信息语言(英文/中文)
JVNS_SendAndRecvS ------------15 与最快服务器交互
JVNS_StartWebServer ----------16 开启web服务
JVNS_StartSelfWebServer ------17 开启自定义web服务(OEM)
JVNS_StopWebServer -----------18 停止web服务
JVNS_SetClientLimit ----------19 设置分控数目上限
JVNS_GetClientLimit ----------20 获取分控数目上限设置值
JVNS_EnablePCTCPServer -------21 开启关闭PC客户TCP服务(保留暂未使用)
JVNS_EnableMOServer ----------22 开启关闭某通道的手机服务
JVNS_SendMOData --------------23 发送手机数据，对TYPE_MO_TCP/TYPE_MO_UDP连接有效
JVNS_Command -----------------24 运行特定指令，要求SDK执行特定操作
JVNS_StartLANSerchServer -----25 开启局域网搜索服务
JVNS_StopLANSerchServer ------26 停止局域网搜索服务
JVNS_SetLocalNetCard ---------27 设置哪一张网卡(IPC)
JVNS_SetDeviceName -----------28 设置本地设备别名
JVNS_SetDomainName -----------29 设置新的域名，系统将从其获取服务器列表
JVNS_SetLocalFilePath --------30 自定义本地文件存储路径，包括日志，生成的其他关键文件等
JVNS_StartStreamService ------31 开启流媒体服务
JVNS_StopStreamService -------32 停止流媒体服务
JVNS_GetStreamServiceStatus --33 查询流媒体服务状态
JVNS_StartBroadcastServer ----34 开启自定义广播服务(IPC)
JVNS_StopBroadcastServer -----35 关闭自定义广播服务(IPC)
JVNS_BroadcastRSP ------------36 发送自定义广播应答(IPC)
JVNS_SendPlay-----------------37 向指定目标发送远程回放数据,暂用于MP4文件远程回放
JVNS_EnableLANToolServer------38 开启关闭局域网生产工具服务
JVNS_RegDownLoadFileName------39 注册回调函数，用于调用者特殊处理远程下载文件名
JVNS_SetDeviceInfo------------40 设置设备信息
JVNS_GetDeviceInfo------------41 获取设备信息
JVNS_SetIPCWANMode------------42 启用IPC外网特殊处理模式
JVNS_GetNetSpeedNow-----------43 获取当前网络状况
JVNS_ClearBuffer--------------44 清空发送缓存
JVNS_SetLSPrivateInfo---------45 设置本地自定义信息，用于设备搜索
JVNS_SetWANClientLimit--------46 设置外网连接数目上限
JVNS_GetExamItem -------------47 查询可以检测的项目
JVNS_ExamItem ----------------48 检测项目
JVNS_SendChannelInfo----------49 设置每一个通道参数
JVNS_SendJvnInfo--------------50 设置每一个通道参数
JVNS_RegNickName--------------51 注册别名
JVNS_DeleteNickName-----------52 删除别名
JVNS_GetNickName--------------53 获取别名
JVNS_RegClientMsgCallBack-----54 注册分控消息回调
JVNS_GetPositionID------------55 查询地区
*******************************************************************************************************/                                                                                                     //


/****************************************************************************
*名称  : JVNS_InitSDK
*功能  : 初始化SDK资源，必须被第一个调用
*参数  : [IN] nYSTPort    用于与云视通服务通信的端口,0时默认9100
         [IN] nLocPort    用于提供主控服务通信的端口,0时默认9101
		 [IN] nSPort      用于与云视通服务器交互(激活号码，注册等)的端口,0时默认9102
		 [IN] nVersion    主控版本号，用于云视通服务器验证
		 [IN] lRSBuffer   主控端用于收发单帧数据的最小缓存，为0时默认150K，
		                  IPC高清数据单帧可能很大，可根据最大帧确定该值，若调大则对应分控端也需要相应调大.
						  该参数需求来自IPC；
		 [IN] bLANSpeedup 是否对内网连接优化提速(IPC局域网大码流传输特殊处理,其他普通码流产品建议置为FALSE)
		 [IN] stDeviceType 产品类型 详见JVNSDKDef.h 中 STDeviceType
		                   普通板卡样例：stDeviceType.nType = 1;stDeviceType.nMemLimit = 1;
						   普通IPC样例： stDeviceType.nType = 3;stDeviceType.nMemLimit = 1;
						   内存严重不足DVR样例：stDeviceType.nType = 2;stDeviceType.nMemLimit = 3;
		 [IN] nDSize       请填入sizeof(STDeviceType),用于校验传入结构合法性
*返回值: TRUE     成功
         FALSE    失败
*其他  : 主控端长期占用两个端口，一个用于与云视通服务器通信，一个用于与分控通信
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_InitSDK(int nYSTPort, int nLocPort, int nSPort, int nVersion, long lRSBuffer, int bLANSpeedup, STDeviceType stDeviceType, int nDSize);
#else
	JVSERVER_API bool __stdcall	JVNS_InitSDK(int nYSTPort, int nLocPort, int nSPort, int nVersion, long lRSBuffer, BOOL bLANSpeedup, STDeviceType stDeviceType, int nDSize);
#endif

/****************************************************************************
*名称  : JVNS_ReleaseSDK
*功能  : 释放SDK资源，必须最后被调用 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_ReleaseSDK();
#else
	JVSERVER_API void __stdcall	JVNS_ReleaseSDK();
#endif

/****************************************************************************
*名称  : JVNS_RegisterSCallBack
*功能  : 设置主控端回调函数 
*参数  : [IN] ConnectCallBack   分控连接状况回调函数
         ...
*返回值: 无
*其他  : 主控端回调函数包括：
             分控身份验证函数            (身份验证)
             与云视通服务器通信状态函数；(上线状态)
		     与分控端通信状态函数；      (连接状态)
		     录像检索函数；              (录像检索请求)
			 远程云台控制函数；          (远程云台控制)
			 语音聊天/文本聊天函数       (远程语音和文本聊天)
			 回放控制       (目前的回放控制mp4文件)
			 别名注册返回消息
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_RegisterCallBack(FUNC_SCHECKPASS_CALLBACK CheckPassCallBack,
											FUNC_SONLINE_CALLBACK OnlineCallBack,
											FUNC_SCONNECT_CALLBACK ConnectCallBack,
											FUNC_SCHECKFILE_CALLBACK CheckFileCallBack,
											FUNC_SYTCTRL_CALLBACK YTCtrlCallBack,
											FUNC_SCHAT_CALLBACK ChatCallBack,
											FUNC_STEXT_CALLBACK TextCallBack,
											FUNC_SFPLAYCTRL_CALLBACK FPlayCtrlCallBack,
											FUNC_RECVSERVERMSG_CALLBACK FRecvMsgCallBack);
#else
	JVSERVER_API void __stdcall	JVNS_RegisterCallBack(FUNC_SCHECKPASS_CALLBACK CheckPassCallBack,
												  FUNC_SONLINE_CALLBACK OnlineCallBack,
												  FUNC_SCONNECT_CALLBACK ConnectCallBack,
												  FUNC_SCHECKFILE_CALLBACK CheckFileCallBack,
												  FUNC_SYTCTRL_CALLBACK YTCtrlCallBack,
												  FUNC_SCHAT_CALLBACK ChatCallBack,
												  FUNC_STEXT_CALLBACK TextCallBack,
												  FUNC_SFPLAYCTRL_CALLBACK FPlayCtrlCallBack,
												  FUNC_RECVSERVERMSG_CALLBACK FRecvMsgCallBack);
#endif

/****************************************************************************
*名称  : JVNS_RegClientMsgCallBack
*功能  : 设置分控信息回调，
*参数  : [IN] FRecvMsgCallBack   分控消息回调函数，视频暂停，视频恢复
*返回值: 无

*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_RegClientMsgCallBack(FUNC_RECVCLIENTMSG_CALLBACK FRecvMsgCallBack);
#else
	JVSERVER_API void __stdcall JVNS_RegClientMsgCallBack(FUNC_RECVCLIENTMSG_CALLBACK FRecvMsgCallBack);
#endif

/****************************************************************************
*名称  : JVNS_ActiveYSTNO
*功能  : 激活云视通号码
*参数  : [IN]  pchPackGetNum  卡信息(STGetNum结构)
         [IN]  nLen           信息长度(sizeof(STGetNum))
         [OUT] nYSTNO         成功激活的云视通号码
*返回值: TRUE  成功
         FALSE 失败
*其他  : 云视通服务器地址在内部获取；
         函数独立使用，只返回激活的YST号码，程序内不做任何记录。
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_ActiveYSTNO(char *pchPackGetNum, int nLen, int *nYSTNO);
#else
	JVSERVER_API bool __stdcall	JVNS_ActiveYSTNO(char *pchPackGetNum, int nLen, int &nYSTNO);
#endif

/****************************************************************************
*名称  : JVNS_InitYST
*功能  : 初始化云视通服务
*参数  : [IN] 云视通号码等信息(STOnline结构)
         [IN] 信息长度
*返回值: 无
*其他  : 该函数需在启动通道云视通服务前调用，否则通道云视通服务将启动失败；
         
		 该函数只需调用一次，即，若所有通道中只要有需要启动云视通服务的，
		 在启动服务前调用一次该接口即可；
         
		 该函数记录云视通号码及服务，在上线时使用这些参数。
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_InitYST(char *pchPackOnline, int nLen);
#else
	JVSERVER_API void __stdcall	JVNS_InitYST(char *pchPackOnline, int nLen);
#endif

/****************************************************************************
*名称  : JVNS_StartChannel
*功能  : 开启某通道网络服务
*参数  : [IN] nChannel  服务通道号 >=1  
                        特别:nChannel=-1时特殊通道，可用于避开视频通道，传输普通数据

         [IN] nType     服务启动类型，0内外网服务都启动；1只启动局域网服务；

		 [IN] bJVP2P    开启中维网络直播，内存不充足或是没有硬盘时该功能不能启用；
		                即为TRUE时为多播方式；为FALSE时为传统方式；
						建议给用户展现时对应的两个选项：常规模式(无延时)和大连结数模式(小量延时)；
						默认为常规模式即可；
						没有特殊需要,可将该功能置为FALSE,不必展现给用户；

		 [IN] lBufLen   通道视频帧缓冲区大小，单位字节, 
		                普通方式时缓存大小指两个帧序列数据量(需>400*1024,应按最大码流计算)；
						JVP2P方式时建议置为>=8M，最小为1024000；
						如果该值设置不足，可能产生每个帧序列末尾的帧丢失造成卡顿的现象；

		 [IN] chBufPath 缓存文件路径，不使用文件缓存时置为""; 

*返回值: TRUE  成功
         FALSE 失败
*其他  : 每个通道的视频帧尺寸不同，因此所需缓冲区也不同，由应用层设置；
         该函数可重复调用，参数有变化时才进行实际操作，重复的调用会被忽略；
		 应用层在更新了某些设置后，可重新调用该接口设置服务。

         若开启了jvp2p，则有两种方式进行缓存：内存方式和文件方式
		 即lbuflen 和 chBufPath必须有一个是有效的，如果同时有效则优先采用内存方式，两者都无效则失败

		 建议：1.内存方式时每个通道建议>=8M内存，效果最佳，内存若充足则建议采用内存方式；
		       2.DVR等内存非常紧张的设备可以安装硬盘并且采用文件存储方式使用jvp2p。
			     DVR内存较充足的设备建议至少每个通道分配>=1M的视频缓冲区，否则不建议使用；
				 DVR内存紧张设备无法使用jvp2p，不必展现给用户，仅默认提供'常规模式'即可；
			   3.每个通道都可以单独开启jvp2p功能，但出于应用层管理的简便以及多通道对整体带宽的竞争，
			     建议所有通道统一开启或关闭该功能；即若使用jvp2p，则所有通道都使用，否则都不用；

         bJVP2P=TRUE时，将以"大连结数和保证流畅"的模式运行，并且主控端只要提供了必要的基础上传带宽，比如2M，
		 就能满足几十上百人同时连接，若配合开通中维VIP转发，将能确保画面流畅。但实时性稍差，
		 远程画面与实际画面可能延时2s-7s；
		 bJVP2P=FALSE时，将以"尽可能无延时"的模式运行，但连接数完全取决于带宽和设备资源，是一种传统传输方式；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartChannel(int nChannel, int nType, int bJVP2P, long lBufLen, char chBufPath[256]);
#else
	JVSERVER_API bool __stdcall	JVNS_StartChannel(int nChannel, int nType, BOOL bJVP2P, long lBufLen, char chBufPath[MAX_PATH]);
#endif

/****************************************************************************
*名称  : JVNS_StopChannel
*功能  : 停止某通道所有服务 
*参数  : [IN] nChannel 服务通道号 >=1
*返回值: 无
*其他  : 停止某个服务也可通过重复调用JVNS_StartChannel实现；
         若想停止所有服务，只能通过该接口实现。
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_StopChannel(int nChannel);
#else
	JVSERVER_API void __stdcall	JVNS_StopChannel(int nChannel);
#endif

/****************************************************************************
*名称  : JVNS_SendData
*功能  : 发送数据 
*参数  : [IN] nChannel   服务通道号 >=1
         [IN] uchType    数据类型：视频I;视频P;视频B;视频S;音频;尺寸;自定义类型;帧发送时间间隔
         [IN] pBuffer    待发数据内容,视频/音频/自定义数据时有效
		 [IN] nSize      待发数据长度,视频/音频/自定义数据时有效
		 [IN] nWidth     uchType=JVNS_DATA_S时表示帧宽/uchType=JVNS_CMD_FRAMETIME时表示帧间隔(单位ms)
		 [IN] nHeight    uchType=JVNS_DATA_S时表示帧高/uchType=JVNS_CMD_FRAMETIME时表示关键帧周期
*返回值: 无
*其他  : 以通道为单位，向通道连接的所有分控发送数据
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendData(int nChannel,unsigned char uchType,unsigned char *pBuffer,int nSize,int nWidth,int nHeight);
#else
	JVSERVER_API void __stdcall	JVNS_SendData(int nChannel,unsigned char uchType,BYTE *pBuffer,int nSize,int nWidth,int nHeight);
#endif

/****************************************************************************
*名称  : JVNS_SendDataTo
*功能  : 发送数据 
*参数  : [IN] nChannel   服务通道号 >=1
         [IN] uchType    数据类型：目前只用于尺寸发送尺寸;断开某连接;自定义类型
         [IN] pBuffer    待发数据内容
		 [IN] nSize      待发数据长度
		 [IN] nWidth     uchType=JVNS_DATA_S时表示帧宽/uchType=JVNS_CMD_FRAMETIME时表示帧间隔(单位ms)
		 [IN] nHeight    uchType=JVNS_DATA_S时表示帧高/uchType=JVNS_CMD_FRAMETIME时表示关键帧周期
*返回值: 无
*其他  : 向通道连接的某个具体分控发送数据
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendDataTo(int nClientID,unsigned char uchType,unsigned char *pBuffer,int nSize,int nWidth,int nHeight);
#else
	JVSERVER_API void __stdcall	JVNS_SendDataTo(int nClientID,unsigned char uchType,BYTE *pBuffer,int nSize,int nWidth,int nHeight);
#endif

/****************************************************************************
*名称  : JVNS_SendChatData
*功能  : 发送聊天信息(语音聊天和文本聊天)
*参数  : [IN] nChannel   服务通道号 >=1,广播式发送时有效;
         [IN] nClientID  分控ID,向指定分控发送,此时nChannel无效,优先级高于nChannel;
         [IN] uchType      数据类型：语音请求;
		                             文本请求;
		                             同意语音请求;
                                     同意文本请求;
								     语音数据;
								     文本数据;
								     语音关闭;
								     文本关闭;
         [IN] pBuffer    待发数据内容
         [IN] nSize      待发数据长度
*返回值: true   成功
         false  失败
*其他  : 调用者将聊天数据发送给请求语音服务的分控端;
         nChannel和nClientID不能同时<=0,即不能同时无效;
		 nChannel和nClientID同时>0时,nClientID优先级高,此时只向指定分控发送。
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SendChatData(int nChannel,int nClientID,unsigned char uchType,unsigned char *pBuffer,int nSize);
#else
	JVSERVER_API bool __stdcall	JVNS_SendChatData(int nChannel,int nClientID,unsigned char uchType,BYTE *pBuffer,int nSize);
#endif

/****************************************************************************
*名称  : JVNS_EnableLog
*功能  : 设置写出错日志是否有效 
*参数  : [IN] bEnable  TRUE:出错时写日志；FALSE:不写任何日志
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_EnableLog(int bEnable);
#else
	JVSERVER_API void __stdcall	JVNS_EnableLog(bool bEnable);
#endif

/****************************************************************************
*名称  : JVNS_SetLanguage
*功能  : 设置日志/提示信息语言(英文/中文) 
*参数  : [IN] nLgType  JVNS_LANGUAGE_ENGLISH/JVNS_LANGUAGE_CHINESE
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetLanguage(int nLgType);
#else
	JVSERVER_API void __stdcall	JVNS_SetLanguage(int nLgType);
#endif

/****************************************************************************
*名称  : JVNS_GetLanguage
*功能  : 获取日志/提示信息语言(英文/中文) 
*参数  : 无
*返回值: JVNS_LANGUAGE_ENGLISH/JVNS_LANGUAGE_CHINESE
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetLanguage();
#else
	JVSERVER_API int __stdcall	JVNS_GetLanguage();
#endif

/****************************************************************************
*名称  : JVNS_SetClientLimit
*功能  : 设置分控数目上限
*参数  : [IN] nChannel  通道号(>0;=0;<0)
         [IN] nMax      数目上限值 <0时表示无限制
		                nChannel<0 时表示分控总数目上限为nMax; 
						nChannel=0 时表示所有通道使用相同单通道分控数目上限为nMax; 
						nChannel>0 时表示单通道分控数目上限为nMax
*返回值: 无
*其他  : 可重复调用,以最后一次设置为准;
         总数上限和单通道上限可同时起作用;

         对普通产品，不严格区分内外网，只使用该接口就能达到限制连接数目的；
		 对于需要区分内外网连接数的产品，可配合调用JVNS_SetWANClientLimit来
		 限定外网总连接数；
		 即
		 如果同时使用JVNS_SetClientLimit和JVNS_SetWANClinetLimit,则：
		             JVNS_SetClientLimit限定的是基本连接数；
		             JVNS_SetWANClientLimit单独限定的是外网连接数；
		 如果只使用JVNS_SetClientLimit,限定的就是(不区分内外网)链接数目；
		 如果只使用JVNS_SetWANClientLimit,限定的就是外网连接数目；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetClientLimit(int nChannel, int nMax);
#else
	JVSERVER_API void __stdcall	JVNS_SetClientLimit(int nChannel, int nMax);
#endif

/****************************************************************************
*名称  : JVNS_SetWANClientLimit
*功能  : 设置外网分控数目上限
*参数  : [IN] nWANMax   数目上限值 <0时表示无限制
		                >=0表示外网分控总数目上限为nWANMax; 
		 
*返回值: 无
*其他  : 可重复调用,以最后一次设置为准;
         总数上限和单通道上限可同时起作用;

		 对普通产品，不严格区分内外网，只使用JVNS_SetClientLimit接口就能达到限制连接数目的；
		 对于需要区分内外网连接数的产品，可配合调用JVNS_SetWANClientLimit来
		 限定外网总连接数；
		 即
		 如果同时使用JVNS_SetClientLimit和JVNS_SetWANClinetLimit,则：
		 JVNS_SetClientLimit限定的是基本连接数；
		 JVNS_SetWANClientLimit限定的是外网连接数；

		 如果只使用JVNS_SetClientLimit,限定的就是(不区分内外网)链接数目；
		 
		 如果只使用JVNS_SetWANClientLimit,限定的就是外网连接数目；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetWANClientLimit(int nWANMax);
#else
	JVSERVER_API void __stdcall	JVNS_SetWANClientLimit(int nWANMax);
#endif

/****************************************************************************
*名称  : JVNS_GetClientLimit
*功能  : 获取分控数目上限设置值
*参数  : [IN] nChannel  通道号(>0;<0)
		                nChannel<0 时表示获取分控总数目上限; 
						nChannel>0 时表示获取单通道分控数目上限;
*返回值: 数目上限值 <=0表示无限制
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetClientLimit(int nChannel);
#else
	JVSERVER_API int __stdcall	JVNS_GetClientLimit(int nChannel);
#endif

/****************************************************************************
*名称  : JVNS_RegCard
*功能  : 注册产品
*参数  : [IN] chGroup    分组号，形如"A" "AAAA"
         [IN] pBuffer    待发数据内容(SOCKET_DATA_TRAN结构)
		 [IN] nSize      待发数据总长度
*返回值: TRUE  成功
         FALSE 失败
*其他  : 向最快服务发送数据
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_RegCard(char chGroup[4], unsigned char *pBuffer,int nSize);
#else
	JVSERVER_API bool __stdcall	JVNS_RegCard(char chGroup[4], BYTE *pBuffer,int nSize);
#endif

/****************************************************************************
*名称  : JVNS_SendAndRecvS
*功能  : 与最快服务器交互
*参数  : [IN] chGroup       分组号，形如"A" "AAAA"
         [IN] pBuffer       待发数据内容
         [IN] nSize         待发数据总长度
         [OUT] pRecvBuffer  接收数据缓冲，由调用者分配
         [IN/OUT] &nRecvLen 传入接收缓冲长度，返回实际数据长度
         [IN] nTimeOut      超时时间(秒)
*返回值: TRUE  成功
FALSE 失败
*其他  : 向最快服务发送数据
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SendAndRecvS(char chGroup[4],unsigned char *pBuffer,int nSize, 
		                               unsigned char *pRecvBuffer,int *nRecvLen,int nTimeOut);
#else
	JVSERVER_API bool __stdcall	JVNS_SendAndRecvS(char chGroup[4], BYTE *pBuffer,int nSize, 
		                                         BYTE *pRecvBuffer,int &nRecvLen,int nTimeOut);
#endif

/****************************************************************************
*名称  : JVNS_StartWebServer
*功能  : 开启web服务
*参数  : [IN] chHomeDir   目的文件所在本地路径 如"D:\\test"
         [IN] chDefIndex  目的文件名(本地)
		 [IN] chLocalIP   本地ip
         [IN] nPort       web服务端口
*返回值: TRUE  成功
		 FALSE 失败
*其他  : web服务功能即返回目的文件给客户端
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartWebServer(char chHomeDir[256], char chDefIndex[256], char chLocalIP[30], int nPort);
#else
	JVSERVER_API bool __stdcall JVNS_StartWebServer(char chHomeDir[256], char chDefIndex[256], char chLocalIP[30], int nPort);
#endif

/****************************************************************************
*名称  : JVNS_StartSelfWebServer
*功能  : 开启自定义web服务(OEM)
*参数  : [IN] chHomeDir   目的文件所在本地路径 如"D:\\test"
         [IN] chDefIndex  目的文件名(本地)
		 [IN] chLocalIP   本地ip
         [IN] nPort       web服务端口
		 [IN] chSelfWebPos   自定义网站控件index文件位置 如"www.afdvr.com/cloudsee"
		 [IN] bOnlyUseLocal  仅使用本地web服务，不使用外网网站
*返回值: TRUE  成功
		 FALSE 失败
*其他  : web服务功能即返回目的文件给客户端
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartSelfWebServer(char chHomeDir[256], char chDefIndex[256], 
		                                     char chLocalIP[30], int nPort,
											 char chSelfWebPos[500], int bOnlyUseLocal);
#else
	JVSERVER_API bool __stdcall JVNS_StartSelfWebServer(char chHomeDir[256], char chDefIndex[256], 
		                                               char chLocalIP[30], int nPort,
													   char chSelfWebPos[500], BOOL bOnlyUseLocal);
#endif

/****************************************************************************
*名称  : JVNS_StopWebServer
*功能  : 停止web服务
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_StopWebServer();
#else
	JVSERVER_API void __stdcall JVNS_StopWebServer();
#endif

/****************************************************************************
*名称  : JVNS_Command
*功能  : 运行特定指令，要求SDK执行特定操作
*参数  : [IN] nChannel  本地通道 ==0时对所有音视频通道有效(不包括特殊通道)
         [IN] nCMD  指令类型
*返回值: 无
*其他  : 支持的指令参看类型定义
         目前仅支持：CMD_TYPE_CLEARBUFFER
		 主控端进行了某个操作，如果希望客户端能立即更新到当前最新的数据，可执行该指令；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_Command(int nChannel, int nCMD);
#else
	JVSERVER_API void __stdcall JVNS_Command(int nChannel, int nCMD);
#endif
	
/****************************************************************************
*名称  : JVNS_StartLANSerchServer
*功能  : 开启局域网搜索服务
*参数  : [IN] nChannelCount 当前设备总的通道数
         [IN] nPort         服务端口号(<=0时为默认9103,建议使用默认值与分控端统一)
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartLANSerchServer(int nPort);
#else
	JVSERVER_API bool __stdcall JVNS_StartLANSerchServer(int nPort);
#endif

/****************************************************************************
*名称  : JVNS_StopLANSerchServer
*功能  : 停止局域网搜索服务
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_StopLANSerchServer();
#else
	JVSERVER_API void __stdcall JVNS_StopLANSerchServer();
#endif
	
/****************************************************************************
*名称  : JVNS_SetLocalNetCard
*功能  : 设置哪一张网卡 eth0,eth1,... for linux or 0, 1, 2,...for win  
*参数  : [IN] strCardName   网卡
*返回值: 成功 TRUE ,失败 FALSE
*日期  : 2012 5
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SetLocalNetCard(char* strCardName);
#else
	JVSERVER_API BOOL __stdcall JVNS_SetLocalNetCard(char* strCardName);
#endif

/****************************************************************************
*名称  : JVNS_EnablePCTCPServer
*功能  : 开启或关闭PC用户的TCP服务
*参数  : [IN] bEnable 开启/关闭
*返回值: TRUE  成功
		 FALSE 失败
*其他  : TCP服务功能接收分控以TCP方式连接，以TCP方式向分控发送数据；
         目前中维分控都未使用该TCP服务,没有特殊需要可不使用该功能；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_EnablePCTCPServer(int bEnable);
#else
	JVSERVER_API bool __stdcall JVNS_EnablePCTCPServer(BOOL bEnable);
#endif

/****************************************************************************
*名称  : JVNS_EnableMOServer
*功能  : 开启/关闭某通道的手机服务 
*参数  : [IN] nChannel   服务通道号 >=1 当=0时开启或关闭所有通道的手机服务
         [IN] bEnable    TRUE为开启；FALSE为关闭
		 [IN] nOpenType  服务开启类型：请置为0;bEnable为TRUE时有效；
		 [IN] bSendVSelf  是否单独发送手机视频数据，如果为TRUE，则手机数据和PC数据完全隔离，
		                 手机数据必须用JVNS_SendMOData发送；否则手机视频数据将不需要单独发送；
						 如果把手机当分控用，给手机和给分控的是相同的码流，bSendVSelf=FALSE即可；
		 [IN] bProtocol  是否用自定义协议,当bSendVSelf时有效；
		                 TRUE时，JVNS_SendMOData的数据打包需要单独添加头尾，使手机端能区别出该数据，
						         旧版主控发送的JPG数据和标准H264数据就是这种发送方式；
						 FALSE时，JVNS_SendMOData的数据打包格式和分控数据一致，与分控数据的区别仅仅是数据内容，
						         如果把手机当分控用，但给手机的数据是单独的码流，可以使用该方式；
								 
*返回值: 无
*其他  : 该函数只对JVNS_StartChannel开启了的通道起作用；JVNS_StopChannel之后需要重新启用手机服务；
         没开启的通道将不能接受手机连接；
         重复调用将以最后一次调用为准；
		 由于旧版分控协议不同，是完全作为分控使用，该功能不能将其区分和禁用；
		 使用该版主控时公司产品已可以完全支持h264码流，数据和分控相同，不再支持JGP数据，
		 仅将手机服务开启即可；
		 <*****使用建议*****>：
		 1.如果给分控的数据和给手机的数据完全相同，则开启方式为JVNS_EnableMOServer(0, TRUE, 0, FALSE, FALSE);
		 2.如果给分控的数据和给手机的数据格式相同，但是一个独立的码流，则开启方式为JVNS_EnableMOServer(0, TRUE, 0, TRUE, FALSE);
		 3.如果给分控的数据和给手机的数据格式不同，需要让手机端单独处理，则开启方式为JVNS_EnableMOServer(0, TRUE, 0, TRUE, TRUE);
		 理论上给手机的数据与分控数据分开，采用小帧率小码流，用第2种方式效果最佳；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_EnableMOServer(int nChannel, int bEnable, int nOpenType, int bSendVSelf, int bProtocol);
#else
	JVSERVER_API BOOL __stdcall	JVNS_EnableMOServer(int nChannel, BOOL bEnable, int nOpenType, BOOL bSendVSelf, BOOL bProtocol);
#endif

/****************************************************************************
*名称  : JVNS_SendMOData
*功能  : 发送手机数据，对TYPE_MO_TCP/TYPE_MO_UDP连接有效 
*参数  : [IN] nChannel   服务通道号 >=1
         [IN] uchType    数据类型：视频:自定义类型;
         [IN] pBuffer    待发数据内容,视频/自定义数据时有效
		 [IN] nSize      待发数据长度,视频/自定义数据时有效
*返回值: 无
*其他  : 以通道为单位，向通道TYPE_MO_TCP/TYPE_MO_UDP连接的所有手机发送一些自定义数据；
         由于JGP数据的淘汰，手机数据和PC分控数据已完全相同，该接口不支持JPG数据；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendMOData(int nChannel,unsigned char uchType,unsigned char *pBuffer,int nSize);
#else
	JVSERVER_API void __stdcall	JVNS_SendMOData(int nChannel,unsigned char uchType,BYTE *pBuffer,int nSize);
#endif

/****************************************************************************
*名称  : JVNS_StartStreamService
*功能  : 开启流媒体服务
*参数  : [IN] nChannel   服务通道号 >=1
         [IN] pSServerIP    流媒体服务器IP;
         [IN] nPort   流媒体服务器端口
	     [IN] bAutoConnect 开启服务失败是否自动重连
*返回值: 成功
         失败
*其他  : 当通道以普通方式运行时，流媒体服务器有效；
         当通道以中维网络直播方式运行时，流媒体服务器无效；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartStreamService(int nChannel,char *pSServerIP,int nPort,int bAutoConnect);
#else
	JVSERVER_API BOOL __stdcall JVNS_StartStreamService(int nChannel,char *pSServerIP,int nPort,BOOL bAutoConnect);
#endif

/****************************************************************************
*名称  : JVNS_StopStreamService
*功能  : 停止流媒体服务
*参数  : [IN] nChannel   服务通道号 >=1
*返回值: 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_StopStreamService(int nChannel);
#else
	JVSERVER_API void __stdcall JVNS_StopStreamService(int nChannel);
#endif

/****************************************************************************
*名称  : JVNS_GetStreamServiceStatus
*功能  : 查询流媒体服务状态
*参数  : [IN] nChannel   服务通道号 >=1
         [OUT] pSServerIP    流媒体服务器IP;
         [OUT] nPort   流媒体服务器端口
		 [OUT] bAutoConnect 流媒体服务是否正在自动重连
*返回值: 流媒体服务是否成功开启
*日期  : 2012 2
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetStreamServiceStatus(int nChannel,char *pSServerIP,int *nPort,int *bAutoConnect);
#else
	JVSERVER_API BOOL __stdcall JVNS_GetStreamServiceStatus(int nChannel,char *pSServerIP,int *nPort,BOOL *bAutoConnect);
#endif

/****************************************************************************
*名称  : JVNS_SetDomainName 
*功能  : 设置新的域名，系统将从其获取服务器列表
*参数  : [IN]  pchDomainName     域名
         [IN]  pchPathName       域名下的文件路径名 形如："/down/YSTOEM/yst0.txt"
*返回值: TRUE  成功
         FALSE 失败
*其他  : 系统初始化(JVNS_InitSDK)完后设置
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SetDomainName(char *pchDomainName,char *pchPathName);
#else
	JVSERVER_API BOOL __stdcall	JVNS_SetDomainName(char *pchDomainName,char *pchPathName);
#endif

/****************************************************************************
*名称  : JVNS_SetDeviceName
*功能  : 设置本地设备别名 
*参数  : [IN] chDeviceName   设备别名
*返回值: 无
*其他  : 为设备起一个别名，局域网设备搜索中可作搜索和显示使用；
         重复调用以最后一次调用为有效；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetDeviceName(char chDeviceName[100]);
#else
	JVSERVER_API void __stdcall	JVNS_SetDeviceName(char chDeviceName[100]);
#endif

/****************************************************************************
*名称  : JVNS_SetLocalFilePath
*功能  : 自定义本地文件存储路径，包括日志，生成的其他关键文件等 
*参数  : [IN] chLocalPath  路径 形如："C:\\jovision"  其中jovision是文件夹
*返回值: 无
*其他  : 参数使用内存拷贝时请注意初始化，字符串需以'\0'结束
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SetLocalFilePath(char chLocalPath[256]);
#else
	JVSERVER_API BOOL __stdcall	JVNS_SetLocalFilePath(char chLocalPath[256]);
#endif

/****************************************************************************
*名称  : JVNS_StartBroadcastServer
*功能  : 开启局域网广播服务
*参数  : [IN] nPort    服务端口号(<=0时为默认9106,建议使用默认值与分控端统一)
         [IN] BCData   广播数据回调函数
*返回值: 成功/失败
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_StartBroadcastServer(int nPort, FUNC_SBCDATA_CALLBACK BCData);
#else
	JVSERVER_API BOOL __stdcall JVNS_StartBroadcastServer(int nPort, FUNC_SBCDATA_CALLBACK BCData);
#endif

/****************************************************************************
*名称  : JVNS_StopBroadcastServer
*功能  : 停止局域网广播服务
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_StopBroadcastServer();
#else
	JVSERVER_API void __stdcall JVNS_StopBroadcastServer();
#endif

/****************************************************************************
*名称  : JVNS_BroadcastRSP
*功能  : 局域网广播响应
*参数  : [IN] nBCID  广播标识，取自回调函数
         [IN] pBuffer 广播净载数据
		 [IN] nSize   广播净载数据长度
		 [IN] nDesPort 广播目标端口，取自回调函数，或是与分控约定固定
*返回值: 成功/失败
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_BroadcastRSP(int nBCID, unsigned char *pBuffer, int nSize, int nDesPort);
#else
	JVSERVER_API BOOL __stdcall JVNS_BroadcastRSP(int nBCID, BYTE *pBuffer, int nSize, int nDesPort);
#endif

/****************************************************************************
*名称  : JVNS_SendPlay
*功能  : 发送回放MP4数据 
*参数  : [IN] nClientID   连接号
         [IN] uchType     类型
		 [IN] nConnectionType     连接类型
		 [IN] ucFrameType    帧类型	JVNS_DATA_I JVNS_DATA_S...
         [IN] pBuffer    待发数据内容,保留
		 [IN] nSize      待发数据长度,保留
         [IN] nWidth    宽度
		 [IN] nHeight      高度
		 [IN] nTotalFram      总帧数
*返回值: 无
*其他  : 向通道连接的某个具体分控发送数据
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendPlay(int nClientID,unsigned char uchType,int nConnectionType,unsigned char ucFrameType,unsigned char *pBuffer,int nSize,int nWidth,int nHeight,int nTotalFram);
#else
	JVSERVER_API void __stdcall	JVNS_SendPlay(int nClientID,BYTE uchType,int nConnectionType,BYTE ucFrameType,BYTE *pBuffer,int nSize,int nWidth,int nHeight,int nTotalFram);
#endif

/****************************************************************************
*名称  : JVNS_EnableLANToolServer
*功能  : 开启或关闭局域网生产工具服务
*参数  : [IN] bEnable         开启/关闭
         [IN] nPort           本地使用的端口，=0时默认为9104
         [IN] LanToolCallBack 工具回调函数
*返回值: TRUE  成功
		 FALSE 失败
*其他  : 局域网生产工具会向本地询问号码信息，并且反馈生产相关的附加信息
         信息详细说明请参考回调函数说明。
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_EnableLANToolServer(int bEnable, int nPort, FUNC_LANTOOL_CALLBACK LanToolCallBack);
#else
	JVSERVER_API bool __stdcall JVNS_EnableLANToolServer(BOOL bEnable, int nPort, FUNC_LANTOOL_CALLBACK LanToolCallBack);
#endif

/****************************************************************************
*名称  : JVNS_RegDownLoadFileName
*功能  : 注册回调函数，用于调用者特殊处理远程下载文件名
*参数  : [IN] DLFNameCallBack 远程下载文件名处理回调函数
*返回值: 无
*其他  : 普通产品不必使用，有特殊要求的可对客户端的下载文件名做二次处理
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_RegDownLoadFileName(FUNC_DLFNAME_CALLBACK DLFNameCallBack);
#else
	JVSERVER_API bool __stdcall JVNS_RegDownLoadFileName(FUNC_DLFNAME_CALLBACK DLFNameCallBack);
#endif

/****************************************************************************
*名称  : JVNS_SetIPCWANMode
*功能  : 启用IPC外网特殊处理模式
*参数  : 无
*返回值: 无
*其他  : 普通产品不必使用，有特殊要求的大码流产品可以使用；
         启用后，外网传输将采用特别处理来减少对内网连接的干扰；
		 在JVNS_InitSDK后，JVNS_StartChannel前调用即可；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetIPCWANMode();
#else
	JVSERVER_API void __stdcall JVNS_SetIPCWANMode();
#endif

/****************************************************************************
*名称  : JVNS_SetDeviceInfo
*功能  : 设置设备信息
*参数  : [IN] pstInfo 设备信息结构体
      	 [IN] nSize   设备信息结构体大小,用来前后兼容
	     [IN] nflag   设置选项标志 
*返回值: 成功返回0，失败返回-1
*其他  : 如设置设备支持的网络模式为有线+wifi，当前正在使用的是wifi，则参数是: 
		 nflag = DEV_SET_NET;//设置设备支持的网络模式
		 pstInfo->nNetMod= NET_MOD_WIFI | NET_MOD_WIRED;
         pstInfo->nNetMod= NET_MOD_WIFI;//设备当前使用的是wifi
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_SetDeviceInfo(STDEVINFO *pstInfo, int nSize,int nflag);
#else
	JVSERVER_API int JVNS_SetDeviceInfo(STDEVINFO *pstInfo, int nSize,int nflag);
#endif
	
/****************************************************************************
*名称  : JVNS_GetDeviceInfo
*功能  : 获取设备信息
*参数  : [OUT] pstInfo 设备信息结构体
	     [IN]  nSize   设备信息结构体大小,用来前后兼容
*返回值: 成功返回0，失败返回-1
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetDeviceInfo(STDEVINFO *pstInfo,int nSize);
#else
	JVSERVER_API int JVNS_GetDeviceInfo(STDEVINFO *pstInfo,int nSize);
#endif

/****************************************************************************
*名称  : JVNS_GetNetSpeedNow
*功能  : 获取当前网络状况
*参数  : [IN] nChannel	通道号
		 [IN] nClientID 连接号
		 [IN] nInterval	计算间隔, 该参数缺省时为默认时间间隔16秒
		 [OUT] pSpeed 返回当前网络状况，单位B/s
		 [OUT] pSendOK TRUE表示发送正常,FALSE表示有发送失败，通常是因为带宽不足
*返回值: 成功返回值>0，失败返回-1
*其他  : 返回当前网络状况值，失败时原因主要有视频传输没有开启，或者参数错误
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetNetSpeedNow(int nChannel, int nClientID, int nInterval, int* pSpeed, int* pSendOK, unsigned long *pdwFrameDelay);
#else
	JVSERVER_API int __stdcall JVNS_GetNetSpeedNow(int nChannel, int nClientID, int nInterval, int* pSpeed, BOOL* pSendOK, DWORD *pdwFrameDelay);
#endif

/****************************************************************************
*名称  : JVNS_SetLSPrivateInfo
*功能  : 设置本地自定义信息，用于设备搜索
*参数  : [IN]	chPrivateInfo  自定义信息
		 [IN]	nSize          自定义信息长度		 
*返回值: 无
*其他  : 可重复调用，以最后一次调用为准，之前的内容会被覆盖；
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetLSPrivateInfo(char chPrivateInfo[500], int nSize);
#else
	JVSERVER_API void __stdcall JVNS_SetLSPrivateInfo(char chPrivateInfo[500], int nSize);
#endif

/****************************************************************************
*名称  : JVNS_RegNickName
*功能  : 注册别名
*参数  : [IN]	chNickName  别名，支持6-32字节UTF8字符串，禁止使用其他字符集格式的字符串,字符串中必须包含一个特殊字符，建议@ & * _
*返回值: 返回注册情况：0开始进行注册; -1别名无效; -2未上线; -3正在注册中
*其他  : 注册返回结果见回调函数FUNC_RECVSERVERMSG_CALLBACK 类型为0
*****************************************************************************/

#ifndef WIN32
	JVSERVER_API int JVNS_RegNickName(char chNickName[36]);
#else
	JVSERVER_API int __stdcall JVNS_RegNickName(char chNickName[36]);
#endif

/****************************************************************************
*名称  : JVNS_DeleteNickName
*功能  : 删除别名
*参数  : [IN]	chNickName  别名，支持6-32字节UTF8字符串，禁止使用其他字符集格式的字符串,字符串中必须包含一个特殊字符，建议@ & * _
*返回值: 返回删除情况：0开始进行删除; -1别名无效; -2未上线; -3正在删除中
*其他  : 删除是否成功见回调函数FUNC_RECVSERVERMSG_CALLBACK 类型为2
*****************************************************************************/

#ifndef WIN32
	JVSERVER_API int JVNS_DeleteNickName(char chNickName[36]);
#else
	JVSERVER_API int __stdcall JVNS_DeleteNickName(char chNickName[36]);
#endif

/****************************************************************************
*名称  : JVNS_SetNickName
*功能  : 设置昵称 
*参数  : [IN] chDeviceName   设备昵称
*返回值: 无
*其他  : 
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SetNickName(char chNickName[32]);
#else
	JVSERVER_API void __stdcall	JVNS_SetNickName(char chNickName[32]);
#endif

/****************************************************************************
*名称  : JVNS_GetNickName
*功能  : 获取别名
*参数  : 无
*返回值: 返回获取情况：0开始进行获取; -1号码无效;-2正在获取中，稍后再试
*其他  : 获取结果见回调函数FUNC_RECVSERVERMSG_CALLBACK 类型为3
*****************************************************************************/

#ifndef WIN32
	JVSERVER_API int JVNS_GetNickName();
#else
	JVSERVER_API int __stdcall JVNS_GetNickName();
#endif


/****************************************************************************
*名称  : SendJvnInfo
*功能  : 设置每一个通道参数
*参数  : [IN]	chnn_info	通道参数
		 [IN]	svrAddr		服务器地址，通过回调函数FUNC_RECVSERVERMSG_CALLBACK获取		 
*返回值: 无
*其他  : 写入请求的信息，收到回调请求后再进行参数信息的发送
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendJvnInfo(JVSERVER_INFO *jvsvr_info, SOCKADDR_IN *svrAddr);
#else
	JVSERVER_API void __stdcall JVNS_SendJvnInfo(JVSERVER_INFO *jvsvr_info, SOCKADDR_IN *svrAddr);
#endif

/****************************************************************************
*名称  : SendChannelInfo
*功能  : 设置每一个通道参数
*参数  : [IN]	nMsgLen		信息长度
		 [IN]	chChnInfo	通道信息CHANNEL_INFO(多个), 	
		 [IN]	nConnCount  每一个号码对应一个连接数，用int表示，低2字节表示p2p连接数，最高1字节表示转发连接数，次高1字节表示手机连接数
*返回值: 无
*其他  : 首次启动时所有通道的信息，以后CHANNEL_INFO中的参数发生变化时再次调用
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API void JVNS_SendChannelInfo(int nMsgLen, char *chChnInfo, int nConnCount);
#else
	JVSERVER_API void __stdcall	JVNS_SendChannelInfo(int nMsgLen, char *chChnInfo, int nConnCount);
#endif


/****************************************************************************
*名称  : JVNS_GetExamItem
*功能  : 获取诊断项目
*参数  : [IN,OUT]	pExamItem  需要诊断的项目列表4BYTE 长度 +文本 [+ 4BYTE + 文本...]
[IN]	nSize          结果回调函数		 
*返回值: int 可以检测项的数目 ==0 没有检测项 <0 项目列表缓存太小
	*****************************************************************************/
	
#ifndef WIN32
	JVSERVER_API int  JVNS_GetExamItem(char *pExamItem,int nSize);
#else
		JVSERVER_API int __stdcall	JVNS_GetExamItem(char *pExamItem,int nSize);
#endif
	
/****************************************************************************
*名称  : JVNS_ExamItem
*功能  : 诊断某一项
*参数  : [IN]	nExamType  诊断类型 ：-1 全部诊断 其他根据返回的已知类型诊断
[IN]	pUserInfo          诊断时用户填写的信息		 
[IN]	callBackExam          结果回调函数		 
*返回值: 无
*****************************************************************************/
	
#ifndef WIN32
	JVSERVER_API int  JVNS_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#else
	JVSERVER_API int __stdcall	JVNS_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#endif


/****************************************************************************
*名称  : JVNS_GetPositionID
*功能  : 查询当前所在的区域
*参数  : nGetType 查询类型 1 先通过第三方查询 2先通过afdvr查询
*返回值: 0 国内  1 国外 -1 出错，未知
*****************************************************************************/
#ifndef WIN32
	JVSERVER_API int JVNS_GetPositionID(int nGetType);
#else
	JVSERVER_API int __stdcall JVNS_GetPositionID(int nGetType);
#endif


#endif
