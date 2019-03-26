#ifndef AIRSWITCHREGGPU_HY_H
#define AIRSWITCHREGGPU_HY_H


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

YOLOV2DLL_API int HYAR_Init_GPU(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYAR_Uninit_GPU(void *hMRHandle);
YOLOV2DLL_API int HYAR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h);
YOLOV2DLL_API int HYAR_AirSwitchRecog_GPU(void *hMRHandle,AR_IMAGES *pImg , HYAR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif