#ifndef MDREGGPU_HY_H
#define MDREGGPU_HY_H
#include "meterDec_COM.h"
#ifdef __cplusplus
extern "C" {
#endif



__declspec(dllexport) int HYMR_Init_GPU(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYMR_Uninit_GPU(void *hMRHandle);
__declspec(dllexport) int HYMR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int gpu_index,int w,int h);
__declspec(dllexport) int HYMR_meterRecog_GPU(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif