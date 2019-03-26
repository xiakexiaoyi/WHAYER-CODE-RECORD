#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "lisort.h"
#include "lidebug.h"

#define PARTITION(pVectData, lBegin, lEnd, DATA)							\
{																			\
	MLong i,j;																\
	MLong v,t;																\
	v = pVectData[lEnd];													\
	i = lBegin-1;															\
	j = lEnd;																\
	while(MTrue)															\
	{																		\
		while(pVectData[++i] < v);											\
		while((--j >= 0) && (v <= pVectData[j])) ;							\
		if ( i >= j ) break;												\
		t = pVectData[i];													\
		pVectData[i] = pVectData[j];										\
		pVectData[j] = (DATA)t;												\
	}																		\
	t = pVectData[i];														\
	pVectData[i] = pVectData[lEnd];											\
	pVectData[lEnd] = (DATA)t;												\
	return i;																\
}
 
#define FINDMEDIAN(pVectData, lBegin, lEnd, lMed, PartitionFun)				\
{																			\
	MLong lPivot = PartitionFun(pVectData,lBegin,lEnd);						\
	while (lPivot != lMed)													\
	{																		\
		if (lPivot > lMed)													\
		{																	\
			lEnd = lPivot-1;												\
		}																	\
		else																\
		{																	\
			lBegin = lPivot + 1;											\
		}																	\
		lPivot = PartitionFun(pVectData,lBegin,lEnd);						\
	}																		\
	return pVectData[lPivot];												\
}
//////////////////////////////////////////////////////////////////////////
JSTATIC MLong Partition_U8(MByte *pVectData, MLong lBegin, MLong lEnd);
JSTATIC MLong FindMidian_U8(MByte *pVarData, MLong lBegin, MLong lEnd, MLong lMed);
JSTATIC MLong Partition_U16(MWord *pVectData, MLong lBegin, MLong lEnd);
JSTATIC MLong FindMidian_U16(MWord *pVarData, MLong lBegin, MLong lEnd, MLong lMed);
//////////////////////////////////////////////////////////////////////////
MLong Partition_U8(MByte *pVectData, MLong lBegin, MLong lEnd)
{
	PARTITION(pVectData, lBegin, lEnd, MByte);
}
MLong FindMidian_U8(MByte *pVarData, MLong lBegin, MLong lEnd, MLong lMed)
{
	FINDMEDIAN(pVarData, lBegin, lEnd, lMed, Partition_U8);																	
}
MLong Partition_U16(MWord *pVectData, MLong lBegin, MLong lEnd)
{
	PARTITION(pVectData, lBegin, lEnd, MWord);
}
MLong FindMidian_U16(MWord *pVarData, MLong lBegin, MLong lEnd, MLong lMed)
{
	FINDMEDIAN(pVarData, lBegin, lEnd, lMed, Partition_U16);
}
MLong FindMidian(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData)
{
	switch(typeData)
	{
	case DATA_U8:
		return FindMidian_U8((MUInt8*)pVectData, 0, lVectLen-1, lVectLen/2);
	case DATA_U16:
		return FindMidian_U16((MWord*)pVectData, 0, lVectLen-1, lVectLen/2);
	default:
		JASSERT(MFalse);
	    return -1;
	}
}

#define FIND_MAX_INDEX(pVectData, lVectLen, TYPE)							\
{																			\
	TYPE *_pCur = (TYPE*)pVectData;											\
	MLong x, index=0;														\
	MLong lMaxValue = _pCur[0];												\
	for(x=1; x<lVectLen; x++)												\
	{																		\
		if(lMaxValue < _pCur[x])											\
		{																	\
			lMaxValue = _pCur[x];											\
			index = x;														\
		}																	\
	}																		\
	return index;															\
}
MLong FindMaxIndex(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData)
{
	switch(typeData)
	{
	case DATA_U16:
		FIND_MAX_INDEX(pVectData, lVectLen, MUInt16);
		break;
	default:
		JASSERT(MFalse);
		break;
	}
	return -1;
}

#ifndef TRIM_REDUNDANCE
#define FIND_MAX(pVectData, lVectLen, TYPE)									\
{																			\
	TYPE *_pCur = (TYPE*)pVectData;											\
	MLong x;																\
	MLong lMaxValue = _pCur[0];												\
	for(x=1; x<lVectLen; x++)												\
	{																		\
		if(lMaxValue < _pCur[x])											\
			lMaxValue = _pCur[x];											\
	}																		\
	return lMaxValue;														\
}
MLong FindMax(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData)
{
	switch(typeData)
	{
	case DATA_U16:
		FIND_MAX(pVectData, lVectLen, MUInt16);
		break;
	default:
		JASSERT(MFalse);
		break;
	}
	return -1;
}

MLong FindHistMidian(MVoid *pHistData, MLong lHistLen, JTYPE_DATA typeData)
{
	MLong lSumLeft = 0, lSumRight = 0;
	MLong lLeft = 0, lRight = lHistLen-1;
	MUInt16 *pHistCur = (MUInt16*)pHistData;
	JASSERT(typeData == DATA_U16);
	while(lLeft < lRight)
	{
		if(lSumLeft < lSumRight)
			lSumLeft += pHistCur[lLeft++];
		else
			lSumRight += pHistCur[lRight--];
	}
	return lLeft;
}
#endif//TRIM_REDUNDANCE