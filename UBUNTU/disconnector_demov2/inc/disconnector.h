#ifndef DISREG_HY_H
#define DISREG_HY_H

#include "disconnector_COM.h"



#ifdef __cplusplus
extern "C" {
#endif

 int HYDSR_Init(void *hMemMgr, void **pMRHandle);
 int HYDSR_Uninit(void *hMRHandle);
 int HYDSR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
 int HYDSR_StateRecog(void *hMRHandle,DSR_IMAGES *pImg,HYDSR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif
