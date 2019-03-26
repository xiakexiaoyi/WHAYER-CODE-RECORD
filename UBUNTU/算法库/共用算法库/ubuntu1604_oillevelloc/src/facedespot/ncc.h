#if !defined(_NCC_H_)
#define _NCC_H_

#include "licomdef.h"

#define FastNCC			PX(FastNCC)
#define FSpecial		PX(FSpecial)
#define MoleMatch		PX(MoleMatch)

#ifdef __cplusplus
extern "C" {
#endif		

MVoid FastNCC(MVoid *pIntegral, MVoid *pSqIntegral, MLong lIntegralLine,
	MLong lSrcWidth, MLong lSrcHeight,
	MVoid* pTemplate, MLong lTemplWidth, MLong lTemplHeight,
	MVoid* pMask, MLong lMaskLine,//the same with RltWidth, lRltHeight
			MByte* pRlt, MLong lRltLine, MLong lRltWidth, MLong lRltHeight);

MVoid FSpecial(MByte *LoG, MLong lLine, MLong fSize);

MRESULT MoleMatch(MHandle hMemMgr, 
				  MVoid* pSrcImage, MLong lSrcLine, MLong lWidth, MLong lHeight,
				  MVoid* pMask, MLong lMaskLine,
				  MVoid* pMole, MLong lMoleLine,
				  MLong sStart, MLong sEnd, MLong sStep,
				  MLong lMemUsed);
	
#ifdef __cplusplus
}
#endif	

#endif // !defined(_NCC_H_)