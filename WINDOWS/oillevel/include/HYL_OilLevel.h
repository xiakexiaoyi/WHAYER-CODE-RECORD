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

HYLAPI(int) HY_OilLevel(OL_IMAGES *src,int *oil, int max, int min);
HYLAPI(int) HY_Oil(OL_IMAGES *src,int *oil);
HYLAPI(int) HY_Oilfit(OL_IMAGES *src);
HYLAPI(int) HY_whiteBlock(OL_IMAGES *src, int *oil);