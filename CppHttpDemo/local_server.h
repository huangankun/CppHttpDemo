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
	int iRecvPort;								//对应媒体接收端端口
	int status;
	int statusErrCnt;
	int running;
	rtp_socket *m_rtpSocket;
	ffmpeg_to_web *m_ffmpeg;
};

struct video_server
{
	std::string m_strRealm;	//域
	std::string m_strID;	//ID
	std::string m_strIP;	//IP
	std::string m_strPassword;	//密码
	std::string xmlCatalog;		//目录缓存

	int m_iPort;	//端口
	int m_iExpires;	//注册时间
	int m_iHeartBeat;	//心跳周期
	int m_iKeepAliveInterval;	//保活间隔

	bool m_bKeepAlive;			//保活
	bool m_bReceiveRTCP;	//接收RTCP
	int call_id;	//连接ID
	int dialog_id;	//会话ID
	int isRegister;	//判断是否是注册请求
	int m_SN;		//信令计数器，没法送一次控制类型信令自增1
};

class local_server
{
public:
	local_server();
	~local_server();

	bool m_bIsStop;          //是否收到停止命令
	bool m_bIsThreadRunning; //现在是否在运行

	std::list<camera_info> m_cameraList;	//正在播放的相机列表
	std::list<video_server> m_platformList;		//下级域服务器列表
	struct eXosip_t *eCtx;

	int m_videoNum;	//视频路数
	int m_port;    // 端口
	std::string m_ip;		//地址
	std::string m_protocol;	//协议：TCP和UDP
	std::string m_strID;			//服务器ID
	std::string m_realm;		//域
	std::string m_password;	//密码			
	bool m_bAuthentication;	//鉴权


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
	int sendPlayBack(const char* cameraId, const char* platformIP, int platformPort, int cameraPort, std::string startTime, std::string endTime);
	std::string getPTZCode(const int m_iSubCMD);
};

