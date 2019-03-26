#ifndef OLREG_HY_H
#define OLREG_HY_H

#include "oillevelRecog_COM.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int HYOLR_Init(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYOLR_Uninit(void *hMRHandle);
__declspec(dllexport) int HYOLR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
__declspec(dllexport) int HYOLR_OilRecog(void *hMRHandle,OLR_IMAGES *pImg,HYOLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif