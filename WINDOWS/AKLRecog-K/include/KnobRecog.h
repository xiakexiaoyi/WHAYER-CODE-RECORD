#ifndef KNOBREG_HY_H
#define KNOBREG_HY_H


#include "KnobRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYKR_Init(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYKR_Uninit(void *hMRHandle);
HYLAPI(int) HYKR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int w,int h);
HYLAPI(int) HYKR_KnobRecog(void *hMRHandle,KR_IMAGES *pImg , HYKR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif