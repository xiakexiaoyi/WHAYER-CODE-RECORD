#ifndef _HYL_PROTECTIONLINK_H
#define _HYL_PROTECTIONLINK_H

#include"amcomdef.h"
#include <opencv2\opencv.hpp>

#ifdef __cplusplus
extern "C" {
#endif

	HYLAPI(MRESULT) HYL_ProtectionLinkInit(MHandle hMemMgr, MHandle *pMRHandle);
	HYLAPI(MRESULT) HYL_ProtectionLinkUninit(MHandle hMRHandle);
	HYLAPI(MRESULT) HYL_ProtectionLinkSetParam(MHandle hMRHandle,char *modelfile,char *Filters0file,char *Filters1file);
	HYLAPI(MRESULT) HYL_ProtectionLinkExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage, HYED_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif//_HYL_PROTECTIONLINK_H