#include "xmlConfig.h"


xmlConfig::xmlConfig()
{
}


xmlConfig::~xmlConfig()
{
}

//************************************
// Method:    getWorkDir ��ȡ������·��
// FullName:  xmlConfig::getWorkDir
// Access:    public static 
// Returns:   const char*
// Qualifier:
//************************************
const char* xmlConfig::getWorkDir()
{
	char *buffer;
	//Ҳ���Խ�buffer��Ϊ�������
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
// Method:    createXML ����xml�ļ�
// FullName:  xmlConfig::createXML
// Access:    public static 
// Returns:   int �ɹ�����0��ʧ�ܷ��ش�����
// Qualifier:
// Parameter: const char * xmlPath �ļ�·�����ļ���
//************************************
int xmlConfig::createXML(const char* xmlPath) 
{
	const char* declaration = "<?xml version=\"1.0\" encoding=\"gb2312\"?>";
	tinyxml2::XMLDocument doc;
	doc.Parse(declaration);//�Ḳ��xml��������

	//�����������ʹ����������
	//XMLDeclaration* declaration=doc.NewDeclaration();
	//doc.InsertFirstChild(declaration);

	XMLElement* root = doc.NewElement("������");
	doc.InsertEndChild(root);

	XMLElement* localNode = doc.NewElement("���ط�����");
	root->InsertEndChild(localNode);

	XMLElement* videoNode = doc.NewElement("�¼���");
	root->InsertEndChild(videoNode);

	XMLElement* nginxNode = doc.NewElement("Nginx������");
	root->InsertEndChild(nginxNode);

	return doc.SaveFile(xmlPath);
}


//************************************
// Method:    insertVideoServerNode ������Ƶ���������
// FullName:  xmlConfig::insertVideoServerNode
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: const local_server & local_server ��Ƶ��������Ϣ
//************************************
int xmlConfig::insertVideoServerNode(const char* xmlPath, const local_server& local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* serversNode = root->FirstChildElement("�¼���");

	XMLElement* serverNode = doc.NewElement("�¼�������");
	serverNode->SetAttribute("ID", local_server.platformServer.m_strID.c_str());
	serverNode->SetAttribute("��", local_server.platformServer.m_strRealm.c_str());
	serversNode->InsertEndChild(serverNode);

	XMLElement* serverIP = doc.NewElement("IP");
	XMLText* ipText = doc.NewText(local_server.platformServer.m_strIP.c_str());
	serverIP->InsertEndChild(ipText);
	serverNode->InsertEndChild(serverIP);

	XMLElement* serverPWD = doc.NewElement("����");
	XMLText* PWDText = doc.NewText(local_server.platformServer.m_strPassword.c_str());
	serverPWD->InsertEndChild(PWDText);
	serverNode->InsertEndChild(serverPWD);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("�˿�");
	_itoa(local_server.platformServer.m_iPort, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	serverNode->InsertEndChild(serverPort);

	XMLElement* serverExpires = doc.NewElement("ע��ʱ��");
	_itoa(local_server.platformServer.m_iExpires, intBuf, 10);
	XMLText* expiresText = doc.NewText(intBuf);
	serverExpires->InsertEndChild(expiresText);
	serverNode->InsertEndChild(serverExpires);

	XMLElement* serverHeartBeat = doc.NewElement("��������");
	_itoa(local_server.platformServer.m_iHeartBeat, intBuf, 10);
	XMLText* heartBeatText = doc.NewText(intBuf);
	serverHeartBeat->InsertEndChild(heartBeatText);
	serverNode->InsertEndChild(serverHeartBeat);

	XMLElement* serverKeepAlive = doc.NewElement("����");
	_itoa(local_server.platformServer.m_bKeepAlive, intBuf, 10);
	XMLText* keepAliveText = doc.NewText(intBuf);
	serverKeepAlive->InsertEndChild(keepAliveText);
	serverNode->InsertEndChild(serverKeepAlive);

	XMLElement* serverKeepAliveInterval = doc.NewElement("������");
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
// Method:    insertHttpServerNode ���뱾�ط��������
// FullName:  xmlConfig::insertHttpServerNode
// Access:    public static 
// Returns:   int
// Qualifier:
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: const local_server & local_server ��Ƶ��������Ϣ
//************************************
int xmlConfig::insertHttpServerNode(const char* xmlPath, const local_server& local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return bRet;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* localNode = root->FirstChildElement("���ط�����");

	XMLElement* serverID = doc.NewElement("ID");
	XMLText* IDText = doc.NewText(local_server.m_strID.c_str());
	serverID->InsertEndChild(IDText);
	localNode->InsertEndChild(serverID);

	XMLElement* serverRealm = doc.NewElement("��");
	XMLText* realmText = doc.NewText(local_server.m_realm.c_str());
	serverRealm->InsertEndChild(realmText);
	localNode->InsertEndChild(serverRealm);

	XMLElement* serverProtocol = doc.NewElement("Э��");
	XMLText* protocolText = doc.NewText(local_server.m_protocol.c_str());
	serverProtocol->InsertEndChild(protocolText);
	localNode->InsertEndChild(serverProtocol);

	XMLElement* serverIP = doc.NewElement("IP");

	XMLText* ipText = doc.NewText(local_server.m_ip.c_str());
	serverIP->InsertEndChild(ipText);
	localNode->InsertEndChild(serverIP);

	char intBuf[10];
	XMLElement* serverPort = doc.NewElement("�˿�");
	_itoa(local_server.m_port, intBuf, 10);
	XMLText* portText = doc.NewText(intBuf);
	serverPort->InsertEndChild(portText);
	localNode->InsertEndChild(serverPort);

	XMLElement* serverPWD = doc.NewElement("����");
	XMLText* PWDText = doc.NewText(local_server.m_password.c_str());
	serverPWD->InsertEndChild(PWDText);
	localNode->InsertEndChild(serverPWD);

	XMLElement* serverAuth = doc.NewElement("��Ȩ");
	_itoa(local_server.m_bAuthentication, intBuf, 10);
	XMLText* authText = doc.NewText(intBuf);
	serverAuth->InsertEndChild(authText);
	localNode->InsertEndChild(serverAuth);

	return doc.SaveFile(xmlPath);
}


//************************************
// Method:    queryVideoServerNodeByName �����û�����ȡ�û��ڵ�
// FullName:  xmlConfig::queryVideoServerNodeByName
// Access:    public static 
// Returns:   tinyxml2::XMLElement*
// Qualifier:
// Parameter: XMLElement * root �ļ����ڵ�
// Parameter: const std::string & serverID �û���
//************************************
XMLElement* xmlConfig::queryVideoServerNodeByName(XMLElement* root, const std::string& serverID)
{

	XMLElement* userNode = root->FirstChildElement("�¼���");
	while (userNode != NULL)
	{
		if (userNode->Attribute("ID") == serverID)
			break;
		userNode = userNode->NextSiblingElement();//��һ���ֵܽڵ�
	}
	return userNode;
}


//************************************
// Method:    readLocalServerNode ��ȡ���ط�������Ϣ
// FullName:  xmlConfig::readLocalServerNode
// Access:    
// Returns:   void
// Qualifier:
// Parameter: const char * xmlPath �ļ�·�����ļ���
// Parameter: local_server & local_server ��Ƶ��������Ϣ
//************************************
void xmlConfig::readLocalServerNode(const char* xmlPath, local_server &local_server)
{
	tinyxml2::XMLDocument doc;
	int bRet = doc.LoadFile(xmlPath);

	//�����ļ�ʧ��
	if (bRet != 0)
	{
		//��־���
		return;
	}

	XMLElement* root = doc.RootElement();
	XMLElement* httpNode = root->FirstChildElement("���ط�����");
	XMLElement* idNode = httpNode->FirstChildElement("ID");
	local_server.m_strID = idNode->GetText();
	XMLElement* realmNode = httpNode->FirstChildElement("��");
	local_server.m_realm = realmNode->GetText();
	XMLElement* protocolNode = httpNode->FirstChildElement("Э��");
	local_server.m_protocol = protocolNode->GetText();
	XMLElement* ipNode = httpNode->FirstChildElement("IP");
	local_server.m_ip = ipNode->GetText();
	XMLElement* portNode = httpNode->FirstChildElement("�˿�");
	local_server.m_port = atoi(portNode->GetText());
	XMLElement* pwdNode = httpNode->FirstChildElement("����");
	local_server.m_password = pwdNode->GetText();
	XMLElement* authNode = httpNode->FirstChildElement("��Ȩ");
	local_server.m_bAuthentication = atoi(authNode->GetText());
}

//************************************
// Method:    buildQueryCmdXml �����豸Ŀ¼��ѯXML
// FullName:  xmlConfig::buildQueryCmdXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
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
// Method:    buildControlXml ������������XML
// FullName:  xmlConfig::buildControlXml
// Access:    public static 
// Returns:   std::string
// Qualifier:
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