#ifndef LINKREGGPU_HY_H
#define LINKREGGPU_HY_H


#include "LinkRecog_COM.h"

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

YOLOV2DLL_API int HYLR_Init_GPU(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYLR_Uninit_GPU(void *hMRHandle);
YOLOV2DLL_API int HYLR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int gpu_index, int w,int h);
YOLOV2DLL_API int HYLR_LinkRecog_GPU(void *hMRHandle,LR_IMAGES *pImg , HYLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif