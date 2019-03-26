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

#define MAX_SEED_NUM 150
#define MED_SEED_NUM 100
#define MIN_SEED_NUM 30
#define LEAST_NUM 3

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
//static MRESULT RANSAC_CIRCLE(MHandle hMemMgr, BLOCK *pHaarResponse,\
//							 BLOCK* pHaarAngleResponse,MPOINT *pPtList, MLong lPtNum, CIRCLE_SET_INFO *pCircleSetInfo,MPOINT *pTemp,MLong lTmpNum, MLong lRadiusCons,
//							 PARAM_INFO *pParam);

MRESULT RANSAC_LINE(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
					MLong lPtNum, MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam);

MRESULT RANSAC_LINE_SF6(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
						MLong lPtNum, MPOINT *pTemp, MLong lTmpNum, PARAM_INFO *pParam, MLong lMaxPtDist);

MRESULT RANSAC_LINE_SWITCH(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
							MLong lPtNum, MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam);

MRESULT RANSAC_LINE_LEAKAGE(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
							MLong lPtNum, MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam);

MRESULT Get_SF6_Line(MHandle hMemMgr, BLOCK *pHaarResponse, CIRCLE circleParam, PARAM_INFO *pParam);
static MRESULT CalcLineEndPt_xsd(MHandle hMemMgr, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold, LINEPARAM *lineParam);
static MRESULT CalcLineEndPtStep2(MHandle hMemMgr, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold,
									LINEPARAM lineParam, MLong minLength, MLong *leftWidthConfidence, MLong *rightWidthConfidence);
static MLong CmpVal_xsd( const MVoid* p1, const MVoid* p2);

static MVoid OptimizeHaar_xsd(ResponseResult *pSrc, ResponseResult *pDst, BLOCK *xBlock, BLOCK *yBlock,BLOCK *pResponseAngle, MLong *lMaxVal, MByte angleIndex);
//static MVoid AdjustLinePosition(MByte *pResponseData, MLong lRspLine, MLong lWidth, MLong lHeight,
//					MByte *pResponseAngleData, MLong lRspAngleLine,
//					LINEPARAM *pLineParam,MPOINT *pptAnchor,
//					MPOINT* pptStart, MPOINT* pptEnd);



MVoid AppendPointList(MByte *pRspData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MByte *pRspAngleData, MLong lAngleDataLine,
	JGSEED *pSeedList, CIRCLE_INFO *pCircleInfo, PARAM_INFO *pParam, MVoid *pTemp, MLong lMemLen);

MRESULT FilterCirclePointList(MHandle hMemMgr,MByte *pRspData, MLong lRspDataLine, MLong lWidth, MLong lHeight,
						CIRCLE_INFO *pCircleInfo, LINE_INFO *pLineInfo, MPOINT ptLineMid,MLong lMaxArcAngle);
static MVoid selectPoint(PTINFO* pSrcPtInfoList, MLong srcPtInfoNum, JGSEED *pSeeds, MLong lRadius);
static MVoid selectPoint2(CIRCLE circleParam, JGSEED *pSeeds, MLong lWidth, MLong lHeight);

MLong lcircleNum = 0;
BLOCK blockFlagTmp = {0};

//******************************** line location*************************************************
MRESULT GetLineInfo(MHandle hMemMgr,BLOCK *pImage, BLOCK *pMask, PARAM_INFO *pParam)
{
	MRESULT res=LI_ERR_NONE;
	JGSEED seedsTemp={0};
	MLong i;
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

	MLong lRate = 35;
	MLong lCount;

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
		haarParam.w = (pParam->lHarrWidth)<<2;
	}
	haarParam.bWhiteBackground = pParam->bWhiteBackground;
	lHaarWidth = pParam->lHarrWidth;

	AllocVectMem(hMemMgr,haarParam.pAngleList,haarParam.lAngleNumber,MLong);
	for (i=0;i<haarParam.lAngleNumber;i++)
	{
		haarParam.pAngleList[i] = i*ANGLE_STEP;//�����ģ��,���Ҫ-90��
	}
	GO(B_Create(hMemMgr, &haarAngleResult, DATA_U8, pImage->lWidth, pImage->lHeight));
	GO(B_Create(hMemMgr,&haarResult,DATA_U8,pImage->lWidth, pImage->lHeight));
    //��ȡ��Ӧͼ
    PrintBmpEx(pImage->pBlockData, pImage->lBlockLine, DATA_U8, pImage->lWidth, pImage->lHeight, 1, "D:\\gray_aaa.bmp");
//	printf("stride=%d, width=%d, height=%d\n", pImage->lBlockLine, pImage->lWidth, pImage->lHeight);
	GO(HaarResponseAngle(hMemMgr,pImage,&haarParam,&haarResult, &haarAngleResult));
	PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, "D:\\response_aaa.bmp");

	lStackLen = haarResult.lWidth * haarResult.lHeight /10;
	AllocVectMem(hMemMgr, pTemp, lStackLen, MPOINT);
	GO(B_Create(hMemMgr, &blockFlag, DATA_U8, haarResult.lWidth, haarResult.lHeight));
	SetVectZero(blockFlag.pBlockData, blockFlag.lBlockLine * blockFlag.lHeight);
	lSeedSize = haarResult.lWidth * haarResult.lHeight;
	AllocVectMem(hMemMgr, seedsTemp.pptSeed, lSeedSize, MPOINT);
	AllocVectMem(hMemMgr, seedsTemp.pcrSeed, lSeedSize, MByte);

	seedsTemp.lMaxNum = lSeedSize;
	seedsTemp.lSeedNum = 0;

	GO(getMaskResponse(&haarResult, pMask));
	PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, "D:\\response_bbb.bmp");

	if (PIC_HIGH_BLUR == pParam->lPicBlurLevel)
	{
		inverseImg(&haarResult);
		PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, "D:\\response_ccc.bmp");

	}

	GO(StatisticNonZeroValue(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine,haarResult.lWidth,
								haarResult.lHeight,0, pTemp, sizeof(MPOINT)*lStackLen, lRate, &lThreshold));
	
	GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
		blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lThreshold, (MLong)(lThreshold*0.5), 50, 128, &seedsTemp, lHaarWidth));	//0.6

	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\localMaxPre_bbb.bmp");
	JPrintf("localMax---------\n");
	Logger("local max ok...\n");
	if (0==seedsTemp.lSeedNum)
	{
		printf("lThreshold=%d, lHarrWidth=%d\n", lThreshold, lHaarWidth);
		lRate += 10;
		GO(StatisticNonZeroValue(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine,haarResult.lWidth,
			haarResult.lHeight,0, pTemp, sizeof(MPOINT)*lStackLen, lRate, &lThreshold));

		GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
			blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lThreshold, (MLong)(lThreshold*0.5), 50, 128, &seedsTemp, lHaarWidth));	//0.6

		PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\localMaxPre_bbb.bmp");
	}
	lHighThreshold = lThreshold;
	lCount = 0;
	while(seedsTemp.lSeedNum >= MED_SEED_NUM)
	{
		lCount++;
		lHighThreshold = lHighThreshold + 15;
		if(lCount > 5 || lHighThreshold>=250)
			break;
		GO(LocalMax_Circle(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,
			blockFlag.lWidth,blockFlag.lHeight,pTemp,lStackLen, lHighThreshold, (MLong)(lHighThreshold*0.6), 50, 128, &seedsTemp, lHaarWidth>>1));

		PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\localMaxPre_bbb.bmp");
	}
	
	if(seedsTemp.lSeedNum < MIN_SEED_NUM)
	{
		if (seedsTemp.lSeedNum < LEAST_NUM)
		{
			lCount = 0;
			while(seedsTemp.lSeedNum < LEAST_NUM)
			{
				lCount++;
				if (lCount>=5)
					break;
				appendSeeds((MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte*)blockFlag.pBlockData,
					blockFlag.lBlockLine, (MByte*)haarAngleResult.pBlockData, haarAngleResult.lBlockLine, haarResult.lWidth,
					haarResult.lHeight, (MLong)(lThreshold * 0.6), lHaarWidth, &seedsTemp);
			}
		}
		else
		{
			appendSeeds((MByte*)haarResult.pBlockData, haarResult.lBlockLine, (MByte*)blockFlag.pBlockData,
				blockFlag.lBlockLine, (MByte*)haarAngleResult.pBlockData, haarAngleResult.lBlockLine, haarResult.lWidth,
				haarResult.lHeight, (MLong)(lThreshold * 0.6), lHaarWidth, &seedsTemp);
		}
		printf("##### pt num=%d\n", seedsTemp.lSeedNum);
	}
	
	if(MNull!=pParam->pCircleInfo)
		selectPoint2(pParam->pCircleInfo->circleParam, &seedsTemp, haarResult.lWidth, haarResult.lHeight);

	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\localMaxPost.bmp");

	if (seedsTemp.lSeedNum >= MAX_SEED_NUM)
	{
		JPrintf("too many seeds error!\n");
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	GO(RANSAC_LINE(hMemMgr, &haarResult, &haarAngleResult, seedsTemp.pptSeed, seedsTemp.lSeedNum, pTemp, lStackLen, pParam));
	pLineParam = &(pParam->pCircleInfo->lineInfo.lineParam);
	lThreshold = lThreshold * 3 / 5;
	CalcLineEndPt_xsd(hMemMgr, &haarResult, pParam->pCircleInfo->circleParam.lRadius, lThreshold, pLineParam);
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
		
		/*sprintf(fileName, "D:\\intergral_%d.dat", i);
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
		
		/*sprintf(fileName, "D:\\response_%d.dat", i);
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
	//printf("lMaxValue=%d \n", lMaxValue);
	//��һ��
	lDataExt = responseResult.lBlockLine - responseResult.lWidth;
	plongData = responseResult.pData;
	/*{
		FILE *fp;
		MLong *pResponseData;
		int xx, yy;
		MLong lExt;
		
		fp = fopen("D:\\responseResultMap.dat", "w+");
		pResponseData = responseResult.pData;
		lExt = responseResult.lBlockLine - responseResult.lWidth;
		for (yy=0; yy<responseResult.lHeight; yy++, pResponseData+=lExt)
		{
			fprintf(fp, "(%4d)", yy);
			for (xx=0; xx<responseResult.lWidth; xx++, pResponseData++)
			{
				fprintf(fp, "%6d(%3d)", *pResponseData, xx);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);

	}*/

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
					if (sum3<(sum2*19/20) && sum3<(sum4*19/20))
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
MRESULT ReadNumber(MHandle hMemMgr,BLOCK *image,MPOINT *point,MLong lNUmber,MPOINT startPoint,MPOINT endPoint,MDouble *numberData,MDouble *result)
{
	MLong i,j;
	BLOCK imageTemp1={0},imageTemp2={0};
	MLong lLine;
	LineSegmentPoint lineSeg={0};
	MPOINT leftPoint={0};
	MPOINT rightPoint={0};
	MUInt8* pTempData1;
	MUInt8 *pTempData2;
	MLong lX=-1;
	MLong lY=-1;
	MLong sumX=0;
	MLong sumY=0;
	MLong lWidth=0;
	MLong lHeight=0;
	MLong lNum=0;
	MDouble dis1,dis2;
	MRESULT res=LI_ERR_NONE;
    MLong x=0;
    MLong y=0;
    MLong r=0;
    MLong xc=0;
    MLong yc=0;

	GO(B_Create(hMemMgr,&imageTemp1,DATA_U8,image->lWidth,image->lHeight));
	B_Set(&imageTemp1,0);
	GO(B_Create(hMemMgr,&imageTemp2,DATA_U8,image->lWidth,image->lHeight));
	B_Set(&imageTemp2,0);

	PrintBmpEx(image->pBlockData, image->lBlockLine,image->typeDataA & ~0x100,image->lWidth, image->lHeight, 1, "D:\\dkdkd.bmp");

	lineSeg.startPoint.x=startPoint.x;
	lineSeg.startPoint.y=startPoint.y;
	lineSeg.endPoint.x=endPoint.x;
	lineSeg.endPoint.y=endPoint.y;
	lWidth=image->lWidth;
	lHeight=image->lHeight;


    pTempData1=(MUInt8*)(imageTemp1.pBlockData);
    pTempData2=(MUInt8*)(imageTemp2.pBlockData);


	for (i=0;i<lNUmber-1;i++)
	{
		vLineTo((MByte*)(imageTemp1.pBlockData),imageTemp1.lBlockLine,imageTemp1.lWidth,imageTemp1.lHeight,100,point[i],point[i+1]);
	}
    vCircleFitting(point,lNUmber,&x,&y,&r);
    lLine=imageTemp2.lBlockLine;
    //��չ��̶ȵ㣬��ָ��ָ��̶ȵ����ʱ
    for (i=0;i<=x;i++)
    {
        if (i>=x-r)
        {
            if (i>point[lNUmber-1].x)
            {
                yc=(MLong)(y-sqrt((MDouble)r*r-(x-i)*(x-i)));
                if (yc>=0&&yc<lHeight&&yc<=point[lNUmber-1].y)
                {
                    pTempData1[yc*lLine+i]=100;
                }
            }
            if (i>point[0].x)
            {
                yc=(MLong)(y+sqrt((MDouble)r*r-(x-i)*(x-i)));
                if (yc>=0&&yc<lHeight&&yc>=point[0].y)
                {
                    pTempData1[yc*lLine+i]=100;
                }

            }
        }
    }

	//vLineTo_EX((MByte*)(imageTemp2.pBlockData),imageTemp2.lBlockLine,imageTemp2.lWidth,imageTemp2.lHeight,100,startPoint,endPoint);
	
	

	for (j=1;j<image->lHeight-1;j++)
	{
		for (i=1;i<image->lWidth-1;i++)
		{
			if (pTempData1[j*lLine+i]==100)
			{
				if (pTempData2[j*lLine+i]==100)
				{
					lX=i;
					lY=j;
				}
				else
				{
					if (pTempData2[(j-1)*lLine+i]==100||pTempData2[(j+1)*lLine+i]==100||pTempData2[(j)*lLine+i+1]==100||pTempData2[(j)*lLine+i-1]==100)
					{
						sumX+=i;
						sumY+=j;
						lNum++;
					}
				}
			}


		}
	}
	if (lX!=-1&&lY!=-1)
	{

	}
	else
	{
		if (lNum!=0)
		{
			lX=sumX/lNum;
			lY=sumY/lNum;
		}
		else
		{
			res=LI_ERR_INDEX_NO_FIND;
			goto EXT;
		}
	}

    //�޸ĳɶ���360�ȶ�����

    PrintBmpEx(imageTemp1.pBlockData,imageTemp1.lBlockLine,DATA_U8,imageTemp1.lWidth,imageTemp1.lHeight,1,"D:\\line1.bmp");
    PrintBmpEx(imageTemp2.pBlockData,imageTemp2.lBlockLine,DATA_U8,imageTemp2.lWidth,imageTemp2.lHeight,1,"D:\\line2.bmp");


	for (i=0;i<lNUmber-1;i++)
	{
		//if (lX>=point[i].x&&lX<=point[i+1].x&&lY>=point[i].y&&lY<=point[i+1].y)
        if ((lX-point[i].x)*(lX-point[i+1].x)<=0&&(lY-point[i].y)*(lY-point[i+1].y)<=0)
			break;
	}

    if (lX>=point[0].x&&lY>=point[0].y)
    {
        *result=numberData[0];
    }
    else
    {
        if (lX>=point[lNUmber-1].x&&lY<=point[lNUmber-1].y)
        {
            *result=numberData[lNUmber-1];
        }
        else
        {
            dis1=sqrt((MDouble)((point[i].y-lY)*(point[i].y-lY)+(point[i].x-lX)*(point[i].x-lX)));
            dis2=sqrt((MDouble)((point[i+1].y-point[i].y)*(point[i+1].y-point[i].y)+(point[i+1].x-point[i].x)*(point[i+1].x-point[i].x)));
            *result=dis1/(dis2+0.001)*(numberData[i+1]-numberData[i])+numberData[i];
        }
    }
    


EXT:
	B_Release(hMemMgr,&imageTemp1);
	B_Release(hMemMgr,&imageTemp2);
	return res;
}

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

//xsd
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
			//��β�����ͼ���������Ĵ�С
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
				//ֱ��ȡ�����֮���meanvalue
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
			sprintf(path, "D:\\localCircle%dline%dconfi%d.bmp",i,j,pLineInfo->lConfidence);
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
		sprintf(path, "D:\\frame[%d]localCircle%dconfi%d.bmp",lframeNum,i,pCircleInfo->lConfidence);
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
	MLong j, k, l, m;
	MPOINT randomPoint[3];
	MPOINT *inPoint=pTemp;
	MLong numInPoint=0;
	MDouble distance, dTmpAngle;
	MLong lAngleValue,lAngleValue2;
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

	/*printf("seed num = %d\n", lPtNum);
	for (j=0; j<lPtNum; j++)
	{
		printf("[%d,(%d,%d)] ", j, (pPtList+j)->x, (pPtList+j)->y);
	}
	printf("\n");*/

	//����Բ��ѡ�㣬��ÿ����ѡ��Բ�����Ӧ �Ĵ�ָ��ֵ�����ܳ�Ϊһ������
	//��ģʽ��
	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));
//	JPrintf("CreateLineSet~~~\n");
	{
		CIRCLE_INFO *pCircleInfo = pParam->pCircleInfo;
		JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
		seedTmp.lSeedNum = lPtNum;
		
		maxDist2Center = MAX(5, pCircleInfo->circleParam.lRadius>>2);	//1/4 ?  1/2 ?	1/8?
		randomPoint[2].x = pCircleInfo->circleParam.xcoord;
		randomPoint[2].y = pCircleInfo->circleParam.ycoord;
		JPrintf("center[%d,%d], %d\n", pCircleInfo->circleParam.xcoord, 
				pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius);	
		tmpCircleParam.xcoord = pCircleInfo->circleParam.xcoord;
		tmpCircleParam.ycoord = pCircleInfo->circleParam.ycoord;
		tmpCircleParam.lRadius = pCircleInfo->circleParam.lRadius * 9 / 10;
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
							pTmpPt[numInPoint].x = seedTmp.pptSeed[m].x;
							pTmpPt[numInPoint++].y = seedTmp.pptSeed[m].y;
						}
					}
					//����ϵĵ�����Ȩ��???
					//��Щ��ͬһ�����ߵ�Ȩ������
					vFitLine(pTmpPt, numInPoint, &dCoeffK1, &dCoeffB1, &bVert1);//��Ϊǰ���Ѿ����˳��������ﲻ���ٳ���
					//���fitting�����Ĳ�����һ��
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
			}
			else
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_X));
			}

			{	
				MChar path[256]={0};
				sprintf(path, "D:\\line\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}

			//������ѡ����β�����ģ����ǿ����м���ӵ�Ӱ��������ֵ�����ԱȽ�
			//��β�����ͼ���������Ĵ�С
			lTotalSumNum = 0;
			lTotalPtNum = 0;
			for(k=0; k<pLineInfo->lPtNum-2; k+=2)
			{
				lPTMeanSum1 =  vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint, 0xFF,
					&lSumNum1, &lPtNum1);
				lPTMeanSum1 *= lSumNum1;
				lPTMeanSum2 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k+1].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,
					&lSumNum2, &lPtNum2);
				lPTMeanSum2 *= lSumNum2;

				//ֱ��ȡ�����֮���meanvalue
				lPTMeanSum3 = 0;
				//����֮��Ҫ����һ���Ĺ�ϵ������ϴ�Ķ۽�
				if(vComputeIntersactionAngle(pLineInfo->pPtinfo[k].ptPoint,pLineInfo->pPtinfo[k+1].ptPoint,pLineInfo->pPtinfo[k+2].ptPoint)>8*V_PI/9)//160��
				{
					lPTMeanSum3 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,&lSumNum3, &lPtNum3);
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
					&lSumNum1, &lPtNum1);
				lMeanSum += lPTMeanSum1*lSumNum1;
				lTotalSumNum += lSumNum1;
				lTotalPtNum += lPtNum1;
			}
			
			lMeanSum /= lTotalSumNum;
//			printf("lPtNum=%d\n", lTotalPtNum);

			calcLineCrossCirclePt(&pCircleInfo->circleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			ptMid.x = (ptCross1.x + ptCross2.x)>>1;
			ptMid.y = (ptCross1.y + ptCross2.y)>>1;
			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&pLineInfo->lineParam, &ptMid, &ptCross1, &ptCross2);

			calcLineCrossCirclePt(&tmpCircleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, pHaarResponse->lWidth, 
										pHaarResponse->lHeight, ptCross1, ptCross2,0xFF,&lSumNum1, &lTotalPtNum);
			pLineInfo->lMaxLength = calcMaxContinuityLength((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
										pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xFF);

			pLineInfo->lConfidence = lMeanSum * lTotalPtNum / pCircleInfo->circleParam.lRadius;

			/*printf("index=%d, line:k=%.4f, b=%.4f, con=%d, sum=%d, lPtNum=%d, lPtNum2=%d\n", j, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb,
					pLineInfo->lConfidence, lMeanSum, lTotalPtNum, lTotalSumNum);*/
			{
				FILE *infoFile = MNull;
				infoFile = fopen("D:\\lineInfo.dat", "ab+");
				fprintf(infoFile, "No(%4d), length(%5d) confidence(%5d)\n", j, pLineInfo->lMaxLength, pLineInfo->lConfidence);
				fclose(infoFile);
			}
		}
		GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
		//���û��ֱ�ߣ����confidenceΪ0
		if (lineSetInfo.lLineNum == 0)
		{
			pCircleInfo->lConfidence = 0;
			pCircleInfo->lLineNum = 0;
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}
		
		lResponseThres = lineSetInfo.pLineInfo[0].lConfidence * 3 / 5;
		GO(QuickSort(hMemMgr, (MVoid*)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoLengthCmp));
		lIndex = 0;
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
			/*{	
				MChar path[256]={0};
				sprintf(path, "D:\\line\\line2\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}*/
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

			{	
				MChar path[256]={0};
				LINE_INFO *pLineInfo = &(pCircleInfo->lineInfo);
				sprintf(path, "D:\\line.bmp");
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
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

MRESULT RANSAC_LINE_LEAKAGE(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, 
							MPOINT *pPtList, MLong lPtNum, MPOINT *pTemp, MLong lTmpNum, PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;
	MLong j, k, l, m;
	MPOINT randomPoint[3];
	MPOINT *inPoint=pTemp;
	MLong numInPoint=0;
	MDouble distance, dTmpAngle;
	MLong lAngleValue,lAngleValue2;
	MLong minNumPtOnLine = 3;	// 3? 2?
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
	JGSEED seedTmp = {0};//Ŀ����Ϊ����ʱ�洢��Բ�ϵ�֮��������㣬�������ֱ��
	LINE_SET_INFO lineSetInfo = {0};
	LINEPARAM lineParam;
	CIRCLE tmpCircleParam;

	MPOINT ptCross1, ptCross2, ptMid;

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

	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));
	{
		CIRCLE_INFO *pCircleInfo = pParam->pCircleInfo;
		JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
		seedTmp.lSeedNum = lPtNum;
		
		maxDist2Center = MAX(5, pCircleInfo->circleParam.lRadius>>2);	//1/4 ?  1/2 ?
		randomPoint[2].x = pCircleInfo->circleParam.xcoord;
		randomPoint[2].y = pCircleInfo->circleParam.ycoord;
		JPrintf("center[%d,%d], %d\n", pCircleInfo->circleParam.xcoord, 
				pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius);	
		tmpCircleParam.xcoord = pCircleInfo->circleParam.xcoord;
		tmpCircleParam.ycoord = pCircleInfo->circleParam.ycoord;
		tmpCircleParam.lRadius = pCircleInfo->circleParam.lRadius * 9 / 10;
		//�Ժ�ѡ���б�������
		for (j=0; j<seedTmp.lSeedNum; j++)
		{
			randomPoint[0].x = seedTmp.pptSeed[j].x;
			randomPoint[0].y = seedTmp.pptSeed[j].y;
			for (k=j+1; k<seedTmp.lSeedNum; k++)
			{
				MBool bChanged;
	
				randomPoint[1].x = seedTmp.pptSeed[k].x;
				randomPoint[1].y = seedTmp.pptSeed[k].y;

				//����
				lAngleValue = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
								randomPoint[0].y+randomPoint[0].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
								randomPoint[1].y+randomPoint[1].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				dTmpAngle = ABS(lAngleValue - lAngleValue2) * ANGLE_STEP;
				if (dTmpAngle>10 && dTmpAngle<170)
					continue;
				if(vFitLine(randomPoint,2,&dCoeffK, &dCoeffB, &bVert)<0)
					continue;

				//һֱ��ϣ�ֱ��������������
				bChanged = MTrue;
				numInPoint1 = 0;
				while(bChanged)
				{
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
							pTmpPt[numInPoint].x = seedTmp.pptSeed[m].x;
							pTmpPt[numInPoint++].y = seedTmp.pptSeed[m].y;
						}
					}

					vFitLine(pTmpPt, numInPoint, &dCoeffK1, &dCoeffB1, &bVert1);
				
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

		MergeSet((MVoid*)(lineSetInfo.pLineInfo), sizeof(LINE_INFO), &(lineSetInfo.lLineNum), LineInfoCmp, LineInfoReplace);

		//��ÿ��ֱ�߶ε�confidence:
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			MLong lTotalSumNum,lSumNum1, lSumNum2, lSumNum3;
			MLong lTotalPtNum, lPtNum1, lPtNum2, lPtNum3; 
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;

			lPTMeanSum1= 0, lPTMeanSum2=0, lPTMeanSum3=0;
			lMeanSum = 0;
			
			{	
				MChar path[256]={0};
				sprintf(path, "D:\\line\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}	

			if(pLineInfo->lineParam.bVertical || (pLineInfo->lineParam.Coefk>1 || pLineInfo->lineParam.Coefk<-1))
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_Y));
			}
			else
			{
				GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_X));
			}

			if(2 > pLineInfo->lPtNum)
				continue;

			lTotalSumNum = 0;
			lTotalPtNum = 0;
			for(k=0; k<pLineInfo->lPtNum-2; k+=2)
			{
				lPTMeanSum1 =  vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint, 0xFF,
					&lSumNum1, &lPtNum1);
				lPTMeanSum1 *= lSumNum1;
				lPTMeanSum2 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k+1].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,
					&lSumNum2, &lPtNum2);
				lPTMeanSum2 *= lSumNum2;

				//ֱ��ȡ�����֮���meanvalue
				lPTMeanSum3 = 0;
				//����֮��Ҫ����һ���Ĺ�ϵ������ϴ�Ķ۽�
				if(vComputeIntersactionAngle(pLineInfo->pPtinfo[k].ptPoint,pLineInfo->pPtinfo[k+1].ptPoint,pLineInfo->pPtinfo[k+2].ptPoint)>8*V_PI/9)//160��
				{
					lPTMeanSum3 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,&lSumNum3, &lPtNum3);
					lPTMeanSum3 *= lSumNum3;
				}
				if (lPTMeanSum3>lPTMeanSum1+lPTMeanSum2)
				{
					lMeanSum+=lPTMeanSum3;
					lTotalSumNum+=lSumNum3;
					lTotalPtNum += lPtNum3;
				}
				else
				{
					lMeanSum+=lPTMeanSum1+lPTMeanSum2;
					lTotalSumNum+=lSumNum1+lSumNum2;
					lTotalPtNum += lPtNum1 + lPtNum2;
				}
			}
			for (; k<pLineInfo->lPtNum-1;k++)
			{
				lPTMeanSum1=vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
					pHaarResponse->lWidth, pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint,0xFF,
					&lSumNum1, &lPtNum1);
				lMeanSum+=lPTMeanSum1*lSumNum1;
				lTotalSumNum+=lSumNum1;
				lTotalPtNum += lPtNum1;
			}
			
			lMeanSum /= lTotalSumNum;
//			printf("lPtNum=%d\n", lTotalPtNum);

			calcLineCrossCirclePt(&pCircleInfo->circleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			ptMid.x = (ptCross1.x + ptCross2.x)>>1;
			ptMid.y = (ptCross1.y + ptCross2.y)>>1;
			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&pLineInfo->lineParam, &ptMid, &ptCross1, &ptCross2);

			calcLineCrossCirclePt(&tmpCircleParam, &pLineInfo->lineParam, &ptCross1, &ptCross2);
			vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, pHaarResponse->lWidth, 
										pHaarResponse->lHeight, ptCross1, ptCross2,0xFF,&lSumNum1, &lTotalPtNum);

			pLineInfo->lConfidence = lMeanSum * lTotalPtNum / pCircleInfo->circleParam.lRadius;

			if (pLineInfo->lineParam.Coefk>-0.7 && pLineInfo->lineParam.Coefk<0.7)
			{
				pLineInfo->lConfidence = -1;
			}

			distance = vPointDistToLine2(randomPoint[2], pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
			if (RANSAC_THRESHOLD3 < distance)
			{
				pLineInfo->lConfidence = (pLineInfo->lConfidence>>2);
			}
			/*printf("index=%d, line:k=%.4f, b=%.4f, con=%d, sum=%d, lPtNum=%d, lPtNum2=%d\n", j, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb,
					pLineInfo->lConfidence, lMeanSum, lTotalPtNum, lTotalSumNum);*/
		}
		GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
		JPrintf("res: k=%f, b=%f, con=%d\n", lineSetInfo.pLineInfo[0].lineParam.Coefk, lineSetInfo.pLineInfo[0].lineParam.Coefb, lineSetInfo.pLineInfo[0].lConfidence);
		//���û��ֱ�ߣ����confidenceΪ0
		if (lineSetInfo.lLineNum == 0)
		{
			JPrintf("1812  lineSetInfo.lLineNum=0\n");
			pCircleInfo->lConfidence = 0;
			pCircleInfo->lLineNum = 0;
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}

		//����ֱ�ߣ�ȷ��ֱ�ߵķ���
		//ֱ����Բ�Ľ���
		{
			calcLineCrossCirclePt(&pCircleInfo->circleParam, &lineSetInfo.pLineInfo[0].lineParam, &ptCross1, &ptCross2);

//			printf("ransac_line: cross1(%d,%d), cross2(%d,%d)\n", ptCross1.x, ptCross1.y, ptCross2.x, ptCross2.y);

			ptMid.x = (ptCross1.x+ptCross2.x)>>1;
			ptMid.y = (ptCross1.y+ptCross2.y)>>1;

			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&lineSetInfo.pLineInfo[0].lineParam,
				&ptMid, &ptCross1, &ptCross2);
			calcLineCrossCirclePt(&pCircleInfo->circleParam, &lineSetInfo.pLineInfo[0].lineParam, &ptCross1, &ptCross2);
//			printf("adjust cross1(%d,%d), cross2(%d,%d)\n", ptCross1.x, ptCross1.y, ptCross2.x, ptCross2.y);

			lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, 
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xff,MNull);

			if (lPTMeanSum1<=0)
			{
				pCircleInfo->lConfidence = 0;
				pCircleInfo->lLineNum = 0;
				printf("RANSAC LINE lPTMeanSum<=0~~\n");
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}

			if((ptCross1.x==ptMid.x && ptCross1.y==ptMid.y) || (ptCross2.x==ptMid.x && ptCross2.y==ptMid.y))
			{
				printf("ptCross1=ptMid || ptCross2=ptMid err\n");
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}

			lineSetInfo.pLineInfo[0].lineParam.ptStart = ptCross1;
			lineSetInfo.pLineInfo[0].lineParam.ptEnd = ptCross2;
			lineSetInfo.pLineInfo[0].lineParam.ptMid = ptMid;
			lineParam = lineSetInfo.pLineInfo[0].lineParam;

			pCircleInfo->lLineNum=1;
			pCircleInfo->lineInfo.lConfidence= lPTMeanSum1;
			pCircleInfo->lineInfo.lPtNum =lineSetInfo.pLineInfo[0].lPtNum;
			JMemCpy(&(pCircleInfo->lineInfo.lineParam),&(lineParam), sizeof(LINEPARAM));
			JMemCpy(pCircleInfo->lineInfo.pPtinfo, lineSetInfo.pLineInfo[0].pPtinfo,sizeof(PTINFO)*lineSetInfo.pLineInfo[0].lPtNum);
			JPrintf("k=%f, b=%f\n", lineParam.Coefk, lineParam.Coefb);

			{	
				MChar path[256]={0};
				LINE_INFO *pLineInfo = &(pCircleInfo->lineInfo);
				sprintf(path, "D:\\line.bmp");
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
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

MRESULT RANSAC_LINE_SF6(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse, MPOINT *pPtList, 
						MLong lPtNum, MPOINT *pTemp, MLong lTmpNum, PARAM_INFO *pParam, MLong lMaxPtDist)
{
	MRESULT res = LI_ERR_NONE;
	MLong j, k, l, m;
	MPOINT randomPoint[3];
	MPOINT *inPoint=pTemp;
	MLong numInPoint=0;
	MDouble distance;
	//MLong lAngleValue,lAngleValue2;
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
	JGSEED seedTmp = {0};//Ŀ����Ϊ����ʱ�洢��Բ�ϵ�֮��������㣬�������ֱ��
	LINE_SET_INFO lineSetInfo = {0};
	LINEPARAM lineParam;

	MPOINT ptCross1, ptCross2, ptMid;
	//MLong lTmpPtDist;

	MLong lResponseThres;
	MLong lIndex;

	FILE *tmpFp = MNull;

	MPOINT *pTmpPt = MNull;
	BLOCK blockFlagTmp1 = {0};
	if(lPtNum < 2)
	{
		printf("lPtNum<2\n");
		res = LI_ERR_NO_FIND;
		return res;
	}

	/*tmpFp = fopen("D:\\log.dat", "ab+");
	fprintf(tmpFp, "SF6   ransac...\n");
	fclose(tmpFp);*/

	AllocVectMem(hMemMgr, seedTmp.pptSeed, lPtNum, MPOINT);
	AllocVectMem(hMemMgr, seedTmp.pcrSeed, lPtNum, MByte);
	AllocVectMem(hMemMgr, pTmpPt, lPtNum, MPOINT);
	//	AllocVectMem(hMemMgr, pAngleValue, lPtNum, MLong);
	B_Create(hMemMgr, &blockFlagTmp1, DATA_U8,pHaarResponse->lWidth, pHaarResponse->lHeight);

	//����Բ��ѡ�㣬��ÿ����ѡ��Բ�����Ӧ �Ĵ�ָ��ֵ�����ܳ�Ϊһ������
	//��ģʽ��
	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));
	/*tmpFp = fopen("D:\\log.dat", "ab+");
	fprintf(tmpFp, "SF6   step0---\n");
	fclose(tmpFp);*/
	//	JPrintf("CreateLineSet~~~\n");
	{
		CIRCLE_INFO *pCircleInfo = pParam->pCircleInfo;
		JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
		seedTmp.lSeedNum = lPtNum;

		maxDist2Center = MAX(5, pCircleInfo->circleParam.lRadius>>2);	//1/4 ?  1/2 ?	1/8?
		randomPoint[2].x = pCircleInfo->circleParam.xcoord;
		randomPoint[2].y = pCircleInfo->circleParam.ycoord;

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
				/*lAngleValue = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
					randomPoint[0].y+randomPoint[0].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
					randomPoint[1].y+randomPoint[1].x) &(~TEMPLATE_TYPE_BYTE_BIT);
				dTmpAngle = ABS(lAngleValue -lAngleValue2)*ANGLE_STEP;*/
				/*if (dTmpAngle>10 && dTmpAngle<170)
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
						dTmpAngle=ABS(lAngleValue-lAngleValue2)*ANGLE_STEP;*/
						/*if (dTmpAngle>10 &&dTmpAngle<170)
							continue;*/
						if (distance < DISTANCE_THRESHOLD)
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
							dCoeffB=dCoeffB1;
							bVert=bVert1;
							numInPoint1 = numInPoint;
							JMemCpy(inPoint, pTmpPt, numInPoint*sizeof(MPOINT));
						}
						else	// numInPoint <= numInPoint1
						{
							//����з������������ô��ֱ�����������������������б��洢��list��ȥ
							if (numInPoint>=minNumPtOnLine)
							{
								distance = vPointDistToLine2(randomPoint[2], dCoeffK1, dCoeffB1, bVert1);
								if (distance < maxDist2Center && lineSetInfo.lLineNum<lineSetInfo.lMaxLineNum)
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

		/*tmpFp = fopen("D:\\log.dat", "ab+");
		fprintf(tmpFp, "step1   lineNum=%d\n", lineSetInfo.lLineNum);
		fclose(tmpFp);*/

		//��ÿ��ֱ�߶ε�confidence:
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			//�Ե���������γ���β�����ĵ���б�
			//����ÿ���߶ε�confidence������һ��
			MLong lTotalSumNum,lSumNum1, lSumNum2;
			//MLong lSumNum3;
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

			{	
				MChar path[256]={0};
				sprintf(path, "D:\\line\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
			//������ѡ����β�����ģ����ǿ����м���ӵ�Ӱ��������ֵ�����ԱȽ�
			//��β�����ͼ���������Ĵ�С
			lTotalSumNum = 0;

			vComputeCrossPt(pCircleInfo->circleParam.xcoord, pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius,
				pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical, &ptCross1, &ptCross2);
			ptMid.x = (ptCross1.x + ptCross2.x)>>1;
			ptMid.y = (ptCross1.y + ptCross2.y)>>1;
			//================================
			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, pHaarResponse->lWidth, pHaarResponse->lHeight, 
								&(pLineInfo->lineParam), &ptMid, &ptCross1, &ptCross2);
			pLineInfo->lMaxLength = calcMaxContinuityLength((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xFF);
			lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
							pHaarResponse->lWidth, pHaarResponse->lHeight, ptMid, ptCross1, 0xFF, &lSumNum1);
			lPTMeanSum2 = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
								pHaarResponse->lWidth, pHaarResponse->lHeight, ptMid, ptCross2, 0xFF, &lSumNum2);
			if(lPTMeanSum1>lPTMeanSum2)
			{
				pLineInfo->lConfidence = lPTMeanSum1;
				pLineInfo->lPtNum = lSumNum1;
			}
			else
			{
				pLineInfo->lConfidence = lPTMeanSum2;
				pLineInfo->lPtNum = lSumNum2;
			}
		}
		GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));

		lResponseThres = lineSetInfo.pLineInfo[0].lConfidence * 3 / 5;
		GO(QuickSort(hMemMgr, (MVoid*)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoLengthCmp));
		lIndex = 0;
		for (j=0; j<lineSetInfo.lLineNum; j++)
		{
			LINE_INFO *pLineInfo = lineSetInfo.pLineInfo + j;
			{	
				MChar path[256]={0};
				sprintf(path, "D:\\line\\line2\\line_%d.bmp", j);
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
				/*vDrawLine2((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					0, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb);*/
			}
			if (pLineInfo->lConfidence >= lResponseThres)
			{
				lIndex = j;
				break;
			}
		}
//		printf("**********lIndex=%d, lCon=%d\n", lIndex, lineSetInfo.pLineInfo[lIndex].lMaxLength);
		/*tmpFp = fopen("D:\\log.dat", "ab+");
		fprintf(tmpFp, "SF6   lineNum=%d\n", lineSetInfo.lLineNum);
		fclose(tmpFp);*/

		//���û��ֱ�ߣ����confidenceΪ0
		if (lineSetInfo.lLineNum == 0)
		{
			JPrintf("2315  lineSetInfo.lLineNum=0\n");
			/*tmpFp = fopen("D:\\log.dat", "ab+");
			fprintf(tmpFp, "2315  lineSetInfo.lLineNum=0\n");
			fclose(tmpFp);*/
			pCircleInfo->lConfidence = 0;
			pCircleInfo->lLineNum = 0;
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}

		//����ֱ�ߣ�ȷ��ֱ�ߵķ���
		//ֱ����Բ�Ľ���
		{
			//MPOINT ptCross1, ptCross2, ptMid;
			//LINEPARAM lineParam;

			res = vComputeCrossPt(pCircleInfo->circleParam.xcoord, pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius,
				lineSetInfo.pLineInfo[lIndex].lineParam.Coefk,lineSetInfo.pLineInfo[lIndex].lineParam.Coefb,lineSetInfo.pLineInfo[lIndex].lineParam.bVertical,
				&ptCross1, &ptCross2);
			//			printf("center(%d,%d), r=%d\n", pCircleInfo->circleParam.xcoord, pCircleInfo->circleParam.ycoord, pCircleInfo->circleParam.lRadius);
			//			printf("k=%f, b=%f\n", lineSetInfo.pLineInfo[0].lineParam.Coefk,lineSetInfo.pLineInfo[0].lineParam.Coefb);
			//			printf("ransac_line: cross1(%d,%d), cross2(%d,%d)\n", ptCross1.x, ptCross1.y, ptCross2.x, ptCross2.y);
			if(!res)	// not find two cross
			{
				/*tmpFp = fopen("D:\\log.dat", "ab+");
				fprintf(tmpFp, "SF6  2357 err\n");
				fclose(tmpFp);*/
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}
			else
				res = LI_ERR_NONE;
			ptMid.x = (ptCross1.x+ptCross2.x)/2;
			ptMid.y = (ptCross1.y+ptCross2.y)/2;

			AdjustLinePosition((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,
				pHaarResponse->lWidth, pHaarResponse->lHeight, 
				&lineSetInfo.pLineInfo[lIndex].lineParam,
				&ptMid, &ptCross1, &ptCross2);	

			lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, 
				pHaarResponse->lWidth, pHaarResponse->lHeight, ptCross1, ptCross2, 0xff,MNull);


			/*tmpFp = fopen("D:\\log.dat", "ab+");
			fprintf(tmpFp, "SF6   ptSum=%d, pt1(%d,%d), pt2(%d,%d)\n", lPTMeanSum1, ptCross1.x, ptCross1.y, ptCross2.x, ptCross2.y);
			fclose(tmpFp);*/

			if (lPTMeanSum1<=0)
			{
				pCircleInfo->lConfidence = 0;
				pCircleInfo->lLineNum = 0;
				printf("RANSAC LINE lPTMeanSum<=0~~\n");
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

			//CalcLineEndPt_xsd(hMemMgr, pHaarResponse, pCircleInfo->circleParam.lRadius, lThreshold, &lineParam);
			//printf("******************endPt(%d,%d)\n", lineParam.ptEnd.x, lineParam.ptEnd.y);
			//����ֱ�ߵķ���ֱ������������Բ��ѡ��н�С�ķ���

			//pCircleInfo->lConfidence = pCircleInfo->lConfidence + lPTMeanSum1 *lineSetInfo.pLineInfo[0].lPtNum/3;
			pCircleInfo->lLineNum=1;
			//pCircleInfo->lineInfo.lConfidence=lineSetInfo.pLineInfo[0].lConfidence;
			pCircleInfo->lineInfo.lConfidence= lPTMeanSum1;
			pCircleInfo->lineInfo.lPtNum =lineSetInfo.pLineInfo[lIndex].lPtNum;
			JMemCpy(&(pCircleInfo->lineInfo.lineParam),&(lineParam), sizeof(LINEPARAM));
			JMemCpy(pCircleInfo->lineInfo.pPtinfo, lineSetInfo.pLineInfo[lIndex].pPtinfo,sizeof(PTINFO)*lineSetInfo.pLineInfo[lIndex].lPtNum);
			JPrintf("k=%f, b=%f\n", lineParam.Coefk, lineParam.Coefb);
			//			pCircleInfo->lineInfo.lineParam.ptStart = ptCross1;
			//			pCircleInfo->lineInfo.lineParam.ptEnd = ptCross2;

			{	
				MChar path[256]={0};
				LINE_INFO *pLineInfo = &(pCircleInfo->lineInfo);
				sprintf(path, "D:\\line.bmp");
				B_Cpy(&blockFlagTmp1, pHaarResponse);
				vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
				PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
			}
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

MRESULT RANSAC_LINE_SWITCH(MHandle hMemMgr, BLOCK *pHaarResponse,
				BLOCK* pHaarAngleResponse, MPOINT *pPtList, MLong lPtNum, 
				MPOINT *pTemp,MLong lTmpNum, PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;
	MLong j, k, l, m;
	MPOINT randomPoint[2];
	MPOINT *inPoint=pTemp;
	MLong numInPoint, numInPoint1;
	MDouble distance;
	//MLong lAngleValue,lAngleValue2;
	MLong minNumPtOnLine = 3;	// 3? 2?
	MDouble dCoeffK, dCoeffB, dCoeffK1, dCoeffB1;
	MBool bVert, bVert1;
	MBool bChanged;
	MLong lTotalSumNum,lSumNum1, lSumNum2, lSumNum3;
	MLong lTotalPtNum, lPtNum1, lPtNum2, lPtNum3; 
	MLong lPTMeanSum1,lPTMeanSum2,lPTMeanSum3;
	MLong lMeanSum;
	JGSEED seedTmp = {0};
	LINE_SET_INFO lineSetInfo = {0};
	MPOINT *pTmpPt = MNull;
	BLOCK blockFlagTmp1 = {0};
	LINE_INFO *pLineInfo = MNull;

	dCoeffK = dCoeffK1 = dCoeffB = dCoeffB1 = 0;
	bVert = bVert1 = MFalse;
	numInPoint = numInPoint1 = 0;
	
	if(lPtNum < 2)
	{
		Err_Print("lPtNum<2\n");
		res = LI_ERR_NO_FIND;
		return res;
	}

	seedTmp.lSeedNum = lPtNum;
	AllocVectMem(hMemMgr, seedTmp.pptSeed, lPtNum, MPOINT);
	AllocVectMem(hMemMgr, seedTmp.pcrSeed, lPtNum, MByte);
	AllocVectMem(hMemMgr, pTmpPt, lPtNum, MPOINT);
	B_Create(hMemMgr, &blockFlagTmp1, DATA_U8,pHaarResponse->lWidth, pHaarResponse->lHeight);

	GO(CreateLineSet(hMemMgr, &lineSetInfo, lPtNum*(lPtNum-1)/2, lPtNum));

	JMemCpy(seedTmp.pptSeed, pPtList, lPtNum*sizeof(MPOINT));
	for(j=0; j<seedTmp.lSeedNum; j++)
	{
		randomPoint[0] = seedTmp.pptSeed[j];
		for(k=j+1; k<seedTmp.lSeedNum; k++)
		{
			randomPoint[1] = seedTmp.pptSeed[k];
			//����
			/*lAngleValue = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
						randomPoint[0].y+randomPoint[0].x) &(~TEMPLATE_TYPE_BYTE_BIT);
			lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
						randomPoint[1].y+randomPoint[1].x) &(~TEMPLATE_TYPE_BYTE_BIT);
			dTmpAngle = ABS(lAngleValue -lAngleValue2)*ANGLE_STEP;
			if (dTmpAngle>10 && dTmpAngle<170)
					continue;*/
			if(vFitLine(randomPoint,2,&dCoeffK, &dCoeffB, &bVert)<0)
					continue;

			bChanged = MTrue;
			numInPoint1 = 0;
			while(bChanged)
			{
				bChanged = MFalse;
				numInPoint = 0;

				for(m=0; m<seedTmp.lSeedNum; m++)
				{
					distance = vPointDistToLine2(seedTmp.pptSeed[m], dCoeffK, dCoeffB, bVert);
					/*lAngleValue2 = *((MByte*)pHaarAngleResponse->pBlockData+pHaarAngleResponse->lBlockLine*
					seedTmp.pptSeed[m].y+seedTmp.pptSeed[m].x) &(~TEMPLATE_TYPE_BYTE_BIT);
					dTmpAngle=ABS(lAngleValue-lAngleValue2)*ANGLE_STEP;
					if (dTmpAngle>10 &&dTmpAngle<170)
						continue;*/
					if (distance < DISTANCE_THRESHOLD)
					{
						pTmpPt[numInPoint] = seedTmp.pptSeed[m];
						numInPoint++;
					}
				}

				vFitLine(pTmpPt, numInPoint, &dCoeffK1, &dCoeffB1, &bVert1);
				if(numInPoint1==0||(dCoeffK1!=dCoeffK || dCoeffB1!=dCoeffB || bVert!=bVert1))
				{
					if(numInPoint > numInPoint1)
					{
						bChanged = MTrue;
						dCoeffK = dCoeffK1;
						dCoeffB=dCoeffB1;
						bVert=bVert1;
						numInPoint1 = numInPoint;
						JMemCpy(inPoint, pTmpPt, numInPoint*sizeof(MPOINT));
					}
					else
					{
						if(numInPoint >= minNumPtOnLine && lineSetInfo.lLineNum < lineSetInfo.lMaxLineNum)
						{
							lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK1;
							lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB1;
							lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert1;
							lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
							lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint;
							for (l=0;l<numInPoint;l++)
								lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint=inPoint[l];
							lineSetInfo.lLineNum++;
						}

						bChanged = MFalse;
						break;
					}
				}
			}

			if (numInPoint1 >= minNumPtOnLine &&  lineSetInfo.lLineNum < lineSetInfo.lMaxLineNum)
			{
				lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefk = dCoeffK;
				lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.Coefb = dCoeffB;
				lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.bVertical = bVert;
				lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lineParam.DAngle = 0;
				lineSetInfo.pLineInfo[lineSetInfo.lLineNum].lPtNum =numInPoint1;
				for (l=0;l<numInPoint1;l++)
					lineSetInfo.pLineInfo[lineSetInfo.lLineNum].pPtinfo[l].ptPoint=inPoint[l];
				lineSetInfo.lLineNum++;
			}
		}
	}

	MergeSet((MVoid*)(lineSetInfo.pLineInfo), sizeof(LINE_INFO), &(lineSetInfo.lLineNum), LineInfoCmp, LineInfoReplace);

	//��ÿ��ֱ�߶ε�confidence:
	for(j=0; j<lineSetInfo.lLineNum; j++)
	{
		pLineInfo = lineSetInfo.pLineInfo + j;
		lPTMeanSum1= 0, lPTMeanSum2=0, lPTMeanSum3=0;
		lMeanSum = 0;
		lTotalSumNum = 0;
		lTotalPtNum = 0;

		{	
			MChar path[256]={0};
			sprintf(path, "D:\\line\\line_%d.bmp", j);
			B_Cpy(&blockFlagTmp1, pHaarResponse);
			vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
			PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
		}

		if(pLineInfo->lineParam.bVertical || (pLineInfo->lineParam.Coefk>1 || pLineInfo->lineParam.Coefk<-1))
		{
			GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_Y));
		}
		else
		{
			GO(QuickSort(hMemMgr, (MVoid*)pLineInfo->pPtinfo, pLineInfo->lPtNum, sizeof(PTINFO), PTInfoCmp_X));
		}

		for(k=0; k<pLineInfo->lPtNum-2; k+=2)
		{
			lPTMeanSum1 =  vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint, 0xFF, &lSumNum1, &lPtNum1);
			lPTMeanSum1 *= lSumNum1;
			lPTMeanSum2 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k+1].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF, &lSumNum2, &lPtNum2);
			lPTMeanSum2 *= lSumNum2;

			lPTMeanSum3 = 0;

			if(vComputeIntersactionAngle(pLineInfo->pPtinfo[k].ptPoint,pLineInfo->pPtinfo[k+1].ptPoint,pLineInfo->pPtinfo[k+2].ptPoint)>8*V_PI/9)//160��
			{
				lPTMeanSum3 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+2].ptPoint,0xFF,&lSumNum3, &lPtNum3);
				lPTMeanSum3 *= lSumNum3;
			}
			if (lPTMeanSum3>lPTMeanSum1+lPTMeanSum2)
			{
				lMeanSum+=lPTMeanSum3;
				lTotalSumNum+=lSumNum3;
				lTotalPtNum += lPtNum3;
			}
			else
			{
				lMeanSum+=lPTMeanSum1+lPTMeanSum2;
				lTotalSumNum+=lSumNum1+lSumNum2;
				lTotalPtNum += lPtNum1 + lPtNum2;
			}
		}
		for (; k<pLineInfo->lPtNum-1;k++)
		{
			lPTMeanSum1 = vGetPointMeanValueBtwPt_xsd((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine,pHaarResponse->lWidth, 
						pHaarResponse->lHeight, pLineInfo->pPtinfo[k].ptPoint, pLineInfo->pPtinfo[k+1].ptPoint,0xFF,&lSumNum1, &lPtNum1);
			lMeanSum+=lPTMeanSum1*lSumNum1;
			lTotalSumNum+=lSumNum1;
			lTotalPtNum += lPtNum1;
		}

		//lMeanSum /= lTotalSumNum;
		pLineInfo->lineParam.ptStart = pLineInfo->pPtinfo[pLineInfo->lPtNum-1].ptPoint;
		pLineInfo->lineParam.ptEnd= pLineInfo->pPtinfo[0].ptPoint;
		pLineInfo->lConfidence = lMeanSum;
	}
	GO(QuickSort(hMemMgr, (MVoid *)(lineSetInfo.pLineInfo), lineSetInfo.lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
	if (lineSetInfo.lLineNum == 0)
	{
		JPrintf("lineSetInfo.lLineNum=0\n");
		pParam->pCircleInfo->lConfidence = 0;
		pParam->pCircleInfo->lLineNum = 0;
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	pParam->pCircleInfo->lLineNum = 1;
	pParam->pCircleInfo->lineInfo.lConfidence= lPTMeanSum1;
	pParam->pCircleInfo->lineInfo.lPtNum =lineSetInfo.pLineInfo[0].lPtNum;
	JMemCpy(&(pParam->pCircleInfo->lineInfo.lineParam),&(lineSetInfo.pLineInfo[0].lineParam), sizeof(LINEPARAM));
	JMemCpy(pParam->pCircleInfo->lineInfo.pPtinfo, lineSetInfo.pLineInfo[0].pPtinfo,sizeof(PTINFO)*lineSetInfo.pLineInfo[0].lPtNum);
	{	
		MChar path[256]={0};
		LINE_INFO *pLineInfo = &(lineSetInfo.pLineInfo[0]);
		sprintf(path, "D:\\line.bmp");
		B_Cpy(&blockFlagTmp1, pHaarResponse);
		vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
					255, pLineInfo->lineParam.Coefk, pLineInfo->lineParam.Coefb, pLineInfo->lineParam.bVertical);
		PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
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
MRESULT Get_SF6_Line(MHandle hMemMgr, BLOCK *pHaarResponse, CIRCLE circleParam, PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;

	MPOINT ptCenter, ptCross;
	LINE_INFO curLine, maxLine;
	MLong left, right, top, bottom;
	MLong xx, y1, y2;
	MLong xDiff, sqareR, delta;
	MLong lWidth, lHeight;

	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	left = MAX(0, circleParam.xcoord-circleParam.lRadius);
	right = MIN(lWidth-1, circleParam.xcoord+circleParam.lRadius);
	top = MAX(0, circleParam.ycoord-circleParam.lRadius);
	bottom = MIN(lHeight-1, circleParam.ycoord +circleParam.lRadius);

	maxLine.lConfidence = 0;
	maxLine.lPtNum = 0;
	ptCenter.x = circleParam.xcoord;
	ptCenter.y = circleParam.ycoord;
	sqareR = SQARE(circleParam.lRadius);
	for (xx=left; xx<=right; xx++)
	{
		xDiff = xx - circleParam.xcoord;
		delta = sqareR + SQARE(xDiff);

		curLine.lConfidence = 0;
		curLine.lPtNum = 0;
		y1 = (MLong)(circleParam.ycoord - sqrt((MDouble)delta) + 0.5);
		ptCross.x = xx;
		ptCross.y = y1;
		if (ptCross.x == circleParam.xcoord)
		{
			curLine.lineParam.bVertical = MTrue;
		}
		else
		{
			curLine.lineParam.Coefk = ((MDouble)(ptCross.y - circleParam.ycoord)) / ((MDouble)(ptCross.x - circleParam.xcoord + WYRD_EPSILON));
			curLine.lineParam.Coefb = ptCross.y -curLine.lineParam.Coefk * ptCross.x;
		}
		curLine.lConfidence = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, lWidth, lHeight,
			ptCross, ptCenter, 0xFF, &curLine.lPtNum);
		if (curLine.lConfidence>maxLine.lConfidence && curLine.lPtNum>maxLine.lPtNum)
		{
			maxLine.lConfidence = curLine.lConfidence;
			maxLine.lPtNum = curLine.lPtNum;
			maxLine.lineParam.Coefk = curLine.lineParam.Coefk;
			maxLine.lineParam.Coefb = curLine.lineParam.Coefb;
			maxLine.lineParam.bVertical = curLine.lineParam.bVertical;
			maxLine.lineParam.ptEnd = ptCross;
		}

		curLine.lConfidence = 0;
		curLine.lPtNum = 0;
		y2 = (MLong)(circleParam.ycoord + sqrt((MDouble)delta) + 0.5);
		ptCross.x = xx;
		ptCross.y = y2;
		if (ptCross.x == circleParam.xcoord)
		{
			curLine.lineParam.bVertical = MTrue;
		}
		else
		{
			curLine.lineParam.Coefk = ((MDouble)(ptCross.y - circleParam.ycoord)) / ((MDouble)(ptCross.x - circleParam.xcoord + WYRD_EPSILON));
			curLine.lineParam.Coefb = ptCross.y -curLine.lineParam.Coefk * ptCross.x;
		}
		curLine.lConfidence = vGetPointMeanValueBtwPt((MByte*)pHaarResponse->pBlockData, pHaarResponse->lBlockLine, lWidth, lHeight,
			ptCross, ptCenter, 0xFF, &curLine.lPtNum);
		if (curLine.lConfidence>maxLine.lConfidence && curLine.lPtNum>maxLine.lPtNum)
		{
			maxLine.lConfidence = curLine.lConfidence;
			maxLine.lPtNum = curLine.lPtNum;
			maxLine.lineParam.Coefk = curLine.lineParam.Coefk;
			maxLine.lineParam.Coefb = curLine.lineParam.Coefb;
			maxLine.lineParam.bVertical = curLine.lineParam.bVertical;
			maxLine.lineParam.ptEnd = ptCross;
		}
	}

	JMemCpy(&(pParam->pCircleInfo->lineInfo), &(maxLine), sizeof(LINE_INFO));

	{	
		MChar path[256]={0};
		BLOCK blockFlagTmp1 = {0};
		sprintf(path, "D:\\line.bmp");
		B_Create(hMemMgr, &blockFlagTmp1, DATA_U8,pHaarResponse->lWidth, pHaarResponse->lHeight);
		B_Cpy(&blockFlagTmp1, pHaarResponse);
		vDrawLine3((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 
			255, maxLine.lineParam.Coefk, maxLine.lineParam.Coefb, maxLine.lineParam.bVertical);
		vDrawCircle((MByte*)blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight,
			maxLine.lineParam.ptEnd.x, maxLine.lineParam.ptEnd.y, 2, 255);
		PrintBmpEx(blockFlagTmp1.pBlockData, blockFlagTmp1.lBlockLine, DATA_U8, blockFlagTmp1.lWidth, blockFlagTmp1.lHeight, 1,path);
		B_Release(hMemMgr, &blockFlagTmp1);
	}
	return res;
}
//=============================================================================

MRESULT CalcLineEndPt_xsd(MHandle hMemMgr, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold, LINEPARAM *lineParam)
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

	lMaxLength = (3*lRadius)>>2;	// (3/4)r

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
	minLength = MIN(leftLength, rightLength);

	if (leftLength >= (rightLength * 3 / 2) || (leftLength * 3 / 2) <= rightLength)
	{
		leftWidthConfidence = rightWidthConfidence = 0;
		goto CALC_CONFIDENCE;
	}
	else
	{
		GO(CalcLineEndPtStep2(hMemMgr, pHaarResponse, lRadius, lThreshold, *lineParam,
												minLength, &leftWidthConfidence, &rightWidthConfidence));
	}

CALC_CONFIDENCE:
	ptLeftConfidence = (leftLength<<1) + leftWidthConfidence;
	ptRightConfidence = (rightLength<<1) + rightWidthConfidence;
	JPrintf("******leftCon=%d, rightCon=%d\n", ptLeftConfidence, ptRightConfidence);

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

MRESULT CalcLineEndPtStep2(MHandle hMemMgr, BLOCK *pHaarResponse, MLong lRadius, MLong lThreshold, 
							LINEPARAM lineParam, MLong minLength, MLong *leftWidthConfidence, MLong *rightWidthConfidence)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lMaxWidth, lWidthStep, lWidthRange;
	MPOINT ptLeft, ptRight, ptMid, ptTmp;
	MLong *leftWidthArrray, *rightWidthArray;
	MDouble dCoeffK, dCoeffB, dCoeffKTmp, dCoeffBTmp;
	MBool bVert;
	MByte *leftVal, *rightVal, *pData;
	MByte *tmpVal1, *tmpVal2;
	MLong lWidth, lHeight, lStride;
	MLong leftX, leftY, rightX, rightY, tmpX, tmpY, tmpX2, tmpY2;
	MLong tmpOff;

	//init
	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lStride = pHaarResponse->lBlockLine;
	pData = (MByte*)pHaarResponse->pBlockData;
	dCoeffK = lineParam.Coefk;
	dCoeffB = lineParam.Coefb;
	bVert = lineParam.bVertical;	
	lMaxWidth = MAX(lRadius>>4, 10);		// r/16
	lWidthStep = lRadius>>1;		// r/2

	ptLeft = lineParam.ptStart;
	ptRight = lineParam.ptEnd;
	ptMid = lineParam.ptMid;
	leftWidthArrray = rightWidthArray = MNull;
	leftVal = rightVal = MNull;

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
		for (i=0; i<lWidthStep; i++, leftY--, rightY++)	
		{
			if (leftY>=0 && leftY<lHeight)
			{
				leftVal = pData + leftY * lStride + leftX;
				if (*leftVal >= lThreshold)
					leftWidthArrray[i]++;
				tmpX = leftX - 1;
				tmpY = leftY;
				tmpX2 = leftX + 1;
				tmpY2 = leftY;
				for (j=0; j<lMaxWidth; j++, tmpX--, tmpX2++)
				{
					if (tmpX>=0 && tmpX<lWidth)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						if (*tmpVal1>=lThreshold)
							leftWidthArrray[i]++;
					}
					if (tmpX2>=0 && tmpX2<lWidth)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						if(*tmpVal2>=lThreshold)
							leftWidthArrray[i]++;
					}
				}
			}
			
			if (rightY>=0 && rightY<lHeight)
			{
				rightVal = pData + rightY * lStride + rightX;
				if(*rightVal >= lThreshold)
					rightWidthArray[i]++;
				tmpX = rightX - 1;
				tmpY = rightY;
				tmpX2 = rightX + 1;
				tmpY2 = rightY;
				for (j=0; j<lMaxWidth; j++, tmpX--, tmpX2++)
				{
					if (tmpX>=0 && tmpX<lWidth)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						if (*tmpVal1>=lThreshold)
							rightWidthArray[i]++;
					}
					if (tmpX2>=0 && tmpX2<lWidth)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						if(*tmpVal2>=lThreshold)
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
		for (i=0; i<lWidthStep; i++, leftX--, rightX++)	
		{
			if (leftX>=0 && leftX<lWidth)
			{
				leftVal = pData + leftY * lStride + leftX;
				if (*leftVal >= lThreshold)
					leftWidthArrray[i]++;
				tmpX = leftX;
				tmpY = leftY - 1;
				tmpX2 = leftX;
				tmpY2 = leftY + 1;
				for (j=0; j<lMaxWidth; j++, tmpY--, tmpY2++)
				{
					if (tmpY>=0 && tmpY<lHeight)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						if (*tmpVal1>=lThreshold)
							leftWidthArrray[i]++;
					}
					if (tmpY2>=0 && tmpY2<lHeight)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						if(*tmpVal2>=lThreshold)
							leftWidthArrray[i]++;
					}
				}
			}
			if (rightX>=0 && rightX<lWidth)
			{
				rightVal = pData + rightY * lStride + rightX;
				if (*rightVal >= lThreshold)
					rightWidthArray[i]++;
				tmpX = rightX;
				tmpY = rightY - 1;
				tmpX2 = rightX;
				tmpY2 = rightY + 1;
				for (j=0; j<lMaxWidth; j++, tmpY--, tmpY2++)
				{
					if (tmpY>=0 && tmpY<lHeight)
					{
						tmpVal1 = pData + tmpY * lStride + tmpX;
						if (*tmpVal1>=lThreshold)
							rightWidthArray[i]++;
					}
					if (tmpY2>=0 && tmpY2<lHeight)
					{
						tmpVal2 = pData + tmpY2 * lStride + tmpX2;
						if(*tmpVal2>=lThreshold)
							rightWidthArray[i]++;
					}
				}
			}
		}
	}
	else if (dCoeffK<1.0 && dCoeffK>-1.0)	// x
	{
		dCoeffKTmp = tan(atan(dCoeffK) + V_PI/2);
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
			for (j=0; j<lWidthRange; j++, tmpY++)	// y 
			{
				tmpX = (MLong)((tmpY - dCoeffBTmp) / dCoeffKTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				if(*tmpVal1>=lThreshold)
					leftWidthArrray[i]++;
			}

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
			for (j=0; j<lWidthRange; j++, tmpY++)
			{
				tmpX = (MLong)((tmpY - dCoeffBTmp) / dCoeffKTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				if(*tmpVal1>=lThreshold)
					rightWidthArray[i]++;
			}
		}
	}
	else	// y
	{
		dCoeffKTmp = tan(atan(dCoeffK) + V_PI/2);
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
			for (j=0; j<lWidthRange; j++, tmpX++)
			{
				tmpY = (MLong)(dCoeffKTmp * tmpX + dCoeffBTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				if(*tmpVal1>=lThreshold)
					leftWidthArrray[i]++;
			}

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
			for (j=0; j<lWidthRange; j++, tmpX++)
			{
				tmpY = (MLong)(dCoeffKTmp * tmpX + dCoeffBTmp);
				if(tmpX<0 || tmpX>lWidth || tmpY<0 || tmpY>lHeight)
					break;
				tmpVal1 = pData + tmpY * lStride + tmpX;
				if(*tmpVal1>=lThreshold)
					rightWidthArray[i]++;
			}
		}
	}

	for (i=0; i<lWidthStep; i++)
	{
		if(leftWidthArrray[i]<rightWidthArray[i])
			(*leftWidthConfidence)++;
		else if(leftWidthArrray[i]>rightWidthArray[i])
			(*rightWidthConfidence)++;
		else
		{
			(*leftWidthConfidence)++;
			(*rightWidthConfidence)++;
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

//2 ��ָ���ߺͿ̶�����100������ֵ�ֱ�paint������Ϊ0������������
//2 ��ý��㣬�������ɿ̶ȵ�ľ��룬�������ֵ��
MRESULT ReadNumber2(MHandle hMemMgr,BLOCK *image,MPOINT *point,MLong lNUmber,MDouble k,MDouble b,MDouble *numberData,MDouble *result)
{
    MLong i,j;
    BLOCK imageTemp1={0},imageTemp2={0};
    MLong lLine;
    LineSegmentPoint lineSeg={0};
    MPOINT leftPoint={0};
    MPOINT rightPoint={0};
    MUInt8* pTempData1;
    MUInt8 *pTempData2;
    MLong lX=-1;
    MLong lY=-1;
    MLong sumX=0;
    MLong sumY=0;
    MLong lWidth=0;
    MLong lHeight=0;
    MLong lNum=0;
    MDouble dis1,dis2;
    MRESULT res=LI_ERR_NONE;
    MLong x=0;
    MLong y=0;
    MLong r=0;
    MLong xc=0;
    MLong yc=0;

    GO(B_Create(hMemMgr,&imageTemp1,DATA_U8,image->lWidth,image->lHeight));
    B_Set(&imageTemp1,0);
    GO(B_Create(hMemMgr,&imageTemp2,DATA_U8,image->lWidth,image->lHeight));
    B_Set(&imageTemp2,0);

    PrintBmpEx(image->pBlockData, image->lBlockLine,image->typeDataA & ~0x100,image->lWidth, image->lHeight, 1, "D:\\dkdkd.bmp");
    /*
    lineSeg.startPoint.x=startPoint.x;
    lineSeg.startPoint.y=startPoint.y;
    lineSeg.endPoint.x=endPoint.x;
    lineSeg.endPoint.y=endPoint.y;*/
    lWidth=image->lWidth;
    lHeight=image->lHeight;


    pTempData1=(MUInt8*)(imageTemp1.pBlockData);
    pTempData2=(MUInt8*)(imageTemp2.pBlockData);


    for (i=0;i<lNUmber-1;i++)
    {
        vLineTo((MByte*)(imageTemp1.pBlockData),imageTemp1.lBlockLine,imageTemp1.lWidth,imageTemp1.lHeight,100,point[i],point[i+1]);
    }
    vCircleFitting(point,lNUmber,&x,&y,&r);
    lLine=imageTemp2.lBlockLine;
    //��չ��̶ȵ㣬��ָ��ָ��̶ȵ����ʱ
    for (i=0;i<=x;i++)
    {
        if (i>=x-r)
        {
            if (i>point[lNUmber-1].x)
            {
                yc = (MLong)(y-sqrt((MDouble)r*r-(x-i)*(x-i)));
                if (yc>=0&&yc<lHeight&&yc<=point[lNUmber-1].y)
                {
                    pTempData1[yc*lLine+i]=100;
                }
            }
            if (i>point[0].x)
            {
                yc = (MLong)(y+sqrt((MDouble)r*r-(x-i)*(x-i)));
                if (yc>=0&&yc<lHeight&&yc>=point[0].y)
                {
                    pTempData1[yc*lLine+i]=100;
                }

            }
        }
    }

    //vLineTo_EX((MByte*)(imageTemp2.pBlockData),imageTemp2.lBlockLine,imageTemp2.lWidth,imageTemp2.lHeight,100,startPoint,endPoint);

    vDrawLine2((MByte*)(imageTemp2.pBlockData),imageTemp2.lBlockLine,imageTemp2.lWidth,imageTemp2.lHeight,100,k,b);

    for (j=1;j<image->lHeight-1;j++)
    {
        for (i=1;i<image->lWidth-1;i++)
        {
            if (pTempData1[j*lLine+i]==100)
            {
                if (pTempData2[j*lLine+i]==100)
                {
                    lX=i;
                    lY=j;
                }
                else
                {
                    if (pTempData2[(j-1)*lLine+i]==100||pTempData2[(j+1)*lLine+i]==100||pTempData2[(j)*lLine+i+1]==100||pTempData2[(j)*lLine+i-1]==100)
                    {
                        sumX+=i;
                        sumY+=j;
                        lNum++;
                    }
                }
            }


        }
    }
    if (lX!=-1&&lY!=-1)
    {

    }
    else
    {
        if (lNum!=0)
        {
            lX=sumX/lNum;
            lY=sumY/lNum;
        }
        else
        {
            res=LI_ERR_INDEX_NO_FIND;
            goto EXT;
        }
    }

    //�޸ĳɶ���360�ȶ�����

    PrintBmpEx(imageTemp1.pBlockData,imageTemp1.lBlockLine,DATA_U8,imageTemp1.lWidth,imageTemp1.lHeight,1,"D:\\line1.bmp");
    PrintBmpEx(imageTemp2.pBlockData,imageTemp2.lBlockLine,DATA_U8,imageTemp2.lWidth,imageTemp2.lHeight,1,"D:\\line2.bmp");


    for (i=0;i<lNUmber-1;i++)
    {
        //if (lX>=point[i].x&&lX<=point[i+1].x&&lY>=point[i].y&&lY<=point[i+1].y)
        if ((lX-point[i].x)*(lX-point[i+1].x)<=0&&(lY-point[i].y)*(lY-point[i+1].y)<=0)
            break;
    }

    if (lX>=point[0].x&&lY>=point[0].y)
    {
        *result=numberData[0];
    }
    else
    {
        if (lX>=point[lNUmber-1].x&&lY<=point[lNUmber-1].y)
        {
            *result=numberData[lNUmber-1];
        }
        else
        {
            dis1=sqrt((MDouble)((point[i].y-lY)*(point[i].y-lY)+(point[i].x-lX)*(point[i].x-lX)));
            dis2=sqrt((MDouble)((point[i+1].y-point[i].y)*(point[i+1].y-point[i].y)+(point[i+1].x-point[i].x)*(point[i+1].x-point[i].x)));
            *result=dis1/(dis2+0.001)*(numberData[i+1]-numberData[i])+numberData[i];
        }
    }



EXT:
    B_Release(hMemMgr,&imageTemp1);
    B_Release(hMemMgr,&imageTemp2);
    return res;
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
		sprintf(path, "D:\\origin_circle_line.bmp");
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
			sprintf(path, "D:\\circle,angle%f.bmp",i*0.5);
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
				sprintf(path, "D:\\circle,angle%f_transfer%d.bmp",i*0.5,j);
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
	//��ǰ�����нǶ�Լ�����������ĵ�
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
	//��Ҫ�ų�һЩ�㣬���ڵ�ǰ�����߽Ƕȼ���������ģ���Ҫ�ų�
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
/*
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
	//��ǰ�����нǶ�Լ�����������ĵ�
	for (j=0; j<pCircleInfo->lPtNum; j++)
	{
		pCircleInfo->pPtinfo[j].dPtAngle = vComputeAngle(pCircleInfo->pPtinfo[j].ptPoint.x - pCircleInfo->circleParam.xcoord,
			pCircleInfo->pPtinfo[j].ptPoint.y - pCircleInfo->circleParam.ycoord);
	}
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
	//��Ҫ�ų�һЩ�㣬���ڵ�ǰ�����߽Ƕȼ���������ģ���Ҫ�ų�
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
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+k, lPTListLen*sizeof(PTINFO));
		else
		{
			JMemCpy(pCircleInfo->pPtinfo+(lPTListLen-( pCircleInfo->lPtNum - k+1)), pCircleInfo->pPtinfo+k, (pCircleInfo->lPtNum - k+1)*sizeof(PTINFO));
		}
		pCircleInfo->lPtNum = lPTListLen;
	}
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
	if (lNum!=0)
		dMeanAngle /= lNum;
	else
		dMeanAngle = dAngle;
	dInterAngle = (pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
	dInterAngle = MIN(dInterAngle+2*V_PI, dInterAngle);
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
		confidence_left = (MLong)(*((MByte*)pRspData+lRspDataLine*pCircleInfo->pPtinfo[0].ptPoint.y + pCircleInfo->pPtinfo[0].ptPoint.x) 
		*GaussianWeight(2*dMeanAngle, (MLong)dDiffAngle));

		dInterAngle = fabs(pCircleInfo->pPtinfo[pCircleInfo->lPtNum-2].dPtAngle-pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
		dInterAngle = MIN(2*V_PI-dInterAngle, dInterAngle);
		dDiffAngle = fabs(dInterAngle-dMeanAngle);
		confidence_right = (MLong)(*((MByte*)pRspData+lRspDataLine*pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.y + pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].ptPoint.x) 
			*GaussianWeight(2*dMeanAngle, (MLong)dDiffAngle));
		if(confidence_left<confidence_right)
		{
			JMemCpy(pCircleInfo->pPtinfo, pCircleInfo->pPtinfo+1, (pCircleInfo->lPtNum-1)*sizeof(PTINFO));
			pCircleInfo->lPtNum--;
		}
		else
		{
			pCircleInfo->lPtNum--;
		}
		dInterAngle = (pCircleInfo->pPtinfo[0].dPtAngle - pCircleInfo->pPtinfo[pCircleInfo->lPtNum-1].dPtAngle);
		dInterAngle = MIN(dInterAngle+2*V_PI, dInterAngle);					
	}
EXT:
	return res;
}	*/
// ��Ѱ��ֱ�߽׶Σ��Ƚ��̶ȵ��ų�
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
		QuickSort(hMemMgr, (void*)q1, num1, sizeof(QUADRANT), CmpValUp);
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
MRESULT CalcCircleCenter(MHandle hMemMgr, BLOCK *pHaarAngle, PJGSEEDS seedsList, MLong *pSeedsAngle, MPOINT *ptCenter, MLong lRadius)
{
	MRESULT res = LI_ERR_NONE;

	MLong lCurX, lCurY;
	MLong lWidth, lHeight, lStride, lSize;
	MLong lAngle;
	MLong i, j;
	MLong left, right, top, bottom;
//	MLong lThreshold;
	BLOCK voteMap = {0};
	MLong *pFlag = MNull;
	MByte *pAngleData;
	MByte *voteMapData = MNull;
	MUInt32 *pIntegral, *pIntegralData;
	MLong *pFlagData;
	MLong lMaxData, lTmpData;
	MLong lStep, lExt;
	MLong lSum;

	lWidth = pHaarAngle->lWidth;
	lHeight = pHaarAngle->lHeight;
	lStride = pHaarAngle->lBlockLine;
	lSize = lWidth * lHeight;
	AllocVectMem(hMemMgr, pFlag, lSize, MLong);
	SetVectZero(pFlag, lSize*sizeof(MLong));

	AllocVectMem(hMemMgr, pIntegral, lSize, MUInt32);
	SetVectZero(pIntegral, lSize*sizeof(MUInt32));
	pAngleData = (MByte *)pHaarAngle->pBlockData;
	left = lWidth>>2;
	right = lWidth - left;
	top = lHeight>>2;
	bottom = lHeight - top;

	for (i=0; i<seedsList->lSeedNum; i++)
	{
		lCurX = seedsList->pptSeed[i].x;
		lCurY = seedsList->pptSeed[i].y;
		lAngle = (*(pAngleData + lCurY * lStride + lCurX))&(~TEMPLATE_TYPE_BYTE_BIT);
		lAngle = lAngle * ANGLE_STEP;
		*(pSeedsAngle + i) = lAngle;
		LineVoteInFlag(pFlag, lWidth, lHeight, lCurX, lCurY, lAngle);
	}

	// �����������ǿ�ȣ����ֵ�������ƫ
	lSum = 0;
	pFlagData = pFlag;
	pIntegralData = pIntegral;
	for (i=0; i<lWidth; i++, pFlagData++, pIntegralData++)
	{
		lSum += *pFlagData;
		*pIntegralData = lSum;
	}
	for (j=1; j<lHeight; j++)
	{
		lSum = 0;
		for (i=0; i<lWidth; i++, pFlagData++, pIntegralData++)
		{
			lSum += *pFlagData;
			*pIntegralData = *(pIntegralData-lWidth) + lSum;
		}
	}
	JPrintf("integral====\n");

	pIntegralData = pIntegral + lRadius * lWidth + lRadius;
	lStep = lRadius * lWidth;
	lExt = lRadius<<1;
	lMaxData = 0;
	for(j=lRadius; j<lHeight-lRadius; j++, pIntegralData +=lExt)
	{
		for(i=lRadius; i<lWidth-lRadius; i++, pIntegralData++)
		{
			lTmpData = *(pIntegralData + lStep + lRadius) + *(pIntegralData - lStep - lRadius)
						- *(pIntegralData - lStep + lRadius) - *(pIntegralData + lStep - lRadius);
			if(lTmpData > lMaxData)
			{
				lMaxData = lTmpData;
				ptCenter->x = i;
				ptCenter->y = j;
			}
		}
	}
	
	lMaxData /= SQARE(lRadius+1);
	
	if (ptCenter->x <= left || ptCenter->x >= right || ptCenter->y <= top || ptCenter->y >= bottom)
	{
		Err_Print("circle center error !\n");
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}
	B_Create(hMemMgr, &voteMap, DATA_U8, lWidth, lHeight);
	voteMapData = (MByte *)voteMap.pBlockData;
	lMaxData = 0;
	for (i=0; i<lHeight; i++)
	{
		for (j=0; j<lWidth; j++)
		{
			if (*(pFlag+i*lWidth+j)>lMaxData)
			{
				lMaxData = *(pFlag+i*lWidth+j);
			}
		}
	}
	for (i=0; i<lHeight; i++)
	{
		for (j=0; j<lWidth; j++)
		{
			//if(*(pFlag+i*lWidth+j)>lMaxData)
			//	*(voteMapData + i * voteMap.lBlockLine + j) = 255;
			//else
				*(voteMapData + i * voteMap.lBlockLine + j) = (MByte)(*(pFlag+i*lWidth+j) * 255 / lMaxData);
		}
	}

	PrintBmpEx(voteMap.pBlockData, voteMap.lBlockLine, DATA_U8, voteMap.lWidth, voteMap.lHeight, 1, "D:\\vote.bmp");
//	lThreshold = 195;	// maxData*0.8  (255<<2)/5
//	AdjustCenter(voteMap, ptCenter, lRadius, lThreshold);
	JPrintf("$$$$$  lMax=%d (%d,%d)\n", lMaxData, ptCenter->x, ptCenter->y);
EXT:
	FreeVectMem(hMemMgr, pFlag);
	FreeVectMem(hMemMgr, pIntegral);
	B_Release(hMemMgr, &voteMap);
	return res;
}

MRESULT LineVoteInFlag(MLong *pFlag, MLong lWidth, MLong lHeight, MLong lCurX, MLong lCurY, MLong lAngle)
{
	MRESULT res = LI_ERR_NONE;

	MLong xx, yy;
	MDouble dCoeffK, dCoeffB;
	MDouble dAngle;

	if (90==lAngle || 270==lAngle)
	{
		xx = lCurX;
		for (yy=0; yy<lHeight; yy++)
		{
			*(pFlag + yy*lWidth + xx) += 1;
		}
	}
	else
	{
		dAngle = lAngle * ANG2RAD;
		dCoeffK = tan(dAngle);
		dCoeffB = lCurY - dCoeffK * lCurX;
		if (dCoeffK>=-1.0 && dCoeffK<=1.0)
		{
			for (xx=0; xx<lWidth; xx++)
			{
				yy = (MLong)(dCoeffK * xx + dCoeffB + 0.5);
				if(yy>0 && yy<lHeight)
					*(pFlag + yy*lWidth + xx) += 1;
			}
		}
		else
		{
			dCoeffB /= dCoeffK;
			for (yy=0; yy<lHeight; yy++)
			{
				xx = (MLong)(yy/dCoeffK - dCoeffB + 0.5);
				if(xx>0 && xx<lWidth)
					*(pFlag + yy*lWidth + xx) += 1;
			}
		}
	}

	return res;
}

MRESULT AdjustCenter(BLOCK voteMap, MPOINT *ptCenter, MLong lRadius, MLong lThreshold)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lStride;
	MLong lExt;
	MByte *pVoteData;
	MLong lSumX, lSumY, lSumVal;
	MLong lStartX, lStartY, lEndX, lEndY;

	lStride = voteMap.lBlockLine;
	lExt = lStride - (lRadius<<1) - 1;
	lStartX = ptCenter->x - lRadius;
	lStartY = ptCenter->y - lRadius;
	lEndX = ptCenter->x + lRadius;
	lEndY = ptCenter->y + lRadius;
	pVoteData = (MByte*)voteMap.pBlockData;
	pVoteData += lStartY * lStride + lStartX;

	lSumX = lSumY = lSumVal = 0;
	for (i=lStartY; i<=lEndY; i++, pVoteData+=lExt)
	{
		for (j=lStartX; j<=lEndX; j++, pVoteData++)
		{
			if (*pVoteData >= lThreshold)
			{
				lSumX += *pVoteData * j;
				lSumY += *pVoteData * i;
				lSumVal += *pVoteData;
			}
		}
	}
	ptCenter->x = lSumX / lSumVal;
	ptCenter->y = lSumY / lSumVal;

	return res;
}

#define MIN_SIGMA 3
MRESULT CalcCircleRadius(MHandle hMemMgr, BLOCK *pHarrResponse, PJGSEEDS seedsList, MLong *pSeedsAngle, MPOINT ptCenter, MLong *lRadius)
{
	MRESULT res = LI_ERR_NONE;

	MLong i;
	MLong lPtNum;
	MLong *pPt2Center = MNull;
	MLong lSqareTmp, lDsitTmp;
	MLong lSumVal, lNum;
	MLong lMinDist;
	MLong lTolerance, lDelta, lSigma;
	MLong lNewRadius, lOldRadius;
	MLong lTmpAngle, lDiffAngle;
	MLong dx, dy;
	MLong lMaxCount;

	lPtNum = seedsList->lSeedNum;
	AllocVectMem(hMemMgr, pPt2Center, lPtNum, MLong);
	SetVectZero(pPt2Center, lPtNum * sizeof(MLong));
	lMinDist = MIN(pHarrResponse->lWidth, pHarrResponse->lHeight)>>2;

	lSumVal = lNum = 0;
	for (i=0; i<lPtNum; i++)
	{
		dx = seedsList->pptSeed[i].x - ptCenter.x;
		dy = seedsList->pptSeed[i].y - ptCenter.y;
		if(0==dy)
			continue;
		lTmpAngle = (MLong)(vComputeAngle(dx, dy) *RAD2ANG + 0.5);
		lDiffAngle = ABS(pSeedsAngle[i] - lTmpAngle) + 90;
		if((lDiffAngle>70 && lDiffAngle<110) || (lDiffAngle>250 && lDiffAngle<290))
		{
			lSqareTmp = SQARE(dx) + SQARE(dy);
			lDsitTmp = (MLong)(sqrt((MDouble)lSqareTmp) + 0.5);
			if(lDsitTmp<lMinDist)
				continue;
			pPt2Center[i] = lDsitTmp;
			lSumVal += lDsitTmp;
			lNum++;
		}
	}
	JPrintf("### lNum=%d\n", lNum);
	lNewRadius = lSumVal / lNum;
	lOldRadius = 0;
	lSigma = ABS(lOldRadius - lNewRadius);
	lMaxCount = 0;
	while(lSigma>MIN_SIGMA || lMaxCount>=10)
	{
		JPrintf("lR=%d\n", lNewRadius);
		lOldRadius = lNewRadius;
		lTolerance = lNewRadius / 10;
		lSumVal = lNum = 0;
		for (i=0; i<lPtNum; i++)
		{
			lDelta = ABS(pPt2Center[i] - lNewRadius);
			if (lDelta<=lTolerance)
			{
				lSumVal += pPt2Center[i];
				lNum++;
			}
		}
		JPrintf("### lNum=%d\n", lNum);
		lNewRadius = lSumVal / lNum;
		lSigma = ABS(lOldRadius - lNewRadius);
		lMaxCount++;
	}
	JPrintf("lR=%d\n", lNewRadius);
	*lRadius = lNewRadius;
EXT:
	FreeVectMem(hMemMgr, pPt2Center);
	return res;
}


//======================== get result method for meters=====================================
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

		if (lDirection[0]*lDirection[1]<0)
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

	// �㵽ֱ�߾��룿��
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
MRESULT GetCircleRing(BLOCK *pHaarResponse, CIRCLE *pCircleParam, MLong lDelta, MLong lMeterType)
{
	MRESULT res = LI_ERR_NONE;

	MLong lMaxRadius, lMinRadius, lCurDistance, lMedRadius;
	MPOINT centerPt, curPt;
	MLong i, j;
	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MLong lMaxYPos;
	unsigned char *pData = MNull;
	MLong lQuadHeight, lTmpDiff;

	if (0==pCircleParam->lRadius || 0==pCircleParam->xcoord || 0==pCircleParam->ycoord)
	{
		Err_Print("GetCircleRing input circleParam error!\n");
		res = LI_ERR_NOT_INIT;
		return res;
	}

	pData = (unsigned char*)pHaarResponse->pBlockData;
	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lStride = pHaarResponse->lBlockLine;
	lExt = lStride - lWidth;

	if (LEAKAGE_CURRENT_METER == lMeterType)
	{
		lMaxRadius = pCircleParam->lRadius - 4;
		lMedRadius = pCircleParam->lRadius - lDelta * 8;
		lMinRadius = pCircleParam->lRadius - lDelta * 28;	

		centerPt.x = pCircleParam->xcoord;
		centerPt.y = pCircleParam->ycoord;
		lMaxYPos = pCircleParam->ycoord - lMinRadius;
		lQuadHeight = lHeight>>2;
		lTmpDiff = lHeight - lMaxYPos;
		if (lTmpDiff<lQuadHeight)
		{
			lMinRadius = pCircleParam->lRadius - lDelta * 22;
			lMaxYPos = pCircleParam->ycoord - lMinRadius;
		}
		for (j=0; j<lHeight; j++, pData+=lExt)
		{
			for (i=0; i<lWidth; i++, pData++)
			{
				curPt.x = i;
				curPt.y = j;
				lCurDistance = vDistance3L(curPt, centerPt);
				if(lCurDistance>=lMaxRadius || (lCurDistance<=lMedRadius && j<=lMaxYPos))	// && j<=lMaxYPos
					*pData = 0;
			}
		}
	}
	else if(SF6_GAS_METER == lMeterType)	// SF6
	{
		lMaxRadius = pCircleParam->lRadius;
		lMinRadius = pCircleParam->lRadius - (lDelta<<3);	// 8 9 10

		centerPt.x = pCircleParam->xcoord;
		centerPt.y = pCircleParam->ycoord;
		for (j=0; j<lHeight; j++, pData+=lExt)
		{
			for (i=0; i<lWidth; i++, pData++)
			{
				curPt.x = i;
				curPt.y = j;
				lCurDistance = vDistance3L(curPt, centerPt);
				if(lCurDistance > lMaxRadius || lCurDistance < lMinRadius)
					*pData = 0;
			}
		}
	}

	return res;
}

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

MDouble Point2Line(MPOINT pt, MDouble dCoeffK, MDouble dCoeffB, MLong bVert)
{
	if (bVert)
	{
		return (pt.x + dCoeffB);//����Ҫע�⣬���ǲ��õ���0 = -x+b�ķ���
	}
	return (pt.x*dCoeffK+dCoeffB-pt.y);
}

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
		absDist2Center = fabs(dist2Center);
	}
	else
	{
		ptCenter.x = pCircleParam->xcoord;
		ptCenter.y = pCircleParam->ycoord;
		tmpPt.x = ptCenter.x;
		tmpPt.y = (MLong)(pLineParam->Coefk * tmpPt.x + pLineParam->Coefb + 0.5);

		dAngle = atan(pLineParam->Coefk);
		dist2Center = Point2Line(ptCenter, pLineParam->Coefk, pLineParam->Coefb, pLineParam->bVertical);
		tmpSqare = SQARE(pLineParam->Coefk) + 1;
		dist2Center = dist2Center / (sqrt(tmpSqare) + WYRD_EPSILON);
		absDist2Center = fabs(dist2Center);
	}
	
	if(absDist2Center > pCircleParam->lRadius)
	{
		res = -1;
		Err_Print("calcLineCrossCirclePt : absDist2Center > circleParam.lRadius error!\n");
		return res;
	}
	tmpSqare = SQARE(pCircleParam->lRadius) - SQARE(absDist2Center);
	halfChord = sqrt(tmpSqare);
	if (pLineParam->bVertical)
	{
		ptCross1->x = -pLineParam->Coefb;
		ptCross1->y = ptCenter.y - halfChord;
		ptCross2->x = -pLineParam->Coefb;
		ptCross2->y = ptCenter.y + halfChord;
	}
	else if (pLineParam->Coefk <= 0)
	{
		deltaX = (MLong)(fabs(absDist2Center * sin(dAngle)) + 0.5);
		deltaY = (MLong)(fabs(absDist2Center * cos(dAngle)) + 0.5);

		if(ptCenter.y < tmpPt.y)
			isCenterOnRight = 0;
		else
			isCenterOnRight = 1;

		if (isCenterOnRight == 1)
		{
			ptMid.x = ptCenter.x - deltaX;
			ptMid.y = ptCenter.y - deltaY;

			deltaX = (MLong)(fabs(halfChord * cos(dAngle)) + 0.5);
			deltaY = (MLong)(fabs(halfChord * sin(dAngle)) + 0.5);

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

MLong calcExternLinePtNum(BLOCK *pHaarResponse, LINEPARAM *pLineParam, CIRCLE *pCircleParam, MLong lThreshold, MLong lMaxNumPt)
{
	MLong externPtNum;
	MLong lWidth, lHeight, lStride;
	MByte *pResponseData = MNull;
	MByte tmpData;
	MPOINT ptStart, ptEnd, ptTmp;
	MLong left, right, top, bottom;
	MLong lBadPtNum;
	MLong xx, yy;

	//init
	externPtNum = 0;
	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lStride = pHaarResponse->lBlockLine;
	pResponseData = (MByte*)pHaarResponse->pBlockData;
	ptStart = pLineParam->ptStart;
	ptEnd = pLineParam->ptEnd;
	left = MAX(0, pCircleParam->xcoord-pCircleParam->lRadius);
	right = MIN(lWidth-1, pCircleParam->xcoord + pCircleParam->lRadius);
	top = MAX(0, pCircleParam->ycoord - pCircleParam->lRadius);
	bottom = MIN(lHeight-1, pCircleParam->ycoord + pCircleParam->lRadius);

	if (ptStart.x<0 || ptStart.x>=lWidth || ptStart.y<0 || ptStart.y>=lHeight ||
		ptEnd.x<0 || ptEnd.x>=lWidth || ptEnd.y<0 || ptEnd.y>=lHeight)
	{
		Err_Print("calcExternLinePtNum  ptStart or ptEnd error input!\n");
		return 0;
	}

	// �涨��ֱ�����˵�
	if (pLineParam->bVertical)
	{
		if(ptStart.y>ptEnd.y)
		{
			ptTmp = ptStart;
			ptStart = ptEnd;
			ptEnd = ptTmp;
		}
	}
	else
	{
		if(pLineParam->Coefk<1.0 && pLineParam->Coefk>-1.0)
		{
			if (ptStart.x > ptEnd.x)
			{
				ptTmp = ptStart;
				ptStart = ptEnd;
				ptEnd = ptTmp;
			}
		}
		else
		{
			if (ptStart.y > ptEnd.y)
			{
				ptTmp = ptStart;
				ptStart = ptEnd;
				ptEnd = ptTmp;
			}
		}
	}

	// calc extern line point
	if (pLineParam->bVertical)
	{
		lBadPtNum = 0;
		for (yy=ptStart.y-1; yy>top;yy--)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			tmpData = *(pResponseData + yy*lStride + ptStart.x);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
		lBadPtNum = 0;
		for (yy=ptEnd.y+1; yy<bottom; yy++)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			tmpData = *(pResponseData + yy*lStride + ptEnd.x);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
	}

	if (pLineParam->Coefk<1.0 && pLineParam->Coefk>-1.0)
	{
		lBadPtNum = 0;
		for (xx=ptStart.x-1; xx>left; xx--)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			yy = (MLong)(pLineParam->Coefk * xx + pLineParam->Coefb);
			if(yy<0 || yy>=lHeight)
				break;
			tmpData = *(pResponseData + yy*lStride + xx);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
		lBadPtNum = 0;
		for (xx=ptEnd.x+1; xx<right; xx++)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			yy = (MLong)(pLineParam->Coefk * xx + pLineParam->Coefb);
			if(yy<0 || yy>=lHeight)
				break;
			tmpData = *(pResponseData + yy*lStride + xx);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
	}
	else
	{
		lBadPtNum = 0;
		for (yy=ptStart.y-1; yy>top; yy--)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			xx = (MLong)((yy - pLineParam->Coefb)/pLineParam->Coefk);
			if(xx<0 || xx>=lWidth)
				break;
			tmpData = *(pResponseData + yy*lStride + xx);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
		lBadPtNum = 0;
		for (yy=ptEnd.y+1; yy<bottom; yy++)
		{
			if(lBadPtNum>lMaxNumPt)
				break;
			xx = (MLong)((yy - pLineParam->Coefb)/pLineParam->Coefk);
			if(xx<0 || xx>=lWidth)
				break;
			tmpData = *(pResponseData + yy*lStride + xx);
			if(tmpData>=lThreshold)
				externPtNum++;
			else if(0==tmpData)
				lBadPtNum++;
		}
	}

	return externPtNum;
}

MRESULT getRedDiffImage(MByte *pSrc, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)
{
	MRESULT res = LI_ERR_NONE;

	MByte *pSrcData = MNull;
	MByte *pDstData = MNull;
	MLong lWidth, lHeight;
	MLong lSrcExt, lDstExt;
	MLong lMaxNum, lMaxRedNum;
	MLong lMaxVal, lThresVal, lTmpVal, lRedThresVal;
	MLong redHist[256] = {0};
	MLong i, j;

	if (MNull==pSrc || MNull==pDiffImage || MNull==pHistArray)
	{
		Err_Print("getRedDiffImage input error!");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	pSrcData = pSrc;
	pDstData = (MByte*)pDiffImage->pBlockData;
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lSrcExt = lSrcStride - 3*lWidth;
	lDstExt = pDiffImage->lBlockLine - lWidth;
	lMaxNum = (lWidth * lHeight)>>1;
	lMaxRedNum = lMaxNum>>2;
	lMaxVal = 1;

	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData+=3, pDstData++)
		{
			//lTmpVal = ABS((*(pSrcData+2)<<1) - *(pSrcData+1) - *pSrcData);
			lTmpVal = (*(pSrcData+2)<<1) - *(pSrcData+1) - *pSrcData;
			if(lTmpVal<0)
				lTmpVal = 0;
			*pDstData = (MByte)lTmpVal;
			if(lMaxVal<lTmpVal)
				lMaxVal = lTmpVal;
			if(lTmpVal>255)
				lTmpVal = 255;
			redHist[lTmpVal]++;

			lTmpVal = (*(pSrcData+2) + *(pSrcData+1) + *pSrcData)/3;
			pHistArray[lTmpVal]++;
		}
	}
	PrintBmpEx(pDiffImage->pBlockData, pDiffImage->lBlockLine, DATA_U8, pDiffImage->lWidth, pDiffImage->lHeight, 1, "D:\\diffImage0.bmp");

	lTmpVal = 0; 
	for (i=0; i<256; i++)
	{
		if(lTmpVal<lMaxNum)
		{
			lTmpVal += pHistArray[i];
			lThresVal = i;
		}
		else
			break;
	}
	lThresVal *= 3;
	lTmpVal = 0;
	for (i=255; i>=0; i--)
	{
		if(lTmpVal<lMaxRedNum)
		{
			lTmpVal += redHist[i];
			lRedThresVal = i;
		}
		else
			break;
	}

	pSrcData = pSrc;
	pDstData = (MByte*)pDiffImage->pBlockData;
	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData+=3, pDstData++)
		{
			lTmpVal = *pDstData;
			if (lTmpVal >= lRedThresVal &&(*pSrcData + *(pSrcData+1) + *(pSrcData+2)) <lThresVal)
			{
				*pDstData = (MByte)(64 - (lTmpVal * 64) / lMaxVal);
			}
			else
				*pDstData = 128;	
		}
	}		
	PrintBmpEx(pDiffImage->pBlockData, pDiffImage->lBlockLine, DATA_U8, pDiffImage->lWidth, pDiffImage->lHeight, 1, "D:\\diffImage.bmp");

	return res;
}

MRESULT getBlackDiffImage(MByte *pSrc, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)
{
	MRESULT res = LI_ERR_NONE;

	MByte *pSrcData = MNull;
	MByte *pDstData = MNull;
	MLong lWidth, lHeight;
	MLong lSrcExt, lDstExt;
	MLong lMaxNum;
	MLong lMaxVal, lThresVal, lTmpVal;
	MLong i, j;

	if (MNull==pSrc || MNull==pDiffImage || MNull==pHistArray)
	{
		Err_Print("getBlackDiffImage input error!");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	pSrcData = pSrc;
	pDstData = (MByte*)pDiffImage->pBlockData;
	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lSrcExt = lSrcStride - 3*lWidth;
	lDstExt = pDiffImage->lBlockLine - lWidth;
	lMaxNum = (lWidth * lHeight)>>3;		//%40(whb) 12.5%
	lMaxVal = 1;

	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData+=3, pDstData++)
		{
			lTmpVal = ABS((*(pSrcData+2)<<1) - *(pSrcData+1) - *pSrcData);
			*pDstData = (MByte)lTmpVal;

			if(lMaxVal<lTmpVal)
				lMaxVal = lTmpVal;
		}
	}

	lTmpVal = 0; 
	for (i=0; i<256; i++)
	{
		if(lTmpVal<lMaxNum)
		{
			lTmpVal += pHistArray[i];
			lThresVal = i;
		}
		else
			break;
	}
	lThresVal *= 3;	

	pSrcData = pSrc;
	pDstData = (MByte*)pDiffImage->pBlockData;
	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData+=3, pDstData++)
		{
			if ((*pSrcData + *(pSrcData+1) + *(pSrcData+2)) <lThresVal)
			{
				lTmpVal = *pDstData;
				*pDstData = (MByte)((lTmpVal * 128) / lMaxVal);
			}
			else
				*pDstData = 128;
		}
	}		
	PrintBmpEx(pDiffImage->pBlockData, pDiffImage->lBlockLine, DATA_U8, pDiffImage->lWidth, pDiffImage->lHeight, 1, "D:\\diffBlack.bmp");

	return res;
}

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
	lMaxNum = (lWidth * lHeight)>>1;     // whb(50%) /8 /4 /2
	lMaxRedNum = (lWidth * lHeight)>>6;                //  1/32 1/64
	lMaxVal = 1;

	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		for (j=0; j<lWidth; j++)
		{
			pHistArray[*pSrcYData]++;
			pSrcYData++;
		}

	}

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

	for (i=0; i<lHeight; i++)
	{
		pSrcYData = pSrcY + i*lSrcStride;
		pSrcVData = pSrcV + (i>>1)*lSrcStride;
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;

		for (j=0; j<lWidth; j++)
		{
			if (*pSrcYData<lThresVal && *pSrcVData>lRedThresVal)
			{
				*pDstData = (MByte)(64 - (*pSrcVData<<6) / lMaxVal);
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



MRESULT getBlackDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MByte *pSrcU, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray)
{

	MRESULT res = LI_ERR_NONE;

	MLong i,j;
	MLong lMaxNum, lTmpNum;
	MLong lThresVal;
	MLong lWidth, lHeight, lExt;
	MLong lMaxVal = 0;
	MByte *pSrcYData = MNull;
	MByte *pSrcVData = MNull;
	MByte *pSrcUdata = MNull;
	MByte *pDstData = MNull;
	MByte *pDstPtr = MNull;

	lWidth = pDiffImage->lWidth;
	lHeight = pDiffImage->lHeight;
	lExt = pDiffImage->lBlockLine - lWidth;
	pSrcYData = pSrcY;
	pSrcVData = pSrcV;
	pSrcUdata = pSrcU;

	pDstPtr = (MByte*)pDiffImage->pBlockData;

	//lMaxNum = (lWidth * lHeight)>>3;     // >>3  (0.4 whb)
	lMaxNum = (lWidth * lHeight)>>2;
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
		pDstData = pDstPtr + i*pDiffImage->lBlockLine;
		for (j=0; j<lWidth; j++)
		{
			if(*pSrcYData<lThresVal)
				*pDstData = (MByte)((*pDstData * 128) / lMaxVal);
			else
				*pDstData = 128;
			pSrcYData++;
			pDstData++;
		}
	}

	return res;
}

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

MLong calcMaxContinuityLength(MByte *pData, MLong lStride, MLong lWidth, MLong lHeight, 
							  MPOINT ptCross1, MPOINT ptCross2, MLong lMask)
{
	MLong lResult;
	MLong xx, yy;
	MLong lStart, lEnd;
	MDouble k, b;
	MLong lMaxLength, lStep;
	MByte *pTmpData;
	MLong lTmpVal;

	lResult = 1;
	lMaxLength = 0;
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
					}
					lStep = 0;
				}
			}
		}
	}

	if (lMaxLength > lResult)
	{
		lResult = lMaxLength;
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