/*!
* \file Ligaussian.h
* \brief  the function related to the gaussian blur 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#ifndef _LI_GAUSSIAN_H_
#define _LI_GAUSSIAN_H_

#include "licomdef.h"
#include"liblock.h"
/************************************************************************/
#define LiGaussian                PX(LiGaussian)
#define LiGaussianBlur         PX(LiGaussianBlur)
#define LiGaussianBlurF       PX(LiGaussianBlurF)
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif	
MRESULT LiGaussian(MHandle hMemMgr,PBLOCK BlockG,MFloat sigma);
MRESULT LiGaussianBlur(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc,MLong sigma);
MRESULT LiGaussianBlurF(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc,MLong sigma);

#ifdef __cplusplus
}
#endif

#endif