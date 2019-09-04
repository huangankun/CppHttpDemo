#pragma once
#include "all.h"
#include "tinyxml2.h"
using namespace tinyxml2;
#include "local_server.h"
#include "http_server.h"

class HttpServer;
class local_server;
class video_server;

class xmlConfig
{
public:
	xmlConfig();
	~xmlConfig();

	//��ȡ�������ĵ�ǰ·��
	static const char* getWorkDir();

	//����XML�ļ�
	static int createXML();

	//������Ƶ�¼����������
	static int insertVideoServerNode(const video_server& video_server);

	//���뱾�ط�������Ϣ���
	static int insertLocalServerNode(const local_server& local_server);
	
	//��ѯxml�ļ���ָ����Ƶ���������
	static XMLElement* queryVideoServerNodeByName(XMLElement* root, const std::string& userName);

	//��ȡ���ط������Ϣ
	static void readLocalServerNode(local_server& local_server);

	//��ȡhttp����������Ϣ
	static void readHttpServerNode(HttpServer & http_server);

	//������ѯĿ¼xml
	static std::string buildQueryCmdXml(const char* platformID, int sn);

	//������̨����xml
	static std::string builPTZControlXml(const char* deviceID, int sn, const char* ptzCode);

	//������������xml
	static std::string buildControlXml(const char* deviceID, int sn);

	//ȫ�ֱ��ط���������ָ��
	static local_server *gb28181Server;

	//��ȡ��־�����ļ�
	static void logGetConf();

	//��ȡ�¼�������������Ϣ
	static void readVideoServerNodes(local_server &local_server);

	//����·��
	static std::string strWorkPath;

	//�����ļ�����·��
	static std::string configPath;

	//��ȡ��ǰϵͳʱ����������ͼƬ���ļ���
	static std::string getCurrentTime();
};
