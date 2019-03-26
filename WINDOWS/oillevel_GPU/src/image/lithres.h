/*!
* \file Lithres.h
* \brief  the function related to filter the image 
* \author bqj@whayer
* \version vision 1.0 
* \date 10 Sep 2015
*/


#if !defined(_LI_THRES_H_)
#define _LI_THRES_H_

#include "litrimfun.h"
#include "liblock.h"
#include <stdio.h>


/************************************************************************/
#define  AdaptiveThres               PX(AdaptiveThres)


/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	
MRESULT AdaptiveThres(MHandle hMemMgr,PBLOCK pBlockSrc,PBLOCK pBlockDst,
				MLong lBlocksizeW, MLong lBlocksizeH, int maxValue,double delta, int type);
MRESULT OtsuThres(PBLOCK pBlockSrc,PBLOCK pBlockDst,int type);
#ifdef __cplusplus
}
#endif
#endif // _LI_THRES_H_