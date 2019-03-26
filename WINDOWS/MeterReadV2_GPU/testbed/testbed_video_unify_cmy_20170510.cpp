// testBed.cpp : main project file.
#include <opencv2/opencv.hpp>
//#include"amcomdef.h"
//#include"time.h"
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <math.h>
#include <time.h>

// meter Recognise
#include "HYAMR_meterReg.h"
#include "meterDec_GPU.h"
// rig match
#include "wyMatchRigidBodyIF.h"
#include "wyMatchRigidBodyStruct.h"

#include <stdlib.h>  
#include<io.h>

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
#define MR_READ_FILE_PARA "D:\\MeterRead.dat"

#define SOURCE_ROAD	"D://Test Video//yulin//123.0.89.23_01_20161124165309179.mp4"
#define SOURCE_JPG  "D:/20181217/src.bmp"
//#define SOURCE_JPG  "C:\\Users\\li\\Desktop\\表计\\油温表.jpg"

#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif


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

typedef struct
{
	MLong left;
	MLong top;
	MLong right;
	MLong bottom;
}MyRect;

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
void setMaskPic(IplImage *maskImage, MaskParam *pParam,MLong *Dist);
void FittingCircle(MPOINT *pPtList, MLong lPtLen, MLong *xc, MLong *yc, MLong *r);
int HYAMR_GetResult(MHandle hHYMRDHand,HYAMR_IMAGES wy_testImage, HYAMR_INTERACTIVE_PARA inPara,MDouble matchRate,HYAMR_READ_PARA *pReadPara,MLong lFrameNum ,MLong lBufferLen,MDouble (*buffer)[MAX_BUFFER_LEN]);

MFloat lScale = 1.0;
int sobelimgcv(IplImage *img, IplImage *abs_grad_y)
{
	IplImage *grad_y = cvCreateImage(cvGetSize(img), IPL_DEPTH_16S, img->nChannels);
	//IplImage *abs_grad_y= cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, img->nChannels);
	cvSobel(img, grad_y, 0, 1, 3);
	cvConvertScaleAbs(grad_y, abs_grad_y);

	cvReleaseImage(&grad_y);


	return 0;
}
int main(int argc, char* argv[])
{
	HYAMR_INTERACTIVE_PARA para = {0};
	MaskParam mParam = {0};

	MeterReadTrain(&para, &mParam);
//	Sleep(2000);
	WY_DEBUG_PRINT("########  start recognise\n\n");
	MeterReadRecog(para, &mParam);
	
	system("pause");

	return 0;
}

int MeterReadTrain(HYAMR_INTERACTIVE_PARA *pOutPattern, MaskParam *pMaskParam)
{
	int res = 0;
	int count = 0;
	//MyRect rect ={0};
	int rect[4] = {0};

	MHandle hMemMgr = MNull;
	MHandle hHYMRDHand = MNull;
	MVoid *pMem = MNull;

	CvCapture *pCapture = MNull;
	IplImage *pFrame = MNull;
	IplImage *testImage = MNull;
	IplImage *tmpImage = MNull;
	IplImage *maskImage = MNull;
	CvSize ImgSize;

	HYAMR_IMAGES wy_testImage = {0};
	HYAMR_IMAGES wy_maskImage = {0};

	int i, j;
	int lPtNumTmp;
	MPOINT ptListTmp[50];
	int dist[5];
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	int flag = 0;
	MLong lTmpVal;
	MLong lMemSize;
	MLong lHaarWidth=0;
	double temp;

	unsigned char *pData = MNull;

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
	pFrame = cvLoadImage(SOURCE_JPG, CV_LOAD_IMAGE_COLOR);
	ImgSize.height = pFrame->height / lScale;
	ImgSize.width = pFrame->width / lScale;
	testImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(pFrame, testImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", testImage);
	//cvSaveImage("D:\\switch.bmp", testImage);
	cvNamedWindow("TrainImage", 0);
	cvSetMouseCallback("TrainImage", onMouse, (void*)mouseParam);
	tmpImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);

	mouseParam[0]=-1;
	mouseParam[1]=-1;
	mouseParam[2]=-1;

	WY_DEBUG_PRINT("请圈定目标对象：\n");
	while (1)
	{
		if (mouseParam[2]==CV_EVENT_LBUTTONDOWN)
		{
			startPt.x=mouseParam[0];
			startPt.y=mouseParam[1];
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
			
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			flag=1;
			cvShowImage("TrainImage",tmpImage);
		}
		if (mouseParam[2]==CV_EVENT_MOUSEMOVE && flag==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
		
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage",tmpImage);
		}
		if (1 == flagEnd)
		{
			break;
		}
		cvShowImage("TrainImage",tmpImage);
		cvWaitKey(10);
	}
	WY_DEBUG_PRINT("start(%d,%d), end(%d,%d)\n", startPt.x, startPt.y, endPt.x, endPt.y);

	maskImage = cvCreateImage(ImgSize, 8, 1);
	pData = (unsigned char *)(maskImage->imageData);
	for (j=0;j<ImgSize.height;j++)
	{
		for (i=0;i<ImgSize.width;i++)
		{
			if (j>=startPt.y&&j<=endPt.y&&i>=startPt.x&&i<=endPt.x)
			{
				pData[j*maskImage->widthStep+i]=255;
			}
			else
			{
				pData[j*maskImage->widthStep+i]=0;
			}
		}
	}
	cvSaveImage("D:\\maskImage.bmp",maskImage);

	rect[0] = maskImage->width;
	rect[1] = maskImage->height;
//	rect.right=0;
//	rect.top=pMask->lHeight;
//	rect.bottom=0;
	for (j=0;j<maskImage->height;j++)
	{
		for (i=0;i<maskImage->width;i++)
		{
			if (pData[j*maskImage->widthStep+i]>100)
			{
				//count++;
				if(rect[0]>i) 
				{
					rect[0]=i;
				}
				if(rect[2]<i)
				{
					rect[2]=i;
				}
				if (rect[1]>j)
				{
					rect[1]=j;
				}
				if(rect[3]<j)
				{
					rect[3]=j;
				}
			}
		}
	}

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

	WY_DEBUG_PRINT("~~~~~~~~~~Train meter~~~~~~~~~~~~\n");

	int type = 0;//假定油枕油位是1
	//训练后面用到了原来的testImage，wy_testImage，预处理之前的图片，所以这里增加新的变量
	HYAMR_IMAGES wy_testImagesobel = { 0 };
	IplImage *testImagesobel = cvCreateImage(cvGetSize(testImage), IPL_DEPTH_8U, testImage->nChannels);
	if (type == 1)
	{
		sobelimgcv(testImage, testImagesobel);
		wy_testImagesobel.lWidth = testImagesobel->width;
		wy_testImagesobel.lHeight = testImagesobel->height;
		wy_testImagesobel.pixelArray.chunky.lLineBytes = testImagesobel->widthStep;
		wy_testImagesobel.pixelArray.chunky.pPixel = testImagesobel->imageData;
		wy_testImagesobel.lPixelArrayFormat = HYAMR_IMAGE_BGR;
	}
		
	if (type == 1)
	{
		if (0 != HYAMR_TrainTemplateFromMask(hHYMRDHand, &wy_testImagesobel, &wy_maskImage, "OK", 0))
		{
			WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
			res = -1;
			goto EXT;
		}
	}
	else
	{
		if (0 != HYAMR_TrainTemplateFromMask(hHYMRDHand, &wy_testImage, &wy_maskImage, "OK", 0))
		{
			WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
			res = -1;
			goto EXT;
		}
	}
	/*if (0 != HYAMR_TrainTemplateFromMask (hHYMRDHand, &wy_testImage, &wy_maskImage, "OK", 0))
	{
		WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
		res = -1;
		goto EXT;
	}*/

	lMemSize = testImage->width * testImage->height;
	if (MNull != gMeterDes)
	{
		memset(gMeterDes, 0, lMemSize * sizeof(unsigned char));
	}
	else
	{
		gMeterDes = (unsigned char *)malloc(lMemSize * sizeof(unsigned char));
	}
	//if (0 != HYAMR_SaveTDescriptorsGroup(hHYMRDHand, MR_READ_FILE_PARA))
	if(0 != HYAMR_SaveDesMem (hHYMRDHand, gMeterDes, lMemSize, &gDesSize))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
		res = -1;
		goto EXT;
	}

	//训练的时候为什么要进行配准 一旦不准 就全不准了
	if(type==1)
		HYAMR_GetObjRect(hHYMRDHand, &wy_testImagesobel, "OK", 0.75);
	else
		HYAMR_GetObjRect(hHYMRDHand, &wy_testImage, "OK", 0.75);


	//HYAMR_GetObjRect(hHYMRDHand, &wy_testImage, "OK", 0.75);	// 进行一次匹配操作，获取目标对象位置

	if(MNull != gCurTarget)
		memset(gCurTarget, 0, 32 * sizeof(unsigned char));
	else
		gCurTarget = (unsigned char *)malloc(32 * sizeof(unsigned char));

	if(MNull != gTrainTarget)
		memset(gTrainTarget, 0, 32 * sizeof(unsigned char));
	else
		gTrainTarget = (unsigned char *)malloc(32 * sizeof(unsigned char));

	
	//if(0 != HYAMR_SaveTargetInfoToMem (hHYMRDHand, gCurTarget, gTrainTarget, 1))
	//	//if (0 != HYAMR_SaveTargetInfo(hHYMRDHand, MR_READ_FILE_TARGET, MR_TRAIN_TARGET, 1))
	//{
	//	WY_DEBUG_PRINT("HYAMR_SaveTargetInfo error!\n");
	//	res = -1;
	//	goto EXT;
	//}

	if(0 != HYAMR_SaveTargetInfoToMemTmp (hHYMRDHand, gCurTarget, gTrainTarget, 1,rect))
		//if (0 != HYAMR_SaveTargetInfo(hHYMRDHand, MR_READ_FILE_TARGET, MR_TRAIN_TARGET, 1))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTargetInfo error!\n");
		res = -1;
		goto EXT;
	}

//	cvNamedWindow("testImage",0);
	cvShowImage("TrainImage", testImage);
	cvWaitKey(1);
	WY_DEBUG_PRINT("大刻度点标记: 右键添加，左键删除\n");
	mouseParam[0]=-1;
	mouseParam[1]=-1;
	mouseParam[2]=-1;
	cvSetMouseCallback("TrainImage", onMouse, (void*)mouseParam);
	lPtNumTmp = 0;
	while(1)
	{
		if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
		{
			if (mouseParam[0]>=0 && mouseParam[0]<wy_testImage.lWidth
				&& mouseParam[1]>=0 && mouseParam[1]<wy_testImage.lHeight)
			{
				ptListTmp[lPtNumTmp].x = mouseParam[0];
				ptListTmp[lPtNumTmp].y = mouseParam[1];
				lPtNumTmp++;
				cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(255,0,0),-1,8,0);
			}
			WY_DEBUG_PRINT("lPtNum=%d\n", lPtNumTmp);
			mouseParam[2] = -1;
		}
		else if (CV_EVENT_LBUTTONDOWN == mouseParam[2])		// delete white circle
		{
			for(i=0; i<lPtNumTmp; i++)
			{
				if (abs(mouseParam[0]-ptListTmp[i].x)<5 && abs(mouseParam[1]-ptListTmp[i].y)<5)
				{
					cvCircle(testImage, cvPoint(ptListTmp[i].x, ptListTmp[i].y), 1, CV_RGB(255,255,255), -1, 8, 0);
					for(j=i; j<lPtNumTmp-1; j++)
						ptListTmp[j] = ptListTmp[j+1];
					lPtNumTmp--;
					break;
				}
			}
			mouseParam[2] = -1;
		}
		cvShowImage("TrainImage",testImage);
		if(13==cvWaitKey(10))	// Enter
			break;
	}
	WY_DEBUG_PRINT("lPtNum=%d\n", lPtNumTmp);
	pOutPattern->lPtNum = lPtNumTmp;
	for (i=0; i<lPtNumTmp; i++)
	{
		pOutPattern->ptPosList[i] = ptListTmp[i];
	}
	WY_DEBUG_PRINT("请依次输入%d个刻度值\n", lPtNumTmp);
	for (i=0; i<lPtNumTmp; i++)
	{
		scanf("%d", &lTmpVal);
		pOutPattern->dPtValList[i] = (MDouble)lTmpVal;
	}
	if(pOutPattern->lPtNum>2)
	{
		WY_DEBUG_PRINT("请圈定圆心位置,右键添加：\n");
		//WY_DEBUG_PRINT("大刻度点标记: 右键添加，左键删除\n");
		mouseParam[0]=-1;
		mouseParam[1]=-1;
		mouseParam[2]=-1;
		cvSetMouseCallback("TrainImage", onMouse, (void*)mouseParam);
		lPtNumTmp = 0;
		while(1)
		{
			if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
			{
				if (mouseParam[0]>=0 && mouseParam[0]<wy_testImage.lWidth
					&& mouseParam[1]>=0 && mouseParam[1]<wy_testImage.lHeight)
				{
					pOutPattern->circleCoord.x = mouseParam[0];
					pOutPattern->circleCoord.y = mouseParam[1];
					lPtNumTmp++;
					cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(255,0,0),-1,8,0);
				}
				WY_DEBUG_PRINT("lPtNum=%d\n", lPtNumTmp);
				mouseParam[2] = -1;
			}
			cvShowImage("TrainImage",testImage);
			if(13==cvWaitKey(10))	// Enter
				break;
		}
	}

	//WY_DEBUG_PRINT("右键确认起始刻度点\n");
	//while(1)
	//{
	//	if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
	//	{
	//		if (mouseParam[0]>=0 && mouseParam[0]<wy_testImage.lWidth
	//			&& mouseParam[1]>=0 && mouseParam[1]<wy_testImage.lHeight)
	//		{
	//			pOutPattern->ptStart.x = mouseParam[0];
	//			pOutPattern->ptStart.y = mouseParam[1];
	//			cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(0,255,0),-1,8,0);
	//		}
	//		mouseParam[2] = -1;
	//	}
	//	cvShowImage("TrainImage",testImage);
	//	if(13==cvWaitKey(10))	// Enter
	//		break;
	//}

	WY_DEBUG_PRINT("表盘中指针线有几条(1/2?)\n");
	scanf("%d", &lTmpVal);
	while(1>lTmpVal || 2<lTmpVal)
	{
		WY_DEBUG_PRINT("目前只支持1或2根指针线的仪表读数，请重新输入指针线条数\n");
		scanf("%d", &lTmpVal);
	}
	pOutPattern->lLineNum = lTmpVal;
	if(pOutPattern->lLineNum ==1)
	{
		for (i=0; i<pOutPattern->lLineNum; i++)
		{
			WY_DEBUG_PRINT("请输入第%d根指针线颜色（1:黑色 2:白色 3:红色 4:黄色）\n", i+1);
			scanf("%d", &lTmpVal);
			while(1>lTmpVal || 4<lTmpVal)
			{
				WY_DEBUG_PRINT("暂不支持其他颜色，请重新确认指针颜色\n");
				scanf("%d\n", &lTmpVal);
			}
			if (1==lTmpVal)
			{
				pOutPattern->lLineColor[i] = HY_LINE_BLACK;
			}
			else if (2==lTmpVal)
			{
				pOutPattern->lLineColor[i] = HY_LINE_WHITE;
			}
			else if(3==lTmpVal)
			{
				pOutPattern->lLineColor[i] = HY_LINE_RED;
			}
			else
			{
				pOutPattern->lLineColor[i] = HY_LINE_YELLOW;
			}
		}
	}
	else
	{
		//指针线不同的情况下
		WY_DEBUG_PRINT("两根指针请选择 目前只支持 1.一红一黑 2.一红一白 3.两红\n");  
		scanf("%d",&lTmpVal);
		if(1==lTmpVal)
		{
			pOutPattern->lLineColor[1] = HY_LINE_RED;
			pOutPattern->lLineColor[0] = HY_LINE_BLACK;
		}
		else if(2==lTmpVal)
		{
			pOutPattern->lLineColor[0] = HY_LINE_WHITE;
			pOutPattern->lLineColor[1] = HY_LINE_RED;
		}
		else 
		{
			pOutPattern->lLineColor[0] = HY_LINE_RED;
			pOutPattern->lLineColor[1] = HY_LINE_RED;
		}
	}
	WY_DEBUG_PRINT("表盘背景是白色的吗？1.是 2.不是\n");
	scanf("%d", &lTmpVal);
	if (1==lTmpVal)
	{
		pOutPattern->bWhiteBackground = MTrue;
	}
	else
	{
		pOutPattern->bWhiteBackground = MFalse;
	}

	WY_DEBUG_PRINT("表盘指针很细还是正常？1.正常 2.很细\n");
	scanf("%d", &lTmpVal);
	if (2==lTmpVal)
	{
		pOutPattern->lLineWidth = 2;
	}
	else
	{
		WY_DEBUG_PRINT("请在指针线上标记3组点用于计算指针线宽（包含指针线两端）:\n");
		lPtNumTmp = 0;
		while(1)
		{
			if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
			{
				if (mouseParam[0]>=0 && mouseParam[0]<testImage->width
					&& mouseParam[1]>=0 && mouseParam[1]<testImage->height)
				{
					ptListTmp[lPtNumTmp].x = mouseParam[0];
					ptListTmp[lPtNumTmp].y = mouseParam[1];
					lPtNumTmp++;
					printf("%d,%d\n", mouseParam[0], mouseParam[1]);
					//cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(0,255,0),-1,8,0);
				}
				mouseParam[2] = -1;
			}
			cvShowImage("TrainImage",testImage);
			if(13==cvWaitKey(10) || lPtNumTmp>=6)	// Enter
				break;
		}
	
		for (int i=0;i<lPtNumTmp/2;i++)
		{
			temp = (ptListTmp[2*i].x-ptListTmp[2*i+1].x)*(ptListTmp[2*i].x-ptListTmp[2*i+1].x)+
				(ptListTmp[2*i].y-ptListTmp[2*i+1].y)*(ptListTmp[2*i].y-ptListTmp[2*i+1].y);
			dist[i] = sqrt(temp);
			lHaarWidth +=dist[i];
		}
		lHaarWidth /=lPtNumTmp/2;
		//HYAMR_CalcHaarWidth(hHYMRDHand, &wy_testImage, pOutPattern->lLineColor, ptListTmp, lPtNumTmp, &lTmpVal);
		pOutPattern->lLineWidth = lHaarWidth;
	}

	WY_DEBUG_PRINT("请设计表盘MASK类型：（1.矩形 2.圆形 3.圆环 4.圆环+矩形 5.圆形+矩形 6.圆形+2个矩形 7.圆形+3个矩形 8.其它）\n");
	scanf("%d", &lTmpVal);
	switch(lTmpVal)
	{
		case 1:
			lPtNumTmp = 0;
			break;
		case 2:
			lPtNumTmp = 3;
			break;
		case 3:
			lPtNumTmp = 6;
			break;
		case 4:
			lPtNumTmp = 8;
			break;
		case 5:
			lPtNumTmp = 5;
			break;
		case 6:
			lPtNumTmp = 7;
			break;
		case 7:
			lPtNumTmp = 9;
			break;

		default:
			lPtNumTmp = 0;
	}

	if (lPtNumTmp>0)
	{
		WY_DEBUG_PRINT("标记%d个点，构造MASK图\n", lPtNumTmp);
		pMaskParam->lMaskPtNum = lPtNumTmp;
		lPtNumTmp = 0;
		while(1)
		{
			if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
			{
				if (mouseParam[0]>=0 && mouseParam[0]<testImage->width
					&& mouseParam[1]>=0 && mouseParam[1]<testImage->height)
				{
					pMaskParam->maskPt[lPtNumTmp].x = mouseParam[0];
					pMaskParam->maskPt[lPtNumTmp].y = mouseParam[1];
					lPtNumTmp++;
					printf("%d,%d\n", mouseParam[0], mouseParam[1]);
					cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(0,255,0),-1,8,0);
				}
				mouseParam[2] = -1;
			}
			cvShowImage("TrainImage",testImage);
			if(13==cvWaitKey(10) || lPtNumTmp>=pMaskParam->lMaskPtNum)	// Enter
				break;
		}
	}

	WY_DEBUG_PRINT("指针线与表盘背景区分度 1:清晰 2:模糊 3:其他\n");
	scanf("%d", &lTmpVal);
	while(lTmpVal<0 || lTmpVal>3)
	{
		WY_DEBUG_PRINT("error input!!! input again\n");
		scanf("%d\n", &lTmpVal);
	}
	if (1==lTmpVal)
	{
		pOutPattern->picLevel = HY_LOW_BLUR;
	}
	else if(2==lTmpVal)
	{
		pOutPattern->picLevel = HY_HIGH_BLUR;
	}
	else
	{
		pOutPattern->picLevel = HY_OTHER_BLUR;
	}

	WY_DEBUG_PRINT("~~~~~~~~train success~~~~~~~~~~\n");

EXT:
	HYAMR_Uninit (hHYMRDHand);
	HYAMR_MemMgrDestroy (hMemMgr);
	if(MNull!=pMem)
	{	
		free(pMem);
		pMem = MNull;
	}
	cvReleaseImage(&testImage);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&maskImage);

	return res;
}

int MeterReadRecog(HYAMR_INTERACTIVE_PARA inPara, MaskParam *pMaskParam)
{
	int res = 0;
	inPara.Dist = 0xFFFF;
	clock_t time;

	MHandle hMemMgr = MNull;
	MHandle hHYMRDHand = MNull;
	MHandle hTLHandle = MNull;
	MVoid *pMem = MNull;

	CvCapture *pCapture = MNull;
	IplImage *pFrame = MNull;
	IplImage *testImage = MNull;
	IplImage *maskImage = MNull;
	HYAMR_IMAGES wy_testImage = {0};
	HYAMR_IMAGES wy_maskImage = {0};
	CvSize ImgSize ={0};

	MLong lFrameNum = 0;
	CvVideoWriter *pWriter = MNull;

	HYAMR_READ_PARA pReadPara[2] = {0};
	MDouble matchRate;
	int i;

	MDouble dResult;
	MDouble buffer[2][MAX_BUFFER_LEN];
	MLong lBufferLen;
	char textContent[100];
	FILE *fileFp = MNull;
	CvFont font = {0};

	char *cfgfile="../model/tiny-yolo-voc.cfg";
	char *weightfile="../model/tiny-yolo-voc_final.weights";
	float thresh=0.20;
	HYMR_RESULT_LIST  resultlist ={0};
	MR_IMAGES imgs = {0};
	int w=0,h=0;
	int gpu_index = 0;
//***********************  alloc memory  *******************************
	pMem = malloc(MR_MEM_SIZE);
	if (!pMem)
	{
		WY_DEBUG_PRINT("malloc 50*1024*1024 error!!!\n");
		return -1;
	}

	hMemMgr = HYAMR_MemMgrCreate (pMem, MR_MEM_SIZE);
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
	if(0!=HYMR_Init_GPU(NULL,&hTLHandle))//new add
	{
		
		printf("HYDSR_Init error.\n");
		HYAMR_MemMgrDestroy (hMemMgr);
		free(pMem);
		pMem = MNull;
		return -1;
	}
	resultlist.pResult = (HYMR_RESULT*)malloc(20*sizeof(HYMR_RESULT));//new add
//***************************  get video  ******************************************************
	/*pCapture = cvCaptureFromFile(SOURCE_ROAD);
	if (MNull == pCapture)
	{
		WY_DEBUG_PRINT("get video file error!\n");
		goto EXT;
	}
	
	pFrame = cvQueryFrame(pCapture);*/
	pFrame = cvLoadImage(SOURCE_JPG, CV_LOAD_IMAGE_COLOR);
	ImgSize.height = pFrame->height / lScale;
	ImgSize.width = pFrame->width / lScale;
	w = 0;
	h = 0;
	if(0!=HYMR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh, gpu_index,w,h))//new add
	{
		printf("HYDSR_SetParam error.\n");
		HYAMR_MemMgrDestroy (hMemMgr);
		free(pMem);
		pMem = MNull;
		HYMR_Uninit_GPU(hTLHandle);
		return -1;
	}
	testImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(pFrame, testImage, CV_INTER_LINEAR);
	

	maskImage = cvCreateImage(ImgSize, 8, 1);
	setMaskPic(maskImage, pMaskParam,&inPara.Dist);

	wy_maskImage.lWidth = maskImage->width;
	wy_maskImage.lHeight = maskImage->height;
	wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	wy_maskImage.lPixelArrayFormat = HYAMR_IMAGE_GRAY;

	/*pWriter = cvCreateVideoWriter("D:\\result_test2.avi", CV_FOURCC('X', 'V', 'I', 'D'), 5, ImgSize);
	if (MNull == pWriter)
	{
		WY_DEBUG_PRINT("pWriter error!\n");
		goto EXT;
	}*/

	WY_DEBUG_PRINT("~~~~~~~~~~Read meter~~~~~~~~~~~~\n");
	if(0 != HYAMR_GetDesMem (hHYMRDHand, gMeterDes))
	{
		WY_DEBUG_PRINT("HYAMR_GetTemplateFromText error !\n");
		res = -1;
		goto EXT;
	}
	//读取了配准后的目标位置坐标
	if(0 != HYAMR_GetTaregetInfoFromMem (hHYMRDHand, gCurTarget, gTrainTarget, testImage->width, testImage->height))
	{
		WY_DEBUG_PRINT("HYAMR_GetTargetInfo error!\n");
		res = -1;
		goto EXT;
	}

	if (0 != HYAMR_SetParam (hHYMRDHand, &wy_maskImage, &inPara))
	{
		WY_DEBUG_PRINT("HYAMR_SetParam error!\n");
		res = -1;
		goto EXT;
	}

	font = cvFont(2, 1);
	cvNamedWindow("Result", 0);
	lFrameNum = 0;

	matchRate = 0.4;
	
	while(1)
	{
		time=clock();
		//pFrame = cvLoadImage("C:\\a.jpg", CV_LOAD_IMAGE_COLOR);
		lFrameNum++;
		/*if(lFrameNum<90)
			continue;*/
		cvResize(pFrame, testImage, CV_INTER_LINEAR);
		WY_DEBUG_PRINT("lFrameNum=%d\n", lFrameNum);
		//sprintf(textContent1, "D:\\sf6 images\\testImage_%d.bmp", lFrameNum);
		//cvSaveImage(textContent1, testImage);

		imgs.lWidth = testImage->width;
		imgs.lHeight = testImage->height;
		imgs.pixelArray.chunky.lLineBytes = testImage->widthStep;
		imgs.pixelArray.chunky.pPixel = testImage->imageData;
		imgs.lPixelArrayFormat = HYAMR_IMAGE_BGR;

		if(0!=HYMR_meterRecog_GPU(hTLHandle,&imgs,&resultlist))	 //new add
		{
			printf("未找到目标\n");
			printf("HYOLR_OilRecog error.\n");
			res=-1;
			goto EXT;
		}
		if(resultlist.lResultNum<1)
		{
			printf("无表盘图像\n");
			res=-1;
			goto EXT;
		}
		printf("yolo:%f seconds\n",(clock()-time)/1000.0);
		//continue;
		//IplImage *abs_grad_y = cvCreateImage(cvGetSize(imgIpl), IPL_DEPTH_8U, imgIpl->nChannels);
		
		
		int type = 0;//假定油枕油位是1
		if(type==1)
			sobelimgcv(testImage, testImage);//这里对testImage预处理，再输入wy_testImage中，后面testImage和wy_testImage只用在HYAMR_GetLineParam函数，没用到其它地方去

		wy_testImage.lWidth = testImage->width;
		wy_testImage.lHeight = testImage->height;
		wy_testImage.pixelArray.chunky.lLineBytes = testImage->widthStep;
		wy_testImage.pixelArray.chunky.pPixel = testImage->imageData;
		wy_testImage.lPixelArrayFormat = HYAMR_IMAGE_BGR;

		if(lFrameNum<MAX_BUFFER_LEN)
			lBufferLen = lFrameNum;
		else
			lBufferLen = MAX_BUFFER_LEN;
		if(0)
			HYAMR_GetResult(hHYMRDHand, wy_testImage, inPara,matchRate, pReadPara,lFrameNum,lBufferLen,buffer);
		else
		{
			// Recognise the pointer line
			if (0 != HYAMR_GetLineParam(hHYMRDHand, &wy_testImage, &inPara, "OK", matchRate, gCurTarget, gTrainTarget, pReadPara))
			{
				WY_DEBUG_PRINT("HYAMR_GetLineParam error!\n");
				continue;
			}

			for (i=0; i<inPara.lPtNum; i++)
			{
				pReadPara[0].ptInfo[i].ptVal = pReadPara[1].ptInfo[i].ptVal = inPara.dPtValList[i];
			}
			for (i=0; i<inPara.lLineNum; i++)
			{
				HYAMR_GetMeterResult(hHYMRDHand, *(pReadPara+i), &dResult);

				buffer[i][(lFrameNum-1)%MAX_BUFFER_LEN] = dResult;
				dResult = HYAMR_FindMidian(buffer[i], lBufferLen);
				WY_DEBUG_PRINT("result=%.4f\n\n", dResult);
			}
		}

		//printf("total:%f seconds\n",(clock()-time)/1000.0);
		//cvWriteFrame(pWriter, testImage);

		//cvRectangle(testImage,cvPoint(resultlist.pResult[0].Target.left,resultlist.pResult[0].Target.top),cvPoint(resultlist.pResult[0].Target.right,resultlist.pResult[0].Target.bottom),cvScalar(0,0,255),4);
		cvShowImage("Result", testImage);
		cvWaitKey(0);	// delay 2s
	}
	//cvReleaseVideoWriter(&pWriter);

EXT:
	//HYMR_Uninit(hTLHandle);
	HYAMR_Uninit (hHYMRDHand);
	HYAMR_MemMgrDestroy (hMemMgr);
	if(MNull!=pMem)
	{	
		free(pMem);
		pMem = MNull;
	}

	if (MNull != gMeterDes)
	{
		free(gMeterDes);
		gMeterDes = MNull;
	}

	if(MNull!=gCurTarget)
	{
		free(gCurTarget);
		gCurTarget = MNull;
	}
	if(MNull!=gTrainTarget)
	{
		free(gTrainTarget);
		gTrainTarget = MNull;
	}

	cvReleaseImage(&testImage);
	cvReleaseImage(&maskImage);
	return res;
}

MVoid onMouse(int Event,int x,int y,int flags,void* param )
{
	if (Event==CV_EVENT_LBUTTONDOWN)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=1;
	}
	if (Event==CV_EVENT_RBUTTONDOWN)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_RBUTTONDOWN;
	}
	if (Event==CV_EVENT_MOUSEMOVE)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_MOUSEMOVE;
	}
	if (Event==CV_EVENT_LBUTTONUP)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_LBUTTONUP;
		flagEnd=1;
	}
}

void markPoints(IplImage *srcImage, MPOINT *ptList, MLong lPtNum)
{
	MLong lTmpNum;
	int mouseParam[3];
	MPOINT tmpPt;

	cvShowImage("markImage", srcImage);
	cvWaitKey(1);
	mouseParam[0]=-1;
	mouseParam[1]=-1;
	mouseParam[2]=-1;
	cvSetMouseCallback("markImage", onMouse, (void*)mouseParam);

	lTmpNum = 0;
	while(1)
	{
		if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
		{
			if (mouseParam[0]>=0 && mouseParam[0]<srcImage->width
				&& mouseParam[1]>=0 && mouseParam[1]<srcImage->height)
			{
				tmpPt.x = mouseParam[0];
				tmpPt.y = mouseParam[1];
				*(ptList + lTmpNum) = tmpPt;
				lTmpNum++;
				cvCircle(srcImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(0,0,255),-1,8,0);
			}
			mouseParam[2] = -1;
		}
		cvShowImage("markImage",srcImage);
		if(13==cvWaitKey(10) || lTmpNum>=lPtNum)	// Enter
			break;
	}
}

void setMaskPic(IplImage *maskImage, MaskParam *pParam,MLong *Dist)
{
	MByte *pData;
	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MyCircleParam outerCircle, innerCircle, tmpCircle;
	MLong lPoxY;
	MLong i,j;
	MPOINT *tmpPt;
	MPOINT pt1, pt2;
	MLong lDistance1, lDistance2;
	MDouble tmp;

	lWidth = maskImage->width;
	lHeight = maskImage->height;
	lStride = maskImage->widthStep;
	lExt = lStride - lWidth;
	pData = (MByte*)maskImage->imageData;
	tmpPt = pParam->maskPt;

	switch(pParam->lMaskPtNum)
	{
		case 3:
		{
			FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r);
			for (j=0; j<lHeight; j++, pData+=lExt)
			{
				for (i=0; i<lWidth; i++, pData++)
				{
					tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
					lDistance1 = (MLong)(sqrt(tmp));
					if (lDistance1 <= outerCircle.r)
					{
						*pData = 255;
					}
					else
					{
						*pData = 0;
					}
				}
			}
			break;
		}
		case 5:
		{
			FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r); //前面三点拟合圆
			for (j=0; j<lHeight; j++, pData+=lExt)
			{
				for (i=0; i<lWidth; i++, pData++)
				{
					tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
					lDistance1 = (MLong)(sqrt(tmp));
					if (lDistance1 <= outerCircle.r &&((i<tmpPt[3].x || i>tmpPt[4].x) || (j<tmpPt[3].y || j>tmpPt[4].y)))
					{
						*pData = 255;
					}
					else
					{
						*pData = 0;
					}
				}
			}
			break;
		}
		case 6:
			{   
				//为圆环
				FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r);
				FittingCircle(tmpPt+3, 3, &innerCircle.cx, &innerCircle.cy, &innerCircle.r);
				if (outerCircle.r < innerCircle.r)
				{
					tmpCircle = outerCircle;
					outerCircle = innerCircle;
					innerCircle = tmpCircle;
				}
				*Dist = outerCircle.r - innerCircle.r ;

				for (j=0; j<lHeight; j++, pData+=lExt)
				{
					for (i=0; i<lWidth; i++, pData++)
					{
						tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
						lDistance1 = (MLong)(sqrt(tmp));
						tmp = (i - innerCircle.cx)*(i - innerCircle.cx)+(j - innerCircle.cy)*(j - innerCircle.cy);
						lDistance2 = (MLong)(sqrt(tmp));
						if (lDistance1<=outerCircle.r && lDistance2>=innerCircle.r)
						{
							*pData = 255;
						}
						else
						{
							*pData = 0;
						}
					}
				}
				

				break;
			}

		case 7:
		{
			FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r); //前面三点拟合圆
			for (j=0; j<lHeight; j++, pData+=lExt)
			{
				for (i=0; i<lWidth; i++, pData++)
				{
					tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
					lDistance1 = (MLong)(sqrt(tmp));
					if (lDistance1 <= outerCircle.r &&((i<tmpPt[3].x || i>tmpPt[4].x) || (j<tmpPt[3].y || j>tmpPt[4].y)) && ((i<tmpPt[5].x || i>tmpPt[6].x) || (j<tmpPt[5].y || j>tmpPt[6].y)))
					{
						*pData = 255;
					}
					else
					{
						*pData = 0;
					}
				}
			}
			break;
		}
		case 8:
			{
				FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r);
				FittingCircle(tmpPt+3, 3, &innerCircle.cx, &innerCircle.cy, &innerCircle.r);
				if (outerCircle.r < innerCircle.r)
				{
					tmpCircle = outerCircle;
					outerCircle = innerCircle;
					innerCircle = tmpCircle;
				}
				pt1 = tmpPt[6];
				pt2 = tmpPt[7];
				lPoxY = (pt1.y + pt2.y)>>1;
				for (j=0; j<lHeight; j++, pData+=lExt)
				{
					for (i=0; i<lWidth; i++, pData++)
					{
						tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
						lDistance1 = (MLong)(sqrt(tmp));
						tmp = (i - innerCircle.cx)*(i - innerCircle.cx)+(j - innerCircle.cy)*(j - innerCircle.cy);
						lDistance2 = (MLong)(sqrt(tmp));
						if((i>pt1.x && i<pt2.x && j>pt1.y && j<pt2.y)||lDistance1>=outerCircle.r || lDistance2<=innerCircle.r)
						{
							*pData = 0;
						}
					}
				}

				/*for (j=0; j<lHeight; j++, pData+=lExt)
				{
					for (i=0; i<lWidth; i++, pData++)
					{
						tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
						lDistance1 = (MLong)(sqrt(tmp));
						tmp = (i - innerCircle.cx)*(i - innerCircle.cx)+(j - innerCircle.cy)*(j - innerCircle.cy);
						lDistance2 = (MLong)(sqrt(tmp));
						if (lDistance1>=outerCircle.r || (lDistance2<=innerCircle.r && j<=lPoxY))
						{
							*pData = 0;
						}
						else
						{
							*pData = 255;
						}
					}
				}*/

				break;
			}
		case 9:
		{
			FittingCircle(tmpPt, 3, &outerCircle.cx, &outerCircle.cy, &outerCircle.r); //前面三点拟合圆
			for (j=0; j<lHeight; j++, pData+=lExt)
			{
				for (i=0; i<lWidth; i++, pData++)
				{
					tmp = (i - outerCircle.cx)*(i - outerCircle.cx)+(j - outerCircle.cy)*(j - outerCircle.cy);
					lDistance1 = (MLong)(sqrt(tmp));
					if (lDistance1 <= outerCircle.r &&((i<tmpPt[3].x || i>tmpPt[4].x) || (j<tmpPt[3].y || j>tmpPt[4].y))
						&& ((i<tmpPt[5].x || i>tmpPt[6].x) || (j<tmpPt[5].y || j>tmpPt[6].y)) && ((i<tmpPt[7].x || i>tmpPt[8].x) || (j<tmpPt[7].y || j>tmpPt[8].y)))
					{
						*pData = 255;
					}
					else
					{
						*pData = 0;
					}
				}
			}
			break;
		}
		case 0:
		default:
			{
				for (j=0; j<lHeight; j++, pData+=lExt)
				{
					for (i=0; i<lWidth; i++, pData++)
					{
						*pData = 255;
					}
				}
				break;
			}
	}
	//cvSaveImage("D:\\maskImage.bmp", maskImage);
	//cvShowImage("maskImage", maskImage);
	cvWaitKey(10);
}

void FittingCircle(MPOINT *pPtList, MLong lPtLen, 
					   MLong *xc, MLong *yc, MLong *r)
{
	MLong i;
	MDouble X1=0,Y1=0,X2=0,Y2=0,X3=0,Y3=0,X1Y1=0,X1Y2=0,X2Y1=0;
	MDouble C,D,E,G,H,N;
	MDouble a,b,c;
	if (lPtLen<3)
	{
		return;
	}

	for (i=0;i<lPtLen;i++)
	{
		X1 = X1 + pPtList[i].x;
		Y1 = Y1 + pPtList[i].y;
		X2 = X2 + pPtList[i].x*pPtList[i].x;
		Y2 = Y2 + pPtList[i].y*pPtList[i].y;
		X3 = X3 + pPtList[i].x*pPtList[i].x*pPtList[i].x;
		Y3 = Y3 + pPtList[i].y*pPtList[i].y*pPtList[i].y;
		X1Y1 = X1Y1 + pPtList[i].x*pPtList[i].y;
		X1Y2 = X1Y2 + pPtList[i].x*pPtList[i].y*pPtList[i].y;
		X2Y1 = X2Y1 + pPtList[i].x*pPtList[i].x*pPtList[i].y;
	}

	N = lPtLen;
	C = N*X2 - X1*X1;
	D = N*X1Y1 - X1*Y1;
	E = N*X3 + N*X1Y2 - (X2+Y2)*X1;
	G = N*Y2 - Y1*Y1;
	H = N*X2Y1 + N*Y3 - (X2+Y2)*Y1;
	if (C*G-D*D!=0)
	{
		a = (H*D-E*G)/(C*G-D*D);
		b = (H*C-E*D)/(D*D-G*C);
		c = -(a*X1 + b*Y1 + X2 + Y2)/N;
	}
	else
	{
		//拟合不出来
		a = 0;
		b = 0;
		c = 0;
	}


	if (xc)
	{
		*xc=(MLong)(a/(-2));
	}
	if (yc)
	{
		*yc=(MLong)(b/(-2));
	}
	if (r)
	{
		*r=(MLong)(sqrt(a*a+b*b-4*c)/2);
	}	
}


//MRESULT GetRectFromMask(IplImage *pMask,rect pMaskRect)
//{
//	MRECT rect;
//	MInt16 x,y;
//	MUInt8* pMaskData=(MUInt8*)(pMask->pBlockData);
//	int count = 0;
//	MRESULT res = LI_ERR_NONE;
//	rect.left=pMask->lWidth;
//	rect.right=0;
//	rect.top=pMask->lHeight;
//	rect.bottom=0;
//	
//	for (y=0;y<pMask->lHeight;y++)
//	{
//		for (x=0;x<pMask->lWidth;x++)
//		{
//			if (*(pMaskData+y*pMask->lBlockLine+x)>100)
//			{
//				count++;
//				if(rect.left>x) 
//				{
//					rect.left=x;
//				}
//				if(rect.right<x)
//				{
//					rect.right=x;
//				}
//				if (rect.top>y)
//				{
//					rect.top=y;
//				}
//				if(rect.bottom<y)
//				{
//					rect.bottom=y;
//				}
//			}
//		}
//	}
//	if (0==count)
//	{
//		res = LI_ERR_NO_FIND;
//		return res;
//	}
//	pMaskRect->bottom=rect.bottom;
//	pMaskRect->left=rect.left;
//	pMaskRect->right=rect.right;
//	pMaskRect->top=rect.top;
//	return res;
//}
//int main123()//单独对表盘检测测试
//{
//	MHandle hTLHandle = MNull;
//	struct _finddata_t fileinfo;
//	//保存文件句柄 
//	//long fHandle;
//	intptr_t fHandle;
//	char to_search[9000];
//	char single_img[9000];//单张图片文件路径
//	char result_img[9000];//单张图片文件路径
//	//char *cfgfile="../model/tiny-yolo-voc.cfg";
//	//char *weightfile="../model/tiny-yolo-voc_final.weights";
//	//char *cfgfile="../zhizhen/tiny-yolo-voc.cfg";
//	//char *weightfile="../zhizhen/tiny-yolo-voc_final.weights";
//	char *cfgfile="../1106/tiny-yolo-voc.cfg";
//	char *weightfile="../1106/tiny-yolo-voc_38000.weights";
//	float thresh=0.24;
//	
//	MR_IMAGES imgs = {0};
//	char single_picname[9000];//单张图片文件路径
//	int w=0,h=0;
//	//while(1)
//	{
//	if(0!=HYMR_Init(NULL,&hTLHandle))
//	{
//		
//		printf("HYDSR_Init error.\n");
//		return -1;
//	}
//	if(0!=HYMR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h))
//	{
//		printf("HYDSR_SetParam error.\n");
//		HYMR_Uninit(hTLHandle);
//		//continue;
//		return -1;
//	}
//    //HYMR_Uninit(hTLHandle);
//	}
//
//	printf("请输入测试单张照片路径：（例如输入:E:\\nest_photos\\1.jpg)(注：路径中不能包含空格)(回车确定)\n");
//	
//
//	scanf("%s", &single_picname);
//
//	printf("分析开始\n");
//	char result_path[9000] = "..\\result\\";//单张图片文件路径
//	
//
//	sprintf(to_search, "%s\\*.jpg", single_picname);
//
//	if ((fHandle = _findfirst(to_search, &fileinfo)) == -1L)
//	{
//		int fileOrNot = 1;
//		int len = strlen(fileinfo.name);
//		fileinfo.name[len - 4] = '\0';
//		//sprintf(result_img, "%s\\%s_result.jpg", result_path, fileinfo.name);
//		char *result_img = "..\\result\\result.jpg";
//		
//	}
//	else{
//		do{
//			HYMR_RESULT_LIST  resultlist ={0};
//			int fileOrNot = 2;
//			int len = strlen(fileinfo.name);
//			sprintf(single_img, "%s\\%s", single_picname, fileinfo.name);
//			fileinfo.name[len - 4] = '\0';
//			//sprintf(single_img, "%s\\%s.jpg", single_picname, fileinfo.name);
//			
//			//Detec_disconnect(single_img, result_img, fileinfo.name, yolo, thresh, fileOrNot);
//			IplImage *testImage=cvLoadImage(single_img);
//			imgs.lWidth = testImage->width;
//			imgs.lHeight = testImage->height;
//			imgs.pixelArray.chunky.lLineBytes = testImage->widthStep;
//			imgs.pixelArray.chunky.pPixel = testImage->imageData;
//			imgs.lPixelArrayFormat = HYAMR_IMAGE_BGR;
//			resultlist.pResult = (HYMR_RESULT*)malloc(20*sizeof(HYMR_RESULT));
//			if(0!=HYMR_meterRecog(hTLHandle,&imgs,&resultlist))	//
//			{
//				printf("未找到目标\n");
//				printf("HYOLR_OilRecog error.\n");
//			}
//			for(int i=0;i<resultlist.lResultNum;i++)
//			{
//				CvFont font;
//				char text[100]={0};
//				if(resultlist.pResult[i].dVal==0)
//					sprintf(text, "%d:7", i+1);
//				else if(resultlist.pResult[i].dVal==1)
//					sprintf(text, "%d:n", i+1);
//				else if(resultlist.pResult[i].dVal==2)
//					sprintf(text, "%d:fen", i+1);
//				else if(resultlist.pResult[i].dVal==3)
//					sprintf(text, "%d:he", i+1);
//				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8f, 1.0f);
//				cvPutText(testImage, text, cvPoint(resultlist.pResult[i].Target.left,resultlist.pResult[i].Target.bottom), &font,  cvScalar(0,255,0));
//				cvRectangle(testImage,cvPoint(resultlist.pResult[i].Target.left,resultlist.pResult[i].Target.top),cvPoint(resultlist.pResult[i].Target.right,resultlist.pResult[i].Target.bottom),cvScalar(0,0,255),4);
//			}
//			if(resultlist.lResultNum==0)
//			{
//				sprintf(result_img, "%s\\result\\%s_resultnone.jpg", single_picname, fileinfo.name);
//			}
//			else
//			{
//				sprintf(result_img, "%s\\result\\%s_result_%f.jpg", single_picname, fileinfo.name,resultlist.pResult[0].dConfidence);
//			}
//			printf("%s\n",result_img);
//			cvSaveImage(result_img,testImage);
//			free(resultlist.pResult);
//			cvReleaseImage(&testImage);
//		} while (_findnext(fHandle, &fileinfo) == 0);
//	}
//
//	
//}

int HYAMR_GetResult(MHandle hHYMRDHand,HYAMR_IMAGES wy_testImage, HYAMR_INTERACTIVE_PARA inPara,MDouble matchRate,HYAMR_READ_PARA *pReadPara,MLong lFrameNum ,MLong lBufferLen,MDouble (*buffer)[MAX_BUFFER_LEN])
{
	
	MDouble dResult;
	if (0 != HYAMR_GetLineParam(hHYMRDHand, &wy_testImage, &inPara, "OK", matchRate, gCurTarget, gTrainTarget, pReadPara))
	{
		WY_DEBUG_PRINT("HYAMR_GetLineParam error!\n");
		return -1;
		//continue;
	}

	for (int i=0; i<inPara.lPtNum; i++)
	{
		pReadPara[0].ptInfo[i].ptVal = pReadPara[1].ptInfo[i].ptVal = inPara.dPtValList[i];
	}
	

	for (int i=0; i<inPara.lLineNum; i++)
	{
		HYAMR_GetMeterResult(hHYMRDHand, *(pReadPara+i), &dResult);

		buffer[i][(lFrameNum-1)%MAX_BUFFER_LEN] = dResult;
		dResult = HYAMR_FindMidian(buffer[i], lBufferLen);
		WY_DEBUG_PRINT("result=%.4f\n\n", dResult);
	}
	return 0;
}