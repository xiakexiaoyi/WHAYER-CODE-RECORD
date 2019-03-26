#ifndef MDREG_HY_H
#define MDREG_HY_H
#include "meterDec_COM.h"
#ifdef __cplusplus
extern "C" {
#endif



__declspec(dllexport) int HYMR_Init(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYMR_Uninit(void *hMRHandle);
__declspec(dllexport) int HYMR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
__declspec(dllexport) int HYMR_meterRecog(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif