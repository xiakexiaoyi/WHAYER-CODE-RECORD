#ifndef AIRSWITCHREG_HY_H
#define AIRSWITCHREG_HY_H


#include "AirSwitchRecog_COM.h"

#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYAR_Init(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYAR_Uninit(void *hMRHandle);
HYLAPI(int) HYAR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
HYLAPI(int) HYAR_AirSwitchRecog(void *hMRHandle,AR_IMAGES *pImg , HYAR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif