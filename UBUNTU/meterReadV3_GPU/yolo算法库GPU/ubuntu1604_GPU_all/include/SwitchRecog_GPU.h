#ifndef SWREG_HY_GPU_H
#define SWREG_HY_GPU_H


#include "SwitchRecog_COM.h"


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

//__declspec(dllexport)
YOLOV2DLL_API int HYSR_Init_GPU(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYSR_Uninit_GPU(void *hMRHandle);
YOLOV2DLL_API int HYSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index);
YOLOV2DLL_API int HYSR_SwitchRecog_GPU(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result);

#ifdef __cplusplus
}
#endif


#endif