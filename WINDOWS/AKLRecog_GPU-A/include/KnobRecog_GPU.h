#ifndef KNOBREGGPU_HY_H
#define KNOBREGGPU_HY_H


#include "KnobRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYKR_Init_GPU(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYKR_Uninit_GPU(void *hMRHandle);
HYLAPI(int) HYKR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int gpu_index, int w,int h);
HYLAPI(int) HYKR_KnobRecog_GPU(void *hMRHandle,KR_IMAGES *pImg , HYKR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif