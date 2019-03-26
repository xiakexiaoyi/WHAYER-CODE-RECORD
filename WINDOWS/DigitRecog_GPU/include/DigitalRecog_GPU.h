#ifndef NUMREG_HY_GPU_H
#define NUMREG_HY_GPU_H

#include "DigitalRecog_COM.h"


#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYDR_Init_GPU(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYDR_Uninit_GPU(void *hMRHandle);
HYLAPI(int) HYDR_SetParam_GPU(void *hMRHandle,const char *cfgfile,const char *weightfile,float Thresh,int gpu_index,int w,int h);
HYLAPI(int) HYDR_DigitRecog_GPU(void *hMRHandle,DR_IMAGES *pImg , HYDR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif