#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "liintegral.h"

#include "lierrdef.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "litrimfun.h"
#include "liimage.h"
#include "liblock.h"

#ifdef  OPTIMIZATION_SSE
#include "lisystem.h"
#include "liintegral_sse.h"
#endif

#ifdef  OPTIMIZATION_SSE4_1
#include "lisystem.h"
#include "liintegral_sse4.h"
#endif

#define INTEGRAL(pImgSrc,dwImgLine, pIntegral,pSqIntegral,					\
	dwIntegralLine,	dwWidth, dwHeight, lZoom,TYPE_IMG,TYPE_S,TYPE_SQS)		\
{																			\
	MDWord _x, _y;															\
	TYPE_IMG *_pImgCur = (TYPE_IMG*)pImgSrc;								\
	TYPE_S *_pIntegralPre = MNull, *_pIntegralCur = (TYPE_S*)pIntegral;		\
	TYPE_SQS *_pSqIntegralPre = MNull;										\
	TYPE_SQS *_pSqIntegralCur = (TYPE_SQS*)pSqIntegral;						\
	MLong  _s, _it;															\
	MLong _sq;																\
	MDWord _dwImgExt = (dwImgLine) - (dwWidth);								\
	MDWord _dwIntegralExt = (dwIntegralLine) - (dwWidth);					\
	JMemSet(_pIntegralCur, 0, (dwIntegralLine+1)*sizeof(TYPE_S));			\
	_pIntegralCur += (dwIntegralLine)+1;									\
	_pIntegralPre = _pIntegralCur;											\
	if(_pSqIntegralCur != MNull)											\
	{																		\
		JMemSet(_pSqIntegralCur, 0, (dwIntegralLine+1)*sizeof(TYPE_SQS));	\
		_pSqIntegralCur += (dwIntegralLine)+1;								\
		_pSqIntegralPre = _pSqIntegralCur;									\
	}																		\
	if(_pSqIntegralCur == MNull)											\
	{																		\
		_s=0;																\
		for(_x=0; _x< dwWidth;_x++)											\
		{																	\
			*(_pIntegralCur++) = (TYPE_S)(_s += *(_pImgCur++));				\
		}																	\
		_pImgCur += _dwImgExt;_pIntegralCur += _dwIntegralExt;				\
		for( _y=1; _y<dwHeight; _y++, _pImgCur += _dwImgExt,				\
			_pIntegralCur += _dwIntegralExt, _pIntegralPre += _dwIntegralExt)\
		{																	\
			_pIntegralCur[-1] = 0, _s = 0;									\
			for(_x=0; _x< dwWidth;_x++)										\
			{																\
				*(_pIntegralCur++) = (TYPE_S)(*(_pIntegralPre++)			\
					+ (_s += *(_pImgCur++)));								\
			}																\
		}																	\
	}																		\
	else																	\
	{																		\
		_s=0, _sq = 0;														\
		for(_x=0; _x< dwWidth;_x++)											\
		{																	\
			_it = *(_pImgCur++);											\
			*(_pIntegralCur++) = (TYPE_S)(_s+=_it);							\
			_it = DOWN_ROUND(_it, lZoom);									\
			*(_pSqIntegralCur++) = (TYPE_SQS)(_sq+=SQARE(_it));				\
		}																	\
		_pImgCur += _dwImgExt;												\
		_pIntegralCur += _dwIntegralExt, _pSqIntegralCur += _dwIntegralExt;	\
		for( _y=1; _y<dwHeight; _y++, _pImgCur += _dwImgExt,				\
			_pIntegralCur += _dwIntegralExt, _pIntegralPre += _dwIntegralExt,		\
			_pSqIntegralCur += _dwIntegralExt, _pSqIntegralPre += _dwIntegralExt)	\
		{																	\
			_pIntegralCur[-1] = 0, _s= 0;									\
			_pSqIntegralCur[-1] = 0, _sq = 0;								\
			for(_x=0; _x<dwWidth;_x++)										\
			{																\
				*(_pIntegralCur++) = (TYPE_S) (*(_pIntegralPre++)			\
					+ (_s+= (_it = *(_pImgCur++))));						\
				_it = DOWN_ROUND(_it, lZoom);								\
				*(_pSqIntegralCur++) = (TYPE_SQS) (*(_pSqIntegralPre++)		\
					+ (_sq+=SQARE(_it)));									\
			}																\
		}																	\
	}																		\
}

//////////////////////////////////////////////////////////////////////////
#ifdef TRIM_INTEGRAL
MVoid Integral(const MVoid *pImgSrc, MDWord dwImgLine, JTYPE_DATA_A typeDataA, 
			   JIntegral *pIntegral, JSqIntegral *pSqIntegral, MDWord dwIntegralLine, 
			   MDWord dwWidth, MDWord dwHeight)
{
	switch(typeDataA) {
	case DATA_U8:
#ifdef OPTIMIZATION_SSE4_1
		if(sizeof(JIntegral) == 2)
		{
			Integral_U8_SSE4((MUInt8*)pImgSrc, dwImgLine, 
							(MUInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
							dwWidth, dwHeight);
			break;
		}
#endif
#ifdef OPTIMIZATION_SSE
		if(sizeof(JIntegral) == 2)
		{
			Integral_U8_SSE((MUInt8*)pImgSrc, dwImgLine, 
							(MUInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
							dwWidth, dwHeight);
			break;
		}
#endif
		INTEGRAL(pImgSrc, dwImgLine, pIntegral, pSqIntegral, dwIntegralLine, 
			dwWidth, dwHeight, 0, MUInt8, JIntegral, JSqIntegral);
		break;
	case DATA_I8:
#ifdef OPTIMIZATION_SSE
		if(sizeof(JIntegral) == 2)
		{

			Integral_I8_SSE((MInt8*)pImgSrc, dwImgLine, 
				(MInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
				dwWidth, dwHeight);
				break;
		}
#endif
		INTEGRAL(pImgSrc, dwImgLine, pIntegral, pSqIntegral, dwIntegralLine, 
			dwWidth, dwHeight, 0, MInt8, JIntegral, JSqIntegral);
		break;
	case DATA_NONE:
	default:
		JASSERT(MFalse);
	}
}

MVoid Integral_UINT32(const MVoid *pImgSrc, MDWord dwImgLine, JTYPE_DATA_A typeDataA, 
			   MUInt32 *pIntegral, JSqIntegral *pSqIntegral, MDWord dwIntegralLine, 
			   MDWord dwWidth, MDWord dwHeight)
{
	switch(typeDataA) {
	case DATA_U8:
#ifdef OPTIMIZATION_SSE4_1
		if(sizeof(MUInt32) == 2)
		{
			Integral_U8_SSE4((MUInt8*)pImgSrc, dwImgLine, 
				(MUInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
				dwWidth, dwHeight);
			break;
		}
#endif
#ifdef OPTIMIZATION_SSE
		if(sizeof(MUInt32) == 2)
		{
			Integral_U8_SSE((MUInt8*)pImgSrc, dwImgLine, 
				(MUInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
				dwWidth, dwHeight);
			break;
		}
#endif
		INTEGRAL(pImgSrc, dwImgLine, pIntegral, pSqIntegral, dwIntegralLine, 
			dwWidth, dwHeight, 0, MUInt8, MUInt32, JSqIntegral);
		break;
	case DATA_I8:
#ifdef OPTIMIZATION_SSE
		if(sizeof(MUInt32) == 2)
		{
			
			Integral_I8_SSE((MInt8*)pImgSrc, dwImgLine, 
				(MInt16*)pIntegral, pSqIntegral, dwIntegralLine, 
				dwWidth, dwHeight);
			break;
		}
#endif
		INTEGRAL(pImgSrc, dwImgLine, pIntegral, pSqIntegral, dwIntegralLine, 
			dwWidth, dwHeight, 0, MInt8, MUInt32, JSqIntegral);
		break;
	case DATA_NONE:
	default:
		JASSERT(MFalse);
	}
}


#endif	//TRIM_INTEGRAL
//////////////////////////////////////////////////////////////////////////////
#define SMOOTH_BLOCK(pImgRlt, dwRltLine, pIntegral, dwIntegralLine,			\
	dwWidth, dwHeight, LEN, ZOOM_SHIFT, TYPE_IMG, TYPE_INTEGRAL)			\
{																			\
	MDWord x, y;															\
	MDWord winSize = LEN*LEN<<ZOOM_SHIFT;									\
	TYPE_IMG *pImgCur = (TYPE_IMG*)(pImgRlt) + (LEN/2)*((dwRltLine)+1);		\
	TYPE_INTEGRAL *pIntegralT = (TYPE_INTEGRAL*)(pIntegral);				\
	TYPE_INTEGRAL *pIntegralB = pIntegralT + LEN*(dwIntegralLine);			\
	MDWord dwImgExt = (dwRltLine) - (dwWidth) + LEN;						\
	MDWord dwIntegralExt = (dwIntegralLine) - (dwWidth) + LEN;				\
	for(y=dwHeight-LEN; y!=0; y--, pImgCur += dwImgExt,						\
		pIntegralT += dwIntegralExt, pIntegralB += dwIntegralExt)			\
	{																		\
		for(x=dwWidth-LEN; x!=0; x--, pIntegralT++, pIntegralB++)			\
		{																	\
			MLong lSum = (JIntegral)(pIntegralB[LEN] - pIntegralB[0]		\
				- pIntegralT[LEN] + pIntegralT[0]);							\
			lSum = DIV_ROUND(lSum, winSize);								\
			*(pImgCur++) = (TYPE_IMG)(lSum);								\
		}																	\
	}																		\
}

#define SMOOTH_BLOCK_F(pImgRlt, dwRltLine, pIntegral, dwIntegralLine,		\
	dwWidth, dwHeight, LEN_SHIFT, ZOOM_SHIFT, TYPE_IMG, TYPE_INTEGRAL)		\
{																			\
	MDWord x, y;															\
	MLong winLen = 1<<LEN_SHIFT;											\
	MLong lSizeShift = LEN_SHIFT*2 + ZOOM_SHIFT;							\
	TYPE_IMG *pImgCur = (TYPE_IMG*)(pImgRlt) + ((winLen)/2)*((dwRltLine)+1);	\
	TYPE_INTEGRAL *pIntegralT = (TYPE_INTEGRAL*)(pIntegral);				\
	TYPE_INTEGRAL *pIntegralB = pIntegralT + (winLen)*(dwIntegralLine);		\
	MDWord dwImgExt = (dwRltLine) - (dwWidth) + winLen;						\
	MDWord dwIntegralExt = (dwIntegralLine) - (dwWidth) + winLen;			\
	for(y=dwHeight-winLen; y!=0; y--, pImgCur += dwImgExt,					\
		pIntegralT += dwIntegralExt, pIntegralB += dwIntegralExt)			\
	{																		\
		for(x=dwWidth-winLen; x!=0; x--, pIntegralT++, pIntegralB++)		\
		{																	\
			MLong lSum = (JIntegral)(pIntegralB[winLen] - pIntegralB[0]		\
				- pIntegralT[winLen] + pIntegralT[0]);						\
			lSum = DOWN_FLOOR(lSum, lSizeShift);							\
			*(pImgCur++) = (TYPE_IMG)(lSum);								\
		}																	\
	}																		\
}

MRESULT SmoothBlock(MHandle hMemMgr, 
					MVoid *pImgSrc, MDWord dwSrcLine, JTYPE_DATA_A typeSrc, 
					MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeRlt, 
					MDWord dwWidth, MDWord dwHeight, MDWord dwLen)
{
	MVoid* pImgTmp = MNull, *pImgOffSet = MNull;
	MLong lTmpLine = JMemLength(dwWidth+dwLen);
	JIntegral *pIntegral=MNull;
	MLong lIntegralLine =  JMemLength(dwWidth+dwLen+1);
	MRESULT res = LI_ERR_NONE;
	MRECT rtValid;
	MLong lZoomST = IF_DATA_BITS(typeSrc) - IF_DATA_BITS(typeRlt);
	if(dwLen > dwWidth || dwLen > dwHeight)
		return res;	
	JASSERT(typeSrc == DATA_U8);

	AllocVectMem(hMemMgr, pImgTmp, lTmpLine*(dwHeight+dwLen)*IF_DATA_BYTES(typeSrc), MByte);
	rtValid.left = rtValid.top = dwLen/2;
	JImgMemCpy((MByte*)pImgTmp+rtValid.top*lTmpLine+rtValid.left, lTmpLine, 
		pImgSrc, dwSrcLine, dwWidth, dwHeight);
//	PrintBmp(pImgTmp, lTmpLine, typeSrc, dwWidth+dwLen, dwHeight+dwLen, 1);
	rtValid.right = dwWidth+dwLen-rtValid.left;
	rtValid.bottom = dwHeight+dwLen-rtValid.top;
	MirrorFill(pImgTmp, dwWidth+dwLen, dwHeight+dwLen, lTmpLine, 
		IF_DATA_TYPE(typeSrc), &rtValid);
//	PrintBmp(pImgTmp, lTmpLine, typeSrc, dwWidth+dwLen, dwHeight+dwLen, 1);

	AllocVectMem(hMemMgr, pIntegral, lIntegralLine*(dwHeight+dwLen+1), JIntegral);	
	Integral(pImgTmp, lTmpLine, typeSrc, pIntegral, MNull,lIntegralLine, 
		dwWidth+dwLen, dwHeight+dwLen);	

	pImgOffSet = (MByte*)pImgRlt-(dwRltLine+1)*dwLen/2*IF_DATA_BYTES(typeRlt);
	switch(typeRlt) {
	case DATA_U8:			
		if(IS_POW2(dwLen))
		{
			MLong lLenShift;
			LOG2_FLOOR(dwLen, lLenShift);
			SMOOTH_BLOCK_F(pImgOffSet, dwRltLine, pIntegral, lIntegralLine, 
				dwWidth+dwLen, dwHeight+dwLen, lLenShift, lZoomST, MUInt8, JIntegral);		
		}
		else
		{
			SMOOTH_BLOCK(pImgOffSet, dwRltLine, pIntegral, lIntegralLine, 
				dwWidth+dwLen, dwHeight+dwLen, dwLen, lZoomST, MUInt8, JIntegral);		
		}
		break;
#ifdef TRIM_DATA_INT8
	case DATA_I8:
		SMOOTH_BLOCK(pImgOffSet, dwRltLine, pIntegral, lIntegralLine, 
			dwWidth+dwLen, dwHeight+dwLen, dwLen, lZoomST, MInt8, JIntegral);		
		break;
#endif	//TRIM_DATA_INT8
	case DATA_NONE:
	default:
		JASSERT(MFalse);
		break;
	}
EXT:
	FreeVectMem(hMemMgr, pImgTmp);
	FreeVectMem(hMemMgr, pIntegral);
	return res;
}

