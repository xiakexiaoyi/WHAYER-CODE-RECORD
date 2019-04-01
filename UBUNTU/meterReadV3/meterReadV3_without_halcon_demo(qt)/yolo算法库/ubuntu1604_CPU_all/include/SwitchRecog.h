#ifndef SWREG_HY_H
#define SWREG_HY_H



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
YOLOV2DLL_API int HYSR_Init(void *hMemMgr, void **pMRHandle);
YOLOV2DLL_API int HYSR_Uninit(void *hMRHandle);
YOLOV2DLL_API int HYSR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh);
YOLOV2DLL_API int HYSR_SwitchRecog(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result);

#ifdef __cplusplus
}
#endif


#endif
