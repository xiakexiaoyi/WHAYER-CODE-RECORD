#ifndef _HYMETERRECOGV3_COM_H
#define _HYMETERRECOGV3_COM_H

#define HYAMR_MAX_PT_LIST	30

/* Defines for image color format*/
#define HYAMR_IMAGE_GRAY				0x00		//chalk original 0x0A 	
#define HYAMR_IMAGE_YUYV               0x02        //y,u,y,v,y,u,y,v......
#define HYAMR_IMAGE_RGB                0x04        //r,g,b,r,g,b.....
#define HYAMR_IMAGE_BGR                0x06        //b,g,r,b,g,r.....

/* Defines for image data*/
typedef struct {
	long		lWidth;				// Off-screen width
	long		lHeight;			// Off-screen height
	long		lPixelArrayFormat;	// Format of pixel array
	union
	{
		struct
		{
			long lLineBytes;
			void *pPixel;
		} chunky;
		struct
		{
			long lLinebytesArray[4];
			void *pPixelArray[4];
		} planar;
	} pixelArray;
} HYMRV3_IMAGES, *HYLPMRV3_IMAGES;

typedef struct
{
	long x;
	long y;
} MV3POINT, *PMV3POINT;

typedef struct
{
	MV3POINT ptPosList[HYAMR_MAX_PT_LIST];
	double dPtValList[HYAMR_MAX_PT_LIST];
	long lPtNum;
}MV3PT;

typedef struct
{
	//char *cfgfile;
	//char *weightfile;

	long lPtNum;
	MV3POINT ptPosList[HYAMR_MAX_PT_LIST];
	double dPtValList[HYAMR_MAX_PT_LIST];

	int AngleStart;
	int AngleEnd;
	int RadiusOut;
	int RadiusIn;
	int Row;
	int Col;
}MV3Para;

typedef struct
{
	double MeterValue;
}HYMR_POINTERRESULT;

#endif