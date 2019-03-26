#ifndef SWREG_HY_GPU_H
#define SWREG_HY_GPU_H


#include "SwitchRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

//__declspec(dllexport)
int HYSR_Init_GPU(void *hMemMgr, void **pMRHandle);
int HYSR_Uninit_GPU(void *hMRHandle);
int HYSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index);
int HYSR_SwitchRecog_GPU(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result);

#ifdef __cplusplus
}
#endif


#endif