#if !defined(_LI_INTEGRAL_H_)
#define _LI_INTEGRAL_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************/
#define Integral			PX(Integral)
#define Integral_UINT32		PX(Integral_UINT32)
#define SmoothBlock PX(SmoothBlock)
/************************************************/
	
typedef MInt16		JIntegral;
typedef MUInt32		JSqIntegral;
MVoid Integral(const MVoid *pImg, MUInt32 dwImgLine, JTYPE_DATA_A typeData, 
			   JIntegral *pIntegral, JSqIntegral *pSqIntegral, MUInt32 dwIntegralLine, 
			   MUInt32 dwWidth, MUInt32 dwHeight);

MVoid Integral_UINT32(const MVoid *pImg, MUInt32 dwImgLine, JTYPE_DATA_A typeData, 
			   MUInt32 *pIntegral, JSqIntegral *pSqIntegral, MUInt32 dwIntegralLine, 
			   MUInt32 dwWidth, MUInt32 dwHeight);

#define GetRectIntegral(pIntegral, lLine, lLeft, lRight, lTop, lBottom)			\
	((pIntegral)[(lBottom)*(lLine)+(lRight)] - (pIntegral)[(lTop)*(lLine)+(lRight)]	\
		+ (pIntegral)[(lTop)*(lLine)+(lLeft)]- (pIntegral)[(lBottom)*(lLine)+(lLeft)])

MRESULT SmoothBlock(MHandle hMemMgr, 
					MVoid *pDataSrc, MDWord dwSrcLine, JTYPE_DATA_A typeSrc, 
					MVoid *pDataRlt, MDWord dwRltLine, JTYPE_DATA_A typeRlt, 
					MDWord dwWidth, MDWord dwHeight, MDWord dwLen);

#ifdef __cplusplus
}
#endif

#endif // !defined(_LI_INTEGRAL_H_)
