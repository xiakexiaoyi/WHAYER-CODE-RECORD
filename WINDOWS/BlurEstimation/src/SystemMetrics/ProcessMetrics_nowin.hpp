#ifndef INCLUDED_PROCESS_METRICS_H
#define INCLUDED_PROCESS_METRICS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <assert.h>
#include <string.h>
#define PROCESS_ITEM 14//进程CPU时间开始的项数
#define VMRSS_LINE 15//VMRSS所在行
class ProcessMetrics {
public:
	ProcessMetrics(){}
	~ProcessMetrics(){}
	int GetCPUUsage()//获得进程的CPU占用率
	{
		pid_t p=getpid();
		unsigned int totalcputime1,totalcputime2;
		unsigned int procputime1,procputime2;
		totalcputime1 = GetCpuTotalOccupy();
		procputime1 = GetCpuProcessOccupy(p);
		usleep(50000);
		totalcputime2 = GetCpuTotalOccupy();
		procputime2 = GetCpuProcessOccupy(p);
		int pcpu = 100.0*(procputime2 - procputime1)/(totalcputime2 - totalcputime1);
		return pcpu;
	}
	int GetMemoryUsage()//获得进程的内存使用情况
	{
		pid_t p=getpid();
		char file[64] = {0};
		FILE *fd;        
		char line_buff[256] = {0};  
		sprintf(file,"/proc/%d/status",p);                                                                                               
		fd = fopen (file, "r");
		int i;
		char name[32];
		int vmrss;
		for (i=0;i<VMRSS_LINE-1;i++)
		{
			fgets (line_buff, sizeof(line_buff), fd);
		}
		fgets(line_buff, sizeof(line_buff), fd);//读取VmRSS这一行的数据,VmRSS在第15行
		sscanf(line_buff, "%s %d", name,&vmrss);
		fclose(fd);     
		return vmrss/1024;
	}
private:
	unsigned int GetCpuTotalOccupy()
	{
		FILE *fd;         
		char buff[1024] = {0};  
		unsigned int user,nice,system,idle;                                                                                                         
		fd = fopen ("/proc/stat", "r"); 
		fgets (buff, sizeof(buff), fd); 
		char name[16];
		sscanf (buff, "%s %u %u %u %u", name, &user, &nice,&system, &idle);
		fclose(fd);    
		return (user + nice + system + idle);
	}
	unsigned int GetCpuProcessOccupy(const int p)
	{
		char file[64] = {0};
		pid_t pid;
		pid_t p_lin=p;
		unsigned int utime,stime,cutime,cstime;
		FILE *fd;        
		char line_buff[1024] = {0}; 
		sprintf(file,"/proc/%d/stat",p_lin);                                                                                             
		fd = fopen (file, "r"); 
		fgets (line_buff, sizeof(line_buff), fd); 
		sscanf(line_buff,"%u",&pid);
		char* q = GetItems(line_buff,PROCESS_ITEM);
		sscanf(q,"%u %u %u %u",&utime,&stime,&cutime,&cstime);
		fclose(fd);     
		return (utime + stime + cutime + cstime);
	}
	char* GetItems(char* buffer,int ie)
	{
		assert(buffer);
		char* p = buffer;
		int len = strlen(buffer);
		int count = 0;
		if (1 == ie || ie < 1)
		{
			return p;
		}
		int i;
		for (i=0; i<len; i++)
		{
			if (' ' == *p)
			{
				count++;
				if (count == ie-1)
				{
					p++;
					break;
				}
			}
			p++;
		}
		return p;
	}
};
#endif
