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

	//获取程序工作的当前路径
	static const char* getWorkDir();

	//创建XML文件
	static int createXML();

	//插入视频下级服务器结点
	static int insertVideoServerNode(const video_server& video_server);

	//插入本地服务器信息结点
	static int insertLocalServerNode(const local_server& local_server);
	
	//查询xml文件的指定视频服务器结点
	static XMLElement* queryVideoServerNodeByName(XMLElement* root, const std::string& userName);

	//读取本地服务的信息
	static void readLocalServerNode(local_server& local_server);

	//读取http服务器的信息
	static void readHttpServerNode(HttpServer & http_server);

	//创建查询目录xml
	static std::string buildQueryCmdXml(const char* platformID, int sn);

	//创建云台控制xml
	static std::string builPTZControlXml(const char* deviceID, int sn, const char* ptzCode);

	//创建控制类型xml
	static std::string buildControlXml(const char* deviceID, int sn);

	//全局本地服务器对象指针
	static local_server *gb28181Server;

	//获取日志配置文件
	static void logGetConf();

	//读取下级服务器配置信息
	static void readVideoServerNodes(local_server &local_server);

	//工作路径
	static std::string strWorkPath;

	//配置文件所在路径
	static std::string configPath;

	//获取当前系统时间用作保存图片的文件名
	static std::string getCurrentTime();
};
