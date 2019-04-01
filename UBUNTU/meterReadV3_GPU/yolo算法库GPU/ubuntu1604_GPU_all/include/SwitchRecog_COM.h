#ifndef SWREG_COM_H
#define SWREG_COM_H

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
} SR_IMAGES, *SR_PIMAGES;


typedef struct 
{
	int left;
	int top;
	int right;
	int bottom;
} SRECT, *SPRECT;

typedef struct 
{
	int  dlabel;/// 目标属性
	SRECT    rtTarget;///目标rectangle
	float dConfidence;
}HYSR_RESULT;///text read result


#endif