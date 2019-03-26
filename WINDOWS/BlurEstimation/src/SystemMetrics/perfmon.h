#ifndef _PERFMON_H
#define _PERFMON_H
#ifdef WIN32
#include <pdh.h>
#include <list>
#include <map>
#include <string>
#include <stdio.h>
#include <Tlhelp32.h>
#include <pdhmsg.h>
#include<iostream>
#pragma comment(lib, "pdh.lib")
using namespace std;

#define MAX_RAW_VALUES 20
#define CNTR_CPU "\\Processor(_Total)\\% Processor Time" // % of cpu in use
typedef struct _tag_PDHCounterStruct {
	int nIndex;				// The index of this counter, returned by AddCounter()
	LONG lValue;			// The current value of this counter
	HCOUNTER hCounter;      // Handle to the counter - given to use by PDH Library
	int nNextIndex;         // element to get the next raw value
	int nOldestIndex;       // element containing the oldes raw value
	int nRawCount;          // number of elements containing raw values
	double dValue;
	PDH_RAW_COUNTER a_RawValue[MAX_RAW_VALUES]; // Ring buffer to contain raw values
} PDHCOUNTERSTRUCT, *PPDHCOUNTERSTRUCT;

class CPerfMon
{
public:
	CPerfMon();
	~CPerfMon();
	BOOL Initialize(void);
	BOOL CollectQueryData(void);
	int GetCounterValue(int nIndex);
	int  AddCounter(const char *pszCounterName);
private:
	void Uninitialize(void);
	PPDHCOUNTERSTRUCT GetCounterStruct(int nIndex);
	BOOL UpdateValue(PPDHCOUNTERSTRUCT &pCounter);
private:
	HQUERY m_hQuery; // the query to the PDH
	int m_nNextIndex;
	list<PPDHCOUNTERSTRUCT>	m_aCounters;
	map<string, PPDHCOUNTERSTRUCT>	m_aProcessCounters;
};
#endif
#endif
