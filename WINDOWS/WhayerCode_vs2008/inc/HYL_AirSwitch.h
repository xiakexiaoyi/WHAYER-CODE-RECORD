#ifndef _HYL_AIRSWITCH_H
#define _HYL_AIRSWITCH_H

#include"amcomdef.h"


#ifdef __cplusplus
extern "C" {
#endif

	HYLAPI(MRESULT) HYL_AirSwitchInit(MHandle hMemMgr, MHandle *pMRHandle);
	HYLAPI(MRESULT) HYL_AirSwitchUninit(MHandle hMRHandle);
	HYLAPI(MRESULT) HYL_AirSwitchSetParam(MHandle hMRHandle,char *modelfile,char *Filters0file,char *Filters1file);
	HYLAPI(MRESULT) HYL_AirSwitchExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage, HYED_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif//_HYL_AIRSWITCH_H