#pragma once
#include "xmlConfig.h"
#include "all.h"
#include "rtp_socket.h"
#include "ffmpeg_to_web.h"

class ffmpeg_to_web;
class rtp_socket;

#define SUB_UP 0
#define SUB_DOWN 1
#define SUB_LEFT 2
#define SUB_RIGHT 3
#define SUB_ZOOMIN 4
#define SUB_ZOOMOUT 5

struct camera_info
{
	std::string strCamId;
	int iRecvPort;								//media receive port
	int status;
	int statusErrCnt;
	int running;
	int call_id;	
	int dialog_id;	
	rtp_socket *m_rtpSocket;
	ffmpeg_to_web *m_ffmpeg;
};

struct video_server
{
	std::string m_strRealm;	//real
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	std::string m_strPassword;	
	std::string xmlCatalog;		//video catalog

	int m_iPort;
	int m_iExpires;
	int m_iHeartBeat;
	int m_iKeepAliveInterval;

	bool m_bKeepAlive;
	bool m_bReceiveRTCP;
	int isRegister;
	int m_SN;
	int call_id;
	int dialog_id;
};

class local_server
{
public:
	local_server();
	~local_server();

	bool m_bIsStop;          //�Ƿ��յ�ֹͣ����

	std::list<camera_info> m_cameraList;	//���ڲ��ŵ�����б�
	std::list<video_server> m_platformList;		//�¼���������б�
	struct eXosip_t *eCtx;

	int m_videoNum;	//��Ƶ·��
	int m_port;    // �˿�
	std::string m_ip;		//��ַ
	std::string m_protocol;	//Э�飺TCP��UDP
	std::string m_strID;			//������ID
	std::string m_realm;		//��
	std::string m_password;	//����			
	bool m_bAuthentication;	//��Ȩ


	void start();
	void stop();
	void gb28181ServerThread();
	void return200OK(eXosip_event_t* je);
	int eXosipInit();
	int eXosipFree();
	int sendInvite(const char* cameraId, const char* platformIP, int platformPort, int cameraPort);
	int sendBye(int callId, int dialogId);
	int sendQueryCatalog(const char* platformID, int sn, const char* platformIP, int platformPort);
	int sendPTZCMD(const char* deviceID, const int sn, const char* ptzCode, const char* platformID, const char* platformIP, int platformPort);
	int sendPlayBack(const char* cameraId, const char* platformIP, int platformPort, int cameraPort, time_t startTime, time_t endTime);
	std::string getPTZCode(const int m_iSubCMD);
};

