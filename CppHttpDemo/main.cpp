#pragma once
#include "all.h"
#include "http_server.h"
#include "local_server.h"

// ��ʼ��HttpServer��̬���Ա
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
			testServer.sendInvite("35080224001310330351", 6000);
		//}
	//}

	//testServer.sendInvite("34032301051315041603", 6666);
	cJSON *monitor = cJSON_CreateObject();
	cJSON *code = cJSON_CreateNumber(1);
	cJSON *message = cJSON_CreateString("succeed");
	cJSON *dataUrl = cJSON_CreateObject();
	cJSON *urlJson = cJSON_CreateString("rtmp://ip:port/live/1111111");
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

	rsp_callback(c, "rsp2");

	return true;
}

int main(int argc, char *argv[])
{
	std::string strWorkPath = xmlConfig::getWorkDir();
	std::string configPath = strWorkPath + "\\config.xml";
	std::string port = "7999";
	auto http_server = std::shared_ptr<HttpServer>(new HttpServer);
	local_server testServer;
	http_server->Init(port);
	xmlConfig::readLocalServerNode(configPath.c_str(), testServer);
	testServer.start();
	// add handler
	http_server->AddHandler("/realvideo", handle_fun1);
	http_server->AddHandler("/api/fun2", handle_fun2);
	http_server->Start();
	return 0;
}