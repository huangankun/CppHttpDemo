#pragma once
#include "xmlConfig.h"
#include "all.h"

struct camera_info
{
	std::string strCamId;
	int iRecvPort;								//��Ӧý����ն˶˿�
	int status;
	int statusErrCnt;
	int running;
};

struct video_server
{
	std::string m_strRealm;	//��
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	std::string m_strPassword;	//����

	int m_iPort;	//�˿�
	int m_iExpires;	//ע��ʱ��
	int m_iHeartBeat;	//��������
	int m_iKeepAliveInterval;	//������

	bool m_bKeepAlive;	//����
	bool m_bReceiveRTCP;	//����RTCP
	std::string xmlCatalog;
	int call_id;
	int dialog_id;
	int isRegister;
	int m_SN;
};

class local_server
{
public:
	local_server();
	~local_server();

	AVFormatContext* outputContext;
	AVFormatContext* context;


	camera_info camera;
	video_server platformServer;
	struct eXosip_t *eCtx;


	int m_port;    // �˿�
	std::string m_ip;		//��ַ
	std::string m_protocol;	//Э�飺TCP��UDP
	std::string m_strID;			//������ID
	std::string m_realm;		//��
	std::string m_password;	//����
	std::string url;					
	std::string sdp;
	bool m_bAuthentication;	//��Ȩ
	std::thread localThreads[3];

	void start();
	void stop();
	void startToRtmp();
	void stopToRtmp();
	int initUDPsocket(int port, struct sockaddr_in *serverAddr, char* mcastAddr);
	void releaseUDPsocket(int socket_fd);
	void gb28181ServerThread();
	void rtcpThread();
	void gb28181ToRtmpThread();
	void ffmpegInit();
	int openInputSdp(const char* sdp);
	int openOutput(const char* url);
	std::shared_ptr<AVPacket> readPacketFromSource();
	int writePacket(std::shared_ptr<AVPacket> packet);
	void return200OK(eXosip_event_t* je);
	int eXosipInit();
	int eXosipFree();
	int sendInvite(const char* cameraId, const int rtpPort);
	int sendBye();
	int sendQueryCatalog();
};

