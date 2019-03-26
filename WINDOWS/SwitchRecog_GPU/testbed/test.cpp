#include <stdio.h>
#include <string.h>
#include<io.h>
#include <opencv2/opencv.hpp>
#include "SwitchRecog.h"

#ifdef _DEBUG
#pragma comment(lib, "../Debug/SwitchRecog.lib")
#pragma comment(lib, "../Debug/YoloDetector.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")
#pragma comment(lib, "../Debug/opencv_imgproc244d.lib")
#pragma comment(lib, "../Debug/opencv_ml244d.lib")

#else
#pragma comment(lib, "../Release/SwitchRecog.lib")
#pragma comment(lib, "../Release/yolo.lib")
#pragma comment(lib, "../Release/opencv_core244.lib")
#pragma comment(lib, "../Release/opencv_highgui244.lib")

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



int image()
{
	//文件存储信息结构体 
    struct _finddata_t fileinfo;
    //保存文件句柄 
    long fHandle;

	HYSR_RESULT  result ={0};
	void *hTLHandle=NULL;
	float thresh=0.1;
	char *cfgfile="../model/tiny-yolo-voc.cfg";
	char *weightfile="../model/tiny-yolo-voc_74000.weights";
	//char *cfgfile="../model/tiny-yolo-voc_IO.cfg";
	//char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
	//char *filename="../5011开关B相.jpg";

	char txtname[50]="E:/WhayerCode/Web/result/result.txt";
	char to_search[50];
	char to_read[50];
	char imgsavename[50];
	char readname[50]="E:/WhayerCode/Web/fenhe0/";
	char savename[50]="E:/WhayerCode/Web/result/";
	sprintf(to_search,"%s*.jpg",readname);
	FILE *fp = NULL;
	IplImage* pImg = 0;
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
			sprintf(to_read,"%s%s",readname,fileinfo.name);
			pImg = cvLoadImage(to_read,CV_LOAD_IMAGE_COLOR);
			imgs.lHeight = pImg->height;
			imgs.lWidth = pImg->width;
			imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
			imgs.pixelArray.chunky.pPixel = pImg->imageData;
			result.dlabel=-1;
			HYSR_SwitchRecog(hTLHandle,&imgs,&result);
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
			fp = fopen(txtname, "a");
			if(result.dlabel==1)
				fprintf(fp,"%s  he %s\n",fileinfo.name,text);
			else if(result.dlabel==0)
				fprintf(fp,"%s  fen %s\n",fileinfo.name,text);
			else if(result.dlabel==-1)
				fprintf(fp,"%s  unidentification %s\n",fileinfo.name,text);
			else
				fprintf(fp,"%s  error %s\n",fileinfo.name,text);
			fclose(fp);
			printf("%s\n",text);
			sprintf(imgsavename,"%s%s",savename,fileinfo.name);
			cvSaveImage(imgsavename, pImg);

			//cvShowImage("Result Show", pImg);
			//cvWaitKey(10);


			cvReleaseImage(&pImg);
		}while( _findnext(fHandle,&fileinfo)==0);
	}
	_findclose( fHandle );

	/*char *filename="../1.jpg";
	IplImage* pImg = 0;
	SR_IMAGES imgs = {0};

	
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
    
	HYSR_Init(NULL,&hTLHandle);
	HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	HYSR_SwitchRecog(hTLHandle,&imgs,&result);


	
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
	printf("%s\n",text);
	cvSaveImage("result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);*/


	HYSR_Uninit(hTLHandle);

	return 0;
}


int main()
{
  // video();
	image();
}