#include <stdio.h>
#include <string.h>
#include<io.h>
#include <opencv2/opencv.hpp>
#include "SwitchRecog_GPU.h"

//#include <boost/filesystem.hpp>
//#include <boost/thread.hpp>
//#include <boost/thread/condition.hpp>
//#include <boost/thread/mutex.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/timer.hpp>
//#include <boost/bind.hpp>


#ifdef _DEBUG
#pragma comment(lib, "../Debug/SwitchRecog.lib")
#pragma comment(lib, "../Debug/yolo.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")

#else
#pragma comment(lib, "../x64/Release/SwitchRecog_GPU.lib")

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
//
//void detc(int threadNum)
//{
//	float thresh=0.24;
//	void *hTLHandle=NULL;
//	IplImage* pImg = 0;
//	//IplImage* pImg1 = 0;
//	//IplImage* pImg2 = 0;
//	SR_IMAGES imgs = {0};
//	HYSR_RESULT  resultlist ={0};
//	/*DSR_IMAGES imgs1 = {0};
//	HYDSR_RESULT_LIST  resultlist1 ={0};
//	DSR_IMAGES imgs2 = {0};
//	HYDSR_RESULT_LIST  resultlist2 ={0};*/
//	char *cfgfile="../model/tiny-yolo-voc_IO.cfg";
//	char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
//
//	//char *cfgfile="../model/yolo-voc.cfg";
//	//char *weightfile="../model/yolo-voc_final.weights";
//	
//	
//	pImg = cvLoadImage("../img/TIM图片20180718095905.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
//	//pImg1 = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
//	//pImg2 = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
//	int w=0,h=0;
//    int flag=0;
//	w=pImg->width;
//	h=pImg->height;
//	printf("ThreadNum: %d_befor  HYDSR_Init \n", threadNum);
//	HYSR_Init(NULL,&hTLHandle);
//	printf("ThreadNum: %d_after HYDSR_Init\n", threadNum);
//	printf("ThreadNum: %d_befor HYDSR_SetParam  \n", threadNum);
//	flag=HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh);
//	printf("ThreadNum: %d_after HYDSR_SetParam\n", threadNum);
//    if(flag != 0)
//	{
//		printf("Release_%d\n",threadNum);
//		cvReleaseImage(&pImg);
//		HYSR_Uninit(hTLHandle);
//		return;
//	}
//
//	imgs.lHeight = pImg->height;
//	imgs.lWidth = pImg->width;
//	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
//	imgs.pixelArray.chunky.pPixel = pImg->imageData;
//	/*resultlist1.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
//	imgs1.lHeight = pImg1->height;
//	imgs1.lWidth = pImg1->width;
//	imgs1.pixelArray.chunky.lLineBytes = pImg1->widthStep;
//	imgs1.pixelArray.chunky.pPixel = pImg1->imageData;
//	resultlist2.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
//	imgs2.lHeight = pImg2->height;
//	imgs2.lWidth = pImg2->width;
//	imgs2.pixelArray.chunky.lLineBytes = pImg2->widthStep;
//	imgs2.pixelArray.chunky.pPixel = pImg2->imageData;*/
//	printf("ThreadNum: %d_befor  HYDSR_StateRecog \n", threadNum);
//	HYSR_SwitchRecog(hTLHandle,&imgs,&resultlist);
//	printf("ThreadNum: %d_after  HYDSR_StateRecog \n", threadNum);
//
//	//boost::thread t1(boost::bind(HYDSR_StateRecog, hTLHandle, &imgs1, &resultlist1));
//    //boost::thread t2(boost::bind(HYDSR_StateRecog, hTLHandle, &imgs2, &resultlist2));
//	//t1.join();
//    //t2.join();
//	/*while (1)
//    {
//       cvWaitKey(10);
//    }*/
//   /*  if(t1.joinable())
//     {
//      t1.join();
//     }
//    
//
//    if(t2.joinable())
//     {
//      t2.join();
//     }*/
//	//Tmpresultlist.pResult[0].Target.left;
//	//printf("lResultNum%d=%d\n",threadNum,resultlist.pResult[0].Target.left);
//	printf("lResultNum%d=%d\n",threadNum, resultlist.rtTarget.left);
//	//printf("lResultNum1=%d\n",resultlist1.lResultNum);
//	//printf("lResultNum2=%d\n",resultlist2.lResultNum);
//	//for(int i=0;i<resultlist.lResultNum;i++)
//	 {
//		 //printf("dConfidence%d=%f\n",threadNum,resultlist.pResult[i].dConfidence);
//	 }
//     /*for(int i=0;i<resultlist1.lResultNum;i++)
//	 {
//		 printf("dConfidence1=%f\n",resultlist1.pResult[i].dConfidence);
//	 }
//	 for(int i=0;i<resultlist2.lResultNum;i++)
//	 {
//		 printf("dConfidence2=%f\n",resultlist2.pResult[i].dConfidence);
//	 }*/
//	cvReleaseImage(&pImg);
//	//cvReleaseImage(&pImg1);
//	//cvReleaseImage(&pImg2);
//	//if (resultlist.pResult)
//	//	free(resultlist.pResult);
//	/*if (resultlist1.pResult)
//		free(resultlist1.pResult);
//	if (resultlist2.pResult)
//		free(resultlist2.pResult);*/
//	HYSR_Uninit(hTLHandle);
//
//}
//int main()
//{
//	//detc(1);
//	char *cfgfile="../model/tiny-yolo-voc_IO.cfg";
//	char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
//	while(1)
//	{
//		boost::thread t1(boost::bind(detc, 1));
//		//boost::thread t2(boost::bind(detc, 2));
//		//boost::thread t3(boost::bind(detc, 3));
//		//boost::thread t4(boost::bind(detc, 4));
//		//boost::thread t5(boost::bind(detc, 5));
//		//boost::thread t6(boost::bind(detc, 6));
//		//boost::thread t7(boost::bind(detc, 7));
//		//boost::thread t8(boost::bind(detc, 8));
//		t1.join();
//		//t2.join();
//		//t3.join();
//		//t4.join();
//		//t5.join();
//		//t6.join();
//		//t7.join();
//		//t8.join();
//	}
//
//	return 0;
//}
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
	int gpu_index = 0;
	IplImage* src = 0;
	SR_IMAGES imgs = {0};
 
	HYSR_Init_GPU(NULL,&hTLHandle);
	HYSR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh,gpu_index);
	
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
				c=HYSR_SwitchRecog_GPU(hTLHandle,&imgs,&result);
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

	
	HYSR_Uninit_GPU(hTLHandle);

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
	int gpu_index = 0;
	//HYSR_Init(NULL,&hTLHandle);
	if(0!=HYSR_Init_GPU(NULL,&hTLHandle))
	{
		printf("HYSR_Init error.\n");
		return -1;
	}
	if(0!=HYSR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh, gpu_index))
	{
		printf("HYSR_SetParam error.\n");
		HYSR_Uninit_GPU(hTLHandle);
		return -1;
	}
	//HYSR_SetParam(hTLHandle,cfgfile,weightfile,thresh);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	result.dlabel=-1;
	//HYSR_SwitchRecog(hTLHandle,&imgs,&result);
	if(0!=HYSR_SwitchRecog_GPU(hTLHandle,&imgs,&result))
	{
		printf("HYSR_SwitchRecog error.\n");
		HYSR_Uninit_GPU(hTLHandle);
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

	HYSR_Uninit_GPU(hTLHandle);

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