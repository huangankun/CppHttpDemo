#pragma once
#include "all.h"
class camera_info
{
public:
	camera_info();
	~camera_info();
	std::string strCamId;
	int iRecvPort;//��Ӧý����ն˶˿�
	int status;
	int statusErrCnt;
	int running;
};

