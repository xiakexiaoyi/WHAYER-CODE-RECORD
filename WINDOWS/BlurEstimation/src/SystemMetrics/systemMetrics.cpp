#include "systemMetrics.h"
#ifndef WIN32
#include "ctutil.h"
#include <sys/times.h> 
#include <patterns/util/StringUtil.hpp>
using namespace suntek::util;
#endif

TCommContainer::TCommContainer( )
{	
#ifdef WIN32
	InitCpuMemCounter();
#else
#endif
}
TCommContainer::~TCommContainer(){}
#ifdef WIN32
bool TCommContainer::InitCpuMemCounter()
{
	m_perfMon.Initialize();
	m_cpuCounter=m_perfMon.AddCounter(CNTR_CPU);
	return true;
}
#endif
//bool TCommContainer::CollectHDUseStats(int&freeSpace, int& totalSpace,int& freeRate)
//{
//	freeSpace=0;
//	totalSpace=0;
//	freeRate=0;
//	vector<string> nameArray,typeArray,volumeArray,serialNumberArray,
//		        fileSysTypeArray,strTotalSpaceArray,strFreeSpaceArray;
//	vector<unsigned long> freeRatioArray,totalSpaceArray,freeSpaceArray;
//	unsigned int queryDiskType = DriveInfo::HARDDRIVE;
//	DriveInfo driveInfo(queryDiskType);
//	const vector<DriveStats>& driveStats=driveInfo.getDriveStats();
//	if (driveStats.size()<=0)
//		return false;
//	for(size_t i=0;i<driveStats.size();i++)
//	{
//		const DriveStats& driver=driveStats[i];
//		nameArray.push_back(driver.getName());
//		typeArray.push_back(driver.getType());
//		volumeArray.push_back(driver.getVolumeName());
//		serialNumberArray.push_back(driver.getSerialNumber());
//		fileSysTypeArray.push_back(driver.getFileSystemType());
//		strTotalSpaceArray.push_back(driver.getTotalSpace());
//		strFreeSpaceArray.push_back(driver.getFreeSpace());
//		freeRatioArray.push_back((unsigned long)(driver.GetFreeRatio()*100));
//		totalSpaceArray.push_back((unsigned long)(driver.GetTotalSpaceNum()));
//		freeSpaceArray.push_back((unsigned long)(driver.GetFreeSpaceNum()));
//	}
//	size_t hdsize = min(totalSpaceArray.size(), freeSpaceArray.size()); 
//	for (size_t st = 0; st < hdsize; st++)
//	{
//		totalSpace += totalSpaceArray[st];
//		freeSpace += freeSpaceArray[st];
//	}
//	freeRate=freeSpace*100/totalSpace;
//
//	return true;
//}

bool TCommContainer::CollectCpuMemUseStats(int& memUsed,int& memAvil,int& memUsedPerCent,int& cpuRate)
{
#ifdef WIN32
	MEMORYSTATUSEX  MemInfo;
	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatusEx(&MemInfo);
	memUsed = (MemInfo.ullTotalPhys - MemInfo.ullAvailPhys + MEGA_UNIT - 1) / MEGA_UNIT;
	memAvil	= (MemInfo.ullAvailPhys + MEGA_UNIT - 1) / MEGA_UNIT;
	memUsedPerCent = MemInfo.dwMemoryLoad;
	if (m_perfMon.CollectQueryData())
	{
		cpuRate = m_perfMon.GetCounterValue(1);	
	}
	else
		return false;
#else
	//Memory usage stats
	const char* szMemInfoFile="/proc/meminfo";
	const char strLn[2]={0xA,0};
	string strMemInfo;
	ReadDataFromFile(szMemInfoFile,strMemInfo);	
	StringUtil::TKvSetMap memInfoMap;
	StringUtil::ParseKvSet(strMemInfo,memInfoMap,strLn,":");
	size_t memTotal(0);
	memTotal=(size_t)atol(memInfoMap["MemTotal"].c_str())/1024;
	memAvil=(size_t)atol(memInfoMap["MemFree"].c_str());
	memAvil=memAvil/1024;
	memUsed=memTotal-memAvil;
	memUsedPerCent=(size_t)(((double)memUsed/memTotal)*100);
	memLimit=(size_t)atol(memInfoMap["SwapTotal"].c_str())/1024;

	//CPU usage stats
	int cpuProcessCount = 0;
	if(0 == cpuProcessCount)
	{
		const char* szCpuFile="/proc/cpuinfo";
		string strInfoData;
		ReadDataFromFile(szCpuFile,strInfoData);
		string::size_type pos = 0;
		while( (pos = strInfoData.find("processor", pos)) !=  string::npos)
		{
			pos+=9;
			cpuProcessCount++;
		}
		if(0 == cpuProcessCount)
			cpuProcessCount = 1;
	}
	int user(0),system(0),nice(0),idle(0),iowait(0),irq(0),softirq(0);
	int total_time1,total_time2,user_time,system_time;
	const char* szCpuInfoFile="/proc/stat";
	CTstring strCpuInfo;
	ReadDataFromFile(szCpuInfoFile,strCpuInfo);
	vector<CTstring> vctLines;
	StringUtil::split(vctLines,strCpuInfo,strLn);
	vector<CTstring> vctCpu;
	StringUtil::split(vctCpu,vctLines[0]," ",false);
	user=atol(vctCpu[1].c_str());
	nice=atol(vctCpu[2].c_str());
	system=atol(vctCpu[3].c_str());
	idle=atol(vctCpu[4].c_str());
	iowait=atol(vctCpu[5].c_str());
	irq=atol(vctCpu[6].c_str());
	softirq=atol(vctCpu[7].c_str());
	total_time1=user+system+nice+idle+iowait+irq+softirq;
	sleep(1);
	strCpuInfo="";
	ReadDataFromFile(szCpuInfoFile,strCpuInfo);
	StringUtil::split(vctLines,strCpuInfo,strLn);
	StringUtil::split(vctCpu,vctLines[0]," ",false);
	user_time=atol(vctCpu[1].c_str())-user;
	system_time=atol(vctCpu[3].c_str())-system;
	user=atol(vctCpu[1].c_str());
	nice=atol(vctCpu[2].c_str());
	system=atol(vctCpu[3].c_str());
	idle=atol(vctCpu[4].c_str());
	iowait=atol(vctCpu[5].c_str());
	irq=atol(vctCpu[6].c_str());
	softirq=atol(vctCpu[7].c_str());
	total_time2=user+system+nice+idle+iowait+irq+softirq;
	cpuRate=100*(user_time+system_time)/(total_time2-total_time1);
#endif
	return true;
}

