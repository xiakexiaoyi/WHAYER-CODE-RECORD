#ifndef __COMM_CONTAINER__H__
#define __COMM_CONTAINER__H__
//#include "DriveInfo.h"

#ifdef WIN32
#include "perfmon.h"
#endif
class TCommContainer 
{
	enum
	{
		MEGA_UNIT=1024*1024,
	};
public:
	TCommContainer();
	~TCommContainer();
	bool CollectCpuMemUseStats(int& memUsed,int& memAvil,int& memUsedPerCent,int& cpuRate);
	//bool CollectHDUseStats(int&freeSpace, int& totalSpace,int& freeRate);
private:
	bool InitCpuMemCounter();
private:
	CPerfMon m_perfMon;
	int	m_cpuCounter;
};
#endif