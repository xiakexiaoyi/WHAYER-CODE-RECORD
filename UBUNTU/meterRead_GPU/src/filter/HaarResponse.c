#include "HaarResponse.h"
#include "rotateBlock.h"
#include "lidebug.h"
#include "liedge.h"
//#include "blobfilter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "bbgeometry.h"
#include "limath.h"
//#include "HaarWidth.h"
#include "litimer.h"
#include "liintegral.h"
#include "sort.h"
#include "lisort.h"
#include "liimage.h"
#include "liimgfmttrans.h"

//extern MLong lframeNum;
#define ROTATE_PI 3.141592653589793238
#define MAXWIDTH 20
#define RANSAC_TIMES 2000
#define RANSAC_TIMES2 2000
#define RANSAC_THRESHOLD 1
#define RANSAC_THRESHOLD2 10
#define RANSAC_THRESHOLD3 10
#define RANSAC_ANGLE_BIAS 25
#define HaarPI 3.1415926
#define ANGLE_MAX_DISTANCE 5
#define P_MAX_DISTANCE 8
#define PIXEL_MAX_DISTANCE 15
#define MAX_GOOD_POINT 50
#define ANGLE_STEP	10
#define SIN10 0.17365


#define MAX_SEED_NUM 200
#define MED_SEED_NUM 140
#define MIN_SEED_NUM 40
#define LEAST_NUM 8

#define DISTANCE_THRESHOLD 5

//static MRESULT HaarResponseAngle(MHandle hMemMgr ,BLOCK *pImage, PHaarParam pParam,BLOCK *pResponseResultImage,	BLOCK *pResponseAngle);
static MVoid GetResponseResult(JIntegral *pIntegral, MLong lIntegralLine,MLong *pDstData, 
	MLong lDstLine,MLong lWidth, MLong lHeight,PHaarParam pParam);
static MVoid GetResponseResult_xsd(JIntegral *pIntegral, MLong lIntegralLine,MLong *pDstData, 
	MLong lDstLine,MLong lWidth, MLong lHeight,PHaarParam pParam);
static MVoid GetResponseResult_xsd2(JIntegral *pIntegral, MLong lIntegralLine,MLong *pDstData, 
	MLong lDstLine,MLong lWidth, MLong lHeight,PHaarParam pParam, MLong *lMaxVal);
//static MRESULT StatisticNonZeroValue(MHandle hMemMgr, MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,MByte lHorizontalVal,\
//									 MVoid* pMem, MLong length, MLong lPercentage, MLong *retVal);
//static MRESULT CreateCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSet, MLong lMaxCircleCandidate,MLong lMaxPtNumPerCircle);
//static MVoid FreeCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSetInfo);
//static MRESULT CreateLineSet(MHandle hMemMgr, LINE_SET_INFO *pLineSet, MLong lMaxLineCandidate, MLong lMaxPtNumPerLine);
static MVoid FreeLineSet(MHandle hMemMgr, LINE_SET_INFO *pLineSet);

MRESULT RANSAC_LINE(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
	MLong lPtNum, MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam);

static MRESULT CalcLineEndPt_xsd(MHandle hMemMgr, BLOCK *pImage,BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold, LINEPARAM *lineParam);
static MRESULT CalcLineEndPtStep2(MHandle hMemMgr,BLOCK *pImage, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold,
	LINEPARAM lineParam, MLong minLength, MLong *leftWidthConfidence, MLong *rightWidthConfidence);
static MLong CmpVal_xsd( const MVoid* p1, const MVoid* p2);

static MVoid OptimizeHaar_xsd(ResponseResult *pSrc, ResponseResult *pDst, BLOCK *xBlock, BLOCK *yBlock,BLOCK *pResponseAngle, MLong *lMaxVal, MByte angleIndex);

MVoid AppendPointList(MByte *pRspData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MByte *pRspAngleData, MLong lAngleDataLine,
	JGSEED *pSeedList, CIRCLE_INFO *pCircleInfo, PARAM_INFO *pParam, MVoid *pTemp, MLong lMemLen);

MRESULT FilterCirclePointList(MHandle hMemMgr,MByte *pRspData, MLong lRspDataLine, MLong lWidth, MLong lHeight,
	CIRCLE_INFO *pCircleInfo, LINE_INFO *pLineInfo, MPOINT ptLineMid,MLong lMaxArcAngle);
static MVoid selectPoint(PTINFO* pSrcPtInfoList, MLong srcPtInfoNum, JGSEED *pSeeds, MLong lRadius);
static MVoid selectPoint2(CIRCLE circleParam, JGSEED *pSeeds, MLong lWidth, MLong lHeight);
MRESULT filterImg(BLOCK *pImage,LINEPARAM *lineParam,MLong lHaarWidth);

MLong lcircleNum = 0;
BLOCK blockFlagTmp = {0};
int   gPictureNum = 0;
char  gPicName[100]={0};

//******************************** line location*************************************************
// 1 haar响应图计算
// 2 根据mask图，掩模计算，过滤掉表盘部分无效（且干扰识别）的信息，如仅保留圆环上响应信息
// 3 如果图像比较模糊（针对泄流电流表指针区分不明显），响应图取反处理
// 4 计算分位点，筛选局部极大值点
// 5 如果局部极大值点为零，重新计算分位点，筛选局部极大值点；如果极大值点过多，提高分位阈值；如果极大值点过少，对其进行扩展
// 6 ransac 搜索最优指针线参数方程
// 7 判断计算指针线端点
MRESULT GetLineInfo(MHandle hMemMgr,BLOCK *pImage, BLOCK *pMask, PARAM_INFO *pParam)
{
	MRESULT res=LI_ERR_NONE;
	JGSEED seedsTemp={0};
	JGSEED seedsTemp1={0};
	MLong i;
	MLong k=0;
	MLong m;
	MDouble distance;
	MPOINT *pTemp=MNull;
	MLong lThreshold, lHighThreshold;
	BLOCK haarResult={0};
	BLOCK haarAngleResult = {0};
	MLong lStackLen;
	HaarParam haarParam={0};
	BLOCK blockFlag={0};
	MLong lSeedSize;
	MLong lHaarWidth=0;

	MPOINT ptCenter = {0};
	MLong lRadius = 0;
	MLong *pSeedsAngle = MNull;

	LINEPARAM *pLineParam = MNull;

	MLong lRate = 20;
	MLong lCount;
	if(pParam->pCircleInfo->circleParam.xcoord == -10000)
	{
		lRate=40;
	}
	else
	{
		lRate=30;
	}


	gPictureNum++;
	if (gPictureNum > 3)
	{
		gPictureNum = 1;
	}

	haarParam.lClassNumber=4;
	haarParam.lAngleNumber=180/ANGLE_STEP;
	haarParam.lHaarWidth = pParam->lHarrWidth;
	haarParam.h = 3 * pParam->lHarrWidth;
	if (PIC_HIGH_BLUR == pParam->lPicBlurLevel)
	{
		haarParam.w = pParam->lHarrWidth;
	}
	else
	{
		haarParam.w = (pParam->lHarrWidth)<<1;
	}
	haarParam.bWhiteBackground = pParam->bWhiteBackground;
	lHaarWidth = pParam->lHarrWidth;

	AllocVectMem(hMemMgr,haarParam.pAngleList,haarParam.lAngleNumber,MLong);
	for (i=0;i<haarParam.lAngleNumber;i++)
	{
		haarParam.pAngleList[i] = i*ANGLE_STEP;//横向的模板,最后要-90度
	}
	GO(B_Create(hMemMgr, &haarAngleResult, DATA_U8, pImage->lWidth, pImage->lHeight));
	GO(B_Create(hMemMgr,&haarResult,DATA_U8,pImage->lWidth, pImage->lHeight));
	//获取响应图
	sprintf(gPicName,"./log/%d_gray_aaa.bmp",gPictureNum);
	PrintBmpEx(pImage->pBlockData, pImage->lBlockLine, DATA_U8, pImage->lWidth, pImage->lHeight, 1, gPicName);
	//	printf("stride=%d, width=%d, height=%d\n", pImage->lBlockLine, pImage->lWidth, pImage->lHeight);
	GO(HaarResponseAngle(hMemMgr,pImage,&haarParam,&haarResult, &haarAngleResult));
	sprintf(gPicName,"./log/%d_response_aaa.bmp",gPictureNum);
	PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, gPicName);

	lStackLen = haarResult.lWidth * haarResult.lHeight /10;
	AllocVectMem(hMemMgr, pTemp, lStackLen, MPOINT);
	GO(B_Create(hMemMgr, &blockFlag, DATA_U8, haarResult.lWidth, haarResult.lHeight));
	SetVectZero(blockFlag.pBlockData, blockFlag.lBlockLine * blockFlag.lHeight);
	lSeedSize = haarResult.lWidth * haarResult.lHeight;
	AllocVectMem(hMemMgr, seedsTemp.pptSeed, lSeedSize, MPOINT);
	AllocVectMem(hMemMgr, seedsTemp.pcrSeed, lSeedSize, MByte);
	//AllocVectMem(hMemMgr, seedsTemp1.pptSeed, lSeedSize, MPOINT);
	//AllocVectMem(hMemMgr, seedsTemp1.pcrSeed, lSeedSize, MByte);

	seedsTemp.lMaxNum = lSeedSize;
	seedsTemp.lSeedNum = 0;
	//seedsTemp1.lMaxNum = lSeedSize;
	//seedsTemp1.lSeedNum = 0;
	//sprintf(gPicName,"./log/%d_pmask.bmp",gPictureNum);
	//PrintBmpEx(pMask->pBlockData, pMask->lBlockLine, DATA_U8, pMask->lWidth, pMask->lHeight, 1, gPicName);
	GO(getMaskResponse(&haarResult, pMask));
	sprintf(gPicName,"./log/%d_response_bbb.bmp",gPictureNum);
	PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, gPicName);

	if (PIC_HIGH_BLUR == pParam->lPicBlurLevel)
	{
		inverseImg(&haarResult);
		sprintf(gPicName,"./log/%d_response_ccc.bmp",gPictureNum);
		PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, gPicName);

	}
	if(pParam->lDoubleRedLine)
	{
		pLineParam = &(pParam->pCircleInfo->lineInfo.lineParam);
		filterImg(&haarResult,pLineParam,lHaarWidth);
		sprintf(gPicName,"./log/%d_response_ddd.bmp",gPictureNum);
		PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, gPicName);
	}
	GO(StatisticNonZeroValue(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine,haarResult.lWidth,
		haarResult.lHeight,0, pTemp, sizeof(MPOINT)*lStackLen, lRate, &lThreshold));
	GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
		blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lThreshold, (MLong)(lThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth>>1));	//0.6

	ridgePoint(hMemMgr,(MByte*)haarResult.pBlockData,haarResult.lBlockLine, haarResult.lWidth, haarResult.lHeight,
		(MByte *)blockFlag.pBlockData, blockFlag.lBlockLine, lHaarWidth>>1,
		(MLong)(lThreshold*0.6), 0, 250, &seedsTemp);

	sprintf(gPicName, "./log/%d_localMaxPre_bbb.bmp",gPictureNum);
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,gPicName);
	JPrintf("localMax---------\n");
	//Logger("local max ok...\n");
	if (0 == seedsTemp.lSeedNum) // 没有筛选获得种子点，提高分位点，重新筛选
	{
		//printf("lThreshold=%d, lHarrWidth=%d\n", lThreshold, lHaarWidth);
		lRate += 10;
		GO(StatisticNonZeroValue(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine,haarResult.lWidth,
			haarResult.lHeight,0, pTemp, sizeof(MPOINT)*lStackLen, lRate, &lThreshold));

		GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
			blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lThreshold, (MLong)(lThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth));	//0.6
		sprintf(gPicName,"./log/%d_localMaxPre_bbb.bmp",gPictureNum);
		PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,gPicName);
	}
	lHighThreshold = lThreshold;
	lCount = 0;

	while(seedsTemp.lSeedNum >= MED_SEED_NUM)  // 种子点过多，提高阈值，重新筛选
	{
		lCount++;
		lHighThreshold = lHighThreshold + 13;
		if(lCount > 5 || lHighThreshold>=250)
			break;
		else if(lCount >= 5)
		{
			GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
				blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lHighThreshold, (MLong)(lHighThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth));
		}
		else
		{
			GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
				blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lHighThreshold, (MLong)(lHighThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth>>1));
		}
		while(seedsTemp.lSeedNum < MIN_SEED_NUM)
		{
			lHighThreshold = lHighThreshold - 7;
			GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
				blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lHighThreshold, (MLong)(lHighThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth>>1));
		}
		PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"./log/localMaxPre_bbb1.bmp");
	}

	if(seedsTemp.lSeedNum < MIN_SEED_NUM)  // 种子点过少，根据现有种子点做扩展
	{
		lHighThreshold = lHighThreshold - 8;
		if (seedsTemp.lSeedNum < LEAST_NUM)
		{
			lCount = 0;
			while(seedsTemp.lSeedNum < LEAST_NUM)
			{
				lCount++;
				if (lCount>=3 && lHaarWidth>=4)	
				{
					appendSeeds((MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte*)blockFlag.pBlockData,
						blockFlag.lBlockLine, (MByte*)haarAngleResult.pBlockData, haarAngleResult.lBlockLine, haarResult.lWidth,
						haarResult.lHeight, (MLong)(lHighThreshold * 0.6), lHaarWidth>>2, &seedsTemp);
					break;
				}
				else if(lCount>=5)
					break;
				appendSeeds((MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte*)blockFlag.pBlockData,
					blockFlag.lBlockLine, (MByte*)haarAngleResult.pBlockData, haarAngleResult.lBlockLine, haarResult.lWidth,
					haarResult.lHeight, (MLong)(lHighThreshold * 0.6), lHaarWidth>>1, &seedsTemp);
			}
		}
		else
		{
			appendSeeds((MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte*)blockFlag.pBlockData,
				blockFlag.lBlockLine, (MByte*)haarAngleResult.pBlockData, haarAngleResult.lBlockLine, haarResult.lWidth,
				haarResult.lHeight, (MLong)(lHighThreshold * 0.4), lHaarWidth>>1, &seedsTemp);
			//lHighThreshold = lHighThreshold - 7;
			/*GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
			blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lHighThreshold, (MLong)(lHighThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth>>2));*/
		}
		//printf("##### pt num=%d\n", seedsTemp.lSeedNum);
	}

	printf("##### pt num=%d\n", seedsTemp.lSeedNum);
	if(MNull!=pParam->pCircleInfo)
		selectPoint2(pParam->pCircleInfo->circleParam, &seedsTemp, haarResult.lWidth, haarResult.lHeight);

	sprintf(gPicName, "./log/%d_localMaxPost.bmp",gPictureNum);
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,gPicName);

	if (seedsTemp.lSeedNum >= MAX_SEED_NUM)
	{
		JPrintf("too many seeds error!\n");
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}
	//检测第二条红线时，如果种子点过少 视为两指针线重合
	if (seedsTemp.lSeedNum<LEAST_NUM && pParam->lDoubleRedLine==1)
	{
		JPrintf("too few seeds,the same line\n");
		goto EXT;

	}
	GO(RANSAC_LINE(hMemMgr, &haarResult, &haarAngleResult, seedsTemp.pptSeed, seedsTemp.lSeedNum, pTemp, lStackLen, pParam));
	pLineParam = &(pParam->pCircleInfo->lineInfo.lineParam);
	lThreshold = lThreshold * 3 / 5;
	//printf("**********gPictureNum=%d\n",gPictureNum);
	CalcLineEndPt_xsd(hMemMgr,pImage, &haarResult, pParam->pCircleInfo->circleParam.lRadius, lThreshold, pLineParam);

EXT:
	B_Release(hMemMgr,&haarResult);
	B_Release(hMemMgr, &haarAngleResult);
	FreeVectMem(hMemMgr,haarParam.pAngleList);
	FreeVectMem(hMemMgr,seedsTemp.pptSeed);
	FreeVectMem(hMemMgr,seedsTemp.pcrSeed);
	FreeVectMem(hMemMgr,pTemp);
	B_Release(hMemMgr,&blockFlag);
	//FreeVectMem(hMemMgr,pSeedsAngle);

	return res;
}
//************************************************************************************

MRESULT HaarResponseAngle(MHandle hMemMgr ,BLOCK *pImage, PHaarParam pParam,BLOCK *pResponseResultImage,BLOCK *pResponseAngle)
{
	MRESULT res=LI_ERR_NONE;
	MLong i, j;
	BLOCK imageRotate={0};
	MLong lRotateWidth, lRotateHeight;
	BLOCK xDstBlock={0}, yDstBlock={0};
	JIntegral *pIntegral;
	MLong lIntegralLine;
	ResponseResult responseResult = {0};
	ResponseResult responseImage={0};
	MLong lMaxValue;
	MLong *plongData;
	MLong lDataExt, lByteDataExt;
	MByte *pByteData;

	responseResult.lWidth = pImage->lWidth;
	responseResult.lHeight = pImage->lHeight;
	responseResult.lBlockLine = CEIL_4(responseResult.lWidth);
	AllocVectMem(hMemMgr, responseResult.pData, responseResult.lBlockLine*responseResult.lHeight, MLong);
	SetVectZero(responseResult.pData, responseResult.lBlockLine*responseResult.lHeight*sizeof(MLong));
	lRotateWidth = lRotateHeight=(MLong) (sqrt((MDouble)pImage->lWidth*pImage->lWidth+pImage->lHeight*pImage->lHeight)+1);
	lIntegralLine = CEIL_4(lRotateWidth+1);
	GO(B_Create(hMemMgr,&imageRotate,DATA_U8,lRotateWidth,lRotateHeight));
	GO(B_Create(hMemMgr,&xDstBlock,DATA_I16,lRotateWidth,lRotateHeight));
	GO(B_Create(hMemMgr,&yDstBlock,DATA_I16,lRotateWidth,lRotateHeight));
	AllocVectMem(hMemMgr,responseImage.pData,imageRotate.lBlockLine*lRotateHeight,MLong);
	SetVectZero(responseImage.pData, imageRotate.lBlockLine*lRotateHeight*sizeof(MLong));
	AllocVectMem(hMemMgr, pIntegral, lIntegralLine*lRotateHeight, JIntegral);//���Ŀռ�
	//���ǶȵĽ����ʼ��Ϊ127��127��һ����ֵ����Ϊ���λ��������
	SetVectMem(pResponseAngle->pBlockData, pResponseAngle->lBlockLine*pResponseAngle->lHeight, 0, MByte);
	lMaxValue = 0;
	//Ϊ�˼��ټ�������Ҳ���Ը���ͳ��������趨��ֵ����mask
	for (i=0;i<pParam->lAngleNumber;i++)
	{
		/*FILE *fp;
		char fileName[20];
		JIntegral *pIntergralData;
		MLong *pResponseData;
		int xx, yy;
		MLong lExt;*/

		RotateImage_xsd(pImage,&imageRotate,pParam->pAngleList[i],&xDstBlock,&yDstBlock);
		responseImage.lBlockLine=imageRotate.lBlockLine;
		responseImage.lWidth=imageRotate.lWidth;
		responseImage.lHeight=imageRotate.lHeight;
		//��ȡhaar��Ӧͼ
		lIntegralLine = CEIL_4(imageRotate.lWidth);
		Integral(imageRotate.pBlockData, imageRotate.lBlockLine, DATA_U8,pIntegral,MNull, lIntegralLine,imageRotate.lWidth,imageRotate.lHeight);

		/*sprintf(fileName, ".\\log\\intergral_%d.dat", i);
		fp = fopen(fileName, "w+");
		pIntergralData = pIntegral;
		lExt = lIntegralLine - imageRotate1.lWidth;
		for (yy=0; yy<imageRotate1.lHeight; yy++, pIntergralData+=lExt)
		{
		fprintf(fp, "(%4d)", yy);
		for (xx=0; xx<imageRotate1.lWidth; xx++, pIntergralData++)
		{
		fprintf(fp, "%10d(%3d)", *pIntergralData, xx);
		}
		fprintf(fp, "\n");
		}
		fclose(fp);*/

		GetResponseResult(pIntegral, lIntegralLine, responseImage.pData, responseImage.lBlockLine,
			imageRotate.lWidth, imageRotate.lHeight, pParam);

		/*sprintf(fileName, ".\\log\\response_%d.dat", i);
		fp = fopen(fileName, "w+");
		pResponseData = responseImage.pData;
		lExt = responseImage.lBlockLine - responseImage.lWidth;
		for (yy=0; yy<responseImage.lHeight; yy++, pResponseData+=lExt)
		{
		fprintf(fp, "(%4d)", yy);
		for (xx=0; xx<responseImage.lWidth; xx++, pResponseData++)
		{
		fprintf(fp, "%10d(%3d)", *pResponseData, xx);
		}
		fprintf(fp, "\n");
		}
		fclose(fp);*/

		OptimizeHaar_xsd(&responseImage,&responseResult,&xDstBlock,&yDstBlock,pResponseAngle,&lMaxValue,(MByte)i);
	}
	//��һ��
	lDataExt = responseResult.lBlockLine - responseResult.lWidth;
	plongData = responseResult.pData;
	lByteDataExt = pResponseResultImage->lBlockLine-responseResult.lWidth;
	pByteData = pResponseResultImage->pBlockData;
	for (j=0;j<responseResult.lHeight;j++,plongData+=lDataExt,pByteData+=lByteDataExt)
	{
		for (i=0;i<responseResult.lWidth;i++,plongData++,pByteData++)
		{
			if ((*plongData&(~TEMPLATE_TYPE_LONG_BIT))!=0)
			{
				*(pByteData) = TRIM_UINT8((MByte)((*plongData&(~TEMPLATE_TYPE_LONG_BIT))*255/lMaxValue));
			}
			else
			{
				*(pByteData)=0;
			}
		}
	}
EXT:
	FreeVectMem(hMemMgr, responseResult.pData);
	FreeVectMem(hMemMgr,responseImage.pData);
	FreeVectMem(hMemMgr, pIntegral);
	B_Release(hMemMgr,&imageRotate);
	B_Release(hMemMgr,&xDstBlock);
	B_Release(hMemMgr,&yDstBlock);
	return res;
}

MVoid GetResponseResult(JIntegral *pIntegral, MLong lIntegralLine,
	MLong *pDstData, MLong lDstLine, 
	MLong lWidth, MLong lHeight, PHaarParam pParam)
{
	MLong i,j;
	MUInt32 lTrimValue;
	//MUInt32 lTmpValue1, lTmpValue2;
	MLong lDataStep1,lDataStep2,lDataStep3;
	MLong lDstExt = lDstLine - lWidth;
	MLong lIntExt = lIntegralLine  - lWidth;
	MLong w=pParam->w;
	MLong h=pParam->h;
	MUInt32 sum1,sum2,sum3,sum4,sum5;
	MLong t, factor;//����16

	SetVectMem(pDstData,lDstLine*lHeight,0,MLong);

	h = h*5/3;//Ϊ���ܼ���һ��߶ȵ�ģ��
	if (pParam->lClassNumber==4)
	{
		t=8;//t = 0.5 * 16
		factor = 13;//factor = 0.8 *16
		pDstData += (lDstLine)*(h/2)+(w/2);
		pIntegral += lIntegralLine * h + w;
		lDataStep1 = lIntegralLine * (h/5);
		lDataStep2 = lIntegralLine * (h*2/5);
		lDataStep3 = lIntegralLine * (h*3/5);
		//SetVectZero(pDstData-(lDstLine)*(h/2)+(w/2), h*lDstLine);
		for (j=h;j<lHeight;j++,pDstData+=lDstExt+w, pIntegral+=lIntExt+w)
		{
			//SetVectZero(pDstData-w/2, w/2);
			for (i=w;i<lWidth;i++,pDstData++, pIntegral++)
			{
				//3 top
				//lLeft=i-w;
				//lRight=i;
				//lTop=j-h;
				//lBottom=j-h*4/5;
				//sum1=*(pIntData+lBottom*lBlockLine+lRight)+*(pIntData+lLeft+lTop*lBlockLine)-*(pIntData+lBottom*lBlockLine+lLeft)-*(pIntData+lRight+lTop*lBlockLine);
				sum1 = *(pIntegral-lDataStep2*2)+*(pIntegral-lDataStep2-lDataStep3-w)
					-*(pIntegral-lDataStep2*2-w)-*(pIntegral-lDataStep2-lDataStep3);
				//3 top mid
				//lLeft=i-w;
				//lRight=i;
				//lTop=j-h*4/5;
				//lBottom=j-h*3/5;
				//sum2=*(pIntData+lBottom*lBlockLine+lRight)+*(pIntData+lLeft+lTop*lBlockLine)-*(pIntData+lBottom*lBlockLine+lLeft)-*(pIntData+lRight+lTop*lBlockLine);
				sum2 = *(pIntegral-lDataStep3)+*(pIntegral-lDataStep2*2-w)
					-*(pIntegral-lDataStep3-w)-*(pIntegral-lDataStep2*2);
				//3 mid
				//lLeft=i-w;
				//lRight=i;
				//lTop=j-h*3/5;
				//lBottom=j-h*2/5;
				//sum3=*(pIntData+lBottom*lBlockLine+lRight)+*(pIntData+lLeft+lTop*lBlockLine)-*(pIntData+lBottom*lBlockLine+lLeft)-*(pIntData+lRight+lTop*lBlockLine);
				sum3 = *(pIntegral-lDataStep2)+*(pIntegral-lDataStep3-w)
					-*(pIntegral-lDataStep2-w)-*(pIntegral-lDataStep3);
				//3 bottom mid
				//lLeft=i-w;
				//lRight=i;
				//lTop=j-h*2/5;
				//lBottom=j-h/5;
				//sum4=*(pIntData+lBottom*lBlockLine+lRight)+*(pIntData+lLeft+lTop*lBlockLine)-*(pIntData+lBottom*lBlockLine+lLeft)-*(pIntData+lRight+lTop*lBlockLine);
				sum4 = *(pIntegral-lDataStep1)+*(pIntegral-lDataStep2-w)
					-*(pIntegral-lDataStep1-w)-*(pIntegral-lDataStep2);
				//3 bottom
				//lLeft=i-w;
				//lRight=i;
				//lTop=j-h/5;
				//lBottom=j;
				//sum5=*(pIntData+lBottom*lBlockLine+lRight)+*(pIntData+lLeft+lTop*lBlockLine)-*(pIntData+lBottom*lBlockLine+lLeft)-*(pIntData+lRight+lTop*lBlockLine);
				sum5 = *(pIntegral)+*(pIntegral-lDataStep1-w)
					-*(pIntegral-w)-*(pIntegral-lDataStep1);

				lTrimValue = 0;
				//����2��Լ������
				//1.����ģʽ
				//		+++++
				//		------
				//		+++++
				//2.����ģʽ
				//
				//		    ++
				//		    --
				//          ++

				//if (90==i && 90==j)
				//{
				//	printf("hahaha\n");
				//}
				if (pParam->bWhiteBackground)	//���̱������ȸ���ָ������(factor>>4)
				{
					//if (sum3<(sum2*19/20) && sum3<(sum4*19/20))
					if (sum3<(sum2*39/40) && sum3<(sum4*39/40))
					{
						lTrimValue = ((sum2+sum4)>>1) - sum3;
					}
					else
					{
						lTrimValue = 0;
					}
				}
				else
				{
					if (sum3>(sum2*21/20) && sum3>(sum4*21/20))
					{
						lTrimValue = sum3 - ((sum2+sum4)>>1);
					}
					else
					{
						lTrimValue = 0;
					}
				}

				*pDstData = lTrimValue;
			}
		}
	}
}

MVoid GetResponseResult_xsd(JIntegral *pIntegralData, MLong lIntegralLine,MLong *pDst, 
	MLong lDstLine,MLong lWidth, MLong lHeight,PHaarParam pParam)
{
	MLong i,j;
	MLong lDstExt=lDstLine-lWidth;
	MLong lIntExt = lIntegralLine  - lWidth;
	MLong w=pParam->w;
	MLong h=pParam->h;
	MUInt32 sum1,sum2,sum3,sum4,sum5;
	MLong t, factor;//����16
	MLong *pDstData;
	JIntegral *pIntegral;
	MUInt32 lTrimValue, lTmpValue1, lTmpValue2;
	MLong lDataStep1,lDataStep2,lDataStep3;
	MLong lStep, lWOff1, lWOff2, lWOff3;

	pDstData = pDst;
	pIntegral = pIntegralData;
	SetVectMem(pDstData,lDstLine*lHeight,0,MLong);

	h = h*5/3;//Ϊ���ܼ���һ��߶ȵ�ģ��
	if (pParam->lClassNumber==4)
	{
		t=8;//t = 0.5 * 16
		factor = 13;//factor = 0.8 *16
		pDstData+=(lDstLine)*(h/2)+(w/2);
		pIntegral+=lIntegralLine*h+w;
		lDataStep1 = lIntegralLine*h/5;
		lDataStep2 = lIntegralLine*h*2/5;
		lDataStep3 = lIntegralLine*h*3/5;
		//SetVectZero(pDstData-(lDstLine)*(h/2)+(w/2), h*lDstLine);
		for (j=h;j<lHeight;j++,pDstData+=lDstExt+w, pIntegral+=lIntExt+w)
		{
			//SetVectZero(pDstData-w/2, w/2);
			for (i=w;i<lWidth;i++,pDstData++, pIntegral++)
			{
				sum1 = *(pIntegral-lDataStep2*2)+*(pIntegral-lDataStep2-lDataStep3-w)
					- *(pIntegral-lDataStep2*2-w)-*(pIntegral-lDataStep2-lDataStep3);

				sum2 = *(pIntegral-lDataStep3)+*(pIntegral-lDataStep2*2-w)
					- *(pIntegral-lDataStep3-w)-*(pIntegral-lDataStep2*2);

				sum3 = *(pIntegral-lDataStep2)+*(pIntegral-lDataStep3-w)
					- *(pIntegral-lDataStep2-w)-*(pIntegral-lDataStep3);

				sum4 = *(pIntegral-lDataStep1)+*(pIntegral-lDataStep2-w)
					- *(pIntegral-lDataStep1-w)-*(pIntegral-lDataStep2);

				sum5 = *(pIntegral)+*(pIntegral-lDataStep1-w)
					- *(pIntegral-w)-*(pIntegral-lDataStep1);

				lTrimValue = 0;

				if (sum3<(sum2*factor>>4)&&sum3<(sum4*factor>>4))
					lTrimValue =(sum2+sum4)/2-sum3;

				lTmpValue1 =sum1*factor>>4;
				lTmpValue2 = sum5*factor>>4;

				if (sum2<lTmpValue1&&sum3<lTmpValue1&&sum4<lTmpValue1
					&&sum2<lTmpValue2&&sum3<lTmpValue2&&sum4<lTmpValue2 
					&&( (sum5+sum1)/2-(sum2+sum3+sum4)/3>lTrimValue))
				{
					lTrimValue =  ((sum5+sum1)/2-(sum2+sum3+sum4)/3);
					lTrimValue |= TEMPLATE_TYPE_LONG_BIT;
				}
				//*pDstData = lTrimValue/winSize;//����������ͳһ������ѭ�����ڲ�Խ��Խ��
				*pDstData = lTrimValue;
			}
		}

		pDstData = pDst + (lDstLine)*(w/2)+(h/2);
		pIntegral = pIntegralData;
		lStep = lIntegralLine * w;
		lWOff1 = h / 5;
		lWOff2 = h * 2 / 5;
		lWOff3 = h * 3 / 5;
		lDstExt += h;
		lIntExt += h;
		for (j=w; j<lHeight; j++, pDstData+=lDstExt, pIntegral+=lIntExt)
		{
			for (i=h; i<lWidth; i++, pDstData++, pIntegral++)
			{
				lTrimValue = 0;

				sum1 = *pIntegral + *(pIntegral + lStep + lWOff1)
					- *(pIntegral + lWOff1) - *(pIntegral + lStep);

				sum2 = *(pIntegral + lWOff1) + *(pIntegral + lStep + lWOff2)
					- *(pIntegral + lWOff2) - *(pIntegral + lStep + lWOff1);

				sum3 = *(pIntegral + lWOff2) + *(pIntegral + lStep + lWOff3)
					- *(pIntegral + lWOff3) - *(pIntegral + lStep + lWOff2);

				sum4 = *(pIntegral + lWOff3) + *(pIntegral + lStep + lWOff1 + lWOff3)
					- *(pIntegral + lWOff1 + lWOff3) - *(pIntegral + lStep + lWOff3);

				sum5 = *(pIntegral + lWOff1 + lWOff3) + *(pIntegral + lStep + lWOff2 + lWOff3)
					- *(pIntegral + lWOff2 + lWOff3) - *(pIntegral + lStep + lWOff1 + lWOff3);

				if(sum3<(sum2*factor>>4) && sum3<(sum4*factor>>4))
					lTrimValue = (sum2 + sum4)/2 - sum3;

				lTmpValue1 =sum1*factor>>4;
				lTmpValue2 = sum5*factor>>4;

				if(sum2<lTmpValue1 && sum3<lTmpValue1 && sum4<lTmpValue1
					&& sum2<lTmpValue2 && sum3<lTmpValue2 && sum4<lTmpValue2
					&& ((sum1 + sum5)/2 - (sum2 + sum3 + sum4)/3)>lTrimValue)
				{
					lTrimValue = (sum1 + sum5)/2 - (sum2 + sum3 + sum4)/3;
					lTrimValue |= TEMPLATE_TYPE_LONG_BIT;
				}

				if ((lTrimValue & (~TEMPLATE_TYPE_LONG_BIT))>(*pDstData & (~TEMPLATE_TYPE_LONG_BIT)))  // compare
					*pDstData = lTrimValue;
			}
		}
	}
}

MVoid GetResponseResult_xsd2(JIntegral *pIntegral, MLong lIntegralLine,MLong *pDstData, 
	MLong lDstLine,MLong lWidth, MLong lHeight,PHaarParam pParam, MLong *lMaxVal)
{
	MLong i,j;
	MLong lDstExt=lDstLine-lWidth;
	MLong lIntExt = lIntegralLine  - lWidth;
	MLong w=pParam->w;
	MLong h=pParam->h;
	MUInt32 sum1,sum2,sum3;
	MUInt32 lTrimValue;
	MLong lDataStep1,lDataStep2,lDataStep3;

	SetVectMem(pDstData, lDstLine*lHeight, 0, MLong);
	*lMaxVal = 1;

	if (pParam->lClassNumber==4)
	{
		pDstData += (lDstLine)*(h/2) + (w/2);
		pIntegral += lIntegralLine*h + w;
		lDataStep1 = lIntegralLine*h/3;
		lDataStep2 = lIntegralLine*h*2/3;
		lDataStep3 = lIntegralLine*h;
		//SetVectZero(pDstData-(lDstLine)*(h/2)+(w/2), h*lDstLine);
		for (j=h;j<lHeight;j++,pDstData+=lDstExt+w, pIntegral+=lIntExt+w)
		{
			for (i=w;i<lWidth;i++,pDstData++, pIntegral++)
			{
				sum1 = *(pIntegral-lDataStep2) + *(pIntegral-lDataStep3-w)
					- *(pIntegral-lDataStep2-w) - *(pIntegral-lDataStep3);

				sum2 = *(pIntegral-lDataStep1) + *(pIntegral-lDataStep2-w)
					- *(pIntegral-lDataStep1-w) - *(pIntegral-lDataStep2);

				sum3 = *(pIntegral) + *(pIntegral-lDataStep1-w)
					- *(pIntegral-w) - *(pIntegral-lDataStep1);

				lTrimValue = 0;
				//����2��Լ������
				//1.����ģʽ
				//		+++++
				//		------
				//		+++++
				//2.����ģʽ
				//
				//		    ++
				//		    --
				//               ++
				//
				if (sum2<sum1&&sum2<sum3)
					lTrimValue =((sum1+sum3)>>1)-sum2;

				*pDstData = lTrimValue;
				if(*pDstData > *lMaxVal)
					*lMaxVal = *pDstData;
			}
		}
	}
}

//������ͼ���ϻ��̶����Լ�ָ��ķ�ʽ���ָ����̶��̵Ľ��㣬Ȼ����ݶ��������������յĶ���
MLong PTInfoCmp( const MVoid* p1, const MVoid* p2)
{
	PTINFO _p1 = *(PTINFO*)p1;
	PTINFO _p2 = *(PTINFO*)p2;

	if( _p1.dPtAngle< _p2.dPtAngle)
		return 1;
	if (_p1.dPtAngle> _p2.dPtAngle)
		return -1;
	return 0;
}

MLong PTInfoCmp_Ascend( const MVoid* p1, const MVoid* p2)
{
	PTINFO _p1 = *(PTINFO*)p1;
	PTINFO _p2 = *(PTINFO*)p2;

	if( _p1.dPtAngle> _p2.dPtAngle)
		return 1;
	if (_p1.dPtAngle< _p2.dPtAngle)
		return -1;
	return 0;
}

MLong PTInfoCmp_X( const MVoid* p1, const MVoid* p2)
{
	PTINFO _p1 = *(PTINFO*)p1;
	PTINFO _p2 = *(PTINFO*)p2;

	if( _p1.ptPoint.x< _p2.ptPoint.x)
		return 1;
	if (_p1.ptPoint.x> _p2.ptPoint.x)
		return -1;
	return 0;
}

MLong PTInfoCmp_Y( const MVoid* p1, const MVoid* p2)
{
	PTINFO _p1 = *(PTINFO*)p1;
	PTINFO _p2 = *(PTINFO*)p2;

	if( _p1.ptPoint.y< _p2.ptPoint.y)
		return 1;
	if (_p1.ptPoint.y> _p2.ptPoint.y)
		return -1;
	return 0;
}



//(*cmp)()���������ط����ʾ�Ǳ�merge�Ķ��󣬷����� ��ʾ���Ǳ�merge�Ķ���
//(*replace)()
MVoid MergeSet(MVoid *pDataList,MLong elemSize, MLong *pListSize, MLong (*cmp)(const MVoid*, const MVoid*),
	MLong (*replace)(const MVoid*, const MVoid*))
{
	MLong i, j, l,k, m, n;
	MVoid *pElem1, *pElem2;
	i=1;
	j = *pListSize - 1;
	{
		for (l=i; l<=j;l++)
		{
			for (k=0; k<l; k++)
			{
				pElem1 = (MByte*)pDataList+elemSize*k;	// elem before index l
				pElem2 = (MByte*)pDataList+elemSize*l;
				if ((*cmp)(pElem1, pElem2)) 
					break;
			}
			if (k==l)//�޳�ͻ��Ԫ��,��һ��ѭ��
				continue;
			//�г�ͻ��Ԫ�أ�Ҫ��β����ʼ����l==j,����ǰ��Ԫ�������һ����ʱ��
			for (m=j; m>l; m--)
			{
				for (n=0; n<l; n++)
				{
					pElem1 = (MByte*)pDataList+elemSize*n;
					pElem2 = (MByte*)pDataList+elemSize*m;
					if ((*cmp)(pElem1,pElem2)) 
						break;
				}

				if (n!=l)//�г�ͻ����ʵʩ�滻��������
				{
					//β����ֵ��Ҫ��
					(*pListSize) = (*pListSize)-1;
					j--;
					continue;
				}
				//�޳�ͻ���ҵ���ʵʩ�滻��Ԫ��
				break;
			}
			//if (m==l) //�����û�п���ʵʩ�滻��Ԫ�أ���ô�˳�
			//	continue;

			//�޳�ͻ������ֵ�滻ǰ���l ֵ
			pElem1 =  (MByte*)pDataList+elemSize*l;
			pElem2 = (MByte*)pDataList+elemSize*m;
			(*replace)(pElem1, pElem2);
			(*pListSize) = (*pListSize)-1;
			j--;
		}
	}
}

MLong CircleInfoCmp( const MVoid* p1, const MVoid* p2)
{

	CIRCLE_INFO *_p1 = (CIRCLE_INFO*)p1;
	CIRCLE_INFO *_p2 = (CIRCLE_INFO*)p2;

	if (_p1->circleParam.xcoord == _p2->circleParam.xcoord
		&& _p1->circleParam.ycoord == _p2->circleParam.ycoord
		&& _p1->circleParam.lRadius == _p2->circleParam.lRadius)
		return 1;
	else
		return 0;
}

MLong CircleInfoReplace(const MVoid *p1, const MVoid* p2)
{
	CIRCLE_INFO *_p1 = (CIRCLE_INFO*)p1;
	CIRCLE_INFO *_p2 = (CIRCLE_INFO*)p2;

	//��ʱ��ֱ��cpy������˵Ӧ����merge
	if (_p2->lLineNum!=0)
	{
		JMemCpy(&_p1->lineInfo.lineParam,&(_p2->lineInfo.lineParam),sizeof(LINEPARAM));
		JMemCpy(_p1->lineInfo.pPtinfo, _p2->lineInfo.pPtinfo, _p2->lineInfo.lPtNum*sizeof(PTINFO));
		_p1->lineInfo.lConfidence = _p2->lineInfo.lConfidence;
		_p1->lineInfo.lPtNum= _p2->lineInfo.lPtNum;
	}
	JMemCpy(&(_p1->circleParam), &(_p2->circleParam), sizeof(CIRCLE));
	_p1->lConfidence = _p2->lConfidence;
	_p1->lLineNum = _p2->lLineNum;
	_p1->lPtNum = _p2->lPtNum;
	JMemCpy((_p1->pPtinfo), _p2->pPtinfo, _p2->lPtNum*sizeof(PTINFO));
	return  0;
}

MLong LineInfoConfidenceCmp(const MVoid* p1, const MVoid* p2)
{
	LINE_INFO *_p1 = (LINE_INFO*)p1;
	LINE_INFO *_p2 = (LINE_INFO*)p2;

	if( _p1->lConfidence< _p2->lConfidence)
		return 1;
	if (_p1->lConfidence > _p2->lConfidence)
		return -1;
	return 0;
}

//xsd  �Ƚ�����ֱ������ϵ��������
MLong LineInfoPtNumCmp(const MVoid* p1, const MVoid* p2)
{
	LINE_INFO *_p1 = (LINE_INFO*)p1;
	LINE_INFO *_p2 = (LINE_INFO*)p2;

	if( _p1->lPtNum< _p2->lPtNum)
		return 1;
	if (_p1->lPtNum > _p2->lPtNum)
		return -1;
	return 0;
}

MLong CircleInfoConfidenceCmp(const MVoid* p1, const MVoid* p2)
{
	CIRCLE_INFO*_p1 = (CIRCLE_INFO*)p1;
	CIRCLE_INFO *_p2 = (CIRCLE_INFO*)p2;

	if( _p1->lConfidence< _p2->lConfidence)
		return 1;
	if (_p1->lConfidence > _p2->lConfidence)
		return -1;
	return 0;
}

MLong LineInfoCmp(const MVoid* p1, const MVoid* p2)
{
	LINE_INFO *_p1 = (LINE_INFO*)p1;
	LINE_INFO *_p2 = (LINE_INFO*)p2;

	if (_p1->lineParam.Coefk == _p2->lineParam.Coefk
		&& _p1->lineParam.Coefb== _p2->lineParam.Coefb
		&& _p1->lineParam.bVertical== _p2->lineParam.bVertical)
		return 1;
	else
		return 0;
}

MLong LineInfoReplace(const MVoid *p1, const MVoid* p2)
{
	LINE_INFO *_p1 = (LINE_INFO*)p1;
	LINE_INFO *_p2 = (LINE_INFO*)p2;
	//��ʱ��ֱ��cpy������˵Ӧ����merge
	JMemCpy(&_p1->lineParam,&(_p2->lineParam),sizeof(LINEPARAM));
	JMemCpy(_p1->pPtinfo, _p2->pPtinfo, _p2->lPtNum*sizeof(PTINFO));
	_p1->lConfidence = _p2->lConfidence;
	_p1->lPtNum= _p2->lPtNum;
	return  0;
}

//lRadiusCons�ǲ���������Լ���뾶��С
//RANSAC��ʱ��Ҫ������Ӧ�Ƕȣ�ģʽ�Ƿ���Ӧ���������Ĵ�ֱ��
MRESULT RANSAC_CIRCLE(MHandle hMemMgr, BLOCK *pHaarResponse,
	BLOCK* pHaarAngleResponse, MPOINT *pPtList, MLong lPtNum, 
	CIRCLE_SET_INFO *pCircleSetInfo,MPOINT *pTemp,MLong lTmpNum, MLong lRadiusCons,
	PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;
	MLong i, j, k, l, m;
	MPOINT randomPoint[3];
	MPOINT *inPoint=pTemp;
	MLong numInPoint=0;
	MLong x=0,y=0,r=0;
	MDouble distance, dAngle, dTmpAngle;
	MLong lAngleValue,lAngleValue2;
	MLong minNumPoint = 5;
	MLong minNumPtOnLine = 3;
	MDouble dCoeffK=0;
	MDouble dCoeffB=0;
	MBool bVert=MFalse;
	MDouble dCoeffK1=0;
	MDouble dCoeffB1 = 0;
	MBool bVert1=MFalse;
	MLong numInPoint1=0;
	MLong lPTMeanSum1,lPTMeanSum2,lPTMeanSum3;
	MLong lMeanSum;
	JGSEED seedTmp = {0};//Ŀ����Ϊ����ʱ�洢��Բ�ϵ�֮��������㣬�������ֱ��
	LINE_SET_INFO lineSetInfo = {0};
	BLOCK blockFlagTmp2={0};

	MPOINT *pTmpPt = MNull;
	MLong *pAngleValue = MNull;
	MLong timeStart, timeElapsed;
	MLong lMaxCircleRadius = 0;
	//	BLOCK blockFlagTmp = {0};

	//xsd
	//	printf("ransac circle input:\n lPTNum = %d\n", lPtNum);
	//	for(i=0; i<lPtNum; i++)
	//		printf("(%d,%d) ", pPtList[i].x, pPtList[i].y);
	//	printf("\n~~~~~~~~~~~~\n");

	AllocVectMem(hMemMgr, seedTmp.pptSeed, lPtNum, MPOINT);
	AllocVectMem(hMemMgr, seedTmp.pcrSeed, lPtNum, MByte);
	AllocVectMem(hMemMgr, pTmpPt, lPtNum, MPOINT);
	AllocVectMem(hMemMgr, pAngleValue, lPtNum, MLong);
	B_Create(hMemMgr, &blockFlagTmp2, DATA_U8,pHaarResponse->lWidth, pHaarResponse->lHeight);
	timeStart = JGetCurrentTime();

	//�ȼ���angle��ֵ
	for (i=0; i<lPtNum; i++)
	{
		pAngleValue[i] = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
			pPtList[i].y+pPtList[i].x) &(~TEMPLATE_TYPE_BYTE_BIT);
		pAngleValue[i] = pAngleValue[i] * ANGLE_STEP+90;
	}
	//������ظ��ĺ�ѡԲ�����⣬����ļ�����ô����?
	//lPtNum��ѡ3�����в������㣬��Ҫ�������е�3�����
	//ʵ��ʵ���б����ķ�ʽ�����˱仯����֪���Ƿ��
	//������Բ������ǣ�ͬһ��ͨ����ĵ㲻���룬���Կ�������
	//��ͨ���򣬼��ٲ�������Ŀ
	for (i=0; i<lPtNum; i++)
	{
		inPoint[0].x = pPtList[i].x;
		inPoint[0].y = pPtList[i].y;
		for (j=i+1; j<lPtNum; j++)
		{
			inPoint[1].x = pPtList[j].x;
			inPoint[1].y = pPtList[j].y;
			for (k=j+1; k<lPtNum; k++)
			{
				inPoint[2].x = pPtList[k].x;
				inPoint[2].y = pPtList[k].y;
				numInPoint = 0;
				//��ÿһ����Ԫ��Ͻ��в�������
				if (vCircleFitting(inPoint,3,&x,&y,&r)<0)
					continue;
				//���������r��������⣬Բ�ĵ�λ��Ҳ�������ƣ���ô����
				if (r>=lRadiusCons/2||r<=lRadiusCons/4 || x<pHaarResponse->lWidth/4 ||x >pHaarResponse->lWidth*3/4 
					|| y<pHaarResponse->lHeight/4 || y>pHaarResponse->lHeight*3/4)
					continue;
				numInPoint = 0;;
				for (l=0;l<lPtNum;l++)//
				{
					if (bOnCircle_EX(x,y,r,pPtList[l].x,pPtList[l].y,RANSAC_THRESHOLD3)==0)continue;
					dAngle = vComputeAngle(pPtList[l].x-x, pPtList[l].y-y) *180/V_PI;
					dTmpAngle = pAngleValue[l];
					dAngle = fabs(dAngle-dTmpAngle);//0~270
					if ((( dAngle>90-RANSAC_ANGLE_BIAS&&dAngle<90+RANSAC_ANGLE_BIAS)||dAngle>270-RANSAC_ANGLE_BIAS))//ƫ����10������
					{
						inPoint[numInPoint].x=pPtList[l].x;
						inPoint[numInPoint++].y=pPtList[l].y;
					}
				}
				//����ͽ��й���
				if (numInPoint>minNumPoint &&pCircleSetInfo->lParamNum<pCircleSetInfo->lMaxParamNum)
				{
					if (vCircleFitting(inPoint,numInPoint,&x,&y,&r)<0)
						continue;
					//confidence���ں�����
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].lConfidence = 0;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.xcoord = x;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.ycoord = y;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.lRadius = r;
					for (l=0;l<numInPoint;l++)
					{
						pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].pPtinfo[l].ptPoint.x=inPoint[l].x;
						pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].pPtinfo[l].ptPoint.y=inPoint[l].y;
					}
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].lPtNum = numInPoint;
					pCircleSetInfo->lParamNum++; 
				}
			}
		}
	}
	JPrintf("---1\n");
	//merge circle parameters
	//�ϲ����㷨,����ָ��ֱ�ָ��ͷ��β����ͷ��ʼ���������ظ��ģ����ظ�����ɾ��
	//�����β����ʼ����ֱ���������ظ��ģ����ò��ֵ�ֵ���뵽ǰ�棬ֱ��ͷ��β����
	MergeSet((MVoid *)pCircleSetInfo->pCircleInfo, sizeof(CIRCLE_INFO), &(pCircleSetInfo->lParamNum),CircleInfoCmp, CircleInfoReplace);
	timeElapsed = JGetCurrentTime()-timeStart;
	JAddToTimer(timeElapsed, TIMER_RANSACCIRCLE_CIRCLE);
	//����Բ��ѡ�㣬��ÿ����ѡ��Բ�����Ӧ �Ĵ�ָ��ֵ�����ܳ�Ϊһ������
	//��ģʽ��
	timeStart = JGetCurrentTime();
	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));
	lMaxCircleRadius = 0;
	JPrintf("circleNum=%d\n", pCircleSetInfo->lParamNum);
	for (i=0; i<pCircleSetInfo->lParamNum; i++)
	{
		CIRCLE_INFO *pCircleInfo = pCircleSetInfo->pCircleInfo+i; 
		JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
		seedTmp.lSeedNum = lPtNum;

		//filter points which are not in circle
		//��Բ��ĵ�
		for(j=0;j<seedTmp.lSeedNum; j++)
		{
			if (bInCircle_EX(pCircleInfo->circleParam.xcoord,pCircleInfo->circleParam.ycoord,pCircleInfo->circleParam.lRadius+RANSAC_THRESHOLD3/2,//������
				seedTmp.pptSeed[j].x, seedTmp.pptSeed[j].y))continue;
			for(k=seedTmp.lSeedNum-1;k>j; k--)
			{
				if (bInCircle_EX(pCircleInfo->circleParam.xcoord,pCircleInfo->circleParam.ycoord,pCircleInfo->circleParam.lRadius+RANSAC_THRESHOLD3/2,
					seedTmp.pptSeed[k].x, seedTmp.pptSeed[k].y))break;	
				seedTmp.lSeedNum--;
			}
			if (j==k)
				seedTmp.lSeedNum--;
			else
			{
				seedTmp.pptSeed[j].x = seedTmp.pptSeed[k].x;
				seedTmp.pptSeed[j].y = seedTmp.pptSeed[k].y;
				seedTmp.lSeedNum--;
			}
		}
		lineSetInfo.lLineNum = 0;
		//�Ժ�ѡ���б�������
		for (j=0; j<seedTmp.lSeedNum; j++)
		{
			randomPoint[0].x = seedTmp.pptSeed[j].x;
			randomPoint[0].y = seedTmp.pptSeed[j].y;
			for (k=j+1; k<seedTmp.lSeedNum; k++)
			{
				MBool bChanged;
				//��һ�����
				//��ϵ�������Ҫ����һ����Ҫ����Ӧ������һ�µ�
				//Ϊ�˲��ó��ֹ���ϣ���Ҫ����ϸ�о�һ��
				randomPoint[1].x = seedTmp.pptSeed[k].x;
				randomPoint[1].y = seedTmp.pptSeed[k].y;

				//����
				lAngleValue = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
					randomPoint[0].y+randomPoint[0].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
					randomPoint[1].y+randomPoint[1].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				dTmpAngle = ABS(lAngleValue -lAngleValue2)*ANGLE_STEP;
				if (dTmpAngle>10&&dTmpAngle<170)continue;
				if(vFitLine(randomPoint,2,&dCoeffK, &dCoeffB, &bVert)<0)
					continue;

				//һֱ��ϣ�ֱ��������������
				bChanged = MTrue;
				numInPoint1 = 0;
				while(bChanged)
				{
					//����ҪС����ϲ����������⣬����������ѭ��
					//���ȷʵ������������Ҫ��������ʹ��������
					//numInPointҪ�������������,Ӧ���������С
					bChanged = MFalse;
					numInPoint=0;
					for (m = 0 ; m<seedTmp.lSeedNum; m++)
					{
						//�㵽�ߵľ���
						distance = vPointDistToLine2(seedTmp.pptSeed[m], dCoeffK, dCoeffB, bVert);
						lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
							seedTmp.pptSeed[m].y+seedTmp.pptSeed[m].x) &(~TEMPLATE_TYPE_BYTE_BIT);
						dTmpAngle=ABS(lAngleValue-lAngleValue2)*ANGLE_STEP;
						if (dTmpAngle>10 && dTmpAngle<170)continue;
						if (distance < RANSAC_THRESHOLD3)
						{
							pTmpPt[numInPoint].x = seedTmp.pptSeed[m].x;
							pTmpPt[numInPoint++].y = seedTmp.pptSeed[m].y;
						}
					}
					//����ϵĵ�����Ȩ��???
					//��Щ��ͬһ�����ߵ�Ȩ������
					vFitLine(pTmpPt, numInPoint, &dCoeffK1, &dCoeffB1, &bVert1);//��Ϊǰ���Ѿ����˳��������ﲻ���ٳ���
					//���fitting�����Ĳ�����һ��
					if(numInPoint1==0||(dCoeffK1!=dCoeffK || dCoeffB1!=dCoeffB || bVert!=bVert1))
					{
						if (numInPoint>numInPoint1)
						{
							bChanged = MTrue;
							dCoeffK = dCoeffK1;
							dCoeffB = dCoeffB1;
							bVert = bVert1;
							numInPoint1 = numInPoint;
							JMemCpy(inPoint, pTmpPt, numInPoint*sizeof(MPOINT));
						}
						else
						{
							//����з������������ô��ֱ�����������������������б��洢��list��ȥ
							if (numInPoint>=minNumPtOnLine)
							{
								randomPoint[2].x = pCircleInfo->circleParam.xcoord;
								randomPoint[2].y = pCircleInfo->circleParam.ycoord;
								distance = vPointDistToLine2(randomPoint[2], dCoeffK1, dCoeffB1, bVert1);
								if (distance > pCircleInfo->circleParam.lRadius*3 /4 || lineSetInfo.lLineNum>=lineSetInfo.lMaxLineNum)continue;//��Բ�ĵľ��벻��ǿ��Ҫ��
								{
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint;
									for (l=0;l<numInPoint;l++)
									{
										lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.x=pTmpPt[l].x;
										lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.y=pTmpPt[l].y;
									}
									lineSetInfo.lLineNum++;
								}
							}
							bChanged = MFalse;
							break;
						}
					}
				}
				if (numInPoint1<minNumPtOnLine)continue;
				//������һ��ֱ��
				//��ֱ�ߺ�Բ��ƥ��Ƚ��е��飬��������⣬��������
				//����1:��Բ�ĵľ��벻��ǿ��Ҫ��
				//���������������,������ĺ���
				randomPoint[2].x = pCircleInfo->circleParam.xcoord;
				randomPoint[2].y = pCircleInfo->circleParam.ycoord;
				distance = vPointDistToLine2(randomPoint[2], dCoeffK, dCoeffB, bVert);
				if (distance > pCircleInfo->circleParam.lRadius || lineSetInfo.lLineNum>=lineSetInfo.lMaxLineNum)continue;//��Բ�ĵľ��벻��ǿ��Ҫ��
				{
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint1;
					for (l=0;l<numInPoint1;l++)
					{
						lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.x=inPoint[l].x;
						lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.y=inPoint[l].y;
					}
					lineSetInfo.lLineNum++;
				}
			}
		}
		//��ÿ��circle_info��⵽��ֱ�߼��Ͻ��кϲ�������confidence�ɸߵ��͵�Rank
		//1.�ϲ��Ĺ���:�غϵ��Ⱥϲ�
		//confidence�������֮�������ϵ�������Ӧֵ�Ĺ�һ��
		MergeSet((MVoid*)(lineSetInfo.pLineInfo), sizeof(LINE_INFO), &(lineSetInfo.lLineNum), LineInfoCmp, LineInfoReplace);
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			//�Ե���������γ���β�����ĵ���б�
			//����ÿ���߶ε�confidence������һ��
			MLong lTotalSumNum,lSumNum1, lSumNum2, lSumNum3;
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
			lPTMeanSum1= 0, lPTMeanSum2=0, lPTMeanSum3=0;
			lMeanSum = 0;			
			//��β����,���������ʱ���ܹ��ýǶȣ����ֽǶ��ڹ�ԭ�㸽��
			//��ֱ��ʧЧ
			if(pLineInfo->lineParam.bVertical || (pLineInfo->lineParam.Coefk>1 || pLineInfo->lineParam.Coefk<-1))
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_Y));
			}
			else
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_X));
			}
			//������ѡ����β�����ģ����ǿ����м���ӵ�Ӱ��������ֵ�����ԱȽ�
			//��β�����ͼ��������Ĵ�С
			lTotalSumNum = 0;
			for(k=0; k<pLineInfo->lPtNum-2; k+=2)
			{
				lPTMeanSum1 =  vGetPointMeanValueBtwPt(pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint, 0xFF,
					&lSumNum1);
				lPTMeanSum1 *= lSumNum1;
				lPTMeanSum2 = vGetPointMeanValueBtwPt(pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k+1].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,
					&lSumNum2);
				lPTMeanSum2 *= lSumNum2;
				//ֱ��ȡ����֮���meanvalue
				lPTMeanSum3 = 0;
				//����֮��Ҫ����һ���Ĺ�ϵ������ϴ�Ķ۽�
				if(vComputeIntersactionAngle(pLineInfo->pPtinfo[k].ptPoint,pLineInfo->pPtinfo[k+1].ptPoint,pLineInfo->pPtinfo[k+2].ptPoint)>8*V_PI/9)//160��
				{
					lPTMeanSum3 = vGetPointMeanValueBtwPt(pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
						pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,&lSumNum3);
					lPTMeanSum3 *= lSumNum3;
				}
				if (lPTMeanSum3>lPTMeanSum1+lPTMeanSum2)
				{
					lMeanSum+=lPTMeanSum3;
					lTotalSumNum+=lSumNum3;
				}
				else
				{
					lMeanSum+=lPTMeanSum1+lPTMeanSum2;
					lTotalSumNum+=lSumNum1+lSumNum2;
				}
			}
			for (; k<pLineInfo->lPtNum-1;k++)
			{
				lPTMeanSum1=vGetPointMeanValueBtwPt(pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint,0xFF,
					&lSumNum1);
				lMeanSum+=lPTMeanSum1*lSumNum1;
				lTotalSumNum+=lSumNum1;
			}
			if (lTotalSumNum == 0)
				pLineInfo->lConfidence = 0;
			else
			{
				lMeanSum /= lTotalSumNum;
				pLineInfo->lConfidence = (MLong)(lMeanSum *(vDistance3L(pLineInfo->pPtinfo[0].ptPoint, pLineInfo->pPtinfo[k].ptPoint))
					/(2*pCircleInfo->circleParam.lRadius));
			}
			//if (lframeNum==157&&i==11)
			//Logger("frameNum[%d],circleNum[%d],lineNum[%d],confidence[%d](meansum[%d],num[%d],dist[%d])\n",lframeNum,i,j,pLineInfo->lConfidence,lMeanSum*lTotalSumNum,
			//	lTotalSumNum,(vDistance3L(pLineInfo->pPtinfo[0].ptPoint, pLineInfo->pPtinfo[k].ptPoint)));
			//��������confidence�ٳ�һ����һ����ֵ���볤���й�ϵ
		}
		GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
		/*if (lframeNum==157&&i==11)
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{	
		MChar path[256]={0};
		LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
		sprintf(path, ".\\log\\localCircle%dline%dconfi%d.bmp",i,j,pLineInfo->lConfidence);
		B_Cpy(&blockFlagTmp, pHaarResponse);
		vDrawCircle(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, blockFlagTmp.lWidth, blockFlagTmp.lHeight,
		pCircleInfo->circleParam.xcoord,pCircleInfo->circleParam.ycoord,pCircleInfo->circleParam.lRadius,255);
		vDrawLine2(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 
		255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb);
		PrintBmpEx(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, DATA_U8, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 1,path);
		//break;
		}*/
		//��ÿ��Բ��confidence:
		//ԭ��:Բ�ϵ�Ķ��:��Ӧֵ�Ĵ�С
		//���û��ֱ�ߣ����confidenceΪ0
		if (lineSetInfo.lLineNum == 0)
		{
			pCircleInfo->lConfidence = 0;
			pCircleInfo->lLineNum = 0;
		}
		else
		{
			//����ֱ�ߣ�ȷ��ֱ�ߵķ���
			//ֱ����Բ�Ľ���
			MPOINT ptCross1, ptCross2, ptMid;
			//			PTINFO *pCur1, *pCur2;
			//			MDouble dInterAngle, dMeanAngle;
			//			MLong lNum;
			res = vComputeCrossPt(pCircleInfo->circleParam.xcoord, pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius,
				lineSetInfo.pLineInfo[0].lineParam.Coefk,lineSetInfo.pLineInfo[0].lineParam.Coefb,lineSetInfo.pLineInfo[0].lineParam.bVertical,
				&ptCross1, &ptCross2);
			if (res<0) 
			{
				pCircleInfo->lConfidence = 0;
				pCircleInfo->lLineNum = 0;
				continue;
			}
			ptMid.x = (ptCross1.x+ptCross2.x)/2;
			ptMid.y = (ptCross1.y+ptCross2.y)/2;
			//��һ����Χ�ڵ���ֱ�ߵĽؾ࣬�����ڻ�ȡ����ģ����Ӧֵ
			lcircleNum = i;
			AdjustLinePosition(pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&lineSetInfo.pLineInfo[0].lineParam,	&ptMid, &ptCross1, &ptCross2);
			//����ֱ���ϴ�ģ����Ӧ��ķֲ� ������ֱ�߷���
			lPTMeanSum1 = vGetPointMeanValueBtwPt(pHaarAngleResponse->pBlockData, pHaarAngleResponse->lBlockLine, 
				pHaarAngleResponse->lWidth, pHaarAngleResponse->lHeight, ptCross1, ptMid, TEMPLATE_TYPE_BYTE_BIT,MNull);
			lPTMeanSum2 = vGetPointMeanValueBtwPt(pHaarAngleResponse->pBlockData, pHaarAngleResponse->lBlockLine, 
				pHaarAngleResponse->lWidth, pHaarAngleResponse->lHeight, ptCross2, ptMid, TEMPLATE_TYPE_BYTE_BIT,MNull);
			if (lPTMeanSum1<30 && lPTMeanSum2<30)
			{//���˼���õ���ֵ�������п��Ŷȣ���ѡ�����ĺ�����׼
				MLong xXisMean = 0, yXisMean=0;
				for (j=0; j<pCircleInfo->lPtNum; j++)
				{
					//����ֱ����ͶӰ���������ı�
					MPOINT ptProject;
					vProjectToLinePoint(lineSetInfo.pLineInfo[0].lineParam.Coefk,lineSetInfo.pLineInfo[0].lineParam.Coefb,
						lineSetInfo.pLineInfo[0].lineParam.bVertical,pCircleInfo->pPtinfo[j].ptPoint, &ptProject);
					//ͳ�����ı�
					xXisMean+=ptProject.x;
					yXisMean+=ptProject.y;
				}
				xXisMean/=pCircleInfo->lPtNum;
				yXisMean/=pCircleInfo->lPtNum;
				if( (xXisMean>MIN(ptCross1.x, ptMid.x) && xXisMean<MAX(ptCross1.x, ptMid.x))
					||( yXisMean>MIN(ptCross1.y,ptMid.y) && yXisMean<MAX(ptCross1.y,ptMid.y)))
					lineSetInfo.pLineInfo[0].lineParam.DAngle = vComputeAngle(ptCross1.x-ptMid.x, ptCross1.y-ptMid.y);
				else if ((xXisMean>MIN(ptCross2.x, ptMid.x) && xXisMean<MAX(ptCross2.x, ptMid.x))
					||( yXisMean>MIN(ptCross2.y,ptMid.y) && yXisMean<MAX(ptCross2.y,ptMid.y)))
					lineSetInfo.pLineInfo[0].lineParam.DAngle = vComputeAngle(ptCross2.x-ptMid.x, ptCross2.y-ptMid.y); 
			}
			else if (lPTMeanSum1<lPTMeanSum2)//����պ�Ҫ�෴����������ں�ˣ�С������ǰ��//����Ӧ���ǴӺ�˵�ǰ��
				lineSetInfo.pLineInfo[0].lineParam.DAngle = vComputeAngle(ptCross1.x-ptMid.x, ptCross1.y-ptMid.y);
			else
				lineSetInfo.pLineInfo[0].lineParam.DAngle = vComputeAngle(ptCross2.x-ptMid.x, ptCross2.y-ptMid.y); 

			lPTMeanSum1 = vGetPointMeanValueBtwPt(pHaarResponse->pBlockData, pHaarResponse->lBlockLine, 
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xff,MNull);
			FilterCirclePointList(hMemMgr, pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, pHaarResponse->lHeight, pCircleInfo, lineSetInfo.pLineInfo, ptMid,pParam->lMaxArcAngle);
			lMeanSum = 0;
			for (j=0; j<pCircleInfo->lPtNum; j++)
			{
				MByte *pCurHaarRsp = (MByte*)(pHaarResponse ->pBlockData)+pHaarResponse->lBlockLine*pCircleInfo->pPtinfo[j].ptPoint.y
					+pCircleInfo->pPtinfo[j].ptPoint.x;
				MByte *pCurHaarAngleRsp=(MByte*)(pHaarAngleResponse->pBlockData)+pHaarAngleResponse->lBlockLine*pCircleInfo->pPtinfo[j].ptPoint.y
					+pCircleInfo->pPtinfo[j].ptPoint.x;
				//if (*pCurHaarRsp2==*pCurHaarRsp)//˵���Ǵ�ģ�����Ӧ
				if (*pCurHaarAngleRsp & TEMPLATE_TYPE_BYTE_BIT)
				{
					lMeanSum+=*pCurHaarRsp;//��Ӧֵ�ӱ�
					pCircleInfo->pPtinfo[j].ptValue = 1;//��ʾ��ģ����Ϣ
				}
				else
				{
					lMeanSum+=*pCurHaarRsp;
					pCircleInfo->pPtinfo[j].ptValue = 0;//��ʾСģ����Ϣ
				}
			}
			pCircleInfo->lConfidence = lMeanSum;
			//��Ҫ����ֱ�ߵ�Ӱ��,��������Ҫͳһ��Բ�Ǻͣ�ֱ���Ǿ�ֵ������Ҫ��һ��,
			//����ʵ�ʵ������Ҫ����ֱ�ߵ�Ȩ��
			//pCircleInfo->lConfidence = pCircleInfo->lConfidence+lineSetInfo.pLineInfo[0].lConfidence*lineSetInfo.pLineInfo[0].lPtNum/3;
			pCircleInfo->lConfidence = pCircleInfo->lConfidence + lPTMeanSum1 *lineSetInfo.pLineInfo[0].lPtNum/3;
			pCircleInfo->lLineNum=1;
			//pCircleInfo->lineInfo.lConfidence=lineSetInfo.pLineInfo[0].lConfidence;
			pCircleInfo->lineInfo.lConfidence= lPTMeanSum1;
			pCircleInfo->lineInfo.lPtNum =lineSetInfo.pLineInfo[0].lPtNum;
			JMemCpy(&(pCircleInfo->lineInfo.lineParam),&(lineSetInfo.pLineInfo[0].lineParam), sizeof(LINEPARAM));
			JMemCpy(pCircleInfo->lineInfo.pPtinfo, lineSetInfo.pLineInfo[0].pPtinfo,sizeof(PTINFO)*lineSetInfo.pLineInfo[0].lPtNum);

			if (lPTMeanSum1<40)
			{
				pCircleInfo->lConfidence = 0;
				pCircleInfo->lLineNum = 0;
				continue;
			}
			if (lMaxCircleRadius<pCircleInfo->circleParam.lRadius)
				lMaxCircleRadius = pCircleInfo->circleParam.lRadius;
		}
	}
	JPrintf("---2");
	//��������������reweightÿ��Բ��confidence
	/*for (i=0; i<pCircleSetInfo->lParamNum; i++)
	{
	pCircleSetInfo->pCircleInfo[i].lConfidence = (MLong)pCircleSetInfo->pCircleInfo[i].lConfidence 
	*GaussianWeight(lMaxCircleRadius/8*1.1774, lMaxCircleRadius-pCircleSetInfo->pCircleInfo[i].circleParam.lRadius);
	}*/

	GO(QuickSort(hMemMgr, (MVoid *)(pCircleSetInfo->pCircleInfo), pCircleSetInfo->lParamNum, 
		sizeof(CIRCLE_INFO), CircleInfoConfidenceCmp));


	//if (lframeNum == 638)
	/*for (i=0; i<pCircleSetInfo->lParamNum; i++)
	{
	MPOINT pt1, pt2;
	MChar path[256]={0};
	CIRCLE_INFO *pCircleInfo = pCircleSetInfo->pCircleInfo+ i;
	sprintf(path, ".\\log\\frame[%d]localCircle%dconfi%d.bmp",lframeNum,i,pCircleInfo->lConfidence);
	B_Cpy(&blockFlagTmp, pHaarResponse);
	//B_Set(&blockFlagTmp, 0);
	vDrawCircle(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, blockFlagTmp.lWidth, blockFlagTmp.lHeight,
	pCircleInfo->circleParam.xcoord,pCircleInfo->circleParam.ycoord,pCircleInfo->circleParam.lRadius,255);
	vDrawLine2(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 
	255, pCircleInfo->lineInfo.lineParam.Coefk, pCircleInfo->lineInfo.lineParam.Coefb);
	PrintBmpEx(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, DATA_U8, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 1,path);
	//break;
	}*/

	//����ѵ�Բ���е�����
	if (pCircleSetInfo->lParamNum>0)
	{
		CIRCLE_INFO *pCircleInfo = pCircleSetInfo->pCircleInfo;
		MLong lCurPos;
		MLong lMaxDistance;
		for (i=0; i<pCircleSetInfo->pCircleInfo[0].lPtNum; i++)
		{
			MPOINT *pPtTmp = &(pCircleSetInfo->pCircleInfo[0].pPtinfo[i].ptPoint);
			pCircleSetInfo->pCircleInfo[0].pPtinfo[i].dPtAngle
				= vComputeAngle(pPtTmp->x-pCircleInfo->circleParam.xcoord, pPtTmp->y-pCircleInfo->circleParam.ycoord);
		}
		//look for an anchor��������Ϊ��ʼ�ĵ���ê��	
		//�������ڵ�ļ����󣬼ٶ�������˳ʱ����ģ����������������
		GO(QuickSort(hMemMgr, (MVoid*)pCircleInfo->pPtinfo, pCircleInfo->lPtNum, sizeof(PTINFO),PTInfoCmp_Ascend));
		lCurPos = 0;lMaxDistance=0;
		for (i=1;i<pCircleInfo->lPtNum;i++)
		{
			MLong lCurDist = vDistance1L(pCircleInfo->pPtinfo[i].ptPoint, pCircleInfo->pPtinfo[i-1].ptPoint);
			if (lMaxDistance<lCurDist)
			{lMaxDistance=lCurDist;lCurPos=i;}
		}
		if (lMaxDistance<vDistance1L(pCircleInfo->pPtinfo[0].ptPoint, pCircleInfo->pPtinfo[i-1].ptPoint))
		{
			lMaxDistance = vDistance1L(pCircleInfo->pPtinfo[0].ptPoint, pCircleInfo->pPtinfo[i-1].ptPoint);
			lCurPos = 0;
		}
		//������һ���򣬽���line�Ŀռ�������
		if (lCurPos!=0)
		{
			JMemCpy(lineSetInfo.pLineInfo[0].pPtinfo, pCircleInfo->pPtinfo+lCurPos,sizeof(PTINFO)*pCircleInfo->lPtNum-lCurPos);
			JMemCpy(lineSetInfo.pLineInfo[0].pPtinfo+pCircleInfo->lPtNum-lCurPos,pCircleInfo->pPtinfo, sizeof(PTINFO)*lCurPos);
			JMemCpy(pCircleInfo->pPtinfo, lineSetInfo.pLineInfo[0].pPtinfo, sizeof(PTINFO)*pCircleInfo->lPtNum);
		}
	}

EXT:	
	B_Release(hMemMgr, &blockFlagTmp2);
	FreeLineSet(hMemMgr, &lineSetInfo);
	FreeVectMem(hMemMgr, seedTmp.pcrSeed);
	FreeVectMem(hMemMgr, seedTmp.pptSeed);
	FreeVectMem(hMemMgr, pTmpPt);FreeVectMem(hMemMgr, pAngleValue);
	timeElapsed = JGetCurrentTime()-timeStart;
	JAddToTimer(timeElapsed, TIMER_RANSACCIRCLE_LINE);
	return res;
}


//lRadiusCons�ǲ���������Լ���뾶��С
//RANSAC��ʱ��Ҫ������Ӧ�Ƕȣ�ģʽ�Ƿ���Ӧ���������Ĵ�ֱ��
MRESULT RANSAC_LINE(MHandle hMemMgr, BLOCK *pHaarResponse,
	BLOCK* pHaarAngleResponse, MPOINT *pPtList, MLong lPtNum, 
	MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;
	MLong j, k, l, m,n;
	MPOINT randomPoint[3];
	MPOINT *inPoint=pTemp;
	MByte lTmpMax=0;
	MLong numInPoint=0;
	MDouble distance;
	MLong dis=0,disTmp=0;
	MLong minNumPtOnLine = 2;	// 3? 2?
	MLong maxDist2Center;
	MDouble dCoeffK=0;
	MDouble dCoeffB=0;
	MBool bVert=MFalse;
	MDouble dCoeffK1=0;
	MDouble dCoeffB1 = 0;
	MBool bVert1=MFalse;
	MLong numInPoint1=0;
	MLong lPTMeanSum1,lPTMeanSum2,lPTMeanSum3;
	MLong lMeanSum;
	MPOINT tmpR={0};
	JGSEED seedTmp = {0};//Ŀ����Ϊ����ʱ�洢��Բ�ϵ�֮��������㣬�������ֱ��
	LINE_SET_INFO lineSetInfo = {0};
	LINEPARAM lineParam;
	CIRCLE tmpCircleParam;
	//	MLong lExternPtNum;

	MPOINT ptCross1, ptCross2, ptMid;
	MLong lResponseThres;
	MLong lIndex;

	MPOINT *pTmpPt = MNull;
	BLOCK blockFlagTmp1 = {0};
	if(lPtNum < 2)
	{
		Err_Print("lPtNum<2\n");
		res = LI_ERR_NO_FIND;
		return res;
	}
	AllocVectMem(hMemMgr, seedTmp.pptSeed, lPtNum, MPOINT);
	AllocVectMem(hMemMgr, seedTmp.pcrSeed, lPtNum, MByte);
	AllocVectMem(hMemMgr, pTmpPt, lPtNum, MPOINT);
	//	AllocVectMem(hMemMgr, pAngleValue, lPtNum, MLong);
	B_Create(hMemMgr, &blockFlagTmp1, DATA_U8,pHaarResponse->lWidth, pHaarResponse->lHeight);

	//����Բ��ѡ�㣬��ÿ����ѡ��Բ�����Ӧ �Ĵ�ָ��ֵ�����ܳ�Ϊһ������
	//��ģʽ��
	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));
	//	JPrintf("CreateLineSet~~~\n");
	{
		CIRCLE_INFO *pCircleInfo = pParam->pCircleInfo;
		JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
		seedTmp.lSeedNum = lPtNum;
		if(pCircleInfo->circleParam.xcoord !=-10000)
		{
			maxDist2Center = MAX(5, (pCircleInfo->circleParam.lRadius>>5)*7);	//1/4 ?  1/2 ?	1/8?

		}
		else
		{
			maxDist2Center = 0xFFFF;
			for(j=0;j<seedTmp.lSeedNum; j++)
			{
				tmpR.x +=seedTmp.pptSeed[j].x;
				tmpR.y +=seedTmp.pptSeed[j].y;
			}
			pCircleInfo->circleParam.xcoord = tmpR.x/ seedTmp.lSeedNum;
			pCircleInfo->circleParam.ycoord = tmpR.y/ seedTmp.lSeedNum;
		}
		randomPoint[2].x = pCircleInfo->circleParam.xcoord;
		randomPoint[2].y = pCircleInfo->circleParam.ycoord;
		JPrintf("center[%d,%d], %d\n", pCircleInfo->circleParam.xcoord, 
			pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius);	
		tmpCircleParam.xcoord = pCircleInfo->circleParam.xcoord;
		tmpCircleParam.ycoord = pCircleInfo->circleParam.ycoord;
		tmpCircleParam.lRadius = pCircleInfo->circleParam.lRadius ;
		//�Ժ�ѡ���б�������
		for (j=0; j<seedTmp.lSeedNum; j++)
		{
			randomPoint[0].x = seedTmp.pptSeed[j].x;
			randomPoint[0].y = seedTmp.pptSeed[j].y;
			for (k=j+1; k<seedTmp.lSeedNum; k++)
			{
				MBool bChanged;
				//��һ�����
				//��ϵ�������Ҫ����һ����Ҫ����Ӧ������һ�µ�
				//Ϊ�˲��ó��ֹ���ϣ���Ҫ����ϸ�о�һ��
				disTmp = sqrt((MDouble)((randomPoint[0].x- seedTmp.pptSeed[k].x)*(randomPoint[0].x- seedTmp.pptSeed[k].x)
					+ (randomPoint[0].y-seedTmp.pptSeed[k].y)*(randomPoint[0].y-seedTmp.pptSeed[k].y)));
				randomPoint[1].x = seedTmp.pptSeed[k].x;
				randomPoint[1].y = seedTmp.pptSeed[k].y;
				if(disTmp>= pParam->lCircleDist<<1)
					continue;	

				//���������ӵ㷽��Ҫ��ʵ�鷢�֣���ʱ��������������ָ���߸��ų�
				/*lAngleValue = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
				randomPoint[0].y+randomPoint[0].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
				randomPoint[1].y+randomPoint[1].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				dTmpAngle = ABS(lAngleValue - lAngleValue2) * ANGLE_STEP;
				if (dTmpAngle>10 && dTmpAngle<170)
				continue;*/
				if(vFitLine(randomPoint,2,&dCoeffK, &dCoeffB, &bVert)<0)
					continue;

				//һֱ��ϣ�ֱ��������������
				bChanged = MTrue;
				numInPoint1 = 0;
				while(bChanged)
				{
					//����ҪС����ϲ����������⣬����������ѭ��
					//���ȷʵ������������Ҫ��������ʹ��������
					//numInPointҪ�������������,Ӧ���������С
					bChanged = MFalse;
					numInPoint=0;
					for (m = 0 ; m<seedTmp.lSeedNum; m++)
					{
						//�㵽�ߵľ���
						distance = vPointDistToLine2(seedTmp.pptSeed[m], dCoeffK, dCoeffB, bVert);
						/*lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
						seedTmp.pptSeed[m].y+seedTmp.pptSeed[m].x) &(~TEMPLATE_TYPE_BYTE_BIT);
						dTmpAngle = ABS(lAngleValue-lAngleValue2) * ANGLE_STEP;
						if (dTmpAngle>10 && dTmpAngle<170)
						continue;*/
						if (distance < DISTANCE_THRESHOLD)
						{
							if(numInPoint>0)
							{
								dis = 0;
								for(n = 0;n<numInPoint;n++)
								{
									dis = sqrt((MDouble)(seedTmp.pptSeed[m].x-pTmpPt[n].x)*(seedTmp.pptSeed[m].x-pTmpPt[n].x)+
										(seedTmp.pptSeed[m].y-pTmpPt[n].y)*(seedTmp.pptSeed[m].y-pTmpPt[n].y));
									if(dis >= pParam->lCircleDist<<1)
										break;
								}
								if(n != numInPoint)
									continue;
							}

							pTmpPt[numInPoint].x = seedTmp.pptSeed[m].x;
							pTmpPt[numInPoint++].y = seedTmp.pptSeed[m].y;
						}
					}
					//����ϵĵ�����Ȩ��???
					//��Щ��ͬһ�����ߵ�Ȩ������
					vFitLine(pTmpPt, numInPoint, &dCoeffK1, &dCoeffB1, &bVert1);//��Ϊǰ���Ѿ����˳��������ﲻ���ٳ���

					//���fitting�����Ĳ�����һ�� numInPoint1����һֱ����0��
					if(numInPoint1==0||(dCoeffK1!=dCoeffK || dCoeffB1!=dCoeffB || bVert1!=bVert))
					{
						if (numInPoint>numInPoint1)
						{
							bChanged = MTrue;
							dCoeffK = dCoeffK1;
							dCoeffB = dCoeffB1;
							bVert = bVert1;
							numInPoint1 = numInPoint;
							JMemCpy(inPoint, pTmpPt, numInPoint*sizeof(MPOINT));
						}
						else
						{
							//����з������������ô��ֱ�����������������������б��洢��list��ȥ
							if (numInPoint>=minNumPtOnLine)
							{
								distance = vPointDistToLine2(randomPoint[2], dCoeffK1, dCoeffB1, bVert1);
								if (distance<maxDist2Center && lineSetInfo.lLineNum<lineSetInfo.lMaxLineNum)
								{
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert1;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
									lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint;
									for (l=0;l<numInPoint;l++)
									{
										lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.x=pTmpPt[l].x;
										lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.y=pTmpPt[l].y;
									}
									lineSetInfo.lLineNum++;
								}
							}
							bChanged = MFalse;
							break;
						}					
					}
				}
				if (numInPoint1<minNumPtOnLine)
					continue;
				//������һ��ֱ��
				//��ֱ�ߺ�Բ��ƥ��Ƚ��е��飬��������⣬��������
				//����1:��Բ�ĵľ��벻��ǿ��Ҫ��
				//���������������,������ĺ���
				distance = vPointDistToLine2(randomPoint[2], dCoeffK, dCoeffB, bVert);
				if (distance < maxDist2Center &&  lineSetInfo.lLineNum < lineSetInfo.lMaxLineNum)
				{
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint1;
					for (l=0;l<numInPoint1;l++)
					{
						lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.x=inPoint[l].x;
						lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint.y=inPoint[l].y;
					}
					lineSetInfo.lLineNum++;
				}
			}
		}

		//��ÿ��circle_info��⵽��ֱ�߼��Ͻ��кϲ�������confidence�ɸߵ��͵�Rank
		//1.�ϲ��Ĺ���:�غϵ��Ⱥϲ�
		//confidence�������֮�������ϵ�������Ӧֵ�Ĺ�һ��
		MergeSet((MVoid*)(lineSetInfo.pLineInfo), sizeof(LINE_INFO), &(lineSetInfo.lLineNum), LineInfoCmp, LineInfoReplace);

		//��ÿ��ֱ�߶ε�confidence:
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			//�Ե���������γ���β�����ĵ���б�
			//����ÿ���߶ε�confidence������һ��
			MLong lTotalSumNum,lSumNum1, lSumNum2, lSumNum3;
			MLong lTotalPtNum, lPtNum1, lPtNum2, lPtNum3; 
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
			lPTMeanSum1= 0, lPTMeanSum2=0, lPTMeanSum3=0;
			lMeanSum = 0;	

			if(pLineInfo->lineParam.bVertical || (pLineInfo->lineParam.Coefk>1 || pLineInfo->lineParam.Coefk<-1))
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_Y));
				//				JPrintf("1742 PTInfoCmp_Y \n");
			}
			else
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_X));
				//				JPrintf("1747 PTInfoCmp_X \n");
			}
/*
			{	
				MChar path[256]={0};
				sprintf(path, ".\\log\\line\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
*/
			//������ѡ����β�����ģ����ǿ����м���ӵ�Ӱ��������ֵ�����ԱȽ�
			//��β�����ͼ��������Ĵ�С
			lTotalSumNum = 0;
			lTotalPtNum = 0;
			for(k=0; k<pLineInfo->lPtNum-2; k+=2)
			{
				lPTMeanSum1 =  vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint, 0xFF,
					&lSumNum1, &lPtNum1,pParam->lCircleDist<<1);//sumnum��ˮƽ��ֱ�����ϵľ���  
				lPTMeanSum1 *= lSumNum1;
				lPTMeanSum2 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k+1].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,
					&lSumNum2, &lPtNum2,pParam->lCircleDist<<1);
				lPTMeanSum2 *= lSumNum2;

				//ֱ��ȡ����֮���meanvalue
				lPTMeanSum3 = 0;
				//����֮��Ҫ����һ���Ĺ�ϵ������ϴ�Ķ۽�
				if(vComputeIntersactionAngle(pLineInfo->pPtinfo[k].ptPoint,pLineInfo->pPtinfo[k+1].ptPoint,pLineInfo->pPtinfo[k+2].ptPoint)>8*V_PI/9)//160��
				{
					lPTMeanSum3 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,&lSumNum3, &lPtNum3,pParam->lCircleDist<<1);
					lPTMeanSum3 *= lSumNum3;
				}
				if (lPTMeanSum3>lPTMeanSum1+lPTMeanSum2)
				{
					lMeanSum += lPTMeanSum3;
					lTotalSumNum += lSumNum3;
					lTotalPtNum += lPtNum3;
				}
				else
				{
					lMeanSum += lPTMeanSum1+lPTMeanSum2;
					lTotalSumNum += lSumNum1+lSumNum2;
					lTotalPtNum += lPtNum1 + lPtNum2;
				}
			}
			for (; k<pLineInfo->lPtNum-1;k++)
			{
				lPTMeanSum1=vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint,0xFF,
					&lSumNum1, &lPtNum1,pParam->lCircleDist<<1);
				lMeanSum += lPTMeanSum1*lSumNum1;
				lTotalSumNum += lSumNum1;
				lTotalPtNum += lPtNum1;
			}

			lMeanSum /= lTotalSumNum;
			//			printf("lPtNum=%d\n", lTotalPtNum);

			// ��������ָ������Բ�Ľ���
			calcLineCrossCirclePt(&pCircleInfo->circleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			ptMid.x = (ptCross1.x + ptCross2.x)>>1;
			ptMid.y = (ptCross1.y + ptCross2.y)>>1;
			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&pLineInfo->lineParam, &ptMid, &ptCross1, &ptCross2); //�����������Ӧֵ����΢��

			// ����΢��������ֱ����Բ�Ľ���
			calcLineCrossCirclePt(&tmpCircleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, pHaarResponse->lWidth, 
				pHaarResponse->lHeight, ptCross1, ptCross2,0xFF,&lSumNum1, &lTotalPtNum,pParam->lCircleDist<<1);
			// ����ú�ѡֱ���� ����������Ӧֵ��������󳤶�  ���ں����Բ��ֲ��������Ŷȸߵ�ֱ�ߵ��ų�
			pLineInfo->lMaxLength = calcMaxContinuityLength((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xFF);

			pLineInfo->lConfidence = lMeanSum * lTotalPtNum / pCircleInfo->circleParam.lRadius;

			//			printf("index=%d, line:k=%.4f, b=%.4f, con=%d, sum=%d, lPtNum=%d, lPtNum2=%d\n", j, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb,
			//					pLineInfo->lConfidence, lMeanSum, lTotalPtNum, lTotalSumNum);
		}
		GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
		//���û��ֱ�ߣ����confidenceΪ0
		if (lineSetInfo.lLineNum == 0 && pParam->lDoubleRedLine==0)
		{
			pCircleInfo->lConfidence = 0;
			pCircleInfo->lLineNum = 0;
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}
		else if(lineSetInfo.lLineNum == 0 && pParam->lDoubleRedLine==1)
		{
			LINE_INFO *pLineInfo = &(pCircleInfo->lineInfo);
			JPrintf("Two Red Line,The Same Line\n");
			goto EXT;
		}

		lResponseThres = lineSetInfo.pLineInfo[0].lConfidence * 3 / 5;
		GO(QuickSort(hMemMgr, (MVoid*)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoLengthCmp));
		lIndex = 0;
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
/*
			{	
				MChar path[256]={0};
				sprintf(path, ".\\log\\line\\line2\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
*/
			if (pLineInfo->lConfidence >= lResponseThres)
			{
				lIndex = j;
				break;
			}
		}
		//����ֱ�ߣ�ȷ��ֱ�ߵķ���
		//ֱ����Բ�Ľ���
		{
			//LINEPARAM lineParam;

			//			res = vComputeCrossPt(pCircleInfo->circleParam.xcoord, pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius,
			//									lineSetInfo.pLineInfo[0].lineParam.Coefk,lineSetInfo.pLineInfo[0].lineParam.Coefb,lineSetInfo.pLineInfo[0].lineParam.bVertical,
			//									&ptCross1, &ptCross2);
			calcLineCrossCirclePt(&pCircleInfo->circleParam, &lineSetInfo.pLineInfo[lIndex].lineParam, &ptCross1, &ptCross2);

			ptMid.x = (ptCross1.x+ptCross2.x)>>1;
			ptMid.y = (ptCross1.y+ptCross2.y)>>1;

			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&lineSetInfo.pLineInfo[lIndex].lineParam,
				&ptMid, &ptCross1, &ptCross2);
			calcLineCrossCirclePt(&pCircleInfo->circleParam, &lineSetInfo.pLineInfo[lIndex].lineParam, &ptCross1, &ptCross2);
			//			printf("adjust cross1(%d,%d), cross2(%d,%d)\n", ptCross1.x, ptCross1.y, ptCross2.x, ptCross2.y);

			lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, 
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xff,MNull);

			if (lPTMeanSum1<=0)
			{
				pCircleInfo->lConfidence = 0;
				pCircleInfo->lLineNum = 0;
				printf("RANSAC LINE lPTMeanSum<30~~\n");
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}

			if((ptCross1.x==ptMid.x && ptCross1.y==ptMid.y) || (ptCross2.x==ptMid.x && ptCross2.y==ptMid.y))
			{
				printf("ptCross1=ptMid || ptCross2=ptMid err\n");
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}

			lineSetInfo.pLineInfo[lIndex].lineParam.ptStart = ptCross1;
			lineSetInfo.pLineInfo[lIndex].lineParam.ptEnd = ptCross2;
			lineSetInfo.pLineInfo[lIndex].lineParam.ptMid = ptMid;
			lineParam = lineSetInfo.pLineInfo[lIndex].lineParam;

			pCircleInfo->lLineNum=1;
			pCircleInfo->lineInfo.lConfidence= lPTMeanSum1;
			pCircleInfo->lineInfo.lPtNum =lineSetInfo.pLineInfo[lIndex].lPtNum;
			JMemCpy(&(pCircleInfo->lineInfo.lineParam),&(lineParam), sizeof(LINEPARAM));
			JMemCpy(pCircleInfo->lineInfo.pPtinfo, lineSetInfo.pLineInfo[lIndex].pPtinfo,sizeof(PTINFO)*lineSetInfo.pLineInfo[lIndex].lPtNum);
			JPrintf("k=%f, b=%f\n", lineParam.Coefk, lineParam.Coefb);
/*
			{	
				MChar path[256]={0};
				LINE_INFO *pLineInfo = &(pCircleInfo->lineInfo);
				sprintf(path, ".\\log\\%d_line.bmp",gPictureNum);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
*/
		}

	}

EXT:	
	B_Release(hMemMgr, &blockFlagTmp1);
	FreeLineSet(hMemMgr, &lineSetInfo);
	FreeVectMem(hMemMgr, seedTmp.pcrSeed);
	FreeVectMem(hMemMgr, seedTmp.pptSeed);
	FreeVectMem(hMemMgr, pTmpPt);

	return res;
}

//=============================================================================
// �����ж�ָ���������յ�
// 1. �ж�ָ�����е��Ƿ���ͼ���ڲ����������ͼ���ڣ�����Ϊָ���߲��������ؼ���ʧ�ܣ����򣬽���2
// 2. ���е�Ϊ��ʼ�㣬�ֱ���ֱ�������ۻ�������Ӧֵ����������һ��ģ�ָ���յ���ָ����ƫ��������Ӧֵ���������϶ࣩ
// 3. ���һ����Ӧֵ��������Զ������һ�ࣨ�˴�ȡ1.5����������Ϊ�Ѿ�����Ч�ж�ָ�����յ㣬�������أ����򣬽���4
// 4. ���е�Ϊ��㣬���α�������ָ�����ϵĵ㣬�ֱ�����ÿ������ָ�봹ֱ��������Ӧֵ����������������Ϊָ���߿�ȣ�
// 5. һ��أ�����ָ������ʼ��Ͽ�ָ���յ����Խ��Խϸ������4�м��������ָ���߿�ȣ���Ȩ�ж������յ�
MRESULT CalcLineEndPt_xsd(MHandle hMemMgr, BLOCK *pImage,BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold, LINEPARAM *lineParam)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	MLong lMaxLength, lMaxWidth, lWidthStep;
	MPOINT ptLeft, ptRight, ptMid, ptTmp;
	MLong leftLength, rightLength, minLength;
	MLong leftWidthConfidence, rightWidthConfidence;
	MLong ptLeftConfidence, ptRightConfidence;
	MDouble dCoeffK, dCoeffB;
	MBool bVert;
	MByte *leftVal, *rightVal, *pData;
	MLong lWidth, lHeight, lStride;
	MLong leftX, leftY, rightX, rightY;

	// make sure input info is right
	if (MNull==pHaarResponse || MNull==lineParam || 10>lRadius || 0>=lThreshold)
	{
		res = LI_ERR_UNKNOWN;
		return res;
	}

	//init
	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lStride = pHaarResponse->lBlockLine;
	pData = (MByte*)pHaarResponse->pBlockData;
	dCoeffK = lineParam->Coefk;
	dCoeffB = lineParam->Coefb;
	bVert = lineParam->bVertical;

	lMaxWidth = MAX(lRadius>>4, 10);		// r/16
	lWidthStep = lRadius>>1;		// r/2
	leftLength = rightLength = minLength = 0;
	leftWidthConfidence = rightWidthConfidence = 0;
	ptLeftConfidence = ptRightConfidence = 0;
	ptLeft = lineParam->ptStart;
	ptRight = lineParam->ptEnd;
	ptMid = lineParam->ptMid;
	leftVal = rightVal = MNull;

	// 2015.11.02
	if (1 != isPtInImage(ptMid, lWidth, lHeight))
	{
		if (1==isPtInImage(lineParam->ptStart, lWidth, lHeight) && 1!=isPtInImage(lineParam->ptEnd, lWidth, lHeight))
		{
			ptTmp = lineParam->ptStart;
			lineParam->ptStart = lineParam->ptEnd;
			lineParam->ptEnd = ptTmp;
		}
		return res;
	}

	lMaxLength = (3*lRadius)>>2;	// (3/4)r ���lMaxlength�ǲ��Ƕ��һ��
 
	if (bVert)
	{
		if(ptLeft.y>ptRight.y)
		{
			ptTmp = ptLeft;
			ptLeft = ptRight;
			ptRight = ptTmp;
		}
	}
	else
	{
		if(dCoeffK<1.0 && dCoeffK>-1.0)
		{			
			if (ptLeft.x > ptRight.x)
			{
				ptTmp = ptLeft;
				ptLeft = ptRight;
				ptRight = ptTmp;
			}
			lMaxLength = MIN(abs((MDouble)(ptMid.x-ptLeft.x)),abs((MDouble)(ptRight.x-ptMid.x)));
		}
		else
		{
			if (ptLeft.y > ptRight.y)
			{
				ptTmp = ptLeft;
				ptLeft = ptRight;
				ptRight = ptTmp;
			}
			lMaxLength = MIN(abs((MDouble)(ptMid.y-ptLeft.y)),abs((MDouble)(ptRight.y-ptMid.y)));
		}
	}
	// calc the length from ptMid
	if (ptLeft.x==ptRight.x)		// chuizhi
	{
		leftX = ptMid.x;
		leftY = ptMid.y - 1;
		rightX = ptMid.x;
		rightY = ptMid.y + 1;
		lMaxLength = MIN(lMaxLength, (lHeight>>1)-1);	// make sure leftVal/rightVal in image size
		for (i=0; i<lMaxLength; i++, leftY--, rightY++)	
		{
			if (leftY>=0 && leftY<lHeight)
			{
				leftVal = pData + leftY * lStride + leftX;
				if (*leftVal >= lThreshold)		
					leftLength++;
			}
			if (rightY>=0 && rightY<lHeight)
			{
				rightVal = pData + rightY * lStride + rightX;
				if(*rightVal >= lThreshold)
					rightLength++;
			}	
		}
	}
	else if (ptLeft.y==ptRight.y)		// shuiping
	{
		leftX = ptMid.x - 1;
		leftY = ptMid.y;
		rightX = ptMid.x + 1;
		rightY = ptMid.y;
		lMaxLength = MIN(lMaxLength, (lWidth>>1)-1);
		for (i=0; i<lMaxLength; i++, leftX--, rightX++)	
		{
			if (leftX>=0 && leftX<lWidth)
			{
				leftVal = pData + leftY * lStride + leftX;
				if (*leftVal >= lThreshold)		
					leftLength++;
			}
			if (rightX>=0 && rightX<lWidth)
			{
				rightVal = pData + rightY * lStride + rightX;
				if(*rightVal >= lThreshold)
					rightLength++;
			}
		}
	}
	else
	{
		//printf("-------------------x------------\n");
		//printf("***ptMid.x=%d,lMaxLength=%d,lThreshold=%d\n",ptMid.x,lMaxLength,lThreshold);
		if (dCoeffK<1.0 && dCoeffK>-1.0)	// x
		{
			leftX = ptMid.x - 1;
			rightX = ptMid.x + 1;
			for (i=0; i<lMaxLength; i++, leftX--, rightX++)	
			{
				leftY = (MLong)(dCoeffK * leftX + dCoeffB);
				rightY = (MLong)(dCoeffK * rightX + dCoeffB);
				if (leftX>=0 && leftX<lWidth && leftY>=0 && leftY<lHeight)
				{
					leftVal = pData + leftY * lStride + leftX;
					if (*leftVal >= lThreshold)
						leftLength++;
				}
				if (rightX>=0 && rightX<lWidth && rightY>=0 && rightY<lHeight)
				{
					rightVal = pData + rightY * lStride + rightX;
					if(*rightVal >= lThreshold)
						rightLength++;
				}
			}
		}
		else	// y
		{
			leftY = ptMid.y - 1;
			rightY = ptMid.y + 1;
			for (i=0; i<lMaxLength; i++, leftY--, rightY++)
			{
				leftX = (MLong)((leftY - dCoeffB) / dCoeffK);
				rightX = (MLong)((rightY - dCoeffB) / dCoeffK);
				if (leftX>=0 && leftX<lWidth && leftY>=0 && leftY<lHeight)
				{
					leftVal = pData + leftY * lStride + leftX;
					if (*leftVal >= lThreshold)
						leftLength++;
				}
				if (rightX>=0 && rightX<lWidth && rightY>=0 && rightY<lHeight)
				{
					rightVal = pData + rightY * lStride + rightX;
					if(*rightVal >= lThreshold)
						rightLength++;
				}
			}
		}
	}
	JPrintf("******lRadius=%d, lLength=%d, rLength=%d\n", lRadius, leftLength, rightLength);
	//printf("******lRadius=%d, lLength=%d, rLength=%d\n", lRadius, leftLength, rightLength);
	minLength = MIN(leftLength, rightLength);

	if (leftLength >= (rightLength * 3 / 2) || (leftLength * 3 / 2) <= rightLength)
	{
		leftWidthConfidence = rightWidthConfidence = 0;
		goto CALC_CONFIDENCE;
	}
	else
	{
		//printf("-------------------CalcLineEndPtStep2------------\n");
		GO(CalcLineEndPtStep2(hMemMgr,pImage, pHaarResponse, lRadius, lThreshold, *lineParam,
			minLength, &leftWidthConfidence, &rightWidthConfidence));
	}

CALC_CONFIDENCE:
	ptLeftConfidence = (leftLength<<1) + leftWidthConfidence;
	ptRightConfidence = (rightLength<<1) + rightWidthConfidence;
	JPrintf("******leftCon=%d, rightCon=%d\n", ptLeftConfidence, ptRightConfidence);
	//printf("******leftLength=%d, rightLength=%d\n", leftLength, rightLength);
	//printf("******leftWidthConfidence=%d, rightWidthConfidence=%d\n", leftWidthConfidence, rightWidthConfidence);
	//printf("******leftCon=%d, rightCon=%d\n", ptLeftConfidence, ptRightConfidence);
	if (ptLeftConfidence>ptRightConfidence)
	{
		lineParam->ptEnd = ptLeft;
		lineParam->ptStart = ptRight;
	}
	else if (ptLeftConfidence<ptRightConfidence)
	{
		lineParam->ptEnd = ptRight;
		lineParam->ptStart = ptLeft;
	}
	else
	{
		if (leftLength > rightLength)	// �߳����Ŷ�Ȩ�ش����߿�Ȩ��
		{
			lineParam->ptEnd = ptLeft;
			lineParam->ptStart = ptRight;
		}
		else
			res = LI_ERR_UNKNOWN;
	}

EXT:
	return res;
}
MLong ValueCmptmp( const MVoid* p1, const MVoid* p2)
{
	MByte _p1 = *(MByte*)p1;
	MByte _p2 = *(MByte*)p2;

	if( _p1 < _p2)
		return 1;
	if (_p1 > _p2)
		return -1;
	return 0;
}
// �ֱ������е�����ָ���߿��

MRESULT CalcLineEndPtStep2(MHandle hMemMgr, BLOCK *pImage, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold,
	LINEPARAM lineParam, MLong minLength, MLong *leftWidthConfidence, MLong *rightWidthConfidence)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lMaxWidth, lWidthStep, lWidthRange;
	MPOINT ptLeft, ptRight, ptMid, ptTmp;
	MLong *leftWidthArrray, *rightWidthArray;
	MDouble dCoeffK, dCoeffB, dCoeffKTmp, dCoeffBTmp;
	MBool bVert;
	MByte *leftVal, *rightVal, *pData,*pDatagray;
	MByte *tmpVal1, *tmpVal2, *tmpVala, *tmpValb, *tmpValo;
	MLong lWidth, lHeight, lStride;
	MLong leftX, leftY, rightX, rightY, tmpX, tmpY, tmpX2, tmpY2, tmpXp, tmpYp,tmpXq, tmpYq, tmpXr, tmpYr;
	MLong tmpOff;
	MDouble lPercentage;
	MLong cmpflag;

	//init
	cmpflag=0;
	lPercentage=0.16;
	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lStride = pHaarResponse->lBlockLine;
	pData = (MByte*)pHaarResponse->pBlockData;
	pDatagray = (MByte*)pImage->pBlockData;
	dCoeffK = lineParam.Coefk;
	dCoeffB = lineParam.Coefb;
	bVert = lineParam.bVertical;	
	lMaxWidth = MAX(lRadius>>4, 10);		// r/16		
	lWidthStep = lRadius>>1;		// r/2
	/*printf("lRadius=%d\n",lRadius);
	printf("lMaxWidth=%d\n",lMaxWidth);*/
	ptLeft = lineParam.ptStart;
	ptRight = lineParam.ptEnd;
	ptMid = lineParam.ptMid;
	leftWidthArrray = rightWidthArray = MNull;
	leftVal = rightVal = MNull;
	
	//PrintBmpEx(pImage->pBlockData, pImage->lBlockLine, DATA_U8, pImage->lWidth, pImage->lHeight, 1, "D:/test123.bmp");
	if (ptMid.x<0 || ptMid.x>=lWidth || ptMid.y<0 || ptMid.y>=lHeight)
	{
		*leftWidthConfidence = *rightWidthConfidence = 0;
		res = LI_ERR_NOT_INIT;
		return res;
	}

	if (bVert)
	{
		if(ptLeft.y>ptRight.y)
		{
			ptTmp = ptLeft;
			ptLeft = ptRight;
			ptRight = ptTmp;
		}
	}
	else
	{
		if(dCoeffK<1.0 && dCoeffK>-1.0)
		{
			if (ptLeft.x > ptRight.x)
			{
				ptTmp = ptLeft;
				ptLeft = ptRight;
				ptRight = ptTmp;
			}
		}
		else
		{
			if (ptLeft.y > ptRight.y)
			{
				ptTmp = ptLeft;
				ptLeft = ptRight;
				ptRight = ptTmp;
			}
		}
	}

	// calc both width
	AllocVectMem(hMemMgr, leftWidthArrray, lRadius, MLong);
	AllocVectMem(hMemMgr, rightWidthArray, lRadius, MLong);
	if (MNull==leftWidthArrray || MNull==rightWidthArray)
	{
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}
	SetVectZero(leftWidthArrray, lRadius*sizeof(MLong));
	SetVectZero(rightWidthArray, lRadius*sizeof(MLong));

	lWidthStep = MIN(minLength, lWidthStep);

	// calc both width array
	if (ptLeft.x==ptRight.x)
	{
		leftX = ptMid.x;
		leftY = ptMid.y - 1;
		rightX = ptMid.x;
		rightY = ptMid.y + 1;
		lWidthStep = MIN(lWidthStep, (lHeight>>1)-1);
		lMaxWidth = MIN(lMaxWidth, (lWidth>>1)-1);
		pData = (MByte*)pHaarResponse->pBlockData;
		for (i=0; i<lWidthStep; i++, leftY--, rightY++)	
		{
			if (leftY>=0 && leftY<lHeight)
			{
				tmpX = leftX - 1;
				tmpY = leftY;
				tmpX2 = leftX + 1;
				tmpY2 = leftY;

				tmpValo = pData + leftY * lStride + leftX;//��׼����ֵ����Ӧͼ�г�ʼ��
				tmpXp = leftX;
				tmpYp = leftY;
				tmpXq = tmpX;
				tmpYq = tmpY;
				tmpXr = tmpX2;
				tmpYr = tmpY2;
				for (j = 0; j < lMaxWidth; j++, tmpXq--, tmpXr++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
				{
					if (tmpXq >= 0 && tmpXq<lWidth)
					{
						tmpVal1 = pData + tmpYq * lStride + tmpXq;
						if (*tmpVal1 > *tmpValo)
						{
							tmpValo = pData + tmpYq * lStride + tmpXq;
							tmpXp = tmpXq;
							tmpYp = tmpYq;
						}
					}
					if (tmpXr >= 0 && tmpXr<lWidth)
					{
						tmpVal2 = pData + tmpYr * lStride + tmpXr;
						if (*tmpVal2 > *tmpValo)
						{
							tmpValo = pData + tmpYr * lStride + tmpXr;
							tmpXp = tmpXr;
							tmpYp = tmpYr;
						}
					}
				}
				tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ

				leftVal = pData + leftY * lStride + leftX;
				tmpVala = pDatagray + leftY *  pImage->lBlockLine + leftX;
				if (*leftVal >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					leftWidthArrray[i]++;
				for (j=0; j<lMaxWidth; j++, tmpX--, tmpX2++)
				{
					if (tmpX>=0 && tmpX<lWidth)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
						if (*tmpVal1>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							leftWidthArrray[i]++;
					}
					if (tmpX2>=0 && tmpX2<lWidth)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						tmpVala = pDatagray + tmpY2 *  pImage->lBlockLine + tmpX2;
						if(*tmpVal2>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							leftWidthArrray[i]++;
					}
				}
			}

			if (rightY>=0 && rightY<lHeight)
			{
				tmpX = rightX - 1;
				tmpY = rightY;
				tmpX2 = rightX + 1;
				tmpY2 = rightY;

				tmpValo = pData + rightY * lStride + rightX;//��׼����ֵ����Ӧͼ�г�ʼ��
				tmpXp = rightX;
				tmpYp = rightY;
				tmpXq = tmpX;
				tmpYq = tmpY;
				tmpXr = tmpX2;
				tmpYr = tmpY2;
				for (j = 0; j<lMaxWidth; j++, tmpXq--, tmpXr++)
				{
					if (tmpXq >= 0 && tmpXq<lWidth)
					{
						tmpVal1 = pData + tmpYq * lStride + tmpXq;
						if (*tmpVal1 > *tmpValo)
						{
							tmpValo = pData + tmpYq * lStride + tmpXq;
							tmpXp = tmpXq;
							tmpYp = tmpYq;
						}
					}
					if (tmpXr >= 0 && tmpXr<lWidth)
					{
						tmpVal2 = pData + tmpYr * lStride + tmpXr;
						if (*tmpVal2 > *tmpValo)
						{
							tmpValo = pData + tmpYr * lStride + tmpXr;
							tmpXp = tmpXr;
							tmpYp = tmpYr;
						}
					}
				}
				tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ
				
				rightVal = pData + rightY * lStride + rightX;
				tmpVala = pDatagray + rightY *  pImage->lBlockLine + rightX;
				if(*rightVal >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					rightWidthArray[i]++;
				for (j=0; j<lMaxWidth; j++, tmpX--, tmpX2++)
				{
					if (tmpX>=0 && tmpX<lWidth)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
						if (*tmpVal1>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							rightWidthArray[i]++;
					}
					if (tmpX2>=0 && tmpX2<lWidth)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						tmpVala = pDatagray + tmpY2 *  pImage->lBlockLine + tmpX2;
						if(*tmpVal2>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							rightWidthArray[i]++;
					}
				}
			}
		}
	}
	else if (ptLeft.y==ptRight.y)
	{
		leftX = ptMid.x - 1;
		leftY = ptMid.y;
		rightX = ptMid.x + 1;
		rightY = ptMid.y;
		lWidthStep = MIN(lWidthStep, (lWidth>>1)-1);
		lMaxWidth = MIN(lMaxWidth, (lHeight>>1)-1);
		pData = (MByte*)pHaarResponse->pBlockData;
		for (i=0; i<lWidthStep; i++, leftX--, rightX++)	
		{
			if (leftX>=0 && leftX<lWidth)
			{
				tmpX = leftX;
				tmpY = leftY - 1;
				tmpX2 = leftX;
				tmpY2 = leftY + 1;

				tmpValo = pData + leftY * lStride + leftX;//��׼����ֵ����Ӧͼ�г�ʼ��
				tmpXp = leftX;
				tmpYp = leftY;
				tmpXq = tmpX;
				tmpYq = tmpY;
				tmpXr = tmpX2;
				tmpYr = tmpY2;
				for (j = 0; j < lMaxWidth; j++, tmpYq--, tmpYr++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
				{
					if (tmpYq >= 0 && tmpYq<lHeight)
					{ 
						tmpVal1 = pData + tmpYq * lStride + tmpXq;
						if (*tmpVal1 > *tmpValo)
						{
							tmpValo= pData + tmpYq * lStride + tmpXq;
							tmpXp = tmpXq;
							tmpYp = tmpYq;
						}
					}
					if (tmpYr >= 0 && tmpYr < lHeight)
					{
						tmpVal2 = pData + tmpYr * lStride + tmpXr;
						if (*tmpVal2 > *tmpValo)
						{
							tmpValo = pData + tmpYr * lStride + tmpXr;
							tmpXp = tmpXr;
							tmpYp = tmpYr;
						}
					}
				}
				tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ

				leftVal = pData + leftY * lStride + leftX;
				tmpVala = pDatagray + leftY *  pImage->lBlockLine + leftX;
				if (*leftVal >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					leftWidthArrray[i]++;
				for (j=0; j<lMaxWidth; j++, tmpY--, tmpY2++)
				{
					if (tmpY>=0 && tmpY<lHeight)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
						if (*tmpVal1>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							leftWidthArrray[i]++;
					}
					if (tmpY2>=0 && tmpY2<lHeight)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						tmpVala = pDatagray + tmpY2 *  pImage->lBlockLine + tmpX2;
						if(*tmpVal2>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							leftWidthArrray[i]++;
					}
				}
			}
			if (rightX>=0 && rightX<lWidth)
			{
				tmpX = rightX;
				tmpY = rightY - 1;
				tmpX2 = rightX;
				tmpY2 = rightY + 1;

				tmpValo = pData + rightY * lStride + rightX;//��׼����ֵ����Ӧͼ�г�ʼ��
				tmpXp = rightX;
				tmpYp = rightY;
				tmpXq = tmpX;
				tmpYq = tmpY;
				tmpXr = tmpX2;
				tmpYr = tmpY2;
				for (j = 0; j < lMaxWidth; j++, tmpYq--, tmpYr++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
				{
					if (tmpYq >= 0 && tmpYq<lHeight)
					{
						tmpVal1 = pData + tmpYq * lStride + tmpXq;
						if (*tmpVal1 > *tmpValo)
						{
							tmpValo = pData + tmpYq * lStride + tmpXq;
							tmpXp = tmpXq;
							tmpYp = tmpYq;
						}
					}
					if (tmpYr >= 0 && tmpYr < lHeight)
					{
						tmpVal2 = pData + tmpYr * lStride + tmpXr;
						if (*tmpVal2 > *tmpValo)
						{
							tmpValo = pData + tmpYr * lStride + tmpXr;
							tmpXp = tmpXr;
							tmpYp = tmpYr;
						}
					}
				}
				tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ

				rightVal = pData + rightY * lStride + rightX;
				tmpVala = pDatagray + rightY *  pImage->lBlockLine + rightX;
				if (*rightVal >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					rightWidthArray[i]++;
				for (j=0; j<lMaxWidth; j++, tmpY--, tmpY2++)
				{
					if (tmpY>=0 && tmpY<lHeight)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
						if (*tmpVal1>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							rightWidthArray[i]++;
					}
					if (tmpY2>=0 && tmpY2<lHeight)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						tmpVala = pDatagray + tmpY2 *  pImage->lBlockLine + tmpX2;
						if(*tmpVal2>=lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
							rightWidthArray[i]++;
					}
				}
			}
		}
	}
	else if (dCoeffK<1.0 && dCoeffK>-1.0)	// x
	{
		dCoeffKTmp = tan(atan(dCoeffK) + V_PI/2);
		pData = (MByte*)pHaarResponse->pBlockData;
		leftX = ptMid.x - 1;
		rightX = ptMid.x + 1;
		for (i=0; i<lWidthStep; i++, leftX--, rightX++)
		{
			leftY = (MLong)(dCoeffK * leftX + dCoeffB);
			rightY = (MLong)(dCoeffK * rightX + dCoeffB);
			if(leftX<0 || leftX>lWidth || leftY<0 || leftY>lHeight
				|| rightX<0 || rightX>lWidth || rightY<0 || rightY>lHeight)
				break;

			// ********left
			dCoeffBTmp = leftY - dCoeffKTmp * leftX;
			tmpOff = 0;
			tmpY = leftY - lMaxWidth;
			if (tmpY<0)
			{
				tmpOff = tmpY;
				tmpY = 0;

			}
			lWidthRange = lMaxWidth*2+tmpOff;
			
			tmpValo = pData + leftY * lStride + leftX;//��׼����ֵ����Ӧͼ�г�ʼ��
			tmpXp = leftX;
			tmpYp = leftY;
			tmpXq = tmpX;
			tmpYq = tmpY;
			for (j = 0; j < lWidthRange; j++, tmpYq++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
			{
				tmpXq = (MLong)((tmpYq - dCoeffBTmp) / dCoeffKTmp);
				if (tmpXq<0 || tmpXq>lWidth || tmpYq<0 || tmpYq>lHeight)
					break;
				tmpVal1 = pData + tmpYq * lStride + tmpXq;
				if(*tmpVal1>*tmpValo)//������Ӧ���ֵ����λ��
				{
					tmpValo = pData + tmpYq * lStride + tmpXq;
					tmpYp = tmpYq;
					tmpXp = tmpXq;
				}		
			}
			tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ
			//printf("[leftX, leftY]=[%d,%d]=%d\n", tmpXp, tmpYp, *tmpValo);
			for (j=0; j<lWidthRange; j++, tmpY++)	// y 
			{
				tmpX = (MLong)((tmpY - dCoeffBTmp) / dCoeffKTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
				if (*tmpVal1 >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					leftWidthArrray[i]++;
			}
			//printf("[%d,%d],leftWidthArrray[%d]=%d,lWidthRange=%d\n",leftX, leftY,i,leftWidthArrray[i],lWidthRange);
			//********right
			dCoeffBTmp = rightY - dCoeffKTmp * rightX;
			tmpOff = 0;
			tmpY = rightY - lMaxWidth;
			if (tmpY<0)
			{
				tmpOff = tmpY;
				tmpY = 0;
			}
			lWidthRange = lMaxWidth*2+tmpOff;

			tmpValo = pData + rightY * lStride + rightX;//��׼����ֵ����Ӧͼ�г�ʼ��
			tmpYp = rightY;
			tmpXp = rightX;
			tmpXq = tmpX;
			tmpYq = tmpY;
			for (j = 0; j < lWidthRange; j++, tmpYq++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
			{
				tmpXq = (MLong)((tmpYq - dCoeffBTmp) / dCoeffKTmp);
				if (tmpXq<0 || tmpXq>lWidth || tmpYq<0 || tmpYq>lHeight)
					break;
				tmpVal1 = pData + tmpYq * lStride + tmpXq;
				if (*tmpVal1>*tmpValo)//������Ӧ���ֵ����λ��
				{
					tmpValo = pData + tmpYq * lStride + tmpXq;
					tmpYp = tmpYq;
					tmpXp = tmpXq;
				}
			}
			tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ
			for (j=0; j<lWidthRange; j++, tmpY++)
			{
				tmpX = (MLong)((tmpY - dCoeffBTmp) / dCoeffKTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
				if (*tmpVal1 >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
					rightWidthArray[i]++;
			}
			//printf("[%d,%d],rightWidthArray[%d]=%d,lWidthRange=%d-----------------\n", rightX, rightY,i,rightWidthArray[i], lWidthRange);
		}
		
	}
	else	// y
	{
		dCoeffKTmp = tan(atan(dCoeffK) + V_PI/2);
		pData = (MByte*)pHaarResponse->pBlockData;
		leftY = ptMid.y - 1;
		rightY = ptMid.y + 1;
		for (i=0; i<lWidthStep; i++, leftY--, rightY++)
		{
			leftX = (MLong)((leftY - dCoeffB) / dCoeffK);
			rightX = (MLong)((rightY - dCoeffB) / dCoeffK);

			if(leftX<0 || leftX>lWidth || leftY<0 || leftY>lHeight
				|| rightX<0 || rightX>lWidth || rightY<0 || rightY>lHeight)
				break;

			//********left
			dCoeffBTmp = leftY - dCoeffKTmp * leftX;
			tmpOff = 0;
			tmpX = leftX - lMaxWidth;
			if (tmpX<0)
			{
				tmpOff = tmpX;
				tmpX = 0;
			}
			lWidthRange = lMaxWidth*2+tmpOff;
			tmpValo = pData + leftY * lStride + leftX;//��׼����ֵ����Ӧͼ�г�ʼ��
			tmpXp = leftX;
			tmpYp = leftY;
			tmpXq = tmpX;
			tmpYq = tmpY;
			for (j = 0; j < lWidthRange; j++, tmpXq++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
			{
				tmpYq = (MLong)(dCoeffKTmp * tmpXq + dCoeffBTmp);
				if (tmpXq<0 || tmpXq>lWidth || tmpYq<0 || tmpYq>lHeight)
					break;
				tmpVal1 = pData + tmpYq * lStride + tmpXq;
				if (*tmpVal1>*tmpValo)//������Ӧ���ֵ����λ��
				{
					tmpValo = pData + tmpYq * lStride + tmpXq;
					tmpYp = tmpYq;
					tmpXp = tmpXq;
				}
			}
			tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ
			for (j=0; j<lWidthRange; j++, tmpX++)
			{
				tmpY = (MLong)(dCoeffKTmp * tmpX + dCoeffBTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
				if (*tmpVal1 >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
				//if(*tmpVal1>=lThreshold)
					leftWidthArrray[i]++;
			}
			//printf("[%d,%d],leftWidthArrray[%d]=%d\n", leftX, leftY, i,leftWidthArrray[i]);
			//********right
			dCoeffBTmp = rightY - dCoeffKTmp * rightX;
			tmpOff = 0;
			tmpX = rightX - lMaxWidth;
			if (tmpX<0)
			{
				tmpOff = tmpX;
				tmpX = 0;
			}
			lWidthRange = lMaxWidth*2+tmpOff;
			tmpValo = pData + rightY * lStride + rightX;//��׼����ֵ����Ӧͼ�г�ʼ��
			tmpYp = rightY;
			tmpXp = rightX;
			tmpXq = tmpX;
			tmpYq = tmpY;
			for (j = 0; j < lWidthRange; j++, tmpXq++)//��Ӧͼ�б�����ȷ�ΧѰ����Ӧ���λ��
			{
				tmpYq = (MLong)(dCoeffKTmp * tmpXq + dCoeffBTmp);
				if (tmpXq<0 || tmpXq>lWidth || tmpYq<0 || tmpYq>lHeight)
					break;
				tmpVal1 = pData + tmpYq * lStride + tmpXq;
				if (*tmpVal1>*tmpValo)//������Ӧ���ֵ����λ��
				{
					tmpValo = pData + tmpYq * lStride + tmpXq;
					tmpYp = tmpYq;
					tmpXp = tmpXq;
				}
			}
			tmpValo = pDatagray + tmpYp * pImage->lBlockLine + tmpXp;//�Ҷ�ͼ�л�׼����ֵ

			for (j=0; j<lWidthRange; j++, tmpX++)
			{
				tmpY = (MLong)(dCoeffKTmp * tmpX + dCoeffBTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				tmpVala = pDatagray + tmpY *  pImage->lBlockLine + tmpX;
				if (*tmpVal1 >= lThreshold && (abs(*tmpVala - *tmpValo)*1.0<MAX(lPercentage*(*tmpValo), 6)))
				//if(*tmpVal1>=lThreshold)
					rightWidthArray[i]++;
			}
			//printf("[%d,%d],rightWidthArray[%d]=%d-----------------\n", rightX, rightY, i,rightWidthArray[i]);
		}
	}

	if(0)          //�Ľ�������Ŷ��㷨,���Ȩ�أ�Ϊ�㵽�е�ľ���      2018/06/05  ls
	{
		for (i=0; i<lWidthStep; i++)
		{
			if(leftWidthArrray[i]<rightWidthArray[i]){
				(*leftWidthConfidence)=(*leftWidthConfidence)+i+1;
			}
			else if(leftWidthArrray[i]>rightWidthArray[i]){
				(*rightWidthConfidence)=(*rightWidthConfidence)+i+1;
			}
			else
			{
				(*leftWidthConfidence)=(*leftWidthConfidence)+i+1;
				(*rightWidthConfidence)=(*rightWidthConfidence)+i+1;
			}
		}
		(*leftWidthConfidence)=2*(*leftWidthConfidence)/(lWidthStep+1);
		(*rightWidthConfidence)=2*(*rightWidthConfidence)/(lWidthStep+1);
	}
	else
	{
		for (i=0; i<lWidthStep; i++)
		{
			if((leftWidthArrray[i]==0||rightWidthArray[i]==0)&&cmpflag==0)
			{
				(*leftWidthConfidence)++;
				(*rightWidthConfidence)++;
				continue;
			}
			if(cmpflag==0)
				cmpflag=1;
			if(leftWidthArrray[i]<rightWidthArray[i]){
				(*leftWidthConfidence)++;
			}
			else if(leftWidthArrray[i]>rightWidthArray[i]){
				(*rightWidthConfidence)++;
			}
			else
			{
				(*leftWidthConfidence)++;
				(*rightWidthConfidence)++;
			}
		}
		
	}

EXT:
	FreeVectMem(hMemMgr, leftWidthArrray);
	FreeVectMem(hMemMgr, rightWidthArray);

	return res;
}

// up rank
MLong CmpVal_xsd( const MVoid* p1, const MVoid* p2)
{
	MLong _p1 = *(MLong*)p1;
	MLong _p2 = *(MLong*)p2;

	if( _p1 < _p2)
		return 1;
	if (_p1 > _p2)	// swap
		return -1;
	return 0;
}

//���򣬻��ǽ���?????������
MLong ValueCmp( const MVoid* p1, const MVoid* p2)
{
	MByte _p1 = *(MByte*)p1;
	MByte _p2 = *(MByte*)p2;

	if( _p1 < _p2)
		return 1;
	if (_p1 > _p2)
		return -1;
	return 0;
}

//�˺�����Ŀ����Ϊ��softthred������ֵ��
//������lHorizontalValΪˮλ�ߣ�ˮλ�����ϵ���ֵ����ͳ��
//����ֵ������
MRESULT StatisticNonZeroValue(MHandle hMemMgr, MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,MByte lHorizontalVal,
	MVoid* pMem, MLong length, MLong lPercentage, MLong *retVal)
{
	MRESULT res = LI_ERR_NONE;
	MLong i, j;
	MLong lExt = lDataLine - lWidth;
	MByte *pTmpData = pData;
	MByte *pValueList = (MByte *)pMem;
	MLong lCounts= 0 ;

	for (j=0; j<lHeight; j++, pTmpData+=lExt)
	{
		if (lCounts>=length)
			break;
		for (i=0; i<lWidth; i++, pTmpData++)
		{
			if (lCounts>=length)
				break;

			if (*pTmpData>lHorizontalVal)
			{
				*pValueList++ = *pTmpData;
				lCounts++;
			}
		}
	}

	//����Բ��Ի�Ҫȷ��һ��????
	GO(QuickSort(hMemMgr, pMem, lCounts, sizeof(MByte),ValueCmp));	// high --> low

	if (retVal!=MNull)
		*retVal =*((MByte*)pMem+lPercentage*lCounts/100);
EXT:
	return res;
}

// ���ݸ�����ת�ǶȻ�õ���Ӧͼ������ȡÿ�����ص��ϳ��ֵ����ֵ��Ϊ������Ӧֵ
MVoid OptimizeHaar_xsd(ResponseResult *pSrc, ResponseResult *pDst, BLOCK *xBlock, BLOCK *yBlock,
	BLOCK *pResponseAngle, MLong *lMaxVal, MByte angleIndex)
{
	MLong xx, yy;
	MLong srcWidth, srcHeight, srcStride;
	MLong dstWidth, dstHeight, dstStride;
	MLong angStride;
	MLong lSrcExt, lDstExt;
	MLong *pSrcData, *pDstData, *pDstDataTmp;
	MByte *pAngleData, *pAngleDataTmp;
	MUInt16 *xData, *yData;

	srcWidth = pSrc->lWidth;
	srcHeight = pSrc->lHeight;
	srcStride = pSrc->lBlockLine;
	lSrcExt = srcStride - srcWidth;
	pSrcData = pSrc->pData;

	dstWidth = pDst->lWidth;
	dstHeight = pDst->lHeight;
	dstStride = pDst->lBlockLine;
	lDstExt = dstStride - dstWidth;
	pDstData = pDst->pData;

	angStride = pResponseAngle->lBlockLine;
	pAngleData = (MByte *)pResponseAngle->pBlockData;

	xData = (MUInt16 *)xBlock->pBlockData;
	yData = (MUInt16 *)yBlock->pBlockData;

	for(yy=0; yy<srcHeight; yy++, pSrcData+=lSrcExt, xData+=lSrcExt, yData+=lSrcExt)
	{
		for(xx=0; xx<srcWidth; xx++, pSrcData++, xData++, yData++)
		{
			if(*xData>=0 && *xData<dstWidth && *yData>=0 && *yData<dstHeight)
			{
				pDstDataTmp = pDstData + (*yData * dstStride) + *xData;
				pAngleDataTmp = pAngleData + (*yData * angStride) + *xData;
				if((*pDstDataTmp&(~TEMPLATE_TYPE_LONG_BIT)) < (*pSrcData&(~TEMPLATE_TYPE_LONG_BIT)))
				{
					*pDstDataTmp = *pSrcData;
					//����angledatatmp����ģ�巽����Ϣ
					*pAngleDataTmp=(*(pDstDataTmp)&TEMPLATE_TYPE_LONG_BIT)?(angleIndex | TEMPLATE_TYPE_BYTE_BIT):angleIndex;
				}
				if((MLong)(*pDstDataTmp&(~TEMPLATE_TYPE_LONG_BIT)) > (MLong)*lMaxVal)
					*lMaxVal = *pDstDataTmp&(~TEMPLATE_TYPE_LONG_BIT);
			}
		}
	}
}

MRESULT CreateCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSet, MLong lMaxCircleCandidate,
	MLong lMaxPtNumPerCircle)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	if (pCircleSet==MNull || lMaxCircleCandidate <=0 || lMaxPtNumPerCircle < 3)
	{	
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	SetVectZero(pCircleSet, sizeof(CIRCLE_SET_INFO));
	AllocVectMem(hMemMgr, pCircleSet->pCircleInfo, lMaxCircleCandidate, CIRCLE_INFO);
	SetVectZero(pCircleSet->pCircleInfo, lMaxCircleCandidate*sizeof(CIRCLE_INFO));//����һ��Ҫ��ʼ��
	pCircleSet->lMaxParamNum = lMaxCircleCandidate;
	pCircleSet->lParamNum = 0;

	AllocVectMem(hMemMgr, pCircleSet->pMem, lMaxCircleCandidate*lMaxPtNumPerCircle*2, PTINFO);	//pMem:cirPt linePt cirPt linePt ...
	for ( i =0; i<lMaxCircleCandidate; i++)
	{
		pCircleSet->pCircleInfo[i].pPtinfo = (PTINFO*) pCircleSet->pMem+i*2*lMaxPtNumPerCircle;
		pCircleSet->pCircleInfo[i].lPtNum = 0;
		pCircleSet->pCircleInfo[i].lMaxPtNum = lMaxPtNumPerCircle;

		pCircleSet->pCircleInfo[i].lineInfo.pPtinfo = pCircleSet->pCircleInfo[i].pPtinfo + lMaxPtNumPerCircle;
		pCircleSet->pCircleInfo[i].lineInfo.lMaxPtNum = lMaxPtNumPerCircle;
		pCircleSet->pCircleInfo[i].lineInfo.lPtNum = 0;
	}

EXT:
	if (res!=LI_ERR_NONE)
	{
		FreeVectMem(hMemMgr, pCircleSet->pMem);
		FreeVectMem(hMemMgr, pCircleSet->pCircleInfo);
		SetVectZero(pCircleSet, sizeof(CIRCLE_SET_INFO));
	}
	return res;
}

MVoid FreeCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSetInfo)
{
	if(pCircleSetInfo!=MNull)
	{
		FreeVectMem(hMemMgr, pCircleSetInfo->pMem);
		FreeVectMem(hMemMgr, pCircleSetInfo->pCircleInfo);
		SetVectZero(pCircleSetInfo, sizeof(CIRCLE_SET_INFO));
	}
}

MRESULT CreateLineSet(MHandle hMemMgr, LINE_SET_INFO *pLineSet, MLong lMaxLineCandidate, MLong lMaxPtNumPerLine)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	if (pLineSet==MNull || lMaxLineCandidate<=0 || lMaxPtNumPerLine<2)
	{
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	SetVectZero(pLineSet, sizeof(LINE_SET_INFO));
	AllocVectMem(hMemMgr, pLineSet->pLineInfo, lMaxLineCandidate, LINE_INFO);
	SetVectZero(pLineSet->pLineInfo, lMaxLineCandidate*sizeof(LINE_INFO));//����һ��Ҫ��ʼ��
	pLineSet->lMaxLineNum = lMaxLineCandidate;
	pLineSet->lLineNum= 0;

	AllocVectMem(hMemMgr,pLineSet->pMem, lMaxLineCandidate*lMaxPtNumPerLine, PTINFO);

	for (i=0; i<lMaxLineCandidate; i++)
	{
		pLineSet->pLineInfo[i].pPtinfo = (PTINFO*)pLineSet->pMem + i*lMaxPtNumPerLine;
		pLineSet->pLineInfo[i].lPtNum = 0;
		pLineSet->pLineInfo[i].lMaxPtNum = lMaxPtNumPerLine;
	}

EXT:
	if (res<0)
	{
		FreeVectMem(hMemMgr, pLineSet->pMem);
		FreeVectMem(hMemMgr, pLineSet->pLineInfo);
		pLineSet->lMaxLineNum = 0;
	}
	return res;
}

MVoid FreeLineSet(MHandle hMemMgr, LINE_SET_INFO *pLineSet)
{
	if (pLineSet==MNull)return;
	FreeVectMem(hMemMgr, pLineSet->pMem);
	FreeVectMem(hMemMgr, pLineSet->pLineInfo);
	pLineSet->lMaxLineNum = 0;
}

MRESULT CreateCircleInfo(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, MLong lMaxPtNum)
{
	MRESULT res = LI_ERR_NONE;

	if (pCircleInfo==MNull) 
		return LI_ERR_UNKNOWN;

	JMemSet(pCircleInfo, 0, sizeof(CIRCLE_INFO));

	pCircleInfo->lMaxPtNum = lMaxPtNum;
	AllocVectMem(hMemMgr, pCircleInfo->pPtinfo, lMaxPtNum, PTINFO);
	AllocVectMem(hMemMgr, pCircleInfo->lineInfo.pPtinfo, lMaxPtNum, PTINFO);
	pCircleInfo->lineInfo.lMaxPtNum = lMaxPtNum;

EXT:
	if (res!=LI_ERR_NONE)
	{
		FreeVectMem(hMemMgr, pCircleInfo->pPtinfo);
		FreeVectMem(hMemMgr, pCircleInfo->lineInfo.pPtinfo);
		pCircleInfo->lMaxPtNum = 0;
		pCircleInfo->lineInfo.lMaxPtNum = 0;
	}
	return res;
}

MVoid FreeCicleInfo(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo)
{
	if (pCircleInfo==MNull)return;
	FreeVectMem(hMemMgr, pCircleInfo->pPtinfo);
	FreeVectMem(hMemMgr, pCircleInfo->lineInfo.pPtinfo);
	pCircleInfo->lineInfo.lMaxPtNum = 0;
	pCircleInfo->lMaxPtNum = 0; 
	return;
}

MVoid CircleInfoCpy(CIRCLE_INFO *pSrcCircle, CIRCLE_INFO *pDstCircle)
{
	if (pSrcCircle==MNull || pDstCircle==MNull)return;
	pDstCircle->lConfidence = pSrcCircle->lConfidence;
	pDstCircle->circleParam.xcoord = pSrcCircle->circleParam.xcoord;
	pDstCircle->circleParam.ycoord = pSrcCircle->circleParam.ycoord;
	pDstCircle->circleParam.lRadius = pSrcCircle->circleParam.lRadius;

	pDstCircle->lineInfo.lConfidence = pSrcCircle->lineInfo.lConfidence;
	pDstCircle->lineInfo.lPtNum= pSrcCircle->lineInfo.lPtNum;
	pDstCircle->lineInfo.lineParam.bVertical = pSrcCircle->lineInfo.lineParam.bVertical;
	pDstCircle->lineInfo.lineParam.DAngle= pSrcCircle->lineInfo.lineParam.DAngle;
	pDstCircle->lineInfo.lineParam.Coefb= pSrcCircle->lineInfo.lineParam.Coefb;
	pDstCircle->lineInfo.lineParam.Coefk= pSrcCircle->lineInfo.lineParam.Coefk;

	//xsd
	pDstCircle->lineInfo.lineParam.ptStart = pSrcCircle->lineInfo.lineParam.ptStart;
	pDstCircle->lineInfo.lineParam.ptEnd = pSrcCircle->lineInfo.lineParam.ptEnd;

	JMemCpy(pDstCircle->lineInfo.pPtinfo, pSrcCircle->lineInfo.pPtinfo, sizeof(PTINFO)*pSrcCircle->lineInfo.lPtNum);

	pDstCircle->lLineNum = pSrcCircle->lLineNum;
	pDstCircle->lPtNum = pSrcCircle->lPtNum;
	JMemCpy(pDstCircle->pPtinfo, pSrcCircle->pPtinfo, sizeof(PTINFO)*pSrcCircle->lPtNum);
}

//����ptStart��ptEnd֮������Ŷȣ�������dCoeffK��dCoeffB
//ȷ��һ��ê�㣬��ê�㸽��ͨ����ת��ƽ�ƣ���ֱ������Ӧ���ķ�ʽ
//�������ѵ�ֱ��

//#define DEBUG_ADJUSTLINE
MVoid AdjustLinePosition(MByte *pResponseData, MLong lRspLine, MLong lWidth, MLong lHeight,
	LINEPARAM *pLineParam, MPOINT *pptAnchor, MPOINT* pptStart, MPOINT* pptEnd)
{
	BLOCK tmpblock = {0};
	MLong i, j, maxi, maxj;
	MDouble lMeanSum, lNum;
	MDouble lTotalSum, lMax;
	MDouble dMaxValueCoeffK;
	MBool bMaxValueVert, bVert;
	MPOINT ptMaxValueAnchor, ptAnchor;
	MPOINT ptStart, ptEnd;
	MDouble dist;
	MDouble dCurAngle;

	tmpblock.pBlockData = pResponseData;
	tmpblock.lBlockLine = lRspLine;
	tmpblock.lWidth = lWidth;
	tmpblock.lHeight = lHeight;
	tmpblock.typeDataA = DATA_U8;
	ptStart.x = pptStart->x;
	ptStart.y = pptStart->y;
	ptEnd.x = pptEnd->x;
	ptEnd.y = pptEnd->y;
	dist = vDistance3L(ptStart, ptEnd);
	//if (lframeNum==18 && lcircleNum==6)
	//Logger("origin:\n");
	lMeanSum = vGetPointMeanValueBtwPtWithDistWeight(pResponseData, lRspLine, lWidth, lHeight, 
		ptStart, ptEnd, 0xff, &lNum);

#ifdef DEBUG_ADJUSTLINE
	if (lframeNum==92)
	{
		MChar path[256]={0};
		sprintf(path, ".\\log\\origin_circle_line.bmp");
		B_Cpy(&blockFlagTmp, &tmpblock);
		vLineTo(blockFlagTmp.pBlockData,blockFlagTmp.lBlockLine,blockFlagTmp.lWidth, 
			blockFlagTmp.lHeight, 255, ptStart, ptEnd);
		PrintBmpEx(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, DATA_U8, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 1,path);
	}
#endif
	lTotalSum = lMeanSum * lNum;
	if (pLineParam->bVertical)
		dCurAngle = 90;
	//		dCurAngle = V_PI / 2;
	else
		//		dCurAngle = atan(pLineParam->Coefk);
		dCurAngle = atan(pLineParam->Coefk) * 180 / V_PI;
	lMax = lTotalSum;
	dMaxValueCoeffK = pLineParam->Coefk;
	ptMaxValueAnchor.x = pptAnchor->x;
	ptMaxValueAnchor.y=pptAnchor->y;
	bMaxValueVert = pLineParam->bVertical;
	//��ת
	for (i=-20; i<20; i+=1)
	{
		//		MDouble dCurAngle1 = dCurAngle + i*V_PI/360;
		MDouble dCurAngle1 = dCurAngle + i;
		//		if (dCurAngle1 == -V_PI/2 || dCurAngle1 == V_PI/2)
		if (dCurAngle1 == -90 || dCurAngle1 == 90)
			bVert  = MTrue;
		else
			bVert = MFalse;
		//����ê���б�ʣ�������˵ĵ�		
		vGetPointWidthParam(tan(dCurAngle1*V_PI/180), bVert, *pptAnchor, &ptStart, &ptEnd, dist/2);
		lMeanSum = vGetPointMeanValueBtwPtWithDistWeight(pResponseData, lRspLine, lWidth, lHeight, 
			ptStart, ptEnd, 0xff, &lNum);
#ifdef DEBUG_ADJUSTLINE		
		if (lframeNum==92)
		{
			MChar path[256]={0};
			sprintf(path, ".\\log\\circle,angle%f.bmp",i*0.5);
			B_Cpy(&blockFlagTmp, &tmpblock);
			vLineTo(blockFlagTmp.pBlockData,blockFlagTmp.lBlockLine,blockFlagTmp.lWidth, 
				blockFlagTmp.lHeight, 255, ptStart, ptEnd);
			PrintBmpEx(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, DATA_U8, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 1,path);
		}
#endif		
		lTotalSum = lMeanSum * lNum;
		if (lMax < lTotalSum)
		{
			lMax = lTotalSum;
			pptStart->x = ptStart.x;
			pptStart->y=ptStart.y;
			pptEnd->x = ptEnd.x;
			pptEnd->y = ptEnd.y;
			maxi = i; maxj = 0;
			if (bVert)
			{
				dMaxValueCoeffK  = 0;
				bMaxValueVert = MTrue;
				ptMaxValueAnchor.x = pptAnchor->x;ptMaxValueAnchor.y=pptAnchor->y;
			}
			else
			{
				dMaxValueCoeffK = tan(dCurAngle1*V_PI/180);
				bMaxValueVert = MFalse;
				ptMaxValueAnchor.x = pptAnchor->x;ptMaxValueAnchor.y=pptAnchor->y;
			}

		}
		//ƽ��

		for(j=-25; j<25; j+=1)	//-10 10
		{
			if (bVert || (dCurAngle1>-45 && dCurAngle1<45)) 
			{
				//				ptAnchor.x = pptAnchor->x; 
				//				ptAnchor.y = pptAnchor->y+j;
				ptAnchor.x = pptAnchor->x + j; 
				ptAnchor.y = pptAnchor->y;
			}
			else
			{
				//				ptAnchor.x = pptAnchor->x+j;
				//				ptAnchor.y= pptAnchor->y;
				ptAnchor.x = pptAnchor->x;
				ptAnchor.y= pptAnchor->y + j;
			}

			vGetPointWidthParam(tan(dCurAngle1*V_PI/180), bVert, ptAnchor, &ptStart, &ptEnd, dist/2);
			lMeanSum= vGetPointMeanValueBtwPtWithDistWeight(pResponseData, lRspLine, lWidth, lHeight, 
				ptStart, ptEnd, 0xff, &lNum);
#ifdef DEBUG_ADJUSTLINE
			if (lframeNum==92)
			{
				MChar path[256]= {0};
				sprintf(path, ".\\log\\circle,angle%f_transfer%d.bmp",i*0.5,j);
				B_Cpy(&blockFlagTmp, &tmpblock);
				vLineTo(blockFlagTmp.pBlockData,blockFlagTmp.lBlockLine,blockFlagTmp.lWidth, 
					blockFlagTmp.lHeight, 255, ptStart, ptEnd);
				PrintBmpEx(blockFlagTmp.pBlockData, blockFlagTmp.lBlockLine, DATA_U8, blockFlagTmp.lWidth, blockFlagTmp.lHeight, 1,path);
			}
#endif
			lTotalSum = lMeanSum * lNum;
			if (lMax < lTotalSum)
			{
				lMax = lTotalSum;
				pptStart->x = ptStart.x;pptStart->y=ptStart.y;
				pptEnd->x = ptEnd.x;pptEnd->y = ptEnd.y;
				maxi = i; maxj = j;
				if (bVert)
				{
					dMaxValueCoeffK  = 0;
					bMaxValueVert = MTrue;
					ptMaxValueAnchor.x = ptAnchor.x;ptMaxValueAnchor.y=ptAnchor.y;
				}
				else
				{
					dMaxValueCoeffK = tan(dCurAngle1*V_PI/180);
					bMaxValueVert = MFalse;
					ptMaxValueAnchor.x = ptAnchor.x;ptMaxValueAnchor.y=ptAnchor.y;
				}
			}
		} 			
	}	
	pLineParam->Coefk= dMaxValueCoeffK;
	pLineParam->bVertical = bMaxValueVert;
	if (bMaxValueVert)
	{
		pLineParam->Coefb = - ptMaxValueAnchor.x;
	}
	else
		pLineParam->Coefb = ptMaxValueAnchor.y - pLineParam->Coefk * ptMaxValueAnchor.x;

	pptAnchor->x = ptMaxValueAnchor.x;
	pptAnchor->y = ptMaxValueAnchor.y;
}

MVoid AppendPointList(MByte *pRspData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MByte *pRspAngleData, MLong lAngleDataLine,
	JGSEED *pSeedList, CIRCLE_INFO *pCircleInfo, PARAM_INFO *pParam,MVoid *pTemp, MLong lMemLen)
{
	MLong i, j;
	MDouble dAngle, dDiffAngle, dMeanAngle, dCurAngle;
	//	MLong lNum;
	MLong maxConfidence, confidence;
	MLong maxIndex;
	MBool bChanged;
	MLong left_inc, right_inc;
	MPOINT ptOrg;
	MWord *pTmpMeanAngle = (MWord *)pTemp;
	ptOrg.x = pCircleInfo->circleParam.xcoord;
	ptOrg.y = pCircleInfo->circleParam.ycoord;
	if (pCircleInfo->lPtNum <= 0 || (unsigned long)lMemLen< sizeof(MWord)*pCircleInfo->lPtNum)
		return;
	//�ȼ���meanAngle
	dMeanAngle = 0;
	for (i=0; i<pCircleInfo->lPtNum-1; i++)
	{
		dCurAngle = fabs(pCircleInfo->pPtinfo[i].dPtAngle-pCircleInfo->pPtinfo[i+1].dPtAngle);
		dCurAngle = MIN(2*V_PI-dCurAngle, dCurAngle);
		//dMeanAngle += dCurAngle;
		pTmpMeanAngle[i] = (MWord)(dCurAngle * 180/V_PI+0.5);
	}
	dMeanAngle = FindMidian(pTmpMeanAngle, i, DATA_U16);
	dMeanAngle = dMeanAngle * V_PI/180;
	/*
	dMeanAngle /= pCircleInfo->lPtNum-1;
	//�ų�һЩ�Ƚϴ�ĵ�
	dAngle = dMeanAngle;
	dMeanAngle = 0;
	lNum = 0;
	for (i=0; i<pCircleInfo->lPtNum-1; i++)
	{
	dCurAngle = fabs(pCircleInfo->pPtinfo[i].dPtAngle-pCircleInfo->pPtinfo[i+1].dPtAngle);
	dCurAngle = MIN(360-dCurAngle, dCurAngle);
	if (dCurAngle < dAngle *  1.2)
	{
	dMeanAngle += dCurAngle;
	lNum++;
	}
	}

	if (lNum!=0)
	dMeanAngle /= lNum;
	else
	dMeanAngle = dAngle;
	*/
	maxConfidence = 0;
	maxIndex = 0;
	//�����м��
	for (i=0; i<pCircleInfo->lPtNum-1;i++)
	{
		dCurAngle = fabs(pCircleInfo->pPtinfo[i].dPtAngle-pCircleInfo->pPtinfo[i+1].dPtAngle);
		dCurAngle = MIN(2*V_PI-dCurAngle, dCurAngle);
		if (dCurAngle > dMeanAngle *  1.2)
		{
			//search mid
			maxConfidence = 0;
			for (j=0; j<pSeedList->lSeedNum; j++)
			{
				dCurAngle =vComputeAngle(pSeedList->pptSeed[j].x - ptOrg.x,pSeedList->pptSeed[j].y -ptOrg.y); 
				//				if (pSeedList->pptSeed[j].x>175 && pSeedList->pptSeed[j].x<185
				//					&& pSeedList->pptSeed[j].y>95 && pSeedList->pptSeed[j].y<105)
				//				{MLong aa=0;}
				//���ߵļ�඼Ҫ����xx
				//��ߣ���Ϊ����������
				dDiffAngle = dCurAngle - pCircleInfo->pPtinfo[i].dPtAngle;
				dDiffAngle = MIN(2*V_PI+dDiffAngle, dDiffAngle);
				if (dDiffAngle<0.6*dMeanAngle || dDiffAngle>2*dMeanAngle)continue;
				//�ұߣ���Ϊ����������
				dDiffAngle = pCircleInfo->pPtinfo[i+1].dPtAngle - dCurAngle;	
				dDiffAngle = MIN(2*V_PI+dDiffAngle, dDiffAngle);
				if (dDiffAngle<0.6*dMeanAngle || dDiffAngle>2*dMeanAngle)continue;
				confidence = *((MByte*)pRspData+lDataLine*pSeedList->pptSeed[j].y+pSeedList->pptSeed[j].x);
				if (bOnCircle_EX(ptOrg.x, ptOrg.y, pCircleInfo->circleParam.lRadius, pSeedList->pptSeed[j].x,
					pSeedList->pptSeed[j].y, RANSAC_THRESHOLD3*3/2)&&maxConfidence<confidence)
				{
					MDouble curDist = fabs((MDouble)vDistance3L(pSeedList->pptSeed[j], ptOrg) - pCircleInfo->circleParam.lRadius);
					confidence *= (MLong)GaussianWeight(RANSAC_THRESHOLD3*1.5/1.1774 , (MLong)curDist);
					if (maxConfidence<confidence)
					{
						maxConfidence = confidence;
						maxIndex = j;
					}
				}
			}
			//cpy
			if (maxConfidence>0)
			{
				JMemCpy(pCircleInfo->pPtinfo+i+2, pCircleInfo->pPtinfo+i+1, sizeof(PTINFO)*(pCircleInfo->lPtNum-i-1));
				pCircleInfo->pPtinfo[i+1].ptPoint.x = pSeedList->pptSeed[maxIndex].x;
				pCircleInfo->pPtinfo[i+1].ptPoint.y = pSeedList->pptSeed[maxIndex].y;
				pCircleInfo->pPtinfo[i+1].dPtAngle = vComputeAngle(pSeedList->pptSeed[maxIndex].x -ptOrg.x,
					pSeedList->pptSeed[maxIndex].y - ptOrg.y);
				pCircleInfo->lPtNum ++;
			}
		}
	}
	//������֮ǰҪ��������һ�£��Ѵ��д�ģ����Ϣ��������е��pass��

	for (i=0; i<pCircleInfo->lPtNum; i++)
	{
		PTINFO *ptinfo = pCircleInfo->pPtinfo + i;
		if (*((MByte*)pRspAngleData+lAngleDataLine*ptinfo->ptPoint.y+ptinfo->ptPoint.x) >= 128)
			ptinfo->ptValue = 1;
		else
			ptinfo->ptValue = 0;
	}

	//���ǰ�˳ʱ���������е�
	for (i=0; i<pCircleInfo->lPtNum; i++)
	{
		PTINFO *ptinfo = pCircleInfo->pPtinfo+i;
		if (ptinfo->ptValue && i<pCircleInfo->lPtNum/4)
		{
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+i, pCircleInfo->lPtNum-i);
			pCircleInfo->lPtNum-=i;
			break;
		}
	}
	for (i=0; i<pCircleInfo->lPtNum; i++)
	{
		PTINFO *ptinfo = pCircleInfo->pPtinfo+pCircleInfo->lPtNum-1-i;
		if (ptinfo->ptValue && i<pCircleInfo->lPtNum/3)
		{
			pCircleInfo->lPtNum-=i;
			break;
		}
	}

	if (pCircleInfo->lPtNum <= 0)
		return;
	//������
	bChanged = MTrue;
	left_inc = right_inc = 1;//������Ҫ����һ��һ��������ȥ����Ȼ�ᶪ���м�������Ӧ��
	while(bChanged)
	{
		maxConfidence = 0;
		maxIndex = 0;
		//��������
		bChanged =MFalse;
		//	if (pCircleInfo->lPtNum>=pParam->lNumPts)
		//		break;
		//���ȹ���
		dAngle = fabs(pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle - pCircleInfo->pPtinfo[0].dPtAngle);
		dAngle = MIN(2*V_PI-dAngle, dAngle);
		if ( dAngle>= pParam->lMaxArcAngle*V_PI/180)
			break;
		//search right
		if (*((MByte*)pRspAngleData+lAngleDataLine*pCircleInfo->pPtinfo[0].ptPoint.y+pCircleInfo->pPtinfo[0].ptPoint.x) <127)//���Ǵ�ģ��
		{
			for (i=0; i<pSeedList->lSeedNum; i++)
			{
				/*if (pSeedList->pptSeed[i].x>90 && pSeedList->pptSeed[i].x<100
				&& pSeedList->pptSeed[i].y>20 && pSeedList->pptSeed[i].y<30)
				{MLong aa=0;}*/
				dCurAngle =vComputeAngle(pSeedList->pptSeed[i].x - ptOrg.x,pSeedList->pptSeed[i].y - ptOrg.y);
				//�ұ�
				dDiffAngle = pCircleInfo->pPtinfo[0].dPtAngle - dCurAngle;
				dDiffAngle = MIN(2*V_PI+dDiffAngle, dDiffAngle);
				confidence = *((MByte*)pRspData+lDataLine*pSeedList->pptSeed[i].y+pSeedList->pptSeed[i].x);

				if (dDiffAngle>dMeanAngle*(right_inc-0.3)&& dDiffAngle<dMeanAngle*(right_inc+0.5)&& bOnCircle_EX(
					ptOrg.x, ptOrg.y, pCircleInfo->circleParam.lRadius, pSeedList->pptSeed[i].x,
					pSeedList->pptSeed[i].y, RANSAC_THRESHOLD3*2)&&maxConfidence<confidence)
				{
					MDouble curDist = fabs(vDistance3L(pSeedList->pptSeed[i], ptOrg) - pCircleInfo->circleParam.lRadius);
					dAngle = fabs(dCurAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
					dAngle = MIN(2*V_PI-dAngle, dAngle);
					confidence *= (MLong)GaussianWeight(RANSAC_THRESHOLD3*1.5/1.1774 , (MLong)curDist);
					if ( dAngle< pParam->lMaxArcAngle*V_PI/180 &&maxConfidence<confidence)
					{
						maxConfidence = confidence;
						maxIndex = i;
					}
				}
			}
			//cpy
			if (maxConfidence>0)
			{
				JMemCpy(pCircleInfo->pPtinfo+1, pCircleInfo->pPtinfo, sizeof(PTINFO)*pCircleInfo->lPtNum);
				pCircleInfo->pPtinfo[0].ptPoint.x = pSeedList->pptSeed[maxIndex].x;
				pCircleInfo->pPtinfo[0].ptPoint.y = pSeedList->pptSeed[maxIndex].y;
				pCircleInfo->pPtinfo[0].dPtAngle = vComputeAngle(pSeedList->pptSeed[maxIndex].x - ptOrg.x,
					pSeedList->pptSeed[maxIndex].y - ptOrg.y);
				pCircleInfo->lPtNum ++;
				right_inc = 1;
				bChanged = MTrue;
			}
			else
			{
				dDiffAngle = fabs(pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
				dDiffAngle = MIN(2*V_PI-dDiffAngle, dDiffAngle);
				right_inc++;
				if (right_inc*dMeanAngle + dDiffAngle< pParam->lMaxArcAngle*V_PI/180)
					bChanged = MTrue;
			}
		}
		//if (pCircleInfo->lPtNum>=pParam->lNumPts)
		//	break;
		//���ȹ���
		dAngle = fabs(pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle - pCircleInfo->pPtinfo[0].dPtAngle);
		dAngle = MIN(2*V_PI-dAngle, dAngle);
		if ( dAngle>= pParam->lMaxArcAngle*V_PI/180)
			break;
		dAngle = 0;
		maxConfidence = 0;
		//break;
		//search left
		if (*((MByte*)pRspAngleData+lAngleDataLine*pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y
			+pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x) <127)//���Ǵ�ģ��
		{
			for (i=0; i<pSeedList->lSeedNum; i++)
			{
				dCurAngle = vComputeAngle(pSeedList->pptSeed[i].x - ptOrg.x,pSeedList->pptSeed[i].y - ptOrg.y);
				//���
				dDiffAngle = dCurAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle;
				dDiffAngle = MIN(2*V_PI+dDiffAngle, dDiffAngle);
				confidence = *((MByte*)pRspData+lDataLine*pSeedList->pptSeed[i].y+pSeedList->pptSeed[i].x);
				if (dDiffAngle>dMeanAngle*(left_inc-0.3) && dDiffAngle<dMeanAngle*(left_inc+0.5) && bOnCircle_EX(
					ptOrg.x, ptOrg.y, pCircleInfo->circleParam.lRadius, pSeedList->pptSeed[i].x,
					pSeedList->pptSeed[i].y, RANSAC_THRESHOLD3*3))
				{
					MDouble curDist = fabs(vDistance3L(pSeedList->pptSeed[i], ptOrg) - pCircleInfo->circleParam.lRadius);
					dAngle = fabs(dCurAngle - pCircleInfo->pPtinfo[0].dPtAngle);
					dAngle = MIN(2*V_PI-dAngle, dAngle);
					confidence *= (MLong)GaussianWeight(RANSAC_THRESHOLD3*1.5/1.1774 , (MLong)curDist);
					if ( dAngle< pParam->lMaxArcAngle*V_PI/180 &&maxConfidence<confidence)
					{
						maxConfidence = confidence;
						maxIndex = i;
					}
				}
			}
			if (maxConfidence >0)
			{
				pCircleInfo->pPtinfo[pCircleInfo->lPtNum].ptPoint.x = pSeedList->pptSeed[maxIndex].x;
				pCircleInfo->pPtinfo[pCircleInfo->lPtNum].ptPoint.y = pSeedList->pptSeed[maxIndex].y;
				pCircleInfo->pPtinfo[pCircleInfo->lPtNum].dPtAngle = vComputeAngle(pSeedList->pptSeed[maxIndex].x - ptOrg.x,
					pSeedList->pptSeed[maxIndex].y - ptOrg.y);
				pCircleInfo->lPtNum ++;
				left_inc  = 1;
				bChanged = MTrue;
			}
			else
			{
				MDouble dDiffAngle = fabs(pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
				dDiffAngle = MIN(2*V_PI-dDiffAngle, dDiffAngle);
				left_inc++;
				if (left_inc*dMeanAngle + dDiffAngle< pParam->lMaxArcAngle*V_PI/180)
					bChanged = MTrue;
			}
		}
		//break;

	}
}

MRESULT FilterCirclePointList(MHandle hMemMgr,MByte *pRspData, MLong lRspDataLine, MLong lWidth, MLong lHeight,
	CIRCLE_INFO *pCircleInfo, LINE_INFO *pLineInfo, MPOINT ptLineMid,MLong lMaxArcAngle)
{
	MRESULT res = LI_ERR_NONE;
	MLong j, k,l,lNum;
	MPOINT ptMid;
	PTINFO *pCur1, *pCur2;
	MDouble dInterAngle,dMeanAngle,dAngle;
	JASSERT(pCircleInfo && pRspData);
	JASSERT(pLineInfo);
	JASSERT(pCircleInfo->lPtNum>0);

	//��ptlist�еĵ���һ��ɸѡ���ų�ptlist����Բ���ص��ĵ㣬�Է������ĳ������
	for (j=0; j<pCircleInfo->lPtNum-1; )
	{
		if (pCircleInfo->pPtinfo[j].ptPoint.x == pCircleInfo->circleParam.xcoord
			&& pCircleInfo->pPtinfo[j].ptPoint.y == pCircleInfo->circleParam.ycoord)
		{
			JMemCpy(pCircleInfo->pPtinfo+j, pCircleInfo->pPtinfo+j+1, (pCircleInfo->lPtNum-j-1)*sizeof(PTINFO));
			pCircleInfo->lPtNum--;
		}
		else
			j++;
	}

	if (pCircleInfo->lPtNum>0)
		if (pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x == pCircleInfo->circleParam.xcoord
			&&pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y == pCircleInfo->circleParam.ycoord)
			pCircleInfo->lPtNum--;

	//����Ѿ�֪���������߶εķ�����ô������һ���Ƕ��ڵĵ���Ҫ���ų������Բ֮���
	for(j=0;j<pCircleInfo->lPtNum; j++)
	{
		if (!bInValidArc(ptLineMid, pCircleInfo->pPtinfo[j].ptPoint, pLineInfo->lineParam.DAngle, V_PI/3))continue;
		for(k=pCircleInfo->lPtNum-1;k>j; k--)
		{
			if (!bInValidArc(ptLineMid, pCircleInfo->pPtinfo[k].ptPoint, pLineInfo->lineParam.DAngle, V_PI/3))break;	
			pCircleInfo->lPtNum--;
		}
		if (j==k)pCircleInfo->lPtNum--;
		else
		{
			pCircleInfo->pPtinfo[j].ptPoint.x = pCircleInfo->pPtinfo[k].ptPoint.x;
			pCircleInfo->pPtinfo[j].ptPoint.y = pCircleInfo->pPtinfo[k].ptPoint.y;
			pCircleInfo->pPtinfo[j].ptValue = pCircleInfo->pPtinfo[k].ptValue;
			pCircleInfo->pPtinfo[j].dPtAngle= pCircleInfo->pPtinfo[k].dPtAngle;
			pCircleInfo->lPtNum--;
		}
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//��ǰ�����нǶ�Լ����������ĵ�
	for (j=0; j<pCircleInfo->lPtNum; j++)
	{
		pCircleInfo->pPtinfo[j].dPtAngle = vComputeAngle(pCircleInfo->pPtinfo[j].ptPoint.x - pCircleInfo->circleParam.xcoord,
			pCircleInfo->pPtinfo[j].ptPoint.y - pCircleInfo->circleParam.ycoord);
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//�Ե��������
	GO(QuickSort(hMemMgr, (MVoid*)pCircleInfo->pPtinfo, pCircleInfo->lPtNum, sizeof(PTINFO),PTInfoCmp));
	k = 0;//cur
	ptMid.x = pCircleInfo->circleParam.xcoord;
	ptMid.y = pCircleInfo->circleParam.ycoord;
	for (j=1; j<pCircleInfo->lPtNum; j++)
	{
		pCur1 = pCircleInfo->pPtinfo+k;
		pCur2 = pCircleInfo->pPtinfo+j;
		dInterAngle = fabs(pCur1->dPtAngle-pCur2->dPtAngle);
		dInterAngle = MIN(dInterAngle, 2*V_PI-dInterAngle);
		if (dInterAngle>0.125){k=j;pCur1->ptValue=1;pCur2->ptValue = 1;continue;}
		if (ABS(vDistance3L(pCur1->ptPoint, ptMid)-pCircleInfo->circleParam.lRadius) 
			<= ABS(vDistance3L(pCur2->ptPoint, ptMid)-pCircleInfo->circleParam.lRadius))
		{
			pCur1->ptValue = 1;
			pCur2->ptValue = 0;
		}
		else
		{
			pCur1->ptValue = 0;
			pCur2->ptValue = 1;
			k = j;
		}
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//����ptValue ����
	for (j=0; j<pCircleInfo->lPtNum; j++)
	{
		PTINFO *pCur1 = pCircleInfo->pPtinfo+j;
		if (pCur1->ptValue)continue;
		for (k=j; k<pCircleInfo->lPtNum-1; k++)
		{
			JMemCpy(pCircleInfo->pPtinfo+k, pCircleInfo->pPtinfo+k+1,sizeof(PTINFO));
		}
		pCircleInfo->lPtNum--;
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//��Ҫ������β�ĵ��Ƿ���С�Ƕȵ�
	pCur1 = pCircleInfo->pPtinfo+0;
	pCur2 = pCircleInfo->pPtinfo+pCircleInfo->lPtNum-1;
	dInterAngle = fabs(pCur1->dPtAngle-pCur2->dPtAngle);
	dInterAngle = MIN(dInterAngle, 2*V_PI-dInterAngle);
	if (dInterAngle<=0.125)
	{
		if (ABS(vDistance3L(pCur1->ptPoint, ptMid)-pCircleInfo->circleParam.lRadius) 
			<= ABS(vDistance3L(pCur2->ptPoint, ptMid)-pCircleInfo->circleParam.lRadius))
		{
			//���Ҫɾ���Ķ�β�ĵ�
			pCircleInfo->lPtNum--;
		}
		else
		{
			//���Ҫɾ�����Ƕ��׵ĵ�
			pCircleInfo->lPtNum--;
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+1, pCircleInfo->lPtNum*sizeof(PTINFO));
		}
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//��Ҫ�ų�һЩ�㣬���ڵ�ǰ�����߽Ƕȼ������ģ���Ҫ�ų�
	//��һ����ľ��������仯��(С��ĳ�����Ƕ�)��
	//ÿһ��ptValue��Ϊ1
	//���ڶ���һ���о���ı��Ϊ0����ʾһ�ε��յ�
	for (j=0; j<pCircleInfo->lPtNum-1; j++)
	{
		pCur1 = pCircleInfo->pPtinfo+j;
		pCur2 = pCircleInfo->pPtinfo+j+1;
		dInterAngle = fabs(pCur2->dPtAngle-pCur1->dPtAngle);//��Ϊ���ǴӴ���С�Ź����
		//if (dInterAngle>1.047)//60��
		if (dInterAngle>0.7854)//45��
			pCur1->ptValue = 0;
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	//���һ��������ǰ���ıȽ�
	pCur1 = pCircleInfo->pPtinfo+j;
	pCur2 = pCircleInfo->pPtinfo;
	dInterAngle = pCur1->dPtAngle+2*V_PI-pCur2->dPtAngle;
	//if (dInterAngle>1.047)//60��
	if (dInterAngle > 0.7854)//45��
		pCur1->ptValue = 0;
	//�������
	{
		MLong lPTListLen = 0;
		MLong lPTListFirstLen = 0;
		k = 0;
		//��һ��Ҫ��¼����������βʹ��
		for (j=0; j<pCircleInfo->lPtNum; j++)
		{
			pCur1 = pCircleInfo->pPtinfo+j;
			if (pCur1->ptValue==0)
				break;
		}
		if (j==pCircleInfo->lPtNum) {k=0; lPTListLen = pCircleInfo->lPtNum;}//���һ���Զ��������ģ���ô���������б�
		else
		{
			//��ʼ��Ϊ��һ�ε��������Ⱥ����;
			lPTListLen = lPTListFirstLen = (j-k+1);
			k = 0; 
			j++;l=j;
			for (; j<pCircleInfo->lPtNum; j++)
			{
				pCur1 = pCircleInfo->pPtinfo+j;
				if (pCur1->ptValue==0)
				{
					if (j-l+1>lPTListLen)
					{
						lPTListLen = j-l+1;
						k = l;
					}
					l = j+1;
				}
			}
			lPTListFirstLen += (j-l);
			if (lPTListLen<lPTListFirstLen)
			{
				lPTListLen = lPTListFirstLen;
				k = l;
			}
		}
		//������ֵ,��k��ʼ��lPTListLen����Ч�ģ������Ķ�Ҫ���
		if (k+lPTListLen<=pCircleInfo->lPtNum)
		{
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+k, lPTListLen*sizeof(PTINFO));
			pCircleInfo->lPtNum = lPTListLen;
		}
		else
		{
			JMemCpy(pCircleInfo->pPtinfo+(lPTListLen-( pCircleInfo->lPtNum - k+1)), pCircleInfo->pPtinfo+k, (pCircleInfo->lPtNum - k+1)*sizeof(PTINFO));
			pCircleInfo->lPtNum = lPTListLen-1;
		}
		//pCircleInfo->lPtNum = lPTListLen;
	}

	//printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	if (pCircleInfo->lPtNum < 3)goto EXT;
	//��filter����pParam->lMaxArcAngle
	//��meanAngle
	dMeanAngle = 0;
	for (j=0; j<pCircleInfo->lPtNum-1; j++)
	{
		dInterAngle = fabs(pCircleInfo->pPtinfo[j].dPtAngle-pCircleInfo->pPtinfo[j+1].dPtAngle);
		dInterAngle = MIN(2*V_PI-dInterAngle, dInterAngle);
		dMeanAngle += dInterAngle;
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	dMeanAngle /= pCircleInfo->lPtNum-1;
	dAngle = dMeanAngle;
	dMeanAngle = 0;
	lNum = 0;
	for (j=0; j<pCircleInfo->lPtNum-1; j++)
	{
		dInterAngle = fabs(pCircleInfo->pPtinfo[j].dPtAngle-pCircleInfo->pPtinfo[j+1].dPtAngle);
		dInterAngle = MIN(2*V_PI-dInterAngle, dInterAngle);
		if (dInterAngle < dAngle *  1.2)
		{
			dMeanAngle += dInterAngle;
			lNum++;
		}
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	if (lNum!=0)
		dMeanAngle /= lNum;
	else
		dMeanAngle = dAngle;
	dInterAngle = (pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
	dInterAngle = MIN(dInterAngle+2*V_PI, dInterAngle);

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	while (dInterAngle>lMaxArcAngle*V_PI/180)
	{
		MLong confidence_left, confidence_right;
		MDouble dDiffAngle;

		if(pCircleInfo->lPtNum < 2)
		{
			res = LI_ERR_NO_FIND;
			return res;
		}
		//check ����
		dInterAngle = fabs(pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[1].dPtAngle);
		dInterAngle = MIN(2*V_PI-dInterAngle, dInterAngle);
		dDiffAngle = fabs(dInterAngle - dMeanAngle);
		//printf("pCircleInfo->pPtinfo[0].ptPoint.y=%d\n", pCircleInfo->pPtinfo[0].ptPoint.y);
		/*		if(pCircleInfo->pPtinfo[0].ptPoint.y>lHeight|| pCircleInfo->pPtinfo[0].ptPoint.y<0
		|| pCircleInfo->pPtinfo[0].ptPoint.x>lWidth|| pCircleInfo->pPtinfo[0].ptPoint.x<0)
		{
		res = LI_ERR_NO_FIND;
		return res;
		}    */
		/*		printf("x=%d, y=%d~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pCircleInfo->pPtinfo[0].ptPoint.x,
		pCircleInfo->pPtinfo[0].ptPoint.y);  */
		confidence_left = *((MByte*)pRspData+lRspDataLine*pCircleInfo->pPtinfo[0].ptPoint.y + pCircleInfo->pPtinfo[0].ptPoint.x) 
			*(MLong)GaussianWeight(2*dMeanAngle, (MLong)dDiffAngle);

		//		printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
		dInterAngle = fabs(pCircleInfo->pPtinfo[pCircleInfo->lPtNum-2].dPtAngle-pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
		dInterAngle = MIN(2*V_PI-dInterAngle, dInterAngle);
		dDiffAngle = fabs(dInterAngle-dMeanAngle);
		//printf("pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y=%d\n", pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y);
		/*		if(pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y>lHeight|| pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y<0
		|| pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x>lWidth|| pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x<0)
		{
		res = LI_ERR_NO_FIND;
		return res;
		}    */
		/*		printf("x=%d, y=%d~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x,
		pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y);  */
		confidence_right = *((MByte*)pRspData+lRspDataLine*pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y + pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x) 
			*(MLong)GaussianWeight(2*dMeanAngle, (MLong)dDiffAngle);

		//		printf("[%s,%s,%d]\n, left=%d, right=%d", __FILE__,__FUNCTION__,__LINE__, confidence_left, confidence_right);
		if(confidence_left<confidence_right)
		{
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+1, (pCircleInfo->lPtNum-1)*sizeof(PTINFO));
			pCircleInfo->lPtNum--;
		}
		else
		{
			pCircleInfo->lPtNum--;
		}
		//		printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
		dInterAngle = (pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
		dInterAngle = MIN(dInterAngle+2*V_PI, dInterAngle);					
	}

	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
EXT:
	return res;
}

MVoid selectPoint(PTINFO* pSrcPtInfoList, MLong srcPtInfoNum, JGSEED * pSeeds, MLong lRadius)
{
	MLong i, j;
	MLong xDiff, yDiff;
	if (pSrcPtInfoList==MNull || pSeeds==MNull)return;
	if(0==srcPtInfoNum || 0==pSeeds->lSeedNum)return;

	for(i=0; i<pSeeds->lSeedNum;)
	{
		for(j=0; j<srcPtInfoNum; j++)
		{
			xDiff = ABS(pSrcPtInfoList[j].ptPoint.x - pSeeds->pptSeed[i].x);
			yDiff = ABS(pSrcPtInfoList[j].ptPoint.y - pSeeds->pptSeed[i].y);
			if(xDiff<=lRadius && yDiff<=lRadius)	//��ǰseedΪĳ�̶ȵ㣬���ΪlRadius X lRadius����
			{
				if (i<pSeeds->lSeedNum-1) 
				{
					JMemCpy(pSeeds->pptSeed+i, pSeeds->pptSeed+i+1, sizeof(MPOINT)*(pSeeds->lSeedNum-i-1));
				}
				pSeeds->lSeedNum--;
				break;
			}
		}
		if (j==srcPtInfoNum) i++;
	}
	return;
}

MVoid selectPoint2(CIRCLE circleParam, JGSEED *pSeeds, MLong lWidth, MLong lHeight)	// �ų�Բ���
{
	MLong left, right, top, bottom;
	MLong i;

	left = MAX(0, circleParam.xcoord - circleParam.lRadius);
	right = MIN(lWidth, circleParam.xcoord + circleParam.lRadius);
	top = MIN(0, circleParam.ycoord - circleParam.lRadius);
	bottom = MAX(lHeight, circleParam.ycoord  + circleParam.lRadius);

	for (i=0; i<pSeeds->lSeedNum; i++)
	{
		if ((pSeeds->pptSeed+i)->x<=left || (pSeeds->pptSeed+i)->x>=right
			|| (pSeeds->pptSeed+i)->y<=top || (pSeeds->pptSeed+i)->y>=bottom)
		{
			if(i<pSeeds->lSeedNum-1)
			{
				JMemCpy((pSeeds->pptSeed+i), (pSeeds->pptSeed+pSeeds->lSeedNum-1), sizeof(MPOINT));
				pSeeds->lSeedNum--;
			}
		}
	}
}

//**********************************************************************
//xsd cmp
MLong CmpValUp( const MVoid* p1, const MVoid* p2)
{
	QUADRANT _p1 = *(QUADRANT*)p1;
	QUADRANT _p2 = *(QUADRANT*)p2;

	if( _p1.angleVal > _p2.angleVal)
		return 1;
	if (_p1.angleVal < _p2.angleVal)	// swap
		return -1;
	return 0;
}

MLong CmpValDown( const MVoid* p1, const MVoid* p2)
{
	QUADRANT _p1 = *(QUADRANT*)p1;
	QUADRANT _p2 = *(QUADRANT*)p2;

	if( _p1.angleVal < _p2.angleVal)
		return 1;
	if (_p1.angleVal > _p2.angleVal)	// swap
		return -1;
	return 0;
}
//xsd circleSort
/********** startQuadrant ********
|
3  |  4
------ ------
2  |  1
|
*********************************/
#define MAX_QUAD_PT	20
// ��Բ�Ͽ̶ȵ� ��˳ʱ�����ʱ������
// hMemMgr  �ڴ���      ptList  ��̶ȵ�����    ptLen  ��̶ȵ����
// center  ���ԲԲ��     startQuadrant  ������ʼ����   isClockWise ���˳ʱ����������ʱ������ 0 ��ʱ������ 1 ˳ʱ������
MLong circleSort(MHandle hMemMgr, MPOINT *ptList, MLong ptLen, MPOINT center, MLong startQuadrant, MBool isClockWise)
{
	MLong res;
	MLong dx, dy;
	MDouble a, b, c;
	QUADRANT q1[MAX_QUAD_PT], q2[MAX_QUAD_PT], q3[MAX_QUAD_PT], q4[MAX_QUAD_PT];
	MLong num1, num2, num3, num4;
	int i;
	MPOINT ptStart;
	MLong lIndex;
	MLong lNearest, lPtDist;
	MPOINT *pTmpPt = MNull;

	AllocVectMem(hMemMgr, pTmpPt, ptLen, MPOINT);
	res = 0;
	num1 = num2 = num3 = num4 = 0;
	JPrintf("center(%d,%d)\n", center.x, center.y);
	for (i=0; i<ptLen; i++)
	{
		dx = (ptList+i)->x - center.x;
		dy = (ptList+i)->y - center.y;
		a = (MDouble)dx;
		b = (MDouble)dy;
		c = sqrt((MDouble)(dx*dx + dy*dy));
		if (dx>0 && dy>=0)			// ��������Ϊ��ʼ������
		{
			q1[num1].ptPos = *(ptList+i);
			q1[num1++].angleVal = sin(b/c);
		}
		else if (dx<=0 && dy>0)
		{
			q2[num2].ptPos = *(ptList+i);
			q2[num2++].angleVal = sin(b/c);
		}
		else if (dx<0 && dy<=0)
		{
			b = -b;
			q3[num3].ptPos = *(ptList+i);
			q3[num3++].angleVal = sin(b/c);
		}
		else	// dx>=0 dy<0
		{
			b = -b;
			q4[num4].ptPos = *(ptList+i);
			q4[num4++].angleVal = sin(b/c);
		}
	}

	if (isClockWise)	// clockwise
	{
		QuickSort(hMemMgr, (void*)q1, num1, sizeof(QUADRANT), CmpValUp);  // ������������
		QuickSort(hMemMgr, (void*)q2, num2, sizeof(QUADRANT), CmpValDown);
		QuickSort(hMemMgr, (void*)q3, num3, sizeof(QUADRANT), CmpValUp);
		QuickSort(hMemMgr, (void*)q4, num4, sizeof(QUADRANT), CmpValDown);
		if (1==startQuadrant)
		{
			for(i=0; i<num1; i++)
				*(pTmpPt + i) = q1[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num1 + i) = q2[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num1 + num2 + i) = q3[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num1 + num2 + num3 + i) = q4[i].ptPos;
		}
		else if (2==startQuadrant)
		{
			for(i=0; i<num2; i++)
				*(pTmpPt + i) = q2[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num2 + i) = q3[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num2 + num3 + i) = q4[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num2 + num3 + num4 + i) = q1[i].ptPos;
		}
		else if (3==startQuadrant)
		{
			for(i=0; i<num3; i++)
				*(pTmpPt + i) = q3[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num3 + i) = q4[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num3 + num4 + i) = q1[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num3 + num4 + num1 + i) = q2[i].ptPos;
		}
		else if (4==startQuadrant)
		{
			for(i=0; i<num4; i++)
				*(pTmpPt + i) = q4[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num4 + i) = q1[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num4 + num1 + i) = q2[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num4 + num1 + num2 + i) = q3[i].ptPos;
		}
	}
	else
	{
		QuickSort(hMemMgr, (void*)q1, num1, sizeof(QUADRANT), CmpValDown);
		QuickSort(hMemMgr, (void*)q2, num2, sizeof(QUADRANT), CmpValUp);
		QuickSort(hMemMgr, (void*)q3, num3, sizeof(QUADRANT), CmpValDown);
		QuickSort(hMemMgr, (void*)q4, num4, sizeof(QUADRANT), CmpValUp);
		if (1==startQuadrant)
		{
			for(i=0; i<num1; i++)
				*(pTmpPt + i) = q1[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num1 + i) = q4[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num1 + num4 + i) = q3[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num1 + num4 + num3 + i) = q2[i].ptPos;
		}
		else if (2==startQuadrant)
		{
			for(i=0; i<num2; i++)
				*(pTmpPt + i) = q2[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num2 + i) = q1[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num2 + num1 + i) = q4[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num2 + num1 + num4 + i) = q3[i].ptPos;
		}
		else if (3==startQuadrant)
		{
			for(i=0; i<num3; i++)
				*(pTmpPt + i) = q3[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num3 + i) = q2[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num3 + num2 + i) = q1[i].ptPos;
			for(i=0; i<num4; i++)
				*(pTmpPt + num3 + num2 + num1 + i) = q4[i].ptPos;
		}
		else if (4==startQuadrant)
		{
			for(i=0; i<num4; i++)
				*(pTmpPt + i) = q4[i].ptPos;
			for(i=0; i<num3; i++)
				*(pTmpPt + num4 + i) = q3[i].ptPos;
			for(i=0; i<num2; i++)
				*(pTmpPt + num4 + num3 + i) = q2[i].ptPos;
			for(i=0; i<num1; i++)
				*(pTmpPt + num4 + num3 + num2 + i) = q1[i].ptPos;
		}
	}

	ptStart = *(ptList + ptLen);		// �Ӹ�����ʼ��˳ʱ������
	lNearest = 10000;
	for (i=0; i<ptLen; i++)
	{
		lPtDist = vDistance3L(ptStart, *(pTmpPt+i));
		if(lPtDist < lNearest)
		{
			lNearest = lPtDist;
			lIndex = i;
		}
	}
	for (i=0; i<ptLen-lIndex; i++)
	{
		*(ptList+i) = *(pTmpPt + i + lIndex);
	}
	for (i=0; i<lIndex; i++)
	{
		*(ptList + i + ptLen - lIndex) = *(pTmpPt + i);
	}			

EXT:
	FreeVectMem(hMemMgr, pTmpPt);
	return res;
}

//========================================================================
#define MIN_SIGMA 3
//======================== get result method for meters=====================================
// 1 ���μ���ÿ����̶ȵ㵽ָ�����յ�ľ��룬�����ؾ�������Ŀ̶ȵ�����ֵindex
// 2 a ���index=0 ���ʾָ����ָ����̶ȵ㸽�� ��ʱȡ��ʼ�����̶ȵ㣬�ֱ���������̶ȵ㵽ָ���߾��룬���ݱ�����ʽ���������
//    b ���indexΪ���ֵ ���ʾָ����ָ�����̶�ֵ��������ʱȡ��ĩβ�����̶ȵ㣬�ֱ���������̶ȵ㵽ָ���߾��룬���ݱ�����ʽ���������
//    c ����������ֱ��������̶ȵ������ȡһ���̶ȵ㣬���μ����������㵽ֱ�ߵľ���
//       �ж�index��Ӧ�������������Ƿ���ֱ�����࣬��������ģ���ȡ�����Ǹ���͵�ǰ�㣬���ݱ�����ʽ�������Ǳ������
//       ����������뵱ǰ����ֱ��ͬ�࣬�򷵻�index��Ӧ�Ŀ̶�ֵΪ�Ǳ����
MDouble CalcMeterResult(MPOINT *pPtPos, MDouble *pPtVal, LINEPARAM *pLineParam, MLong lPtNum)
{
	MDouble result;
	MLong lIndex;
	MDouble dMinDist, dTmpDist;
	MLong i;
	MDouble dAbsDist[3];
	MLong lDirection[3];
	MPOINT tmpPt;

	dMinDist = vDistance3D(*pPtPos, pLineParam->ptEnd);
	lIndex = 0;
	for (i=1; i<lPtNum; i++)
	{
		dTmpDist = vDistance3D(*(pPtPos+i), pLineParam->ptEnd);
		if (dTmpDist<dMinDist)
		{
			dMinDist = dTmpDist;
			lIndex = i;
		}
	}

	if (0==lIndex)
	{
		dAbsDist[0] = vPointDistToLine2D(*pPtPos, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		dAbsDist[1] = vPointDistToLine2D(*(pPtPos+1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		result = (dAbsDist[0]*pPtVal[1] + dAbsDist[1]*pPtVal[0])/(dAbsDist[0] + dAbsDist[1] + WYRD_EPSILON);
	}
	else if (lPtNum-1 == lIndex)
	{
		dAbsDist[0] = vPointDistToLine2D(*(pPtPos + lPtNum - 1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		dAbsDist[1] = vPointDistToLine2D(*(pPtPos + lPtNum - 2), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		result = (dAbsDist[0]*pPtVal[lPtNum - 2] + dAbsDist[1]*pPtVal[lPtNum - 1])/(dAbsDist[0] + dAbsDist[1] + WYRD_EPSILON);
	}
	else
	{
		// ��̶ȵ㵽ֱ�ߵľ���
		dAbsDist[0] = vPointDistToLine2D(*(pPtPos + lIndex), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		dAbsDist[1] = vPointDistToLine2D(*(pPtPos + lIndex - 1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		dAbsDist[2] = vPointDistToLine2D(*(pPtPos + lIndex + 1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		tmpPt.y = (MLong)((pPtPos+lIndex)->x * pLineParam->Coefk + pLineParam->Coefb + 0.5);
		if((pPtPos+lIndex)->y < tmpPt.y)
			lDirection[0] = -1;
		else
			lDirection[0] = 1;
		tmpPt.y = (MLong)((pPtPos+lIndex-1)->x * pLineParam->Coefk + pLineParam->Coefb + 0.5);
		if((pPtPos+lIndex-1)->y < tmpPt.y)
			lDirection[1] = -1;
		else
			lDirection[1] = 1;
		tmpPt.y = (MLong)((pPtPos+lIndex+1)->x * pLineParam->Coefk + pLineParam->Coefb + 0.5);
		if((pPtPos+lIndex+1)->y < tmpPt.y)
			lDirection[2] = -1;
		else
			lDirection[2] = 1;

		if (lDirection[0]*lDirection[1]<0)  // �ж�������ֱ������
		{
			result = (dAbsDist[0]*pPtVal[lIndex-1] + dAbsDist[1]*pPtVal[lIndex])/(dAbsDist[0] + dAbsDist[1] + WYRD_EPSILON);
		}
		else if (lDirection[0]*lDirection[2]<0)
		{
			result = (dAbsDist[0]*pPtVal[lIndex+1] + dAbsDist[2]*pPtVal[lIndex])/(dAbsDist[0] + dAbsDist[2] + WYRD_EPSILON);
		}
		else
		{
			result = pPtVal[lIndex];
		}
	}

	return result;
}

//  ָ��ʽ����״̬�ж�
//  1 ��������״̬�㵽ָ���ߵľ���
//  2 ѡȡ��ֱ�߾����С��״̬��Ϊ����״̬
MDouble GetSwitchStatus(MPOINT *pPtPos, MDouble *pPtVal, LINEPARAM *pLineParam, MLong lPtNum)
{
	MDouble result;
	MDouble pt2lineDist[2];

	if(MNull == pPtPos || lPtNum<2)
	{
		Err_Print("GetSwitchStatus error !\n");
		return 0;
	}

	//pt2lineDist[0] = vPointDistToLine2(*pPtPos, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
	//pt2lineDist[1] = vPointDistToLine2(*(pPtPos+1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);

	// �㵽ֱ�߾���
	pt2lineDist[0] = vPointDistToLine2D(*pPtPos, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
	pt2lineDist[1] = vPointDistToLine2D(*(pPtPos+1), pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
	/*pt2lineDist[0] = (MDouble)(abs(pPtPos->x - pLineParam->ptEnd.x));
	pt2lineDist[1] = (MDouble)(abs((pPtPos+1)->x - pLineParam->ptEnd.x));*/

	if (pt2lineDist[0] <= pt2lineDist[1])	// near off status
	{
		result = *pPtVal;
	}
	else									// near on status
	{
		result = *(pPtVal + 1);
	}

	return result;
}

//=============================================================================
// ����maskģ�壬����Ӧͼ�С������������򡰵���Ӧ��������ι���
MRESULT getMaskResponse(BLOCK *pResponseImage, BLOCK *pMaskImage)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight, lStride, lMaskStride;
	MLong lExt, lMaskExt;
	MLong i, j;
	MByte *pResponseData = MNull, *pMaskData = MNull;

	lWidth = pResponseImage->lWidth;
	lHeight = pResponseImage->lHeight;
	lStride = pResponseImage->lBlockLine;
	lMaskStride = pMaskImage->lBlockLine;
	lExt = lStride - lWidth;
	lMaskExt = lMaskStride - lWidth;
	pResponseData = pResponseImage->pBlockData;
	pMaskData = pMaskImage->pBlockData;

	for (j=0; j<lHeight; j++, pResponseData+=lExt, pMaskData+=lMaskExt)
	{
		for (i=0; i<lWidth; i++, pResponseData++, pMaskData++)
		{
			if (255!=*pMaskData)
			{
				*pResponseData = 0;
			}
		}
	}

	return res;
}

// ����㵽ֱ�ߵľ��룬û������һ������
MDouble Point2Line(MPOINT pt, MDouble dCoeffK, MDouble dCoeffB, MLong bVert)
{
	if (bVert)
	{
		return (pt.x + dCoeffB);//����Ҫע�⣬���ǲ��õ���0 = -x+b�ķ���
	}
	return (pt.x*dCoeffK+dCoeffB-pt.y);
}

// ����ֱ����Բ�Ľ���
MLong calcLineCrossCirclePt(CIRCLE *pCircleParam, LINEPARAM *pLineParam, MPOINT *ptCross1, MPOINT *ptCross2)	 //???
{
	MLong res = 0;

	MDouble dAngle;
	MPOINT ptCenter;
	MDouble dist2Center, absDist2Center;
	MDouble halfChord, tmpSqare;
	MLong deltaX, deltaY;
	MPOINT ptMid;
	MLong isCenterOnRight;
	MPOINT tmpPt;

	if (pLineParam->bVertical)
	{
		ptCenter.x = pCircleParam->xcoord;
		ptCenter.y = pCircleParam->ycoord;
		dist2Center = Point2Line(ptCenter, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		absDist2Center = fabs(dist2Center);  //Բ�ĵ�ֱ�ߵľ���
	}
	else
	{
		ptCenter.x = pCircleParam->xcoord;
		ptCenter.y = pCircleParam->ycoord;
		// ������ֱ���ϣ���Բ��x�������Ӧ����yֵ�������ж�ֱ����Բ���Ϸ������·�
		tmpPt.x = ptCenter.x;
		tmpPt.y = (MLong)(pLineParam->Coefk * tmpPt.x + pLineParam->Coefb + 0.5); 

		dAngle = atan(pLineParam->Coefk);
		dist2Center = Point2Line(ptCenter, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		tmpSqare = SQARE(pLineParam->Coefk) + 1;
		dist2Center = dist2Center / (sqrt(tmpSqare) + WYRD_EPSILON);
		absDist2Center = fabs(dist2Center);  // ����ֱ�ߵ�Բ�ĵľ���
	}

	if(absDist2Center > pCircleParam->lRadius) // ���ֱ�ߵ�Բ�ĵľ������Բ�뾶�����ֱ����Բ�����ڽ���
	{
		res = -1;
		Err_Print("calcLineCrossCirclePt : absDist2Center > circleParam.lRadius error!\n");
		return res;
	}
	// ֱ����Բ�ཻ����Բ�ڲ���ֱ�� ����ΪԲ�ϵ�һ���ң�����ֱ�������Σ�������ҳ�
	tmpSqare = SQARE(pCircleParam->lRadius) - SQARE(absDist2Center);
	halfChord = sqrt(tmpSqare);  // ���ҳ�
	if (pLineParam->bVertical)  // ��ֱ����������Բ�ģ��ӣ��������ҳ������ɻ����Բ�����y���ꣻx�����ͨ��ֱ�߷���ȷ��
	{
		ptCross1->x = (MLong)(-pLineParam->Coefb);
		ptCross1->y = (MLong)(ptCenter.y - halfChord);
		ptCross2->x = (MLong)(-pLineParam->Coefb);
		ptCross2->y = (MLong)(ptCenter.y + halfChord);
	}
	else if (pLineParam->Coefk <= 0)  // б��С����
	{
		deltaX = (MLong)(fabs(absDist2Center * sin(dAngle)) + 0.5);
		deltaY = (MLong)(fabs(absDist2Center * cos(dAngle)) + 0.5);

		if(ptCenter.y < tmpPt.y) //Բ����ֱ���·�
			isCenterOnRight = 0;
		else
			isCenterOnRight = 1;  // Բ����ֱ���Ϸ�

		if (isCenterOnRight == 1)
		{
			ptMid.x = ptCenter.x - deltaX;
			ptMid.y = ptCenter.y - deltaY;

			// �԰���Ϊб�ߣ�ֱ�߷���Ϊֱ�������ε�һ���ڽǣ���������ֱ�Ǳ߳���
			deltaX = (MLong)(fabs(halfChord * cos(dAngle)) + 0.5);
			deltaY = (MLong)(fabs(halfChord * sin(dAngle)) + 0.5);

			// ��������ֱ�Ǳߺ��ҵ��е㣬����ֱ����Բ�Ľ���
			ptCross1->x = ptMid.x + deltaX;
			ptCross1->y = ptMid.y - deltaY;

			ptCross2->x = ptMid.x - deltaX;
			ptCross2->y = ptMid.y + deltaY;
		}
		else
		{
			ptMid.x = ptCenter.x + deltaX;
			ptMid.y = ptCenter.y + deltaY;

			deltaX = (MLong)(fabs(halfChord * cos(dAngle)) + 0.5);
			deltaY = (MLong)(fabs(halfChord * sin(dAngle)) + 0.5);

			ptCross1->x = ptMid.x + deltaX;
			ptCross1->y = ptMid.y - deltaY;

			ptCross2->x = ptMid.x - deltaX;
			ptCross2->y = ptMid.y + deltaY;
		}
	}
	else
	{
		deltaX = (MLong)(fabs(absDist2Center * sin(dAngle)) + 0.5);
		deltaY = (MLong)(fabs(absDist2Center * cos(dAngle)) + 0.5);

		if(ptCenter.y < tmpPt.y)
			isCenterOnRight = 1;
		else
			isCenterOnRight = 0;

		if (isCenterOnRight==0)
		{
			ptMid.x = ptCenter.x + deltaX;
			ptMid.y = ptCenter.y - deltaY;

			deltaX = (MLong)(fabs(halfChord * cos(dAngle)) + 0.5);
			deltaY = (MLong)(fabs(halfChord * sin(dAngle)) + 0.5);

			ptCross1->x = ptMid.x - deltaX;
			ptCross1->y = ptMid.y - deltaY;

			ptCross2->x = ptMid.x + deltaX;
			ptCross2->y = ptMid.y + deltaY;
		}
		else
		{
			ptMid.x = ptCenter.x - deltaX;
			ptMid.y = ptCenter.y + deltaY;

			deltaX = (MLong)(fabs(halfChord * cos(dAngle)) + 0.5);
			deltaY = (MLong)(fabs(halfChord * sin(dAngle)) + 0.5);

			ptCross1->x = ptMid.x - deltaX;
			ptCross1->y = ptMid.y - deltaY;

			ptCross2->x = ptMid.x + deltaX;
			ptCross2->y = ptMid.y + deltaY;
		}
	}

	return res;
}

// �ڰ׵�����¼����ɫ���ͼ  YUV444
MRESULT getRedDiffImage_yuv(MByte *pSrcY, MByte *pSrcV, BLOCK *pDiffImage, MLong *pHistArray)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pDstData = MNull;
	MLong lWidth, lHeight;
	MLong lExt;
	MLong lMaxNum, lMaxRedNum;
	MLong lMaxVal, lThresVal, lTmpVal, lRedThresVal;
	MLong redHist[256] = {0};

	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	pDstData = (MByte*)pDiffImage->pBlockData;
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - pDiffImage->lWidth;
	lMaxNum = (lWidth * lHeight)>>1;	// whb(50%) /8 /4 /2
	lMaxRedNum = (lWidth * lHeight)>>6;		//  1/32 1/64

	lMaxVal = 1;
	// ֱ��ͼͳ��
	for (i=0; i<lHeight; i++, pSrcYData+=lExt, pSrcVData+=lExt)
	{
		for (j=0; j<lWidth; j++, pSrcYData++, pSrcVData++)
		{
			if(lMaxVal < *pSrcYData)
				lMaxVal = *pSrcYData;
			redHist[*pSrcVData]++;
			pHistArray[*pSrcYData]++;
		}
	}

	// ��λ��ֵ����
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxRedNum)
		{
			lTmpVal += redHist[i];
		}
		else
			break;
	}
	lRedThresVal = i - 1;
	lTmpVal = 0;
	for (i=0; i<256; i++)
	{
		if (lTmpVal < lMaxNum)
		{
			lTmpVal += pHistArray[i];
		}
		else
			break;
	}
	lThresVal = i - 1;

	// ��һ�������ɫ���ͼ
	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	for (i=0; i<lHeight; i++, pSrcYData+=lExt, pSrcVData+=lExt, pDstData+=lExt)
	{
		for (j=0; j<lWidth; j++, pSrcYData++, pSrcVData++, pDstData++)
		{
			if (*pSrcYData<lThresVal && *pSrcVData>lRedThresVal)
			{
				*pDstData = (MByte)(64 - (*pSrcVData<<6) / lMaxVal);
				//*pSrcVData = 0;
			}
			else
			{
				*pDstData = 128;
				//*pSrcVData = 128;
			}
		}
	}

	return res;
}

// ����˫ָ������еĺ�ɫָ���ߣ�����������Ϣ���к�ɫ���ͼ����
MRESULT getBlackDiffImage_yuv(MByte *pSrcY, MByte *pSrcV, MByte *pSrcU, BLOCK *pDiffImage, MLong *pHistArray)
{
	MRESULT res = LI_ERR_NONE;

	MLong i,j;
	MLong lMaxNum, lTmpNum;
	MLong lThresVal;
	MLong lWidth, lHeight, lExt;
	MLong lMaxVal = 1;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pSrcUdata = MNull;
	MByte *pDstData = MNull;

	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - lWidth;
	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	pSrcUdata = pSrcU;
	pDstData = (MByte *)pDiffImage->pBlockData;

	// ��λ��ֵ����
	lMaxNum = (lWidth * lHeight)>>3;	// >>3  (0.4 whb)
	lTmpNum = 0;
	for (i=0; i<256; i++)
	{
		if(lTmpNum<lMaxNum)
			lTmpNum += pHistArray[i];
		else
			break;
	}
	lThresVal = i - 1;

	//  U - V
	for (i=0; i<lHeight; i++, pSrcVData+=lExt, pSrcUdata+=lExt, pDstData+=lExt)
	{
		for (j=0; j<lWidth; j++, pSrcVData++, pSrcUdata++, pDstData++)
		{
			*pDstData = ABS(*pSrcUdata - *pSrcVData);
			if(lMaxVal < *pDstData)
				lMaxVal = *pDstData;
		}
	}

	pDstData = (MByte *)pDiffImage->pBlockData;
	for (i=0; i<lHeight; i++, pSrcYData+=lExt, pDstData+=lExt)
	{
		for (j=0; j<lWidth; j++, pSrcYData++, pDstData++)
		{
			if(*pSrcYData<lThresVal)
				*pDstData = (MByte)((*pDstData * 128) / lMaxVal);
			else
				*pDstData = 128;
		}
	}

	return res;
}

// YUV �ռ����ڰ׵�ͼ���м����ɫ���ͼ  YUV422  ע��׵� ���߲��ֵ�ֵС���������� ��������Ϊ128
// ��ɫ��ֵMaxRedNumҪС;������ֵMaxNumҪ��,���߲��ֵ�����ֵҪС����ֵ
MRESULT getRedDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)

{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, tmp;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pDstData = MNull;
	MByte *pDstPtr = MNull;
	MLong lWidth, lHeight;
	MLong lExt;
	MLong lMaxNum, lMaxRedNum;
	MLong lMaxVal, lThresVal, lTmpVal, lRedThresVal;
	MLong redHist[256] = {0};

	pDstPtr = (MByte*)pDiffImage->pBlockData;
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - pDiffImage->lWidth;
	lMaxNum = (lWidth * lHeight)>>3;     // whb(50%) /8 /4 /2
	lMaxNum=lMaxNum*3;
	lMaxRedNum = (lWidth * lHeight)>>5;                //  1/32 1/64
	lMaxRedNum=lMaxRedNum*1;
	lMaxVal = 1;

	// ���ȷ���ֱ��ͼͳ��
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		for (j=0; j<lWidth; j++)
		{
			pHistArray[*pSrcYData]++;
			pSrcYData++;
		}

	}

	// ��ɫ����ֱ��ͼͳ��
	for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;

		for (j=0; j<(lWidth>>1); j++)
		{
			if(lMaxVal < *pSrcVData)
				lMaxVal = *pSrcVData;
			redHist[*pSrcVData]+=4;
			pSrcVData += 2;
		}
	}

	// ��ɫ��λ��ֵ����
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxRedNum)
		{
			lTmpVal += redHist[i];
		}
		else
			break;
	}
	lRedThresVal = i - 1;

	lTmpVal = 0;
	for (i=0; i<256; i++)
	{
		if (lTmpVal < lMaxNum)
		{
			lTmpVal += pHistArray[i];
		}
		else
			break;
	}
	lThresVal = i - 1;

	// ��һ����ɫ���
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		pSrcVData = pSrcV + (i>>1)*lSrcStride;
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;

		for (j=0; j<lWidth; j++)
		{
			if (*pSrcYData<lThresVal && *pSrcVData>lRedThresVal)
			{
				*pDstData = (MByte)(64 -(*pSrcVData<<6) / lMaxVal);
			}
			else
			{
				*pDstData = 128;
			}		

			pSrcYData++;
			pDstData++;               
			tmp = j&1 ? 2:0;
			pSrcVData += tmp; 
		}
	}

	return res;
}
MRESULT getWhiteDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MByte *pSrcU, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)
{

	MRESULT res = LI_ERR_NONE;

	MLong i,j,tmp;
	MLong lMaxNum, lTmpNum, lMaxRedNum,lTmpVal,lRedThresVal;
	MLong lThresVal;
	MLong lWidth, lHeight, lExt;
	MLong lMaxVal = 1;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pSrcUdata = MNull;
	MByte *pDstData = MNull;
	MByte *pDstPtr = MNull;

	MLong redHist[256] = {0};
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - lWidth;
	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	pSrcUdata = pSrcU;

	pDstPtr = (MByte*)pDiffImage->pBlockData;

	lMaxNum = (lWidth * lHeight)>>3;     // >>3  (0.4 whb)
	lMaxRedNum = (lWidth * lHeight)>>5; 
	//lMaxNum = lWidth * lHeight-((lWidth * lHeight)>>3);

	// ���ȷ���ֱ��ͼͳ��
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		for (j=0; j<lWidth; j++)
		{
			if(lMaxVal < *pSrcYData)
				lMaxVal = *pSrcYData;
			pHistArray[*pSrcYData]++;
			pSrcYData++;
		}

	}

	// ��ɫ����ֱ��ͼͳ��
	for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;

		for (j=0; j<(lWidth>>1); j++)
		{
			/*if(lMaxVal < *pSrcVData)
				lMaxVal = *pSrcVData;*/
			redHist[*pSrcVData]+=4;
			pSrcVData += 2;
		}
	}

	// ��ɫ��λ��ֵ����
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxRedNum)
		{
			lTmpVal += redHist[i];
		}
		else
			break;
	}
	lRedThresVal = i - 1;

	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxNum)
		{
			lTmpVal += pHistArray[i];
		}
		else
			break;
	}
	lTmpNum = 0;
	for (i=255; i>=0; i--)
	{
		if(lTmpNum<lMaxNum)
			lTmpNum += pHistArray[i];
		else
			break;
	}
	lThresVal = i - 1;

	/*for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;
		pSrcUdata = pSrcU + i*lSrcStride;
		pDstData = pDstPtr + (i<<1)*pDiffImage->lBlockLine;

		for (j=0; j<(lWidth>>1); j++)
		{
			*pDstData = ABS(*pSrcUdata - *pSrcVData);
			*(pDstData+1) = *(pDstData+pDiffImage->lBlockLine)
				= *(pDstData+pDiffImage->lBlockLine+1) = *pDstData;
			if(lMaxVal < *pDstData)
				lMaxVal = *pDstData;

			pSrcUdata += 2;
			pSrcVData += 2;
			pDstData += 2;
		}
	}*/

	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		pSrcVData = pSrcV + (i>>1)*lSrcStride;;
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;
		for (j=0; j<lWidth; j++)
		{
			if(*pSrcYData>lThresVal && *pSrcVData<lRedThresVal)
				*pDstData = (MByte)(64 +(*pSrcYData<<6) / lMaxVal);
			else
				*pDstData = 0;
			pSrcYData++;
			pDstData++;
			tmp = j&1 ? 2:0;
			pSrcVData += tmp; 
		}
	}

	return res;
}

// ��ɫ������ɫֱ��ͼ���㣬����ͼ������  YUV422  ��ɫ���ֵ�����ֵ ��ɫָ���ֶ������ ���̲��ֱ���Ϊ128 ������ֵ��Խϸ� maxnum ҪС
MRESULT getBlackDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MByte *pSrcU, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)
{

	MRESULT res = LI_ERR_NONE;

	MLong i,j,tmp;
	MLong lMaxNum, lTmpNum, lMaxRedNum,lTmpVal,lRedThresVal;
	MLong lThresVal;
	MLong lWidth, lHeight, lExt;
	MLong lMaxVal = 1;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pSrcUdata = MNull;
	MByte *pDstData = MNull;
	MByte *pDstPtr = MNull;

	MLong redHist[256] = {0};
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - lWidth;
	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	pSrcUdata = pSrcU;

	pDstPtr = (MByte*)pDiffImage->pBlockData;

	lMaxNum = (lWidth * lHeight)>>3;     // >>3  (0.4 whb)
	lMaxRedNum = (lWidth * lHeight)>>5; 
	//lMaxNum = lWidth * lHeight-((lWidth * lHeight)>>3);

	// ���ȷ���ֱ��ͼͳ��
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		for (j=0; j<lWidth; j++)
		{
			pHistArray[*pSrcYData]++;
			pSrcYData++;
		}

	}

	// ��ɫ����ֱ��ͼͳ��
	for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;

		for (j=0; j<(lWidth>>1); j++)
		{
			if(lMaxVal < *pSrcVData)
				lMaxVal = *pSrcVData;
			redHist[*pSrcVData]+=4;
			pSrcVData += 2;
		}
	}

	// ��ɫ��λ��ֵ����
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxRedNum)
		{
			lTmpVal += redHist[i];
		}
		else
			break;
	}
	lRedThresVal = i - 1;

	lTmpVal = 0;
	for (i=0; i<256; i++)
	{
		if (lTmpVal < lMaxNum)
		{
			lTmpVal += pHistArray[i];
		}
		else
			break;
	}
	lTmpNum = 0;
	for (i=0; i<256; i++)
	{
		if(lTmpNum<lMaxNum)
			lTmpNum += pHistArray[i];
		else
			break;
	}
	lThresVal = i - 1;

	for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;
		pSrcUdata = pSrcU + i*lSrcStride;
		pDstData = pDstPtr + (i<<1)*pDiffImage->lBlockLine;

		for (j=0; j<(lWidth>>1); j++)
		{
			*pDstData = ABS(*pSrcUdata - *pSrcVData);
			*(pDstData+1) = *(pDstData+pDiffImage->lBlockLine)
				= *(pDstData+pDiffImage->lBlockLine+1) = *pDstData;
			if(lMaxVal < *pDstData)
				lMaxVal = *pDstData;

			pSrcUdata += 2;
			pSrcVData += 2;
			pDstData += 2;
		}
	}

	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		pSrcVData = pSrcV + (i>>1)*lSrcStride;;
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;
		for (j=0; j<lWidth; j++)
		{
			if(*pSrcYData<lThresVal && *pSrcVData<lRedThresVal)
				*pDstData = (MByte)((*pDstData * 128) / lMaxVal);
			else
				*pDstData = 128;
			pSrcYData++;
			pDstData++;
			tmp = j&1 ? 2:0;
			pSrcVData += tmp; 
		}
	}

	return res;
}

///�ںڵ�ͼ���м����ɫָ���ߣ���׵ײ�ͬ���Ǻڵ׺�ɫָ���ߵ����ȴ��ڱ���,����ɫ����Ϊ0
///ͬʱ��׵ײ�ͬ,MaxNum��ֵҪ���󣬰�����ɫ����Ŀ
MRESULT getRedDiffImage2_yuvblack(MByte *pSrcY, MByte *pSrcV, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)

{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, tmp;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pDstData = MNull;
	MByte *pDstPtr = MNull;
	MLong lWidth, lHeight;
	MLong lExt;
	MLong lMaxNum, lMaxRedNum;
	MLong lMaxVal, lThresVal, lTmpVal, lRedThresVal;
	MLong redHist[256] = {0};

	pDstPtr = (MByte*)pDiffImage->pBlockData;
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - pDiffImage->lWidth;
	lMaxNum = lWidth * lHeight-((lWidth * lHeight)>>3);     // whb(50%) /8 /4 /2
	lMaxRedNum = (lWidth * lHeight)>>5;                //  1/32 1/64
	lMaxVal = 1;

	// ���ȷ���ֱ��ͼͳ��
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		for (j=0; j<lWidth; j++)
		{
			pHistArray[*pSrcYData]++;
			pSrcYData++;
		}

	}

	// ��ɫ����ֱ��ͼͳ��
	for (i=0; i<(lHeight>>1); i++)
	{
		pSrcVData = pSrcV + i*lSrcStride;

		for (j=0; j<(lWidth>>1); j++)
		{
			if(lMaxVal < *pSrcVData)
				lMaxVal = *pSrcVData;
			redHist[*pSrcVData]+=4;
			pSrcVData += 2;
		}
	}

	// ��ɫ��λ��ֵ����
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if (lTmpVal < lMaxRedNum)
		{
			lTmpVal += redHist[i];
		}
		else
			break;
	}
	lRedThresVal = i - 1;

	lTmpVal = 0;
	for (i=0; i<256; i++)
	{
		if (lTmpVal < lMaxNum)
		{
			lTmpVal += pHistArray[i];
		}
		else
			break;
	}
	lThresVal = i - 1;

	// ��һ����ɫ���
	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		pSrcVData = pSrcV + (i>>1)*lSrcStride;
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;

		for (j=0; j<lWidth; j++)
		{
			if (*pSrcYData<lThresVal && *pSrcVData>lRedThresVal)
			{
				*pDstData = (MByte)(64 +(*pSrcVData<<6) / lMaxVal);
			}
			else
			{
				*pDstData = 0;     //����ɫ�ǵ͵� ������Ҫ��Ϊ0 
			}		

			pSrcYData++;
			pDstData++;               
			tmp = j&1 ? 2:0;
			pSrcVData += tmp; 
		}
	}

	return res;
}


// ��YUV422�� U V ����ͼ������չ��ʹ����ԭͼ��ߴ��Сһ��
MRESULT calcExternImg(MByte *pSrc, MLong lSrcStride, MByte *pDst, MLong lDstStride, MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lSrcExt, lDstExt;
	MByte *pSrcData = MNull;
	MByte *pDstData00 = MNull;
	MByte *pDstData01 = MNull;
	MByte *pDstData10 = MNull;
	MByte *pDstData11 = MNull;

	//init
	pSrcData = pSrc;
	pDstData00 = pDst;
	pDstData01 = pDst + 1;
	pDstData10 = pDst + lDstStride;
	pDstData11 = pDst + lDstStride + 1;

	lSrcExt = lSrcStride - lWidth;
	lDstExt = lDstStride - (lWidth<<1) + lDstStride;

	//calc
	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData00+=lDstExt, pDstData01+=lDstExt, pDstData10+=lDstExt, pDstData11+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData++, pDstData00+=2, pDstData01+=2, pDstData10+=2, pDstData11+=2)
		{
			*pDstData00 = *pSrcData;
			*pDstData01 = *pSrcData;
			*pDstData10 = *pSrcData;
			*pDstData11 = *pSrcData;
		}
	}

	return res;
}

// YUV��ɫ�ռ��£���ȡĿ���������
MRESULT getObjImage_yuv(MHandle hMemMgr, JOFFSCREEN srcImage, BLOCK *pDstBlockY, BLOCK *pDstBlockU, BLOCK *pDstBlockV, MLong lObjLeft, MLong lObjTop)
{
	MRESULT res = LI_ERR_NONE;

	int i;
	JOFFSCREEN ImgSrcYUV420 = {0};
	JOFFSCREEN yuvTmp = {0};
	BLOCK ImgSrcV = {0};
	BLOCK ImgSrcU = {0};
	MByte *pSrcYData, *pSrcUData, *pSrcVData,*pDstYData, *pDstUData, *pDstVData;

	if (MNull==pDstBlockY || MNull==pDstBlockU || MNull==pDstBlockV)
	{
		Err_Print("getObjImage_yuv input error !\n");
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	GO(ImgCreate(hMemMgr, &ImgSrcYUV420, FORMAT_YUV420PLANAR, srcImage.dwWidth, srcImage.dwHeight));
	GO(ImgFmtTrans(&srcImage, &ImgSrcYUV420));
	yuvTmp = ImgSrcYUV420;
	ImgChunky2Plannar(&yuvTmp);
	// extern U V
	GO(B_Create(hMemMgr, &ImgSrcU, DATA_U8, srcImage.dwWidth, srcImage.dwHeight));
	GO(B_Create(hMemMgr, &ImgSrcV, DATA_U8, srcImage.dwWidth, srcImage.dwHeight));
	calcExternImg((MByte*)yuvTmp.pixelArray.planar.pPixel[1], yuvTmp.pixelArray.planar.dwImgLine[1],
		(MByte*)ImgSrcU.pBlockData, ImgSrcU.lBlockLine, srcImage.dwWidth>>1, srcImage.dwHeight>>1);
	calcExternImg((MByte*)yuvTmp.pixelArray.planar.pPixel[2], yuvTmp.pixelArray.planar.dwImgLine[2],
		(MByte*)ImgSrcV.pBlockData, ImgSrcV.lBlockLine, srcImage.dwWidth>>1, srcImage.dwHeight>>1);

	pSrcYData = (MByte*)yuvTmp.pixelArray.planar.pPixel[0] + yuvTmp.pixelArray.planar.dwImgLine[0] * lObjTop + lObjLeft;
	pSrcUData = (MByte*)ImgSrcU.pBlockData + ImgSrcU.lBlockLine * lObjTop + lObjLeft;
	pSrcVData = (MByte*)ImgSrcV.pBlockData + ImgSrcV.lBlockLine * lObjTop + lObjLeft;
	pDstYData = (MByte*)pDstBlockY->pBlockData;
	pDstUData = (MByte*)pDstBlockU->pBlockData;
	pDstVData = (MByte*)pDstBlockV->pBlockData;

	for (i=0; i<pDstBlockY->lHeight; i++)
	{
		JMemCpy(pDstYData, pSrcYData, pDstBlockY->lBlockLine);
		JMemCpy(pDstUData, pSrcUData, pDstBlockU->lBlockLine);
		JMemCpy(pDstVData, pSrcVData, pDstBlockV->lBlockLine);

		pSrcYData += yuvTmp.pixelArray.planar.dwImgLine[0];
		pSrcUData += ImgSrcU.lBlockLine;
		pSrcVData += ImgSrcV.lBlockLine;
		pDstYData += pDstBlockY->lBlockLine;
		pDstUData += pDstBlockU->lBlockLine;
		pDstVData += pDstBlockV->lBlockLine;
	}

EXT:
	ImgRelease(hMemMgr, &ImgSrcYUV420);
	B_Release(hMemMgr, &ImgSrcU);
	B_Release(hMemMgr, &ImgSrcV);
	return res;
}

// ����ͼ��ǶȾ���
MRESULT calcAngleMat(MByte *pSrc, MLong lWidth, MLong lHeight, MLong lStride, MDouble *pDstAngle)
{
	MRESULT res = LI_ERR_NONE;

	MLong xx, yy;
	MByte *pA, *pB, *pC, *pD, *pE, *pF, *pG, *pH;
	MDouble *pAngleData;
	MDouble tmpData;
	MLong dx, dy;
	MLong lExt;

	lExt = lStride - lWidth + 2;

	//  pA  pB  pC
	//  pH      pD
	//  pG  pF  pE
	pA = pSrc;
	pB = pSrc + 1;
	pC = pSrc + 2;
	pD = pSrc + lStride + 2;
	pE = pSrc + (lStride<<1) + 2;
	pF = pSrc + (lStride<<1) + 1;
	pG = pSrc + (lStride<<1);
	pH = pSrc + lStride;

	pAngleData = pDstAngle + lStride + 1;

	for (yy=1; yy<lHeight-1; yy++)
	{
		for (xx=1; xx<lWidth-1; xx++)
		{
			dx = (*pC + ((*pD)<<1) + *pE) - (*pA + ((*pH)<<1) + *pG);
			dy = (*pG + ((*pF)<<1) + *pE) - (*pA + ((*pB)<<1) + *pC);
			if (0==dx)
			{
				*pAngleData = 90;
			}
			else
			{
				tmpData = (MDouble)atan2(dy, dx) * RAD2ANG;
				*pAngleData = (tmpData > 0 ? tmpData : tmpData+360);
			}

			pA++;
			pB++;
			pC++;
			pD++;
			pE++;
			pF++;
			pG++;
			pH++;
			pAngleData++;
		}
		pA += lExt;
		pB += lExt;
		pC += lExt;
		pD += lExt;
		pE += lExt;
		pF += lExt;
		pG += lExt;
		pH += lExt;
		pAngleData += lExt;
	}

	return res;
}
// �жϿ̶ȵ��Ƿ���԰��
MLong isPtInImage(MPOINT pt, MLong lWidth, MLong lHeight)
{
	if (pt.x>=0 && pt.x<lWidth && pt.y>=0 && pt.y<lHeight)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// ͼ������
MRESULT strechImage(BLOCK *pImage)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MByte *pData;
	MLong lMinVal, lMaxVal, lTmpVal, lPercentMaxVal, lPercentMinVal;
	MLong i, j;
	MLong lDelta;
	MDouble dCoeffK;

	lWidth = pImage->lWidth;
	lHeight = pImage->lHeight;
	lStride = pImage->lBlockLine;
	lExt = lStride - lWidth;
	pData = pImage->pBlockData;
	lMinVal = 255;
	lMaxVal = 0;

	// get the maxVal and minVal of the image
	for (j=0; j<lHeight; j++, pData+=lExt)
	{
		for (i=0; i<lWidth; i++, pData++)
		{
			if (*pData < lMinVal)
			{
				lMinVal = *pData;
			}
			if (*pData > lMaxVal)
			{
				lMaxVal = *pData;
			}
		}
	}

	// particle strech
	lPercentMaxVal = lMaxVal * 9 / 10;
	lPercentMinVal = lMinVal * 6 / 5;
	if (lMinVal >= lMaxVal)
	{
		return res;
	}
	lDelta = lMaxVal - lPercentMaxVal;
	dCoeffK = (MDouble)(255-lMaxVal) / lDelta;
	pData = pImage->pBlockData;
	for (j=0; j<lHeight; j++, pData+=lExt)
	{
		for (i=0; i<lWidth; i++, pData++)
		{
			if (*pData <= lPercentMinVal)
			{
				*pData = 255 - *pData;
			}
			else if (*pData > lPercentMaxVal)
			{
				lDelta = *pData - lPercentMaxVal;
				lTmpVal = (MLong)(dCoeffK * lDelta + lPercentMaxVal);
				*pData = (MByte)lTmpVal;
			}
		}
	}

	return res;
}
// ͼ��ȡ��
MRESULT inverseImg(BLOCK *pImage)
{
	MRESULT res = LI_ERR_NONE;
	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MByte *pData;
	MLong i, j;

	pData = pImage->pBlockData;
	lWidth = pImage->lWidth;
	lHeight = pImage->lHeight;
	lStride = pImage->lBlockLine;
	lExt = lStride - lWidth;

	for (j=0; j<lHeight; j++, pData+=lExt)
	{
		for (i=0; i<lWidth; i++, pData++)
		{
			if (*pData)
			{
				*pData = 255 - *pData;
			}
		}
	}

	return res;
}
MRESULT filterImg(BLOCK *pImage,LINEPARAM *lineParam,MLong lHaarWidth)
{
	MRESULT res = LI_ERR_NONE;
	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MByte *pData;
	MLong i, j;
	MDouble distance;
	MPOINT tmpPt;
	pData = pImage->pBlockData;
	lWidth = pImage->lWidth;
	lHeight = pImage->lHeight;
	lStride = pImage->lBlockLine;
	lExt = lStride - lWidth;

	for (j=0; j<lHeight; j++, pData+=lExt)
	{
		for (i=0; i<lWidth; i++, pData++)
		{
			if (*pData)
			{
				tmpPt.x=i;
				tmpPt.y=j;
				distance = vPointDistToLine2(tmpPt, lineParam->Coefk, lineParam->Coefb,lineParam->bVertical);
				if(distance<=lHaarWidth*4/3)
				{
					*pData=50;
				}
			}
		}
	}

	return res;
}

// 8�����������ص㣬����������ʱ��ֹͣ������������������
MRESULT searchPt(BLOCK *pSrcImg, MPOINT seedPt, MLong lDirection, MLong lSearchThreshold, MPOINT *dstPt, MLong *lLength)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight, lStride;
	MLong xx, yy;
	MLong lStep;
	MByte *pData;

	lWidth = pSrcImg->lWidth;
	lHeight = pSrcImg->lHeight;
	lStride = pSrcImg->lBlockLine;
	pData = pSrcImg->pBlockData;

	if (seedPt.x<0 || seedPt.x>=lWidth || seedPt.y<0 || seedPt.y>=lHeight || MNull==pData)
	{
		JPrintf("error input!\n");
		res = LI_ERR_NOT_INIT;
		return res;
	}

	xx = seedPt.x;
	yy = seedPt.y;
	lStep = 0;
	switch(lDirection)
	{
	case 0:		//right
		{
			while(xx<lWidth && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx++;
					lStep++;
				}
			}
			break;
		}
	case 1:		//rightUp
		{
			while(xx<lWidth && yy>=0 && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx++;
					yy--;
					lStep++;
				}
			}
			break;
		}
	case 2:		// up
		{
			while(yy>=0 && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					yy--;
					lStep++;
				}
			}
			break;
		}
	case 3:		// leftUp
		{
			while(xx>=0  && yy>=0 && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx--;
					yy--;
					lStep++;
				}
			}
			break;
		}
	case 4:		// left
		{
			while(xx>=0 && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx--;
					lStep++;
				}
			}
			break;
		}
	case 5:		// leftDown
		{
			while(xx>=0 && yy<lHeight && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx--;
					yy++;
					lStep++;
				}
			}
			break;
		}
	case 6:		// down
		{
			while(yy<lHeight && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					yy++;
					lStep++;
				}
			}
			break;
		}
	case 7:		// rightDown
		{
			while(xx<lWidth && yy<lHeight && lStep<lSearchThreshold)
			{
				if (0==*(pData + yy*lStride + xx))
				{
					break;
				}
				else
				{
					xx++;
					yy++;
					lStep++;
				}
			}
			break;
		}
	default:
		{
			break;;
		}
	}

	dstPt->x = xx;
	dstPt->y = yy;
	*lLength = lStep;

	return res;
}

// Ѱ��ֱ��������������Ӧֵ����������ص�������
// �����������ˮƽ�����ֱ���� ��������ֱ�ߵľ���
MLong calcMaxContinuityLength(MByte *pData, MLong lStride, MLong lWidth, MLong lHeight, 
	MPOINT ptCross1, MPOINT ptCross2, MLong lMask)
{
	MLong lResult;
	MLong xx, yy;
	MLong lStart, lEnd;
	MDouble k=0, b;
	MLong lMaxLength, lStep,Length;
	MByte *pTmpData;
	MLong lTmpVal;

	lResult = 1;
	lMaxLength = 0;
	Length = 0;
	lStep = 0;
	pTmpData = pData;

	if (ptCross1.x == ptCross2.x)
	{
		xx = MIN(lWidth-1, MAX(ptCross1.x, 0));
		pTmpData = pData + xx;
		lStart = MIN(MAX(MIN(ptCross1.y, ptCross2.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptCross1.y, ptCross2.y), 0),lHeight-1);
		for (yy=lStart; yy<=lEnd; yy++, pTmpData+=lStride)
		{
			lTmpVal = (*pTmpData)&lMask;
			if (lTmpVal > 0)
			{
				lStep++;
			}
			else
			{
				if (lMaxLength < lStep)
				{
					lMaxLength = lStep;
					Length =lMaxLength;
				}
				lStep = 0;
			}
		}
	}
	else if (ptCross1.y == ptCross2.y)
	{
		yy = MIN(MAX(ptCross1.y, 0), lHeight-1);
		pTmpData = pData + yy * lStride;
		lStart = MIN(MAX(MIN(ptCross1.x, ptCross2.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptCross1.x, ptCross2.x), 0),lWidth-1);
		for (xx=lStart; xx<=lEnd; xx++, pTmpData++)
		{
			lTmpVal = (*pTmpData)&lMask;
			if (lTmpVal > 0)
			{
				lStep++;
			}
			else
			{
				if (lMaxLength < lStep)
				{
					lMaxLength = lStep;
					Length =lMaxLength;
				}
				lStep = 0;
			}
		}
	}
	else
	{
		k = (ptCross1.y - ptCross2.y)*1.0/(ptCross1.x - ptCross2.x);
		b = ptCross1.y - k * ptCross1.x;
		if (k<1.0 && k>-1.0)
		{
			lStart = MIN(MAX(MIN(ptCross1.x, ptCross2.x), 0),lWidth-1);
			lEnd = MIN(MAX(MAX(ptCross1.x, ptCross2.x), 0),lWidth-1);
			for (xx=lStart; xx<=lEnd; xx++)
			{
				yy = (MLong)(k * xx + b);
				if (yy<0||yy>=lHeight)
					continue;
				lTmpVal = (*(pTmpData + xx + yy * lStride)) & lMask;
				if (lTmpVal > 0)
				{
					lStep++;
				}
				else
				{
					if (lMaxLength < lStep)
					{
						lMaxLength = lStep;
						Length = sqrt((MDouble)lMaxLength*k*lMaxLength*k+lMaxLength*lMaxLength) ;

					}
					lStep = 0;
				}
			}
		}
		else
		{
			lStart = MIN(MAX(MIN(ptCross1.y, ptCross2.y), 0),lHeight-1);
			lEnd = MIN(MAX(MAX(ptCross1.y, ptCross2.y), 0),lHeight-1);
			for (yy=lStart; yy<=lEnd; yy++)
			{
				xx = (MLong)((yy - b) / k);
				if (xx<0||xx>=lWidth)
					continue;
				lTmpVal = (*(pTmpData + xx + yy * lStride)) & lMask;
				if (lTmpVal > 0)
				{
					lStep++;
				}
				else
				{
					if (lMaxLength < lStep)
					{
						lMaxLength = lStep;
						Length = sqrt((MDouble)(lMaxLength/k)*(lMaxLength/k)+lMaxLength*lMaxLength) ;
					}
					lStep = 0;
				}
			}
		}
	}
	if (lMaxLength < lStep)
	{
		lMaxLength = lStep;
		Length = sqrt((MDouble)(lMaxLength/k)*(lMaxLength/k)+lMaxLength*lMaxLength) ;
	}

	if (Length > lResult)
	{
		lResult = Length;
	}
	return lResult;
}

MLong LineInfoLengthCmp(const MVoid* p1, const MVoid* p2)
{
	LINE_INFO *_p1 = (LINE_INFO*)p1;
	LINE_INFO *_p2 = (LINE_INFO*)p2;

	if( _p1->lMaxLength< _p2->lMaxLength)
		return 1;
	if (_p1->lMaxLength > _p2->lMaxLength)
		return -1;
	return 0;
}

MRESULT appendSearch(MByte *pSrcData, MLong lSrcLine, MByte *pRltData, MLong lRltLine, MLong lWidth,
	MLong lHeight, MPOINT tmpSeed,  LINEPARAM lineParam, MLong lStepThres, MPOINT *pRltSeed)
{
	MRESULT res = LI_ERR_NONE;
	MLong lMaxVal;
	MByte rltLabel;
	MLong xx, yy;
	MLong lStep;
	MByte tmpSrcData, tmpRltData;
	MPOINT tmpPt;
	MLong lFlag;
	MLong lStepSum, lMaxStepSum;

	if (tmpSeed.x<0 || tmpSeed.x>=lWidth || tmpSeed.y<0 || tmpSeed.y>=lHeight)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}
	rltLabel = 128;
	lMaxStepSum = MIN(lWidth>>3, lHeight>>3);

	lStep = 0;
	lMaxVal = 0;
	lFlag = 0;
	lStepSum = 0;
	if (lineParam.bVertical)
	{
		xx = tmpSeed.x;
		for (yy=tmpSeed.y; yy>0; yy--, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			tmpRltData = *(pRltData + yy * lRltLine + xx);
			if (rltLabel == tmpRltData)
			{
				/*tmpSrcData = *(pSrcData + yy * lSrcLine + xx);
				if (tmpSrcData >= lMaxVal)
				{
				lMaxVal = tmpSrcData;
				tmpPt.x = xx;
				tmpPt.y = yy;
				}*/
				lStep++;
				if (lStep >= lStepThres)
				{
					lFlag = 1;
					tmpPt.x = xx;
					tmpPt.y = yy;
					break;
				}
			}
			else
			{
				lStep = 0;
				lMaxVal = 0;
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}

		lStep = 0;
		lMaxVal = 0;
		lStepSum = 0;
		for (yy=tmpSeed.y; yy<lHeight; yy++, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			tmpRltData = *(pRltData + yy * lRltLine + xx);
			if (rltLabel == tmpRltData)
			{
				/*tmpSrcData = *(pSrcData + yy * lSrcLine + xx);
				if (tmpSrcData >= lMaxVal)
				{
				lMaxVal = tmpSrcData;
				tmpPt.x = xx;
				tmpPt.y = yy;
				}*/
				lStep++;
				if (lStep >= lStepThres)
				{
					lFlag = 1;
					tmpPt.x = xx;
					tmpPt.y = yy;
					break;
				}
			}
			else
			{
				lStep = 0;
				lMaxVal = 0;
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}
	}
	else if (lineParam.Coefk>-1.0 && lineParam.Coefk<1.0)
	{
		for (xx=tmpSeed.x; xx>=0; xx--, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			yy = (MLong)(lineParam.Coefk * xx + lineParam.Coefb);
			if (yy>=0 && yy<lHeight)
			{
				tmpRltData = *(pRltData + yy * lRltLine + xx);
				if (rltLabel == tmpRltData)
				{
					/*tmpSrcData = *(pSrcData + yy * lRltLine + xx);
					if (tmpSrcData > lMaxVal)
					{
					lMaxVal = tmpSrcData;
					tmpPt.x = xx;
					tmpPt.y = yy;
					}*/
					lStep++;
					if (lStep >= lStepThres)
					{
						lFlag = 1;
						tmpPt.x = xx;
						tmpPt.y = yy;
						break;
					}
				}
				else
				{
					lStep = 0;
					lMaxVal = 0;
				}
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}

		lStep = 0;
		lMaxVal = 0;
		lStepSum = 0;
		for (xx=tmpSeed.x; xx<lWidth; xx++, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			yy = (MLong)(lineParam.Coefk * xx + lineParam.Coefb);
			if (yy>=0 && yy<lHeight)
			{
				tmpRltData = *(pRltData + yy * lRltLine + xx);
				if (rltLabel == tmpRltData)
				{
					/*tmpSrcData = *(pSrcData + yy * lRltLine + xx);
					if (tmpSrcData > lMaxVal)
					{
					lMaxVal = tmpSrcData;
					tmpPt.x = xx;
					tmpPt.y = yy;
					}*/
					lStep++;
					if (lStep >= lStepThres)
					{
						lFlag = 1;
						tmpPt.x = xx;
						tmpPt.y = yy;
						break;
					}
				}
				else
				{
					lStep = 0;
					lMaxVal = 0;
				}
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}
	}
	else
	{
		for (yy=tmpSeed.y; yy>=0; yy--, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			xx = (MLong)((yy - lineParam.Coefb) / lineParam.Coefk);
			if (xx>=0 && xx<lWidth)
			{
				tmpRltData = *(pRltData + yy * lRltLine + xx);
				if (rltLabel == tmpRltData)
				{
					tmpSrcData = *(pSrcData + yy * lRltLine + xx);
					if (tmpSrcData > lMaxVal)
					{
						lMaxVal = tmpSrcData;
						tmpPt.x = xx;
						tmpPt.y = yy;
					}
					lStep++;
					if (lStep >= lStepThres)
					{
						lFlag = 1;
						break;
					}
				}
				else
				{
					lStep = 0;
					lMaxVal = 0;
				}
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}

		lStep = 0;
		lMaxVal = 0;
		lStepSum = 0;
		for (yy=tmpSeed.y; yy<lHeight; yy++, lStepSum++)
		{
			if(lStepSum >= lMaxStepSum)
				break;
			xx = (MLong)((yy - lineParam.Coefb) / lineParam.Coefk);
			if (xx>=0 && xx<lWidth)
			{
				tmpRltData = *(pRltData + yy * lRltLine + xx);
				if (rltLabel == tmpRltData)
				{
					tmpSrcData = *(pSrcData + yy * lRltLine + xx);
					if (tmpSrcData > lMaxVal)
					{
						lMaxVal = tmpSrcData;
						tmpPt.x = xx;
						tmpPt.y = yy;
					}
					lStep++;
					if (lStep >= lStepThres)
					{
						lFlag = 1;
						break;
					}
				}
				else
				{
					lStep = 0;
					lMaxVal = 0;
				}
			}
		}
		if (1==lFlag)
		{
			pRltSeed->x = tmpPt.x;
			pRltSeed->y = tmpPt.y;
			goto EXT;
		}
	}

	if (1!=lFlag)
	{
		res = LI_ERR_UNKNOWN;
	}
EXT:
	return res;
}

MRESULT appendSeeds(MByte *pSrcData, MLong lSrcLine, MByte *pRltData, MLong lRltLine, MByte *pAngleData,
	MLong lAngleLine, MLong lWidth, MLong lHeight, MLong lThreshold, MLong lRadius, JGSEED *pSeeds)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	MLong xx, yy;
	MByte tmpSrcData, tmpRltData;	
	MLong lRadiusThres;
	MPOINT tmpSeed, rltSeed;
	LINEPARAM lineParam;
	MLong lTmpAngle;
	MLong lCurSeedNum;
	MByte lMaxVal;
	MLong lFlag;

	lRadiusThres = lRadius;
	lRadius = lRadius/2;
	lCurSeedNum = pSeeds->lSeedNum;

	for (i=0; i<pSeeds->lSeedNum; i++)
	{
		tmpSeed = pSeeds->pptSeed[i];
		lTmpAngle =*((MByte*)pAngleData+lAngleLine*tmpSeed.y + tmpSeed.x) &(~TEMPLATE_TYPE_BYTE_BIT);
		lTmpAngle = 360 - lTmpAngle * ANGLE_STEP;
		if(1==pSeeds->lSeedNum)
			lTmpAngle = 90;
		if(90==lTmpAngle || 270==lTmpAngle)
		{
			lineParam.bVertical = MTrue;
		}
		else
		{
			lineParam.Coefk = tan((MDouble)lTmpAngle);
			lineParam.Coefb = tmpSeed.y - tmpSeed.x * lineParam.Coefk;
			lineParam.bVertical = MFalse;
		}
		if (LI_ERR_NONE == appendSearch(pSrcData, lSrcLine, pRltData, lRltLine, lWidth, lHeight, tmpSeed,
			lineParam, lRadiusThres, &rltSeed))
		{
			lMaxVal = 0;
			lFlag = 0;
			for (yy=rltSeed.y-lRadius; yy<=rltSeed.y+lRadius; yy++)
			{
				for (xx=rltSeed.x-lRadius; xx<=rltSeed.x+lRadius; xx++)
				{
					if(xx<0 || xx>=lWidth || yy<0 || yy>=lHeight)
						continue;
					tmpSrcData = *(pSrcData + yy * lSrcLine + xx);
					tmpRltData = *(pRltData + yy * lRltLine + xx);
					if (tmpSrcData>lThreshold && 128==tmpRltData)
					{
						if (lMaxVal <= tmpSrcData)
						{
							lMaxVal = tmpSrcData;
							tmpSeed.x = xx;
							tmpSeed.y = yy;
							lFlag = 1;
						}
					}
				}
			}
			if (1==lFlag && MAX_SEED_NUM>lCurSeedNum)
			{
				pSeeds->pptSeed[lCurSeedNum] = tmpSeed;
				pSeeds->pcrSeed[lCurSeedNum] = lMaxVal;
				lCurSeedNum++;
				*(pRltData + tmpSeed.y * lRltLine + tmpSeed.x) = 250;
			}
		}
	}
	pSeeds->lSeedNum = lCurSeedNum;

	return res;
}
