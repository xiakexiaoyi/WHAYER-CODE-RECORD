#pragma once



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
} YOLOV3_IMAGES, *YOLOV3_PIMAGES;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} YOLOV3RECT, *YOLOV3PRECT;

typedef struct
{
	int  flag;// 
	YOLOV3RECT    Target;///目标rectangle
	float dVal;  //目标value
	float dConfidence; //目标confidence
}HYYOLOV3_RESULT;///text read result

typedef struct
{
	HYYOLOV3_RESULT *pResult;
	int  lResultNum;   //程序得到结果数目
}HYYOLOV3RESULT_LIST,*HYYOLOV3RESULT_PLIST;

//#define YOLODLL_API __declspec(dllexport) 
#if defined(_MSC_VER)
#define YOLODLL_API __declspec(dllexport)
#else
#define YOLODLL_API __attribute__((visibility("default")))
#endif

#ifdef GPU
YOLODLL_API int init(void **handle, char *cfgfile, char *weightfile,float thresh,int gpu_index);
YOLODLL_API int uninit(void *handle);
YOLODLL_API int detec(void *handle, YOLOV3_PIMAGES img, HYYOLOV3RESULT_PLIST pResultList);
#else
YOLODLL_API int init_no_gpu(void **handle, char *cfgfile, char *weightfile, float thresh);
YOLODLL_API int uninit_no_gpu(void *handle);
YOLODLL_API int detec_no_gpu(void *handle, YOLOV3_PIMAGES img, HYYOLOV3RESULT_PLIST pResultList);
#endif
