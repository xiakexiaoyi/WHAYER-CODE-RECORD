#ifndef NUMREG_HY_H
#define NUMREG_HY_H


#include "DigitalRecog_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

HYLAPI(int) HYDR_Init(void *hMemMgr, void **pMRHandle);
HYLAPI(int) HYDR_Uninit(void *hMRHandle);
HYLAPI(int) HYDR_SetParam(void *hMRHandle,const char *cfgfile,const char *weightfile,float Thresh,int w,int h);
HYLAPI(int) HYDR_DigitRecog(void *hMRHandle,DR_IMAGES *pImg , HYDR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif