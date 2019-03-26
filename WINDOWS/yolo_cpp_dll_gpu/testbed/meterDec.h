#ifndef OLREG_HY_H
#define OLREG_HY_H

#ifdef __cplusplus
extern "C" {
#endif

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
} MRECT, *MPRECT;

typedef struct 
{
	int  flag;/// 目标属性
	MRECT    Target;///目标rectangle
	float dVal;
	float dConfidence;
}HYMR_RESULT;///text read result

typedef struct
{
	HYMR_RESULT *pResult;
	int  lResultNum;   //程序得到结果数目
}HYMR_RESULT_LIST;

__declspec(dllexport) int HYMR_Init(void *hMemMgr, void **pMRHandle);
__declspec(dllexport) int HYMR_Uninit(void *hMRHandle);
__declspec(dllexport) int HYMR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h);
__declspec(dllexport) int HYMR_meterRecog(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList);

#ifdef __cplusplus
}
#endif

#endif