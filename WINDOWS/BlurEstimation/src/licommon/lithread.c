#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif

#include "lithread.h"

#ifdef OPTIMIZATION_THREAD

extern MUInt32 CopWaitHandle(MHandle hCop);

MVoid DoWork_Parallel(fnThreadWork fnWork, MVoid **ppData, MLong lWorkNum)
{
#ifdef PLATFORM_COACH
	MLong i = 0;
	for(i=0; i<lWorkNum-1; i+=2)
	{
		MHandle hCop = RunThreadOnCopNB( fnWork, ppData[i]);
		fnWork(ppData[i+1]);
		CopWaitHandle(hCop);
	}
	if(i<lWorkNum)
		fnWork(ppData[i]);
#else
	MLong i;
	for(i=0; i<lWorkNum; i++)
		fnWork(ppData[i]);
#endif
}

#endif//OPTIMIZATION_THREAD