// PerfMon.h: interface for the CPerfMon class.
//
#include "perfmon.h"
#ifdef WIN32
CPerfMon::CPerfMon(){}
CPerfMon::~CPerfMon()
{
	Uninitialize();
}

// Function name	: CPerfMon::Initialize
// Description	    : Initialize the query and memory
// Return type		: BOOL ; true on success; false on fail
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::Initialize()
{
	m_nNextIndex = 0;
	if (PdhOpenQuery(NULL, 1, &m_hQuery) != ERROR_SUCCESS)
		return false;
	return true;
}

// Function name	: CPerfMon::Uninitialize
// Description	    : Closes the query and fress all memory
// Return type		: void
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
void CPerfMon::Uninitialize()
{
	// clean memory	
	std::list<PPDHCOUNTERSTRUCT> ::iterator it ;
	PPDHCOUNTERSTRUCT	pdh ;
	for (it = m_aCounters.begin(); it != m_aCounters.end(); it ++)
	{
		pdh = *it;
		if (pdh != NULL)
		{
			delete pdh ;
		}		
	}
	m_aCounters.clear();
	PDH_STATUS ret=PdhCloseQuery(m_hQuery);
}

// Function name	: CPerfMon::AddCounter
// Description	    : Adds a counter to the query.
// Return type		: int ; -1 on fail, index to counter on success.
// Argument         : const char *pszCounterName
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
int CPerfMon::AddCounter(const char *pszCounterName)
{
	PPDHCOUNTERSTRUCT pCounter;
	pCounter = new PDHCOUNTERSTRUCT;
	if (!pCounter) 
		return -1;
	// add to current query
	if (PdhAddCounter(m_hQuery,(LPCSTR)pszCounterName, (DWORD)pCounter, &(pCounter->hCounter)) != ERROR_SUCCESS)
	{
		delete pCounter; // clean memory
		pCounter=NULL;
		return -1;
	}
	// insert counter into array(s)
	pCounter->nIndex = m_nNextIndex++;
	pCounter->lValue = 0;
	pCounter->nNextIndex = 0;
	pCounter->nOldestIndex = 0;
	pCounter->nRawCount = 0;
	m_aCounters.push_back(pCounter);
	return pCounter->nIndex;
}

// Function name	: CPerfMon::CollectQueryData
// Description	    : Collects the data for all the counters added with AddCounter()
// Return type		: BOOL ; false fail ; true success
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::CollectQueryData()
{
	if (PdhCollectQueryData(m_hQuery) != ERROR_SUCCESS) 
		return false;
	return true;
}


// Function name	: CPerfMon::UpdateValue
// Description	    : Updates the counter value for the counter in pCounter
// Return type		: BOOL ; false fail ; true success
// Argument         : PPDHCOUNTERSTRUCT pCounter
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
BOOL CPerfMon::UpdateValue(PPDHCOUNTERSTRUCT &pCounter)
{
	PDH_FMT_COUNTERVALUE pdhFormattedValue;

	LONG lre = 0;
	// get the value from the PDH
	if ((lre=PdhGetFormattedCounterValue(pCounter->hCounter, PDH_FMT_LONG, NULL, &pdhFormattedValue)) != ERROR_SUCCESS)
		return false;
	// test the value for validity
	if (pdhFormattedValue.CStatus != ERROR_SUCCESS)
		return false;
	// set value
	pCounter->lValue = pdhFormattedValue.longValue;
	return true;
}

// Function name	: CPerfMon::GetCounterValue
// Description	    : return the value of the counter
// Return type		: long ; -999 on failed ; value on success
// Argument         : int nIndex
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
int CPerfMon::GetCounterValue(int nIndex)
{
	PPDHCOUNTERSTRUCT pCounter = GetCounterStruct(nIndex);
	if (!pCounter)
		return 0;
	UpdateValue(pCounter);
	return pCounter->lValue;
}
// Function name	: CPerfMon::GetCounterStruct
// Description	    : Lookup a counterstruct based on the index
// Return type		: PPDHCOUNTERSTRUCT ; null on failed ; pointer to counter struct on success
// Argument         : int nIndex
//
// R E V I S I O N S:
// DATE       PROGRAMMER      CHANGES
//
PPDHCOUNTERSTRUCT CPerfMon::GetCounterStruct(int nIndex)
{
	if (nIndex < 0)
		return NULL;
	list<PPDHCOUNTERSTRUCT>::iterator it ;
	for (it = m_aCounters.begin(); it != m_aCounters.end(); it++)
	{
		nIndex -- ;
		if (nIndex < 0)
			return *it;
	}
	return NULL;
}
#endif



