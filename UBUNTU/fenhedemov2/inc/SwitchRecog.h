#ifndef SWREG_HY_H
#define SWREG_HY_H



#include "SwitchRecog_COM.h"


#ifdef __cplusplus
extern "C" {
#endif

//__declspec(dllexport)
int HYSR_Init(void *hMemMgr, void **pMRHandle);
int HYSR_Uninit(void *hMRHandle);
int HYSR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh);
int HYSR_SwitchRecog(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result);

#ifdef __cplusplus
}
#endif


#endif