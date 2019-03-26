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
	MTChar *strFileName = "C:\\logger.txt" ;

	sprintf(str, "MASK_GENERATE: %d\n", g_nTime[TIMER_MASK_GENERATE]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----AUTO_SEED: %d\n", g_nTime[TIMER_AUTO_SEED]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "----ColorMask: %d\n", g_nTime[TIMER_COLOR_MASK]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----RemoveSmallSkin: %d\n", g_nTime[TIMER_RemoveSmallSkin]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----ClearMaskFlag: %d\n", g_nTime[TIMER_MASK_CLEAR]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----CatchEachConnectedMask: %d\n", g_nTime[TIMER_CatchEachConnectedMask]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----Smooth: %d\n", g_nTime[TIMER_SMOOTH]);
	PRINT_STR(str, strFileName);

	sprintf(str, "CLEAN_FACEMASK(%d): %d\n", g_nNumber[NUMBER_BLOCK], g_nTime[TIMER_CLEAN_FACEMASK]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----COPYFROMIMG: %d\n", g_nTime[TIMER_COPYFROMIMG]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "----ExpandBlock: %d\n", g_nTime[TIMER_ExpandBlock]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------ExpandOneLine_Arm: %d\n", g_nTime[TIMER_ExpandOneLine_Arm]);	
	PRINT_STR(str, strFileName);	
	sprintf(str, "----PYD_ANALYSIS: %d\n", g_nTime[TIMER_PYD_ANALYSIS]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------ImgSubtract: %d\n", g_nTime[TIMER_ImgSubtract]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------ReduceBlock_U8: %d\n", g_nTime[TIMER_ReduceBlock_U8]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "------------ReduceLine_U8_Arm: %d\n", g_nTime[TIMER_ReduceLine_U8_Arm]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "------------ReduceBlock_U8_C: %d\n", g_nTime[TIMER_ReduceBlock_U8_C]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------AnalysisBlock_U8_C: %d\n", g_nTime[TIMER_AnalysisBlock_U8_C]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------PydAnalysisOneLine_Arm: %d\n", g_nTime[TIMER_PydAnalysisOneLine_Arm]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "----SHARP: %d\n", g_nTime[TIMER_SHARP]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----EST_CLEANLEVEL: %d\n", g_nTime[TIMER_EST_CLEANLEVEL]);
	PRINT_STR(str, strFileName);
	sprintf(str, "--------Integral: %d\n", g_nTime[TIMER_Integral]);
	PRINT_STR(str, strFileName);
	sprintf(str, "--------EstVar: %d\n", g_nTime[TIMER_EstVar]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----SET_NOISE_IMG: %d\n", g_nTime[TIMER_SET_NOISE_IMG]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "----FLT_LAPLA: %d\n", g_nTime[TIMER_FLT_LAPLA]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "----PYD_SYNTHESIS: %d\n", g_nTime[TIMER_PYD_SYNTHESIS]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------ImgAdd: %d\n", g_nTime[TIMER_ImgAdd]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------PydSynthesisOneLine_Arm: %d\n", g_nTime[TIMER_PydSynthesisOneLine_Arm]);
	PRINT_STR(str, strFileName);		
	sprintf(str, "----FLT_GAUSS: %d\n", g_nTime[TIMER_FLT_GAUSS]);
	PRINT_STR(str, strFileName);
	sprintf(str, "--------FLT_NL: %d\n", g_nTime[TIMER_FLT_NL]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "--------FLT_SQNL(%d): %d\n", g_nNumber[NUMBER_FLT_88], g_nTime[TIMER_FLT_SQNL]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------SmoothMeasure: %d\n", g_nTime[TIMER_SmoothMeasure]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "------------Expand_88(%d): %d\n", g_nNumber[NUMBER_PREFLT], g_nTime[TIMER_EXPAND_88]);
	PRINT_STR(str, strFileName);	
	sprintf(str, "------------DiffAbsSum_H8_U8_Arm: %d\n", g_nTime[TIMER_DiffAbsSum_H8_U8_Arm]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------DiffAbsSum_V8_U8_Arm: %d\n", g_nTime[TIMER_DiffAbsSum_V8_U8_Arm]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------DiffAbsSum_DL8_U8_Arm: %d\n", g_nTime[TIMER_DiffAbsSum_DL8_U8_Arm]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------DiffAbsSum_DR8_U8_Arm: %d\n", g_nTime[TIMER_DiffAbsSum_DR8_U8_Arm]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------DiffAbsSum_B8_U8_Arm: %d\n", g_nTime[TIMER_DiffAbsSum_B8_U8_Arm]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------Loopup: %d\n", g_nTime[TIMER_LOOKUP]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------OneWeightSum_B8_U8_U16: %d\n", g_nTime[TIMER_OneWeightSum_B8_U8_U16]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------TwoWeightSum_B8_U8_U16: %d\n", g_nTime[TIMER_WeightSum2_B8_U8_U16]);
	PRINT_STR(str, strFileName);
	sprintf(str, "------------ResultFromWeightSum_B8_U8_U16: %d\n", g_nTime[TIMER_ResultFromWeightSum_B8_U8_U16]);
	PRINT_STR(str, strFileName);
	sprintf(str, "----COPYTOIMG: %d\n", g_nTime[TIMER_COPYTOIMG]);
	PRINT_STR(str, strFileName);

	sprintf(str, "FACE_WHITENING: %d\n", g_nTime[TIMER_FACE_WHITENING]);
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
