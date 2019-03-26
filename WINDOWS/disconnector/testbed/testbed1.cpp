#include "disconnector.h"
#include <opencv2/opencv.hpp>
#include<io.h>
#include "putText.h"


#ifdef _WIN64
#pragma comment(lib, "../x64/Release/disconnector.lib")
#else
#pragma comment(lib, "../Release/disconnector.lib")
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

int main()
{
	//char *filename="../img/out/201801161840311.jpg";
	/******YOLO模型*******
	char *cfgfile="../model/yolo-voc.cfg";
	char *weightfile="../model/yolo-voc_final.weights";
	/*************/
	/******TINY-YOLO模型*******/
START:

	char *cfgfile = "../model/old/1123/tiny-yolo-voc.cfg";
	char *weightfile = "../model/old/1123/tiny-yolo-voc_70000.weights";
	/*************/
	void *hTLHandle = NULL;
	DSR_IMAGES imgs = {0};
	
	float thresh=0.1;
	
	int w=0,h=0;


	//文件存储信息结构体 
	struct _finddata_t fileinfo;
	//保存文件句柄 
	__int64 fHandle;
	char to_search[100];
	char imgname[100];
	//char txtname[100];
	//char imgsavename[100];
	char resultname[100];
	int len;
	char readname[100] = "../testimage/2/";
	char savename[100] = "../testimage/result/";
	HYDSR_Init(NULL, &hTLHandle);
	/*if(0 != HYDSR_SetParam(hTLHandle, cfgfile, weightfile, thresh, gpu_index, w, h));
	{
		HYDSR_Uninit(hTLHandle);
		return -1;
	}*/
	int flag = HYDSR_SetParam(hTLHandle, cfgfile, weightfile, thresh, w, h);
	if (0 != flag)
	{
		HYDSR_Uninit(hTLHandle);
		return -1;
	}
	sprintf(to_search, "%s*.jpg", readname);
	if ((fHandle = _findfirst(to_search, &fileinfo)) == -1L)
			{
				printf("当前目录下没有jpg文件\n");
				return 0;
			}
	else{
		do{
			
			HYDSR_RESULT_LIST  resultlist = { 0 };
			IplImage* pImg = 0;
			len = strlen(fileinfo.name);
			fileinfo.name[len - 4] = '\0';
			sprintf(imgname, "%s%s.jpg", readname, fileinfo.name);
			//sprintf(imgsavename, "%s%s_o.jpg", savename, fileinfo.name);
			
			//sprintf(txtname, "%s%s.txt", savename, fileinfo.name);\
			IplImage* src = cvLoadImage(imgname, 1);
			printf("%s\n",imgname);

			pImg = cvLoadImage(imgname,CV_LOAD_IMAGE_COLOR); //原图输入

		//w=pImg->width;
		//h=pImg->height;
		resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
		

		imgs.lHeight = pImg->height;
		imgs.lWidth = pImg->width;
		imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
		imgs.pixelArray.chunky.pPixel = pImg->imageData;
		resultlist.lResultNum = 0;
		HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);
	    if(resultlist.lResultNum==0)
			sprintf(resultname, "%s%s_nonesult.jpg", savename, fileinfo.name);
		else
			sprintf(resultname, "%s%s.jpg", savename, fileinfo.name);
		for(int i=0;i < resultlist.lResultNum;i++)
		{
			CvPoint ptStart, ptStop, ptText;
			char text[256]={0};
			char text1[512]={0};
			CvFont font;
			ptStart.x = resultlist.pResult[i].Target.left;
			ptStart.y = resultlist.pResult[i].Target.top;
			ptStop.x = resultlist.pResult[i].Target.right;
			ptStop.y = resultlist.pResult[i].Target.bottom;

			ptText.x = resultlist.pResult[i].Target.left;
			ptText.y = resultlist.pResult[i].Target.bottom;
			if (ptText.y<pImg->height-10)
				ptText.y += 10;
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
			if(resultlist.pResult[i].flag !=1)
			{
				sprintf(text, "%d:state=none", i+1);
			}
			else if(resultlist.pResult[i].dVal==0)
			{
				//continue;
				sprintf(text, "%d:7", i+1);
			}
			else if(resultlist.pResult[i].dVal==1)
			{
				//continue;
				sprintf(text, "%d:n", i+1);
			}
			else if(resultlist.pResult[i].dVal==2)
				sprintf(text, "%d:fen", i+1);
			else if(resultlist.pResult[i].dVal==3)
				sprintf(text, "%d:he", i+1);
			sprintf(text1,"%s %f\n",text,resultlist.pResult[i].dConfidence);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.5f, 1.5f,0,5);
			cvPutText(pImg, text1, ptText, &font,  cvScalar(0,255,0));
		}
		cvSaveImage(resultname, pImg);
		//cvShowImage("Result Show", pImg);
		//cvWaitKey(10);
		cvReleaseImage(&pImg);
		if (resultlist.pResult)
			free(resultlist.pResult);
		
		}while (_findnext(fHandle, &fileinfo) == 0);
	}

	

	HYDSR_Uninit(hTLHandle);
	
	
	
}
int main123()
{
	char single_img[255];//单张图片文件路径
	char single_picname[100];//单张图片文件路径
	char *filename="../testimage/20181118121306.jpg";

	char *cfgfile="../model/tiny-yolo-voc.cfg";
	char *weightfile="../model/tiny-yolo-voc_final.weights";

	IplImage* pImg = 0;
	Mat m;
	DSR_IMAGES imgs = {0};
	void *hTLHandle=NULL;
	float thresh=0.1;
	HYDSR_RESULT_LIST  resultlist ={0};
	int w=0,h=0;
	
	//printf("请输入测试单张照片路径：（例如输入:123.jpg）(图像放在img文件夹下)\n");
	//scanf("%s",&single_picname);

	//IplImage* src = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);    //图像四周加白边
    //pImg = cvCreateImage(cvSize(src->width*1.02, src->height*1.04), src->depth, src->nChannels);
	//cvCopyMakeBorder(src, pImg, cvPoint(src->width*0.01, src->height*0.02), IPL_BORDER_CONSTANT, cvScalarAll(255));

	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR); //原图输入
	//pImg = cvLoadImage(single_picname,CV_LOAD_IMAGE_COLOR); //原图输入
	//pImg = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
	if(!pImg)
	{
		printf("加载图片失败\n");
		system("pause");
		exit(-1);
	}
   
	//m=Mat(pImg,0);
	//putTextZH(m, "操作步骤：\n1、选取单排空开区域。\n", Point(50, 50), Scalar(0, 255, 255), 30, "Arial");
	//pImg=&(IplImage)m;
	w=pImg->width;
	h=pImg->height;
	resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	//HYDSR_Init(NULL,&hTLHandle);
	if(0!=HYDSR_Init(NULL,&hTLHandle))
	{
		printf("HYDSR_Init error.\n");
		return -1;
	}
	//HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);
	if(0!=HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h))
	{
		printf("HYDSR_SetParam error.\n");
		HYDSR_Uninit(hTLHandle);
		return -1;
	}
     
	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	
	//HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);
	if(0!=HYDSR_StateRecog(hTLHandle,&imgs,&resultlist))
	{
		printf("HYDSR_StateRecog error.\n");
		HYDSR_Uninit(hTLHandle);
		return -1;
	}
	/*
	int result=-1;
	float Ctmp=0;
	for(int i=0;i < resultlist.lResultNum;i++)
	{
		if(resultlist.pResult[i].dConfidence > Ctmp)
		{
			Ctmp=resultlist.pResult[i].dConfidence;
			result=resultlist.pResult[i].dVal;
		}
	}
	if(result==-1)
	{
		printf("未找到刀闸\n");
	}
	else if(result==0)
	{
		printf("刀闸状态为分\n");
	}
	else if(result==1)
	{
		printf("刀闸状态为合\n");
	}
	*/
	for(int i=0;i < resultlist.lResultNum;i++)
	{
		CvPoint ptStart, ptStop, ptText;
		char text[100]={0};
		char text1[256]={0};
		CvFont font;
		ptStart.x = resultlist.pResult[i].Target.left;
		ptStart.y = resultlist.pResult[i].Target.top;
		ptStop.x = resultlist.pResult[i].Target.right;
		ptStop.y = resultlist.pResult[i].Target.bottom;
		printf("%d %d %d %d\n",ptStart.x, ptStart.y, ptStop.x, ptStop.y);
		ptText.x = resultlist.pResult[i].Target.left;
		ptText.y = resultlist.pResult[i].Target.bottom;
		if (ptText.y<pImg->height-10)
			ptText.y += 10;
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		if(resultlist.pResult[i].flag !=1)
		{
			sprintf(text, "%d:state=none", i+1);
		}
		else if(resultlist.pResult[i].dVal==0)
		{
			//continue;
			sprintf(text, "%d:7", i+1);
		}
		else if(resultlist.pResult[i].dVal==1)
		{
			//continue;
			sprintf(text, "%d:n", i+1);
		}
		else if(resultlist.pResult[i].dVal==2)
			sprintf(text, "%d:fen", i+1);
		else if(resultlist.pResult[i].dVal==3)
			sprintf(text, "%d:he", i+1);
		//printf("%f\n",resultlist.pResult[i].dConfidence);
		sprintf(text1,"%s %.2f\n",text,resultlist.pResult[i].dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8f, 1.0f);
		cvPutText(pImg, text1, ptText, &font,  cvScalar(0,255,0));
		printf("%s\n", text1);
	
	}
	cvSaveImage("../result/result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);


	cvReleaseImage(&pImg);
	if (resultlist.pResult)
		free(resultlist.pResult);
	HYDSR_Uninit(hTLHandle);
	
	return 0;

}