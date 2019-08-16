#include "xmlConfig.h"


xmlConfig::xmlConfig()
{
}


xmlConfig::~xmlConfig()
{
}

//************************************
// Method:    getWorkDir 获取程序工作路径
// FullName:  xmlConfig::getWorkDir
// Access:    public static 
// Returns:   const char*
// Qualifier:
//************************************
const char* xmlConfig::getWorkDir()
{
	char *buffer;
	//也可以将buffer作为输出参数
	if ((buffer = _getcwd(NULL, 0)) == NULL)
	{
		perror("getcwd error");
		return NULL;
	}
	else
	{
		printf("%s\n", buffer);
		return buffer;
	}
}


//************************************
// Method:    createXML 创建xml文件
// FullName:  xmlConfig::createXML
// Access:    public static 
// Returns:   int 成功返回0，失败返回错误码
// Qualifier:
// Parameter: const char * xmlPath 文件路径含文件名
//************************************
int xmlConfig::createXML(const char* xmlPath) 
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"gb2312\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);//会覆盖xml所有内容

	//添加申明可以使用如下两行
	//XMLDeclaration* declaration=doc.NewDeclaration();
	//doc.InsertFirstChild(declaration);

	XMLElement* root = doc.NewElement("配置项");
	doc.InsertEndChild(root);

	XMLElement* localNode = doc.NewElement("本地服务器");
	root->InsertEndChild(localNode);

	XMLElement* videoNode = doc.NewElement("下级域");
	root->InsertEndChild(videoNode);

	XMLElement* nginxNode = doc.NewElement("Nginx服务器");
	root->InsertEndChild(nginxNode);

	return doc.SaveFile(xmlPath);
}


//************************************
// Method:    insertVideoServerNode 插入视频服务器结点
// FullName:  xmlConfig::insertVideoServerNode
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: const char * xmlPath 文件路径含文件名
// Parameter: const local_server & local_server 视频服务器信息
//************************************
int xmlConfig::insertVideoServerNode(const char* xmlPath, const local_server& local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//加载文件失败
	if (bRet != 0)
	{
		//日志输出
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* serversNode = root->FirstChildElement("下级域");

	XMLElement* serverNode = doc.NewElement("下级服务器");
	serverNode->SetAttribute("ID", local_server.platformServer.m_strID.c_str());
	serverNode->SetAttribute("域", local_server.platformServer.m_strRealm.c_str());
	serversNode->InsertEndChild(serverNode);

	XMLElement* serverIP = doc.NewElement("IP");
	XMLText* ipText = doc.NewText(local_server.platformServer.m_strIP.c_str());
	serverIP->InsertEndChild(ipText);
	serverNode->InsertEndChild(serverIP);

	XMLElement* serverPWD = doc.NewElement("密码");
	XMLText* PWDText = doc.NewText(local_server.platformServer.m_strPassword.c_str());
	serverPWD->InsertEndChild(PWDText);
	serverNode->InsertEndChild(serverPWD);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("端口");
	_itoa(local_server.platformServer.m_iPort, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	serverNode->InsertEndChild(serverPort);

	XMLElement* serverExpires = doc.NewElement("注册时间");
	_itoa(local_server.platformServer.m_iExpires, intBuf, 10);
	XMLText* expiresText = doc.NewText(intBuf);
	serverExpires->InsertEndChild(expiresText);
	serverNode->InsertEndChild(serverExpires);

	XMLElement* serverHeartBeat = doc.NewElement("心跳周期");
	_itoa(local_server.platformServer.m_iHeartBeat, intBuf, 10);
	XMLText* heartBeatText = doc.NewText(intBuf);
	serverHeartBeat->InsertEndChild(heartBeatText);
	serverNode->InsertEndChild(serverHeartBeat);

	XMLElement* serverKeepAlive = doc.NewElement("保活");
	_itoa(local_server.platformServer.m_bKeepAlive, intBuf, 10);
	XMLText* keepAliveText = doc.NewText(intBuf);
	serverKeepAlive->InsertEndChild(keepAliveText);
	serverNode->InsertEndChild(serverKeepAlive);

	XMLElement* serverKeepAliveInterval = doc.NewElement("保活间隔");
	_itoa(local_server.platformServer.m_iKeepAliveInterval, intBuf, 10);
	XMLText* keepAliveIntervalText = doc.NewText(intBuf);
	serverKeepAliveInterval->InsertEndChild(keepAliveIntervalText);
	serverNode->InsertEndChild(serverKeepAliveInterval);

	XMLElement* serverReceiveRTCP = doc.NewElement("RTCP");
	_itoa(local_server.platformServer.m_bReceiveRTCP, intBuf, 10);
	XMLText* receiveRTCPText = doc.NewText(intBuf);
	serverReceiveRTCP->InsertEndChild(receiveRTCPText);
	serverNode->InsertEndChild(serverReceiveRTCP);

	return doc.SaveFile(xmlPath);
}


//************************************
// Method:    insertHttpServerNode 插入本地服务器结点
// FullName:  xmlConfig::insertHttpServerNode
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: const char * xmlPath 文件路径含文件名
// Parameter: const local_server & local_server 视频服务器信息
//************************************
int xmlConfig::insertHttpServerNode(const char* xmlPath, const local_server& local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//加载文件失败
	if (bRet != 0)
	{
		//日志输出
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* localNode = root->FirstChildElement("本地服务器");

	XMLElement* serverID = doc.NewElement("ID");
	XMLText* IDText = doc.NewText(local_server.m_strID.c_str());
	serverID->InsertEndChild(IDText);
	localNode->InsertEndChild(serverID);

	XMLElement* serverRealm = doc.NewElement("域");
	XMLText* realmText = doc.NewText(local_server.m_realm.c_str());
	serverRealm->InsertEndChild(realmText);
	localNode->InsertEndChild(serverRealm);

	XMLElement* serverProtocol = doc.NewElement("协议");
	XMLText* protocolText = doc.NewText(local_server.m_protocol.c_str());
	serverProtocol->InsertEndChild(protocolText);
	localNode->InsertEndChild(serverProtocol);

	XMLElement* serverIP = doc.NewElement("IP");

	XMLText* ipText = doc.NewText(local_server.m_ip.c_str());
	serverIP->InsertEndChild(ipText);
	localNode->InsertEndChild(serverIP);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("端口");
	_itoa(local_server.m_port, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	localNode->InsertEndChild(serverPort);

	XMLElement* serverPWD = doc.NewElement("密码");
	XMLText* PWDText = doc.NewText(local_server.m_password.c_str());
	serverPWD->InsertEndChild(PWDText);
	localNode->InsertEndChild(serverPWD);

	XMLElement* serverAuth = doc.NewElement("鉴权");
	_itoa(local_server.m_bAuthentication, intBuf, 10);
	XMLText* authText = doc.NewText(intBuf);
	serverAuth->InsertEndChild(authText);
	localNode->InsertEndChild(serverAuth);

	return doc.SaveFile(xmlPath);
}


//************************************
// Method:    queryVideoServerNodeByName 根据用户名获取用户节点
// FullName:  xmlConfig::queryVideoServerNodeByName
// Access:    public static 
// Returns:   tinyxml2::XMLElement*
// Qualifier:
// Parameter: XMLElement * root 文件根节点
// Parameter: const std::string & serverID 用户名
//************************************
XMLElement* xmlConfig::queryVideoServerNodeByName(XMLElement* root, const std::string& serverID)
{

	XMLElement* userNode = root->FirstChildElement("下级域");
	while (userNode != NULL)
	{
		if (userNode->Attribute("ID") == serverID)
			break;
		userNode = userNode->NextSiblingElement();//下一个兄弟节点
	}
	return userNode;
}


//************************************
// Method:    readLocalServerNode 读取本地服务结点信息
// FullName:  xmlConfig::readLocalServerNode
// Access:    
// Returns:   void
// Qualifier:
// Parameter: const char * xmlPath 文件路径含文件名
// Parameter: local_server & local_server 视频服务器信息
//************************************
void xmlConfig::readLocalServerNode(const char* xmlPath, local_server &local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//加载文件失败
	if (bRet != 0)
	{
		//日志输出
		return;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* httpNode = root->FirstChildElement("本地服务器");
	XMLElement* idNode = httpNode->FirstChildElement("ID");
	local_server.m_strID = idNode->GetText();
	XMLElement* realmNode = httpNode->FirstChildElement("域");
	local_server.m_realm = realmNode->GetText();
	XMLElement* protocolNode = httpNode->FirstChildElement("协议");
	local_server.m_protocol = protocolNode->GetText();
	XMLElement* ipNode = httpNode->FirstChildElement("IP");
	local_server.m_ip = ipNode->GetText();
	XMLElement* portNode = httpNode->FirstChildElement("端口");
	local_server.m_port = atoi(portNode->GetText());
	XMLElement* pwdNode = httpNode->FirstChildElement("密码");
	local_server.m_password = pwdNode->GetText();
	XMLElement* authNode = httpNode->FirstChildElement("鉴权");
	local_server.m_bAuthentication = atoi(authNode->GetText());
}

//************************************
// Method:    buildQueryCmdXml 构建设备目录查询XML
// FullName:  xmlConfig::buildQueryCmdXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
// Parameter: const char * platformID 下级域平台ID
// Parameter: int sn 命令序列号
//************************************
std::string xmlConfig::buildQueryCmdXml(const char* platformID, int sn)
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);//会覆盖xml所有内容
	XMLElement* rootNode = doc.NewElement("Query");
	doc.InsertEndChild(rootNode);

	XMLElement* cmdNode = doc.NewElement("CmdType");
	XMLText* cmdType = doc.NewText("Catalog");
	cmdNode->InsertEndChild(cmdType);
	rootNode->InsertEndChild(cmdNode);

	XMLElement* snNode = doc.NewElement("SN");
	char intBuf[10];
	_itoa(sn, intBuf, 10);
	XMLText* snText = doc.NewText(intBuf);
	snNode->InsertEndChild(snText);
	rootNode->InsertEndChild(snNode);

	XMLElement* deviceID = doc.NewElement("DeviceID");
	XMLText* idText = doc.NewText(platformID);
	deviceID->InsertEndChild(idText);
	rootNode->InsertEndChild(deviceID);

	XMLPrinter strXml;
	doc.Print(&strXml);
	return strXml.CStr();
}


//************************************
// Method:    buildControlXml 构建控制命令XML
// FullName:  xmlConfig::buildControlXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
// Parameter: const char * deviceID 摄像机ID
// Parameter: int sn 命令序列号
//************************************
std::string xmlConfig::buildControlXml(const char* deviceID, int sn)
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);
	XMLElement* rootNode = doc.NewElement("Control");
	doc.InsertEndChild(rootNode);

	XMLElement* cmdNode = doc.NewElement("CmdType");
	XMLText* cmdType = doc.NewText("DeviceControl");
	cmdNode->InsertEndChild(cmdType);
	rootNode->InsertEndChild(cmdNode);

	XMLElement* snNode = doc.NewElement("SN");
	char intBuf[10];
	_itoa(sn, intBuf, 10);
	XMLText* snText = doc.NewText(intBuf);
	snNode->InsertEndChild(snText);
	rootNode->InsertEndChild(snNode);

	XMLElement* deviceIDNode = doc.NewElement("DeviceID");
	XMLText* idText = doc.NewText(deviceID);
	deviceIDNode->InsertEndChild(idText);
	rootNode->InsertEndChild(deviceIDNode);

	XMLPrinter strXml;
	doc.Print(&strXml);
	return strXml.CStr();
}

//************************************
// Method:    builPTZControlXml 构建云台控制XML
// FullName:  xmlConfig::builPTZControlXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
// Parameter: const char * deviceID ID
// Parameter: int sn 命令序列号
// Parameter: const char * ptzCode 云台控制命令码
//************************************
std::string xmlConfig::builPTZControlXml(const char* deviceID, int sn, const char* ptzCode)
{
	std::string strControlXml = buildControlXml(deviceID, sn);
	tinyxml2::XMLDocument doc;
	XMLError bRet;
	bRet = doc.Parse(strControlXml.c_str());
	XMLElement* rootNode = doc.RootElement();

	XMLElement* PtzNode = doc.NewElement("PTZCmd");
	XMLText* ptzText = doc.NewText(ptzCode);
	PtzNode->InsertEndChild(ptzText);
	rootNode->InsertEndChild(PtzNode);

	XMLPrinter strXml;
	doc.Print(&strXml);
	return strXml.CStr();
}