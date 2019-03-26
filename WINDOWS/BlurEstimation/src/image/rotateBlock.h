#ifndef _ROTATEBLOCK_H_
#define _ROTATEBLOCK_H_

#include "licomdef.h"
#include "litrimfun.h"
#include"liblock.h"

#ifdef __cplusplus
extern "C" {
#endif	

MRESULT RotateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MLong lAngle);
MVoid RotateImage_EX(BLOCK *pSrc,BLOCK *pDst,MLong lAngle);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_ROTATEBLOCK_H_
