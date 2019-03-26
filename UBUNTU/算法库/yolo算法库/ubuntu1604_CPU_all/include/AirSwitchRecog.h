#ifndef AIRSWITCHREG_HY_H
#define AIRSWITCHREG_HY_H


#include "AirSwitchRecog_COM.h"

#if defined(_MSC_VER)
//#ifndef YOLOV2DLL_API
#define YOLOV2DLL_API __declspec(dllexport)
#else
//#ifndef YOLOV2DLL_API
#define YOLOV2DLL_API __attribute ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

YOLOV2DLL_API int HYAR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYAR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYAR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
YOLOV2DLL_API int HYAR_AirSwitchRecog(void *hMRHandle,AR_IMAGES *pImg , HYAR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif
