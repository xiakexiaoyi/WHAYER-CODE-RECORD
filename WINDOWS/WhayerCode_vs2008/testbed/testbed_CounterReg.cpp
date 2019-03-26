#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "HY_CNNCharRecog.h"
#include "HY_TextLocation.h"
#include "highgui.h"
#include "cv.h"

#include <math.h>

#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\CounterReg.lib")
#pragma comment(lib, "..\\Debug\\MatchRigidBody.lib")
#else
#pragma comment(lib, "..\\Release\\CounterReg.lib")
#pragma comment(lib, "..\\Release\\MatchRigidBody.lib")
#endif
using namespace std;
#define MaxAreaNum 20
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEnd = 0;
//double resultOld;
MVoid onMouse(int Event,int x,int y,int flags,void* param );

void run_testtl();
//void Cut(IplImage *img,MRECT *area);
#define DEBUG_DEMO 
#ifdef DEBUG_DEMO
#define WY_DEBUG_PRINT printf
#else
#define WY_DEBUG_PRINT
#endif
int main(int argc, char** argv){
    const CR_Version *p = CR_GetVersion();
	printf("%s\n",p->Version);
	run_testtl();
	return 0;
}


void run_testtl()
{
	IplImage *pImg = NULL, *pOrgImg=NULL, *tmpImage=NULL;
	MHandle hTLHandle=NULL;
	TL_IMAGES imgs = {0};
	HYTL_RESULT_LIST resultlist = {0};
	resultlist.DrArea = (HYED_DRAREA*)malloc(MaxAreaNum*sizeof(HYED_DRAREA));
	int scale=1;
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	int flag=0,k=0,ImgWdith,ImgHeight,i=0;
	MLong DigitalNum=0;

	pOrgImg = cvLoadImage("F:\\12345\\2016-09-26\\1\\9.jpg", 1);//colord   \\计数器
	//pOrgImg = cvLoadImage("..\\photo\\1\\192.168.4.88_01_20160812155746802.jpg", 1);//colord 
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1);
	}
	tmpImage = cvCreateImage(cvSize(pOrgImg->width/scale, pOrgImg->height/scale), IPL_DEPTH_8U,3);
	pImg = cvCreateImage(cvSize(pOrgImg->width/scale, pOrgImg->height/scale), IPL_DEPTH_8U,3);
	cvResize(pOrgImg,tmpImage);
	cvSaveImage("D:\\testImage.bmp", tmpImage);
	cvNamedWindow("TrainImage", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("TrainImage", onMouse, (void*)mouseParam);
	WY_DEBUG_PRINT("请选择数字区域：\n");
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
			resultlist.DrArea[i].left = startPt.x+1;
			resultlist.DrArea[i].top = startPt.y+1; 
		}
		if(mouseParam[2]==CV_EVENT_MOUSEMOVE && flag==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];

			cvReleaseImage(&tmpImage);
			tmpImage=cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage,startPt,endPt,CV_RGB(0,0,255),1);
			cvShowImage("TrainImage",tmpImage);
			resultlist.DrArea[i].right = endPt.x;
			resultlist.DrArea[i].bottom = endPt.y ; 
		}
		if(1 == flagEnd)
		{
			printf("请输入识别内容，大框为1，小框为2，倾斜为3\n");
			scanf("%d",&resultlist.DrArea[i].Type);
			if(resultlist.DrArea[i].Type == 1)
				resultlist.DrArea[i].DigitalNum=5;
			else if(resultlist.DrArea[i].Type == 2)
				resultlist.DrArea[i].DigitalNum=6;
			else if(resultlist.DrArea[i].Type == 3)
				resultlist.DrArea[i].DigitalNum=3;
			i++;
			resultlist.lAreaNum++;
			flagEnd=0;
			flag=0;
			
	    }
		if (32 == cvWaitKey(10))  //blank space 退出
		{
			break;
		}
		cvShowImage("TrainImage",tmpImage);
		//k=cvWaitKey(10);
	}
	cvDestroyWindow("TrainImage");

	/*printf("请输入识别内容，大框为1，小框为2，倾斜为3\n");
	scanf("%d",&resultlist.type);
	if(resultlist.type == 1)
		DigitalNum=5;
	else if(resultlist.type == 2)
		DigitalNum=6;
	else if(resultlist.type == 3)
		DigitalNum=3;*/

	/*ImgWdith=endPt.x-startPt.x-1;
	ImgHeight=endPt.y-startPt.y-1;
	pImg = cvCreateImage(cvSize(ImgWdith, ImgHeight), IPL_DEPTH_8U,3);
	cvSetImageROI(tmpImage,cvRect(startPt.x+1,startPt.y+1,ImgWdith,ImgHeight));
	cvCopy(tmpImage,pImg);
	cvSaveImage("D:\\testImage.bmp", pImg);*/
	cvResize(pOrgImg, pImg);
	//pImg=tmpImage;
	imgs.lWidth = pImg->width;
	imgs.lHeight = pImg->height;
	imgs.lPixelArrayFormat = HYTL_IMAGE_BGR;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	//INPUT.MIN_DISTANCE_BW_BLOB1 = SetParam(INPUT);
	resultlist.lMaxResultNum = 20;
	resultlist.pResult = (HYTR_RESULT *)malloc(resultlist.lMaxResultNum*sizeof(HYTR_RESULT));
	resultlist.lResultNum = 0;
	HYDL_Init(NULL, &hTLHandle);
	HYDL_SetParam(hTLHandle, "29x29-20C4.0-MP2-40C5.0-MP3-150N.txt", "ModelPa");
	//HYDL_SetParam(hTLHandle, "29x29-20C4.0-MP2-40C5.0-MP3-150N.txt", "ModelPa",DigitalNum);  12/19
	if (HYDL_TextLocation(hTLHandle, &imgs, &resultlist)<0)
	{
		printf("error recognize\n");
	}
	

	for (int i=0; i<resultlist.lResultNum; i++)
	{
		CvPoint ptStart, ptStop, ptText;
		char text[256]={0};
		CvFont font;
		ptStart.x = resultlist.pResult[i].rtTarget.left;
		ptStart.y = resultlist.pResult[i].rtTarget.top;
		ptStop.x = resultlist.pResult[i].rtTarget.right;
		ptStop.y = resultlist.pResult[i].rtTarget.bottom;
		ptText.x = resultlist.pResult[i].rtTarget.left;
		ptText.y = resultlist.pResult[i].rtTarget.bottom;
		if (ptText.y<pImg->height-10)
			ptText.y += 10;
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		sprintf(text, "val=%.1f", resultlist.pResult[i].dVal);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	
	}
	cvSaveImage("result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	/*cvNamedWindow("Result Show");
	cvShowImage("Result Show", pImg);*/
	cvWaitKey(0);
	
	cvSaveImage("result.jpg", pImg);
	HYDL_Uninit(hTLHandle);
	cvReleaseImage(&pImg);
	cvReleaseImage(&pOrgImg);
	free(resultlist.pResult);
	cvDestroyWindow("Result Show");
}

void onMouse(int Event,int x,int y,int flags,void* param)
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