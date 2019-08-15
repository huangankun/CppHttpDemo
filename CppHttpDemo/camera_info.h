#pragma once
#include "all.h"
class camera_info
{
public:
	camera_info();
	~camera_info();
	std::string strCamId;
	int iRecvPort;//对应媒体接收端端口
	int status;
	int statusErrCnt;
	int running;
};

