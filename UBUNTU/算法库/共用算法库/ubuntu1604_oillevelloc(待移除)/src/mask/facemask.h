#ifndef _FACE_MASK_H_
#define _FACE_MASK_H_

#include "licomdef.h"
#include "limask.h"

#define MASKV_BACK					0
#define MASKV_TOOTH					1
#define MASKV_EYE					2
#define MASKV_MIN_SKIN				8

typedef enum{
	FM_ORIGINAL, 
	FM_CLEAN
} JENUM_FACEMASK;

typedef struct  
{
	PJPoint pptSeed;
	MCOLORREF *pcrSeed;
	MLong lSeedNum;	
} JSEEDS, *PJSEEDS;

#define COLOR_EXT			6
#define MIN_CB				(85-10)
#define MAX_CB				(135+6)
#define MIN_CR				(135-7)
#define MAX_CR				(180+10)
#define MAX_Y				256
#define MIN_Y				32

#define IS_SKIN_COLOR(y, cb, cr)			\
	((cb) >= MIN_CB && (cb) < MAX_CB &&		\
	 (cr) >= MIN_CR && (cr) < MAX_CR &&		\
	 (y)  >= MIN_Y  && (y)  < MAX_Y  &&     \
     (cb) <= (cr))


#define CatchEachConnectedMask	PX(CatchEachConnectedMask)
#define ClearMaskFlag			PX(ClearMaskFlag)
#define FaceMaskFromFaceRect	PX(FaceMaskFromFaceRect)
#define SkinMaskFromFaceRect	PX(SkinMaskFromFaceRect)
#define SmoothMask				PX(SmoothMask)
#define MidColor				PX(MidColor)
#define GenerateSeeds			PX(GenerateSeeds)
#define FilterSeeds				PX(FilterSeeds)
#define ReduceImage				PX(ReduceImage)
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif	

MRESULT FaceMaskFromFaceRect(MHandle hMemMgr, const JOFFSCREEN* pFaceImg, 
	const MRECT* pFaceRect, MLong lFacesNum, MLong lMskReduceW, MLong lMskReduceH, 
	JMASK *pFaceMask, JENUM_FACEMASK eFaceMask);
MRESULT SkinMaskFromFaceRect(MHandle hMemMgr, const JOFFSCREEN* pFaceImg, 
	const MRECT* pFaceRect, MLong lFacesNum, MLong lMskReduceW, MLong lMskReduceH, 
	JMASK *pFaceMask);

MVoid	ClearMaskFlag(JMASK *pMask);

MRESULT SmoothMask(MHandle hMemMgr, 
				   const JMASK* pMaskSrc, JMASK* pMaskRlt,
                   MLong dwLen);
MLong	CatchEachConnectedMask(MHandle hMemMgr, JMASK* pMaskFrom, JMASK* pMaskTo,
							   MLong* plStartRow, MLong *plSum);

//////////////////////////////////////////////////////////////////////////
MCOLORREF MidColor(MHandle hMemMgr, const MCOLORREF *pcrSeed, MLong lSeedNum);
MRESULT	GenerateSeeds(const JOFFSCREEN* pImg, const MRECT *pFaceRect,
					  PJSEEDS pSeeds, MLong lInterFaceOffset);
MRESULT FilterSeeds(MHandle hMemMgr, MCOLORREF crMid, PJSEEDS pSeeds, 
					MLong lGroupStep);
//////////////////////////////////////////////////////////////////////////
MVoid ReduceImage(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
                  MLong lReduceW, MLong lReduceH);

#ifdef __cplusplus
}
#endif

#endif//_FACE_MASK_H_

