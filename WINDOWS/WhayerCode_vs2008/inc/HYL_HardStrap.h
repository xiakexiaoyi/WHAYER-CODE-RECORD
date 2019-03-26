#ifndef _HYL_HARDSTRAP_H
#define _HYL_HARDSTRAP_H

#include"amcomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

	HYLAPI(MRESULT) HYL_HardStrapInit(MHandle hMemMgr, MHandle *pMRHandle);
	HYLAPI(MRESULT) HYL_HardStrapUninit(MHandle hMRHandle);
	HYLAPI(MRESULT) HYL_HardStrapSetParam(MHandle hMRHandle,char *modelfile,char *Filters0file,char *Filters1file);
	HYLAPI(MRESULT) HYL_HardStrapExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage, HYED_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif//_HYL_HARDSTRAP_H