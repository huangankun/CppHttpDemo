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

	//获取程序工作的当前路径
	static const char* getWorkDir();

	//创建XML文件
	static int createXML(const char* xmlPath);

	//插入视频下级服务器结点
	static int insertVideoServerNode(const char* xmlPath, const local_server& local_server);

	//插入本地服务器信息结点
	static int insertHttpServerNode(const char* xmlPath, const local_server& local_server);
	
	//查询xml文件的指定视频服务器结点
	static XMLElement* queryVideoServerNodeByName(XMLElement* root, const std::string& userName);

	//读取本地服务的信息
	static void readLocalServerNode(const char* xmlPath, local_server& local_server);

	static std::string buildQueryCmdXml(const char* platformID, int sn);

	static std::string builPTZControlXml(const char* deviceID, int sn, const char* ptzCode);

	static std::string buildControlXml(const char* deviceID, int sn);
};
