#ifndef _LI_DEBUG_H_
#define _LI_DEBUG_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_ASSERT
#include <assert.h>
#define JASSERT assert
#else
#define JASSERT		(MVoid)
#endif

#define		PrintChannel			PX(PrintChannel)
#define    PrintBmp				PX(PrintBmp)
#define    PrintBmpEx			PX(PrintBmpEx)
	
//////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_DEBUG
	MVoid PrintChannel(const MVoid *pImg, MUInt32 dwImgLine, JTYPE_DATA_A typeDataA,
		MUInt32 dwWidth, MUInt32 dwHeight, MUInt32 dwChannelNum, MUInt32 dwChannelCur);
	MVoid PrintBmp(const MVoid *pImgData, MInt32 lImgLine, JTYPE_DATA_A typeDataA,
		MInt32 lWidth, MInt32 lHeight, MInt32 lChannelNum);
	MVoid PrintBmpEx(const MVoid *pImgData, MInt32 lImgLine, JTYPE_DATA_A typeDataA,
		MInt32 lWidth, MInt32 lHeight, MInt32 lChannelNum, const MTChar* szBmpName);
#else	//ENABLE_DEBUG
	#define PrintChannel			(MVoid)	
	#define PrintBmp				(MVoid)	
	#define PrintBmpEx				(MVoid)
#endif	//ENABLE_DEBUG

	//////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_LOG
	MVoid	Log_ger(MTChar *szFormat, ...);	
	MVoid	MemLog_ger(MHandle hMemMgr);
#else
	#define Log_ger								(MVoid)
	#define MemLog_ger							(MVoid)
#endif

#ifdef ENABLE_PRINT
	MVoid	JPrintf(MTChar *szFormat, ...);	
#else
	#define JPrintf		(MVoid)
#endif
	
#ifdef __cplusplus
}
#endif //__cplusplus

#endif	//_LI_DEBUG_H_