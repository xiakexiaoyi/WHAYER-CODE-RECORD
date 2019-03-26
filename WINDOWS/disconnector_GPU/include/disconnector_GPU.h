#ifndef DISREG_HYGPU_H
#define DISREG_HYGPU_H

#include "disconnector_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int HYDSR_Init_GPU(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYDSR_Uninit_GPU(void *hMRHandle);
__declspec(dllexport) int HYDSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h);
__declspec(dllexport) int HYDSR_StateRecog_GPU(void *hMRHandle,DSR_IMAGES *pImg,HYDSR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif