#ifndef LINKREG_COM_H
#define LINKREG_COM_H


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
} LR_IMAGES, *LR_PIMAGES;
typedef struct
{
	int x;
	int y;
} MLPOINT, *PMLPOINT;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} MLRECT, *PMLRECT;
typedef struct
{
	int flag;
	double dVal;///值
	MLRECT    Target;///目标rectangle

}HYLR_RESULT;
typedef struct
{
	HYLR_RESULT *pResult;
	MLRECT *LRArea;
	MLRECT MatchArea;
	int lMaxResultNum;  //最大数目
	int lRealResultNum;  //实际数目
	int  lResultNum;   //程序得到结果数目
	int  lAreaNum;//框定目标区域数目
	MLPOINT origin;
	MLPOINT offset;
}HYLR_RESULT_LIST;

#endif