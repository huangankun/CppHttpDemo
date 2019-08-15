#include "local_server.h"
#include "../httpclient/mongoose.h"

local_server::local_server()
{
	url = "rtmp://127.0.0.1:19350/live";
	platformServer.m_strRealm = "34000000";
	platformServer.m_strPassword = "123456";
	platformServer.m_strIP = "127.0.0.1";
	platformServer.m_strID = "34032301051315041603";
	platformServer.m_iPort = 16001;
	platformServer.m_iKeepAliveInterval = 30000;
	platformServer.m_iHeartBeat = 30;
	platformServer.m_iExpires = 3600;
	platformServer.m_bReceiveRTCP = true;
	platformServer.m_bKeepAlive = true;
	char body[2048];
	snprintf(body, 2048, "v=0\r\n"
		"o=%s 0 0 IN IP4 %s\r\n"
		"s=Play\r\n"
		"c=IN IP4 %s\r\n"
		"t=0 0\r\n"
		"m=video %d RTP/AVP 96 97 98\r\n"
		"a=rtpmap:96 PS/90000\r\n"
		"a=rtpmap:97 MPEG4/90000\r\n"
		"a=rtpmap:98 H264/90000\r\n"
		"a=recvonly\r\n", "1", "127.0.0.1", "127.0.0.1", 6666);
	sdp = body;
}


local_server::~local_server()
{
}

void local_server::gb28181ServerThread()
{
	eXosipInit();
	char *pBuf;
	int keepAliveFlag = 0;

	while (true)
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
			if (MSG_IS_REGISTER(je->request))
			{
				isRegister = 1;
			}
			else if (MSG_IS_MESSAGE(je->request))
			{
				osip_body_t* body = NULL;
				osip_message_get_body(je->request, 0, &body);
				if (body != NULL)
				{
					pBuf = strstr(body->body, "KeepAlive");
					if (pBuf != NULL)
					{
						if (keepAliveFlag == 0)
						{
							//log
							keepAliveFlag = 1;
							isRegister = 1;
						}
					}
					else
					{
						//log
					}
				}
				else
				{
					//log 获取body失败
				}
			}
			else if (strncmp(je->request->sip_method, "BYE", 4) != 0)
			{
				//log 不支持sip方法
			}
			registerSuccess(je);
		}
		break;
		case EXOSIP_MESSAGE_ANSWERED:
		{
			//log回答方法
			registerSuccess(je);
		}
		break;
		case EXOSIP_CALL_ANSWERED:
		{
			osip_message_t* ack = NULL;
			call_id = je->cid;
			dialog_id = je->did;
			//log for call_id dialog_ id
			eXosip_call_build_ack(eCtx, je->did, &ack);
			eXosip_lock(eCtx);
			eXosip_call_send_ack(eCtx, je->did, ack);
			eXosip_unlock(eCtx);
			startToRtmp();
		}
		break;
		default:
		{
			//test type je->type
			registerSuccess(je);
		}
		}
		eXosip_event_free(je);
	}
}

void local_server::gb28181ToRtmpThread()
{
	ffmpegInit();
	if (openInputSdp(sdp.c_str()) < 0)
	{
		//Open file input failed
		std::this_thread::sleep_for(std::chrono::seconds(1000));
		return;
	}
	else
	{
		if (openOutput(url.c_str()) >= 0)
		{
			while (true)
			{
				std::shared_ptr<AVPacket> packet = nullptr;
				packet = readPacketFromSource();
				if (packet)
				{
					if (writePacket(packet) >= 0)
					{
						//write packet success
					}
					else
					{
						//write packet failed
					}
				}
				else
				{
					return;
				}
			}
		}
	}
}

void local_server::ffmpegInit()
{
	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_ERROR);
}

int local_server::openInputSdp(const char* sdp)
{
	context = avformat_alloc_context();
	AVDictionary* dicts = NULL;
	AVInputFormat* ifmt = av_find_input_format("sdp");

	int ret = 0;
	ret = av_dict_set(&dicts, "protocol_whitelist", "file,udp,rtp", 0);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}
	unsigned char* strSdp = (unsigned char*)(this->sdp.c_str());

	AVIOContext* avio = avio_alloc_context(strSdp, this->sdp.length(), 0, (void *)NULL, NULL, NULL, NULL);
	context->pb = avio;
	ret = avformat_open_input(&context, "nothing", ifmt, &dicts);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}

	ret = avformat_find_stream_info(context, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}

	int vidx = 0, aidx = 0;
	auto codecContext = context->streams[0]->codec;
	ret = avcodec_open2(codecContext, avcodec_find_decoder(codecContext->codec_id), nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}
	return ret;
}

int local_server::openOutput(const char* url)
{
	int ret = 0;
	ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", url);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}

	ret = avio_open2(&outputContext->pb, url, AVIO_FLAG_READ_WRITE, nullptr, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}

	for (int i = 0; i < context->nb_streams; i++)
	{
		AVStream* stream = avformat_new_stream(outputContext, nullptr);
		ret = avcodec_copy_context(stream->codec, context->streams[i]->codec);
		if (ret < 0)
		{
			char errStr[256];
			av_strerror(ret, errStr, 256);
			printf(errStr);
			return ret;
		}
	}

	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0)
	{
		char errStr[256];
		av_strerror(ret, errStr, 256);
		printf(errStr);
		return ret;
	}

	return ret;
}

int local_server::writePacket(std::shared_ptr<AVPacket> packet)
{
	auto inputStream = context->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
	return av_interleaved_write_frame(outputContext, packet.get());
}

std::shared_ptr<AVPacket> local_server::readPacketFromSource()
{
	std::shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket *p) { av_free_packet(p); av_freep(&p); });
	av_init_packet(packet.get());
	int ret = av_read_frame(context, packet.get());
	if (ret >= 0)
	{
		return packet;
	}
	else
	{
		return nullptr;
	}
}

int local_server::initUDPsocket(int port, struct sockaddr_in *serverAddr, char* mcastAddr)
{
	int err = -1;
	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		//log 建立socket失败，端口号
		return -1;
	}

	memset(serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);

	err = bind(socket_fd, (struct sockaddr*)serverAddr, sizeof(struct sockaddr));
	if (err < 0)
	{
		//bind失败
		return -2;
	}
	
	//组播回送
	const char loop = 1;
	err = setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	if (err < 0)
	{
		//组播回送失败
		return -3;
	}
	return socket_fd;
}

void local_server::rtcpThread()
{
	int socket_fd;
	int rtcp_port = 1935 + 1;
	struct  sockaddr_in serverAddr;
	
	socket_fd = initUDPsocket(rtcp_port, &serverAddr, NULL);
	if (socket_fd >= 0)
	{
		//socket创建成功
	}

	char* buf = (char*)malloc(1024);
	if (buf == NULL)
	{
		//申请控件失败
	}

	int recvLen;
	int addrLen = sizeof(struct sockaddr);
	//rtcp 开始接收...
	memset(buf, 0, 1024);
	while (true)
	{
		recvLen = recvfrom(socket_fd, buf, 1024, 0, (struct sockaddr*)&serverAddr, (socklen_t*)&addrLen);
		if (recvLen > 0)
		{
			recvLen = sendto(socket_fd, buf, recvLen, 0, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
			if (recvLen <= 0)
			{
				//发送失败
			}
		}
		else
		{
			//接收失败
		}
	}
	releaseUDPsocket(socket_fd);
	if (buf != NULL)
	{
		free(buf);
	}
	//rtcp线程结束
}

void local_server::releaseUDPsocket(int socket_fd)
{
	_close(socket_fd);
}

void local_server::registerSuccess(eXosip_event_t* je)
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
	eCtx = eXosip_malloc();
	if (iRet != OSIP_SUCCESS)
	{
		//log
		return iRet;
	}
	else
	{
		//return iRet;
		//log
		iRet = eXosip_init(eCtx);
		if (iRet != 0)
		{
			printf("Can't initialize eXosip!\n");
			return iRet;
		}
	}

	//监听端口
	iRet = eXosip_listen_addr(eCtx, IPPROTO_UDP, NULL, m_port, AF_INET, 0);
	if (iRet != OSIP_SUCCESS)
	{
		return iRet;
	}
}

int local_server::eXosipFree()
{
	eXosip_quit(eCtx);
	osip_free(eCtx);
	eCtx = NULL;
	return 0;
}

int local_server::sendInvite(const char* cameraId, const int rtpPort)
{
	char destCall[256], srcCall[256], sub[128];
	camera.iRecvPort = rtpPort;

	osip_message_t *invite = NULL;
	int iRet;

	snprintf(destCall, 256, "sip:%s@%s:%d", cameraId, platformServer.m_strIP.c_str(), platformServer.m_iPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	snprintf(sub, 128, cameraId, m_strID.c_str());

	iRet = eXosip_call_build_initial_invite(eCtx, &invite, destCall, srcCall, NULL, sub);
	if (iRet != OSIP_SUCCESS)
	{
		//log 创建请求失败
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
		m_ip.c_str(), rtpPort);
	sdp = body;
	osip_message_set_body(invite, body, bodyLen);
	osip_message_set_content_type(invite, "APPLICATION/SDP");
	eXosip_lock(eCtx);
	iRet = eXosip_call_send_initial_invite(eCtx, invite);
	eXosip_unlock(eCtx);
	return iRet;
}

int local_server::sendBye()
{
	eXosip_lock(eCtx);
	eXosip_call_terminate(eCtx, call_id, dialog_id);
	eXosip_unlock(eCtx);
	return 0;
}

int local_server::sendQueryCatalog()
{
	std::string bufQuery = xmlConfig::buildQueryCmdXml(platformServer.m_strID.c_str(), m_SN);

	int iRet = 0;
	char destCall[256], srcCall[256];
	osip_message_t* queryCatalog = NULL;
	snprintf(destCall, 256, "sip:%s@%s:%d", platformServer.m_strID.c_str(), platformServer.m_strIP.c_str(), platformServer.m_iPort);
	snprintf(srcCall, 256, "sip:%s@%s", m_strID.c_str(), m_ip.c_str());
	iRet = eXosip_message_build_request(eCtx, &queryCatalog, "MESSAGE", destCall, srcCall, NULL);
	if (iRet == OSIP_SUCCESS && queryCatalog != NULL)
	{
		osip_message_set_body(queryCatalog, bufQuery.c_str(), bufQuery.length());
		osip_message_set_content_type(queryCatalog, "APPLICATION/MANSCDP+XML");
		eXosip_lock(eCtx);
		eXosip_message_send_request(eCtx, queryCatalog);
		eXosip_unlock(eCtx);
		m_SN++;
		//log 发送设备查询信令成功
	}
	else
	{
		//log 发送设备查询信令失败
	}
	return iRet;
}

void local_server::start()
{
	localThreads[0] = std::thread(&local_server::gb28181ServerThread, this);
	localThreads[0].detach();
}

void local_server::startToRtmp()
{
	localThreads[1] = std::thread(&local_server::gb28181ToRtmpThread, this);
	localThreads[2] = std::thread(&local_server::rtcpThread, this);
	localThreads[1].join();
	localThreads[2].join();
}

