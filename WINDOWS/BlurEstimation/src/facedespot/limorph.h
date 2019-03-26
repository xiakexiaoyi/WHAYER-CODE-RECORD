#ifndef _LI_MORPH_
#define _LI_MORPH_

#include "licomdef.h"

/***********************************************************/
#define Dilate		PX(Dilate)
#define Erose		PX(Erose)
#define Dilate_KeepOthers	PX(Dilate_KeepOthers)
/***********************************************************/

#ifdef __cplusplus
extern "C" {
#endif	

/*lFlag!=255*/
MRESULT Dilate(MHandle hMemMgr, MVoid* pMaskData,
			   MLong lMaskLine, MLong lWidth, 
			   MLong lHeight, MLong lSizeX, 
			   MLong lSizeY, MByte lFlag/*=0*/);

/*lFlag!=255*/
MRESULT Erose(MHandle hMemMgr, MVoid* pMaskData, 
			  MLong lMaskLine, MLong lWidth, 
			  MLong lHeight, MLong lSizeX, 
			  MLong lSizeY, MByte lFlag/*=0*/);
MRESULT Dilate_KeepOthers(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
						  MLong lWidth, MLong lHeight, 
						  MLong lSizeX, MLong lSizeY, 
			   MByte lFlag/*=0*/);
	
#ifdef __cplusplus
}
#endif

#endif