#ifndef DISREG_HY_COM_H
#define DISREG_HY_COM_H

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
} DSR_IMAGES, *DSR_PIMAGES;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} DSRECT, *DSPRECT;

typedef struct
{
	int  flag;///
	DSRECT    Target;///Ŀ��rectangle
	float dVal;// Ŀ������
	float dConfidence;
}HYDSR_RESULT;///text read result

typedef struct
{
	HYDSR_RESULT *pResult;
	int  lResultNum;   //����õ������Ŀ
}HYDSR_RESULT_LIST;



#endif