#ifndef KNOBREG_COM_H
#define KNOBREG_COM_H

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
} KR_IMAGES, *KR_PIMAGES;
typedef struct
{
	int x;
	int y;
} MKPOINT, *PMKPOINT;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} MKRECT, *PMKRECT;
typedef struct
{
	int flag;
	double dVal;///最后的值
	MKRECT    Target;///目标rectangle

}HYKR_RESULT;
typedef struct
{
	HYKR_RESULT *pResult;
	MKRECT *LRArea;
	MKRECT MatchArea;
	int lMaxResultNum;  //最大数目
	int lRealResultNum;  //实际数目
	int  lResultNum;   //程序得到结果数目
	int  lAreaNum;
	MKPOINT origin;
	MKPOINT offset;
}HYKR_RESULT_LIST;

#endif