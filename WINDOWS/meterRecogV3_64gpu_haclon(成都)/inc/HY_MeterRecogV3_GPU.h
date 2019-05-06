#ifndef _HYMETERRECOGV3_GPU_H
#define _HYMETERRECOGV3_GPU_H

#include "HY_MeterRecogV3_COM.h"
__declspec(dllexport) int yoloinit_GPU(void **handle, char *cfgfile, char *weightfile, float Thresh, int gpu_index);
__declspec(dllexport) int yolouninit_GPU(void *handle);
__declspec(dllexport) int MeterReadRecogV3_GPU(const void *handle, HYLPMRV3_IMAGES src, MV3Para Meterdescrip, HYMR_POINTERRESULT *MeterResult);

#endif