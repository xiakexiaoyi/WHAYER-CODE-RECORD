#ifndef OLREG_COM_H
#define OLREG_COM_H

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
} OLR_IMAGES, *OLR_PIMAGES;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} OLRECT, *OLPRECT;

typedef struct
{
	int  flag;/// 目标属性
	OLRECT    Target;///目标rectangle
	float dVal;
	float dConfidence;
}HYOLR_RESULT;///text read result

typedef struct
{
	HYOLR_RESULT *pResult;
	int  lResultNum;   //程序得到结果数目
}HYOLR_RESULT_LIST;

#endif
