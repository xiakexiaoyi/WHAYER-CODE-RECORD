#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "putText.h"
#include "highgui.h"
#include "cv.h"


#include "HYL_LightOff.h"
//#include "HYL_MatchRigidBody.h"

#include <math.h>

#define CRTDBG_MAP_ALLOC   
#include <stdlib.h>   
#include <crtdbg.h>   

#include <intrin.h>

#pragma comment( lib, "vfw32.lib" )   
#pragma comment( lib, "comctl32.lib" )   

#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\LightOff.lib")
#pragma comment(lib, "..\\Debug\\MatchRigidBody.lib")
#else
#pragma comment(lib, "..\\Release\\LightOff.lib")
//#pragma comment(lib, "..\\Release\\MatchRigidBody.lib")
#endif
using namespace std;
void run_trainnet();
void
run_testcbcr();
//need to confirm model parameters, store & load 
void run_testtl();
void run_testtl_video();

#define DEBUG_DEMO 
#ifdef DEBUG_DEMO
#define WY_DEBUG_PRINT printf
#else
#define WY_DEBUG_PRINT
#endif

#define MaxAreaNum 20
//#define MR_READ_FILE_PARA "../MeterRead.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;
//double resultOld;
MVoid onMouse(int Event,int x,int y,int flags,void* param );

unsigned char *gMeterDes = MNull;
MLong gDesSize;

int LightOffTrain(HYED_RESULT_LIST *outPattern);	//训练
int LightOffRecog(HYED_RESULT_LIST inPara);	//识别
//
int main(int argc, char** argv){
	HYED_RESULT_LIST resultlist = {0};
	resultlist.DtArea = (HYED_DTAREA*)malloc(MaxAreaNum*sizeof(HYED_DTAREA));
	resultlist.pResult = (HYED_RESULT*)malloc(MaxAreaNum*sizeof(HYED_RESULT)) ;
	
	LightOffTrain(&resultlist);
	LightOffRecog(resultlist);

	if (resultlist.DtArea)
		free(resultlist.DtArea);
	if (resultlist.pResult)
		free(resultlist.pResult);
	
	//system("pause");

	return 0;
}

int LightOffTrain(HYED_RESULT_LIST *resultlist)
{
	int res = 0;;
    IplImage *pImg = NULL, *pImg2=NULL, *pOrgImg=NULL, *pOrgImg2=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	MHandle MHandle=NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	HYL_IMAGES imgs = {0}, imgs2={0},mask={0};
	CvSize ImgSize; 
	MLong lMemSize;
	float scale=1;
	unsigned char *pData;
	

	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};

	int modeltype=0,flagL = 0,flagR = 0,i=0,j;
	//HYL_MatchInit(NULL, &MHandle);

	CvCapture* capture = cvCaptureFromFile("../../../Web/light.mp4"); 
	pOrgImg=cvQueryFrame(capture);
	//pOrgImg  = cvLoadImage("..\\..\\Web\\1\\201708171827411.jpg", 1);//colord  基准图    
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;
	testImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	pImg = cvCreateImage(ImgSize, IPL_DEPTH_8U,3);
	pImg2 = cvCreateImage(ImgSize, IPL_DEPTH_8U,3);
	cvResize(pOrgImg, testImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", testImage);
	cvNamedWindow("TrainImage2", 0);
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);
	Mat m=cvarrToMat(tmpImage,true);
	putTextZH(m, "操作步骤：\n1、用左键逐个框选指示灯。\n2、按Enter键确认。\n", Point(50, 50), Scalar(0, 255, 255), 30, "Arial");
	tmpImage=cvCloneImage(&(IplImage)m);
	cvSaveImage("D:\\testImage.bmp", tmpImage);

	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	
	WY_DEBUG_PRINT("请圈定指示灯目标对象：\n");
	while (1)
	{
		//左键进行标记 类型为1 检测亮度
		if (mouseParam[2]==CV_EVENT_LBUTTONDOWN)
		{
			startPt.x=mouseParam[0];
			startPt.y=mouseParam[1];
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
			
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			flagL=1;
			cvShowImage("TrainImage2",tmpImage);
			resultlist->DtArea[i].left = startPt.x;
			resultlist->DtArea[i].top = startPt.y; 			
		}
		if (mouseParam[2]==CV_EVENT_MOUSEMOVE && flagL==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
		
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage2",tmpImage);
			resultlist->DtArea[i].right = endPt.x;
			resultlist->DtArea[i].bottom = endPt.y ; 				
		}

		if(1 == flagEndL && i == 0 )
		{
			//printf("匹配目标选择完成，下面选择识别目标\n");
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
		}
		if (1 == flagEndL && i != 0 )   //左键标记一个进行重置
		{
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
		}

		if (13 == cvWaitKey(10))  //enter 退出
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
	}
	printf("交互完成\n");
	cvDestroyWindow("TrainImage2");
	resultlist->origin.x=(resultlist->DtArea[0].left+resultlist->DtArea[0].right)/2;
	resultlist->origin.y=(resultlist->DtArea[0].top+resultlist->DtArea[0].bottom)/2;
	//WY_DEBUG_PRINT("start(%d,%d), end(%d,%d)\n", startPt.x, startPt.y, endPt.x, endPt.y);
	/*cvSetImageROI(testImage,cvRect(resultlist.DtArea[0].left,resultlist.DtArea[0].top,resultlist.DtArea[0].right-resultlist.DtArea[0].left,resultlist.DtArea[0].bottom-resultlist.DtArea[0].top));
	cvSaveImage("D:\\testImage.bmp", testImage);*/

	//maskImage = cvCreateImage(ImgSize, 8, 1);   //mask image
	//pData = (unsigned char *)(maskImage->imageData);
	//for (j=0;j<ImgSize.height;j++)
	//{
	//	for (i=0;i<ImgSize.width;i++)
	//	{
	//		if (j>=resultlist->DtArea[0].top&&j<=resultlist->DtArea[0].bottom&&i>=resultlist->DtArea[0].left&&i<=resultlist->DtArea[0].right)
	//		{
	//			pData[j*maskImage->widthStep+i]=255;
	//		}
	//		else
	//		{
	//			pData[j*maskImage->widthStep+i]=0;
	//		}

	//	}
	//}
	//cvSaveImage("D:\\maskImage.bmp",maskImage);

	//wy_testImage.lWidth = testImage->width;
	//wy_testImage.lHeight = testImage->height;
	//wy_testImage.pixelArray.chunky.lLineBytes = testImage->widthStep;
	//wy_testImage.pixelArray.chunky.pPixel = testImage->imageData;
	//wy_testImage.lPixelArrayFormat = HYL_IMAGE_BGR;
	////mask
	//wy_maskImage.lWidth = maskImage->width;
	//wy_maskImage.lHeight = maskImage->height;
	//wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	//wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	//wy_maskImage.lPixelArrayFormat = HYL_IMAGE_GRAY;
	//if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//训练图片，得到模板
	//{
	//	WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
	//	goto EXT;
	//}
	//
	//if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	//{
	//	WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
	//}
EXT:
	//HYL_MatchUninit(MHandle);
	return res;
}

int LightOffRecog(HYED_RESULT_LIST resultlist)
{
	int res = 0;
	char modelfile[50];
	char Filters0file[50];
	char Filters1file[50];
    IplImage  *pImg2=NULL, *pOrgImg=NULL, *pOrgImg2=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	MHandle KRHandle=NULL;
	MHandle MHandle=NULL;
	Mat m;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	HYL_IMAGES imgs = {0}, imgs2={0},mask={0};
	CvSize ImgSize; 
	float scale=1;
	
	MPOINT *offset;

	

	offset = (MPOINT*)malloc(1*sizeof(MPOINT));
	int modeltype=0,flagL = 0,flagR = 0,i=0;
	//HYL_LightOffInit(NULL, &KRHandle);
	//HYL_MatchInit(NULL, &MHandle);

	
	//WY_DEBUG_PRINT("输入分析类型：0左半圈旋钮（横竖），1上半圈旋钮（非横）\n");
	//scanf("%d",&modeltype);
	//if(modeltype == 0 || modeltype == 1 || modeltype == 4)
	//{
	//	sprintf(modelfile,"..\\..\\model\\%d\\all_age_svm.xml",modeltype);
	//	sprintf(Filters0file,"..\\..\\model\\%d\\all_age_filters1.txt",modeltype);
	//	sprintf(Filters1file,"..\\..\\model\\%d\\all_age_filters2.txt",modeltype);
	//}
	//else
	//{
	//	exit(1);//错误简单处理
	//}
	
	
	//HYL_LightOffSetParam(KRHandle,modelfile,Filters0file,Filters1file);//分类器算法输入参数
	int n=0;
	CvCapture* capture = cvCaptureFromFile("../../../Web/light.mp4"); 
	while(pOrgImg2=cvQueryFrame(capture))
	{
		int res;
		n=n+1;
		if(n%24 > 0)continue;

		ImgSize.height = pOrgImg2->height;
		ImgSize.width = pOrgImg2->width;
		IplImage* pImg = cvCreateImage(ImgSize, IPL_DEPTH_8U,3);
		cvResize(pOrgImg2, pImg);
		imgs.lHeight = pImg->height;
		imgs.lWidth = pImg->width;
		imgs.lPixelArrayFormat = HYL_IMAGE_BGR;
		imgs.pixelArray.chunky.pPixel = pImg->imageData;
		imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;

		resultlist.offset.y=0;
		resultlist.offset.x=0;

		if (HYL_LightOffExceptionDetection(NULL, &imgs, &resultlist)<0)//识别目标
		{
			printf("error recognize\n");
			goto EXT;
		}
		int y=0;
		for(int i=0;i < resultlist.lAreaNum;i++)
		{
			CvPoint ptStart, ptStop, ptText;
			char text[256]={0};
			ptStart.x = resultlist.DtArea[i].left;
			ptStart.y = resultlist.DtArea[i].top;
			ptStop.x = resultlist.DtArea[i].right;
			ptStop.y = resultlist.DtArea[i].bottom;

			ptText.x = resultlist.DtArea[i].left;
			ptText.y = resultlist.DtArea[i].bottom;
			cvRectangle(pOrgImg2, ptStart, ptStop, cvScalar(0,0,255));
			if(resultlist.pResult[i].result == 1)
			{
				sprintf(text,"指示灯%d:亮\n",y+1);
				//printf("%d状态:亮\n",y+1);
			}
			else if(resultlist.pResult[i].result == 2)
			{
				sprintf(text,"指示灯%d:灭\n",y+1);
				//printf("%d状态:灭\n",y+1);
			}
			else
			{
				printf("error\n");
			}
			y+=1;
			m=Mat(pOrgImg2,true);
			putTextZH(m, "操作步骤：\n1、用左键逐个框选指示灯。\n2、按Enter键确认。\n", Point(50, 50), Scalar(0, 255, 255), 30, "Arial");
			putTextZH(m, text, ptText, Scalar(0, 255, 255), 18, "Arial");
			pOrgImg2=&(IplImage)m;
		}
		cvShowImage("Result Show", pOrgImg2);
		cvWaitKey(10);
		cvReleaseImage(&pImg);
	}

EXT:
	free(offset);
	//HYL_MatchUninit(MHandle);
//	HYL_LightOffUninit(KRHandle);
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
		flagEndL=1;
	}
	if (Event==CV_EVENT_RBUTTONUP)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_RBUTTONUP;
		flagEndR=1;
	}
}