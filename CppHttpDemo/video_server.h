#pragma once
#include "all.h"
class video_server
{
public:
	video_server();
	video_server(const std::string& m_strRealm, const std::string& m_strID, const std::string& m_strIP, const std::string& m_strPassword, int m_iPort, int m_iExpires, int m_iHeartBeat, int m_iKeepAliveInterval, bool m_bKeepAlive, bool m_bReceiveRTCP);
	~video_server();
	std::string m_strRealm;	//��
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	int m_iPort;	//�˿�
	std::string m_strPassword;	//����
	int m_iExpires;	//ע��ʱ��
	int m_iHeartBeat;	//��������

	int m_iKeepAliveInterval;	//������
	bool m_bKeepAlive;	//����
	bool m_bReceiveRTCP;	//����RTCP
};

