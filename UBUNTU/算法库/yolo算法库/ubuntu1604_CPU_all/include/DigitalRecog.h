#ifndef NUMREG_HY_H
#define NUMREG_HY_H


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

YOLOV2DLL_API int HYDR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYDR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYDR_SetParam(void *hMRHandle,const char *cfgfile,const char *weightfile,float Thresh,int w,int h);
YOLOV2DLL_API int HYDR_DigitRecog(void *hMRHandle,DR_IMAGES *pImg , HYDR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif
