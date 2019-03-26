#include "disconnector.h"
#include <opencv2/opencv.hpp>
#include "putText.h"
#include<io.h>

#ifdef _DEBUG
#pragma comment(lib, "../Debug/stateRecog.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")

#else
#pragma comment(lib, "../Release/disconnector.lib")
#pragma comment(lib, "../Release/opencv_core244.lib")
#pragma comment(lib, "../Release/opencv_highgui244.lib")
#pragma comment(lib, "../Release/opencv_imgproc244.lib")

using namespace cv;
using namespace std;

#endif
int main()
{
	//文件存储信息结构体 
	struct _finddata_t fileinfo;
	//保存文件句柄 
	long fHandle;
	char single_img[9000];//单张图片文件路径
	char result_img[9000];//单张图片文件路径
	char single_picname[9000];//单张图片文件路径
	/******YOLO模型*******
	char *cfgfile="../model/0313/yolo-voc.cfg";
	char *weightfile="../model/0313/yolo-voc_final.weights";
	/*************/
	/******TINY-YOLO模型*******/
	char *cfgfile="../model/1123/tiny-yolo-voc.cfg";
	char *weightfile="../model/1123/tiny-yolo-voc_70000.weights";
	/*************/
	
	
	DSR_IMAGES imgs = {0};
	
	float thresh=0.24;
	
	int w=0,h=0;
	char to_search[9000];



	printf("请输入测试照片路径：（例如输入:E:\\nest_photos）(注：路径中不能包含空格)(回车确定)\n");
	scanf("%s",&single_picname);
	printf("分析开始\n");
	char result_path[9000]="..\\result\\";//单张图片文件路径
	sprintf(to_search, "%s\\*.jpg", single_picname);
	if ((fHandle = _findfirst(to_search, &fileinfo)) == -1L)
	{
		//printf("当前目录下没有jpg文件\n");
		Mat m;
		IplImage* pImg = cvLoadImage(single_picname,CV_LOAD_IMAGE_COLOR);
		if(!pImg)
			{
				printf("当前目录下没有jpg文件\n");
				system("pause");
				exit(-1);
			}
		void *hTLHandle=NULL;
		HYDSR_RESULT_LIST  resultlist ={0};
		w=pImg->width;
		h=pImg->height;
		
		resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
		
		
		HYDSR_Init(NULL,&hTLHandle);
		HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);

		imgs.lHeight = pImg->height;
		imgs.lWidth = pImg->width;
		imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
		imgs.pixelArray.chunky.pPixel = pImg->imageData;

		HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);

		/*int result=-1;
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
		}*/
		printf("resultlist.lResultNum=%d\n",resultlist.lResultNum);
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

			ptText.x = resultlist.pResult[i].Target.left;
			ptText.y = resultlist.pResult[i].Target.bottom;
			if (ptText.y<pImg->height-10)
				ptText.y += 10;
			if(resultlist.pResult[i].dVal==0)
				cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255),8);		
			else
				cvRectangle(pImg, ptStart, ptStop, cvScalar(0,255,0),8);	
			if(resultlist.pResult[i].flag !=1)
			{
				sprintf(text, "%d:state=none", i+1);
			}
			else if(resultlist.pResult[i].dVal==0)
			{
				//continue;
				sprintf(text, "分");
			}
			else if(resultlist.pResult[i].dVal==1)
			{
				//continue;
				sprintf(text, "合");
			}
			/*m=cvarrToMat(pImg);
			if(resultlist.pResult[i].dVal==0)
				putTextZH(m, text, ptText, Scalar(0, 0, 255), 30, "Arial");
			else
				putTextZH(m, text, ptText, Scalar(0, 255, 0), 30, "Arial");
			pImg=&(IplImage)m;*/
		}

		/*m=cvarrToMat(pImg);
		if(result==-1)
		{
			putTextZH(m, "未找到刀闸\n", Point(50, 50), Scalar(255, 0, 0), 30, "Arial");
		}
		else if(result==0)
		{
			putTextZH(m, "刀闸状态为分\n", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
		}
		else if(result==1)
		{
			putTextZH(m, "刀闸状态为合\n", Point(50, 50), Scalar(0, 255, 0), 30, "Arial");
		}
		pImg=&(IplImage)m;*/
		cvSaveImage("..\\result\\result.jpg", pImg);
		//system("pause");
		return 0;	
	}
	else{
		do{
			void *hTLHandle=NULL;
			IplImage* pImg = 0;
			Mat m;
			HYDSR_RESULT_LIST  resultlist ={0};
			int len = strlen(fileinfo.name);
			fileinfo.name[len - 4] = '\0';
			sprintf(single_img, "%s\\%s.jpg", single_picname, fileinfo.name);
			sprintf(result_img, "%s\\%s_result.jpg", result_path, fileinfo.name);

			
			pImg = cvLoadImage(single_img,CV_LOAD_IMAGE_COLOR); //原图输入
			if(!pImg)
			{
				printf("加载图片失败\n");
				system("pause");
				exit(-1);
			}

			printf("%s\n", single_img);
			
			w=pImg->width;
			h=pImg->height;
			
			resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
			
			
			HYDSR_Init(NULL,&hTLHandle);
			HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);

			imgs.lHeight = pImg->height;
			imgs.lWidth = pImg->width;
			imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
			imgs.pixelArray.chunky.pPixel = pImg->imageData;

			HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);

			/*int result=-1;
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
			}*/
			m=cvarrToMat(pImg);
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

				ptText.x = resultlist.pResult[i].Target.left;
				ptText.y = resultlist.pResult[i].Target.bottom;
				if (ptText.y<pImg->height-10)
					ptText.y += 10;
				if(resultlist.pResult[i].dVal==0)
					cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255),8);		
				else
					cvRectangle(pImg, ptStart, ptStop, cvScalar(0,255,0),8);	
				if(resultlist.pResult[i].flag !=1)
				{
					sprintf(text, "%d:state=none", i+1);
				}
				else if(resultlist.pResult[i].dVal==0)
				{
					//continue;
					sprintf(text, "分");
				}
				else if(resultlist.pResult[i].dVal==1)
				{
					//continue;
					sprintf(text, "合");
				}
				
				
			/*	if(resultlist.pResult[i].dVal==0)
					putTextZH(m, text, ptText, Scalar(0, 0, 255), 30, "Arial");
				else
					putTextZH(m, text, ptText, Scalar(0, 255, 0), 30, "Arial");*/
				
				
			}
			pImg=&(IplImage)m;
			/*m=cvarrToMat(pImg);
			if(result==-1)
			{
				putTextZH(m, "未找到刀闸\n", Point(50, 50), Scalar(255, 0, 0), 30, "Arial");
			}
			else if(result==0)
			{
				putTextZH(m, "刀闸状态为分\n", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
			}
			else if(result==1)
			{
				putTextZH(m, "刀闸状态为合\n", Point(50, 50), Scalar(0, 255, 0), 30, "Arial");
			}
			pImg=&(IplImage)m;*/

			cvSaveImage(result_img, pImg);
			//cvShowImage("Result Show", pImg);
			//cvWaitKey(10);

			//m.release();
			cvReleaseImage(&pImg);
			if (resultlist.pResult)
				free(resultlist.pResult);
			HYDSR_Uninit(hTLHandle);
		}while (_findnext(fHandle, &fileinfo) == 0);
	}
	printf("分析完毕\n");
	//IplImage* src = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);    //图像四周加白边
    //pImg = cvCreateImage(cvSize(src->width*1.02, src->height*1.04), src->depth, src->nChannels);
	//cvCopyMakeBorder(src, pImg, cvPoint(src->width*0.01, src->height*0.02), IPL_BORDER_CONSTANT, cvScalarAll(255));

	//pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR); //原图输入
/*
	pImg = cvLoadImage(single_img,CV_LOAD_IMAGE_COLOR); //原图输入
	if(!pImg)
	{
		printf("加载图片失败\n");
		system("pause");
		exit(-1);
	}


	
	w=pImg->width;
	h=pImg->height;
	
	resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	
	
	HYDSR_Init(NULL,&hTLHandle);
	HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;

	HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);
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
		//cvPutText(pImg, text1, ptText, &font,  cvScalar(0,255,0));
		
	
	}
	m=cvarrToMat(pImg,true);
	if(result==-1)
	{
		putTextZH(m, "未找到刀闸\n", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
	}
	else if(result==0)
	{
		putTextZH(m, "刀闸状态为分\n", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
	}
	else if(result==1)
	{
		putTextZH(m, "刀闸状态为合\n", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
	}
	pImg=&(IplImage)m;
	cvSaveImage("../img/result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);


	//cvReleaseImage(&pImg);
	if (resultlist.pResult)
		free(resultlist.pResult);
	HYDSR_Uninit(hTLHandle);
	*/
	return 0;

}