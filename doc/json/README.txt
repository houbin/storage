通信协议：
基于JSON-RPC 2.0(http://www.jsonrpc.org/specification)修改而来。
不同：
1.取消了"jsonrpc"成员，id可以省略，不支持通知请求
2.错误对象可以只包含"code"成员，"message"非必须
3.不支持批量调用

发送：
		{
			"method":	"method",	//调用的方法的名字
			"params":				//方法传入的参数,如无可省略。
		}
响应：
		{
			"result":				//调用结果，当调用成功时，该成员是必须的；如果调用方法出现错误时，必须不包含该成员；该成员的值由服务器上调用的方法决定。
			"error":	{
				"code":	-32700,		//当调用发生错误时，该成员是必须的。在调用期间如果没有错误产生，必须不包含该成员。错误代码值-32768到-32000为保留值，作为预定义错误。
				"message":	"Parse error. Invalid JSON was received by the server."		//提供简短错误描述的字符串。做了改变，我们的协议可以省略该成员
			}
		}
		
jsonrpc库：https://github.com/hmng/jsonrpc-c


1.NVS_Cmd_Connect
获取设备信息，例如4屏的嵌入式解码器，有4个设备，每个设备驱动一个屏
eg.
	send:
		{
			"method":	"NVS_Cmd_Connect",
			"params":	{
				"username":	"abc",
				"passwd":	"123"
			}
		}
	response:
{
	"result":	[{
			"nVersion":	10,
			"acModel":	"VM9000-4HD",
			"acSwVersion":	"V1.0.1.3240",
			"acReleaseTime":	"Nov  5 2014 18:35:26",
			"nSN":	411806,
			"acGroup":	"B",
			"nYSTNO":	0,
			"acDevIP":	"199.196.199.1",
			"nChCount":	16,
			"nDeviceID":	0,
			"nDeviceCount":	4
		}, {
			"nVersion":	10,
			"acModel":	"VM9000-4HD",
			"acSwVersion":	"V1.0.1.3240",
			"acReleaseTime":	"Nov  5 2014 18:35:26",
			"nSN":	7661,
			"acGroup":	"B",
			"nYSTNO":	0,
			"acDevIP":	"199.196.199.2",
			"nChCount":	4,
			"nDeviceID":	1,
			"nDeviceCount":	4
		}, {
			"nVersion":	10,
			"acModel":	"VM9000-4HD",
			"acSwVersion":	"V1.0.1.3240",
			"acReleaseTime":	"Nov  5 2014 18:35:26",
			"nSN":	7661,
			"acGroup":	"B",
			"nYSTNO":	0,
			"acDevIP":	"199.196.199.3",
			"nChCount":	4,
			"nDeviceID":	2,
			"nDeviceCount":	4
		}, {
			"nVersion":	10,
			"acModel":	"VM9000-4HD",
			"acSwVersion":	"V1.0.1.3240",
			"acReleaseTime":	"Nov  5 2014 18:35:26",
			"nSN":	7661,
			"acGroup":	"B",
			"nYSTNO":	0,
			"acDevIP":	"199.196.199.4",
			"nChCount":	4,
			"nDeviceID":	3,
			"nDeviceCount":	4
		}]
}

		
2.NVS_Cmd_Disconnect
断开连接
eg.
	send:
		{
			"method":	"NVS_Cmd_Disconnect"
		}
	response:
		{
			"result":	0
		}

3.NVS_Cmd_GetDecoder
获取解码通道配置，nDevID为-1表示获取所有设备的所有解码通道
eg.
	send:
		{
			"method":	"NVS_Cmd_GetDecoder",
			"params":	{
				"nDevID":	0,
				"nCh":		-1
			}
		}
	response:
		{
			"result":	[{
					"nDevID":	0,
					"nCh":	0,
					"stVs":	{
						"acGroup":	"A",
						"nYSTNO":	219329020,
						"acDevIP":	"192.168.6.223",
						"nPort":	9101,
						"nChannel":	1,
						"acID":	"abc",
						"acPW":	"123",
						"nIpcType":	0,
						"acServiceAddr":	""
					}
				}]
		}

4.NVS_Cmd_SetDecoder
设置解码通道，nYSTNO为0表示删除这个解码器
eg.
	send:
		{
			"method":	"NVS_Cmd_SetDecoder",
			"params":	{
					"nDevID":       0,
					"nCh":  0,
					"stVs": {
									"acGroup":	"A",
									"nYSTNO":	219329020,
									"acDevIP":	"192.168.6.223",
									"nPort":	9101,
									"nChannel":	0,
									"acID":	"abc",
									"acPW":	"123",
									"nIpcType":	0,
									"acServiceAddr":	""
					}
			}
		}


		{
			"method":	"NVS_Cmd_SetDecoder",
			"params":	{
					"nDevID":       0,
					"nCh":  0,
					"stVs": {
									"acGroup":	"A",
									"nYSTNO":	2526293556,
									"acDevIP":	"192.168.6.105",
									"nPort":	9101,
									"nChannel":	0,
									"acID":	"abc",
									"acPW":	"123",
									"nIpcType":	0,
									"acServiceAddr":	""
					}
			}
		}
{
			"method":	"NVS_Cmd_SetDecoder",
			"params":	{
					"nDevID":       0,
					"nCh":  0,
					"stVs": {
									"acGroup":	"A",
									"nYSTNO":	506968066,
									"acDevIP":	"192.168.6.162",
									"nPort":	9101,
									"nChannel":	0,
									"acID":	"abc",
									"acPW":	"123",
									"nIpcType":	0,
									"acServiceAddr":	""
					}
			}
		}
	response:
		{
			"result":	0
		}
	

6.NVS_Cmd_GetDisplay
获取画面显示模式
eg.
	send:
	{
		"method":	"NVS_Cmd_GetDisplay",
		"params":	{
			"nDevID":	0,
			"nCh":		-1
		}
	}
	response:
	{
		"result":	{
			"nViewMode":	16,
			"nViewWindow":	16,
			"nSelected":	0,
			"nStartPos":	0,
			"nModeList":	[1, 4, 8, 9, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
		}
	}

7.NVS_Cmd_SetDisplay
设置画面显示模式
eg.
	send:
	{
		"method":	"NVS_Cmd_SetDisplay",
		"params":	{
	        "nDevID":       0,
	        "stDisplay":    {
	                "nViewMode":    8,
	                "nViewWindow":  8,
	                "nSelected":    1,
	                "nStartPos":    0
	        }
		}
	}
	response:
	{
		"result":	0
	}
	
8.NVS_Cmd_GetTour
获取轮显信息
eg.
	send:
		{
	        "method":       "NVS_Cmd_GetTour",
	        "params":       {
	                "nDevID":       1,
	                "nCh":  -1
	        }
		}
	response:
		{
		        "result":       {
		                "bTour":        1,
		                "secInterval":  5
		        }
		}	
	
8.NVS_Cmd_SetTour
获取轮显信息
eg.
	send:
		{
		    "method":       "NVS_Cmd_SetTour",
		    "params":       {
		        "nDevID":       1,
		        "stTour":       {
		            "bTour":        0,
		            "secInterval":  5
		        }
		    }
		}
	response:
		{
		    "result":       0
		}

9.NVS_Cmd_GetDecoderStatus
获取解码状态
eg.
	send:
{
        "method":       "NVS_Cmd_GetDecoderStatus",
        "params":       {
                "nDevID":       0,
                "nCh":  1
        }
}
	response:
{
        "result":       [{
                        "nDevID":       1,
                        "nCh":  0,
                        "nStatus":      3
                }, {
                        "nDevID":       1,
                        "nCh":  1,
                        "nStatus":      3
                }, {
                        "nDevID":       1,
                        "nCh":  2,
                        "nStatus":      3
                }, {
                        "nDevID":       1,
                        "nCh":  3,
                        "nStatus":      3
                }]
}

10.NVS_Cmd_GetScreen
获取显示器视频调节参数
eg.
	send:
{
        "method":       "NVS_Cmd_GetScreen",
        "params":       {
                "nDevID":       0,
                "nCh":  -1
        }
}
	response:
{
        "result":       {
                "nBright":      30,
                "nContrast":    35,
                "nSaturation":  45,
                "stResolution": {
                        "nW":   1280,
                        "nH":   1024
                },
                "stList":       [{
                                "nW":   1024,
                                "nH":   768
                        }, {
                                "nW":   1280,
                                "nH":   800
                        }, {
                                "nW":   1280,
                                "nH":   1024
                        }, {
                                "nW":   1440,
                                "nH":   900
                        }, {
                                "nW":   1920,
                                "nH":   1080
                        }, {
                                "nW":   0,
                                "nH":   0
                        }, {
                                "nW":   0,
                                "nH":   0
                        }, {
                                "nW":   0,
                                "nH":   0
                        }]
        }
}

11.NVS_Cmd_SetScreen
获取显示器视频调节参数
eg.
	send:
{
        "method":       "NVS_Cmd_SetScreen",
        "params":       {
                "nDevID":       0,
                "stScreen":     {
                        "nBright":      50,
                        "nContrast":    55,
                        "nSaturation":  55,
                        "stResolution": {
                                "nW":   1280,
                                "nH":   1024
                        },
                        "stList":       [{
                                        "nW":   1024,
                                        "nH":   768
                                }, {
                                        "nW":   1280,
                                        "nH":   800
                                }, {
                                        "nW":   1280,
                                        "nH":   1024
                                }, {
                                        "nW":   1440,
                                        "nH":   900
                                }, {
                                        "nW":   1920,
                                        "nH":   1080
                                }, {
                                        "nW":   0,
                                        "nH":   0
                                }, {
                                        "nW":   0,
                                        "nH":   0
                                }, {
                                        "nW":   0,
                                        "nH":   0
                                }]
                }
        }
}
	response:
{
        "result":       0
}


12.NVS_Cmd_SetReboot
设备重启或者恢复设置，nDevID：-1表示所有设备
eg.
	send:
{
        "method":       "NVS_Cmd_SetReboot",
        "params":       {
                "nDevID":       1,
                "stReboot":     {
                        "bReboot":      1,
                        "bResetCfg":    1,
                        "bResetNetCfg": 1,
                        "bResetUser":   1,
                        "bResetYstNO":  1,
                        "bResetLanguage":       1,
                        "bResetLog":    1
                }
        }
}
	response:
{
        "result":       0
}


13.NVS_Cmd_SysCmd
执行系统命令
eg.
	send:
{
        "method":       "NVS_Cmd_SysCmd",
        "params":       {
                "nDevID":       1,
                "stCmd":        {
                        "buf":  "df"
                }
        }
}
	response:
{
        "result":       {
                "buf":  "Filesystem           1K-blocks      Used Available Use% Mounted on\n/dev/root                38733     29662      7071  81% /\ntmpfs                   256436         4    256432   0% /dev\n/dev/mtdblock3            2048       244      1804  12% /etc/conf.d\n192.168.6.53:/home/zwq/nfs\n                      19737280  14455808   4278880  77% /progs\n"
        }
}

14.NVS_Cmd_Update
系统升级，nDevID，无意义
eg.
	send:
{
        "method":       "NVS_Cmd_Update",
        "params":       {
                "nDevID":       1,
                "stUpdate":        {
                        "addr":  "usb"
                }
        }
}
{
        "method":       "NVS_Cmd_Update",
        "params":       {
                "nDevID":       1,
                "stUpdate":        {
                        "addr":  "ftp://192.168.8.118"
                }
        }
}
{
        "method":       "NVS_Cmd_Update",
        "params":       {
                "nDevID":       1,
                "stUpdate":        {
                        "addr":  "http://www.jovetech.com"
                }
        }
}
	response:
{
        "result":       {
                "buf":  "Filesystem           1K-blocks      Used Available Use% Mounted on\n/dev/root                38733     29662      7071  81% /\ntmpfs                   256436         4    256432   0% /dev\n/dev/mtdblock3            2048       244      1804  12% /etc/conf.d\n192.168.6.53:/home/zwq/nfs\n                      19737280  14455808   4278880  77% /progs\n"
        }
}

15.NVS_Cmd_GetNetwork
获取网络参数，nDevID，无意义
会返回所有网络接口的配置。
eg.
	send:
{
        "method":       "NVS_Cmd_GetNetwork",
        "params":       {
                "nDevID":       -1,
                "nCh":  -1
        }
}
	response:

{
	"result":	[{
			"devName":	"eth1",
			"bDhcp":	0,
			"ip":	"192.168.6.122",
			"mask":	"255.255.255.0",
			"gw":	"192.168.6.1",
			"dns":	"192.168.6.1"
		}]
}

16.NVS_Cmd_SetNetwork
设置网络参数，nDevID，无意义
会返回所有网络接口的配置。
eg.
	send:
{
        "method":       "NVS_Cmd_SetNetwork",
        "params":       {
                "nDevID":       1,
                "stNetwork":        {
						"devName":	"eth1",
						"bDhcp":	0,
						"ip":	"192.168.6.122",
						"mask":	"255.255.255.0",
						"gw":	"192.168.6.1",
						"dns":	"192.168.6.1"
                }
		}
}
	response:
一般收不到回复
{
        "result":       0
}


17.NVS_Cmd_GetUser
获取用户配置，nDevID，无意义
会返回全部的用户配置。
eg.
	send:
{
        "method":       "NVS_Cmd_GetUser",
        "params":       {
                "nDevID":       -1,
                "nCh":  -1
        }
}
	response:
{
	"result":	[{
			"username":	"abc",
			"passwd":	"",
			"bLocal":	1,
			"pwRecord":	1,
			"pwSnap":	1,
			"pwNet":	1,
			"pwSys":	1,
			"pwPTZ":	1,
			"pwVideo":	1,
			"pwQuit":	1,
			"pwOther":	1,
			"pwVsMgr":	1,
			"pwView":	1
		}, {
			"username":	"abc",
			"passwd":	"123",
			"bLocal":	0,
			"pwRecord":	1,
			"pwSnap":	0,
			"pwNet":	0,
			"pwSys":	1,
			"pwPTZ":	1,
			"pwVideo":	0,
			"pwQuit":	0,
			"pwOther":	1,
			"pwVsMgr":	0,
			"pwView":	1
		}]
}

17.NVS_Cmd_SetUser
设置用户配置，nDevID，无意义。
如果用户已存在就修改，否则就是添加。
{
        "method":       "NVS_Cmd_SetUser",
        "params":       {
                "nDevID":       1,
                "stUser":        {
									"username":	"abc",
									"passwd":	"123",
									"bLocal":	1,
									"pwRecord":	1,
									"pwSnap":	0,
									"pwNet":	0,
									"pwSys":	1,
									"pwPTZ":	1,
									"pwVideo":	0,
									"pwQuit":	0,
									"pwOther":	1,
									"pwVsMgr":	0,
									"pwView":	1
								}
		}
}
	response:
{
	"result":	0
}

