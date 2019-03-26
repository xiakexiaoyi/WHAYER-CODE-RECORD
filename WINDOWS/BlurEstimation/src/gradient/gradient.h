#ifndef _GRADIENT_H_
#define _GRADIENT_H_

#include "licomdef.h"
#include "litrimfun.h"
//#include"liblock.h"

#ifdef __cplusplus
extern "C" {
#endif	

	MLong GetAngle(MInt32 lY,MInt32 lX);
	MVoid CreateGradAngleImg(const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MUInt16 *pAngle);
	MLong GetSin(MLong angle);
	MLong GetCosA(MLong angle);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_GRADIENT_H_
