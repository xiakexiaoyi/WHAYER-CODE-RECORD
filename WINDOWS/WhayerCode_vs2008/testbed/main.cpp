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
#pragma comment(lib, "..\\Debug\\digitRecognition.lib")
#else
#pragma comment(lib, "..\\Release\\digitRecognition.lib")
#endif
using namespace std;
void run_trainnet();
void
run_testcbcr();
//need to confirm model parameters, store & load 
void run_testnet();
void run_testtl();
void run_testtl_video();
void run_testcolorcvt();
int main(int argc, char** argv){
    const CR_Version *p = CR_GetVersion();
	printf("%s\n",p->Version);
	run_testtl();
	//run_testcbcr();
	//run_testtl_video();
	//run_trainnet();
	return 0;
}

////由于H图是圆周形的，所以分离的时候不能用一分为二的方法
////使用相对距离比较合适,以红色为最大值(Hue=0)，Hue值在圆周上离红色越远值越小 
void BGRImg2RedLedColor(unsigned char* BGR, int lBGRLine, unsigned char* RedLedC, int lRedLedCLine,
				int lWidth, int lHeight)
{
	int i, j;
	int lSrcExt, lDstExt;
	if (!BGR || !RedLedC)return ;

	lSrcExt = lBGRLine - 3*lWidth;
	lDstExt = lRedLedCLine - lWidth;
	for (j=0; j<lHeight; j++, BGR+=lSrcExt, RedLedC+=lDstExt)
	{
		for (i=0; i<lWidth; i++,BGR+=3, RedLedC++)
		{
			unsigned char B = BGR[0];//*((unsigned char*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3);
			unsigned char G = BGR[1];//*((unsigned char*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+1);
			unsigned char R = BGR[2];//*((unsigned char*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+2);

			unsigned char maxVal = max(B,max(G,R));
			unsigned char minVal = min(B,min(G,R));
			double H=0;

			if (maxVal == minVal)
			{
				///look for local H value
				MLong lSum = 0;
				MLong lNum = 0;
				if (i!=0)
				{
					lSum += (*(RedLedC-1)*2 - *(BGR-3+2));
					lNum++;
				}
				if (j!=0)
				{
					lSum += (*(RedLedC-lRedLedCLine)*2 - *(BGR-lRedLedCLine+2));
					lNum++;
				}
				if (i!=0 && j!=0)
				{
					lSum += (*(RedLedC-lRedLedCLine-1)*2 - *(BGR-lRedLedCLine-3+2));
					lNum++;
				}

				if (lNum==0)
					H=128;
				else
					H = lSum*1.0/lNum;
				*RedLedC = (BGR[2]+H)/2;
			}
			else
			{
				if (R==maxVal)H=(G-B)*1.0/(maxVal-minVal);
				else if (G==maxVal)H=2+(B-R)*1.0/(maxVal-minVal);
				else if (B==maxVal)H=4+(R-G)*1.0/(maxVal-minVal);
				H = 60*H;
				*RedLedC = (BGR[2]+(unsigned char)((180-abs(H))*255/180))/2;
			}
		}
	}
}

void run_testcolorcvt()
{
	IplImage *pImg = NULL, *pOrgImg = NULL;
	pOrgImg = cvLoadImage("E:\\textRecognize_svn\\ColorChar.bmp", 1);//colord       

	pImg = cvCreateImage(cvSize(pOrgImg->width,pOrgImg->height), IPL_DEPTH_8U, 1);
	BGRImg2RedLedColor((unsigned char*)pOrgImg->imageData, pOrgImg->widthStep, 
		(unsigned char*)pImg->imageData, pImg->widthStep, pOrgImg->width, pOrgImg->height);
	cvSaveImage("E:\\textRecognize_svn\\GrayChar.bmp", pImg);
	cvReleaseImage(&pImg);
	cvReleaseImage(&pOrgImg);
}

void run_testtl()
{
	IplImage *pImg = NULL, *pOrgImg=NULL;
	IplImage *pImghue = NULL;
	MHandle hTLHandle=NULL;
	TL_IMAGES imgs = {0};
	TL_IMAGES imghue = {0};
	HYTL_RESULT_LIST resultlist = {0};
	int scale=2   ;
	
	//pOrgImg = cvLoadImage("C:\\Users\\Think\\Pictures\\g.png", 1);//color 
	pOrgImg = cvLoadImage("testImage.jpg", 1);//colord       
	///pOrgImg = cvLoadImage("E:\\textRecognize_svn\\ColorChar.bmp", 1);//colord       
	//pOrgImg = cvLoadImage("ledTest3.jpg",1);
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1);
	}

	pImg = cvCreateImage(cvSize(pOrgImg->width/scale, pOrgImg->height/scale), IPL_DEPTH_8U,3);
	cvResize(pOrgImg, pImg);
	
	imgs.lWidth = pImg->width;
	imgs.lHeight = pImg->height;
	imgs.lPixelArrayFormat = HYTL_IMAGE_BGR;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;

	resultlist.lMaxResultNum = 20;
	resultlist.pResult = (HYTR_RESULT *)malloc(resultlist.lMaxResultNum*sizeof(HYTR_RESULT));
	resultlist.lResultNum = 0;
	HYTL_Init(NULL, &hTLHandle);
	HYTL_SetParam(hTLHandle, "29x29-20C4.0-MP2-40C5.0-MP3-150N.txt", "model_29X29_11", -1);
	if (HYTL_TextLocation(hTLHandle, &imgs, &resultlist)<0)
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
		if(resultlist.pResult[i].dlabel==1)
		{
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
			sprintf(text, "val=%c%.1f confi=%.2f", resultlist.pResult[i].dUnit,resultlist.pResult[i].dVal, resultlist.pResult[i].dConfidence);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
			cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
		}
		else
		{
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
			sprintf(text, "val=%.1f confi=%.2f", resultlist.pResult[i].dVal, resultlist.pResult[i].dConfidence);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
			cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
		}
	}
	
	cvNamedWindow("Result Show");
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);
	
	cvSaveImage("result.jpg", pImg);
	HYTL_Uninit(hTLHandle);
	cvReleaseImage(&pImg);
	cvReleaseImage(&pOrgImg);
	free(resultlist.pResult);
	cvDestroyWindow("Result Show");
}

void run_testtl_video()
{
	CvVideoWriter *pwriter;
	
	IplImage *pImg = NULL, *pOrgImg=NULL; 
	MHandle hTLHandle=NULL;
	TL_IMAGES imgs = {0}; 
	HYTL_RESULT_LIST resultlist = {0};
	int scale=2;
	CvCapture *pCapture = NULL;
	CvFont font;
	int lWidth, lHeight;
	int lEveryFrame = 0;
	FILE *fp = fopen("..\\result.txt", "w");

	//pCapture = cvCaptureFromFile("C:\\testVideo\\192.168.8.72_01_20150613181229388_x264.avi");
	pCapture = cvCaptureFromFile("D:/1.mp4");
	if (!pCapture)
	{
		printf("Error when loading video.\n"), exit(1);
	}
	lWidth = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	lHeight = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);
	pImg = cvCreateImage(cvSize(lWidth/scale, lHeight/scale), IPL_DEPTH_8U,3);
	resultlist.lMaxResultNum = 20;
	resultlist.pResult = (HYTR_RESULT *)malloc(resultlist.lMaxResultNum*sizeof(HYTR_RESULT));
	if (!pImg||!resultlist.pResult)goto Ext;

	HYTL_Init(NULL, &hTLHandle);
	HYTL_SetParam(hTLHandle, "29x29-20C4.0-MP2-40C5.0-MP3-150N.txt", "model_29X29_11", -1);
	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 0.6f, 0.6f,0,1,CV_AA);
	cvNamedWindow("Result Show");
	
	lEveryFrame = 0;	
	//cvSetCaptureProperty(pCapture, CV_CAP_PROP_POS_FRAMES, 0);
	int frames=(int)cvGetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_COUNT);
	//pOrgImg=cvQueryFrame(pCapture);
	while(pOrgImg=cvQueryFrame(pCapture))
	{
		if (lEveryFrame%1!=0){lEveryFrame++;continue;}
		{
			int a = 0;
		}
		lEveryFrame++;
		resultlist.lResultNum = 0;
		/*if(lEveryFrame!=125)
			continue;*/
		cvResize(pOrgImg, pImg);
		imgs.lWidth = pImg->width;
		imgs.lHeight = pImg->height;
		imgs.lPixelArrayFormat = HYTL_IMAGE_BGR;
		imgs.pixelArray.chunky.pPixel = pImg->imageData;
		imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
		if (HYTL_TextLocation(hTLHandle, &imgs, &resultlist)<0)
		{
			printf("error recognize\n");
			continue;
			//break;
		}
		for (int i=0; i<resultlist.lResultNum; i++)
		{
			CvPoint ptStart, ptStop, ptText;
			char text[256]={0};
			
			ptStart.x = resultlist.pResult[i].rtTarget.left;
			ptStart.y = resultlist.pResult[i].rtTarget.top;
			ptStop.x = resultlist.pResult[i].rtTarget.right;
			ptStop.y = resultlist.pResult[i].rtTarget.bottom;
			ptText.x = resultlist.pResult[i].rtTarget.left;
			ptText.y = resultlist.pResult[i].rtTarget.bottom;
			if (ptText.y<pImg->height-10)
				ptText.y += 10;
			if(resultlist.pResult[i].dlabel==1)
			{
				cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
				sprintf(text, "val=%c%.1f,conf=%.2f", resultlist.pResult[i].dUnit,resultlist.pResult[i].dVal, resultlist.pResult[i].dConfidence);
				cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
				printf("frame = %d, value = %c%.1f\n", lEveryFrame-1, resultlist.pResult[i].dUnit,resultlist.pResult[i].dVal);
			}
			else
			{
				cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
				sprintf(text, "val=%.1f,conf=%.2f", resultlist.pResult[i].dVal, resultlist.pResult[i].dConfidence);
				cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
				printf("frame = %d, value = %.1f\n", lEveryFrame-1, resultlist.pResult[i].dVal);
			}
			/*if (resultlist.pResult[i].dVal!=481 && resultlist.pResult[i].dVal!=491 &&
				resultlist.pResult[i].dVal!=4481 && resultlist.pResult[i].dVal!=4491)
				fprintf(fp, "frame=%d,value=%.1f\n", lEveryFrame-1, resultlist.pResult[i].dVal);*/
		}
		int i;
		for (i=0; i<resultlist.lResultNum; i++)
		{
			if (resultlist.pResult[i].dVal==481 || resultlist.pResult[i].dVal==491 ||
				resultlist.pResult[i].dVal==4481 || resultlist.pResult[i].dVal==4491)
				break;
		}
		if (i>=resultlist.lResultNum)
		{
			for (i=0; i<resultlist.lResultNum; i++)
				fprintf(fp, "frame=%d,value=%.1f,confidence=%.3f\n", lEveryFrame-1, resultlist.pResult[i].dVal,resultlist.pResult[i].dConfidence);
		}
		cvSaveImage("result.jpg", pImg);
		cvShowImage("Result Show", pImg);
		cvWaitKey(1);
		//pOrgImg=cvQueryFrame(pCapture);
	}
	HYTL_Uninit(hTLHandle);
	
	cvSaveImage("..\\result.jpg", pImg);
Ext:
	fclose(fp);
	cvReleaseImage(&pImg);
	if (resultlist.pResult)
	free(resultlist.pResult);
	cvDestroyWindow("Result Show");
	cvReleaseCapture(&pCapture);
}

void 
run_testhue()
{
}

void
run_testcbcr()
{
	IplImage *pImgYcbcr = NULL;
	IplImage *pImgRgb = NULL;
	char savedImg[128];
	pImgYcbcr = cvCreateImage(cvSize(255,255), IPL_DEPTH_8U, 3);
	
	pImgRgb = cvCreateImage(cvSize(255,255), IPL_DEPTH_8U,3);

	//
	for (int m = 0 ; m<256; m++)
	{
		for (int y=0; y<256; y++)
			for (int x=0; x<256; x++)
			{
				*(pImgYcbcr->imageData + pImgYcbcr->widthStep * y + x*3)
					 = m;
				*(pImgYcbcr->imageData + pImgYcbcr->widthStep * y + x*3 + 1)
					= y;
				*(pImgYcbcr->imageData + pImgYcbcr->widthStep * y + x*3 + 2)
					= x;
			}

			cvCvtColor(pImgYcbcr, pImgRgb, CV_YCrCb2BGR);

			sprintf(savedImg, "..\\color_%d.jpg", m);
			cvSaveImage(savedImg, pImgRgb);
			cvWaitKey(1);
	}

	cvReleaseImage(&pImgRgb);
	cvReleaseImage(&pImgYcbcr);
}
