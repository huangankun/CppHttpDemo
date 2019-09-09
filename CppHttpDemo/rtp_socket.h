#pragma once
#include "all.h"
typedef struct CallInfo
{
	int m_iRTPPort;

	SOCKADDR_IN Uaddr;
	int USocket;
	SOCKADDR_IN Raddr;
	int RSocket;
	SOCKADDR_IN Caddr;
	int CSocket;

	int m_PT;
	int m_SN;
	DWORD m_TS;
	DWORD m_SSRC;
	DWORD m_CurSSRC;

	DWORD m_dwDuration;



	DWORD m_dwReceiveTime;
	DWORD m_dwReceivedLen;
	BOOL m_bExitThread;

	//NewQueue* recvqueue;
} CALL_INFO_ST;

class PACKET
{
public:
	PACKET()
	{
		m_SN = 0;
		m_TS = 0;
		m_len = 0;
		m_buf = NULL;
	}

	int m_SN;
	DWORD m_TS;
	int m_len;
	char* m_buf;
};

class rtp_socket
{
public:
	rtp_socket();
	~rtp_socket();
	int m_iPort;
	bool m_bSaveVideo;	//保存码流标识位
	int InitSendSocket(const char *ip, int port);
	void start();
	CALL_INFO_ST *m_pCallInfo;
	void stop(bool isWait = false);
protected:
	void run();
private:
	void sMediaReceiverProc();
	void sRTCPProc();
};

