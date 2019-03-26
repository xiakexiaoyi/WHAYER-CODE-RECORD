#ifndef _HYL_LIGHTOFF_H
#define _HYL_LIGHTOFF_H

#include"amcomdef.h"

#if defined(_MSC_VER)
#define YOLOV2DLL_API __declspec(dllexport)
#else
#define YOLOV2DLL_API __attribute ((visibility("default")))
#endif					  
#ifdef __cplusplus
extern "C" {
#endif

	YOLOV2DLL_API MRESULT HYL_LightOffExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage,HYED_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif//_HYL_LIGHTOFF_H