#ifndef LINKREGGPU_HY_H
#define LINKREGGPU_HY_H


#include "LinkRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYLR_Init_GPU(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYLR_Uninit_GPU(void *hMRHandle);
HYLAPI(int) HYLR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int gpu_index, int w,int h);
HYLAPI(int) HYLR_LinkRecog_GPU(void *hMRHandle,LR_IMAGES *pImg , HYLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif