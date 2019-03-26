#include "disconnector_GPU.h"
#include <opencv2/opencv.hpp>
#include<io.h>
#include "putText.h"


#ifdef _WIN64
#pragma comment(lib, "../x64/Release/disconnector_GPU.lib")
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
	int gpu_index = 0;
	char readname[100] = "../testimage/2/";
	char savename[100] = "../testimage/result/";
	HYDSR_Init_GPU(NULL, &hTLHandle);
	/*if(0 != HYDSR_SetParam(hTLHandle, cfgfile, weightfile, thresh, gpu_index, w, h));
	{
		HYDSR_Uninit(hTLHandle);
		return -1;
	}*/
	int flag = HYDSR_SetParam_GPU(hTLHandle, cfgfile, weightfile, thresh, gpu_index, w, h);
	if (0 != flag)
	{
		HYDSR_Uninit_GPU(hTLHandle);
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
		HYDSR_StateRecog_GPU(hTLHandle,&imgs,&resultlist);
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

	

	HYDSR_Uninit_GPU(hTLHandle);
	
	
	
}
