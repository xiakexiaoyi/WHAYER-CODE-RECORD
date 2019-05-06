#ifndef _LI_RESIZE_
#define _LI_RESIZE_

#include "licomdef.h"

typedef enum resize_type
{
	POINTSAMPLE =0,
	BILINEAR    =1,
	BICUBIC     =2,
} INTERPOLATE;

/***********************************************************/
#define  Resize		PX(Resize)
#define  MapToOrigin PX(MapToOrigin)
#define  Reduce		PX(Reduce)
/***********************************************************/

#ifdef __cplusplus
extern "C" {
#endif	

MVoid Resize(MVoid* pSrcData, MLong lSrcLine, 
			MLong lSrcWidth, MLong lSrcHeight, 
			MVoid* pRltData, MLong lRltLine,
			MLong lRltWidth, MLong lRltHeight,
			INTERPOLATE type);
MVoid Reduce(MVoid* pSrc, MLong lSrcLine, 
			 MLong lSrcWidth, MLong lSrcHeight, 
			 MVoid* pRlt, MLong lRltLine,
			 MLong lRltWidth, MLong lRltHeight);
MVoid MapToOrigin(MVoid* pSrc, MLong lSrcLine, 
				  MLong lSrcWidth, MLong lSrcHeight, 
				  MVoid* pRlt, MLong lRltLine,
			 MLong lRltWidth, MLong lRltHeight, MByte flag);
	
#ifdef __cplusplus
}
#endif

#endif