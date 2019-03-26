#ifndef MDREG_HY_H
#define MDREG_HY_H
#include "meterDec_COM.h"
#ifdef __cplusplus
extern "C" {
#endif



 int HYMR_Init(void *hMemMgr, void **pMRHandle);
 int HYMR_Uninit(void *hMRHandle);
 int HYMR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
 int HYMR_meterRecog(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif
