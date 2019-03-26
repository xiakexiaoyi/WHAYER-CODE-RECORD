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
	double dVal;///ֵ
	MLRECT    Target;///Ŀ��rectangle

}HYLR_RESULT;
typedef struct
{
	HYLR_RESULT *pResult;
	MLRECT *LRArea;
	MLRECT MatchArea;
	int lMaxResultNum;  //�����Ŀ
	int lRealResultNum;  //ʵ����Ŀ
	int  lResultNum;   //����õ������Ŀ
	int  lAreaNum;//��Ŀ��������Ŀ
	MLPOINT origin;
	MLPOINT offset;
}HYLR_RESULT_LIST;

#endif