#pragma once
#include "all.h"
#include "http_server.h"
#include "local_server.h"
INITIALIZE_EASYLOGGINGPP

// Initialize HttpServer static class members
std::string xmlConfig::strWorkPath = xmlConfig::getWorkDir();
std::string xmlConfig::configPath = xmlConfig::strWorkPath + "\\config.xml";
mg_serve_http_opts HttpServer::s_server_option;
std::string HttpServer::s_web_dir = "./web";
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;
std::unordered_set<mg_connection *> HttpServer::s_websocket_session_set;
local_server testServer;


bool is_exist_platform(const std::string platform_id, int & sn, std::string & ip, int & port, std::string & catalog)
{
	bool isExist = false;
	std::list<video_server>::iterator va;
	for (va = testServer.m_platformList.begin(); va != testServer.m_platformList.end(); va++)
	{
		if (va->m_strID == platform_id)
		{
			va->m_SN++;
			sn = va->m_SN;
			ip = va->m_strIP;
			port = va->m_iPort;
			catalog = va->xmlCatalog;
			isExist = true;
			break;
		}
	}
	return isExist;
}

bool is_exist_device_in_catalog(const std::string deviceId,const std::string platform_id)
{
	bool isExist = false;
	std::list<video_server>::iterator va;
	std::string::size_type idx;
	for (va = testServer.m_platformList.begin(); va != testServer.m_platformList.end(); va++)
	{
		idx = va->xmlCatalog.find(deviceId);
		if (idx != std::string::npos)
		{
			isExist = true;
			break;
		}
	}
	return isExist;
}

bool is_exist_on_live(const std::string deviceId, bool isNeedToStop, std::string &url)
{
	bool isExist = false;
	std::list<camera_info>::iterator ca;
	for (ca = testServer.m_cameraList.begin(); ca != testServer.m_cameraList.end(); ca++)
	{
		if (ca->strCamId == deviceId)
		{
			isExist = true;
			url = ca->m_ffmpeg->m_strUrl;
			if (isNeedToStop)
			{
				testServer.sendBye(ca->call_id, ca->dialog_id);
				ca->m_ffmpeg->stop();
				ca->m_rtpSocket->stop();
				testServer.m_cameraList.erase(ca);
			}
			break;
		}
	}
	return isExist;
}

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
	bool isExist = false;

	//Determine if it is already playing and stop
	isExist = is_exist_on_live(deviceid, true, urlRtmp);

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
	// Do sth parses the json request, determines whether there is a corresponding camera in the existing directory, 
	//then sends the invite and starts the rtmp thread, and returns the corresponding http request.

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
	bool isLive = false;
	
	isLive = is_exist_on_live(deviceid, false, urlRtmp);
	if (!isLive)
	{
		std::list<video_server>::iterator vt;
		for (vt = testServer.m_platformList.begin(); vt != testServer.m_platformList.end(); vt++)
		{
			//目前还不能获取到摄像机的设备列表，所以不做是否存在设备的判断；否则应该先判断设备列表是否存在设备后再进行点播消息
			//if (is_exist_device_in_catalog(deviceid,username))
			//{
			//}
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
				break;
			}
		}
	}


	cJSON *monitor = cJSON_CreateObject();
	cJSON *code = cJSON_CreateNumber(1);
	cJSON *message;
	if (isLive)
	{
		message = cJSON_CreateString("the camera is playing");
	}
	else
	{
		message = cJSON_CreateString("succeed");
	}
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

bool handle_start_ptz_control(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	cJSON *bodyJson = cJSON_Parse(body.c_str());
	cJSON *paramJson = cJSON_GetObjectItemCaseSensitive(bodyJson, "userparam");
	cJSON* usernameJson = cJSON_GetObjectItemCaseSensitive(paramJson, "username");
	cJSON *deviceIDJson = cJSON_GetObjectItemCaseSensitive(paramJson, "deviceID");
	cJSON *directionJson = cJSON_GetObjectItemCaseSensitive(paramJson, "direction");

	std::string username = usernameJson->valuestring;
	std::string deviceId = deviceIDJson->valuestring;
	std::string ip = "";
	std::string catalog = "";
	int direction = directionJson->valueint;
	int sn = 0;
	int port = 0;
	bool isExist = is_exist_platform(username, sn, ip, port, catalog);

	cJSON *monitor = cJSON_CreateObject();
	cJSON *code;
	cJSON *message;
	
	if (isExist)
	{
		testServer.sendPTZCMD(deviceId.c_str(), sn, direction, username.c_str(), ip.c_str(), port);
		message = cJSON_CreateString("succeed");
		code = cJSON_CreateNumber(1);
	}
	else
	{
		code = cJSON_CreateNumber(0);
		message = cJSON_CreateString("the platform is not exist");
	}
	char *strTmp = cJSON_Print(monitor);
	cJSON_AddItemToObject(monitor, "code", code);
	cJSON_AddItemToObject(monitor, "message", message);

	rsp_callback(c, strTmp);
	return true;
}

bool handle_get_catalog(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	cJSON *bodyJson = cJSON_Parse(body.c_str());
	cJSON *paramJson = cJSON_GetObjectItemCaseSensitive(bodyJson, "userparam");
	cJSON* usernameJson = cJSON_GetObjectItemCaseSensitive(paramJson, "username");
	std::string platformId = usernameJson->valuestring;
	std::string ip = "";
	std::string catalog = "";
	int sn;
	int port;
	bool isExist = false;
	cJSON *monitor = cJSON_CreateObject();
	cJSON *code;
	cJSON *message;

	isExist = is_exist_platform(platformId, sn, ip, port, catalog);
	if (isExist)
	{
		code = cJSON_CreateNumber(1);
		message = cJSON_CreateString(catalog.c_str());
	}
	else
	{
		code = cJSON_CreateNumber(0);
		message = cJSON_CreateString("the platform is not exist");
	}
	char *strTmp = cJSON_Print(monitor);
	cJSON_AddItemToObject(monitor, "code", code);
	cJSON_AddItemToObject(monitor, "message", message);

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
	http_server->AddHandler("/startPtzControl", handle_start_ptz_control);
	http_server->Start();
	LOG(INFO) << "***** End of program *****";
}