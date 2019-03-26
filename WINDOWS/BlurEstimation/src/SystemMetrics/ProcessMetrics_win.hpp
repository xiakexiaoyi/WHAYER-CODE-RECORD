#ifndef INCLUDED_PROCESS_METRICS_H
#define INCLUDED_PROCESS_METRICS_H

typedef long long           int64;
typedef unsigned long long  uint64;


#include <stdio.h>  
#include <Windows.h>
#include <iostream>
#include <psapi.h>
//#include "SystemMetrics/systemMetrics.h"
#include "SystemMetrics/perfmon.h"

#pragma  comment(lib,"Psapi.lib")

class ProcessMetrics
{
	enum
	{
		MEGA_UNIT=1024*1024,
	};
public:
	ProcessMetrics(){}
	~ProcessMetrics(){}

	int GetCPUUsage()//获得进程的CPU占用率
	{
		int pid=GetCurrentProcessId();
		int nProcessorNum=GetProcessorNumber();
		if (nProcessorNum==0)//如果处理器个数为零则返回
		{
			return 0;
		}
		FILETIME nowTime;
		FILETIME creationTime;
		FILETIME exitTime;
		FILETIME kernelTime;
		FILETIME userTime;
		static int64 lastTime = 0;
		static int64 lastSystemTime = 0;

		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
		if (!GetProcessTimes(hProcess, &creationTime, &exitTime,
			&kernelTime, &userTime)) 
		{
			CloseHandle(hProcess);
			return 0;
		}
		CloseHandle(hProcess);//关闭句柄

		int64 systemTime = (FileTimeToUTC(kernelTime)+FileTimeToUTC(userTime))/nProcessorNum;
		GetSystemTimeAsFileTime(&nowTime);
		int64 time = FileTimeToUTC(nowTime);
		if ((lastSystemTime == 0) || (lastTime == 0))
		{
			// First call, just set the last values.
			lastSystemTime = systemTime;
			lastTime = time;
			return 0;
		}

		int64 systemTimeDelta = systemTime - lastSystemTime;
		int64 timeDelta = time - lastTime;
		if (timeDelta == 0)
			return 0;

		// We add time_delta / 2 so the result is rounded.
		int nCpuRate = (int)((systemTimeDelta * 100 + timeDelta / 2)/timeDelta);

		lastSystemTime = systemTime;
		lastTime = time;
		return nCpuRate;
	}

	int GetMemoryUsage()//获得进程的内存使用情况
	{
		int processID=GetCurrentProcessId();
		PROCESS_MEMORY_COUNTERS pmc;
		HANDLE hProcess= OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,FALSE, processID );
		int nRet = 0;
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			nRet = (pmc.WorkingSetSize/(1024*1024));//单位是M
		}
		CloseHandle(hProcess);
		return nRet;
	}

	int GetSystemCPUUsage()//获得系统的CPU占用率
	{
		int memUsed,memAvil,memUsedPerCent,cpuRate;
		int	cpuCounter;
		CPerfMon perfMon;

		MEMORYSTATUSEX  MemInfo;
		MemInfo.dwLength = sizeof(MemInfo);

		perfMon.Initialize();
		cpuCounter=perfMon.AddCounter(CNTR_CPU);
		GlobalMemoryStatusEx(&MemInfo);

		memUsed = (MemInfo.ullTotalPhys - MemInfo.ullAvailPhys + MEGA_UNIT - 1) / MEGA_UNIT;
		memAvil	= (MemInfo.ullAvailPhys + MEGA_UNIT - 1) / MEGA_UNIT;
		memUsedPerCent = MemInfo.dwMemoryLoad;
		if (perfMon.CollectQueryData())
		{
			cpuRate = perfMon.GetCounterValue(cpuCounter);	
		}
		else
			return false;

		//m_comContainer.CollectCpuMemUseStats(memUsed,memAvil,memUsedPerCent,cpuRate);
		return cpuRate;
	}

private:
	int GetProcessorNumber()//获得系统的CPU个数
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (int)info.dwNumberOfProcessors;
	}
	uint64 FileTimeToUTC(const FILETIME& ftime) //时间转换
	{
		LARGE_INTEGER li;
		li.LowPart = ftime.dwLowDateTime;
		li.HighPart = ftime.dwHighDateTime;
		return li.QuadPart;
	}
	//TCommContainer m_comContainer;
};

#endif //INCLUDED_PROCESS_METRICS_H