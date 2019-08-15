#pragma once
#include "all.h"
class video_server
{
public:
	video_server();
	video_server(const std::string& m_strRealm, const std::string& m_strID, const std::string& m_strIP, const std::string& m_strPassword, int m_iPort, int m_iExpires, int m_iHeartBeat, int m_iKeepAliveInterval, bool m_bKeepAlive, bool m_bReceiveRTCP);
	~video_server();
	std::string m_strRealm;	//域
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	int m_iPort;	//端口
	std::string m_strPassword;	//密码
	int m_iExpires;	//注册时间
	int m_iHeartBeat;	//心跳周期

	int m_iKeepAliveInterval;	//保活间隔
	bool m_bKeepAlive;	//保活
	bool m_bReceiveRTCP;	//接收RTCP
};

