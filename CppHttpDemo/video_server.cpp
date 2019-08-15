#include "video_server.h"


video_server::video_server()
{

}

video_server::video_server(const std::string& m_strRealm, const std::string& m_strID, const std::string& m_strIP, const std::string& m_strPassword, int m_iPort, int m_iExpires, int m_iHeartBeat, int m_iKeepAliveInterval, bool m_bKeepAlive, bool m_bReceiveRTCP)
{
	this->m_bKeepAlive = m_bKeepAlive;
	this->m_bReceiveRTCP = m_bReceiveRTCP;
	this->m_iExpires = m_iExpires;
	this->m_iHeartBeat = m_iHeartBeat;
	this->m_iKeepAliveInterval = m_iKeepAliveInterval;
	this->m_iPort = m_iPort;
	this->m_strRealm = m_strRealm;
	this->m_strID = m_strID;
	this->m_strIP = m_strIP;
	this->m_strPassword = m_strPassword;
}


video_server::~video_server()
{
}
