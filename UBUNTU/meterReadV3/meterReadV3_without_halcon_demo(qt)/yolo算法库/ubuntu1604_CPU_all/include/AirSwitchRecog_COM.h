#ifndef AIRSWITCHREG_COM_H
#define AIRSWITCHREG_COM_H


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
} AR_IMAGES, *AR_PIMAGES;
typedef struct
{
	int x;
	int y;
} MAPOINT, *PMAPOINT;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} MARECT, *PMARECT;
typedef struct
{
	int flag;
	double dVal;///����ֵ
	MARECT    Target;///Ŀ��rectangle

}HYAR_RESULT;
typedef struct
{
	HYAR_RESULT *pResult;
	MARECT *LRArea;
	MARECT MatchArea;
	int lMaxResultNum;  //�����Ŀ
	int lRealResultNum;  //ʵ����Ŀ
	int  lResultNum;   //����õ������Ŀ
	int  lAreaNum;
	MAPOINT origin;
	MAPOINT offset;
}HYAR_RESULT_LIST;



#endif

