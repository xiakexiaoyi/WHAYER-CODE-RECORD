#ifndef SWREG_HY_GPU_H
#define SWREG_HY_GPU_H


#include "SwitchRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

//__declspec(dllexport)
__declspec(dllexport) int HYSR_Init_GPU(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYSR_Uninit_GPU(void *hMRHandle);
__declspec(dllexport) int HYSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index);
__declspec(dllexport) int HYSR_SwitchRecog_GPU(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result);

#ifdef __cplusplus
}
#endif


#endif