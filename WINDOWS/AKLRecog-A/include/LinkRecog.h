#ifndef LINKREG_HY_H
#define LINKREG_HY_H
#include "LinkRecog_COM.h"

#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYLR_Init(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYLR_Uninit(void *hMRHandle);
HYLAPI(int) HYLR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,  int w,int h);
HYLAPI(int) HYLR_LinkRecog(void *hMRHandle,LR_IMAGES *pImg , HYLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif