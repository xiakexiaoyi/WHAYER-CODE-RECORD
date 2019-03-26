#ifndef MDREG_COM_H
#define MDREG_COM_H

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
} MR_IMAGES, *MR_PIMAGES;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} MDRECT, *MDPRECT;

typedef struct
{
	int  flag;/// Ŀ������
	MDRECT    Target;///Ŀ��rectangle
	float dVal;
	float dConfidence;
}HYMR_RESULT;///text read result

typedef struct
{
	HYMR_RESULT *pResult;
	int  lResultNum;   //����õ������Ŀ
}HYMR_RESULT_LIST;

#endif
