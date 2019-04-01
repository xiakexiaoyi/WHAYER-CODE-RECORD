#ifndef YOLODETECTGPU_H
#define YOLODETECTGPU_H

#include "detector_GPU.h"

#ifdef __cplusplus //ָ�������ı��뷽ʽ���Եõ�û���κ����εĺ�����
extern "C"
{
#endif

int init(void **handle, char *cfgfile, char *weightfile, float Thresh,int gpu_index);
int uninit(void *handle);
int detec(void *handle, unsigned char* imagdata, int height, int width, int channels, int widstep, int *numDST, float **boxout);
#ifdef __cplusplus
}
#endif

#endif