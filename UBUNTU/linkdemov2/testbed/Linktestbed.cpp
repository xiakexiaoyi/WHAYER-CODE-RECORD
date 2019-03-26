#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "LinkRecog.h"
#include "HYL_MatchRigidBody.h"

#define L_SIZE 100 * 1024 * 1024
#define MR_READ_FILE_PARA "../../MatchRigidBody.dat"
int mouseX = -1, mouseY = -1, mouseFlag = 0;
int flagEndL = 0, flagEndR = 0;
void onMouse(int Event, int x, int y, int flags, void *param);
int LinkTrain(HYLR_RESULT_LIST *resultlist);
int LinkRecog(HYLR_RESULT_LIST resultlist);

char imgname[50] = "002.jpg";
char filename[100] = "../../photo/";
char resultname[100] = "../../result/";

int main()
{

	HYLR_RESULT_LIST resultlist = {0};
	strcat(filename, imgname);
	strcat(resultname, imgname);
	resultlist.lMaxResultNum = 50; //最大目标数目
	resultlist.LRArea = (MLRECT *)malloc(resultlist.lMaxResultNum * sizeof(MLRECT));
	resultlist.pResult = (HYLR_RESULT *)malloc(resultlist.lMaxResultNum * sizeof(HYLR_RESULT));
	LinkTrain(&resultlist); //训练

	LinkRecog(resultlist); //识别

	if (resultlist.LRArea)
		free(resultlist.LRArea);
	if (resultlist.pResult)
		free(resultlist.pResult);
}

int LinkTrain(HYLR_RESULT_LIST *resultlist)
{
	int res = 0;
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	CvSize ImgSize;
	int j, flagL = 0, i = 0;
	unsigned char *pData;
	IplImage *pOrgImg = NULL, *maskImage = NULL;
	IplImage *tmpImage = NULL;
	MHandle hMemMgr = NULL;
	MHandle MHandle = NULL;
	MVoid *pMem = NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	//char *filename="../006.jpg";
	pMem = malloc(L_SIZE); //配准初始化
	if (!pMem)
	{
		printf("malloc error\n");
		return -1;
	}

	hMemMgr = HYL_MemMgrCreate(pMem, L_SIZE);
	if (!hMemMgr)
	{
		printf("HYL_MemMgrCreate error\n");
		free(pMem);
		pMem = NULL;
		return -1;
	}
	HYL_MatchInit(hMemMgr, &MHandle);
	if (!MHandle)
	{
		printf("HYL_MatchInit error!!!\n");
		return -1;
	}
	pOrgImg = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1);
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(pOrgImg, tmpImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", tmpImage);
	cvNamedWindow("TrainImage2", 0);
	cvSetMouseCallback("TrainImage2", onMouse, (void *)mouseParam);
	printf("please select the match area.\n");
	while (1)
	{
		//左键进行标记
		if (mouseParam[2] == CV_EVENT_LBUTTONDOWN)
		{
			startPt.x = mouseParam[0];
			startPt.y = mouseParam[1];
			endPt.x = mouseParam[0];
			endPt.y = mouseParam[1];

			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp", CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0, 0, 255), 1);
			flagL = 1;
			cvShowImage("TrainImage2", tmpImage);
		}
		if (mouseParam[2] == CV_EVENT_MOUSEMOVE && flagL == 1)
		{
			endPt.x = mouseParam[0];
			endPt.y = mouseParam[1];

			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp", CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0, 0, 255), 1);
			cvShowImage("TrainImage2", tmpImage);
		}

		if (1 == flagEndL && i == 0)
		{
			resultlist->MatchArea.top = startPt.y;
			resultlist->MatchArea.bottom = endPt.y;
			resultlist->MatchArea.left = startPt.x;
			resultlist->MatchArea.right = endPt.x;
			printf("Matching complete, please select detection target(note: each and every links should be selected). Press enter to finish.\n");
			i++;

			flagEndL = 0;
			flagL = 0;
			//break;
		}
		if (1 == flagEndL && i != 0)
		{
			resultlist->LRArea[resultlist->lAreaNum].left = startPt.x - resultlist->MatchArea.left;
			resultlist->LRArea[resultlist->lAreaNum].top = startPt.y - resultlist->MatchArea.top;
			resultlist->LRArea[resultlist->lAreaNum].right = endPt.x - resultlist->MatchArea.left;
			resultlist->LRArea[resultlist->lAreaNum].bottom = endPt.y - resultlist->MatchArea.top;
			resultlist->lAreaNum++;
			resultlist->lRealResultNum++;

			flagEndL = 0;
			flagL = 0;
		}

		if (-1 != cvWaitKey(10)) //enter 退出
		{
			break;
		}
		cvShowImage("TrainImage2", tmpImage);
	}
	printf("Detection complete\n");
	cvDestroyWindow("TrainImage2");
	cvSetImageROI(pOrgImg, cvRect(resultlist->MatchArea.left, resultlist->MatchArea.top, resultlist->MatchArea.right - resultlist->MatchArea.left, resultlist->MatchArea.bottom - resultlist->MatchArea.top));
	cvSaveImage("../cut.jpg", pOrgImg);
	cvResetImageROI(pOrgImg);
	resultlist->origin.x = (resultlist->MatchArea.left + resultlist->MatchArea.right) / 2;
	resultlist->origin.y = (resultlist->MatchArea.top + resultlist->MatchArea.bottom) / 2;

	maskImage = cvCreateImage(ImgSize, 8, 1); //mask image
	pData = (unsigned char *)(maskImage->imageData);
	for (j = 0; j < ImgSize.height; j++)
	{
		for (i = 0; i < ImgSize.width; i++)
		{
			if (j >= resultlist->MatchArea.top && j <= resultlist->MatchArea.bottom && i >= resultlist->MatchArea.left && i <= resultlist->MatchArea.right)
			{
				pData[j * maskImage->widthStep + i] = 255;
			}
			else
			{
				pData[j * maskImage->widthStep + i] = 0;
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
	if (0 != HYL_TrainTemplateFromMask(MHandle, &wy_testImage, &wy_maskImage, "OK", 0)) //训练图片，得到模板
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
	if (pMem != NULL)
	{
		free(pMem);
		pMem = NULL;
	}
	return res;
}
int LinkRecog(HYLR_RESULT_LIST resultlist)
{
	void *hTLHandle = NULL;
	float thresh = 0.1;
	int w = 0, h = 0;
	LR_IMAGES imgs = {0};
	MHandle hMemMgr = NULL;
	MHandle MHandle = NULL;
	MVoid *pMem = NULL;
	HYL_IMAGES src = {0};
	//柱状压板
	/**/
	char *cfgfile = "../../model/Link/yaban1/tiny-yolo-voc.cfg";
	char *weightfile = "../../model/Link/yaban1/tiny-yolo-voc_38000.weights"; // 模型
																			  /**/
																			  //I型压板
																			  /**
	char *cfgfile="../model/Link/yaban2/tiny-yolo-voc.cfg";
	char *weightfile="../model/Link/yaban2/tiny-yolo-voc_final.weights";
	/**/
																			  //char *filename="../006.jpg";
	MPOINT *centre;
	//HYLR_RESULT_LIST Tmpresultlist = {0};
	MPOINT offset;
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	IplImage *pImg = 0;
	pImg = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	pMem = malloc(L_SIZE); //配准初始化
	if (!pMem)
	{
		printf("malloc error\n");
		return -1;
	}

	hMemMgr = HYL_MemMgrCreate(pMem, L_SIZE);
	if (!hMemMgr)
	{
		printf("HYL_MemMgrCreate error\n");
		free(pMem);
		pMem = NULL;
		return -1;
	}
	HYL_MatchInit(hMemMgr, &MHandle);
	if (!MHandle)
	{
		printf("HYL_MatchInit error!!!\n");
		return -1;
	}
	w = resultlist.MatchArea.right - resultlist.MatchArea.left;
	h = resultlist.MatchArea.bottom - resultlist.MatchArea.top;
	HYLR_Init(NULL, &hTLHandle);
	HYLR_SetParam(hTLHandle, cfgfile, weightfile, thresh, w, h);

	//resultlist.lMaxResultNum=3;
	// Tmpresultlist.pResult = (HYLR_RESULT *)malloc(1*sizeof(HYLR_RESULT));
	centre = (MPOINT *)malloc(1 * sizeof(MPOINT));
	if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_GetTemplateFromText error !\n");
	}
	src.lHeight = pImg->height;
	src.lWidth = pImg->width;
	src.lPixelArrayFormat = HYL_IMAGE_BGR;
	src.pixelArray.chunky.pPixel = pImg->imageData;
	src.pixelArray.chunky.lLineBytes = pImg->widthStep;
	HYL_GetDashboard(MHandle, &src, "OK", 0.45, centre); //匹配模板,求图像偏移量
	resultlist.offset.x = centre->x - resultlist.origin.x;
	resultlist.offset.y = centre->y - resultlist.origin.y;

	MLRECT rtArea;
	//for(int i=0;i<resultlist.lAreaNum;i++)//实际没有循环
	{
		IplImage *RectImg = NULL;
		rtArea.bottom = resultlist.MatchArea.bottom + resultlist.offset.y;
		rtArea.left = resultlist.MatchArea.left + resultlist.offset.x;
		rtArea.top = resultlist.MatchArea.top + resultlist.offset.y;
		rtArea.right = resultlist.MatchArea.right + resultlist.offset.x;
		if (rtArea.bottom >= pImg->height)
			rtArea.bottom = pImg->height - 1;
		if (rtArea.right >= pImg->width)
			rtArea.right = pImg->width - 1;
		if (rtArea.left < 0)
			rtArea.left = 0;
		if (rtArea.top < 0)
			rtArea.top = 0;
		RectImg = cvCreateImage(cvSize(rtArea.right - rtArea.left, rtArea.bottom - rtArea.top), pImg->depth, pImg->nChannels);
		cvSetImageROI(pImg, cvRect(rtArea.left, rtArea.top, rtArea.right - rtArea.left, rtArea.bottom - rtArea.top));
		cvCopy(pImg, RectImg);
		cvResetImageROI(pImg);
		//cvSaveImage("D:\\RectImg.bmp", RectImg);

		/*if (!RectImg)
		{
			printf("Error when cut image.\n");   
			j=j+1;
			continue;
		}*/
		imgs.lHeight = RectImg->height;
		imgs.lWidth = RectImg->width;
		imgs.pixelArray.chunky.lLineBytes = RectImg->widthStep;
		imgs.pixelArray.chunky.pPixel = RectImg->imageData;
		//Tmpresultlist.lResultNum=0;
		HYLR_LinkRecog(hTLHandle, &imgs, &resultlist);
		for (int j = 0; j < resultlist.lResultNum; j++)
		{
			resultlist.pResult[j].Target.left = resultlist.pResult[j].Target.left + rtArea.left;
			resultlist.pResult[j].Target.right = resultlist.pResult[j].Target.right + rtArea.left;
			resultlist.pResult[j].Target.top = resultlist.pResult[j].Target.top + rtArea.top;
			resultlist.pResult[j].Target.bottom = resultlist.pResult[j].Target.bottom + rtArea.top;
		}

		cvReleaseImage(&RectImg);
	}

	////cvDestroyWindow("Result Show");
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		CvPoint ptStart, ptStop, ptText;
		char text[256] = {0};
		CvFont font;
		ptStart.x = resultlist.pResult[i].Target.left;
		ptStart.y = resultlist.pResult[i].Target.top;
		ptStop.x = resultlist.pResult[i].Target.right;
		ptStop.y = resultlist.pResult[i].Target.bottom;

		//printf("left=%d,top=%d,right=%d,bottom=%d\n",ptStart.x,ptStart.y,ptStop.x,ptStop.y);
		ptText.x = resultlist.pResult[i].Target.left;
		ptText.y = resultlist.pResult[i].Target.bottom;

		if (ptText.y < pImg->height - 10)
			ptText.y += 10;
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0, 0, 255));
		if (resultlist.pResult[i].flag != 1)
		{
			sprintf(text, "%d:state=none", i + 1);
		}
		else if (resultlist.pResult[i].dVal == 0)
			sprintf(text, "%d:state=fen", i + 1);
		else if (resultlist.pResult[i].dVal == 1)
			sprintf(text, "%d:state=he", i + 1);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text, ptText, &font, cvScalar(0, 255, 0));
	}
	cvSaveImage(resultname, pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);
	/*cvNamedWindow("Result Show");
	cvShowImage("Result Show", pImg);*/
	cvReleaseImage(&pImg);
	//free(Tmpresultlist.pResult);
	free(centre);
	HYL_MatchUninit(MHandle);
	HYL_MemMgrDestroy(hMemMgr);
	if (pMem != NULL)
	{
		free(pMem);
		pMem = NULL;
	}
	HYLR_Uninit(hTLHandle);

	return 0;
}

void onMouse(int Event, int x, int y, int flags, void *param)
{
	if (Event == CV_EVENT_LBUTTONDOWN)
	{
		int *Data = (int *)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = 1;
	}
	if (Event == CV_EVENT_RBUTTONDOWN)
	{
		int *Data = (int *)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_RBUTTONDOWN;
	}
	if (Event == CV_EVENT_MOUSEMOVE)
	{
		int *Data = (int *)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_MOUSEMOVE;
	}
	if (Event == CV_EVENT_LBUTTONUP)
	{
		int *Data = (int *)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_LBUTTONUP;
		flagEndL = 1;
	}
	if (Event == CV_EVENT_RBUTTONUP)
	{
		int *Data = (int *)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_RBUTTONUP;
		flagEndR = 1;
	}
}
