#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Location.h"
#include "rotateBlock.h"
#include "lidebug.h"
#include "liedge.h"
#include "limath.h"
#include "litimer.h"
#include "liintegral.h"
#include "sort.h"
#include "lisort.h"

//=========================================================================
#define MIN_PT_NUM_ON_CIR 5
#define MIN_RATIO 4		// 矩形最小长宽比
#define MAX_RATIO 25	// 矩形最大长宽比
#define MAX_PT_LIST_NUM	50
#define MIN_PT_NUM_ON_LINE 3
#define _8_9_PI	2.79253		// 8/9 * PI
//=========================================================================

//=========================================================================
MRESULT GetCalibLocation(MHandle hMemMgr, BLOCK* pImage, PARAM_INFO *pParam)
{
	MRESULT res = LI_ERR_NONE;

	MLong i;
	
	BLOCK haarResult = {0};
	BLOCK haarAngle = {0};
	HaarParam haarParam = {0};
	MLong lHaarWidth;

	MLong lStackLen;
	MLong lThreshold;
	MLong lSeedSize;
	MPOINT *ptTmpList = MNull;
	BLOCK blockFlag = {0};
	SEEDSETINFO seedList = {0};
	SEEDINFO *pSeedInfo = MNull;

	MLong *pMark = MNull;
	RegionInfo regInfo;

	CIRCLE_SET_INFO circleSet = {0};

	JPrintf("image[%d,%d]\n", pImage->lWidth, pImage->lHeight);

	GO(B_Create(hMemMgr, &haarResult, DATA_U8, pImage->lWidth, pImage->lHeight));
	GO(B_Create(hMemMgr, &haarAngle, DATA_U8, pImage->lWidth, pImage->lHeight));

	PrintBmpEx(pImage->pBlockData, pImage->lBlockLine, DATA_U8, pImage->lWidth, pImage->lHeight, 1, "D:\\gray.bmp");

	//lHaarWidth = 5;
	lHaarWidth = DIV_ROUND(pImage->lHeight, 100);
	haarParam.w = lHaarWidth;
	haarParam.h = lHaarWidth * 3;
	haarParam.lClassNumber = 4;
	haarParam.lAngleNumber = 180 / ANGLE_STEP;
	AllocVectMem(hMemMgr, haarParam.pAngleList, haarParam.lAngleNumber, MLong);
	haarParam.pAngleList[0] = 0;
	for (i=1; i<haarParam.lAngleNumber; i++)
	{
		haarParam.pAngleList[i] = haarParam.pAngleList[i] + ANGLE_STEP;	// i*ANGLE_STEP
	}
	GO(HaarResponseAngle(hMemMgr, pImage, &haarParam, &haarResult, &haarAngle));
	PrintBmpEx(haarResult.pBlockData, haarResult.lBlockLine, DATA_U8, haarResult.lWidth, haarResult.lHeight, 1, "D:\\response.bmp");
	JPrintf("haar response sucess !\n");

	lStackLen = (haarResult.lWidth * haarResult.lHeight)>>3;
	AllocVectMem(hMemMgr, ptTmpList, lStackLen, MPOINT);
	GO(StatisticNonZeroValue(hMemMgr, (MByte*)haarResult.pBlockData, haarResult.lBlockLine, haarResult.lWidth, 
								haarResult.lHeight, 15, (MVoid*)ptTmpList, lStackLen * sizeof(MPOINT), 75, &lThreshold));
	JPrintf("lThreshold=%d\n", lThreshold);
	
	GO(B_Create(hMemMgr, &blockFlag, DATA_U8, haarResult.lWidth, haarResult.lHeight));
	SetVectZero(blockFlag.pBlockData, haarResult.lBlockLine * haarResult.lHeight);
	lSeedSize = haarResult.lWidth * haarResult.lHeight;
	AllocVectMem(hMemMgr, pSeedInfo, lSeedSize, SEEDINFO);
	seedList.pSeedInfo = pSeedInfo;
	seedList.lMaxNum = lSeedSize;
	seedList.lSeedNum = 0;
	AllocVectMem(hMemMgr, pMark, haarResult.lWidth*haarResult.lHeight, MLong);
	SetVectZero(pMark, haarResult.lWidth*haarResult.lHeight * sizeof(MLong));

	regInfo.lRegNum = 0;
	regInfo.lMaxNum = 1000;
	regInfo.lRegSize = (lHaarWidth * lHaarWidth);
	regInfo.lHaarWidth = lHaarWidth<<1;
	AllocVectMem(hMemMgr, regInfo.pRegCenter, 1000, MPOINT);
	SetVectZero(regInfo.pRegCenter, 1000 * sizeof(MPOINT));
	AllocVectMem(hMemMgr, regInfo.pRegArea, 1000, MLong);
	SetVectZero(regInfo.pRegArea, 1000*sizeof(MLong));
	AllocVectMem(hMemMgr, regInfo.pRegMeanVal, 1000, MLong);
	SetVectZero(regInfo.pRegMeanVal, 1000*sizeof(MLong));
	AllocVectMem(hMemMgr, regInfo.pRegWidth, 1000, MLong);
	SetVectZero(regInfo.pRegWidth, 1000*sizeof(MLong));
	GO(GetRegions(hMemMgr, &haarResult, &blockFlag, pMark, ptTmpList, lStackLen, lThreshold, 128, &seedList, &regInfo));
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1, "D:\\regions.bmp");
	JPrintf("GetRegions \n");

//	pSeedInfo = seedList.pSeedInfo;
	GO(QuickSort(hMemMgr, (MVoid*)pSeedInfo, seedList.lSeedNum, sizeof(SEEDINFO), SeedInfoCmp_Y));	

	GO(CreateCircleSet(hMemMgr, &circleSet, 1000, seedList.lSeedNum<<1));
	
	//=============circleRank===========
	GO(CircleRank(hMemMgr, &haarResult, pMark, &seedList, &regInfo, &circleSet, ptTmpList, lStackLen));

//	JPrintf("get circle sucess !\n (x,y,r):(%d,%d,%d), cons=%d, radio=%f, lPtNum=%d\n", circleSet.pCircleInfo->circleParam.xcoord, circleSet.pCircleInfo->circleParam.ycoord,
//				circleSet.pCircleInfo->circleParam.lRadius, circleSet.pCircleInfo->lConfidence, circleSet.pCircleInfo->dFitRadio, circleSet.pCircleInfo->lPtNum);
	vDrawCircle((MByte*)blockFlag.pBlockData, blockFlag.lBlockLine, blockFlag.lWidth, blockFlag.lHeight,
		circleSet.pCircleInfo->circleParam.xcoord, circleSet.pCircleInfo->circleParam.ycoord,
		circleSet.pCircleInfo->circleParam.lRadius, 255);
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1, "D:\\circleResult.bmp");
//	JPrintf("- - - - -\n");	
	
	if (circleSet.pCircleInfo!=MNull &&circleSet.pCircleInfo[0].lPtNum > 0)
	{
		CircleInfoCpy_xsd(circleSet.pCircleInfo, pParam->pCircleInfo);
		pParam->bUpdated = MTrue;
	}

EXT:
	B_Release(hMemMgr, &haarResult);
	B_Release(hMemMgr, &haarAngle);
	FreeVectMem(hMemMgr, haarParam.pAngleList);
	FreeVectMem(hMemMgr, ptTmpList);
	B_Release(hMemMgr, &blockFlag);
	FreeVectMem(hMemMgr, pSeedInfo);
	FreeVectMem(hMemMgr, pMark);
	FreeVectMem(hMemMgr, regInfo.pRegCenter);
	FreeVectMem(hMemMgr, regInfo.pRegArea);
	FreeVectMem(hMemMgr, regInfo.pRegMeanVal);
	FreeVectMem(hMemMgr, regInfo.pRegWidth);
	FreeCircleSet(hMemMgr, &circleSet);

	 return res;
}

MRESULT GetRegions(MHandle hMemMgr, BLOCK *pSrc, BLOCK *pDst, MLong *pMark, 
					MVoid *pTmpMem, MLong lMemLen, MLong lThres, MByte dstLabel,
					SEEDSETINFO *pSeeds, RegionInfo *pRegInfo)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight, lStride;
	MByte *pSrcData = MNull;
	MByte *pDstData = MNull;
	MLong *pMarkData = MNull;
	MPOINT *ptStack = MNull;
	MByte *ptValStack = MNull;
	MLong lSrcExt, lDstExt;
	MLong lStackLen;
	MLong i, j;
	MLong lHighThres, lLowThres;

	lWidth = pSrc->lWidth;
	lHeight = pSrc->lHeight;
	lStride = pSrc->lBlockLine;
	pSrcData = (MByte *)pSrc->pBlockData;
	lSrcExt = lStride - lWidth;
	pDstData = (MByte *)pDst->pBlockData;
	lDstExt = pDst->lBlockLine - lWidth;
	pMarkData = pMark;
	ptStack = (MPOINT *)pTmpMem;
	AllocVectMem(hMemMgr, ptValStack, lMemLen, MByte);

	lHighThres = lThres;
	lLowThres = (lThres>>1);
	lStackLen = 0;
	for (i=0; i<lHeight; i++, pSrcData+=lSrcExt, pDstData+=lDstExt)
	{
		for (j=0; j<lWidth; j++, pSrcData++, pDstData++, pMarkData++)
		{
			if(i==0 || i==lHeight-1 || j==0 || j==lWidth-1)
			{
				*pMarkData = -1;
				*pDstData = 0;
				continue;
			}
			if (*pSrcData>lHighThres && lStackLen<lMemLen)
			{
				if (*pSrcData > *(pSrcData-1)
					&& *pSrcData > *(pSrcData+1)
					&& *pSrcData > *(pSrcData-lStride)
					&& *pSrcData > *(pSrcData+lStride))
				{
					ptStack[lStackLen].x = j;
					ptStack[lStackLen].y = i;
					ptValStack[lStackLen++] = *pSrcData;
				}
			}
			else if(*pSrcData < lLowThres)
			{
				*pMarkData = -1;
				*pDstData = 0;
			}
		}
	}
//	JPrintf("lStackLen=%d\n", lStackLen);
	for(i=0; i<lStackLen; i++)
	{
		if (0 == *(pMark + ptStack[i].y * lWidth + ptStack[i].x))	// 该种子点所在区域还未被标记
		{
			if (pRegInfo->lRegNum >= pRegInfo->lMaxNum)
			{
				LOCATION_ERR("too many regions err !\n");
				res = LI_ERR_UNKNOWN;
				goto EXT;
			}
			pRegInfo->lRegNum++;
			RegionGrow(hMemMgr, pSrc, pDst, pMark, ptStack[i], ptValStack[i], lThres, dstLabel, pRegInfo, pSeeds);
		}
	}
	JPrintf("pSeedNum=%d\n", pSeeds->lSeedNum);	

EXT:
	FreeVectMem(hMemMgr, ptValStack);
	return res;
}

MRESULT RegionGrow(MHandle hMemMgr, BLOCK *pSrc, BLOCK *pDst, MLong *pMark, MPOINT ptSeed, MByte ptVal, 
					MLong lThresVal, MByte label, RegionInfo *pRegInfo, SEEDSETINFO *pSeeds)
{
	MRESULT res = LI_ERR_NONE;

	MLong xx, yy, k;
	MLong lStart, lEnd;
	MLong lUltThresVal;
	MLong sumX, sumY, sumVal;
	MPOINT cenPt, curPt;
	MByte *pSrcData = MNull;
	MByte *pDstData = MNull;
	MLong lWidth, lHeight, lStride;
	MLong lDstStride;
	MDouble dAngle;
	MLong minRegSize, maxRegSize;
	MByte tmpData;

	MPOINT *ptStack = MNull;
	MLong lMemLen;
	MLong xNum[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	MLong yNum[8] = {0, -1, -1, -1, 0, 1, 1, 1};

	lUltThresVal = (lThresVal * 3)/5;	// or 0.6, 0.5, 0.8
	lWidth = pSrc->lWidth;
	lHeight = pSrc->lHeight;
	lStride = pSrc->lBlockLine;
	pSrcData = (MByte *)pSrc->pBlockData;
	pDstData = (MByte *)pDst->pBlockData;
	lDstStride = pDst->lBlockLine;
	lMemLen = (lHeight*lWidth)>>3;
	AllocVectMem(hMemMgr, ptStack, lMemLen, MPOINT);

	lStart = lEnd = 0;

	ptStack[0] = ptSeed;
	*(pMark + ptSeed.y * lWidth + ptSeed.x) = pRegInfo->lRegNum;
	sumX = sumY = sumVal = 0;
	while(lStart <= lEnd)
	{
		curPt = ptStack[lStart];
		tmpData = *(pSrcData + curPt.y * lStride + curPt.x);
		sumVal += tmpData;
		sumX += tmpData * curPt.x;
		sumY += tmpData * curPt.y;
		for (k=0; k<8; k++)
		{
			xx = curPt.x + xNum[k];
			yy = curPt.y + yNum[k];
			if(xx>0 && xx<lWidth && yy>0 && yy<lHeight)
			{
				if (lUltThresVal <= *(pSrcData + yy * lStride + xx) && 0 == *(pMark + yy * lWidth + xx))
				{
					lEnd++;
					if(lEnd>=lMemLen)
						break;
					ptStack[lEnd].x = xx;
					ptStack[lEnd].y = yy;

					*(pMark + yy * lWidth + xx) = pRegInfo->lRegNum;
					*(pDstData + yy * lDstStride + xx) = label;
				}
			}
		}
		lStart++;
	}

	minRegSize = (pRegInfo->lRegSize)<<4;
	maxRegSize = minRegSize<<2;

	cenPt.x = sumX / sumVal;
	cenPt.y = sumY / sumVal;
	if(cenPt.x>0 && cenPt.x<lWidth && cenPt.y>0 && cenPt.y<lHeight)
	{
		dAngle = CalcRegionDirection(ptStack, lStart, cenPt);
		if(label == *(pDstData + cenPt.y * lDstStride + cenPt.x) && lStart >= minRegSize && lStart <= maxRegSize)
		{
			*(pDstData + cenPt.y * lDstStride + cenPt.x) = 255;
			pSeeds->pSeedInfo[pSeeds->lSeedNum].seedPos = cenPt;
			pSeeds->pSeedInfo[pSeeds->lSeedNum].seedVal = *(pSrcData + cenPt.y * lStride + cenPt.x);		// or meanVal
			pSeeds->pSeedInfo[pSeeds->lSeedNum].seedAngle = dAngle;
			pSeeds->pSeedInfo[pSeeds->lSeedNum].seedArea = lStart;
			pSeeds->lSeedNum++;
		}
		else
			res = LI_ERR_UNKNOWN;

		pRegInfo->pRegCenter[pRegInfo->lRegNum] = cenPt;
		pRegInfo->pRegArea[pRegInfo->lRegNum] = lStart;
		pRegInfo->pRegMeanVal[pRegInfo->lRegNum] = sumVal / lStart;
		pRegInfo->pRegWidth[pRegInfo->lRegNum] = calcRectWidth(cenPt, pDst, label, dAngle);
	}
	else
	{
		pRegInfo->pRegArea[pRegInfo->lRegNum] = 1;		// invalid
		pRegInfo->pRegMeanVal[pRegInfo->lRegNum] = 1;
		pRegInfo->pRegWidth[pRegInfo->lRegNum] = pRegInfo->pRegArea[pRegInfo->lRegNum];
		res = LI_ERR_UNKNOWN;
	}

EXT:
	FreeVectMem(hMemMgr, ptStack);
	return res;
}

MLong calcRectWidth(MPOINT center, BLOCK *pSrc, MByte label, MDouble dAngle)
{
	MLong lRectWidth;
	MDouble dCoeffK, dCoeffB;
	MLong lWidth, lHeight, lStride;
	MByte *pSrcData;
	MLong xx, yy;

	lWidth = pSrc->lWidth;
	lHeight = pSrc->lHeight;
	lStride = pSrc->lBlockLine;
	pSrcData = (MByte *)pSrc->pBlockData;
	lRectWidth = 1;

	dAngle = dAngle + 90;
	if(dAngle>360)
		dAngle -= 360;
	if (90==dAngle || 270==dAngle)
	{
		xx = center.x;
		yy = center.y - 1;
		while(yy>=0)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				yy--;
			}
			else
				break;
		}
		yy = center.y + 1;
		while(yy<lHeight)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				yy++;
			}
			else
				break;
		}

		return lRectWidth;
	}

	dCoeffK = tan(dAngle*ANG2RAD);
	dCoeffB = center.y - dCoeffK * center.x;
	if (dCoeffK>=-1.0 && dCoeffK<=1.0)
	{
		xx = center.x - 1;
		yy = (MLong)(dCoeffK * xx + dCoeffB);
		while (xx>=0 && yy>=0 && yy<lHeight)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				xx--;
				yy = (MLong)(dCoeffK * xx + dCoeffB);
			}
			else
				break;
		}
		xx = center.x + 1;
		yy = (MLong)(dCoeffK * xx + dCoeffB);
		while (xx<lWidth && yy>=0 && yy<lHeight)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				xx++;
				yy = (MLong)(dCoeffK * xx + dCoeffB);
			}
			else
				break;
		}
		return lRectWidth;
	}
	else
	{
		yy = center.y - 1;
		xx = (MLong)((yy-dCoeffB)/dCoeffK);
		while(xx>=0 && xx<lWidth && yy>=0)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				yy--;
				xx = (MLong)((yy-dCoeffB)/dCoeffK);
			}
			else
				break;
		}
		yy = center.y + 1;
		xx = (MLong)((yy-dCoeffB)/dCoeffK);
		while(xx>=0 && xx<lWidth && yy>=0)
		{
			if (*(pSrcData + yy*lStride + xx) >= label)
			{
				lRectWidth++;
				yy++;
				xx = (MLong)((yy-dCoeffB)/dCoeffK);
			}
			else
				break;
		}
		
		return lRectWidth;
	}
}

MDouble CalcRegionDirection(MPOINT *ptList, MLong lPtNum, MPOINT center)
{
	MDouble result;
	MLong i;
	MDouble lxDiff, lyDiff;
	MDouble mat[3] = {0,0,0};
	MDouble tmp;

	for (i=0; i<lPtNum; i++)
	{
		lxDiff = (MDouble)(ptList[i].x - center.x);
		lyDiff = (MDouble)(ptList[i].y - center.y);
		mat[0] += lxDiff * lxDiff;	//mat11
		mat[1] += lxDiff * lyDiff;	//mat12, mat21
		mat[2] += lyDiff * lyDiff;	//mat22
	}
	mat[0] /= lPtNum;
	mat[1] /= lPtNum;
	mat[2] /= lPtNum;

	tmp = (mat[2] - mat[0] + sqrt(SQARE(mat[0]-mat[2]) + 4*SQARE(mat[1]))) / (2*mat[1]);
	result = (atan(tmp) * RAD2ANG);
	if(result<0)
		result += 360;

	return result;
}

MRESULT GetAllCircles(MHandle hMemMgr, BLOCK *pHaarResponse, SEEDSETINFO *pSeeds, 
						CIRCLE_SET_INFO *pCircleSetInfo, MPOINT *ptTmp, MLong lNum)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, k, l;
	MLong cx1, cy1, cr1;
	MLong cx2, cy2, cr2;
	MLong lRadiusCons, lLowRadius, lHighRadius;
	MLong lLeftX, lRightX, lTopY, lBottomY;
	MPOINT randSeed[3];
	MLong lPtNumOnCir;
	MPOINT *ptOnCir = MNull;
	MDouble radiusAngle, seedAngle, diffAngle;
	MLong lTolerance;

	lRadiusCons = MAX(pHaarResponse->lWidth, pHaarResponse->lHeight);
	lLowRadius = lRadiusCons>>2;
	lHighRadius = lRadiusCons>>1;
	lLeftX = (pHaarResponse->lWidth)>>2;	// 1/4 w
	lRightX = 3 * lLeftX;	// 3/4 w
	lTopY = (pHaarResponse->lHeight)>>2;	// 1/4 h
	lBottomY = 3 * lTopY;	// 3/4 h

	ptOnCir = ptTmp;

	for (i=0; i<pSeeds->lSeedNum; i++)
	{
		randSeed[0] = pSeeds->pSeedInfo[i].seedPos;
		for (j=i+1; j<pSeeds->lSeedNum; j++)
		{
			randSeed[1] = pSeeds->pSeedInfo[j].seedPos;
			for (k=j+1; k<pSeeds->lSeedNum; k++)
			{
				randSeed[2] = pSeeds->pSeedInfo[k].seedPos;
				if(vCircleFitting(randSeed, 3, &cx1, &cy1, &cr1)<0)
					continue;
				if(cx1<lLeftX || cx1>lRightX || cy1<lTopY || cy1>lBottomY || cr1<lLowRadius || cr1>lHighRadius)	//圆参数不合适
					continue;
				lPtNumOnCir = 0;
				lTolerance = cr1>>4;
				lTolerance = MAX(lTolerance, 5);
				for (l=0; l<pSeeds->lSeedNum; l++)	//根据圆参数筛选刻度点
				{
					if(0==bOnCircle_EX(cx1, cy1, cr1, pSeeds->pSeedInfo[l].seedPos.x, pSeeds->pSeedInfo[l].seedPos.y, lTolerance))
						continue;
					radiusAngle = vComputeAngle(pSeeds->pSeedInfo[l].seedPos.x - cx1, pSeeds->pSeedInfo[l].seedPos.y - cy1) * RAD2ANG;
					if(radiusAngle<0)
						radiusAngle += 360;
					seedAngle = pSeeds->pSeedInfo[l].seedAngle;
					diffAngle = fabs(seedAngle - radiusAngle);

					if(((diffAngle<=15) || (diffAngle>=165 && diffAngle<=195)) && lPtNumOnCir<lNum)		// 0/180
						ptOnCir[lPtNumOnCir++] = pSeeds->pSeedInfo[l].seedPos;
				}	
				if (lPtNumOnCir>=MIN_PT_NUM_ON_CIR && pCircleSetInfo->lParamNum < pCircleSetInfo->lMaxParamNum
					&& lPtNumOnCir < pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].lMaxPtNum)
				{
					if(vCircleFitting(ptOnCir, lPtNumOnCir, &cx2, &cy2, &cr2) < 0)
						continue;

					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.xcoord = cx2;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.ycoord = cy2;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].circleParam.lRadius = cr2;
					for (l=0; l<lPtNumOnCir; l++)
					{
						pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].pPtinfo[l].ptPoint = ptOnCir[l];
					}
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].lPtNum = lPtNumOnCir;
					pCircleSetInfo->pCircleInfo[pCircleSetInfo->lParamNum].lConfidence = 0;
					pCircleSetInfo->lParamNum++;
				}	
			}
		}
	}
	JPrintf("%d circles\n", pCircleSetInfo->lParamNum);
	MergeSet((MVoid *)pCircleSetInfo->pCircleInfo, sizeof(CIRCLE_INFO), &(pCircleSetInfo->lParamNum),CircleInfoCmp, CircleInfoReplace);
	JPrintf("%d circles\n", pCircleSetInfo->lParamNum);

	return res;
}

MLong notExistInStack(MLong *pStack, MLong lStackLen, MLong lData)
{
	MLong res = 1;

	MLong i;

	for (i=0; i<lStackLen; i++)
	{
		if(pStack[i] == lData)
		{
			res = 0;
			break;
		}
	}

	return res;
}

MRESULT CalcCircleConfidence(MHandle hMemMgr, BLOCK *pHaarResponse, SEEDSETINFO *pSeeds,
							RegionInfo *pRegion, MLong *pMark, CIRCLE_INFO *pCircleInfo)
{
	MRESULT res = LI_ERR_NONE;

	MLong i;
	MLong minSize, maxSize;	 // 矩形面积大小
	MLong lCrossRegNum;			 // 圆弧经过的矩形数目
	MLong lMeanResponse;	 // 圆弧经过的矩形的响应均值
	MLong left, right;		 // 拟合圆上搜索范围
	MLong lx, ly1, ly2;
	MLong rr, xx;
	MDouble delta;
	MLong *pMarkStack;
	MLong lMarkData;
	MLong ratio;	// 长宽比

	AllocVectMem(hMemMgr, pMarkStack, pRegion->lRegNum, MLong);
	SetVectZero(pMarkStack, pRegion->lRegNum * sizeof(MLong));
	lCrossRegNum = 0;
	lMeanResponse = 0;
	minSize = (pRegion->lRegSize)<<3;
	maxSize = minSize<<3;

	left = MAX(pCircleInfo->circleParam.xcoord - pCircleInfo->circleParam.lRadius, 0);
	right = MIN(pCircleInfo->circleParam.xcoord  + pCircleInfo->circleParam.lRadius, pHaarResponse->lWidth-1);
	rr = SQARE(pCircleInfo->circleParam.lRadius);
	for (i=left; i<=right; i++)
	{
		xx = pCircleInfo->circleParam.xcoord - i;
		xx = SQARE(xx);
		delta = sqrt((MDouble)(rr-xx));
		lx = i;
		ly1 = (MLong)(pCircleInfo->circleParam.ycoord - delta + 0.5);
		ly2 = (MLong)(pCircleInfo->circleParam.ycoord + delta + 0.5);
		if (ly1 == ly2 && ly1>=0 && ly1<pHaarResponse->lHeight)
		{
			lMarkData = *(pMark + pHaarResponse->lWidth * ly1 + lx);
			if (lMarkData > 0 && pRegion->pRegArea[lMarkData]>=minSize && pRegion->pRegArea[lMarkData]<=maxSize)
			{
				ratio = pRegion->pRegArea[lMarkData] / pRegion->pRegWidth[lMarkData] / pRegion->pRegWidth[lMarkData];
				if (1==notExistInStack(pMarkStack, lCrossRegNum, lMarkData) && ratio>=MIN_RATIO && ratio<=MAX_RATIO)
				{
					pMarkStack[lCrossRegNum] = lMarkData;
					lMeanResponse += pRegion->pRegMeanVal[lMarkData];
					lCrossRegNum++;
				}	
			}
		}
		else
		{
			if (ly1 >= 0)
			{
				lMarkData = *(pMark + pHaarResponse->lWidth * ly1 + lx);
				if (lMarkData > 0 && pRegion->pRegArea[lMarkData]>=minSize && pRegion->pRegArea[lMarkData]<=maxSize)
				{
					ratio = pRegion->pRegArea[lMarkData] / pRegion->pRegWidth[lMarkData] / pRegion->pRegWidth[lMarkData];
					if (1==notExistInStack(pMarkStack, lCrossRegNum, lMarkData) && ratio>=MIN_RATIO && ratio<=MAX_RATIO)
					{
						pMarkStack[lCrossRegNum] = lMarkData;
						lMeanResponse += pRegion->pRegMeanVal[lMarkData];
						lCrossRegNum++;
					}	
				}
			}
			if (ly2 < pHaarResponse->lHeight)
			{
				lMarkData = *(pMark + pHaarResponse->lWidth * ly2 + lx);
				if (lMarkData > 0 && pRegion->pRegArea[lMarkData]>=minSize && pRegion->pRegArea[lMarkData]<=maxSize)
				{
					ratio = pRegion->pRegArea[lMarkData] / pRegion->pRegWidth[lMarkData] / pRegion->pRegWidth[lMarkData];
					if (1==notExistInStack(pMarkStack, lCrossRegNum, lMarkData) && ratio>=MIN_RATIO && ratio<=MAX_RATIO)
					{
						pMarkStack[lCrossRegNum] = lMarkData;
						lMeanResponse += pRegion->pRegMeanVal[lMarkData];
						lCrossRegNum++;
					}	
				}
			}
		}
	}

	pCircleInfo->lConfidence = lCrossRegNum;
//	JPrintf("---cons=%d\n", lCrossRegNum);
EXT:
	FreeVectMem(hMemMgr, pMarkStack);
	return res;
}

MDouble CalcCircleRadio(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo)
{
	MDouble res = 0;
	MDouble dx, dy;
	MDouble tmp;
	MDouble sum = 0;
	MLong i;

	for (i=0; i<pCircleInfo->lPtNum; i++)
	{
		dx = (MDouble)SQARE((pCircleInfo->pPtinfo[i].ptPoint.x - pCircleInfo->circleParam.xcoord));
		dy = (MDouble)SQARE((pCircleInfo->pPtinfo[i].ptPoint.y - pCircleInfo->circleParam.ycoord));
		tmp = sqrt(dx + dy);
		sum += pCircleInfo->circleParam.lRadius - fabs(pCircleInfo->circleParam.lRadius - tmp);
	}

	res = sum / (pCircleInfo->circleParam.lRadius * pCircleInfo->lPtNum);
//	JPrintf("### %f\n", res);
	return res;
}

MLong SeedInfoCmp_Y(const MVoid* p1, const MVoid* p2)
{
	SEEDINFO *_p1 = (SEEDINFO *)p1;
	SEEDINFO *_p2 = (SEEDINFO *)p2;

	if(_p1->seedPos.y < _p2->seedPos.y)
		return 1;
	if(_p1->seedPos.y > _p2->seedPos.y)
		return -1;

	return 0;
}

MLong CircleInfoFitRadioCmp(const MVoid* p1, const MVoid* p2)
{
	CIRCLE_INFO *_p1 = (CIRCLE_INFO*)p1;
	CIRCLE_INFO *_p2 = (CIRCLE_INFO *)p2;

	if(_p1->dFitRadio < _p2->dFitRadio)
		return 1;
	if(_p1->dFitRadio > _p2->dFitRadio)
		return -1;	

	return 0;
}

MLong CircleInfoPtNumCmp(const MVoid* p1, const MVoid *p2)
{
	CIRCLE_INFO *_p1 = (CIRCLE_INFO*)p1;
	CIRCLE_INFO *_p2 = (CIRCLE_INFO*)p2;

	if(_p1->lPtNum < _p2->lPtNum)
		return 1;
	if(_p1->lPtNum > _p2->lPtNum)
		return -1;

	return 0;
}

MRESULT CircleRank(MHandle hMemMgr, BLOCK *pHaarResponse, MLong *pMark, SEEDSETINFO *pSeeds, 
				RegionInfo *pRegion, CIRCLE_SET_INFO *pCircleSetInfo, MPOINT *ptTmp, MLong lNum)
{
	MRESULT res = LI_ERR_NONE;

	int i;
	MLong lCircleNum;
	CIRCLE_INFO *pCircleInfo = MNull;

	if (MNull==hMemMgr || MNull==pHaarResponse || MNull==pMark
		|| MNull==pSeeds || MNull==pRegion || MNull==pCircleSetInfo || MNull==ptTmp)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}
	GetAllCircles(hMemMgr, pHaarResponse, pSeeds, pCircleSetInfo, ptTmp, lNum);

	lCircleNum = pCircleSetInfo->lParamNum;
	if (0 >= lCircleNum)
	{
		LOCATION_ERR("lCircleNum <= 0 error !\n");
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	GO(QuickSort(hMemMgr, (MVoid *)(pCircleSetInfo->pCircleInfo), lCircleNum, 
		sizeof(CIRCLE_INFO), CircleInfoPtNumCmp));
	for (i=0; i<lCircleNum; i++)
	{
		pCircleInfo = pCircleSetInfo->pCircleInfo + i;
		CalcCircleConfidence(hMemMgr, pHaarResponse, pSeeds, pRegion, pMark, pCircleInfo);
		pCircleInfo->dFitRadio = CalcCircleRadio(hMemMgr, pCircleInfo);
	}
	JPrintf("sort ptNum OK \n");
	
	GO(OptimizePtList(hMemMgr, pHaarResponse, pCircleSetInfo->pCircleInfo, pMark, pRegion, ptTmp, lNum));
	JPrintf("OptimizePtList OK \n");


/*	goto EXT;

	for (i=0; i<lCircleNum; i++)
	{
		pCircleInfo = pCircleSetInfo->pCircleInfo + i;
		CalcCircleConfidence(hMemMgr, pHaarResponse, pSeeds, pRegion, pMark, pCircleInfo);
	}
	JPrintf("calc confidence OK \n");
	GO(QuickSort(hMemMgr, (MVoid *)(pCircleSetInfo->pCircleInfo), lCircleNum, 
		sizeof(CIRCLE_INFO), CircleInfoConfidenceCmp));		// rank based on crossNUM
	JPrintf("sort confidence OK \n");	

	lCircleNum = MIN(lCircleNum>>2, 5);
	for (i=0; i<lCircleNum; i++)
	{
		pCircleInfo = pCircleSetInfo->pCircleInfo + i;
		pCircleInfo->dFitRadio = CalcCircleRadio(hMemMgr, pCircleInfo);
	}
	GO(QuickSort(hMemMgr, (MVoid *)(pCircleSetInfo->pCircleInfo), lCircleNum, 
		sizeof(CIRCLE_INFO), CircleInfoFitRadioCmp));		// rank based on radio		*/
EXT:
	return res;
}

MRESULT SearchPtOnCircle(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, RegionInfo *pRegion, MLong *pMark, 
							MLong *pStack, MPOINT *ptTmp, MLong *lPtNum, MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	MLong *pMarkStack = MNull;
	MLong lLeft, lRight;
	MLong cx, cy, cr;
	MLong rr, xx;
	MLong lx, ly1, ly2;
	MLong lNum;
	MLong i;
	MDouble delta;
	MLong lMarkData;
	MPOINT tmpPt;

	pMarkStack = pStack;

	cx = pCircleInfo->circleParam.xcoord;
	cy = pCircleInfo->circleParam.ycoord;
	cr = pCircleInfo->circleParam.lRadius;
	lLeft = MAX(0, cx-cr);
	lRight = MIN(lWidth, cx+cr);
	rr = cr * cr;
	lNum = 0;

	for (i=lLeft; i<lRight; i++)
	{
		xx = SQARE(cx - i);
		delta = sqrt((MDouble)(rr - xx));
		lx = i;
		ly1 = (MLong)(cy - delta + 0.5);
		ly2 = (MLong)(cy + delta + 0.5);
		if (ly1 != ly2)
		{
			if (ly1>=0)
			{
				lMarkData = *(pMark + ly1 * lWidth + lx);
				tmpPt.x = lx;
				tmpPt.y = ly1;
				SearchPtOnCircle_step0(hMemMgr, pRegion, pCircleInfo->circleParam, lMarkData, ptTmp, &lNum, tmpPt, pMarkStack);
			}
			if (ly2<lHeight)
			{
				lMarkData = *(pMark + ly2 * lWidth + lx);
				tmpPt.x = lx;
				tmpPt.y = ly2;
				SearchPtOnCircle_step0(hMemMgr, pRegion, pCircleInfo->circleParam, lMarkData, ptTmp, &lNum, tmpPt, pMarkStack);
			}
		}
		else
		{
			lMarkData = *(pMark + ly1 * lWidth + lx);
			tmpPt.x = lx;
			tmpPt.y = ly1;
			SearchPtOnCircle_step0(hMemMgr, pRegion, pCircleInfo->circleParam, lMarkData, ptTmp, &lNum, tmpPt, pMarkStack);
		}
	}

	*lPtNum = lNum;

	return res;
}

MRESULT SearchPtOnCircle_step0(MHandle hMemMgr, RegionInfo *pRegion, CIRCLE cirParam, MLong lMarkData, 
								MPOINT *ptTmp, MLong *lPtNum, MPOINT srcPt, MLong *pStack)
{
	MRESULT res = LI_ERR_NONE;

	MLong lSigma, lEpsilon, lTmpRadius, lTmpSquare;
	MLong lSize;
	MLong ratio;
	MLong lNum;

	lSigma = (cirParam.lRadius)>>4;
	lSize = (pRegion->lRegSize)<<4;
	lNum = *lPtNum;
	if (lMarkData>0 && pRegion->pRegArea[lMarkData] >= lSize)
	{
		ratio = pRegion->pRegArea[lMarkData] / pRegion->pRegWidth[lMarkData] / pRegion->pRegWidth[lMarkData];
		if (1==notExistInStack(pStack, lNum, lMarkData) && ratio>=MIN_RATIO && ratio<=MAX_RATIO)
		{
			pStack[lNum] = lMarkData;
			lTmpSquare = SQARE(cirParam.xcoord - pRegion->pRegCenter[lMarkData].x) + SQARE(cirParam.ycoord - pRegion->pRegCenter[lMarkData].y);
			lTmpRadius = (MLong)sqrt((MDouble)lTmpSquare);
			lEpsilon = abs(cirParam.lRadius - lTmpRadius);
			if (lEpsilon < lSigma)
			{
				ptTmp[lNum] = pRegion->pRegCenter[lMarkData];
				lNum++;
			}
			else
			{
				ptTmp[lNum] = srcPt;
				lNum++;
			}
		}
	}
	*lPtNum = lNum;

	return res;
}

MRESULT AdjustPtPos(MHandle hMemMgr, RegionInfo *pRegion, MLong *pMark, MPOINT *ptCross, MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	MLong lStep, lRadius;
	MLong lMarkData;
	MPOINT tmpPt;

	lRadius = pRegion->lHaarWidth + 1;
	lStep = 0;

	while(lStep < lRadius)
	{
		tmpPt.x = ptCross->x - lStep;	//left
		tmpPt.y = ptCross->y;
		if (tmpPt.x>0 && tmpPt.x<lWidth && tmpPt.y>0 && tmpPt.y<lHeight)
		{
			lMarkData = *(pMark + tmpPt.y * lWidth + tmpPt.x);
			if(lMarkData > 0)
				break;
		}

		tmpPt.x = ptCross->x;			//top
		tmpPt.y = ptCross->y - lStep;
		if (tmpPt.x>0 && tmpPt.x<lWidth && tmpPt.y>0 && tmpPt.y<lHeight)
		{
			lMarkData = *(pMark + tmpPt.y * lWidth + tmpPt.x);
			if(lMarkData > 0)
				break;
		}

		tmpPt.x = ptCross->x + lStep;	//right
		tmpPt.y = ptCross->y;
		if (tmpPt.x>0 && tmpPt.x<lWidth && tmpPt.y>0 && tmpPt.y<lHeight)
		{
			lMarkData = *(pMark + tmpPt.y * lWidth + tmpPt.x);
			if(lMarkData > 0)
				break;
		}

		tmpPt.x = ptCross->x;			//bottom
		tmpPt.y = ptCross->y + lStep;
		if (tmpPt.x>0 && tmpPt.x<lWidth && tmpPt.y>0 && tmpPt.y<lHeight)
		{
			lMarkData = *(pMark + tmpPt.y * lWidth + tmpPt.x);
			if(lMarkData > 0)
				break;
		}
		lStep++;
	}
	if(lStep < lRadius)
		*ptCross = tmpPt;

	return res;
}

MRESULT AddCrossPt(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, RegionInfo *pRegion, MLong *pMark,
					MPOINT *ptTmp, MLong *lPtNum, MLong *pDistTmp, MLong lPtDist, MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	CIRCLE cirParam1, cirParam2;
	MPOINT crossPt1, crossPt2;
	MLong dist1, dist2;
	MLong lTmpLen, sigma, tmpDist, tmpSqare;
	MLong lNum;
	MLong i, j;
	
	lNum = *lPtNum;
	lTmpLen = (pRegion->lHaarWidth)<<2;
	sigma = lPtDist>>1;

	pDistTmp[lNum-1] = lPtDist;
	cirParam1 = pCircleInfo->circleParam;
	cirParam2.lRadius = lPtDist;

	for (i=0; i<lNum-1; i++)
	{
		tmpDist = abs(lPtDist - pDistTmp[i]);
		if (tmpDist>sigma && lNum<=lTmpLen)
		{
			cirParam2.xcoord = ptTmp[i].x;
			cirParam2.ycoord = ptTmp[i].y;
			if(-1 == FindTwoCirclePt(cirParam1, cirParam2, &crossPt1, &crossPt2, lWidth, lHeight))
				continue;
			tmpSqare = SQARE(ptTmp[i+1].x-crossPt1.x) + SQARE(ptTmp[i+1].y - crossPt1.y);
			dist1 = (MLong)(sqrt((MDouble)tmpSqare + 0.5));
			tmpSqare = SQARE(ptTmp[i+1].x-crossPt2.x) + SQARE(ptTmp[i+1].y - crossPt2.y);
			dist2 = (MLong)(sqrt((MDouble)tmpSqare + 0.5));
			if (dist1 <= dist2)
			{
				AdjustPtPos(hMemMgr, pRegion, pMark, &crossPt1, lWidth, lHeight);
				for (j=lNum; j>i+1; j--)
				{
					ptTmp[j] = ptTmp[j-1];
					pDistTmp[j] = pDistTmp[j-1];
				}
				ptTmp[j] = crossPt1;
				tmpSqare = SQARE(ptTmp[j].x-ptTmp[j+1].x) + SQARE(ptTmp[j].y - ptTmp[j+1].y);
				pDistTmp[j] = (MLong)(sqrt((MDouble)tmpSqare + 0.5));
				lNum++;		
			}
			else 
			{
				AdjustPtPos(hMemMgr, pRegion, pMark, &crossPt2, lWidth, lHeight);
				for (j=lNum; j>i+1; j--)
				{
					ptTmp[j] = ptTmp[j-1];
					pDistTmp[j] = pDistTmp[j-1];
				}
				ptTmp[j] = crossPt2;
				tmpSqare = SQARE(ptTmp[j].x-ptTmp[j+1].x) + SQARE(ptTmp[j].y - ptTmp[j+1].y);
				pDistTmp[j] = (MLong)(sqrt((MDouble)tmpSqare + 0.5));
				lNum++;		
			}
		}
	}
	*lPtNum = lNum;

	return res;
}

MRESULT OptimizePtList(MHandle hMemMgr, BLOCK *pHaarResponse, CIRCLE_INFO *pCircleInfo, 
						MLong *pMark, RegionInfo *pRegion, MPOINT *ptTmp, MLong lNum)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight;
	MLong *pMarkStack;

	MLong left, right;
	MLong i, j;
	MPOINT center;
	MLong *pDistArray;
	MLong *pDstTmp;
	MLong tmpDist, tmpSqare;
	MLong lMaxCount, lPtDist;
	MLong epsilon;
	MLong lRadius, lTmpLen;

	AllocVectMem(hMemMgr, pMarkStack, pRegion->lRegNum, MLong);
	SetVectZero(pMarkStack, pRegion->lRegNum * sizeof(MLong));

	lWidth = pHaarResponse->lWidth;
	lHeight = pHaarResponse->lHeight;
	lPtDist = 0;

	SearchPtOnCircle(hMemMgr, pCircleInfo, pRegion, pMark, pMarkStack, ptTmp, &lNum, lWidth, lHeight);

	center.x = pCircleInfo->circleParam.xcoord;
	center.y = pCircleInfo->circleParam.ycoord;
	lRadius = (pCircleInfo->circleParam.lRadius)>>1;
	lTmpLen = lNum<<2;

	circleSort(hMemMgr, ptTmp, lNum, center, 1, 2);
	
	AllocVectMem(hMemMgr, pDistArray, lRadius, MLong);
	SetVectZero(pDistArray, lRadius * sizeof(MLong));
	AllocVectMem(hMemMgr, pDstTmp, lTmpLen, MLong);
	SetVectZero(pDstTmp, lTmpLen * sizeof(MLong));
	for (i=0; i<lNum-1; i++)
	{
		tmpSqare = SQARE(ptTmp[i].x-ptTmp[i+1].x) + SQARE(ptTmp[i].y - ptTmp[i+1].y);
		tmpDist = (MLong)(sqrt((MDouble)tmpSqare + 0.5));
		pDstTmp[i] = tmpDist;
		left = MAX(1, tmpDist-2);
		right = MIN(lRadius, tmpDist+2);
		for(j=left; j<=right; j++)
		{
			pDistArray[j]++;
		}
	}
	lMaxCount = 0;
	for (i=0; i<lRadius; i++)
	{
		if(pDistArray[i]>lMaxCount)
		{
			lMaxCount = pDistArray[i];
			lPtDist = i;
		}
	}
	if (0==lPtDist)
	{
		LOCATION_ERR("0==lPtDist error!\n");
		goto EXT;
	}
	AddCrossPt(hMemMgr, pCircleInfo, pRegion, pMark, ptTmp, &lNum, pDstTmp, lPtDist, lWidth, lHeight);
//	JPrintf("cirNum=%d, lNum=%d\n", pCircleInfo->lPtNum, lNum);
	
	epsilon = pRegion->lHaarWidth;
	for (i=0; i<lNum; i++)		// merge two ptList
	{
		if(pCircleInfo->lPtNum >= lTmpLen)
			break;
		for (j=0; j<pCircleInfo->lPtNum; j++)
		{
			if(abs(pCircleInfo->pPtinfo[j].ptPoint.x - ptTmp[i].x)<epsilon 
				&& abs(pCircleInfo->pPtinfo[j].ptPoint.y - ptTmp[i].y)<epsilon)
				break;
		}
		if (j==pCircleInfo->lPtNum)
		{
			pCircleInfo->pPtinfo[j].ptPoint = ptTmp[i];
			pCircleInfo->lPtNum++;
		}
	}
	JPrintf("cirNum=%d, lNum=%d\n", pCircleInfo->lPtNum, lNum);

EXT:
	FreeVectMem(hMemMgr, pMarkStack);
	FreeVectMem(hMemMgr, pDistArray);
	FreeVectMem(hMemMgr, pDstTmp);
	return res;
}

//求两圆交点pt1, pt2
MLong FindTwoCirclePt(CIRCLE cirParam1, CIRCLE cirParam2, MPOINT *pt1, MPOINT *pt2, MLong lWidth, MLong lHeight)		// cr1 > cr2
{
	MLong res = 0;

	MLong A, B, C;
	MDouble C_B, A_B;
	MDouble D, E, F, H;
	MDouble delta;

	A = (cirParam2.xcoord - cirParam1.xcoord)<<1;
	B = (cirParam2.ycoord - cirParam1.ycoord)<<1;
	C = SQARE(cirParam1.lRadius) - SQARE(cirParam2.lRadius) + SQARE(cirParam2.xcoord) 
		+ SQARE(cirParam2.ycoord) -SQARE(cirParam1.xcoord) - SQARE(cirParam1.ycoord);

	if (0==B)
	{
		delta = 0.866 * (cirParam2.lRadius>>1);		// 0.866 = 1.732/2
		pt1->y = (MLong)(cirParam1.ycoord - delta + 0.5);
		pt2->y = (MLong)(cirParam2.ycoord - delta + 0.5);
		pt1->x = pt2->x = (MLong)(((MDouble)C)/A);
	}
	else
	{
		C_B = ((MDouble)C) / B;
		A_B = ((MDouble)A) / B;
		D = SQARE(A_B) + 1;
		E = 2 * (cirParam1.ycoord*A_B - cirParam1.xcoord - C_B*A_B);
		F = SQARE(cirParam1.xcoord) + SQARE(C_B) - 2*cirParam1.ycoord*C_B + SQARE(cirParam1.ycoord) - SQARE(cirParam1.lRadius);
		H = SQARE(E) - 4 * D * F;
		if(H<0)
		{
			pt1->x = pt1->y = -1;
			pt2->x = pt2->y = -1;
			return res = -1;
		}
		delta = sqrt(H);
		pt1->x = (MLong)(0.5 + (-E+delta)/(2*D));
		pt1->y = (MLong)(0.5 + C_B - A_B * pt1->x);
		pt2->x = (MLong)(0.5 + (-E-delta)/(2*D));
		pt2->y = (MLong)(0.5 + C_B - A_B * pt2->x);
	}

	if(pt1->x<0 || pt1->x>=lWidth || pt1->y<0 || pt1->y>=lHeight)
		pt1->x = pt1->y = 0;
	if(pt2->x<0 || pt2->x>=lWidth || pt2->y<0 || pt2->y>=lHeight)
		pt2->x = pt2->y = 0;

	return res;
}

MVoid CircleInfoCpy_xsd(CIRCLE_INFO *pSrcCircle, CIRCLE_INFO *pDstCircle)
{
	if (pSrcCircle==MNull || pDstCircle==MNull)return;
	pDstCircle->lConfidence = pSrcCircle->lConfidence;
	pDstCircle->circleParam.xcoord = pSrcCircle->circleParam.xcoord;
	pDstCircle->circleParam.ycoord = pSrcCircle->circleParam.ycoord;
	pDstCircle->circleParam.lRadius = pSrcCircle->circleParam.lRadius;
	pDstCircle->dFitRadio = pSrcCircle->dFitRadio;

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

//================================================ line location ==========================================================================
MRESULT GetLineLocation(MHandle hMemMgr, BLOCK *pImage, PARAM_INFO *pParam, MLong lHaarWidth, MLong lPercentage)
{
	MRESULT res = LI_ERR_NONE;

	MLong lStackLen;
	MLong lThreshold;
	MLong lSeedSize;
	MPOINT *ptTmpList = MNull;
	BLOCK blockFlag = {0};
	JGSEED seedList = {0};
	CIRCLE cirParam = {0};
	LINE_SET_INFO lineSet = {0};
	
	lStackLen = (pImage->lWidth * pImage->lHeight)>>3;
	AllocVectMem(hMemMgr, ptTmpList, lStackLen, MPOINT);
	GO(StatisticNonZeroValue(hMemMgr, (MByte*)pImage->pBlockData, pImage->lBlockLine, pImage->lWidth, 
		pImage->lHeight, 0, (MVoid*)ptTmpList, lStackLen * sizeof(MPOINT), lPercentage, &lThreshold));
	JPrintf("line --- lThreshold=%d\n", lThreshold);

	GO(B_Create(hMemMgr, &blockFlag, DATA_U8, pImage->lWidth, pImage->lHeight));
	SetVectZero(blockFlag.pBlockData, blockFlag.lBlockLine * blockFlag.lHeight);
	lSeedSize = blockFlag.lWidth * blockFlag.lHeight;
	seedList.lMaxNum = lSeedSize;
	seedList.lSeedNum = 0;

	AllocVectMem(hMemMgr, seedList.pptSeed, lSeedSize, MPOINT);
	AllocVectMem(hMemMgr, seedList.pcrSeed, lSeedSize, MByte);
	seedList.lMaxNum = lSeedSize;
	seedList.lSeedNum = 0;
	GO(LocalMax_Circle(hMemMgr, (MByte*)pImage->pBlockData, pImage->lBlockLine, (MByte *)blockFlag.pBlockData,blockFlag.lBlockLine,\
		blockFlag.lWidth,blockFlag.lHeight,ptTmpList,lStackLen, lThreshold, (MLong)(lThreshold*0.6), 50, 128, &seedList, lHaarWidth));
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\line_localMaxPre.bmp");
	JPrintf("========seedsNum=%d\n", seedList.lSeedNum);
	ridgePoint(hMemMgr, (MByte*)pImage->pBlockData, pImage->lBlockLine, pImage->lWidth, pImage->lHeight, 
		(MByte*)blockFlag.pBlockData, blockFlag.lBlockLine, lHaarWidth, (MLong)(lThreshold*0.6), 0, 250, &seedList);
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1,"D:\\lineRidge.bmp");
	JPrintf("========seedsNum=%d\n", seedList.lSeedNum);

	cirParam.xcoord = pParam->pCircleInfo->circleParam.xcoord;
	cirParam.ycoord = pParam->pCircleInfo->circleParam.ycoord;
	cirParam.lRadius = pParam->pCircleInfo->circleParam.lRadius;
	GO(CreateLineSet(hMemMgr, &lineSet, (seedList.lSeedNum*(seedList.lSeedNum-1)>>1), seedList.lSeedNum));
	GO(LinesRank(hMemMgr, pImage, &seedList, cirParam, ptTmpList, lStackLen, &lineSet));
	JPrintf("k=%f, b=%f\n", lineSet.pLineInfo[0].lineParam.Coefk, lineSet.pLineInfo[0].lineParam.Coefb);
	vDrawLine2((MByte*)pImage->pBlockData, pImage->lBlockLine, pImage->lWidth, pImage->lHeight, 
				255, lineSet.pLineInfo[0].lineParam.Coefk, lineSet.pLineInfo[0].lineParam.Coefb);
	PrintBmpEx(pImage->pBlockData, pImage->lBlockLine, DATA_U8, pImage->lWidth, pImage->lHeight, 1,"D:\\line.bmp");

	if (lineSet.pLineInfo!=MNull && lineSet.pLineInfo[0].lPtNum>0)
	{
		//lineInfoCpy
		pParam->bUpdated = MTrue;
	}
EXT:
	FreeVectMem(hMemMgr, ptTmpList);
	B_Release(hMemMgr, &blockFlag);
	FreeVectMem(hMemMgr,seedList.pptSeed);
	FreeVectMem(hMemMgr,seedList.pcrSeed);

	return res;
}

MRESULT LinesRank(MHandle hMemMgr, BLOCK *pResponse, JGSEED *pSeedList, CIRCLE cirParam, MPOINT *ptTmp, MLong lNum, LINE_SET_INFO *pLineSetInfo)
{
	MRESULT res = LI_ERR_NONE;

	MLong i;
	MLong lLineNum;
	LINE_INFO *pLineInfo = MNull;
	MPOINT ptCross1, ptCross2, ptMid;

	if (MNull==hMemMgr || MNull==pSeedList || MNull==ptTmp || MNull==pLineSetInfo)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	GO(GetAllLines(hMemMgr, pSeedList, cirParam, ptTmp, lNum, pLineSetInfo));
	lLineNum = pLineSetInfo->lLineNum;
	for (i=0; i<lLineNum; i++)
	{
		pLineInfo = pLineSetInfo->pLineInfo + i;
		CalcLineConfidence(hMemMgr, pResponse, cirParam, pLineInfo);
		JPrintf(" %d ", pLineInfo->lConfidence);
	}
	JPrintf("\n########################\n");
	//sort
	GO(QuickSort(hMemMgr, (MVoid *)(pLineSetInfo->pLineInfo), pLineSetInfo->lLineNum, sizeof(LINE_INFO), LineInfoConfidenceCmp));
	
	vComputeCrossPt(cirParam.xcoord, cirParam.ycoord, cirParam.lRadius, pLineSetInfo->pLineInfo[0].lineParam.Coefk,
					pLineSetInfo->pLineInfo[0].lineParam.Coefb, pLineSetInfo->pLineInfo[0].lineParam.bVertical, &ptCross1, &ptCross2);
	ptMid.x = (ptCross1.x + ptCross2.x)>>1;
	ptMid.y = (ptCross1.y + ptCross2.y)>>1;

	AdjustLinePosition((MByte*)pResponse->pBlockData, pResponse->lBlockLine, pResponse->lWidth, pResponse->lHeight,
						&pLineSetInfo->pLineInfo[0].lineParam, &ptMid, &ptCross1, &ptCross2);

EXT:
	return res;
}

MRESULT GetAllLines(MHandle hMemMgr, JGSEED *pSeedList, CIRCLE cirParam, MPOINT *ptTmp, MLong lNum, LINE_SET_INFO *pLineSetInfo)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, k;
	MPOINT randPoint[2];
	MDouble dCoeffK, dCoeffB, dCoeffK1, dCoeffB1;
	MBool bVert, bVert1;
	MPOINT *ptOnLine = MNull;
	MPOINT *ptListTmp = MNull;
	MLong lPtOnLineNum, lPointNum;
	MLong lDistance, lTolerance;
	MBool bUpdate;
	MPOINT center;

	lTolerance = cirParam.lRadius>>3;	// R/16	R/8
	center.x = cirParam.xcoord;
	center.y = cirParam.ycoord;

	ptOnLine = ptTmp;
	AllocVectMem(hMemMgr, ptListTmp, pSeedList->lSeedNum, MPOINT);
	for (i=0; i<pSeedList->lSeedNum; i++)
	{
		randPoint[0] = pSeedList->pptSeed[i];
		for (j=i+1; j<pSeedList->lSeedNum; j++)
		{
			randPoint[1] = pSeedList->pptSeed[j];
			if(0>vFitLine(randPoint, 2, &dCoeffK, &dCoeffB, &bVert))
				continue;

			bUpdate = MTrue;
			lPointNum = 0;
			while(bUpdate)
			{
				bUpdate = MFalse;
				lPtOnLineNum = 0;
				for (k=0; k<pSeedList->lSeedNum; k++)
				{
					lDistance = vPointDistToLine2(pSeedList->pptSeed[k], dCoeffK, dCoeffB, bVert);
					if (lDistance < lTolerance && lPtOnLineNum<lNum)
						ptOnLine[lPtOnLineNum++] = pSeedList->pptSeed[k];
				}
				if(0>vFitLine(ptOnLine, lPtOnLineNum, &dCoeffK1, &dCoeffB1, &bVert1))
					continue;
				if (0==lPointNum || dCoeffK!=dCoeffK1 || dCoeffB!=dCoeffB1 || bVert!=bVert1)
				{
					if (lPtOnLineNum > lPointNum)
					{
						bUpdate = MTrue;
						dCoeffK = dCoeffK1, dCoeffB = dCoeffB1, bVert = bVert1;
						lPointNum = lPtOnLineNum;
						JMemCpy(ptListTmp, ptOnLine, lPointNum*sizeof(MPOINT));
					}
					else
					{
						if (lPtOnLineNum > MIN_PT_NUM_ON_LINE)
						{
							lDistance = vPointDistToLine2(center, dCoeffK1, dCoeffB1, bVert1);
							if (lDistance<lTolerance && pLineSetInfo->lLineNum < pLineSetInfo->lMaxLineNum 
								&& lPtOnLineNum<pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lMaxPtNum)
							{
								pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.Coefk = dCoeffK;
								pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.Coefb = dCoeffB;
								pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.bVertical = bVert;
								pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lPtNum = lPtOnLineNum;
								for (k=0; k<lPtOnLineNum; k++)
								{
									pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].pPtinfo[k].ptPoint = ptOnLine[k];
								}
								pLineSetInfo->lLineNum++;
							}
						}
						bUpdate = MFalse;
						break;
					}
				}
			}	// end while
			if (lPointNum > MIN_PT_NUM_ON_LINE)
			{
				lDistance = vPointDistToLine2(center, dCoeffK1, dCoeffB1, bVert1);
				if (lDistance<lTolerance && pLineSetInfo->lLineNum < pLineSetInfo->lMaxLineNum 
					&& lPtOnLineNum<pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lMaxPtNum)
				{
					pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.Coefk = dCoeffK;
					pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.Coefb = dCoeffB;
					pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lineParam.bVertical = bVert;
					pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].lPtNum = lPointNum;
					for (k=0; k<lPointNum; k++)
					{
						pLineSetInfo->pLineInfo[pLineSetInfo->lLineNum].pPtinfo[k].ptPoint = ptListTmp[k];
					}
					pLineSetInfo->lLineNum++;
				}
			}
		}
	}		
	if (0==pLineSetInfo->lLineNum)
	{
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}
	JPrintf("###############lineNum=%d\n", pLineSetInfo->lLineNum);
	MergeSet((MVoid*)(pLineSetInfo->pLineInfo), sizeof(LINE_INFO), &(pLineSetInfo->lLineNum), LineInfoCmp, LineInfoReplace);
	JPrintf("###############lineNum=%d\n", pLineSetInfo->lLineNum);
EXT:
	FreeVectMem(hMemMgr, ptListTmp);
	return res;
}

MRESULT CalcLineConfidence(MHandle hMemMgr, BLOCK *pResponse, CIRCLE cirParam, LINE_INFO *pLineInfo)
{
	MRESULT res = LI_ERR_NONE;

	MLong lTotalSumNum,lSumNum1, lSumNum2, lSumNum3;
	MLong lPTMeanSum1,lPTMeanSum2,lPTMeanSum3;
	MLong lMeanSum;
	MLong i;

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

	if(3 > pLineInfo->lPtNum)
	{
			res = LI_ERR_UNKNOWN;
			goto EXT;
	}

	lTotalSumNum = 0;
	for (i=0; i<pLineInfo->lPtNum-2; i+=2)
	{
		lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pResponse->pBlockData, pResponse->lBlockLine, pResponse->lWidth, pResponse->lHeight,
												pLineInfo->pPtinfo[i].ptPoint, pLineInfo->pPtinfo[i+1].ptPoint, 0xFF, &lSumNum1);
		lPTMeanSum1 *= lSumNum1;
		lPTMeanSum2 = vGetPointMeanValueBtwPt((MByte*)pResponse->pBlockData, pResponse->lBlockLine, pResponse->lWidth, pResponse->lHeight,
												pLineInfo->pPtinfo[i+1].ptPoint, pLineInfo->pPtinfo[i+2].ptPoint, 0xFF, &lSumNum2);
		lPTMeanSum2 *= lSumNum2;
		lPTMeanSum3 = 0;
		if (vComputeIntersactionAngle(pLineInfo->pPtinfo[i].ptPoint,pLineInfo->pPtinfo[i+1].ptPoint,pLineInfo->pPtinfo[i+2].ptPoint)>_8_9_PI)	//160 degree
		{
			lPTMeanSum3 = vGetPointMeanValueBtwPt((MByte*)pResponse->pBlockData, pResponse->lBlockLine, pResponse->lWidth, pResponse->lHeight,
													pLineInfo->pPtinfo[i].ptPoint, pLineInfo->pPtinfo[i+2].ptPoint, 0xFF, &lSumNum3);
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
	for (; i<pLineInfo->lPtNum-1;i++)
	{
		lPTMeanSum1 = vGetPointMeanValueBtwPt((MByte*)pResponse->pBlockData, pResponse->lBlockLine, pResponse->lWidth, pResponse->lHeight,
												pLineInfo->pPtinfo[i].ptPoint, pLineInfo->pPtinfo[i+1].ptPoint, 0xFF, &lSumNum1);
		lMeanSum+=lPTMeanSum1*lSumNum1;
		lTotalSumNum+=lSumNum1;
	}

	lMeanSum /= lTotalSumNum;
	pLineInfo->lConfidence = (MLong)(lMeanSum *(vDistance3L(pLineInfo->pPtinfo[0].ptPoint, pLineInfo->pPtinfo[i].ptPoint))/(cirParam.lRadius<<1));
	//JPrintf("lConfidence=%d\n", pLineInfo->lConfidence);
EXT:
	return res;
}