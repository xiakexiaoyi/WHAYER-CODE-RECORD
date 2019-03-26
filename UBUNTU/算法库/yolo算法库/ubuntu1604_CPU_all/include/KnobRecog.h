#ifndef KNOBREG_HY_H
#define KNOBREG_HY_H


#include "KnobRecog_COM.h"

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

YOLOV2DLL_API int HYKR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYKR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYKR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh, int w,int h);
YOLOV2DLL_API int HYKR_KnobRecog(void *hMRHandle,KR_IMAGES *pImg , HYKR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif


#endif
