#pragma once
#include "xmlConfig.h"
#include "all.h"

struct camera_info
{
	std::string strCamId;
	int iRecvPort;								//对应媒体接收端端口
	int status;
	int statusErrCnt;
	int running;
};

struct video_server
{
	std::string m_strRealm;	//域
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	std::string m_strPassword;	//密码

	int m_iPort;	//端口
	int m_iExpires;	//注册时间
	int m_iHeartBeat;	//心跳周期
	int m_iKeepAliveInterval;	//保活间隔

	bool m_bKeepAlive;	//保活
	bool m_bReceiveRTCP;	//接收RTCP
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


	int m_port;    // 端口
	std::string m_ip;		//地址
	std::string m_protocol;	//协议：TCP和UDP
	std::string m_strID;			//服务器ID
	std::string m_realm;		//域
	std::string m_password;	//密码
	std::string url;					
	std::string sdp;
	bool m_bAuthentication;	//鉴权
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

