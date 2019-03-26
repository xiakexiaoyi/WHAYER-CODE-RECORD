#pragma once
#include"amcomdef.h"

typedef struct {
	int		lWidth;				// Off-screen width
	int 	lHeight;			// Off-screen height
	int		lPixelArrayFormat;	// Format of pixel array
	union
	{
		struct
		{
			int lLineBytes;
			void *pPixel;
		} chunky;
		struct
		{
			int lLinebytesArray[4];
			void *pPixelArray[4];
		} planar;
	} pixelArray;
} OL_IMAGES, *OL_PIMAGES;

#if defined(_MSC_VER)
#define YOLOV2DLL_API __declspec(dllexport)
#else
#define YOLOV2DLL_API __attribute ((visibility("default")))
#endif

YOLOV2DLL_API int HY_OilLevel(OL_IMAGES *src,int *oil, int max, int min);
YOLOV2DLL_API int HY_Oil(OL_IMAGES *src,int *oil);
YOLOV2DLL_API int HY_Oilfit(OL_IMAGES *src);
YOLOV2DLL_API int HY_whiteBlock(OL_IMAGES *src, int *oil);
