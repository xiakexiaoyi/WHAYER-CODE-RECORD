#ifndef _HYMETERRECOGV3_H
#define _HYMETERRECOGV3_H

#include "HY_MeterRecogV3_COM.h"
#if defined(_MSC_VER)
#define YOLOV2DLL_API __declspec(dllexport)
#else
#define YOLOV2DLL_API __attribute ((visibility("default")))
#endif

YOLOV2DLL_API int yoloinit(void **handle, char *cfgfile, char *weightfile, float Thresh);
YOLOV2DLL_API int yolouninit(void *handle);
YOLOV2DLL_API int MeterReadRecogV3(const void *handle, HYLPMRV3_IMAGES src,  MV3Para Meterdescrip,HYMR_POINTERRESULT *MeterResult);

#endif
