#pragma once
#include "video_server.h"
#include "camera_info.h"
#include "xmlConfig.h"
#include "all.h"

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
	int m_SN;
	bool m_bAuthentication;	//鉴权
	int call_id;
	int dialog_id;
	int isRegister;
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
	void registerSuccess(eXosip_event_t* je);
	int eXosipInit();
	int eXosipFree();
	int sendInvite(const char* cameraId, const int rtpPort);
	int sendBye();
	int sendQueryCatalog();
};

