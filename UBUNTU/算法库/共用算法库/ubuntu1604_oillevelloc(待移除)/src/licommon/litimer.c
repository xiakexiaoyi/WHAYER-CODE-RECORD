#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "litimer.h"

#include "lierrdef.h"
#include "limem.h"
#include "litrimfun.h"

#ifdef PLATFORM_GREENHILLS
typedef struct t_systime{
	short ltime; /* Time (lower 16 bits) */
	unsigned long utime; /* Time (higher 32 bits) */
}SYSTIME;
typedef struct t_systime  SYSTIME;
extern short     get_tim(SYSTIME *);
#elif defined WIN32
#include <time.h>
#endif

MLong	JGetCurrentTime()
{
#ifdef PLATFORM_SYMBIAN
	return MGetCurTimeStamp();
#elif defined PLATFORM_SOFTUNE
	return get_tick_count();
#elif  defined PLATFORM_COACH
	return _tx_counter_get()/54000;
#elif defined PLATFORM_ADS
	return clock()*10;
#elif defined PLATFORM_GREENHILLS
	SYSTIME timenow={0};
	get_tim(&timenow);
	return timenow.utime*1000+timenow.ltime;
#elif defined WIN32
	return clock();
#else
	return 0;
#endif
}

#ifdef ENABLE_TIMER

#ifdef PLATFORM_SOFTUNE
#elif  defined PLATFORM_SYMBIAN
#include "amkernel.h"
#include <stdio.h>
#else
#include <time.h>
#include <stdio.h>
#endif

#ifdef PLATFORM_SOFTUNE
extern MLong WriteLogFile(const char* path, const MVoid* data, MDWord size);
#define PRINT_STR(str, filename)	WriteLogFile("SD:\\DSC00001.txt", str, strlen(str))
#elif defined PLATFORM_COACH
extern int write_file( const void* data, unsigned long size, const char* path );
#define PRINT_STR(str, filename)	write_file(str, strlen(str),"I:\\DSC00001.txt");
#else
#define PRINT_STR(str, filename)											\
{																			\
	FILE*fp = fopen(filename, "a+");										\
	fprintf(fp, str);														\
	fclose(fp);																\
}
#endif

JSTATIC MLong g_nTime[TIMER_END]		= {0};
JSTATIC MLong g_nNumber[NUMBER_END]		= {0};

MVoid	JAddToTimer(MLong lTimeCur, ENUM_TIMER eTimer)
{
	g_nTime[eTimer] += lTimeCur;
}
MVoid	JIncreaseNumber(ENUM_NUMBER eNUmber)
{
	g_nNumber[eNUmber] ++;
}
MVoid	JPrintAllTimer()
{
	MTChar str[512];
	MTChar *strFileName = "D:\\logger.txt" ;

	sprintf(str, "Match Candidates: %d\n", g_nTime[TIMER_MATCH]);
	PRINT_STR(str, strFileName);
	sprintf(str, "Haar Response: %d\n", g_nTime[TIMER_HAARRESPONSE]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "Ransac circle: %d\n", g_nTime[TIMER_RANSACCIRCLE]);
	PRINT_STR(str, strFileName);
	sprintf(str, "Ransac circle circle detection: %d\n", g_nTime[TIMER_RANSACCIRCLE_CIRCLE]);
	PRINT_STR(str, strFileName);
	sprintf(str, "Ransac circle line detection: %d\n", g_nTime[TIMER_RANSACCIRCLE_LINE]);
	PRINT_STR(str, strFileName);
	sprintf(str, "Ransac circle fitting: %d\n", g_nTime[TIMER_CIRCLEFITTING]);
	PRINT_STR(str, strFileName);
	sprintf(str, "GetLocation: %d\n", g_nTime[TIMER_GETPIXELLOCATION]);
	PRINT_STR(str, strFileName);


	SetVectMem(g_nTime, TIMER_END, 0, MLong);
	SetVectMem(g_nNumber, NUMBER_END, 0, MLong);
}

#endif	//ENABLE_TIMER

//////////////////////////////////////////////////////////////////////////
//Call back
#ifdef ENABLE_TIMEOUT
#include "lidebug.h"

#define g_fnCallback        PX(g_fnCallback)
#define g_CallbackParam     PX(g_CallbackParam)
JFNPROGRESS g_fnCallback;
MVoid *g_CallbackParam;

MVoid CALLBACK_START(JFNPROGRESS fnCallBack, MVoid *pParam)
{
    g_fnCallback = fnCallBack;
    g_CallbackParam = pParam;
    JPrintf("Begin callback log\n");
}
MRESULT _CALLBACK_TICK(MVoid *pStrMeg)
{
    MRESULT res = LI_ERR_NONE;
    if (g_fnCallback != MNull)
    {
        JPrintf(pStrMeg);
        GO((*g_fnCallback)(0, 0, g_CallbackParam));
    }
EXT:
    return res;
}

#endif  //ENABLE_TIMEOUT
