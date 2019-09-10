#pragma once
#include "all.h"
#include "http_server.h"
#include "local_server.h"
INITIALIZE_EASYLOGGINGPP

// 初始化HttpServer静态类成员
std::string xmlConfig::strWorkPath = xmlConfig::getWorkDir();
std::string xmlConfig::configPath = xmlConfig::strWorkPath + "\\config.xml";
mg_serve_http_opts HttpServer::s_server_option;
std::string HttpServer::s_web_dir = "./web";
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;
std::unordered_set<mg_connection *> HttpServer::s_websocket_session_set;
local_server testServer;

bool handle_stop_gb_to_rtmp(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth 解析json请求，判断现有的目录中是否有对应的相机然后发送invite并启动rtmp线程，返回相应http请求。

	cJSON* bodyJson = cJSON_Parse(body.c_str());
	cJSON* paramJson = cJSON_GetObjectItemCaseSensitive(bodyJson, "userparam");
	cJSON* usernameJson = cJSON_GetObjectItemCaseSensitive(paramJson, "username");
	cJSON* deviceIDJson = cJSON_GetObjectItemCaseSensitive(paramJson, "deviceid");
	cJSON* paramJson2 = cJSON_GetObjectItemCaseSensitive(bodyJson, "parameter");
	cJSON* urlJson = cJSON_GetObjectItemCaseSensitive(paramJson2, "url");
	cJSON* portJson = cJSON_GetObjectItemCaseSensitive(paramJson2, "port");

	std::string deviceid = deviceIDJson->valuestring;
	std::string username = usernameJson->valuestring;
	std::string urlRtmp = urlJson->valuestring;
	int portRecv = portJson->valueint;

	std::list<video_server>::iterator vt;
	std::list<camera_info>::iterator ca;
	bool isExist = false;
	for (vt = testServer.m_platformList.begin(); vt != testServer.m_platformList.end(); vt++)
	{
		if (username == vt->m_strID)
		{
			for (ca = testServer.m_cameraList.begin(); ca != testServer.m_cameraList.end(); ca++)
			{
				if (ca->strCamId == deviceid)
				{
					isExist = true;
					testServer.sendBye(ca->call_id, ca->dialog_id);
					ca->m_ffmpeg->stop();
					ca->m_rtpSocket->stop();
					testServer.m_cameraList.erase(ca);
					break;
				}
			}
		}
	}

	//testServer.sendInvite("34032301051315041603", 6666);
	cJSON *monitor = cJSON_CreateObject();
	if (isExist)
	{
		LOG(INFO) << "Stop gb to rtmp, deviceid: " << deviceid;
		cJSON *code = cJSON_CreateNumber(1);
		cJSON *message = cJSON_CreateString("succeed");
		cJSON *dataUrl = cJSON_CreateObject();
		cJSON *urlJson2 = cJSON_CreateString(urlRtmp.c_str());
		cJSON_AddItemToObject(dataUrl, "url", urlJson2);
		cJSON_AddItemToObject(monitor, "code", code);
		cJSON_AddItemToObject(monitor, "message", message);
		cJSON_AddItemToObject(monitor, "data", dataUrl);
	}
	else
	{
		LOG(INFO) << "deviceid: " << deviceid<<"does not exist";
		cJSON *code = cJSON_CreateNumber(0);
		cJSON *message = cJSON_CreateString("not exist");
		cJSON_AddItemToObject(monitor, "code", code);
		cJSON_AddItemToObject(monitor, "message", message);
	}
	char *strTmp = cJSON_Print(monitor);

	rsp_callback(c, strTmp);

	return true;
}

bool handle_start_gb_to_rtmp(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth 解析json请求，判断现有的目录中是否有对应的相机然后发送invite并启动rtmp线程，返回相应http请求。

	cJSON* bodyJson = cJSON_Parse(body.c_str());
	cJSON* paramJson = cJSON_GetObjectItemCaseSensitive(bodyJson, "userparam");
	cJSON* usernameJson = cJSON_GetObjectItemCaseSensitive(paramJson, "username");
	cJSON* deviceIDJson = cJSON_GetObjectItemCaseSensitive(paramJson, "deviceid");
	cJSON* paramJson2 = cJSON_GetObjectItemCaseSensitive(bodyJson, "parameter");
	cJSON* urlJson = cJSON_GetObjectItemCaseSensitive(paramJson2, "url");
	cJSON* portJson = cJSON_GetObjectItemCaseSensitive(paramJson2, "port");

	std::string deviceid = deviceIDJson->valuestring;
	std::string username = usernameJson->valuestring;
	std::string urlRtmp = urlJson->valuestring;
	int portRecv = portJson->valueint;

	std::list<video_server>::iterator vt;
	for (vt = testServer.m_platformList.begin(); vt != testServer.m_platformList.end(); vt++)
	{
		if (username == vt->m_strID)
		{
			testServer.sendInvite(deviceid.c_str(), vt->m_strIP.c_str(), vt->m_iPort, portRecv);
			camera_info myCamera;
			myCamera.iRecvPort = portRecv;
			myCamera.m_ffmpeg = new ffmpeg_to_web();
			myCamera.m_ffmpeg->m_bSaveJPEG = false;
			myCamera.m_ffmpeg->m_bIsRunning = true;
			myCamera.m_ffmpeg->m_fileExt = "flv";
			myCamera.m_ffmpeg->m_strUrl = urlRtmp;
			int udpPort = 30000 + portRecv;
			char bufUdp[80];
			snprintf(bufUdp, 80, "udp://127.0.0.1:%d?buffer_size=655360", udpPort);
			myCamera.m_ffmpeg->m_strFileName = bufUdp;
			myCamera.m_rtpSocket = new rtp_socket();
			myCamera.m_rtpSocket->m_bSaveVideo = false;
			myCamera.m_rtpSocket->m_iPort = portRecv;
			myCamera.running = false;
			myCamera.strCamId = deviceid;
			testServer.m_cameraList.push_back(myCamera);
		}
	}

	//testServer.sendInvite("34032301051315041603", 6666);
	cJSON *monitor = cJSON_CreateObject();
	cJSON *code = cJSON_CreateNumber(1);
	cJSON *message = cJSON_CreateString("succeed");
	cJSON *dataUrl = cJSON_CreateObject();
	cJSON *urlJson2 = cJSON_CreateString(urlRtmp.c_str());
	cJSON_AddItemToObject(dataUrl, "url", urlJson2);
	cJSON_AddItemToObject(monitor, "code", code);
	cJSON_AddItemToObject(monitor, "message", message);
	cJSON_AddItemToObject(monitor, "data", dataUrl);
	char *strTmp = cJSON_Print(monitor);

	rsp_callback(c, strTmp);

	return true;
}

int main(int argc, char *argv[])
{
	LOG(INFO) << "***** Program startup *****";
	LOG(INFO) << "Program configuration file directory：" << xmlConfig::configPath;

	auto http_server = std::shared_ptr<HttpServer>(new HttpServer);
	http_server->Init();
	testServer.start();

	// add handler
	http_server->AddHandler("/startGb2Rtmp", handle_start_gb_to_rtmp);
	http_server->AddHandler("/stopGb2Rtmp", handle_stop_gb_to_rtmp);
	http_server->Start();
	LOG(INFO) << "***** End of program *****";
}