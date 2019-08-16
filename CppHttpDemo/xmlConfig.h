#pragma once
#include "all.h"
#include "tinyxml2.h"
using namespace tinyxml2;
#include "local_server.h"
class local_server;

class xmlConfig
{
public:
	xmlConfig();
	~xmlConfig();

	//��ȡ�������ĵ�ǰ·��
	static const char* getWorkDir();

	//����XML�ļ�
	static int createXML(const char* xmlPath);

	//������Ƶ�¼����������
	static int insertVideoServerNode(const char* xmlPath, const local_server& local_server);

	//���뱾�ط�������Ϣ���
	static int insertHttpServerNode(const char* xmlPath, const local_server& local_server);
	
	//��ѯxml�ļ���ָ����Ƶ���������
	static XMLElement* queryVideoServerNodeByName(XMLElement* root, const std::string& userName);

	//��ȡ���ط������Ϣ
	static void readLocalServerNode(const char* xmlPath, local_server& local_server);

	static std::string buildQueryCmdXml(const char* platformID, int sn);

	static std::string builPTZControlXml(const char* deviceID, int sn, const char* ptzCode);

	static std::string buildControlXml(const char* deviceID, int sn);
};
