#include <stdio.h>
#include <string.h>
#include<io.h>
#include <opencv2/opencv.hpp>
#include "SwitchRecog.h"



#ifdef _DEBUG
#pragma comment(lib, "../Debug/SwitchRecog.lib")
#pragma comment(lib, "../Debug/yolo.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")

#else
#pragma comment(lib, "../x64/Release/SwitchRecog.lib")

#endif

#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif

int video()
{
	//文件存储信息结构体 
    struct _finddata_t fileinfo;
    //保存文件句柄 
    long fHandle;

	HYSR_RESULT  result ={0};
	void *hTLHandle=NULL;
	float thresh=0.1;
	char *cfgfile="../model/tiny-yolo-voc.cfg";
	char *weightfile="../model/tiny-yolo-voc_final.weights";
	//char *cfgfile="../model/tiny-yolo-voc_IO.cfg";
	//char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
	//char *filename="../5011开关B相.jpg";
	
	char to_search[50];
	char to_read[50];
	char imgsavename[50];
	char readname[50]="E:/WhayerCode/Web/Test Video/";
	char savename[50]="E:/WhayerCode/Web/result/";
	sprintf(to_search,"%s*.avi",readname);

	IplImage* src = 0;
	SR_IMAGES imgs = {0};
 
	HYSR_Init(NULL,&hTLHandle);
	HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh);
	
	if( (fHandle=_findfirst( to_search, &fileinfo )) == -1L ) 
    {
        printf( "当前目录下没有jpg文件\n");
        return 0;
    }
    else{
        do{
			CvVideoWriter *writer = 0;
			sprintf(to_read,"%s%s",readname,fileinfo.name);
			sprintf(imgsavename,"%s%s",savename,fileinfo.name);
			CvCapture* capture = cvCaptureFromFile(to_read); 
			int num=0;
			int c;
			IplImage* pImg = 0;
			int frameH = (int) cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT);
			int frameW = (int) cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH);
			writer = cvCreateVideoWriter(imgsavename,CV_FOURCC('D','I','V','3'),1,cvSize(frameW,frameH),1);
			pImg=cvCreateImage(cvSize(frameW,frameH),IPL_DEPTH_8U,3);
			while(src=cvQueryFrame(capture))
			{
				cvCopy(src,pImg);
				num=num+1;
				if(num%25!=1)
					continue;
				imgs.lHeight = pImg->height;
				imgs.lWidth = pImg->width;
				imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
				imgs.pixelArray.chunky.pPixel = pImg->imageData;
				result.dlabel=-1;
				c=HYSR_SwitchRecog(hTLHandle,&imgs,&result);
				if(c==-1)
				{
					sprintf(to_read,"%serror_%d_%s",savename,num,fileinfo.name);
					cvSaveImage(to_read, pImg);
				}
				CvPoint ptStart, ptStop, ptText;
				char text[256]={0};
				CvFont font;
				ptStart.x = result.rtTarget.left;
				ptStart.y = result.rtTarget.top;
				ptStop.x = result.rtTarget.right;
				ptStop.y = result.rtTarget.bottom;
				ptText.x = result.rtTarget.left;
				ptText.y = result.rtTarget.bottom;
				if (ptText.y<pImg->height-10)
					ptText.y += 10;
				if(result.dlabel==1)
				{
					cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
					sprintf(text, "val=he confi=%.2f", result.dConfidence);
					cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
					cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
				}
				else
				{
					cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
					sprintf(text, "val=fen confi=%.2f", result.dConfidence);
					cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
					cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
				}
				
				cvWriteFrame(writer,pImg);
				cvShowImage("Result Show", pImg);
			    cvWaitKey(1);
				
				
			}
			
			cvReleaseVideoWriter(&writer);
			cvReleaseCapture(&capture);
		}while( _findnext(fHandle,&fileinfo)==0);
	}
	_findclose( fHandle );

	
	HYSR_Uninit(hTLHandle);

	return 0;
}



int image(char *filename)
{
	
	IplImage* pImg = 0;
	SR_IMAGES imgs = {0};
	HYSR_RESULT  result ={0};
	void *hTLHandle=NULL;
	float thresh=0.1;
	char *cfgfile="../model/tiny-yolo-voc.cfg";    //字符
	char *weightfile="../model/tiny-yolo-voc_final.weights";
	//char *cfgfile="../model/tiny-yolo-voc_IO.cfg";   //符号
	//char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	
	//HYSR_Init(NULL,&hTLHandle);
	if(0!=HYSR_Init(NULL,&hTLHandle))
	{
		printf("HYSR_Init error.\n");
		return -1;
	}
	if(0!=HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh))
	{
		printf("HYSR_SetParam error.\n");
		HYSR_Uninit(hTLHandle);
		return -1;
	}
	//HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	result.dlabel=-1;
	//HYSR_SwitchRecog(hTLHandle,&imgs,&result);
	if(0!=HYSR_SwitchRecog(hTLHandle,&imgs,&result))
	{
		printf("HYSR_SwitchRecog error.\n");
		HYSR_Uninit(hTLHandle);
		return -1;
	}


	
	CvPoint ptStart, ptStop, ptText;
	char text[256]={0};
	CvFont font;
	ptStart.x = result.rtTarget.left;
	ptStart.y = result.rtTarget.top;
	ptStop.x = result.rtTarget.right;
	ptStop.y = result.rtTarget.bottom;
	printf("%d %d %d %d\n", ptStart.x, ptStart.y, ptStop.x, ptStop.y);
	ptText.x = result.rtTarget.left;
	ptText.y = result.rtTarget.bottom;
	if (ptText.y<pImg->height-10)
		ptText.y += 10;
	printf("result.dlabel=%d\n",result.dlabel);
	if(result.dlabel==1)
	{
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		sprintf(text, "val=he confi=%.2f", result.dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8f, 1.0f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	}
	else
	{
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		sprintf(text, "val=fen confi=%.2f", result.dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8f, 1.0f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	}
	printf("%s\n",text);
	cvSaveImage("result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(10);
    cvReleaseImage(&pImg);

	HYSR_Uninit(hTLHandle);

	return 0;
}

int main()
{
  // video();
	int count=0;
	while(1){
	char *filename = "../img/20181119151521.jpg";
	image(filename);
	printf("count = %d \n\n",++count);
	}
	return 0;
}