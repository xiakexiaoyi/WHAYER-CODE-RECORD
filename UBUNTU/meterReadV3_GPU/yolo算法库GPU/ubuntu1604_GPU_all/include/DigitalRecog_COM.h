#ifndef NUMREG_COM_H
#define NUMREG_COM_H
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
} DR_IMAGES, *DR_PIMAGES;
typedef struct dr_point
{
	int x;
	int y;
} MDPOINT, *PMDPOINT;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} DRRECT, *PMDRECT;
typedef struct dr_result
{
	double dVal;///最后的值
	DRRECT    Target;///目标rectangle

}HYDR_RESULT;
typedef struct
{
	HYDR_RESULT *pResult;
	DRRECT *DRArea;
	int  lResultNum;
	int  lAreaNum;
	MDPOINT origin;
	MDPOINT offset;
}HYDR_RESULT_LIST;

#endif