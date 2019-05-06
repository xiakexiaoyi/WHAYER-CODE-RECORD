#ifndef _HYMETERRECOGV3_GPU_H
#define _HYMETERRECOGV3_GPU_H

#include "HY_MeterRecogV3_COM.h"

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

YOLOV2DLL_API int yoloinit_GPU(void **handle, char *cfgfile, char *weightfile, float Thresh, int gpu_index);
YOLOV2DLL_API int yolouninit_GPU(void *handle);
YOLOV2DLL_API int MeterReadRecogV3_GPU(const void *handle, HYLPMRV3_IMAGES src, MV3Para Meterdescrip, HYMR_POINTERRESULT *MeterResult);

#ifdef __cplusplus
}
#endif

#endif
