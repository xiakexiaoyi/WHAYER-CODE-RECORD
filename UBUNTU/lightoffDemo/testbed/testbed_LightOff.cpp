#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "HYL_LightOff.h"
#include "HYL_MatchRigidBody.h"

#define MaxAreaNum 1000
#define L_SIZE 100*1024*1024
#define MR_READ_FILE_PARA "../../MatchRigidBody.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;

MVoid onMouse(int Event,int x,int y,int flags,void* param );

int LightOffTrain(HYED_RESULT_LIST *outPattern);	//ѵ��
int LightOffRecog(HYED_RESULT_LIST inPara);	//ʶ��

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
	
	return 0;
}

int LightOffTrain(HYED_RESULT_LIST *resultlist)
{
	int res = 0;;
	IplImage *pImg = NULL, *pImg2=NULL, *pOrgImg=NULL, *pOrgImg2=NULL;
	IplImage *maskImage= NULL,*tmpImage = NULL,*testImage = NULL;
	MHandle hMemMgr=NULL;
	MHandle MHandle=NULL;
	MVoid *pMem=NULL;
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
	
	pMem=malloc(L_SIZE);  //��׼��ʼ��
	if(!pMem){
		printf("malloc error\n");
		return -1;
	}

	hMemMgr=HYL_MemMgrCreate(pMem,L_SIZE);
	if(!hMemMgr)
	{
		printf("HYL_MemMgrCreate error\n");
		free(pMem);
		pMem=NULL;
		return -1;
	}
	HYL_MatchInit(hMemMgr, &MHandle);
	if(!MHandle)
	{
		printf("HYL_MatchInit error!!!\n");
		return -1;
	}

	pOrgImg  = cvLoadImage("../../photo/IMG_20170602_105402.jpg", 1);//colord  ��׼ͼ    
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;
	testImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(pOrgImg, testImage, CV_INTER_LINEAR);
	cvSaveImage("testImage.bmp", testImage);
	cvNamedWindow("TrainImage2", 1);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);
	printf("please select the match area.\n");
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
			tmpImage = cvLoadImage("testImage.bmp",CV_LOAD_IMAGE_COLOR);
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
			tmpImage = cvLoadImage("testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage2",tmpImage);
			resultlist->DtArea[i].right = endPt.x;
			resultlist->DtArea[i].bottom = endPt.y ; 				
		}
		
		if(1 == flagEndL && i == 0 )
		{
			printf("Matching complete, please select detection target. Press enter to finish\n");
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
		if (-1 != cvWaitKey(10))  //enter �˳�
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
	}
	printf("Detection complete.\n");
	resultlist->origin.x=(resultlist->DtArea[0].left+resultlist->DtArea[0].right)/2;//���㻭�����ĵ�
	resultlist->origin.y=(resultlist->DtArea[0].top+resultlist->DtArea[0].bottom)/2;
	
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
	cvSaveImage("maskImage.bmp",maskImage);

	wy_testImage.lWidth = pOrgImg->width;
	wy_testImage.lHeight = pOrgImg->height;
	wy_testImage.pixelArray.chunky.lLineBytes = pOrgImg->widthStep;
	wy_testImage.pixelArray.chunky.pPixel = pOrgImg->imageData;
	wy_testImage.lPixelArrayFormat = HYL_IMAGE_BGR;
	//mask
	wy_maskImage.lWidth = maskImage->width;
	wy_maskImage.lHeight = maskImage->height;
	wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	wy_maskImage.lPixelArrayFormat = HYL_IMAGE_GRAY;
	if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//ѵ��ͼƬ���õ�ģ��
	{
		printf("HYAMR_TrainTemplateFromMask error!\n");
		goto EXT;
	}
	if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_SaveTDescriptorsGroup error!\n");
	}
EXT:
	cvReleaseImage(&pOrgImg);
	cvReleaseImage(&testImage);
	cvReleaseImage(&tmpImage);
	HYL_MatchUninit(MHandle);
	HYL_MemMgrDestroy(hMemMgr);
	if(pMem!=NULL){
		free(pMem);
		pMem=NULL;
	}
	return res;
}

int LightOffRecog(HYED_RESULT_LIST resultlist)
{
	int res = 0;
	int y=0;
    	IplImage *pImg = NULL,*pOrgImg2=NULL;
	
	MHandle hMemMgr=NULL;
	MHandle MHandle=NULL;
	MVoid *pMem=NULL;
	HYL_IMAGES imgs = {0};
	CvSize ImgSize; 
	float scale=1;
	
	MPOINT *offset;
	offset = (MPOINT*)malloc(1*sizeof(MPOINT));
	
	pOrgImg2 = cvLoadImage("../../photo/IMG_20170602_105402.jpg", 1);//��ȡ����ͼ
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

	pMem=malloc(L_SIZE);  //��׼��ʼ��
	if(!pMem){
		printf("malloc error\n");
		return -1;
	}

	hMemMgr=HYL_MemMgrCreate(pMem,L_SIZE);
	if(!hMemMgr)
	{
		printf("HYL_MemMgrCreate error\n");
		free(pMem);
		pMem=NULL;
		return -1;
	}
	HYL_MatchInit(hMemMgr, &MHandle);
	if(!MHandle)
	{
		printf("HYL_MatchInit error!!!\n");
		return -1;
	}
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))//��ȡģ��
	{
		printf("HYAMR_GetTemplateFromText error !\n");
	}

	HYL_GetDashboard(MHandle, &imgs, "OK", 0.45, offset);//ƥ��ģ��,��ͼ��ƫ����
	resultlist.offset.x=offset->x-resultlist.origin.x;
	resultlist.offset.y=offset->y-resultlist.origin.y;

	if (HYL_LightOffExceptionDetection(NULL, &imgs, &resultlist)<0)//ʶ��Ŀ��
	{
		printf("error recognize\n");
		goto EXT;
	}
	
	for(int i=1;i < resultlist.lAreaNum;i++)//��ͼƬ����ʾ
	{
		CvPoint ptStart,ptStop;
		int left=resultlist.DtArea[i].left+resultlist.offset.x;
		int right=resultlist.DtArea[i].right+resultlist.offset.x;
		int top=resultlist.DtArea[i].top+resultlist.offset.y;
		int bot=resultlist.DtArea[i].bottom+resultlist.offset.y;
		ptStart.x=left;
		ptStart.y=top;
		ptStop.x=right;
		ptStop.y=bot;
		if(resultlist.pResult[i].result == 1)//��
		{
			printf("light%dstate:light\n",y+1);
			cvRectangle(pImg,ptStart,ptStop,cvScalar(0,0,255),4);
		}
		else if(resultlist.pResult[i].result == 2)//��
		{
			printf("light%dstate:off\n",y+1);
			cvRectangle(pImg,ptStart,ptStop,cvScalar(0,255,0),4);
		}
		else
		{
			printf("error\n");
		}
		y+=1;
	}
	cvSaveImage("../../result/result.jpg",pImg);
	cvShowImage("Result",pImg);
	//cvWaitKey();

EXT:
	cvReleaseImage(&pImg);
	cvReleaseImage(&pOrgImg2);
	free(offset);
	HYL_MatchUninit(MHandle);
	HYL_MemMgrDestroy(hMemMgr);
	if(pMem!=NULL){
		free(pMem);
		pMem=NULL;
	}
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
