#ifndef _ROTATEBLOCK_H_
#define _ROTATEBLOCK_H_

#include "licomdef.h"
#include "litrimfun.h"
#include"liblock.h"

//////////////////////////////////////////////////////
#define RotateImage				 PX(RotateImage)
#define RotateImage_EX			 PX(RotateImage_EX)
#define RotateImage_EX2          PX(RotateImage_EX2)
#define RotateImage_EX3          PX(RotateImage_EX3)
#define RotateImage_xsd          PX(RotateImage_xsd)
/////////////////////////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif	

MRESULT RotateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MLong lAngle);
MVoid RotateImage_EX(BLOCK *pSrc,BLOCK *pDst,MLong lAngle);
MVoid RotateImage_EX2(BLOCK *pSrc,BLOCK *pDst,MLong lAngle);
MVoid RotateImage_EX3(BLOCK *pSrc,BLOCK *pDst,MLong lAngle,BLOCK *xDstBlock,BLOCK *yDstBlock);
MVoid RotateImage_xsd(BLOCK *pSrc,BLOCK *pDst,MLong lAngle,BLOCK *xDstBlock,BLOCK *yDstBlock);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_ROTATEBLOCK_H_
