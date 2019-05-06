#ifndef _HYMETERRECOGV3_H
#define _HYMETERRECOGV3_H

#include "HY_MeterRecogV3_COM.h"
__declspec(dllexport) int yoloinit(void **handle, char *cfgfile, char *weightfile, float Thresh);
__declspec(dllexport) int yolouninit(void *handle);
__declspec(dllexport) int MeterReadRecogV3(const void *handle, HYLPMRV3_IMAGES src,  MV3Para Meterdescrip,HYMR_POINTERRESULT *MeterResult);

#endif