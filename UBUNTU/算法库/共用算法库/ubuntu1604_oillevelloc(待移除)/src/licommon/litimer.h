#if !defined(_LI_TIME_H_)
#define _LI_TIME_H_

#include "licomdef.h"
#include "litrimfun.h"

#define PRINT_TIMER_ONEND

//NUMBER
typedef enum{
	NUMBER_BLOCK,	
	NUMBER_FLT_88,	
	NUMBER_PREFLT,	
	NUMBER_CPY_HW,	
	NUMBER_CPY	,	
	NUMBER_END
}ENUM_NUMBER;
//TIMER

typedef enum{
	TIMER_MATCH		,
	TIMER_GETPIXELLOCATION,
	TIMER_HAARRESPONSE,
	TIMER_RANSACCIRCLE,
	TIMER_CIRCLEFITTING,
	TIMER_RANSACCIRCLE_CIRCLE,
	TIMER_RANSACCIRCLE_LINE,
	TIMER_RANSACLINE,
	TIMER_END,
}ENUM_TIMER;
/*
typedef enum{ 
	TIMER_MASK_GENERATE	, 
	TIMER_CLEAN_FACEMASK, 
	TIMER_FACE_WHITENING, 
	TIMER_COPYFROMIMG	, 
	TIMER_PYD_ANALYSIS	, 
	TIMER_SHARP			, 
	TIMER_EST_CLEANLEVEL, 
	TIMER_SET_NOISE_IMG	, 
	TIMER_AUTO_SEED		, 
	TIMER_COLOR_MASK	, 
	TIMER_MASK_CLEAR	, 
	TIMER_FLT_NL		, 
	TIMER_FLT_SQNL		, 
	TIMER_COPYTOIMG		, 
	TIMER_FLT_LAPLA		, 
	TIMER_PYD_SYNTHESIS	, 
	TIMER_FLT_GAUSS		, 
	TIMER_EXPAND_88		, 
	TIMER_COMPUTE3X3WEIGHT, 
	TIMER_SMOOTH			, 
	TIMER_PydAnalysisOneLine_Arm, 
	TIMER_AnalysisBlock_U8_C	, 
	TIMER_ReduceBlock_U8		, 
	TIMER_OneWeightSum_B8_U8_U16, 
	TIMER_ResultFromWeightSum_B8_U8_U16, 
	TIMER_PydSynthesisOneLine_Arm	, 
	TIMER_COMPUTEOTHERWEIGHT		, 
	TIMER_ReduceLine_U8_Arm		, 
	TIMER_ReduceBlock_U8_C		, 
	TIMER_Integral				, 
	TIMER_EstVar				, 
	TIMER_LOOKUP				, 
	TIMER_DiffAbsSum_H8_U8_Arm	, 
	TIMER_DiffAbsSum_V8_U8_Arm	, 
	TIMER_DiffAbsSum_DL8_U8_Arm	, 
	TIMER_DiffAbsSum_DR8_U8_Arm	, 
	TIMER_DiffAbsSum_B8_U8_Arm	, 
	TIMER_WeightSum2_B8_U8_U16	, 
	TIMER_CatchEachConnectedMask, 
	TIMER_RemoveSmallSkin		, 
	TIMER_ExpandOneLine_Arm		, 
	TIMER_ExpandBlock			, 
	TIMER_ImgAdd				, 
	TIMER_ImgSubtract			, 
	TIMER_SmoothMeasure			, 
	TIMER_LoadYFromYUYV			, 
	TIMER_SaveYToYUYV			,
	TIMER_LOADBLOCKFROMIMAGE	, 
	TIMER_SAVEBLOCKTOIMAGE		,
	TIMER_END
}ENUM_TIMER;*/

#ifdef __cplusplus
extern "C" {
#endif

#define JGetCurrentTime         PX(JGetCurrentTime)
MLong	JGetCurrentTime();
	
#ifdef ENABLE_TIMER	

MVoid	JIncreaseNumber(ENUM_NUMBER eNUmber);
MVoid	JAddToTimer(MLong lTimeCur, ENUM_TIMER eTimer);
MVoid	JPrintAllTimer();

#else	//ENABLE_TIMER

#define JIncreaseNumber						(MVoid)
#define JAddToTimer							(MVoid)
#define JPrintAllTimer()

#endif	//ENABLE_TIMER

//////////////////////////////////////////////////////////////////////////
//Call back
#ifdef ENABLE_TIMEOUT
    #define CALLBACK_START          PX(CALLBACK_START)
    #define _CALLBACK_TICK          PX(_CALLBACK_TICK)

	MRESULT _CALLBACK_TICK(MVoid *pStrMeg);
	MVoid   CALLBACK_START(JFNPROGRESS fnCallBack, MVoid *pParam);

	#define CALLBACK_TICK(pStrMeg)				GO(_CALLBACK_TICK(pStrMeg))
	#define CALLBACK_TICK_RETURN(pStrMeg)       {MRESULT res = _CALLBACK_TICK(pStrMeg); if(res != LI_ERR_NONE) return res;}

#else  //!ENABLE_TIMEOUT
	
    #define CALLBACK_START                            (MVoid)
	#define CALLBACK_TICK                             (MVoid)
	#define CALLBACK_TICK_RETURN                      (MVoid)

#endif  //ENABLE_TIMEOUT


#ifdef ENABLE_PROCESS_CALLBACK
	#define CALLBACK_PROCESS(fnCallBack, lProgress, lStatus, pParam)    \
	{                                                                   \
		JASSERT(fnCallBack!=MNull);                                     \
		JPrintf("\n Call back, process %d\n", lProgress);               \
		GO((*fnCallBack)(lProgress, lStatus, pParam));                  \
	}
#else  //ENABLE_PROCESS_CALLBACK
	#define CALLBACK_PROCESS                          (MVoid)
#endif  //ENABLE_PROCESS_CALLBACK



#ifdef __cplusplus
}
#endif
#endif // !defined(_LI_TIME_H_)
