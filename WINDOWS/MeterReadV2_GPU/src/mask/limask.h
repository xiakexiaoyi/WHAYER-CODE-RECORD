#ifndef _LI_MASK_H_
#define _LI_MASK_H_

#include "licomdef.h"

//////////////////////////////////////////////////////////////////////////
typedef MByte	JMaskData;
typedef struct  
{
	JMaskData *pData;
	MLong lMaskLine;
	MLong lWidth, lHeight;
	MRECT rcMask;
} JMASK, *LPJMASK;

typedef MInt16 JCOLOR;
typedef struct tagTRegionItem
{
	MUInt16 area;
	JCOLOR  Color;
} TRegionItem;
#define IS_EDGE(lColorVal)	((lColorVal) >= 0x4000)

typedef MBool	(*FNMASK_COMPARE)(MLong lMaskVal, MLong lVal0);
typedef MLong	(*FNMASK_SET)(MLong lMaskVal);
//////////////////////////////////////////////////////////////////////////
#define MaskCreate		PX(MaskCreate)
#define MaskRelease		PX(MaskRelease)
#define MaskCpy			PX(MaskCpy)
#define MaskSet			PX(MaskSet)
#define Mask2Img		PX(Mask2Img)
#define Img2Mask		PX(Img2Mask)

#define MaskRange       PX(MaskRange)
#define MaskLineRange   PX(MaskLineRange)
#define MaskCenter		PX(MaskCenter)
#define MaskSize_NotZero   PX(MaskSize_NotZero)
#define MaskSize		PX(MaskSize)

#define MaskDilate      PX(MaskDilate)
#define MaskErode       PX(MaskErode)

#define MaskColor		PX(MaskColor)
#define MaskFillBySeed	PX(MaskFillBySeed)
#define MaskFill		PX(MaskFill)
#define MaskConvex		PX(MaskConvex)

#define MaskUnion		PX(MaskUnion)
#define MaskIntersect	PX(MaskIntersect)
#define MaskSub			PX(MaskSub)

#define RectTrim		PX(RectTrim)
#define RectUnion		PX(RectUnion)
#define RectIntersect	PX(RectIntersect)
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif	

MRESULT MaskCreate(MHandle hMemMgr, LPJMASK pMask, MLong lWidth, MLong lHeight);
MVoid	MaskRelease(MHandle hMemMgr, LPJMASK pMask);
MRESULT	MaskCpy(const JMASK* pMaskSrc, LPJMASK pMaskRlt);
MVoid	MaskSet(LPJMASK pMask, JMaskData mskVal);

MVoid	Mask2Img(const JMASK* pMask, PJOFFSCREEN pImg);
MVoid	Img2Mask(const JOFFSCREEN* pImg, JMASK* pMask);
//////////////////////////////////////////////////////////////////////////
MVoid   MaskRange(JMASK *pMask);
JPoint	MaskLineRange(JMaskData* pLineData, MLong lWidth, MLong lMskThreshold);
MLong	MaskSize(const JMASK* pMask, FNMASK_COMPARE fnCompare, MLong lVal0);
JPoint	MaskCenter(const JMASK *pMask);
//////////////////////////////////////////////////////////////////////////
MRESULT MaskDilate(MHandle hMemMgr, const JMASK *pMskSrc, JMASK *pMskDst, MLong Ksize);
MRESULT MaskErode(MHandle hMemMgr, const JMASK *pMskSrc, JMASK *pMskDst, MLong Ksize);

//////////////////////////////////////////////////////////////////////////
MVoid MaskColor(const JMaskData* pMskData, MLong lWidth, MLong lHeight, MLong lMskLine, 
	FNMASK_COMPARE fnCompare, MLong lVal0, JCOLOR* pColor, TRegionItem* pItem, MLong lItemNum);
MLong MaskFillBySeed(MHandle hMemMgr, JMASK *pMask, MLong lSeedX, MLong lSeedY, 
					 FNMASK_COMPARE fnCompare, MLong lVal0, FNMASK_SET fnFill, MLong lNewVal);
MLong MaskFill(JMASK *pMask, FNMASK_COMPARE fnCompare, MLong lVal0, 
			   FNMASK_SET fnFill, MLong lNewVal);
MVoid MaskConvex(JMASK *pMask, FNMASK_COMPARE fnCompare, MLong lVal0, 
				 FNMASK_SET fnFill, MLong lNewVal);
//////////////////////////////////////////////////////////////////////////
MVoid	MaskUnion(const JMASK* pMask1, const JMASK* pMask2, JMASK *pMaskRlt);
MVoid	MaskIntersect(const JMASK* pMask1, const JMASK* pMask2, JMASK *pMaskRlt);
MVoid	MaskSub(const JMASK* pMaskLeft, const JMASK* pMaskRight, JMASK *pMaskRlt);

MRECT	RectUnion(const MRECT* pRect1, const MRECT* pRect2);
MRECT	RectIntersect(const MRECT* pRect1, const MRECT* pRect2);
MVoid	RectTrim(MRECT *pRect, MLong lLeft, MLong lTop, MLong lRight, MLong lBottom);

#ifdef __cplusplus
}
#endif

#endif//_LI_MASK_H_

