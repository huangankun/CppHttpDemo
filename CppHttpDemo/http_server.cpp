
#include "http_server.h"
#include "xmlConfig.h"

void HttpServer::Init()
{
	xmlConfig::readHttpServerNode(*this);
	s_server_option.enable_directory_listing = "yes";
	s_server_option.document_root = s_web_dir.c_str();


	// Other http settings
	// Open CORS, this item is only valid for home page loading.
	// s_server_option.extra_headers = "Access-Control-Allow-Origin: *";
}

bool HttpServer::Start()
{
	LOG(INFO) << "HttpServer::Start thread start ";

	mg_mgr_init(&m_mgr, NULL);
	mg_connection *connection = mg_bind(&m_mgr, m_port.c_str(), HttpServer::OnHttpWebsocketEvent);
	if (connection == NULL)
	{
		LOG(INFO) << "HttpServer::Start bing port fail��" << m_port;
		return false;
	}
	// for both http and websocket
	mg_set_protocol_http_websocket(connection);

	LOG(INFO) << "HttpServer::Start listen http request��port��" << m_port;

	// loop
	while (true)
		mg_mgr_poll(&m_mgr, 500); // ms

	LOG(INFO) << "http_server stop";
	return true;
}

void HttpServer::OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data)
{
	// Differentiate between http and websocket
	if (event_type == MG_EV_HTTP_REQUEST)
	{
		http_message *http_req = (http_message *)event_data;
		HandleHttpEvent(connection, http_req);
	}
	else if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE ||
		     event_type == MG_EV_WEBSOCKET_FRAME ||
		     event_type == MG_EV_CLOSE)
	{
		websocket_message *ws_message = (struct websocket_message *)event_data;
		HandleWebsocketMessage(connection, event_type, ws_message);
	}
}

// ---- simple http ---- //
static bool route_check(http_message *http_msg, char *route_prefix)
{
	if (mg_vcmp(&http_msg->uri, route_prefix) == 0 && mg_vcmp(&http_msg->method, "POST") == 0)
		return true;
	else
		return false;

	// TODO: Can judge GET, POST, PUT, DELTE and other methods
	//mg_vcmp(&http_msg->method, "GET");
	//mg_vcmp(&http_msg->method, "POST");
	//mg_vcmp(&http_msg->method, "PUT");
	//mg_vcmp(&http_msg->method, "DELETE");
}

void HttpServer::AddHandler(const std::string &url, ReqHandler req_handler)
{
	if (s_handler_map.find(url) != s_handler_map.end())
		return;

	s_handler_map.insert(std::make_pair(url, req_handler));
}

void HttpServer::RemoveHandler(const std::string &url)
{
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
		s_handler_map.erase(it);
}

void HttpServer::SendHttpRsp(mg_connection *connection, std::string rsp)
{
	// --- CORS is not turned on
	// The header must be sent first, and HTTP/2.0 cannot be used yet.
	mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	// Return in json form
	mg_printf_http_chunk(connection, "%s", rsp.c_str());
	// Send blank characters fast, end current response
	mg_send_http_chunk(connection, "", 0);

	// --- Open CORS
	/*mg_printf(connection, "HTTP/1.1 200 OK\r\n"
			  "Content-Type: text/plain\n"
			  "Cache-Control: no-cache\n"
			  "Content-Length: %d\n"
			  "Access-Control-Allow-Origin: *\n\n"
			  "%s\n", rsp.length(), rsp.c_str()); */
}

void HttpServer::HandleHttpEvent(mg_connection *connection, http_message *http_req)
{
	std::string req_str = std::string(http_req->message.p, http_req->message.len);

	LOG(INFO) << "HttpServer::HandleHttpEvent get HTTP request��" << req_str;

	// First filter whether the registered function callback
	std::string url = std::string(http_req->uri.p, http_req->uri.len);
	std::string body = std::string(http_req->body.p, http_req->body.len);
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
	{
		ReqHandler handle_func = it->second;
		handle_func(url, body, connection, &HttpServer::SendHttpRsp);
		return;
	}

	// other request
	//if (route_check(http_req, "/")) // index page
	//	mg_serve_http(connection, http_req, s_server_option);
	//else if (route_check(http_req, "/api/return")) 
	//{
	//	// Direct return

	//	SendHttpRsp(connection, body);
	//}
	//else if (route_check(http_req, "/api/sum"))
	//{
	//	// ��post���󣬼ӷ��������
	//	char n1[100], n2[100];
	//	double result;

	//	/* Get form variables */
	//	mg_get_http_var(&http_req->body, "n1", n1, sizeof(n1));
	//	mg_get_http_var(&http_req->body, "n2", n2, sizeof(n2));

	//	/* Compute the result and send it back as a JSON object */
	//	result = strtod(n1, NULL) + strtod(n2, NULL);
	//	SendHttpRsp(connection, std::to_string(result));
	//}
	//else
	//{
	//	mg_printf(
	//		connection,
	//		"%s",
	//		"HTTP/1.1 501 Not Implemented\r\n" 
	//		"Content-Length: 0\r\n\r\n");
	//}
}

// ---- websocket ---- //
int HttpServer::isWebsocket(const mg_connection *connection)
{
	return connection->flags & MG_F_IS_WEBSOCKET;
}

void HttpServer::HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg)
{
	if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE)
	{
		printf("client websocket connected\n");

		LOG(INFO) << "HttpServer::HandleWebsocketMessage WebSocket handshake succeeded��";

		// Get the IP and port of the connected client
		char addr[32];
		mg_sock_addr_to_str(&connection->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

		LOG(INFO) << "HttpServer::HandleWebsocketMessage WebSocket address��"<<addr;

		// add session
		s_websocket_session_set.insert(connection);

		SendWebsocketMsg(connection, "client websocket connected");

		LOG(INFO) << "HttpServer::HandleWebsocketMessage SendWebsocketMsg";
	}
	else if (event_type == MG_EV_WEBSOCKET_FRAME)
	{
		mg_str received_msg = {
			(char *)ws_msg->data, ws_msg->size
		};

		char buff[1024] = {0};
		strncpy(buff, received_msg.p, received_msg.len); // must use strncpy, specifiy memory pointer and length

		// do sth to process request
		LOG(INFO) << "HttpServer::HandleWebsocketMessage get message��" << buff;

		SendWebsocketMsg(connection, "send your msg back: " + std::string(buff));
		//BroadcastWebsocketMsg("broadcast msg: " + std::string(buff));
	}
	else if (event_type == MG_EV_CLOSE)
	{
		if (isWebsocket(connection))
		{
			LOG(INFO) << "HttpServer::HandleWebsocketMessage close WebSocket��";
			// �Ƴ�session
			if (s_websocket_session_set.find(connection) != s_websocket_session_set.end())
				s_websocket_session_set.erase(connection);
		}
	}
}

void HttpServer::SendWebsocketMsg(mg_connection *connection, std::string msg)
{
	mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

void HttpServer::BroadcastWebsocketMsg(std::string msg)
{
	for (mg_connection *connection : s_websocket_session_set)
		mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

bool HttpServer::Close()
{
	LOG(INFO) << "http_server exit()";
	mg_mgr_free(&m_mgr);
	return true;
}