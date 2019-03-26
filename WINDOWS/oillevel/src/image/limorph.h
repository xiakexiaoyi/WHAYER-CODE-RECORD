#ifndef _LI_MORPH_
#define _LI_MORPH_

#include "licomdef.h"

/***********************************************************/
#define Dilate		PX(Dilate)
#define Erose		PX(Erose)
/***********************************************************/

#ifdef __cplusplus
extern "C" {
#endif	

/*lFlag!=lBgFlag*/
///lFlag是前景对象的像素值，lBgFlag是背景对象的像素值
MRESULT Dilate(MHandle hMemMgr, MVoid* pMaskData,
			   MLong lMaskLine, MLong lWidth, 
			   MLong lHeight, MLong lSizeX, 
			   MLong lSizeY, MByte lFlag, MByte lBgFlag);

/*lFlag!=lBgFlag*/
///lFlag是前景对象的像素值，lBgFlag是背景对象的像素值
MRESULT Erose(MHandle hMemMgr, MVoid* pMaskData, 
			  MLong lMaskLine, MLong lWidth, 
			  MLong lHeight, MLong lSizeX, 
			  MLong lSizeY, MByte lFlag, MByte lBgFlag);
	
#ifdef __cplusplus
}
#endif

#endif