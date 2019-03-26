
/*!
* \file LiBlur.h
* \brief  the function related to the calculate the blur kernel 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#ifndef _LI_BLUR_H_
#define _LI_BLUR_H_

#include "licomdef.h"
#include"liblock.h"
/************************************************************************/
#define  LiBlur  			        PX(LiBlur)
#define  Lisigma					PX(Lisigma)
#define  SigmaSel			    PX(SigmaSel)
#define  SelePlainBlock      PX(SelePlainBlock)
#define  SeleBlock              PX(SeleBlock)
#define  CalSigmaTwoGaussBlur PX(CalSigmaTwoGaussBlur)
#define  CalImgPieceWeight PX(CalImgPieceWeight)
/************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif	
	MRESULT LiBlur(MHandle hMemMgr,PBLOCKEXT blockres, PBLOCKEXT blocksrc, PBLOCKEXT blockedge,PBLOCKEXT blocka,PBLOCKEXT blockb);
	MRESULT Lisigma(MHandle hMemMgr,MFloat *sigma,PBLOCKEXT blocksrc,MLong sigmaa,MLong sigmab);
	MRESULT SigmaSel(MFloat *sigres,MFloat sigma1,MFloat sigma2);	
	MRESULT SelePlainBlock(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc);
	MRESULT SeleBlock(MHandle hMemMgr,PBLOCKEXT block1,PBLOCKEXT block2);
    MRESULT CalSigmaTwoGaussBlur(MHandle hMemMgr,MFloat *pSigma,PBLOCKEXT pBlocksrc,PBLOCKEXT pBlockedge,
		                                                MLong sigmaa,MLong sigmab);
	MRESULT CalImgPieceWeight(MHandle hMemMgr,MFloat *pWeight,PBLOCKEXT pBlockedge);
	MRESULT EdgeMap(MHandle hMemMgr,PBLOCKEXT pBinaryImg,PBLOCKEXT pGrayImg,float fThresh);
#ifdef __cplusplus
}
#endif

#endif