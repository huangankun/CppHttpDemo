#include "rtp_socket.h"
#include "xmlConfig.h"


rtp_socket::rtp_socket()
{
	//m_pCallInfo->m_bExitThread = FALSE;
	m_bSaveVideo = false;
}


rtp_socket::~rtp_socket()
{
}

int rtp_socket::InitSendSocket(const char * ip, int port, CALL_INFO_ST *& item)
{
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	int iRet = WSAStartup(wVersionRequested, &wsaData);
	if (0 != iRet) {
		LOG(INFO) << " rtp_socket::InitSendSocket::WSAStartup, error code" << iRet;
		return -1;
	}

	int iSendSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 >= iSendSocket) {
		LOG(INFO) << " rtp_socket::InitSendSocket::socket, error code" << iSendSocket;
		return -2;
	}

	memset(&item->Uaddr, 0, sizeof(SOCKADDR_IN));

	item->Uaddr.sin_family = AF_INET;
	item->Uaddr.sin_addr.S_un.S_addr = inet_addr(ip);
	item->Uaddr.sin_port = htons(port);

	return iSendSocket;
}

void rtp_socket::start()
{
	//m_pCallInfo->m_bExitThread = FALSE;
	this->run();
}

void rtp_socket::stop(bool isWait)
{
	m_pCallInfo->m_bExitThread = TRUE;


	if (m_pCallInfo != NULL)
	{
		m_pCallInfo->m_bExitThread = TRUE;
		if (m_pCallInfo->CSocket != INVALID_SOCKET)
		{
			shutdown(m_pCallInfo->CSocket, SD_BOTH);
			closesocket(m_pCallInfo->CSocket);
		}
		if (m_pCallInfo->RSocket != INVALID_SOCKET)
		{
			shutdown(m_pCallInfo->RSocket, SD_BOTH);
			closesocket(m_pCallInfo->RSocket);
		}
		if (m_pCallInfo->USocket != INVALID_SOCKET)
		{
			shutdown(m_pCallInfo->USocket, SD_BOTH);
			closesocket(m_pCallInfo->USocket);
		}
		m_pCallInfo->m_bExitThread = FALSE;
		delete m_pCallInfo;
		m_pCallInfo = NULL;
	}
}

void rtp_socket::run()
{
	//UDP Init
	m_pCallInfo = new CALL_INFO_ST();
	memset(m_pCallInfo, 0, sizeof(CALL_INFO_ST));
	//m_pCallInfo->recvqueue = &recvqueue;

	m_pCallInfo->m_iRTPPort = m_iPort;
	m_pCallInfo->m_bExitThread = FALSE;
	int USocket = InitSendSocket((char *)"127.0.0.1", m_iPort + 30000, m_pCallInfo);

	LOG(INFO) << " rtp_socket::run::InitSendSocket, 端口号: " << 30000+m_iPort;

	if (0 >= USocket) {
		delete m_pCallInfo;
	}

	//RTP Init
	UINT uID;
	int RSocket = 0;
	RSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (RSocket == INVALID_SOCKET)
	{
		return;
	}
	m_pCallInfo->Raddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_pCallInfo->Raddr.sin_family = AF_INET;
	m_pCallInfo->Raddr.sin_port = htons(m_iPort);
	if (bind(RSocket, (SOCKADDR*)&(m_pCallInfo->Raddr), sizeof(SOCKADDR)) < 0)
	{
		closesocket(RSocket);
		//OutputDebugPrintf("bind %d error", port);
		LOG(INFO) << " rtp_socket::run::bind失败, 端口号: " <<m_iPort;
		return;
	}

	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	DWORD status;

	LOG(INFO) << " rtp_socket::run::bind成功, 端口号: " << m_iPort;

	int recv_buf_size = 10 * 1024 * 1024;
	::setsockopt(RSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recv_buf_size, sizeof(int));

	//RTCP
	int CSocket = 0;
	CSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (CSocket == INVALID_SOCKET)
	{

		LOG(INFO) << " rtp_socket::run::socket RTCP申请socket失败 ";

		closesocket(RSocket);
		return;
	}
	m_pCallInfo->Caddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	m_pCallInfo->Caddr.sin_family = AF_INET;
	m_pCallInfo->Caddr.sin_port = htons(m_iPort + 1);

	if (bind(CSocket, (SOCKADDR*)&(m_pCallInfo->Caddr), sizeof(SOCKADDR)) < 0)
	{

		LOG(INFO) << " rtp_socket::run::bind RTCP绑定端口： " << m_iPort + 1 << "失败";

		closesocket(RSocket);
		closesocket(CSocket);
		return;
	}

	m_pCallInfo->USocket = USocket;
	m_pCallInfo->RSocket = RSocket;
	m_pCallInfo->CSocket = CSocket;

	LOG(INFO) << " rtp_socket::run::bind RTCP绑定端口： " << m_iPort + 1 << "成功";

	if (CSocket)
	{

		LOG(INFO) << " rtp_socket::run::sRTCPProc RTCP接收线程启动";

		std::thread([&](rtp_socket *pointer)
		{
			pointer->sRTCPProc();

		}, this).detach();
	}

	std::thread([&](rtp_socket *pointer)
	{
		LOG(INFO) << " rtp_socket::run::sMediaReceiverProc 视频流接收线程启动";

		pointer->sMediaReceiverProc();

	}, this).detach();
}

void rtp_socket::sMediaReceiverProc()
{
	CALL_INFO_ST *pItem = m_pCallInfo;
	LONGLONG pts = 0;
	FILE *fp = NULL;
	FILE *fpRecord = NULL;
	int len = 0;
	DWORD dwFirstTS = -1;
	DWORD dwFirstTick = 0;
	BYTE buf[4096 * 64];
	int size = 4096 * 64;
	SOCKADDR_IN addrClient;
	int sock_addr_size = sizeof(SOCKADDR);

	std::list<PACKET> PacketArray;
	BOOL firstFlag = TRUE;
	int minSN = -1;
	int maxSN = -1;

	int MaxQueueSize = 1000;
	long num = 0;
	long prevTime = 0;

	LOG(INFO) << " rtp_socket::run::sMediaReceiverProc 进入接收数据循环之前";

	while (!pItem->m_bExitThread)
	{
		prevTime = pItem->m_TS;

		len = recvfrom(pItem->RSocket, (char *)buf, size, 0, (SOCKADDR*)&addrClient, &sock_addr_size);
		if (len < 0)
		{
			break;
		}
		num++;
		if (len < 12)
		{
			continue;
		}

		USHORT port = (pItem->Uaddr).sin_port;
		char* chIP = inet_ntoa(addrClient.sin_addr);
		
		LOG(INFO) << " rtp_socket::run::sMediaReceiverProc RTP客户端 IP地址："<<chIP<<" 端口号："<<addrClient.sin_port;

		pItem->m_PT = buf[1] & 0x7F;
		pItem->m_SN = htons((*(unsigned short *)(buf + 2)));
		pItem->m_TS = htonl((*(unsigned long *)(buf + 4)));
		pItem->m_SSRC = htonl((*(unsigned long *)(buf + 8)));

		if (dwFirstTS == -1)
		{
			dwFirstTS = pItem->m_TS;
		}

		if (pItem->m_CurSSRC == -1)
		{
			pItem->m_CurSSRC = pItem->m_SSRC;
		}
		PACKET one;
		one.m_SN = pItem->m_SN;
		one.m_TS = pItem->m_TS;
		one.m_len = len - 12;
		one.m_buf = (char *)malloc(len - 12);
		memcpy(one.m_buf, buf + 12, len - 12);
		PacketArray.push_back(one);

		if (m_bSaveVideo)
		{
			if (!fp)
			{
				std::string fileName;
				fileName = xmlConfig::strWorkPath + "\\video";
				if (0 != _access(fileName.c_str(), 0))
					if (0 != _mkdir(fileName.c_str()))
					{
						LOG(INFO) << "创建文件夹失败，文件夹路径：" << fileName; 				  // 返回 0 表示创建成功，-1 表示失败
						return;
					}
				fileName = fileName + "\\" + xmlConfig::getCurrentTime() + ".ps";
				fp = fopen(fileName.c_str(), "wb");
			}
			if (fp)
				fwrite(one.m_buf, 1, len - 12, fp);
		}
		else
		{
			if (fp)
			{
				fclose(fp);
			}
		}
		LOG(DEBUG) << " rtp_socket::run::sMediaReceiverProc RAW add SN= " << one.m_SN;

		if (minSN == -1)
		{
			minSN = one.m_SN;
			maxSN = one.m_SN;
		}
		else
		{
			minSN = min(minSN, one.m_SN);
			maxSN = max(maxSN, one.m_SN);
		}

		if (firstFlag)
		{
			//最初发包，缓存100个之后，找到最小的SN
			if (PacketArray.size() > 100)
			{
				firstFlag = FALSE;
			}
		}

		if (!firstFlag)
		{
			int count = PacketArray.size();
			if (count >= MaxQueueSize)
			{
				//队列满，意味着minSN没有被找到，重新寻找最小的minSN
				minSN = -1;
				maxSN = -1;
				std::list<PACKET>::iterator it;
				for (it = PacketArray.begin(); it != PacketArray.end(); it++)
				{
					if (minSN == -1)
					{
						minSN = it->m_SN;
						maxSN = it->m_SN;
					}
					else
					{
						minSN = min(minSN, it->m_SN);
						maxSN = max(maxSN, it->m_SN);
					}
				}
			}

			BOOL found;
			for (int sn = minSN; sn <= maxSN; sn++)
			{
				found = FALSE;
				std::list<PACKET>::iterator it;
				for (it = PacketArray.begin(); it != PacketArray.end(); it++)
				{
					if (it->m_SN == sn)
					{
						sendto(pItem->USocket, it->m_buf, it->m_len, 0, (SOCKADDR *)&(pItem->Uaddr), sizeof(SOCKADDR));
						found = TRUE;

						LOG(DEBUG) << " rtp_socket::run::sMediaReceiverProc sendBuf长度：" << it->m_len << " SN：" << sn;

						free(it->m_buf);
						it = PacketArray.erase(it);
						minSN = sn + 1;
						break;
					}
				}
				if (!found)
				{
					break;
				}
			}
		}
	}
	if (fp)
	{
		fclose(fp);
	}

	LOG(INFO) << " rtp_socket::run::sMediaReceiverProc 线程退出";
}

void rtp_socket::sRTCPProc()
{
	CALL_INFO_ST *pItem = m_pCallInfo;
	int len;
	int retryCount = 0;
	BYTE buf[4096 * 64];
	int size = 4096 * 64;
	SOCKADDR_IN addrClient;
	int sock_addr_size = sizeof(SOCKADDR_IN);

	LOG(INFO) << " rtp_socket::run::sRTCPProc进入接收数据循环之前";

	while (!pItem->m_bExitThread)
	{
		len = -1;
		retryCount = 0;
		for (int i = 0; i < 10; i++)
		{

			LOG(DEBUG) << " rtp_socket::run::sRTCPProc recvfrom之前";

			len = recvfrom(pItem->CSocket, (char *)buf, size, 0, (SOCKADDR*)&addrClient, &sock_addr_size);

			LOG(DEBUG) << " rtp_socket::run::sRTCPProc recvfrom之后，接收长度：" << len;

			if (len <= 0)
			{
				int errorCode = WSAGetLastError();
				retryCount += 1;

				LOG(DEBUG) << " rtp_socket::run::sRTCPProc recvfrom出错，接收端口：" << pItem->m_iRTPPort + 1 << " 错误码：" << errorCode << " 重新接收次数：" << retryCount;

				if (errorCode != WSAECONNRESET) //忽略掉winsock 的一个bug
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}
				continue;
			}

			LOG(DEBUG) << " rtp_socket::run::sRTCPProc break ten loop";

			break;
		}
		if (len < 0)
		{

			LOG(INFO) << " rtp_socket::run::sRTCPProc 接收出错，已经尝试十次重新接收，线程终止";

			break;
		}
		int rtcp_typeTmp = buf[1];

		LOG(DEBUG) << " rtp_socket::run::sRTCPProc rtcp_type = " << rtcp_typeTmp;

		if (buf[1] >= 200)
		{
			int rtcp_type = buf[1];

			if (rtcp_type == 200 || rtcp_type == 202)
			{
				unsigned long ssrc = htonl((*(unsigned long *)(buf + 4)));
				DWORD mySSRC = 0x290F123E;
				BYTE send_data[1024];

				send_data[0] = 0x80;
				send_data[1] = 201;
				send_data[2] = 0x00;
				send_data[3] = 0x01;
				//修复以前RTCP 包中SSRC为固定值 ，未取RTCP包中SSRC值
				send_data[4] = ssrc >> 24;
				send_data[5] = (ssrc >> 16) & 255;
				send_data[6] = (ssrc >> 8) & 255;
				send_data[7] = ssrc & 255;

				static const char CNAME[] = "(none)";
				int sdes_len = (strlen(CNAME) + 2 + 3) / 4 + 1;
				send_data[8] = 0x81;
				send_data[9] = 202;
				send_data[10] = 0x00;
				send_data[11] = sdes_len;

				send_data[12] = ssrc >> 24;
				send_data[13] = (ssrc >> 16) & 255;
				send_data[14] = (ssrc >> 8) & 255;
				send_data[15] = ssrc & 255;

				send_data[16] = 0x01;//CName type
				send_data[17] = strlen(CNAME);
				send_data[18] = CNAME[0];
				send_data[19] = CNAME[1];
				send_data[20] = CNAME[2];
				send_data[21] = CNAME[3];
				send_data[22] = CNAME[4];
				send_data[23] = CNAME[5];
				char* chIP = inet_ntoa(addrClient.sin_addr);

				LOG(DEBUG) << " rtp_socket::run::sRTCPProc 端口：" << pItem->m_iRTPPort + 1 << " 发送IP地址：" << chIP << " 发送端口：" << addrClient.sin_port;

				//修复bug，解决RTCP发送Receive回复包通过RTP Socket发送
				sendto(pItem->CSocket, (char *)send_data, 24, 0, (SOCKADDR *)&addrClient, sock_addr_size);
			}
			continue;
		}
	}
	LOG(INFO) << " rtp_socket::run::sRTCPProc 端口：" << pItem->m_iRTPPort + 1 << " 线程退出标识：" << m_pCallInfo->m_bExitThread << "接收数据尝试次数：" << retryCount;
}
