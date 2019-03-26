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

int HY_OilLevel(OL_IMAGES *src,int *oil, int max, int min);
int HY_Oil(OL_IMAGES *src,int *oil);
int HY_Oilfit(OL_IMAGES *src);
int HY_whiteBlock(OL_IMAGES *src, int *oil);
