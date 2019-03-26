#include "HYL_MatchRigidBody.h"
#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\MatchRigidBody.lib")
#else
#pragma comment(lib, "..\\Release\\MatchRigidBody.lib")
#endif

#define MaxAreaNum 1000
#define MR_READ_FILE_PARA "../MeterRead.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;

MVoid onMouse(int Event,int x,int y,int flags,void* param );

int LightOffTrain(HYED_RESULT_LIST *outPattern);	//训练
int LightOffRecog(HYED_RESULT_LIST inPara);	//识别

int main(){
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
int LightOffTrain(HYED_RESULT_LIST *resultlist){
	int res=0;
	CvSize ImgSize; 
	MHandle MHandle=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	IplImage* pOrgImg  = cvLoadImage("..\\..\\Web\\link\\IMG_20170602_105402.jpg", 1);
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;

	unsigned char *pData;
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	int modeltype=0,flagL = 0,flagR = 0,i=0,j;
	HYL_MatchInit(NULL, &MHandle);
	
	testImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(pOrgImg, testImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", testImage);
	cvNamedWindow("TrainImage2", 1);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);
	printf("请圈定匹配目标对象：\n");
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
		printf("HYAMR_TrainTemplateFromMask error!\n");
		
	}
	if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_SaveTDescriptorsGroup error!\n");
	}
EXT:
	HYL_MatchUninit(MHandle);
	return res;
}
int LightOffRecog(HYED_RESULT_LIST resultlist){
	int res = 0;
	MHandle MHandle=NULL;
	MPOINT *offset;
	offset = (MPOINT*)malloc(1*sizeof(MPOINT));
	HYL_MatchInit(NULL, &MHandle);
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_GetTemplateFromText error !\n");
	}

	//HYL_GetDashboard(MHandle, &imgs, "OK", 0.45, offset);
EXT:
	free(offset);
	HYL_MatchUninit(MHandle);
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