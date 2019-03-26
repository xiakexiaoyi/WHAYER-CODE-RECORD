#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <string>

#include <opencv2/opencv.hpp>


#include "HYL_LightOff.h"
#include "HYL_MatchRigidBody.h"

//#include <math.h>

#define CRTDBG_MAP_ALLOC   
//#include <stdlib.h>   




#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\LightOff.lib")
#pragma comment(lib, "..\\Debug\\MatchRigidBody.lib")
#else
#pragma comment(lib, "..\\x64\\Release\\LightOff.lib")
#pragma comment(lib, "..\\x64\\Release\\MatchRigidBody.lib")
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
#define MR_READ_FILE_PARA "../MeterRead.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;
//double resultOld;
MVoid onMouse(int Event,int x,int y,int flags,void* param );

//unsigned char *gMeterDes = MNull;
//MLong gDesSize;

int LightOffTrain(HYED_RESULT_LIST *outPattern);	//ѵ��
int LightOffRecog(HYED_RESULT_LIST inPara);	//ʶ��
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
	
	system("pause");

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
	HYL_MatchInit(NULL, &MHandle);

	pOrgImg  = cvLoadImage("..\\IMG_20170602_105402.jpg", 1);//colord  ��׼ͼ    
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
	WY_DEBUG_PRINT("��Ȧ��ƥ��Ŀ�����\n");
	while (1)
	{
		//������б�� ����Ϊ1 �������
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
			printf("ƥ��Ŀ��ѡ����ɣ�����ѡ��ʶ��Ŀ��\n");
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
		}
		if (1 == flagEndL && i != 0 )   //������һ����������
		{
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
		}

		if (13 == cvWaitKey(10))  //enter �˳�
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
	}
	printf("�������\n");
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
	if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//ѵ��ͼƬ���õ�ģ��
	{
		WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
		goto EXT;
	}
	//lMemSize = testImage->width * testImage->height;
	//if (MNull != gMeterDes)
	//{
	//	memset(gMeterDes, 0, lMemSize * sizeof(unsigned char));
	//}
	//else
	//{
	//	gMeterDes = (unsigned char *)malloc(lMemSize * sizeof(unsigned char));
	//}
	//if(0 != HYL_SaveDesMem (MHandle, gMeterDes, lMemSize, &gDesSize))//���������鱣�浽�ڴ��У�gMeterDesΪ��ͷ
	//{
	//	WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
	//}
	if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
	}
EXT:
	HYL_MatchUninit(MHandle);
	return res;
}

int LightOffRecog(HYED_RESULT_LIST resultlist)
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
	//HYL_LightOffInit(NULL, &KRHandle);
	HYL_MatchInit(NULL, &MHandle);

	
	//WY_DEBUG_PRINT("����������ͣ�0���Ȧ��ť����������1�ϰ�Ȧ��ť���Ǻᣩ\n");
	//scanf("%d",&modeltype);
	//if(modeltype == 0 || modeltype == 1 || modeltype == 4)
	//{
	//	sprintf(modelfile,"..\\..\\model\\%d\\all_age_svm.xml",modeltype);
	//	sprintf(Filters0file,"..\\..\\model\\%d\\all_age_filters1.txt",modeltype);
	//	sprintf(Filters1file,"..\\..\\model\\%d\\all_age_filters2.txt",modeltype);
	//}
	//else
	//{
	//	exit(1);//����򵥴���
	//}
	
	
	//HYL_LightOffSetParam(KRHandle,modelfile,Filters0file,Filters1file);//�������㷨�������
	pOrgImg2 = cvLoadImage("..\\IMG_20170602_105402.jpg", 1);
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



	//if(0 != HYL_GetDesMem (MHandle, gMeterDes))//ȡ���ڴ��е���������
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		WY_DEBUG_PRINT("HYAMR_GetTemplateFromText error !\n");
	}

	HYL_GetDashboard(MHandle, &imgs, "OK", 0.45, offset);//ƥ��ģ��,��ͼ��ƫ����
	resultlist.offset.x=offset->x-resultlist.origin.x;
	resultlist.offset.y=offset->y-resultlist.origin.y;

	if (HYL_LightOffExceptionDetection(NULL, &imgs, &resultlist)<0)//ʶ��Ŀ��
	{
		printf("error recognize\n");
		goto EXT;
	}
	int y=0;
	for(int i=1;i < resultlist.lAreaNum;i++)
	{
		CvPoint ptStart, ptStop;
		int left=resultlist.DtArea[i].left+resultlist.offset.x;
		int right=resultlist.DtArea[i].right+resultlist.offset.x;
		int top=resultlist.DtArea[i].top+resultlist.offset.y;
		int bot=resultlist.DtArea[i].bottom+resultlist.offset.y;
		ptStart.x=left;
		ptStart.y=top;
		ptStop.x=right;
		ptStop.y=bot;
		if(resultlist.pResult[i].result == 1)
		{
			printf("����%d״̬:��\n",y+1);
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255),4);
		}
		else if(resultlist.pResult[i].result == 2)
		{
			printf("����%d״̬:��\n",y+1);
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,255,0),4);
		}
		else
		{
			printf("error\n");
		}
		y+=1;
	}
	cvShowImage("Result Show", pImg);
	cvWaitKey(10);

EXT:
	free(offset);
	HYL_MatchUninit(MHandle);
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