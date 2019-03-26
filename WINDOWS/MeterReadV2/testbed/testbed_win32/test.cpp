// testBed.cpp : main project file.
#include <opencv2/opencv.hpp>
//#include"amcomdef.h"
//#include"time.h"
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <math.h>

// meter Recognise
#include"HYAMR_meterReg.h"

// rig match
#include "wyMatchRigidBodyIF.h"
#include "wyMatchRigidBodyStruct.h"

//#ifdef _DEBUG
//#pragma comment(lib, "../Debug/HYAlgorithm.lib")
//#else
//#pragma comment(lib, "../Release/HYAlgorithm.lib")
//#endif
//#pragma comment(lib, "opencv_imgproc244d.lib")
//#pragma comment(lib, "opencv_core244d.lib")
//#pragma comment(lib, "opencv_highgui244d.lib")

#define DEBUG_DEMO

#ifdef DEBUG_DEMO
#define WY_DEBUG_PRINT printf
#else
#define WY_DEBUG_PRINT
#endif

#define GO(FUNCTION)		{if((res = FUNCTION) != 0) goto EXT;}
#define WORK_BUFFER 100000000
#define MR_MEM_SIZE (100*1024*1024)

#define MAX_BUFFER_LEN 5

// save descriptor info
#define MR_READ_FILE_PARA "C:\\MeterRead.dat"

#define SOURCE_ROAD	"C:\\testVideo\\ywb1.avi"

typedef struct
{
	MPOINT maskPt[10];
	MLong lMaskPtNum;
}MaskParam;

typedef struct
{
	MLong cx;
	MLong cy;
	MLong r;
}MyCircleParam;

int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEnd = 0;
MVoid onMouse(int Event,int x,int y,int flags,void* param );

unsigned char *gTrainTarget = MNull;
unsigned char *gCurTarget = MNull;
unsigned char *gMeterDes = MNull;
MLong gDesSize;

int MeterReadTrain(HYAMR_INTERACTIVE_PARA *pOutPattern, MaskParam *pMaskParam);	//表盘训练
int MeterReadRecog(HYAMR_INTERACTIVE_PARA inPara, MaskParam *pMaskParam);	//表盘识别
int RidMatchTrain(int trainType, int rotateEn);	//开关状态训练
int RidMatchRecog();	//开关状态识别
void markPoints(IplImage *srcImage, MPOINT *ptList, MLong lPtNum);
void setMaskPic(IplImage *maskImage, MaskParam *pParam);
void FittingCircle(MPOINT *pPtList, MLong lPtLen, MLong *xc, MLong *yc, MLong *r);

MLong lScale = 1;

int main(int argc, char* argv[])
{
	HYAMR_INTERACTIVE_PARA para = {0};
	MaskParam mParam = {0};

	MeterReadTrain(&para, &mParam);
//	Sleep(2000);
//	WY_DEBUG_PRINT("########  start recognise\n\n");
//	MeterReadRecog(para, &mParam);
	
	system("pause");

	return 0;
}

int MeterReadTrain(HYAMR_INTERACTIVE_PARA *pOutPattern, MaskParam *pMaskParam)
{
	int res = 0;

	MHandle hMemMgr = MNull;
	MHandle hHYMRDHand = MNull;
	MVoid *pMem = MNull;

	CvCapture *pCapture = MNull;
	IplImage *pFrame = MNull;
	IplImage *testImage = MNull;
	IplImage *tmpImage = MNull;
	IplImage *maskImage = MNull;
	IplImage *pFrame1 = MNull;
	IplImage *testImage1 = MNull;
	IplImage *maskImage1 = MNull;
	CvSize ImgSize;

	HYAMR_IMAGES wy_testImage = {0};
	HYAMR_IMAGES wy_maskImage = {0};
	HYAMR_IMAGES wy_testImage1 = {0};
	HYAMR_IMAGES wy_maskImage1 = {0};

	int i, j;
	int lPtNumTmp;
	MPOINT ptListTmp[50];
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	int flag = 0;
	MLong lTmpVal;
	MLong lMemSize;

	unsigned char *pData = MNull;
	unsigned char *pData1 = MNull;

	pMem = malloc(MR_MEM_SIZE);
	if (!pMem)
	{
		WY_DEBUG_PRINT("malloc 100*1024*1024 error!!!\n");
		return -1;
	}

	hMemMgr = HYAMR_MemMgrCreate(pMem, MR_MEM_SIZE);
	if (!hMemMgr)
	{
		WY_DEBUG_PRINT("HYAMR_MemMgrCreate error!!!\n");
		free(pMem);
		pMem = MNull;
		return -1;
	}

	HYAMR_Init (hMemMgr, &hHYMRDHand);
	if (!hHYMRDHand)
	{
		WY_DEBUG_PRINT("HYAMR_Init error!!!\n");
		HYAMR_MemMgrDestroy (hMemMgr);
		free(pMem);
		pMem = MNull;

		return -1;
	}

	//lMeterType = METER_TYPE;
	/*pCapture = cvCaptureFromFile(SOURCE_ROAD);
	if (MNull == pCapture)
	{
		WY_DEBUG_PRINT("get video file error!\n");
		goto EXT;
	}

	pFrame = cvQueryFrame(pCapture);*/
	pFrame = cvLoadImage("15.jpg", CV_LOAD_IMAGE_COLOR);
	pFrame1 = cvLoadImage("16.jpg", CV_LOAD_IMAGE_COLOR);
	ImgSize.height = pFrame->height / lScale;
	ImgSize.width = pFrame->width / lScale;
	testImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	testImage1 = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(pFrame, testImage, CV_INTER_LINEAR);
	cvResize(pFrame1, testImage1, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", testImage);

	maskImage = cvCreateImage(ImgSize, 8, 1);
	maskImage1 = cvCreateImage(ImgSize, 8, 1);
	pData = (unsigned char *)(maskImage->imageData);
	pData1 = (unsigned char *)(maskImage1->imageData);
	for (j=0;j<ImgSize.height;j++)
	{
		for (i=0;i<ImgSize.width;i++)
		{
			pData[j*maskImage->widthStep+i]=255;
			pData1[j*maskImage->widthStep+i]=255;
		}
	}
	cvSaveImage("D:\\maskImage.bmp",maskImage);

	wy_testImage.lWidth = testImage->width;
	wy_testImage.lHeight = testImage->height;
	wy_testImage.pixelArray.chunky.lLineBytes = testImage->widthStep;
	wy_testImage.pixelArray.chunky.pPixel = testImage->imageData;
	wy_testImage.lPixelArrayFormat = HYAMR_IMAGE_BGR;
	//mask
	wy_maskImage.lWidth = maskImage->width;
	wy_maskImage.lHeight = maskImage->height;
	wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	wy_maskImage.lPixelArrayFormat = HYAMR_IMAGE_GRAY;

	wy_testImage1.lWidth = testImage1->width;
	wy_testImage1.lHeight = testImage1->height;
	wy_testImage1.pixelArray.chunky.lLineBytes = testImage1->widthStep;
	wy_testImage1.pixelArray.chunky.pPixel = testImage1->imageData;
	wy_testImage1.lPixelArrayFormat = HYAMR_IMAGE_BGR;
	//mask
	wy_maskImage1.lWidth = maskImage1->width;
	wy_maskImage1.lHeight = maskImage1->height;
	wy_maskImage1.pixelArray.chunky.lLineBytes = maskImage1->widthStep;
	wy_maskImage1.pixelArray.chunky.pPixel = maskImage1->imageData;
	wy_maskImage1.lPixelArrayFormat = HYAMR_IMAGE_GRAY;

	WY_DEBUG_PRINT("~~~~~~~~~~Train meter~~~~~~~~~~~~\n");

	if (0 != HYAMR_TrainTemplateFromMask1(hHYMRDHand, &wy_testImage, &wy_maskImage, &wy_testImage1, &wy_maskImage1, 0))
	{
		WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
		res = -1;
		goto EXT;
	}



	EXT:
	HYAMR_Uninit (hHYMRDHand);
	HYAMR_MemMgrDestroy (hMemMgr);
	if(MNull!=pMem)
	{	
		free(pMem);
		pMem = MNull;
	}
	cvReleaseImage(&testImage);
	//cvReleaseImage(&tmpImage);
	cvReleaseImage(&maskImage);

	return res;
}