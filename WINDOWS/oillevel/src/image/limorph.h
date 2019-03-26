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
///lFlag��ǰ�����������ֵ��lBgFlag�Ǳ������������ֵ
MRESULT Dilate(MHandle hMemMgr, MVoid* pMaskData,
			   MLong lMaskLine, MLong lWidth, 
			   MLong lHeight, MLong lSizeX, 
			   MLong lSizeY, MByte lFlag, MByte lBgFlag);

/*lFlag!=lBgFlag*/
///lFlag��ǰ�����������ֵ��lBgFlag�Ǳ������������ֵ
MRESULT Erose(MHandle hMemMgr, MVoid* pMaskData, 
			  MLong lMaskLine, MLong lWidth, 
			  MLong lHeight, MLong lSizeX, 
			  MLong lSizeY, MByte lFlag, MByte lBgFlag);
	
#ifdef __cplusplus
}
#endif

#endif