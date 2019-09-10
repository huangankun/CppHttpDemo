#include "xmlConfig.h"

local_server *xmlConfig::gb28181Server = nullptr;


xmlConfig::xmlConfig()
{
}


xmlConfig::~xmlConfig()
{
}

//************************************
// Method:    getWorkDir
// FullName:  xmlConfig::getWorkDir
// Access:    public static 
// Returns:   const char*
// Qualifier: ��ȡ������·��
//************************************
const char* xmlConfig::getWorkDir()
{
	xmlConfig::logGetConf();
	char *buffer;
	//Ҳ���Խ�buffer��Ϊ�������
	if ((buffer = _getcwd(NULL, 0)) == NULL)
	{
		LOG(INFO) << "Program working directory failed to get" << buffer;
		return NULL;
	}
	else
	{
		LOG(INFO) << "Program working directory��" << buffer;
		return buffer;
	}
}


//************************************
// Method:    createXML
// FullName:  xmlConfig::createXML
// Access:    public static 
// Returns:   int �ɹ�����0��ʧ�ܷ��ش�����
// Qualifier: ����xml�ļ�
// Parameter: const char * xmlPath �ļ�·�����ļ���
//************************************
int xmlConfig::createXML() 
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);//�Ḳ��xml��������

	//�����������ʹ����������
	//XMLDeclaration* declaration=doc.NewDeclaration();
	//doc.InsertFirstChild(declaration);

	XMLElement* root = doc.NewElement("Configuration");
	doc.InsertEndChild(root);

	XMLElement* localNode = doc.NewElement("local");
	root->InsertEndChild(localNode);

	XMLElement* videoNode = doc.NewElement("SubRealm");
	root->InsertEndChild(videoNode);

	return doc.SaveFile(xmlConfig::configPath.c_str());
}


//************************************
// Method:    insertVideoServerNode
// FullName:  xmlConfig::insertVideoServerNode
// Access:    public static 
// Returns:   int
// Qualifier: ������Ƶ���������
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: const local_server & local_server ��Ƶ��������Ϣ
//************************************
int xmlConfig::insertVideoServerNode(const video_server& video_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlConfig::configPath.c_str());

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* serversNode = root->FirstChildElement("SubRealm");

	XMLElement* serverNode = doc.NewElement("subServer");
	serverNode->SetAttribute("ID", video_server.m_strID.c_str());
	serverNode->SetAttribute("realm", video_server.m_strRealm.c_str());
	serversNode->InsertEndChild(serverNode);

	XMLElement* serverIP = doc.NewElement("IP");
	XMLText* ipText = doc.NewText(video_server.m_strIP.c_str());
	serverIP->InsertEndChild(ipText);
	serverNode->InsertEndChild(serverIP);

	XMLElement* serverPWD = doc.NewElement("password");
	XMLText* PWDText = doc.NewText(video_server.m_strPassword.c_str());
	serverPWD->InsertEndChild(PWDText);
	serverNode->InsertEndChild(serverPWD);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("port");
	_itoa(video_server.m_iPort, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	serverNode->InsertEndChild(serverPort);

	XMLElement* serverExpires = doc.NewElement("expries");
	_itoa(video_server.m_iExpires, intBuf, 10);
	XMLText* expiresText = doc.NewText(intBuf);
	serverExpires->InsertEndChild(expiresText);
	serverNode->InsertEndChild(serverExpires);

	XMLElement* serverHeartBeat = doc.NewElement("HeartbeatCycle");
	_itoa(video_server.m_iHeartBeat, intBuf, 10);
	XMLText* heartBeatText = doc.NewText(intBuf);
	serverHeartBeat->InsertEndChild(heartBeatText);
	serverNode->InsertEndChild(serverHeartBeat);

	XMLElement* serverKeepAlive = doc.NewElement("keepalive");
	_itoa(video_server.m_bKeepAlive, intBuf, 10);
	XMLText* keepAliveText = doc.NewText(intBuf);
	serverKeepAlive->InsertEndChild(keepAliveText);
	serverNode->InsertEndChild(serverKeepAlive);

	XMLElement* serverKeepAliveInterval = doc.NewElement("aliveInterval");
	_itoa(video_server.m_iKeepAliveInterval, intBuf, 10);
	XMLText* keepAliveIntervalText = doc.NewText(intBuf);
	serverKeepAliveInterval->InsertEndChild(keepAliveIntervalText);
	serverNode->InsertEndChild(serverKeepAliveInterval);

	XMLElement* serverReceiveRTCP = doc.NewElement("RTCP");
	_itoa(video_server.m_bReceiveRTCP, intBuf, 10);
	XMLText* receiveRTCPText = doc.NewText(intBuf);
	serverReceiveRTCP->InsertEndChild(receiveRTCPText);
	serverNode->InsertEndChild(serverReceiveRTCP);

	return doc.SaveFile(xmlConfig::configPath.c_str());
}


//************************************
// Method:    insertHttpServerNode
// FullName:  xmlConfig::insertHttpServerNode
// Access:    public static 
// Returns:   int
// Qualifier: ���뱾�ط��������
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: const local_server & local_server ��Ƶ��������Ϣ
//************************************
int xmlConfig::insertLocalServerNode(const local_server& local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlConfig::configPath.c_str());

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* localNode = root->FirstChildElement("local");

	XMLElement* serverID = doc.NewElement("ID");
	XMLText* IDText = doc.NewText(local_server.m_strID.c_str());
	serverID->InsertEndChild(IDText);
	localNode->InsertEndChild(serverID);

	XMLElement* serverRealm = doc.NewElement("realm");
	XMLText* realmText = doc.NewText(local_server.m_realm.c_str());
	serverRealm->InsertEndChild(realmText);
	localNode->InsertEndChild(serverRealm);

	XMLElement* serverProtocol = doc.NewElement("protocol");
	XMLText* protocolText = doc.NewText(local_server.m_protocol.c_str());
	serverProtocol->InsertEndChild(protocolText);
	localNode->InsertEndChild(serverProtocol);

	XMLElement* serverIP = doc.NewElement("IP");

	XMLText* ipText = doc.NewText(local_server.m_ip.c_str());
	serverIP->InsertEndChild(ipText);
	localNode->InsertEndChild(serverIP);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("port");
	_itoa(local_server.m_port, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	localNode->InsertEndChild(serverPort);

	XMLElement* serverPWD = doc.NewElement("password");
	XMLText* PWDText = doc.NewText(local_server.m_password.c_str());
	serverPWD->InsertEndChild(PWDText);
	localNode->InsertEndChild(serverPWD);

	XMLElement* serverAuth = doc.NewElement("Authentication");
	_itoa(local_server.m_bAuthentication, intBuf, 10);
	XMLText* authText = doc.NewText(intBuf);
	serverAuth->InsertEndChild(authText);
	localNode->InsertEndChild(serverAuth);

	return doc.SaveFile(xmlConfig::configPath.c_str());
}


//************************************
// Method:    queryVideoServerNodeByName
// FullName:  xmlConfig::queryVideoServerNodeByName
// Access:    public static 
// Returns:   tinyxml2::XMLElement*
// Qualifier: �����û�����ȡ�û��ڵ�
// Parameter: XMLElement * root �ļ����ڵ�
// Parameter: const std::string & serverID �û���
//************************************
XMLElement* xmlConfig::queryVideoServerNodeByName(XMLElement* root, const std::string& serverID)
{

	XMLElement* userNode = root->FirstChildElement("SubRealm");
	while (userNode != NULL)
	{
		if (userNode->Attribute("ID") == serverID)
			break;
		userNode = userNode->NextSiblingElement();//��һ���ֵܽڵ�
	}
	return userNode;
}


//************************************
// Method:    readLocalServerNode
// FullName:  xmlConfig::readLocalServerNode
// Access:    public static
// Returns:   void
// Qualifier: ��ȡ���ط�������Ϣ
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: local_server & local_server ��Ƶ��������Ϣ
//************************************
void xmlConfig::readLocalServerNode(local_server &local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlConfig::configPath.c_str());

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* localNode = root->FirstChildElement("local");

	XMLElement* idNode = localNode->FirstChildElement("ID");
	local_server.m_strID = idNode->GetText();

	XMLElement* realmNode = localNode->FirstChildElement("realm");
	local_server.m_realm = realmNode->GetText();

	XMLElement* protocolNode = localNode->FirstChildElement("protocol");
	local_server.m_protocol = protocolNode->GetText();

	XMLElement* ipNode = localNode->FirstChildElement("IP");
	local_server.m_ip = ipNode->GetText();

	XMLElement* portNode = localNode->FirstChildElement("port");
	local_server.m_port = atoi(portNode->GetText());

	XMLElement* pwdNode = localNode->FirstChildElement("password");
	local_server.m_password = pwdNode->GetText();

	XMLElement* authNode = localNode->FirstChildElement("Authentication");
	local_server.m_bAuthentication = atoi(authNode->GetText());

	XMLElement* videoNumNode = localNode->FirstChildElement("videoNum");
	local_server.m_videoNum = atoi(videoNumNode->GetText());
}

//************************************
// Method:    readHttpServerNode
// FullName:  xmlConfig::readHttpServerNode
// Access:    public static 
// Returns:   void
// Qualifier: ��ȡHTTP����������Ϣ
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: HttpServer & HttpServer	HTTP�������Ϣ
//************************************
void xmlConfig::readHttpServerNode(HttpServer & http_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlConfig::configPath.c_str());

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* httpNode = root->FirstChildElement("http");

	XMLElement* ipNode = httpNode->FirstChildElement("IP");
	http_server.m_ip = ipNode->GetText();

	XMLElement* portNode = httpNode->FirstChildElement("port");
	http_server.m_port = portNode->GetText();
}

//************************************
// Method:    buildQueryCmdXml 
// FullName:  xmlConfig::buildQueryCmdXml
// Access:    public static 
// Returns:   std::string
// Qualifier: �����豸Ŀ¼��ѯXML
// Parameter: const char * platformID �¼���ƽ̨ID
// Parameter: int sn �������к�
//************************************
std::string xmlConfig::buildQueryCmdXml(const char* platformID, int sn)
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);//�Ḳ��xml��������
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
// Method:    buildControlXml 
// FullName:  xmlConfig::buildControlXml
// Access:    public static 
// Returns:   std::string
// Qualifier: ������������XML
// Parameter: const char * deviceID �����ID
// Parameter: int sn �������к�
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
// Method:    logGetConf
// FullName:  xmlConfig::logGetConf
// Access:    public static 
// Returns:   void
// Qualifier:	��ȡ��־�ļ�����
//************************************
void xmlConfig::logGetConf()
{
	el::Configurations conf("my_log.conf");
	el::Loggers::reconfigureAllLoggers(conf);
}

//************************************
// Method:    readVideoServerNodes
// FullName:  xmlConfig::readVideoServerNodes
// Access:    public static 
// Returns:   void
// Qualifier:	�������ļ��ж�ȡ�¼�����������Ϣ����
// Parameter: const char * xmlPath
// Parameter: local_server & local_server
//************************************
void xmlConfig::readVideoServerNodes(local_server & local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlConfig::configPath.c_str());

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* videoNodes = root->FirstChildElement("SubRealm");
	XMLElement* videoNode = videoNodes->FirstChildElement("subServer");
	while (videoNode != nullptr)
	{
		video_server newNode;
		newNode.m_strID = videoNode->Attribute("ID");
		newNode.m_strRealm = videoNode->Attribute("realm");

		XMLElement* ipNode = videoNode->FirstChildElement("IP");
		newNode.m_strIP = ipNode->GetText();

		XMLElement* pwdNode = videoNode->FirstChildElement("password");
		newNode.m_strPassword = pwdNode->GetText();

		XMLElement* portNode = videoNode->FirstChildElement("port");
		newNode.m_iPort = atoi(portNode->GetText());

		XMLElement* expiresNode = videoNode->FirstChildElement("expries");
		newNode.m_iExpires = atoi(expiresNode->GetText());

		XMLElement* beatNode = videoNode->FirstChildElement("HeartbeatCycle");
		newNode.m_iHeartBeat = atoi(beatNode->GetText());

		XMLElement* aliveNode = videoNode->FirstChildElement("keepalive");
		newNode.m_bKeepAlive = atoi(aliveNode->GetText());

		XMLElement* intervalNode = videoNode->FirstChildElement("aliveInterval");
		newNode.m_iKeepAliveInterval = atoi(intervalNode->GetText());

		XMLElement* rtcpNode = videoNode->FirstChildElement("RTCP");
		newNode.m_bReceiveRTCP = atoi(rtcpNode->GetText());

		local_server.m_platformList.push_back(newNode);
		videoNode = videoNode->NextSiblingElement();
	}
}

//************************************
// Method:    getCurrentTime
// FullName:  xmlConfig::getCurrentTime
// Access:    public static 
// Returns:   std::string
// Qualifier: 	��ȡ��ǰϵͳʱ����������ͼƬ���ļ���
//************************************
std::string xmlConfig::getCurrentTime()
{
	time_t nowTime = time(0);
	struct tm *infoTm = localtime(&nowTime);
	char buffer[80];
	strftime(buffer, 80, "%Y_%m_%d_%H_%M_%S", infoTm);
	std::string strTime(buffer);
	return strTime;
}

//************************************
// Method:    builPTZControlXml ������̨����XML
// FullName:  xmlConfig::builPTZControlXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
// Parameter: const char * deviceID ID
// Parameter: int sn �������к�
// Parameter: const char * ptzCode ��̨����������
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