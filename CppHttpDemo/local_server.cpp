#include "local_server.h"
#include "../httpclient/mongoose.h"

local_server::local_server()
{
	xmlConfig::gb28181Server = this;
	xmlConfig::readLocalServerNode(*this);
	xmlConfig::readVideoServerNodes(*this);
}


local_server::~local_server()
{

}

void local_server::gb28181ServerThread()
{
	eXosipInit();

	while (!m_bIsStop)
	{
		eXosip_event_t *je = NULL;
		je = eXosip_event_wait(eCtx, 0, 4);
		if (je == NULL)
		{
			osip_usleep(100000);
			continue;
		}

		switch (je->type)
		{
			case EXOSIP_MESSAGE_NEW:
			{
				osip_contact_t *co = NULL;
				co = (osip_contact_t *)osip_list_get(&je->request->contacts, 0);
				bool isFound = false;
				std::list<video_server>::iterator it;
				for (it = m_platformList.begin(); it != m_platformList.end(); it++)
				{
					if (co->url->username == it->m_strID)
					{
						if (MSG_IS_REGISTER(je->request))
						{
							it->isRegister = 1;
						}
						else if (MSG_IS_MESSAGE(je->request))
						{
							osip_body_t* body = NULL;
							osip_message_get_body(je->request, 0, &body);
							if (body != NULL)
							{
								std::string::size_type idx;
								std::string bodyStr = body->body;

								//保活消息
								idx = bodyStr.find("Keepalive");
								if (idx != std::string::npos)
								{
									if (!it->m_bKeepAlive)
									{
										//log
										it->m_bKeepAlive = true;
										it->isRegister = 1;
									}
								}

								//设备消息回复
								idx = bodyStr.find("Catalog");
								if (idx != std::string::npos)
								{
									it->xmlCatalog = bodyStr;
									LOG(INFO) << "获取设备目录信息成功，下级域ID：" << it->m_strID;
								}
							}
							else
							{
								//log 获取body失败
								LOG(INFO) << "获取设备目录信息BODY失败，下级域ID：" << it->m_strID;
							}
						}
						else if (strncmp(je->request->sip_method, "BYE", 4) != 0)
						{
							//log 不支持sip方法
						}
					}
				}
				return200OK(je);
			}
		break;
			case EXOSIP_MESSAGE_ANSWERED:
			{
				//log回答方法
				return200OK(je);
			}
		break;
			case EXOSIP_CALL_ANSWERED:
			{
				osip_message_t* ack = NULL;
				//osip_contact_t *co = NULL;
				//co = (osip_contact_t *)osip_list_get(&je->request->req_uri->, 0);
				std::list<video_server>::iterator it;
				for (it = m_platformList.begin(); it != m_platformList.end(); it++)
				{
					if (je->request->req_uri->host == it->m_strIP)
					{
						LOG(INFO) << "EXOSIP_CALL_ANSWERED je->cid：" << je->cid << " je->did：" << je->did;
						it->call_id = je->cid;
						it->dialog_id = je->did;
						eXosip_call_build_ack(eCtx, je->did, &ack);
						eXosip_lock(eCtx);
						eXosip_call_send_ack(eCtx, je->did, ack);
						eXosip_unlock(eCtx);

						std::list<camera_info>::iterator ct;
						for (ct = m_cameraList.begin();ct!=m_cameraList.end();ct++)
						{
							if (je->request->req_uri->username == ct->strCamId)
							{
								ct->m_ffmpeg->start();
								ct->m_rtpSocket->start();
								break;
							}
						}
						break;
					}
				}
				//log for call_id dialog_ id
			}
		break;
			default:
			{
				//test type je->type
				LOG(INFO) << "default return200OK(je)";
				return200OK(je);
			}
		}
		eXosip_event_free(je);
	}
}

void local_server::return200OK(eXosip_event_t* je)
{
	int iRet = 0;
	osip_message_t* pRegister = NULL;
	iRet = eXosip_message_build_answer(eCtx, je->tid, 200, &pRegister);
	if (iRet == 0 && pRegister!=NULL)
	{
		eXosip_lock(eCtx);
		eXosip_message_send_answer(eCtx, je->tid, 200, pRegister);
		eXosip_unlock(eCtx);
	}
}

int local_server::eXosipInit()
{
	int iRet = 0;

	//exosip初始化
	LOG(INFO) << "local_server::eXosipInit";

	eCtx = eXosip_malloc();
	if (iRet != OSIP_SUCCESS)
	{
		LOG(INFO) << "local_server::eXosipInit 失败，错误码：" << iRet;
		return iRet;
	}
	else
	{
		LOG(INFO) << "local_server::eXosipInit eXosip_malloc成功";
		iRet = eXosip_init(eCtx);
		if (iRet != 0)
		{
			LOG(INFO) << "local_server::eXosipInit eXosip_init失败";
			return iRet;
		}
	}

	//监听端口

	iRet = eXosip_listen_addr(eCtx, IPPROTO_UDP, NULL, m_port, AF_INET, 0);
	if (iRet != OSIP_SUCCESS)
	{
		LOG(INFO) << "local_server::eXosipInit eXosip_listen_addr失败，端口：" << m_port;
		return iRet;
	}
	LOG(INFO) << "local_server::eXosipInit eXosip_listen_addr成功，端口：" << m_port;
	//sendInvite("34000000001317006215", 6000);
}

int local_server::eXosipFree()
{
	eXosip_quit(eCtx);
	osip_free(eCtx);
	eCtx = NULL;
	return 0;
}

int local_server::sendInvite(const char* cameraId, const char* platformIP, int platformPort, int cameraPort)
{
	char destCall[256], srcCall[256], sub[128];
	//camera.iRecvPort = rtpPort;

	osip_message_t *invite = NULL;
	int iRet;

	snprintf(destCall, 256, "sip:%s@%s:%d", cameraId, platformIP, platformPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	snprintf(sub, 128, cameraId, m_strID.c_str());

	LOG(INFO) << "local_server::sendInvite，设备ID：" << cameraId;

	iRet = eXosip_call_build_initial_invite(eCtx, &invite, destCall, srcCall, NULL, sub);
	if (iRet != OSIP_SUCCESS)
	{
		LOG(INFO) << "local_server::sendInvite::eXosip_call_build_initial_invite失败，错误码：" << iRet;

		return iRet;
	}

	char body[2048];
	int bodyLen = snprintf(body,2048,
		"v=0\r\n"
		"o=%s 0 0 IN IP4 %s\r\n"
		"s=Play\r\n"
		"c=IN IP4 %s\r\n"
		"t=0 0\r\n"
		"m=video %d RTP/AVP 96 97 98\r\n"
		"a=rtpmap:96 PS/90000\r\n"
		"a=rtpmap:97 MPEG4/90000\r\n"
		"a=rtpmap:98 H264/90000\r\n"
		"a=recvonly\r\n", cameraId, m_ip.c_str(),
		m_ip.c_str(), cameraPort);
	osip_message_set_body(invite, body, bodyLen);
	osip_message_set_content_type(invite, "APPLICATION/SDP");
	eXosip_lock(eCtx);
	iRet = eXosip_call_send_initial_invite(eCtx, invite);
	eXosip_unlock(eCtx);

	LOG(INFO) << "local_server::sendInvite成功，返回码：" << iRet;
	return iRet;
}

int local_server::sendBye(int callId, int dialogId)
{
	eXosip_lock(eCtx);
	eXosip_call_terminate(eCtx, callId, dialogId);
	eXosip_unlock(eCtx);
	return 0;
}

int local_server::sendQueryCatalog(const char* platformID, int sn, const char* platformIP, int platformPort)
{
	std::string bufQuery = xmlConfig::buildQueryCmdXml(platformID, sn);

	LOG(INFO) << "local_server::sendQueryCatalog，平台ID：" << platformID;

	int iRet = 0;
	char destCall[256], srcCall[256];
	osip_message_t* queryCatalog = NULL;
	snprintf(destCall, 256, "sip:%s@%s:%d", platformID, platformIP, platformPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	iRet = eXosip_message_build_request(eCtx, &queryCatalog, "MESSAGE", destCall, srcCall, NULL);
	if (iRet == OSIP_SUCCESS && queryCatalog != NULL)
	{
		osip_message_set_body(queryCatalog, bufQuery.c_str(), bufQuery.length());
		osip_message_set_content_type(queryCatalog, "APPLICATION/MANSCDP+XML");
		eXosip_lock(eCtx);
		eXosip_message_send_request(eCtx, queryCatalog);
		eXosip_unlock(eCtx);

		LOG(INFO) << "local_server::sendQueryCatalog成功，返回码：" << iRet;
	}
	else
	{
		LOG(INFO) << "local_server::sendQueryCatalog失败，返回码：" << iRet;
	}
	return iRet;
}

int local_server::sendPTZCMD(const char* deviceID, const int sn, const char* ptzCode, const char* platformID, const char* platformIP, int platformPort)
{
	std::string bufPTZ = xmlConfig::builPTZControlXml(deviceID, sn, ptzCode);

	LOG(INFO) << "local_server::sendPTZCMD，设备ID：" << deviceID;

	int iRet = 0;
	char destCall[256], srcCall[256];
	osip_message_t* queryPTZ = NULL;
	snprintf(destCall, 256, "sip:%s@%s:%d", platformID, platformIP, platformPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	iRet = eXosip_message_build_request(eCtx, &queryPTZ, "MESSAGE", destCall, srcCall, NULL);
	if (iRet == OSIP_SUCCESS && queryPTZ != NULL)
	{
		osip_message_set_body(queryPTZ, bufPTZ.c_str(), bufPTZ.length());
		osip_message_set_content_type(queryPTZ, "APPLICATION/MANSCDP+XML");
		eXosip_lock(eCtx);
		eXosip_message_send_request(eCtx, queryPTZ);
		eXosip_unlock(eCtx);

		LOG(INFO) << "local_server::sendPTZCMD成功，返回码：" << iRet;
	}
	else
	{
		LOG(INFO) << "local_server::sendPTZCMD失败，返回码：" << iRet;
	}
	return iRet;
}

int local_server::sendPlayBack(const char* cameraId, const char* platformIP, int platformPort, int cameraPort, time_t startTime, time_t endTime)
{
	char destCall[256], srcCall[256], sub[128];

	LOG(INFO) << "local_server::sendPlayBack，设备ID：" << cameraId;

	osip_message_t *invite = NULL;
	int iRet;

	snprintf(destCall, 256, "sip:%s@%s:%d", cameraId, platformIP, platformPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	snprintf(sub, 128, cameraId, m_strID.c_str());

	iRet = eXosip_call_build_initial_invite(eCtx, &invite, destCall, srcCall, NULL, sub);
	if (iRet != OSIP_SUCCESS)
	{
		//log 创建请求失败
		LOG(INFO) << "local_server::sendPlayBack::eXosip_call_build_initial_invite失败，返回码：" << iRet;
		return iRet;
	}
	char body[2048];
	int bodyLen = snprintf(body, 2048,
		"v=0\r\n"
		"o=%s 0 0 IN IP4 %s\r\n"
		"s=Playback\r\n"
		"c=IN IP4 %s\r\n"
		"t=%lld %lld\r\n"
		"m=video %d RTP/AVP 96 97 98\r\n"
		"a=rtpmap:96 PS/90000\r\n"
		"a=rtpmap:97 MPEG4/90000\r\n"
		"a=rtpmap:98 H264/90000\r\n"
		"a=recvonly\r\n", cameraId, m_ip.c_str(),
		m_ip.c_str(), startTime, endTime, cameraPort);
	osip_message_set_body(invite, body, bodyLen);
	osip_message_set_content_type(invite, "APPLICATION/SDP");
	eXosip_lock(eCtx);
	iRet = eXosip_call_send_initial_invite(eCtx, invite);
	eXosip_unlock(eCtx);

	LOG(INFO) << "local_server::sendPlayBack成功，返回码：" << iRet;

	return iRet;
}

std::string local_server::getPTZCode(const int m_iSubCMD)
{
	std::string strPTZCMD;
	char str[10];
	int v = 0;
	int m_iSpeed = 200, m_iAddress = 0;
	BYTE PTZCMD[8] = { 0, };
	PTZCMD[0] = 0xA5;
	PTZCMD[1] = (v << 4) + ((PTZCMD[0] >> 4) + (PTZCMD[0] & 0xF) + (v & 0xF)) & 0xF;
	PTZCMD[2] = (m_iAddress & 0xFF);
	switch (m_iSubCMD)
	{
	case SUB_UP:
		PTZCMD[3] = 0x08;
		PTZCMD[5] = (BYTE)m_iSpeed;
		break;
	case SUB_DOWN:
		PTZCMD[3] = 0x04;
		PTZCMD[5] = (BYTE)m_iSpeed;
		break;
	case SUB_LEFT:
		PTZCMD[3] = 0x02;
		PTZCMD[4] = (BYTE)m_iSpeed;
		break;
	case SUB_RIGHT:
		PTZCMD[3] = 0x01;
		PTZCMD[4] = (BYTE)m_iSpeed;
		break;
	case SUB_ZOOMIN:
		PTZCMD[3] = 0x10;
		PTZCMD[6] = (BYTE)(m_iSpeed << 4);
		break;
	case SUB_ZOOMOUT:
		PTZCMD[3] = 0x20;
		PTZCMD[6] = (BYTE)(m_iSpeed << 4);
		break;
	default:
		break;
	}
	int i;
	int sum = 0;
	for (i = 0; i < 7; i++)
	{
		sum += PTZCMD[i];
	}
	PTZCMD[i] = (sum & 0xFF);
	for (i = 0; i < 8; i++)
	{
		//str.Format("%02X", PTZCMD[i]);
		sprintf(str,"%02X",PTZCMD[i]);
		strPTZCMD += str;
	}
	return strPTZCMD;
}

void local_server::start()
{
	m_bIsStop = false;

	LOG(INFO) << "local_server::start 线程开始运行...";

	std::thread([&](local_server *pointer)
	{
		pointer->gb28181ServerThread();

	}, this).detach();

}

void local_server::stop()
{
	m_bIsStop = true;

	LOG(INFO) << "local_server::stop 线程停止";

	while (m_bIsThreadRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	eXosipFree();
}

