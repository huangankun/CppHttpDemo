#pragma once
#include "mongoose.h"
#include "cJSON.h"
#include "all.h"

// 定义http返回callback
typedef void OnRspCallback(mg_connection *c, std::string);

// 定义http请求handler
using ReqHandler = std::function<bool (std::string, std::string, mg_connection *c, OnRspCallback)>;

class HttpServer
{
public:
	HttpServer() {}

	~HttpServer() {}

	void Init(); // Initialization settings

	bool Start(); // start http server

	bool Close(); // close http server

	void AddHandler(const std::string &url, ReqHandler req_handler); // Register event handler

	void RemoveHandler(const std::string &url); // Remove time handler

	static std::string s_web_dir; // Web root directory 

	static mg_serve_http_opts s_server_option; // Web server option

	static std::unordered_map<std::string, ReqHandler> s_handler_map; // Callback function mapping table

	std::string m_port;    // port

	std::string m_ip;		//ip
private:
	// Static event response function
	static void OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data);

	static void HandleHttpEvent(mg_connection *connection, http_message *http_req);

	static void SendHttpRsp(mg_connection *connection, std::string rsp);

	static int isWebsocket(const mg_connection *connection); // Determine if it is a websoket type connection

	static void HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg); //Respond to websocket messages

	static void SendWebsocketMsg(mg_connection *connection, std::string msg); // Send a message to the specified connection

	static void BroadcastWebsocketMsg(std::string msg); // Broadcast messages to all connections

	static std::unordered_set<mg_connection *> s_websocket_session_set; // Cache websocket connection

	mg_mgr m_mgr;          // Connection manager
};

