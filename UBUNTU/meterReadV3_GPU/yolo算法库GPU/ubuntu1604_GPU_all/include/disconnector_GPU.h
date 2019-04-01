#ifndef DISREG_HYGPU_H
#define DISREG_HYGPU_H

#include "disconnector_COM.h"

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

YOLOV2DLL_API int HYDSR_Init_GPU(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYDSR_Uninit_GPU(void *hMRHandle);
YOLOV2DLL_API int HYDSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h);
YOLOV2DLL_API int HYDSR_StateRecog_GPU(void *hMRHandle,DSR_IMAGES *pImg,HYDSR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif
