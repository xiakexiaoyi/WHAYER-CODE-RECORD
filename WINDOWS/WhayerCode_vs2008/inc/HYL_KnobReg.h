#ifndef _HYL_KNOBREG_H
#define _HYL_KNOBREG_H

#include"amcomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

	HYLAPI(MRESULT) HYL_KnobRegInit(MHandle hMemMgr, MHandle *pMRHandle);
	HYLAPI(MRESULT) HYL_KnobRegUninit(MHandle hMRHandle);
	HYLAPI(MRESULT) HYL_KnobRegSetParam(MHandle hMRHandle,char *modelfile,char *Filters0file,char *Filters1file);
	HYLAPI(MRESULT) HYL_KnobRegExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage, HYED_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif//_HYL_KNOBREG_H