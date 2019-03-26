#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif
#include "sigar_fileinfo.h"
#ifdef __cplusplus
}
#endif

#include "highgui.h"
#include "cv.h"


#include "HYL_KnobReg.h"
#include "HYL_MatchRigidBody.h"

#include <math.h>

#define CRTDBG_MAP_ALLOC   
#include <stdlib.h>   
#include <crtdbg.h>   

#include <intrin.h>

#pragma comment( lib, "vfw32.lib" )   
#pragma comment( lib, "comctl32.lib" )   

#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\Knob.lib")
#pragma comment(lib, "..\\Debug\\MatchRigidBody.lib")
#else
#pragma comment(lib, "..\\Release\\Knob.lib")
#pragma comment(lib, "..\\Release\\MatchRigidBody.lib")
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
#define MR_READ_FILE_PARA "C:\\MeterRead.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;
//double resultOld;
MVoid onMouse(int Event,int x,int y,int flags,void* param );

//unsigned char *gMeterDes = MNull;
//MLong gDesSize;

int KnobTrain(HYED_RESULT_LIST *outPattern);	//训练
int KnobRecog(HYED_RESULT_LIST inPara);	//识别
//旋钮开关
int main(int argc, char** argv){
	sigar_t*   p_sigar;
	sigar_open(&p_sigar);
	sigar_proc_mem_t procmem;
	sigar_proc_cpu_t proccpu;
	sigar_proc_state_t procstate;
	sigar_pid_t pid = sigar_pid_get(p_sigar);

	HYED_RESULT_LIST resultlist = {0};
	resultlist.DtArea = (HYED_DTAREA*)malloc(MaxAreaNum*sizeof(HYED_DTAREA));
	resultlist.pResult = (HYED_RESULT*)malloc(MaxAreaNum*sizeof(HYED_RESULT)) ;
	
	KnobTrain(&resultlist);
	while(1)
	{
		if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
			return false;
		}
		if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
			return false;
		}

		if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
			return false;
		}
		printf("开始：size=%d,resident=%d,share=%d\n",procmem.size,procmem.resident,procmem.share);

	    KnobRecog(resultlist);
	
		if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
			return false;
		}
		if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
			return false;
		}

		if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
			return false;
		}
		printf("结束：size=%d,resident=%d,share=%d\n\n",procmem.size,procmem.resident,procmem.share);
	}
	if(p_sigar)
	{
		sigar_close(p_sigar);
		p_sigar = NULL;
	}
	if (resultlist.DtArea)
		free(resultlist.DtArea);
	if (resultlist.pResult)
		free(resultlist.pResult);
	
	system("pause");

	return 0;
}

int KnobTrain(HYED_RESULT_LIST *resultlist)
{
	int res = 0;;
    IplImage *pImg = NULL, *pImg2=NULL, *pOrgImg=NULL, *pOrgImg2=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	MHandle MHandle=NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	HYL_IMAGES imgs = {0}, imgs2={0},mask={0};
	CvSize ImgSize; 
	
	float scale=1;
	unsigned char *pData;
	

	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};

	int modeltype=0,flagL = 0,flagR = 0,i=0,j;
	HYL_MatchInit(NULL, &MHandle);

	pOrgImg  = cvLoadImage("..\\..\\Web\\CaptureFiles\\2.jpg", 1);//colord  基准图    
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
	cvNamedWindow("TrainImage2", 1);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);
	WY_DEBUG_PRINT("请圈定匹配目标对象：\n");
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
			printf("匹配目标选择完成，下面选择识别目标\n");
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
	resultlist->origin.x=(resultlist->DtArea[0].left+resultlist->DtArea[0].right)/2;
	resultlist->origin.y=(resultlist->DtArea[0].top+resultlist->DtArea[0].bottom)/2;
	//WY_DEBUG_PRINT("start(%d,%d), end(%d,%d)\n", startPt.x, startPt.y, endPt.x, endPt.y);
	/*cvSetImageROI(testImage,cvRect(resultlist.DtArea[0].left,resultlist.DtArea[0].top,resultlist.DtArea[0].right-resultlist.DtArea[0].left,resultlist.DtArea[0].bottom-resultlist.DtArea[0].top));
	cvSaveImage("D:\\testImage.bmp", testImage);*/
	maskImage = cvCreateImage(ImgSize, 8, 1);   //mask image
	pData = (unsigned char *)(maskImage->imageData);
	for (j=0;j<ImgSize.height;j++)
	{
		for (i=0;i<ImgSize.width;i++)
		{
			if (j>=resultlist->DtArea[0].top&&j<=resultlist->DtArea[0].bottom&&i>=resultlist->DtArea[0].left&&i<=resultlist->DtArea[0].right)
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

	wy_testImage.lWidth = testImage->width;
	wy_testImage.lHeight = testImage->height;
	wy_testImage.pixelArray.chunky.lLineBytes = testImage->widthStep;
	wy_testImage.pixelArray.chunky.pPixel = testImage->imageData;
	wy_testImage.lPixelArrayFormat = HYL_IMAGE_BGR;
	//mask
	wy_maskImage.lWidth = maskImage->width;
	wy_maskImage.lHeight = maskImage->height;
	wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	wy_maskImage.lPixelArrayFormat = HYL_IMAGE_GRAY;
	if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//训练图片，得到模板
	{
		WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
		goto EXT;
	}
	//MLong lMemSize;
	//lMemSize = testImage->width * testImage->height;
	//if (MNull != gMeterDes)
	//{
	//	memset(gMeterDes, 0, lMemSize * sizeof(unsigned char));
	//}
	//else
	//{
	//	gMeterDes = (unsigned char *)malloc(lMemSize * sizeof(unsigned char));
	//}
	//if(0 != HYL_SaveDesMem (MHandle, gMeterDes, lMemSize, &gDesSize))//将描述符块保存到内存中，gMeterDes为开头
	if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
	}
EXT:
	cvReleaseImage(&pImg);
	cvReleaseImage(&pImg2);
	cvReleaseImage(&pOrgImg);
	cvReleaseImage(&pOrgImg2);
	cvReleaseImage(&maskImage);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&testImage);
	HYL_MatchUninit(MHandle);
	return res;
}

int KnobRecog(HYED_RESULT_LIST resultlist)
{
	int res = 0;
	char modelfile[50];
	char Filters0file[50];
	char Filters1file[50];
    IplImage *pImg = NULL, *pImg2=NULL, *pOrgImg=NULL, *pOrgImg2=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	MHandle KRHandle=NULL;
	MHandle MHandle=NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	HYL_IMAGES imgs = {0}, imgs2={0},mask={0};
	CvSize ImgSize; 
	float scale=1;
	
	MPOINT *offset;

	

	offset = (MPOINT*)malloc(1*sizeof(MPOINT));
	int modeltype=0,flagL = 0,flagR = 0,i=0;
	HYL_KnobRegInit(NULL, &KRHandle);
	HYL_MatchInit(NULL, &MHandle);

	
	//WY_DEBUG_PRINT("输入分析类型：0左半圈旋钮（横竖），1上半圈旋钮（非横）\n");
	//scanf("%d",&modeltype);
	modeltype=0;
	if(modeltype == 0 || modeltype == 1)
	{
		sprintf(modelfile,"..\\..\\model\\1\\all_age_svm0");
		sprintf(Filters0file,"..\\..\\model\\1\\all_age_svm1");
	}
	else
	{
		exit(1);//错误简单处理
	}

	
	
	HYL_KnobRegSetParam(KRHandle,modelfile,Filters0file,Filters1file);//分类器算法输入参数

	pOrgImg2 = cvLoadImage("..\\..\\Web\\CaptureFiles\\2.jpg", 1);
	if (!pOrgImg2)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg2->height;
	ImgSize.width = pOrgImg2->width;
	pImg = cvCreateImage(ImgSize, IPL_DEPTH_8U,3);
	cvResize(pOrgImg2, pImg);
	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.lPixelArrayFormat = HYL_IMAGE_BGR;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;



	//if(0 != HYL_GetDesMem (MHandle, gMeterDes))//取得内存中的描述符块
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		WY_DEBUG_PRINT("HYAMR_GetTemplateFromText error !\n");
	}

	HYL_GetDashboard(MHandle, &imgs, "OK", 0.45, offset);//匹配模板,求图像偏移量
	resultlist.offset.x=offset->x-resultlist.origin.x;
	resultlist.offset.y=offset->y-resultlist.origin.y;
	if (HYL_KnobRegExceptionDetection(KRHandle, &imgs, &resultlist)<0)//识别目标
	{
		printf("error recognize\n");
		goto EXT;
	}

	//for(int i=1;i < resultlist.lAreaNum;i++)
	//{
	//	if(modeltype == 1)
	//	{
	//		if(resultlist.pResult[i].result == 3)
	//		{
	//		}
	//		else if(resultlist.pResult[i].result == 1)
	//		{
	//			resultlist.pResult[i].result=2;
	//		}
	//		else if(resultlist.pResult[i].result ==2)
	//		{
	//			resultlist.pResult[i].result=1;
	//		}
	//	}
	//	else if(modeltype == 0)
	//	{
	//		if(resultlist.pResult[i].result == 1)
	//		{
	//		}
	//		else if(resultlist.pResult[i].result == 3 ||resultlist.pResult[i].result == 2)
	//		{
	//			resultlist.pResult[i].result=2;
	//		}
	//	}
	//}

	int y=0;
	for(int i=1;i < resultlist.lAreaNum;i++)
	{
		if(modeltype == 1)
		{
			if(resultlist.pResult[i].result == 3)
			{
				printf("开关%d状态：偏左\n",y+1);
			}
			else if(resultlist.pResult[i].result == 1)
			{
				printf("开关%d状态：竖直\n",y+1);
			}
			else if(resultlist.pResult[i].result ==2)
			{
				printf("开关%d状态：偏右\n",y+1);
			}
		}
		else if(modeltype == 0)
		{
			if(resultlist.pResult[i].result == 1)
			{
				printf("开关%d状态：竖\n",y+1);
			}
			else if(resultlist.pResult[i].result == 3 ||resultlist.pResult[i].result == 2)
			{
				printf("开关%d状态：横\n",y+1);
			}
		}
		y+=1;
	}

EXT:
	cvReleaseImage(&pImg);
	cvReleaseImage(&pImg2);
	cvReleaseImage(&pOrgImg);
	cvReleaseImage(&pOrgImg2);
	cvReleaseImage(&maskImage);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&testImage);
	free(offset);
	HYL_MatchUninit(MHandle);
	HYL_KnobRegUninit(KRHandle);
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