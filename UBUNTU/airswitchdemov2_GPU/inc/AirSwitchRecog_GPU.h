#ifndef AIRSWITCHREGGPU_HY_H
#define AIRSWITCHREGGPU_HY_H


#include "AirSwitchRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYAR_Init_GPU(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYAR_Uninit_GPU(void *hMRHandle);
HYLAPI(int) HYAR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h);
HYLAPI(int) HYAR_AirSwitchRecog_GPU(void *hMRHandle,AR_IMAGES *pImg , HYAR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif