#pragma once
#include "all.h"
#include "http_server.h"
#include "local_server.h"
INITIALIZE_EASYLOGGINGPP

// ��ʼ��HttpServer��̬���Ա
std::string xmlConfig::strWorkPath = xmlConfig::getWorkDir();
std::string xmlConfig::configPath = xmlConfig::strWorkPath + "\\config.xml";
mg_serve_http_opts HttpServer::s_server_option;
std::string HttpServer::s_web_dir = "./web";
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;
std::unordered_set<mg_connection *> HttpServer::s_websocket_session_set;
local_server testServer;

bool handle_fun1(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth ����json�����ж����е�Ŀ¼���Ƿ��ж�Ӧ�����Ȼ����invite������rtmp�̣߳�������Ӧhttp����

	/*cJSON* bodyJson = cJSON_Parse(body.c_str());
	cJSON* paramJson = cJSON_GetObjectItemCaseSensitive(bodyJson, "userparam");
	cJSON* usernameJson = cJSON_GetObjectItemCaseSensitive(paramJson, "username");
	cJSON* deviceIDJson = cJSON_GetObjectItemCaseSensitive(paramJson, "deviceid");
	std::string deviceid = deviceIDJson->valuestring;
	std::string username = usernameJson->valuestring;
	if (username == testServer.platformServer.m_strID)
	{*/
		//if (testServer.platformServer.xmlCatalog.find(deviceid) != std::string::npos)
		//{
			//testServer.sendInvite("34000000001317006215", 6000);
		//}
	//}

	//testServer.sendInvite("34032301051315041603", 6666);
	cJSON *monitor = cJSON_CreateObject();
	cJSON *code = cJSON_CreateNumber(1);
	cJSON *message = cJSON_CreateString("succeed");
	cJSON *dataUrl = cJSON_CreateObject();
	cJSON *urlJson = cJSON_CreateString("rtmp://127.0.0.1:1935/live");
	cJSON_AddItemToObject(dataUrl, "url", urlJson);
	cJSON_AddItemToObject(monitor, "code", code);
	cJSON_AddItemToObject(monitor, "message", message);
	cJSON_AddItemToObject(monitor, "data", dataUrl);
	char *strTmp = cJSON_Print(monitor);

	rsp_callback(c, strTmp);

	return true;
}

bool handle_fun2(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth
	std::cout << "handle fun2" << std::endl;
	std::cout << "url: " << url << std::endl;
	std::cout << "body: " << body << std::endl;
	//testServer.sendQueryCatalog("35080000002000000128", 5000, "112.111.229.121", 7100);
	testServer.sendInvite("35080224001310129637", "112.111.229.121", 7100, 6000);
	time_t endTm = time(0);
	time_t startTm = endTm - 10000;
	//testServer.sendPlayBack("34000000001317006215", "117.71.25.9", 7100, 6000, startTm, endTm);
	camera_info myCamera;
	myCamera.iRecvPort = 6000;
	myCamera.m_ffmpeg = new ffmpeg_to_web();
	myCamera.m_ffmpeg->m_bSaveJPEG = true;
	myCamera.m_ffmpeg->m_bIsRunning = true;
	myCamera.m_ffmpeg->m_fileExt = "flv";
	myCamera.m_ffmpeg->m_strUrl = "rtmp://127.0.0.1:1935/live";
	myCamera.m_ffmpeg->m_strFileName = "udp://127.0.0.1:36000?buffer_size=655360";
	myCamera.m_rtpSocket = new rtp_socket();
	myCamera.m_rtpSocket->m_bSaveVideo = false;
	myCamera.m_rtpSocket->m_iPort = 6000;
	myCamera.running = false;
	myCamera.strCamId = "35080224001310129637";
	//myCamera.m_rtpSocket->start();
	//myCamera.m_ffmpeg->start();
	testServer.m_cameraList.push_back(myCamera);
	rsp_callback(c, "rsp2");

	return true;
}

int main(int argc, char *argv[])
{
	LOG(INFO) << "***** �������� *****";
	LOG(INFO) << "���������ļ�Ŀ¼��" << xmlConfig::configPath;

	auto http_server = std::shared_ptr<HttpServer>(new HttpServer);
	http_server->Init();
	testServer.start();

	// add handler
	http_server->AddHandler("/realvideo", handle_fun1);
	http_server->AddHandler("/api/fun2", handle_fun2);
	http_server->Start();
	LOG(INFO) << "***** ������� *****";
}