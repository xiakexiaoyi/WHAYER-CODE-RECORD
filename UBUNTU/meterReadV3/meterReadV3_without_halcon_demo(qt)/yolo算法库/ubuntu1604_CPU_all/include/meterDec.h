#ifndef MDREG_HY_H
#define MDREG_HY_H
#include "meterDec_COM.h"

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



YOLOV2DLL_API int HYMR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYMR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYMR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
YOLOV2DLL_API int HYMR_meterRecog(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif
