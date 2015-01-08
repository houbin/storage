#ifndef JVCLIENT_H
#define JVCLIENT_H

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
JVC_InitSDK -----------------01 初始化SDK资源
JVC_ReleaseSDK --------------02 释放SDK资源，必须最后被调用
JVC_RegisterCallBack --------03 设置分控端回调函数
JVC_Connect -----------------04 连接某通道网络服务
JVC_DisConnect --------------05 断开某通道服务连接
JVC_SendData ----------------06 发送数据
JVC_EnableLog ---------------07 设置写出错日志是否有效
JVC_SetLanguage -------------08 设置日志/提示信息语言(英文/中文) 
JVC_TCPConnect --------------09 方式连接某通道网络服务
JVC_GetPartnerInfo ----------10 获取伙伴节点信息
JVC_RegisterRateCallBack ----11 注册缓冲进度回调函数
JVC_StartLANSerchServer -----12 开启服务可以搜索局域网中维设备
JVC_StopLANSerchServer ------13 停止搜索服务
JVC_LANSerchDevice ----------14 搜索局域网中维设备
JVC_SetLocalFilePath --------15 自定义本地文件存储路径，包括日志，生成的其他关键文件等
JVC_SetDomainName -----------16 设置新的域名，系统将从其获取服务器列表
JVC_WANGetChannelCount ------17 通过外网获取某个云视通号码所具有的通道总数
JVC_StartBroadcastServer ----18 开启自定义广播服务
JVC_StopBroadcastServer -----19 停止自定义广播服务
JVC_BroadcastOnce -----------20 发送广播消息
JVC_ClearBuffer -------------21 清理本地缓存

JVC_EnableHelp---------------22 启用/停用快速链接服务(云视通小助手使用)
JVC_SetHelpYSTNO-------------23 设置对某些云视通号码的辅助支持
JVC_GetHelpYSTNO-------------24 获取对某些云视通号码的辅助支持
JVC_EnableLANTool------------25 开启服务可以搜索配置局域网中的设备
JVC_LANToolDevice------------26 搜索局域网中的可配置设备
JVC_SendCMD------------------27 向主控端发送一些特殊命令
JVC_AddFSIpSection-----------28 增加优先发送广播的IP组
JVC_MOLANSerchDevice --------29 手机搜索局域网中维设备
JVC_RegisterCommonCallBack---30 手机专用回调注册
JVC_QueryPosition------------31 当前区域查询
JVC_GetExamItem--------------32 获取需要检测的项目
JVC_ExamItem-----------------33 检测项目
JVC_GetDemo------------------34 获取演示点
JVC_HelperRemove-------------35 删除助手内号码
JVC_HelpQuery----------------36 查询助手是否连接成功
JVC_QueryDevice--------------37 查询设备是不是已经配置成功(手机搜索)
JVC_SetNet3G-----------------38 设置当前网络环境为3G或一般网络
JVC_SetPlayMode--------------39 播放模式设置
JVC_WANGetChannelCountByNickName--40 根据别名通过外网查询通道数
JVC_GetPositionID------------41 查询地区
*******************************************************************************************************/


/****************************************************************************
*名称  : JVC_InitSDK
*功能  : 初始化SDK资源，必须被第一个调用 
*参数  : [IN] nLocalStartPort 本地连接使用的起始端口 <0时默认9200
*返回值: TRUE     成功
         FALSE    失败
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_InitSDK(int nLocStartPort);
#else
	JVCLIENT_API bool __stdcall	JVC_InitSDK(int nLocStartPort);
#endif

/****************************************************************************
*名称  : JVC_ReleaseSDK
*功能  : 释放SDK资源，必须最后被调用 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_ReleaseSDK();
#else
	JVCLIENT_API void __stdcall	JVC_ReleaseSDK();
#endif

/****************************************************************************
*名称  : JVC_RegisterSCallBack
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
	JVCLIENT_API void  JVC_RegisterCallBack(FUNC_CCONNECT_CALLBACK ConnectCallBack,
													FUNC_CNORMALDATA_CALLBACK NormalData,
													FUNC_CCHECKRESULT_CALLBACK CheckResult,
													FUNC_CCHATDATA_CALLBACK ChatData,
													FUNC_CTEXTDATA_CALLBACK TextData,
													FUNC_CDOWNLOAD_CALLBACK DownLoad,
													FUNC_CPLAYDATA_CALLBACK PlayData);
#else
	JVCLIENT_API void __stdcall	JVC_RegisterCallBack(FUNC_CCONNECT_CALLBACK ConnectCallBack,
													FUNC_CNORMALDATA_CALLBACK NormalData,
													FUNC_CCHECKRESULT_CALLBACK CheckResult,
													FUNC_CCHATDATA_CALLBACK ChatData,
													FUNC_CTEXTDATA_CALLBACK TextData,
													FUNC_CDOWNLOAD_CALLBACK DownLoad,
													FUNC_CPLAYDATA_CALLBACK PlayData);
#endif

/****************************************************************************
*名称  : JVC_Connect
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
		 [IN] nTURNType     转发功能类型(禁用转发\启用转发\仅用转发)
		 [IN] nConnectType  连接类型，手机使用
		 [IN] chNickName	别名，不使用时赋值NULL;支持6-32字节UTF8字符串，禁止使用其他字符集格式的字符串,字符串中必须包含一个特殊字符，建议@ & * _
		 [IN] bVIP			是否是VIP，账号登陆时根据账号获取到的设备属性是还是否，本地登陆为否
*返回值: 无
*其他  : nLocalChannel <= -2 且 nChannel = -1 可连接服务端的特殊通道，
         可避开视频数据，用于收发普通数据
*****************************************************************************/
#ifndef WIN32
	#ifdef MOBILE_CLIENT
	JVCLIENT_API void JVC_Connect(int nType,int nLocalChannel,int nChannel,
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
	JVCLIENT_API void JVC_Connect(int nType,int nLocalChannel,int nChannel,
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
	JVCLIENT_API void __stdcall	JVC_Connect(int nType,int nLocalChannel,int nChannel,
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
*名称  : JVC_DisConnect
*功能  : 断开某通道服务连接 
*参数  : [IN] nLocalChannel 服务通道号 >=1
*返回值: 无
*其他  : 
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_DisConnect(int nLocalChannel);
#else
	JVCLIENT_API void __stdcall	JVC_DisConnect(int nLocalChannel);
#endif

/****************************************************************************
*名称  : JVC_SendData
*功能  : 发送数据 
*参数  : [IN] nLocalChannel   本地通道号 >=1
         [IN] uchType          数据类型：各种请求；各种控制；各种应答
         [IN] pBuffer         待发数据内容
		 [IN] nSize           待发数据长度
*返回值: 无
*其他  : 向通道连接的主控发送数据
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_SendData(int nLocalChannel, unsigned char uchType, unsigned char *pBuffer,int nSize);
#else
	JVCLIENT_API void __stdcall	JVC_SendData(int nLocalChannel, unsigned char uchType, unsigned char  *pBuffer,int nSize);
#endif

/****************************************************************************
*名称  : JVN_EnableLog
*功能  : 设置写出错日志是否有效 
*参数  : [IN] bEnable  TRUE:出错时写日志；FALSE:不写任何日志
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_EnableLog(int bEnable);
#else
	JVCLIENT_API void __stdcall	JVC_EnableLog(bool bEnable);
#endif

/****************************************************************************
*名称  : JVC_SetLanguage
*功能  : 设置日志/提示信息语言(英文/中文) 
*参数  : [IN] nLgType  JVN_LANGUAGE_ENGLISH/JVN_LANGUAGE_CHINESE
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_SetLanguage(int nLgType);
#else
	JVCLIENT_API void __stdcall	JVC_SetLanguage(int nLgType);
#endif

/****************************************************************************
*名称  : JVC_TCPConnect
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
	JVCLIENT_API void JVC_TCPConnect(int nLocalChannel,int nChannel,
									char *pchServerIP,int nServerPort,
									char *pchPassName,char *pchPassWord,
									int nYSTNO,char chGroup[4],
									int bLocalTry,
									int nConnectType,
									int nTURNType);
#else
	JVCLIENT_API void __stdcall	JVC_TCPConnect(int nLocalChannel,int nChannel,
												char *pchServerIP,int nServerPort,
												char *pchPassName,char *pchPassWord,
												int nYSTNO,char chGroup[4],
												BOOL bLocalTry,
												int nConnectType,
												int nTURNType);
#endif


/****************************************************************************
*名称  : JVC_GetPartnerInfo
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
	JVCLIENT_API void JVC_GetPartnerInfo(int nLocalChannel, char *pMsg, int *nSize);
#else
	JVCLIENT_API void __stdcall	JVC_GetPartnerInfo(int nLocalChannel, char *pMsg, int &nSize);
#endif

#ifndef WIN32
	JVCLIENT_API void JVC_RegisterRateCallBack(FUNC_CBUFRATE_CALLBACK BufRate);
#else
	JVCLIENT_API void __stdcall	JVC_RegisterRateCallBack(FUNC_CBUFRATE_CALLBACK BufRate);
#endif

/****************************************************************************
*名称  : JVC_StartLANSerchServer
*功能  : 开启服务可以搜索局域网中维设备
*参数  : [IN] nLPort      本地服务端口，<0时为默认9400
         [IN] nServerPort 设备端服务端口，<=0时为默认9103,建议统一用默认值与服务端匹配
		 [IN] LANSData    搜索结果回调函数
		 [IN] nType			0 只搜索本网段 1 启用ping 可以跨网段 2 发送整个C网
*返回值: TRUE/FALSE
*其他  : 开启了搜索服务才可以接收搜索结果，搜索条件通过JVC_LANSerchDevice接口指定
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_StartLANSerchServer(int nLPort, int nServerPort, FUNC_CLANSDATA_CALLBACK LANSData,int nType);
#else
	JVCLIENT_API bool __stdcall	JVC_StartLANSerchServer(int nLPort, int nServerPort, FUNC_CLANSDATA_CALLBACK LANSData,int nType);
#endif

/****************************************************************************
*名称  : JVC_StopLANSerchServer
*功能  : 停止搜索服务 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_StopLANSerchServer();
#else
	JVCLIENT_API void __stdcall	JVC_StopLANSerchServer();
#endif

/****************************************************************************
*名称  : JVC_LANSerchDevice
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
	JVCLIENT_API int  JVC_LANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence );
#else
	JVCLIENT_API bool __stdcall	JVC_LANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence );
#endif

/****************************************************************************
*名称  : JVC_SetLocalFilePath
*功能  : 自定义本地文件存储路径，包括日志，生成的其他关键文件等 
*参数  : [IN] chLocalPath  路径 形如："C:\\jovision"  其中jovision是文件夹
*返回值: 无
*其他  : 参数使用内存拷贝时请注意初始化，字符串需以'\0'结束
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_SetLocalFilePath(char chLocalPath[256]);
#else
	JVCLIENT_API bool __stdcall	JVC_SetLocalFilePath(char chLocalPath[256]);
#endif

/****************************************************************************
*名称  : JVC_SetDomainName 
*功能  : 设置新的域名，系统将从其获取服务器列表
*参数  : [IN]  pchDomainName     域名
[IN]  pchPathName       域名下的文件路径名 形如："/down/YSTOEM/yst0.txt"
*返回值: TRUE  成功
FALSE 失败
*其他  : 系统初始化(JVN_InitSDK)完后设置
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_SetDomainName(char *pchDomainName,char *pchPathName);
#else
	JVCLIENT_API bool __stdcall	JVC_SetDomainName(char *pchDomainName,char *pchPathName);
#endif

/****************************************************************************
*名称  : JVC_WANGetChannelCount
*功能  : 通过外网获取某个云视通号码所具有的通道总数
*参数  : [IN]  chGroup   编组号
         [IN]  nYstNO    云视通号码
		 [IN]  nTimeOutS 等待超时时间(秒)
*返回值: >0  成功,通道数
         -1 失败，原因未知
		 -2 失败，号码未上线
		 -3 失败，主控版本较旧，不支持该查询
*其他  : 系统初始化(JVN_InitSDK)完后 可独立调用
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_WANGetChannelCount(char chGroup[4], int nYSTNO, int nTimeOutS);
#else
	JVCLIENT_API int __stdcall	JVC_WANGetChannelCount(char chGroup[4], int nYSTNO, int nTimeOutS);
#endif

/****************************************************************************
*名称  : JVC_WANGetChannelCountByNickName
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
	JVCLIENT_API int JVC_WANGetChannelCountByNickName(char *chNickName, int nTimeOutS);
#else
	JVCLIENT_API int __stdcall	JVC_WANGetChannelCountByNickName(char *chNickName, int nTimeOutS);
#endif

/****************************************************************************
*名称  : JVC_StartBroadcastServer
*功能  : 开启自定义广播服务 
*参数  : [IN] nLPort      本地服务端口，<0时为默认9500
         [IN] nServerPort 设备端服务端口，<=0时为默认9106,建议统一用默认值与服务端匹配
		 [IN] BroadcastData  广播结果回调函数
*返回值: TRUE/FALSE
*其他  : 开启了广播服务才可以接收广播结果，广播内容通过JVC_BroadcastOnce接口指定；
         端口设置请一定注意和设备搜索相关端口作区别，否则数据将异常；
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_StartBroadcastServer(int nLPort, int nServerPort, FUNC_CBCDATA_CALLBACK BCData);
#else
	JVCLIENT_API bool __stdcall	JVC_StartBroadcastServer(int nLPort, int nServerPort, FUNC_CBCDATA_CALLBACK BCData);
#endif

/****************************************************************************
*名称  : JVC_StopBroadcastServer
*功能  : 停止自定义广播服务 
*参数  : 无
*返回值: 无
*其他  : 无
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_StopBroadcastServer();
#else
	JVCLIENT_API void __stdcall	JVC_StopBroadcastServer();
#endif

/****************************************************************************
*名称  : JVC_BroadcastOnce
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
	JVCLIENT_API int JVC_BroadcastOnce(int nBCID, unsigned char *pBuffer, int nSize, int nTimeOut);
#else
	JVCLIENT_API BOOL __stdcall JVC_BroadcastOnce(int nBCID, unsigned char  *pBuffer, int nSize, int nTimeOut);
#endif

/****************************************************************************
*名称  : JVC_ClearBuffer
*功能  : 清空本地缓存 
*参数  : [IN] nLocalChannel 服务通道号 >=1
*返回值: 无
*其他  : 仅对普通模式链接有效，多播连接不允许从本地清空
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API void JVC_ClearBuffer(int nLocalChannel);
#else
	JVCLIENT_API void __stdcall	JVC_ClearBuffer(int nLocalChannel);
#endif 
/****************************************************************************
*名称  : JVC_SetLocalNetCard
*功能  : 设置哪一张网卡 eth0,eth1,... for linux or 0, 1, 2,...for win  
*参数  : [IN] strCardName   网卡
*返回值: 成功 TRUE ,失败 FALSE
*日期  : 2013 5
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_SetLocalNetCard(char* strCardName);
#else
	JVCLIENT_API BOOL __stdcall JVC_SetLocalNetCard(char* strCardName);
#endif
/****************************************************************************
*名称  : JVC_EnableHelp
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
	JVCLIENT_API int JVC_EnableHelp(int bEnable, int nType,int nMaxLimit);
#else
	JVCLIENT_API BOOL __stdcall JVC_EnableHelp(BOOL bEnable, int nType);
#endif

/****************************************************************************
*名称  : JVC_SetHelpYSTNO
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
	JVCLIENT_API int JVC_SetHelpYSTNO(unsigned char *pBuffer, int nSize);
#else
	JVCLIENT_API BOOL __stdcall JVC_SetHelpYSTNO(unsigned char  *pBuffer, int nSize);
#endif

/****************************************************************************
*名称  : JVC_GetHelpYSTNO
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
	JVCLIENT_API int JVC_GetHelpYSTNO(unsigned char *pBuffer, int *nSize);
#else
	JVCLIENT_API int __stdcall JVC_GetHelpYSTNO(unsigned char  *pBuffer, int &nSize);
#endif

/****************************************************************************
*名称  : JVC_GetYSTStatus
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
	JVCLIENT_API int JVC_GetYSTStatus(char chGroup[4], int nYSTNO, int nTimeOut);
#else
	JVCLIENT_API int __stdcall JVC_GetYSTStatus(char chGroup[4], int nYSTNO, int nTimeOut);
#endif

/****************************************************************************
*名称  : JVC_EnableLANTool
*功能  : 开启服务可以搜索配置局域网中的设备 
*参数  : [IN] nEnable     1开启 0关闭
         [IN] nLPort      本地服务端口，<0时为默认9600
         [IN] nServerPort 设备端服务端口，<=0时为默认9104,建议统一用默认值与服务端匹配
		 [IN] LANTData    搜索结果回调函数
*返回值: 0失败/1成功
*其他  : 开启了搜索服务才可以接收搜索结果，搜索条件通过JVC_LANToolDevice接口指定
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_EnableLANTool(int nEnable, int nLPort, int nServerPort, FUNC_CLANTDATA_CALLBACK LANTData);
#else
	JVCLIENT_API int __stdcall	JVC_EnableLANTool(int nEnable, int nLPort, int nServerPort, FUNC_CLANTDATA_CALLBACK LANTData);
#endif

/****************************************************************************
*名称  : JVC_LANToolDevice
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
	JVCLIENT_API int JVC_LANToolDevice(char chPName[256], char chPWord[256], int nTimeOut);
#else
	JVCLIENT_API int __stdcall	JVC_LANToolDevice(char chPName[256], char chPWord[256], int nTimeOut);
#endif

/****************************************************************************
*名称  : JVC_SendCMD
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
	JVCLIENT_API int JVC_SendCMD(int nLocalChannel, unsigned char uchType, unsigned char  *pBuffer, int nSize);
#else
	JVCLIENT_API int __stdcall	JVC_SendCMD(int nLocalChannel, unsigned char  uchType, unsigned char  *pBuffer, int nSize);
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
 	JVCLIENT_API int JVC_AddFSIpSection( const IPSECTION * pStartIp, int nSize ,int bEnablePing );
#else
 	JVCLIENT_API int __stdcall	JVC_AddFSIpSection( const IPSECTION * pStartIp, int nSize ,BOOL bEnablePing );
 
#endif
 
/****************************************************************************
*名称  : JVC_MOLANSerchDevice
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
	JVCLIENT_API int  JVC_MOLANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence);
#else
	JVCLIENT_API bool __stdcall	JVC_MOLANSerchDevice(char chGroup[4], int nYSTNO, int nCardType, int nVariety, char chDeviceName[100], int nTimeOut,unsigned int unFrequence);
#endif

/****************************************************************************
*名称  : JVC_RegisterCommonCallBack
*功能  : 云视通库与应用层数据交互 回调注册
*参数  : 回调函数

*返回值: 无
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API void JVC_RegisterCommonCallBack(FUNC_COMM_DATA_CALLBACK pfWriteReadDataCallBack);
#else
JVCLIENT_API void __stdcall JVC_RegisterCommonCallBack(FUNC_COMM_DATA_CALLBACK pfWriteReadDataCallBack);
#endif

/****************************************************************************
*名称  : JVC_SetLocalNetCard
*功能  : 设置哪一张网卡 eth0,eth1,... for linux or 0, 1, 2,...for win  
*参数  : [IN] strCardName   网卡
*返回值: 成功 TRUE ,失败 FALSE
*日期  : 2013 5
*****************************************************************************/
#ifndef WIN32
JVCLIENT_API int JVC_SetLocalNetCard(char* strCardName);
#else
JVCLIENT_API BOOL __stdcall JVC_SetLocalNetCard(char* strCardName);
#endif

/****************************************************************************
*名称  : JVC_GetDemo
*功能  : 获取演示点
*参数  : [OUT] pBuff		  存放查询的号码的列表,内存由应用层创建分配 编组 : 4 BYTE 号码 : 4 BYTE 通道数 : 1 BYTE
         [IN] nBuffSize       创建的内存大小

*返回值: 正数 演示点的数量，-2 系统未初始化 -1 为内存太小，0 可能是网络库还没有获取到
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVC_GetDemo(unsigned char* pBuff,int nBuffSize);
#else
JVCLIENT_API int __stdcall JVC_GetDemo(BYTE* pBuff,int nBuffSize);
#endif

/****************************************************************************
*名称  : JVC_HelperRemove
*功能  : 删除助手内的号码 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效

*返回值: 无
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API void JVC_HelperRemove(char* pGroup,int nYST);
#else
JVCLIENT_API void __stdcall JVC_HelperRemove(char* pGroup,int nYST);
#endif
/****************************************************************************
*名称  : JVC_HelpQuery
*功能  : 查询助手的号码连接状态 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
		 [OUT] nCount     助手的数量

*返回值: -1 未初始化 0 未连接 1 连接 内网 2 转发连接 3 连接 外网
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVC_HelpQuery(char* pGroup,int nYST,int *nCount);
#else
JVCLIENT_API int __stdcall JVC_HelpQuery(char* pGroup,int nYST,int &nCount);
#endif


/****************************************************************************
*名称  : JVC_QueryDevice
*功能  : 查询号码是否已经搜索出来 
*参数  : [IN] chGroup     编组号，编组号+nYSTNO可确定唯一设备
         [IN] nYSTNO      搜索具有某云视通号码的设备，>0有效
         [IN] nTimeOut    超时时间单位毫秒
		 [IN] callBack    回调函数

*返回值: 0 错误 1 成功 等待回调
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int JVC_QueryDevice(char* pGroup,int nYST,int nTimeOut,FUNC_DEVICE_CALLBACK callBack);
#else
JVCLIENT_API int __stdcall JVC_QueryDevice(char* pGroup,int nYST,int nTimeOut,FUNC_DEVICE_CALLBACK callBack);
#endif



/****************************************************************************
*名称  : JVC_QueryPosition
*功能  : 查询当前的地址编号 
*参数  : [IN] callBack     回调函数，查询后由此函数返回
 
*返回值: 0 错误 1 成功 等待回调
*****************************************************************************/
#ifndef WIN32
JVCLIENT_API int JVC_QueryPosition(FUNC_POSITION_CALLBACK callBack);
#else
JVCLIENT_API int __stdcall JVC_QueryPosition(FUNC_POSITION_CALLBACK callBack);
#endif


/****************************************************************************
*名称  : JVC_GetExamItem
*功能  : 获取诊断项目
*参数  : [IN,OUT]	pExamItem  需要诊断的项目列表4BYTE 长度 +文本 [+ 4BYTE + 文本...]
[IN]	nSize          结果回调函数		 
*返回值: int 可以检测项的数目 ==0 没有检测项 <0 项目列表缓存太小
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int  JVC_GetExamItem(char *pExamItem,int nSize);
#else
JVCLIENT_API int __stdcall	JVC_GetExamItem(char *pExamItem,int nSize);
#endif


/****************************************************************************
*名称  : JVC_ExamItem
*功能  : 诊断某一项
*参数  : [IN]	nExamType  诊断类型 ：-1 全部诊断 其他根据返回的已知类型诊断
		 [IN]	pUserInfo          诊断时用户填写的信息		 
		 [IN]	callBackExam          结果回调函数		 
*返回值: 无
*****************************************************************************/
#ifndef WIN32
JVCLIENT_API int  JVC_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#else
JVCLIENT_API int __stdcall	JVC_ExamItem(int nExamType,char* pUserInfo,FUNC_EXAM_CALLBACK callBackExam);
#endif


/****************************************************************************
*名称  : JVC_SetNet3G
*功能  : 设置当前的网络模式是否是3G模式
*参数  : [IN]	bIs3GNet  TRUE 3G网络(跑流量的网) FALSE 正常的网络
*返回值: 0 网络未初始化 1 成功
*****************************************************************************/

#ifndef WIN32
JVCLIENT_API int  JVC_SetNet3G(int bIs3GNet);
#else
JVCLIENT_API int __stdcall	JVC_SetNet3G(BOOL bIs3GNet);
#endif


/****************************************************************************
*名称  : JVC_GetPositionID
*功能  : 查询当前所在的区域
*参数  : nGetType 查询类型 1 先通过第三方查询 2先通过afdvr查询
*返回值: 0 国内  1 国外 -1 出错，未知
*****************************************************************************/
#ifndef WIN32
	JVCLIENT_API int JVC_GetPositionID(int nGetType);
#else
	JVCLIENT_API int __stdcall JVC_GetPositionID(int nGetType);
#endif
#endif
