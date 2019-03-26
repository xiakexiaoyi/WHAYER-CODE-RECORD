#ifndef _PRIOR_AREA_H_
#define _PRIOR_AREA_H_

#include "licomdef.h"
#include "liimage.h"
#include "limask.h"

#define MASKV_SKIN		255
#define MAX_SEEDS_NUM	64

#define GRAY_VAL(lColorDist)	(255-(lColorDist)/4)

#ifdef __cplusplus
extern "C" {
#endif
	
	
MRESULT SkinMask(MHandle hMemMgr, const JOFFSCREEN *pImg, 
				 const MRECT *prtSkin, JMASK *pMskSkin, 
				 MCOLORREF *pcrSeeds, MLong *plSeedsNum);
MRESULT FaceMask(MHandle hMemMgr, const JOFFSCREEN *pImg, 
				 const MRECT *prtFaces, MLong lFaceNum, 
				 JMASK *pMskSkin, MCOLORREF *pcrSeeds, MLong *plSeedsNum);

MCOLORREF SeedColorByPriorArea(const JOFFSCREEN* pImg, const MRECT* prtSkin);
MVoid	  GrayMaskByColorSeed(const JOFFSCREEN* pImg, MCOLORREF crSeed, 
							  JMASK *pMskGray);


#ifdef __cplusplus
}
#endif

#endif //_PRIOR_AREA_H_
