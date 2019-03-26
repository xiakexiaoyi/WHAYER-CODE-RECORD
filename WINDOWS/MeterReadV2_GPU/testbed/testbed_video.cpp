// testBed.cpp : main project file.
#include <opencv2/opencv.hpp>
//#include"amcomdef.h"
//#include"time.h"
#include <conio.h>
#include <windows.h>
#include <math.h>

// meter Recognise
#include"HYAMR_meterReg.h"

// rig match
#include "wyMatchRigidBodyIF.h"
#include "wyMatchRigidBodyStruct.h"

//#ifdef _DEBUG
//#pragma comment(lib, "../Debug/HYAlgorithm.lib")
//#else
//#pragma comment(lib, "../Release/HYAlgorithm.lib")
//#endif
//#pragma comment(lib, "opencv_imgproc244d.lib")
//#pragma comment(lib, "opencv_core244d.lib")
//#pragma comment(lib, "opencv_highgui244d.lib")

#define DEBUG_DEMO

#ifdef DEBUG_DEMO
#define WY_DEBUG_PRINT printf
#else
#define WY_DEBUG_PRINT
#endif

#define GO(FUNCTION)		{if((res = FUNCTION) != 0) goto EXT;}
#define WORK_BUFFER 100000000
#define MR_MEM_SIZE (100*1024*1024)

#define MAX_BUFFER_LEN 5

// save descriptor info
#define MR_READ_FILE_PARA "C:\\MeterRead.dat"

#define SOURCE_ROAD	"C:\\testVideo\\ywb1.avi"
#define METER_TYPE  HY_OIL_METER

int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEnd = 0;

MVoid onMouse(int Event,int x,int y,int flags,void* param );

unsigned char *gTrainTarget = MNull;
unsigned char *gCurTarget = MNull;
unsigned char *gMeterDes = MNull;
MLong gDesSize;

int MeterReadTrain(HYAMR_PATTERN_OUT *outPattern);	//表盘训练
int MeterReadRecog(HYAMR_PATTERN_OUT inPara);	//表盘识别
int RidMatchTrain(int trainType, int rotateEn);	//开关状态训练
int RidMatchRecog();	//开关状态识别
void printMain();
void printMeter();
void printRid();

int main(int argc, char* argv[])
{
	HYAMR_PATTERN_OUT pattern = {0};

	MeterReadTrain(&pattern);
//	Sleep(2000);
	WY_DEBUG_PRINT("########  start recognise\n\n");
	MeterReadRecog(pattern);
	
	system("pause");

	return 0;
}

int MeterReadTrain(HYAMR_PATTERN_OUT *outPattern)
{
	int res = 0;

	MHandle hMemMgr = MNull;
	MHandle hHYMRDHand = MNull;
	MVoid *pMem = MNull;

	CvCapture *pCapture = MNull;
	IplImage *pFrame = MNull;
	IplImage *testImage = MNull;
	IplImage *tmpImage = MNull;
	IplImage *maskImage = MNull;
	CvSize ImgSize;

	HYAMR_IMAGES wy_testImage = {0};
	HYAMR_IMAGES wy_maskImage = {0};

	int i, j, k;
	int lPtNumTmp;
	MPOINT ptListTmp[50];
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	int flag = 0;

	MLong lMemSize;

	MLong lMeterType;

	unsigned char *pData = MNull;

	pMem = malloc(MR_MEM_SIZE);
	if (!pMem)
	{
		WY_DEBUG_PRINT("malloc 100*1024*1024 error!!!\n");
		return -1;
	}

	hMemMgr = HYAMR_MemMgrCreate(pMem, MR_MEM_SIZE);
	if (!hMemMgr)
	{
		WY_DEBUG_PRINT("HYAMR_MemMgrCreate error!!!\n");
		free(pMem);
		pMem = MNull;
		return -1;
	}

	HYAMR_Init (hMemMgr, &hHYMRDHand);
	if (!hHYMRDHand)
	{
		WY_DEBUG_PRINT("HYAMR_Init error!!!\n");
		HYAMR_MemMgrDestroy (hMemMgr);
		free(pMem);
		pMem = MNull;

		return -1;
	}

	lMeterType = METER_TYPE;
	pCapture = cvCaptureFromFile(SOURCE_ROAD);
	if (MNull == pCapture)
	{
		WY_DEBUG_PRINT("get video file error!\n");
		goto EXT;
	}

	pFrame = cvQueryFrame(pCapture);
	ImgSize.height = pFrame->height;
	ImgSize.width = pFrame->width;
	testImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(pFrame, testImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", testImage);
	cvNamedWindow("TrainImage2", 0);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	tmpImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(testImage, tmpImage, CV_INTER_LINEAR);
	WY_DEBUG_PRINT("请圈定目标对象：\n");
	while (1)
	{
		if (mouseParam[2]==CV_EVENT_LBUTTONDOWN)
		{
			startPt.x=mouseParam[0];
			startPt.y=mouseParam[1];
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
			
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			flag=1;
			cvShowImage("TrainImage2",tmpImage);
		}
		if (mouseParam[2]==CV_EVENT_MOUSEMOVE && flag==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
		
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage2",tmpImage);
		}
		if (1 == flagEnd)
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
		k = cvWaitKey(10);
	}
	WY_DEBUG_PRINT("start(%d,%d), end(%d,%d)\n", startPt.x, startPt.y, endPt.x, endPt.y);

	maskImage = cvCreateImage(ImgSize, 8, 1);
	pData = (unsigned char *)(maskImage->imageData);
	for (j=0;j<ImgSize.height;j++)
	{
		for (i=0;i<ImgSize.width;i++)
		{
			if (j>=startPt.y&&j<=endPt.y&&i>=startPt.x&&i<=endPt.x)
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
	wy_testImage.lPixelArrayFormat = HYAMR_IMAGE_BGR;
	//mask
	wy_maskImage.lWidth = maskImage->width;
	wy_maskImage.lHeight = maskImage->height;
	wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	wy_maskImage.lPixelArrayFormat = HYAMR_IMAGE_GRAY;

	WY_DEBUG_PRINT("~~~~~~~~~~Train meter~~~~~~~~~~~~\n");

	if (0 != HYAMR_TrainTemplateFromMask (hHYMRDHand, &wy_testImage, &wy_maskImage, "OK", 0))
	{
		WY_DEBUG_PRINT("HYAMR_TrainTemplateFromMask error!\n");
		res = -1;
		goto EXT;
	}

	lMemSize = testImage->width * testImage->height;
	if (MNull != gMeterDes)
	{
		memset(gMeterDes, 0, lMemSize * sizeof(unsigned char));
	}
	else
	{
		gMeterDes = (unsigned char *)malloc(lMemSize * sizeof(unsigned char));
	}
	//if (0 != HYAMR_SaveTDescriptorsGroup(hHYMRDHand, MR_READ_FILE_PARA))
	if(0 != HYAMR_SaveDesMem (hHYMRDHand, gMeterDes, lMemSize, &gDesSize))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTDescriptorsGroup error!\n");
		res = -1;
		goto EXT;
	}

	HYAMR_GetPoint (hHYMRDHand, &wy_testImage, "OK", 0.75, outPattern, lMeterType);	// 如果获取大刻度点失败，人工圈定大刻度点

	//////////////////////////////////////////////////////////////////////
	lPtNumTmp = 0;
	for (i=0; i<outPattern->lptNum; i++)
	{
		if (outPattern->ptList[i].x>0 && outPattern->ptList[i].x<wy_testImage.lWidth
			&& outPattern->ptList[i].y>0 && outPattern->ptList[i].y<wy_testImage.lHeight)
		{
			ptListTmp[lPtNumTmp++] = outPattern->ptList[i]; 
			cvCircle(testImage, cvPoint(outPattern->ptList[i].x, outPattern->ptList[i].y), 2, CV_RGB(255,0,0), -1, 8, 0);
		}
	}
	WY_DEBUG_PRINT("GetPoint---lPtNum=%d\n", lPtNumTmp);
//	cvNamedWindow("testImage",0);
	cvShowImage("TrainImage2", testImage);
	cvWaitKey(1);
	WY_DEBUG_PRINT("大刻度点修正: 右键添加，左键删除\n");
	mouseParam[0]=-1;
	mouseParam[1]=-1;
	mouseParam[2]=-1;
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);

	while(1)
	{
		if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
		{
			if (mouseParam[0]>=0 && mouseParam[0]<wy_testImage.lWidth
				&& mouseParam[1]>=0 && mouseParam[1]<wy_testImage.lHeight)
			{
				ptListTmp[lPtNumTmp].x = mouseParam[0];
				ptListTmp[lPtNumTmp].y = mouseParam[1];
				lPtNumTmp++;
				cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(255,0,0),-1,8,0);
			}
			WY_DEBUG_PRINT("lPtNum=%d\n", lPtNumTmp);
			mouseParam[2] = -1;
		}
		else if (CV_EVENT_LBUTTONDOWN == mouseParam[2])		// delete white circle
		{
			for(i=0; i<lPtNumTmp; i++)
			{
				if (abs(mouseParam[0]-ptListTmp[i].x)<5 && abs(mouseParam[1]-ptListTmp[i].y)<5)
				{
					cvCircle(testImage, cvPoint(ptListTmp[i].x, ptListTmp[i].y), 1, CV_RGB(255,255,255), -1, 8, 0);
					for(j=i; j<lPtNumTmp-1; j++)
						ptListTmp[j] = ptListTmp[j+1];
					lPtNumTmp--;
					break;
				}
			}
			mouseParam[2] = -1;
		}
		cvShowImage("TrainImage2",testImage);
		if(13==cvWaitKey(10))	// Enter
			break;
	}
	WY_DEBUG_PRINT("later---lPtNum=%d\n", lPtNumTmp);

	WY_DEBUG_PRINT("右键确认起始刻度点\n");
	while(1)
	{
		if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
		{
			if (mouseParam[0]>=0 && mouseParam[0]<wy_testImage.lWidth
				&& mouseParam[1]>=0 && mouseParam[1]<wy_testImage.lHeight)
			{
				ptListTmp[lPtNumTmp].x = mouseParam[0];
				ptListTmp[lPtNumTmp].y = mouseParam[1];
				lPtNumTmp++;
				cvCircle(testImage,cvPoint(mouseParam[0],mouseParam[1]),1,CV_RGB(0,255,0),-1,8,0);
			}
			WY_DEBUG_PRINT("lPtNum=%d\n", lPtNumTmp);
			mouseParam[2] = -1;
		}
		cvShowImage("TrainImage2",testImage);
		if(13==cvWaitKey(10))	// Enter
			break;
	}
	WY_DEBUG_PRINT("later2---lPtNum=%d\n", lPtNumTmp);		
	
	for (i=0; i<lPtNumTmp; i++)
	{
		outPattern->ptList[i] = ptListTmp[i];
	}
	outPattern->lptNum = lPtNumTmp;
	printf("please input %d values>>>\n", lPtNumTmp-1);		// lPtNumTmp-1   lPtNumTmp
	for (i=0; i<lPtNumTmp-1; i++)		// lPtNumTmp-1   lPtNumTmp
	{
		switch(lMeterType)
		{
			case HY_OIL_METER:
				outPattern->dValList[i] = (MDouble)(i * 10);
				break;
			case HY_OIL_TEMPERATURE:
				outPattern->dValList[i] = (MDouble)(i * 20 - 20);
				break;
			case HY_VOLT_METER:
				outPattern->dValList[i] = (MDouble)(i * 50);
				break;
			case HY_SF6_GAS_METER:
				outPattern->dValList[i] = (MDouble)(i * 10 - 10);
				break;
			case HY_DISCHARGE_METER:
				outPattern->dValList[i] = (MDouble)(i);
				break;
			case HY_SWITCH_METER:
				outPattern->dValList[i] = (MDouble)(i+1);
				break;
			case HY_QUALITROL_METER:
				outPattern->dValList[i] = (MDouble)(i * 10);
				break;
			case HY_LEAKAGE_CURRENT_METER:
				outPattern->dValList[i] = (MDouble)(i * 0.5);
				break;
			default:
				outPattern->dValList[i] = (MDouble)(i * 10);
				break;
		}
	}
	printf("get val sucess!\n");

	if(MNull!=gCurTarget)
		memset(gCurTarget, 0, 32 * sizeof(unsigned char));
	else
		gCurTarget = (unsigned char *)malloc(32 * sizeof(unsigned char));

	if(MNull!=gTrainTarget)
		memset(gTrainTarget, 0, 32 * sizeof(unsigned char));
	else
		gTrainTarget = (unsigned char *)malloc(32 * sizeof(unsigned char));

	if(0 != HYAMR_SaveTargetInfoToMem (hHYMRDHand, gCurTarget, gTrainTarget, 1))
	//if (0 != HYAMR_SaveTargetInfo(hHYMRDHand, MR_READ_FILE_TARGET, MR_TRAIN_TARGET, 1))
	{
		WY_DEBUG_PRINT("HYAMR_SaveTargetInfo error!\n");
		res = -1;
		goto EXT;
	}
	WY_DEBUG_PRINT("~~~~~~~~train success~~~~~~~~~~\n");

EXT:
	HYAMR_Uninit (hHYMRDHand);
	HYAMR_MemMgrDestroy (hMemMgr);
	if(MNull!=pMem)
	{	
		free(pMem);
		pMem = MNull;
	}
	cvReleaseImage(&testImage);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&maskImage);

	return res;
}

int MeterReadRecog(HYAMR_PATTERN_OUT inPara)
{
	int res = 0;

	MHandle hMemMgr = MNull;
	MHandle hHYMRDHand = MNull;
	MVoid *pMem = MNull;

	CvCapture *pCapture = MNull;
	IplImage *pFrame = MNull;
	IplImage *testImage;
	HYAMR_IMAGES wy_testImage = {0};
	CvSize ImgSize;

	CvVideoWriter *pWriter = MNull;

	HYAMR_PATTERN_IN tmpPattern = {0};
	HYAMR_READ_PARA pReadPara[2] = {0};
	HYAMR_READ_PARA  readPara1, readPara2;
	MDouble matchRate;
	int i;
//	MLong tmpVal;
	MDouble result1, result2;
	MDouble dResult;
	MLong lMeterType;

	MDouble blackBuffer[MAX_BUFFER_LEN], redBuffer[MAX_BUFFER_LEN];
	MLong lBufferLen;

	MLong lFrameNum = 0;
	char textContent1[100], textContent2[100];
	CvFont font = {0};
	FILE *blackFp = MNull;
	FILE *redFp = MNull;
	MLong isFail = 1;

	MLong lStatus = 0;

//***********************  alloc memory  *******************************
	pMem = malloc(MR_MEM_SIZE);
	if (!pMem)
	{
		WY_DEBUG_PRINT("malloc 100*1024*1024 error!!!\n");
		return -1;
	}

	hMemMgr = HYAMR_MemMgrCreate (pMem, MR_MEM_SIZE);
	if (!hMemMgr)
	{
		WY_DEBUG_PRINT("HYAMR_MemMgrCreate error!!!\n");
		free(pMem);
		pMem = MNull;
		return -1;
	}

	HYAMR_Init (hMemMgr, &hHYMRDHand);
	if (!hHYMRDHand)
	{
		WY_DEBUG_PRINT("HYAMR_Init error!!!\n");
		HYAMR_MemMgrDestroy (hMemMgr);
		free(pMem);
		pMem = MNull;

		return -1;
	}

//***************************  get video  ******************************************************
	//pCapture = cvCaptureFromFile("C:\\test.avi");
	lMeterType = METER_TYPE;
	pCapture = cvCaptureFromFile(SOURCE_ROAD);
	if (MNull == pCapture)
	{
		WY_DEBUG_PRINT("get video file error!\n");
		goto EXT;
	}
	
	pFrame = cvQueryFrame(pCapture);
	ImgSize.height = pFrame->height;
	ImgSize.width = pFrame->width;
	testImage = cvCreateImage(ImgSize, pFrame->depth, pFrame->nChannels);
	cvResize(pFrame, testImage, CV_INTER_LINEAR);

	pWriter = cvCreateVideoWriter("D:\\result_test2.avi", CV_FOURCC('X', 'V', 'I', 'D'), 5, ImgSize);
	if (MNull == pWriter)
	{
		WY_DEBUG_PRINT("pWriter error!\n");
		goto EXT;
	}

	WY_DEBUG_PRINT("~~~~~~~~~~Read meter~~~~~~~~~~~~\n");
	//if (0 != HYAMR_GetTemplateFromText(hHYMRDHand, MR_READ_FILE_PARA))
	if(0 != HYAMR_GetDesMem (hHYMRDHand, gMeterDes))
	{
		WY_DEBUG_PRINT("HYAMR_GetTemplateFromText error !\n");
		res = -1;
		goto EXT;
	}

	if(0 != HYAMR_GetTaregetInfoFromMem (hHYMRDHand, gCurTarget, gTrainTarget, testImage->width, testImage->height))
	{
		WY_DEBUG_PRINT("HYAMR_GetTargetInfo error!\n");
		res = -1;
		goto EXT;
	}

	if (inPara.lptNum<=0 || inPara.lptNum>=120)
	{
		WY_DEBUG_PRINT("inPara.lptNum error !\n");
		return -1;
	}
	tmpPattern.lptNum = inPara.lptNum;
	for (i=0; i<inPara.lptNum; i++)
	{
		if (inPara.ptList[i].x<0 || inPara.ptList[i].x>testImage->width
			|| inPara.ptList[i].y<0 || inPara.ptList[i].y>testImage->height)
		{
			WY_DEBUG_PRINT("inPara.ptList error!\n");
			res = -1;
			goto EXT;
		}
		tmpPattern.ptList[i] = inPara.ptList[i];
	}

	if (0 != HYAMR_SetParam (hHYMRDHand, &tmpPattern, lMeterType))
	{
		WY_DEBUG_PRINT("HYAMR_SetParam error!\n");
		res = -1;
		goto EXT;
	}

	font = cvFont(2, 1);
	cvNamedWindow("Result2", 0);
	lFrameNum = 0;

	matchRate = 0.45;

	while(pFrame=cvQueryFrame(pCapture))
	{
		lFrameNum++;
		cvResize(pFrame, testImage, CV_INTER_LINEAR);
		WY_DEBUG_PRINT("lFrameNum=%d\n", lFrameNum);
		//sprintf(textContent1, "D:\\sf6 images\\testImage_%d.bmp", lFrameNum);
		//cvSaveImage(textContent1, testImage);

		wy_testImage.lWidth = testImage->width;
		wy_testImage.lHeight = testImage->height;
		wy_testImage.pixelArray.chunky.lLineBytes = testImage->widthStep;
		wy_testImage.pixelArray.chunky.pPixel = testImage->imageData;
		wy_testImage.lPixelArrayFormat = HYAMR_IMAGE_BGR;

		// Recognise the pointer line
		//if (0 != HYAMR_GetLineInfo(hHYMRDHand, &wy_testImage, "OK", matchRate, pReadPara, lMeterType))
		if (0 != HYAMR_GetLineInfo_mem (hHYMRDHand, &wy_testImage, "OK", matchRate, pReadPara, lMeterType, gCurTarget, gTrainTarget))
		{
			WY_DEBUG_PRINT("HYAMR_GetLineInfo error !\n");
			isFail = 1;
			//			res = -1;
			//			goto EXT;
		}
		else
		{
			readPara1 = pReadPara[0];
			readPara2 = pReadPara[1];
			result1 = result2 = -1;
			readPara1.lPtNum = readPara2.lPtNum= inPara.lptNum - 1;
			isFail = 0;
		}
		//		printf("please input %d values:\n", inPara.lptNum);
		for (i=0; i<inPara.lptNum - 1; i++)
		{
			//tmpVal = 10 * i;
			//tmpVal = 50 * i;
			readPara1.ptInfo[i].ptVal = readPara2.ptInfo[i].ptVal = inPara.dValList[i];
		}
		if(lFrameNum<MAX_BUFFER_LEN)
			lBufferLen = lFrameNum;
		else
			lBufferLen = MAX_BUFFER_LEN;
		if (HY_OIL_TEMPERATURE==lMeterType || HY_OIL_METER==lMeterType || HY_VOLT_METER==lMeterType 
			|| HY_SF6_GAS_METER==lMeterType || HY_QUALITROL_METER==lMeterType || HY_LEAKAGE_CURRENT_METER==lMeterType)
		{
			if (0 != HYAMR_ReadNumber (hHYMRDHand, readPara1, &result1, lMeterType))
			{
				WY_DEBUG_PRINT("HYAMR_ReadNumber black line error!\n");
				res = -1;
				goto EXT;
			}

			//blackBuffer[(lFrameNum-1) % MAX_BUFFER_LEN] = (MLong)(result1 + 0.5);
			blackBuffer[(lFrameNum-1) % MAX_BUFFER_LEN] = result1;
			dResult = HYAMR_FindMidian (blackBuffer, lBufferLen);
			WY_DEBUG_PRINT("black[%.4f  %.4f  (%d,%d)]\n ", result1, dResult, readPara1.ptEnd.x, readPara1.ptEnd.y);

			sprintf(textContent1, "lFramNum[%d] black[%.4f] end(%d,%d)", lFrameNum, dResult, readPara1.ptEnd.x, readPara1.ptEnd.y);
			if(1==isFail)
				cvPutText(testImage, textContent1, cvPoint(450, 150), &font, CV_RGB(255, 0, 0));
			else
				cvPutText(testImage, textContent1, cvPoint(450, 150), &font, CV_RGB(0, 255, 0));

			blackFp = fopen("D:\\tempBlack2.dat", "ab+");
			fprintf(blackFp, "[black]  %5d  %.4f, [line] %.4f %.4f  (%3d,%3d)\n", lFrameNum, dResult, 
				readPara1.dCoeffK, readPara1.dCoeffB, readPara1.ptEnd.x, readPara1.ptEnd.y);
			fclose(blackFp);
		}

		if (HY_OIL_TEMPERATURE == lMeterType || HY_DISCHARGE_METER == lMeterType)
		{
			if (0 != HYAMR_ReadNumber (hHYMRDHand, readPara2, &result2, lMeterType))
			{
				WY_DEBUG_PRINT("HYAMR_ReadNumber black line error!\n");
				res = -1;
				goto EXT;
			}

			//redBuffer[(lFrameNum-1) % MAX_BUFFER_LEN] = (MLong)(result2 + 0.5);
			redBuffer[(lFrameNum-1) % MAX_BUFFER_LEN] = result2;
			dResult = HYAMR_FindMidian (redBuffer, lBufferLen);
			WY_DEBUG_PRINT("red[%.4f  %.4f  (%d, %d)]\n", result2, dResult, readPara2.ptEnd.x, readPara2.ptEnd.y);

			sprintf(textContent2, "lFramNum[%d] red[%.4f] end(%d,%d)", lFrameNum, dResult, readPara2.ptEnd.x, readPara2.ptEnd.y);
			if(1==isFail)
				cvPutText(testImage, textContent2, cvPoint(450, 200), &font, CV_RGB(255, 0, 0));
			else
				cvPutText(testImage, textContent2, cvPoint(450, 200), &font, CV_RGB(0, 255, 0));

			redFp = fopen("D:\\tempRed2.dat", "ab+");
			fprintf(redFp, "[ red ]  %5d  %.4f, [line] %.4f %.4f (%3d,%3d)\n", lFrameNum, dResult, 
				readPara2.dCoeffK, readPara2.dCoeffB, readPara2.ptEnd.x, readPara2.ptEnd.y);
			fclose(redFp);
		}
		else if (HY_SWITCH_METER == lMeterType)		// 开关“刻度”先标记合 再标记分
		{
			result2 = 1;
			if (0 != HYAMR_ReadNumber (hHYMRDHand, readPara2, &result2, lMeterType))
			{
				WY_DEBUG_PRINT("HYAMR_ReadNumber switch error!\n");
				res = -1;
				goto EXT;
			}

			if(result2 > 0)
			{
				sprintf(textContent2, "lFramNum[%d] status: on", lFrameNum);
				cvPutText(testImage, textContent2, cvPoint(450, 200), &font, CV_RGB(255, 0, 0));
				WY_DEBUG_PRINT("status: ON\n");
			}
			else if(result2<0)
			{
				sprintf(textContent2, "lFramNum[%d] status: off", lFrameNum);
				cvPutText(testImage, textContent2, cvPoint(450, 200), &font, CV_RGB(255, 0, 0));
				WY_DEBUG_PRINT("status: OFF\n");
			}
			else
			{
				sprintf(textContent2, "lFramNum[%d] status: ---", lFrameNum);
				cvPutText(testImage, textContent2, cvPoint(450, 200), &font, CV_RGB(255, 0, 0));
				WY_DEBUG_PRINT("status:---\n");
			}
		}
		
//		WY_DEBUG_PRINT("black=%f, red=%f\n\n\n", result1, result2);
		cvWriteFrame(pWriter, testImage);

		cvShowImage("Result2", testImage);
		cvWaitKey(500);	// delay 2s
	}
	cvReleaseVideoWriter(&pWriter);

EXT:
	HYAMR_Uninit (hHYMRDHand);
	HYAMR_MemMgrDestroy (hMemMgr);
	if(MNull!=pMem)
	{	
		free(pMem);
		pMem = MNull;
	}

	if (MNull != gMeterDes)
	{
		free(gMeterDes);
		gMeterDes = MNull;
	}

	if(MNull!=gCurTarget)
	{
		free(gCurTarget);
		gCurTarget = MNull;
	}
	if(MNull!=gTrainTarget)
	{
		free(gTrainTarget);
		gTrainTarget = MNull;
	}

	cvReleaseImage(&testImage);
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
		flagEnd=1;
	}
}