#include <stdio.h>
#include <opencv2/opencv.hpp>
#include<string.h>
#include "DigitalRecog.h"
#include "HYL_MatchRigidBody.h"

#define L_SIZE 100*1024*1024
#define MR_READ_FILE_PARA "../../MatchRigidBody.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;
void onMouse(int Event,int x,int y,int flags,void* param );
int DigitalTrain(HYDR_RESULT_LIST *resultlist);
int DigitalRecog(HYDR_RESULT_LIST resultlist);


char imgname[50]="002.jpg"; 
char filename[100]="../../photo/";
char resultname[100]="../../result/";


int main()
{

	
	HYDR_RESULT_LIST resultlist = {0};
	strcat(filename,imgname); 
	strcat(resultname,imgname); 
    resultlist.lResultNum=20;
	resultlist.DRArea = (MDRECT*)malloc(resultlist.lResultNum*sizeof(MDRECT));
	resultlist.pResult = (HYDR_RESULT*)malloc(resultlist.lResultNum*sizeof(HYDR_RESULT)) ;
	DigitalTrain(&resultlist);//训练
	
	
	DigitalRecog(resultlist);//识别
	
		

	

	if (resultlist.DRArea)
		free(resultlist.DRArea);
	if (resultlist.pResult)
		free(resultlist.pResult);
}

int DigitalTrain(HYDR_RESULT_LIST *resultlist)
{
	int res=0;
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	CvSize ImgSize; 
	int j,flagL = 0,i=0;
	unsigned char *pData;
	IplImage *pOrgImg=NULL,*maskImage= NULL;
	IplImage *tmpImage = NULL;
	MHandle hMemMgr=NULL;
	MHandle MHandle=NULL;
	MVoid *pMem=NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	//char *filename="../001.jpg";
	pMem=malloc(L_SIZE);  //配准初始化
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
	pOrgImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(pOrgImg, tmpImage, CV_INTER_LINEAR);
	cvSaveImage("../testImage.bmp", tmpImage);
	cvNamedWindow("TrainImage2", 1);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	printf("Please select the match area.\n");
	while (1)
	{
		//左键进行标记 
		if (mouseParam[2]==CV_EVENT_LBUTTONDOWN)
		{
			startPt.x=mouseParam[0];
			startPt.y=mouseParam[1];
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
			
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("../testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			flagL=1;
			cvShowImage("TrainImage2",tmpImage);	
			
			resultlist->DRArea[i].left = startPt.x;
			resultlist->DRArea[i].top = startPt.y; 		
		}
		if (mouseParam[2]==CV_EVENT_MOUSEMOVE && flagL==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
		
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("../testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage2",tmpImage);	
			
			resultlist->DRArea[i].right = endPt.x;
			resultlist->DRArea[i].bottom = endPt.y ; 
		}
		
		if(1 == flagEndL && i == 0 )
		{
			printf("Matching complete,Please select the number area (include all numbers). Press enter to finish\n");
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
		if (-1 != cvWaitKey(10))  //enter 退出
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
	}
	printf("Detection complete\n");
	cvDestroyWindow("TrainImage2");
	cvSetImageROI(pOrgImg,cvRect(resultlist->DRArea[1].left,resultlist->DRArea[1].top,resultlist->DRArea[1].right-resultlist->DRArea[1].left,resultlist->DRArea[1].bottom-resultlist->DRArea[1].top));
	cvSaveImage("../cut.jpg",pOrgImg);
	cvResetImageROI(pOrgImg);
	resultlist->origin.x=(resultlist->DRArea[0].left+resultlist->DRArea[0].right)/2;
	resultlist->origin.y=(resultlist->DRArea[0].top+resultlist->DRArea[0].bottom)/2;

	maskImage = cvCreateImage(ImgSize, 8, 1);   //mask image
	pData = (unsigned char *)(maskImage->imageData);
	for (j=0;j<ImgSize.height;j++)
	{
		for (i=0;i<ImgSize.width;i++)
		{
			if (j>=resultlist->DRArea[0].top&&j<=resultlist->DRArea[0].bottom&&i>=resultlist->DRArea[0].left&&i<=resultlist->DRArea[0].right)
			{
				pData[j*maskImage->widthStep+i]=255;
			}
			else
			{
				pData[j*maskImage->widthStep+i]=0;
			}

		}
	}
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
	if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//训练图片，得到模板
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
	cvReleaseImage(&maskImage);
	cvReleaseImage(&tmpImage);
	HYL_MatchUninit(MHandle);
	HYL_MemMgrDestroy(hMemMgr);
	if(pMem!=NULL){
		free(pMem);
		pMem=NULL;
	}
	return res;

}
int DigitalRecog(HYDR_RESULT_LIST resultlist)
{
	void *hTLHandle=NULL;
	float thresh=0.2;
	DR_IMAGES imgs = {0};
	MHandle hMemMgr=NULL;
	MHandle MHandle=NULL;
	MVoid *pMem=NULL;
	HYL_IMAGES src = {0};
	char *cfgfile="../../model/tiny-yolo-voc.cfg";
	char *weightfile="../../model/tiny-yolo-voc_final.weights";
	//char *filename="../001.jpg";
    MPOINT *centre;
	HYDR_RESULT_LIST Tmpresultlist = {0};
    MPOINT offset;
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	IplImage* pImg = 0;
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	pMem=malloc(L_SIZE);  //配准初始化
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
	HYDR_Init(NULL,&hTLHandle);

	int w = 0;
	int h = 0;
	if(0!=HYDR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h))
	{
		printf("HYDR_SetParam error.\n");
		HYDR_Uninit(hTLHandle);
		return -1;
	}
	
	
	//resultlist.lMaxResultNum=3;
    Tmpresultlist.pResult = (HYDR_RESULT *)malloc(1*sizeof(HYDR_RESULT));
	centre = (MPOINT*)malloc(1*sizeof(MPOINT));
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_GetTemplateFromText error !\n");
	}
	src.lHeight = pImg->height;
	src.lWidth = pImg->width;
	src.lPixelArrayFormat = HYL_IMAGE_BGR;
	src.pixelArray.chunky.pPixel = pImg->imageData;
	src.pixelArray.chunky.lLineBytes = pImg->widthStep;
	HYL_GetDashboard(MHandle, &src, "OK", 0.45, centre);//匹配模板,求图像偏移量
    resultlist.offset.x=centre->x-resultlist.origin.x;
	resultlist.offset.y=centre->y-resultlist.origin.y;

	
	int j=0;
    MDRECT rtArea;
	for(int i=1;i<resultlist.lAreaNum;i++)
	{
		/*if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
		return false;
		}
		if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
			return false;
		}

		if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
			return false;
		}
		printf("begin:size=%d,resident=%d,share=%d\n",procmem.size,procmem.resident,procmem.share);*/
	    IplImage *RectImg=NULL;

		rtArea.bottom=MAX(resultlist.DRArea[i].bottom,resultlist.DRArea[i].top)+resultlist.offset.y;
		rtArea.left = MIN(resultlist.DRArea[i].left,resultlist.DRArea[i].right)+resultlist.offset.x;
		rtArea.top = MIN(resultlist.DRArea[i].bottom,resultlist.DRArea[i].top)+resultlist.offset.y;
		rtArea.right = MAX(resultlist.DRArea[i].left,resultlist.DRArea[i].right)+resultlist.offset.x;

		//rtArea.bottom = resultlist.DRArea[i].bottom+resultlist.offset.y;
		//rtArea.left = resultlist.DRArea[i].left+resultlist.offset.x;
		//rtArea.top = resultlist.DRArea[i].top+resultlist.offset.y;
		//rtArea.right = resultlist.DRArea[i].right+resultlist.offset.x;
		if(rtArea.bottom >= pImg->height)rtArea.bottom=pImg->height-1;
		if(rtArea.right >= pImg->width )rtArea.right=pImg->width-1;
		if(rtArea.left < 0 )rtArea.left=0;
		if(rtArea.top < 0 )rtArea.top =0;
		RectImg = cvCreateImage(cvSize(rtArea.right-rtArea.left,rtArea.bottom-rtArea.top), pImg->depth, pImg->nChannels);
		cvSetImageROI(pImg,cvRect(rtArea.left,rtArea.top,rtArea.right-rtArea.left,rtArea.bottom-rtArea.top));
		cvCopy(pImg,RectImg);
		cvResetImageROI(pImg);
		//cvSaveImage("D:\\RectImg.bmp", RectImg);

		if (!RectImg||!RectImg->height||!RectImg->width)
		{
			printf("Error when cut image.\n");   
			j=j+1;
			continue;
		}
		imgs.lHeight = RectImg->height;
		imgs.lWidth = RectImg->width;
		imgs.pixelArray.chunky.lLineBytes = RectImg->widthStep;
		imgs.pixelArray.chunky.pPixel = RectImg->imageData;
	    Tmpresultlist.lResultNum=0;
		HYDR_DigitRecog(hTLHandle,&imgs,&Tmpresultlist);
		resultlist.pResult[i-j].dVal=Tmpresultlist.pResult[0].dVal;
		resultlist.pResult[i-j].Target.left=Tmpresultlist.pResult[0].Target.left+rtArea.left;
		resultlist.pResult[i-j].Target.right=Tmpresultlist.pResult[0].Target.right+rtArea.left;
		resultlist.pResult[i-j].Target.top=Tmpresultlist.pResult[0].Target.top+rtArea.top;
		resultlist.pResult[i-j].Target.bottom=Tmpresultlist.pResult[0].Target.bottom+rtArea.top;
	    cvReleaseImage(&RectImg);	
		//i=i-1;
		
		/*if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
			return false;
		}
		if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
			return false;
		}

		if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
			return false;
		}
		printf("end:size=%d,resident=%d,share=%d\n",procmem.size,procmem.resident,procmem.share);*/
	}

	

	////cvDestroyWindow("Result Show");
	for (int i=1; i<resultlist.lAreaNum; i++)
	{
		CvPoint ptStart, ptStop, ptText;
		char text[256]={0};
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
		sprintf(text, "val=%.1f", resultlist.pResult[i].dVal);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	
	}
	cvSaveImage(resultname, pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);
	/*cvNamedWindow("Result Show");
	cvShowImage("Result Show", pImg);*/
	cvReleaseImage(&pImg);
	free(Tmpresultlist.pResult);
	free(centre);
	HYL_MatchUninit(MHandle);
	HYL_MemMgrDestroy(hMemMgr);
	if(pMem!=NULL){
		free(pMem);
		pMem=NULL;
	}
	HYDR_Uninit(hTLHandle);

	return 0;
}

void onMouse(int Event,int x,int y,int flags,void* param )
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
