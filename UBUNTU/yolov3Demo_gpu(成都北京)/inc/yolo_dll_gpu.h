#ifndef YOLO_DLL_GPU_H
#define YOLO_DLL_GPU_H


#include "yolo_dll_com.h"

#ifdef __cplusplus
extern "C" {
#endif

YOLODLL_API int init_gpu(void **handle, char *cfgfile, char *weightfile,float thresh,int gpu_index);
YOLODLL_API int uninit_gpu(void *handle);
YOLODLL_API int detec_gpu(void *handle, YOLOV3_PIMAGES img, HYYOLOV3RESULT_PLIST pResultList);


#ifdef __cplusplus
}
#endif




#endif
