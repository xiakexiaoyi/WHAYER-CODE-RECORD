#ifndef OLREG_HY_H
#define OLREG_HY_H

#include "oillevelRecog_COM.h"
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

YOLOV2DLL_API int HYOLR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYOLR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYOLR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
YOLOV2DLL_API int HYOLR_OilRecog(void *hMRHandle,OLR_IMAGES *pImg,HYOLR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif
