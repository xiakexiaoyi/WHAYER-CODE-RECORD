#ifndef _HYMETERRECOGV3_H
#define _HYMETERRECOGV3_H

#include "HY_MeterRecogV3_COM.h"
int yoloinit(void **handle, char *cfgfile, char *weightfile, float Thresh);
int yolouninit(void *handle);
int MeterReadRecogV3(const void *handle, HYLPMRV3_IMAGES src,  MV3Para Meterdescrip,HYMR_POINTERRESULT *MeterResult);

#endif
