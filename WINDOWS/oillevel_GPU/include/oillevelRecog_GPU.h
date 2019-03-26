#ifndef OLREG_HYGPU_H
#define OLREG_HYGPU_H

#include "oillevelRecog_COM.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int HYOLR_Init_GPU(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYOLR_Uninit_GPU(void *hMRHandle);
__declspec(dllexport) int HYOLR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h);
__declspec(dllexport) int HYOLR_OilRecog_GPU(void *hMRHandle,OLR_IMAGES *pImg,HYOLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif