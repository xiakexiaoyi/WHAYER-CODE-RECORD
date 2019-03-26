#ifndef NUMREG_HY_GPU_H
#define NUMREG_HY_GPU_H

#include "DigitalRecog_COM.h"

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

YOLOV2DLL_API int HYDR_Init_GPU(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYDR_Uninit_GPU(void *hMRHandle);
YOLOV2DLL_API int HYDR_SetParam_GPU(void *hMRHandle,const char *cfgfile,const char *weightfile,float Thresh,int gpu_index,int w,int h);
YOLOV2DLL_API int HYDR_DigitRecog_GPU(void *hMRHandle,DR_IMAGES *pImg , HYDR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif