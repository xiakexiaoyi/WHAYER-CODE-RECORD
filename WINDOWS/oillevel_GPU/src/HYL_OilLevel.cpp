#include "HYL_OilLevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "licomdef.h"
#include "liimage.h"
#include "hough.h"
#include "limath.h"
#include "limem.h"
#include "HaarResponse.h"

#include "lidebug.h"
void ImageIntegral1(OL_IMAGES *src, double* Integral)
{
	unsigned char* Data;
	unsigned long pixel;
	Data = (unsigned char*)src->pixelArray.chunky.pPixel;
	for (int j = 0; j < src->lHeight; j++)
	{
		for (int i = 0; i < src->lWidth; i++)
		{
			pixel = Data[j*src->pixelArray.chunky.lLineBytes + i];
			if (i == 0 && j == 0)
				*(Integral + i + j*src->lWidth) = pixel;
			else if (i != 0 && j == 0)
				*(Integral + i + j*src->lWidth) = *(Integral + i - 1 + j*src->lWidth) + pixel;
			else if (i == 0 && j != 0)
				*(Integral + i + j*src->lWidth) = *(Integral + i + (j - 1)*src->lWidth) + pixel;
			else
				*(Integral + i + j*src->lWidth) = *(Integral + i - 1 + j*src->lWidth) + *(Integral + i + (j - 1)*src->lWidth) - *(Integral + i - 1 + (j - 1)*src->lWidth) + pixel;
		}
	}
}
int fitImageHaarResponse(double* Integral, JOFFSCREEN* dst)
{
	int res = 0;
	unsigned char *Data;
	int w, h;
	unsigned long white, blackup, blackdown, max = 0;
	unsigned long* dstData;
	Data = (unsigned char*)dst->pixelArray.chunky.pPixel;
	dstData = (unsigned long*)malloc((dst->dwWidth*dst->dwHeight) * sizeof(unsigned long));
	if (dstData == NULL)
	{
		res = -1;
		return res;
	}
	memset(dstData, 0, (dst->dwWidth*dst->dwHeight) * sizeof(unsigned long));

	for (int j = 0; j < dst->dwHeight; j++)
	{
		for (int i = 0; i < dst->dwWidth; i++)
		{
			Data[j*dst->pixelArray.chunky.dwImgLine + i] = 0;
		}
	}
	//w = 20;
	//h = 4;
	w = dst->dwWidth*0.5;
	h = dst->dwHeight*0.1;
	for (int j = h + h / 2; j < dst->dwHeight - h - h / 2; j++)//中间黑两边白
	{
		for (int i = w / 2; i < dst->dwWidth - w / 2; i++)
		{
			white = *(Integral + (j + h / 2)*dst->dwWidth + (i + w / 2)) + *(Integral + (j - h / 2)*dst->dwWidth + (i - w / 2)) - *(Integral + (j - h / 2)*dst->dwWidth + (i + w / 2)) - *(Integral + (j + h / 2)*dst->dwWidth + (i - w / 2));
			blackup = *(Integral + (j - h / 2)*dst->dwWidth + (i + w / 2)) + *(Integral + (j - h / 2 - h)*dst->dwWidth + (i - w / 2)) - *(Integral + (j - h / 2 - h)*dst->dwWidth + (i + w / 2)) - *(Integral + (j - h / 2)*dst->dwWidth + (i - w / 2));
			blackdown = *(Integral + (j + h / 2 + h)*dst->dwWidth + (i + w / 2)) + *(Integral + (j + h / 2)*dst->dwWidth + (i - w / 2)) - *(Integral + (j + h / 2)*dst->dwWidth + (i + w / 2)) - *(Integral + (j + h / 2 + h)*dst->dwWidth + (i - w / 2));
			if (white < blackup && white < blackdown)
				*(dstData + j*dst->dwWidth + i) = 1.0*(blackdown + blackup) / 2 - white;
			else
				*(dstData + j*dst->dwWidth + i) = 0;
			if (*(dstData + j*dst->dwWidth + i) > max)
				max = *(dstData + j*dst->dwWidth + i);
		}
	}
	for (int j = 0; j < dst->dwHeight; j++)
	{
		for (int i = 0; i < dst->dwWidth; i++)
		{
			Data[j*dst->pixelArray.chunky.dwImgLine + i] = *(dstData + j*dst->dwWidth + i)*255.0 / max;
		}
	}
	if (dstData)
		free(dstData);
	return res;
}
void ImageHaarResponse1(double* Integral, JOFFSCREEN* dst)
{
	unsigned char *Data;
	int w, h;
	unsigned long white, blackup, blackdown, max = 0;
	unsigned long* dstData;
	Data = (unsigned char*)dst->pixelArray.chunky.pPixel;
	dstData = (unsigned long*)malloc((dst->dwWidth*dst->dwHeight) * sizeof(unsigned long));
	memset(dstData, 0, (dst->dwWidth*dst->dwHeight) * sizeof(unsigned long));
	
	for (int j = 0; j < dst->dwHeight; j++)
	{
		for (int i = 0; i < dst->dwWidth; i++)
		{
			Data[j*dst->pixelArray.chunky.dwImgLine + i] = 0;
		}
	}
	int buttom = 1;
if(buttom == 1)
{ 
	w = 20;
	h = 4;
	for (int j = h+h/2; j < dst->dwHeight - h-h/2; j++)//中间黑两边白
	{
		for (int i = w/2; i < dst->dwWidth - w/2; i++)
		{
		white = *(Integral + (j + h/2)*dst->dwWidth + (i + w/2)) + *(Integral + (j - h/2)*dst->dwWidth + (i - w/2)) - *(Integral + (j - h/2)*dst->dwWidth + (i + w/2)) - *(Integral + (j + h/2)*dst->dwWidth + (i - w/2));
		blackup = *(Integral + (j - h/2)*dst->dwWidth + (i + w/2)) + *(Integral + (j - h/2-h)*dst->dwWidth + (i - w/2)) - *(Integral + (j - h/2-h)*dst->dwWidth + (i + w/2)) - *(Integral + (j - h/2)*dst->dwWidth + (i - w/2));
		blackdown = *(Integral + (j + h/2+h)*dst->dwWidth + (i + w/2)) + *(Integral + (j + h/2)*dst->dwWidth + (i - w/2)) - *(Integral + (j + h/2)*dst->dwWidth + (i + w/2)) - *(Integral + (j + h/2+h)*dst->dwWidth + (i - w/2));
		if (white < blackup && white < blackdown)
			*(dstData + j*dst->dwWidth + i) = 1.0*(blackdown + blackup) / 2- white;
		else
			*(dstData + j*dst->dwWidth + i) = 0;
		if (*(dstData + j*dst->dwWidth + i) > max)
			max = *(dstData + j*dst->dwWidth + i);
		}
	}
}
if(buttom == 0)
{ 
	w = 10;
	h = 1;
	for (int j = 3 * h + 2; j < dst->dwHeight - (3 * h + 1); j++)
	{
		for (int i = w + 1; i < dst->dwWidth - w; i++)
		{
			white = *(Integral + (j + h)*dst->dwWidth + (i + w)) + *(Integral + (j - h - 1)*dst->dwWidth + (i - w - 1)) - *(Integral + (j - h - 1)*dst->dwWidth + (i + w)) - *(Integral + (j + h)*dst->dwWidth + (i - w - 1));
			blackup = *(Integral + (j - h - 1)*dst->dwWidth + (i + w)) + *(Integral + (j - 3 * h - 2)*dst->dwWidth + (i - w - 1)) - *(Integral + (j - 3 * h - 2)*dst->dwWidth + (i + w)) - *(Integral + (j - h - 1)*dst->dwWidth + (i - w - 1));
			blackdown = *(Integral + (j + 3 * h + 1)*dst->dwWidth + (i + w)) + *(Integral + (j + h)*dst->dwWidth + (i - w - 1)) - *(Integral + (j + h)*dst->dwWidth + (i + w)) - *(Integral + (j + 3 * h + 1)*dst->dwWidth + (i - w - 1));
			if (white > blackup && white > blackdown)
				*(dstData + j*dst->dwWidth + i) = white - 1.0*(blackdown + blackup) / 2;
			else
				*(dstData + j*dst->dwWidth + i) = 0;
			if (*(dstData + j*dst->dwWidth + i) > max)
				max = *(dstData + j*dst->dwWidth + i);
		}
	}
}
	for (int j = 0; j < dst->dwHeight; j++)
	{
		for (int i = 0; i < dst->dwWidth; i++)
		{
			Data[j*dst->pixelArray.chunky.dwImgLine + i] = *(dstData + j*dst->dwWidth + i)*255.0 / max;
		}
	}
}
MRESULT  SmoothHist(MHandle hMemMgr, MLong *plHist, MLong lHistLen)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	MLong *pCpyHist = MNull;

	AllocVectMem(hMemMgr, pCpyHist, lHistLen, MLong);
	JMemCpy(pCpyHist, plHist, lHistLen * sizeof(MLong));

	plHist[0] = (pCpyHist[0] + pCpyHist[1]) / 2;
	for (i = 1; i<lHistLen - 1; i++)
	{
		plHist[i] = (pCpyHist[i - 1] + pCpyHist[i] + pCpyHist[i + 1]) / 3;
	}
	plHist[lHistLen - 1] = (pCpyHist[lHistLen - 2] + pCpyHist[lHistLen - 1]) / 2;
EXT:
	FreeVectMem(hMemMgr, pCpyHist);
	return res;
}

MRESULT GetPeaksValue(MHandle hMemMgr, const MLong* pLVData, MLong lLVLen,
	MLong *pMinIdx, MLong *plMinNum,
	MLong *pMaxIdx, MLong *plMaxNum, MLong lMinPeakDist)
{
	MRESULT res = LI_ERR_NONE;
	MLong *pPeakMinData = MNull;
	MLong *pPeakMaxData = MNull;
	MLong lNearestIndex = -lMinPeakDist;

	///分配中间变量，用于存储最大最小值列表
	///1--表示极值，0--表示非极值
	AllocVectMem(hMemMgr, pPeakMinData, lLVLen * 2, MLong);
	SetVectZero(pPeakMinData, lLVLen * 2 * sizeof(MLong));
	pPeakMaxData = pPeakMinData + lLVLen;

	for (int i = 1; i <lLVLen - 1; i++)
	{
		///包含了其中一个是相等的，如果三个全相等，就不算是极值
		if ((pLVData[i]>pLVData[i - 1] && pLVData[i] >= pLVData[i + 1]) || (pLVData[i] >= pLVData[i - 1] && pLVData[i]>pLVData[i + 1]))
			pPeakMaxData[i] = 1;
		if ((pLVData[i]<pLVData[i - 1] && pLVData[i] <= pLVData[i + 1]) || (pLVData[i] <= pLVData[i - 1] && pLVData[i]<pLVData[i + 1]))
			pPeakMinData[i] = 1;
	}

	///filter the peak list according to the lMinPeakDist
	for (int i = 0; i<lLVLen; i++)
	{
		if (pPeakMaxData[i])///peak value
		{
			if (i - lNearestIndex<lMinPeakDist)
			{
				///replace
				if (pLVData[i] > pLVData[lNearestIndex])
				{
					///delete the peak value
					pPeakMaxData[lNearestIndex] = 0;
					lNearestIndex = i;
				}
				else///delete the current value, keep the lNearestIndex unchanged
					pPeakMaxData[i] = 0;
			}
			else
				lNearestIndex = i;
		}
	}

	///filter the peak list according to the lMinPeakDist
	for (int i = 0; i<lLVLen; i++)
	{
		if (pPeakMinData[i])///peak value
		{
			if (i - lNearestIndex<lMinPeakDist)
			{
				///replace
				if (pLVData[i] < pLVData[lNearestIndex])
				{
					///delete the peak value
					pPeakMinData[lNearestIndex] = 0;
					lNearestIndex = i;
				}
				else///delete the current value, keep the lNearestIndex unchanged
					pPeakMinData[i] = 0;
			}
			else
				lNearestIndex = i;
		}
	}

	if (pMinIdx && plMinNum)
	{
		*plMinNum = 0;
		for (int i = 0; i<lLVLen; i++)
		{
			if (pPeakMinData[i])
			{
				pMinIdx[*plMinNum] = i;
				(*plMinNum)++;
			}
		}
	}
	if (pMaxIdx&& plMaxNum)
	{
		*plMaxNum = 0;
		for (int i = 0; i<lLVLen; i++)
		{
			if (pPeakMaxData[i])
			{
				pMaxIdx[*plMaxNum] = i;
				(*plMaxNum)++;
			}
		}
	}

EXT:
	FreeVectMem(hMemMgr, pPeakMinData);
	return res;
}
int HY_Oilfit(OL_IMAGES *src)
{
	int res = 0;
	MLong lThreshold, lSeedSize,lStackLen;
	MPOINT *pTemp = MNull;
	JOFFSCREEN ImageH = { 0 };
	BLOCK blockFlag = { 0 };
	JGSEED seedsTemp = { 0 };
	MLong lHaarWidth = 2;
	int ImageSize = src->lHeight*src->lWidth;
	double* integralImage;
	integralImage = (double*)malloc(ImageSize * sizeof(double));
	GO(ImgCreate(NULL, &ImageH, FORMAT_GRAY, src->lWidth, src->lHeight));
	GO(B_Create(NULL, &blockFlag, DATA_U8, src->lWidth, src->lHeight));
	SetVectZero(blockFlag.pBlockData, blockFlag.lBlockLine*blockFlag.lHeight);
	lSeedSize = src->lWidth* src->lHeight;
	seedsTemp.lMaxNum = lSeedSize;
	seedsTemp.lSeedNum = 0;
	AllocVectMem(NULL, seedsTemp.pptSeed, lSeedSize, MPOINT);
	AllocVectMem(NULL, seedsTemp.pcrSeed, lSeedSize, MByte);
	ImageIntegral1(src, integralImage);//积分图
	ImageHaarResponse1(integralImage, &ImageH);//haar响应
	PrintBmpEx(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, DATA_U8, ImageH.dwWidth, ImageH.dwHeight, 1, "..\\ImageH.bmp");
	lStackLen = ImageH.dwWidth * ImageH.dwHeight / 10;
	AllocVectMem(NULL, pTemp, lStackLen, MPOINT);
	StatisticNonZeroValue(NULL, (MByte*)ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, ImageH.dwWidth,
		ImageH.dwHeight, 0, pTemp, sizeof(MPOINT)*lStackLen, 30, &lThreshold);
	LocalMax_Circle(NULL, (MByte*)ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, (MByte *)blockFlag.pBlockData, blockFlag.lBlockLine, \
		blockFlag.lWidth, blockFlag.lHeight, pTemp, lStackLen, lThreshold, (MLong)(lThreshold*0.3), 50, 128, &seedsTemp, lHaarWidth >> 1);//离散
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1, "..\\localMaxPre_bbb.bmp");
	ridgePoint(NULL, (MByte*)ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, ImageH.dwWidth, ImageH.dwHeight,
		(MByte*)blockFlag.pBlockData, blockFlag.lBlockLine, lHaarWidth, (MLong)(lThreshold*0.6), 0, 250, &seedsTemp);
	PrintBmpEx(blockFlag.pBlockData, blockFlag.lBlockLine, DATA_U8, blockFlag.lWidth, blockFlag.lHeight, 1, "..\\localMaxPost.bmp");
EXT:
	return res;
}
void Lswap(LineSegment* pLines,int i,int j)
{
	MPOINT tmpStartPt= pLines->StartPt[i];
	MPOINT tmpEndPt= pLines->EndPt[i];
	pLines->StartPt[i] = pLines->StartPt[j];
	pLines->EndPt[i] = pLines->EndPt[j];
	pLines->StartPt[j] = tmpStartPt;
	pLines->EndPt[j] = tmpEndPt;

}
void BubbleSort(LineSegment* pLines)
{
	for (int j = 0; j < pLines->lLineNum; j++)
	{
		for (int i = 0; i < pLines->lLineNum - 1 - j; i++)
		{
			int tmpminA = MIN(pLines->StartPt[i].y, pLines->EndPt[i].y);
			int tmpminB = MIN(pLines->StartPt[i+1].y, pLines->EndPt[i+1].y);
			if (tmpminA > tmpminB)
				Lswap(pLines, i, i + 1);
		}
	}
}
int HY_Oil(OL_IMAGES *src,int *oil)
{
	int res = 0;
	MLong *plHist = MNull;
	MLong *plHist1D = MNull;
	MByte *pTmp = MNull;
	MLong *pPeakMinValues = MNull;
	MLong *pPeakMaxValues = MNull;
	MLong lPeakMinIdxNum, lPeakMaxIdxNum;
	MLong MaxValue = 0;
	JOFFSCREEN ImageH = { 0 };
	JOFFSCREEN ImageBinary = { 0 };
	int tmp[100];
	LineSegment* pLines;
	AllocVectMem(NULL, pLines, 1, LineSegment);
	AllocVectMem(NULL, pLines->EndPt, 100, MPOINT);
	AllocVectMem(NULL, pLines->StartPt, 100, MPOINT);
	int ImageSize = src->lHeight*src->lWidth;
	double* integralImage;
	integralImage = (double*)malloc(ImageSize * sizeof(double));
	GO(ImgCreate(NULL, &ImageH, FORMAT_GRAY, src->lWidth, src->lHeight));
	GO(ImgCreate(NULL, &ImageBinary, FORMAT_GRAY, src->lWidth, src->lHeight));
	ImageIntegral1(src, integralImage);//积分图
	ImageHaarResponse1(integralImage, &ImageH);//haar响应
	PrintBmpEx(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, DATA_U8, ImageH.dwWidth, ImageH.dwHeight, 1, "..\\ImageH1.bmp");
	//二值化，阈值26=255*0.1
	ImgThresh(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine, ImageH.dwWidth, ImageH.dwHeight, 26, LI_THRESH_BINARY);
	PrintBmpEx(ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine, DATA_U8, ImageBinary.dwWidth, ImageBinary.dwHeight, 1, "..\\ImageBinary1.bmp");
	
	int LineLength,Threshold;
	if (ImageBinary.dwWidth < 60)
	{
		LineLength = 5;
		Threshold = 10;
	}
	else
	{
		LineLength = 15;
		Threshold = 20;
	}
		
	ProbabilisticHoughTransform_ls(NULL, (unsigned char*)ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine,
		ImageBinary.dwWidth, ImageBinary.dwHeight, 255, 1, V_PI / 180, 20, 15, 2, 2, 100, pLines);//找直线
	BubbleSort(pLines);//从高到低排序
	for (int i = 0; i < pLines->lLineNum; i++)
	{
		printf("%d:start(%d,%d) end(%d,%d)\n", i, pLines->StartPt[i].x, pLines->StartPt[i].y, pLines->EndPt[i].x, pLines->EndPt[i].y);
	}
	//
	AllocVectMem(NULL, plHist, src->lHeight * 2, MLong);
	SetVectZero(plHist, src->lHeight * 2 * sizeof(MLong));
	plHist1D = plHist + src->lHeight;
	AllocVectMem(NULL, pPeakMaxValues, src->lHeight * 2, MLong);
	SetVectZero(pPeakMaxValues, src->lHeight * 2 * sizeof(MLong));
	pPeakMinValues = pPeakMaxValues + src->lHeight;
	pTmp = (MByte*)ImageH.pixelArray.chunky.pPixel;
	for (int j = 0; j < src->lHeight; j++)//水平投影
	{
		for (int i = 0; i < src->lWidth; i++)
		{
			plHist[j] += pTmp[j*ImageH.pixelArray.chunky.dwImgLine+i];
		}
	}
	SmoothHist(NULL, plHist,src->lHeight);
	/*
	for(int i=1;i<src->lHeight-1;i++)
		plHist1D[i] = (plHist[i + 1] - plHist[i - 1]) / 2;
	SmoothHist(NULL, plHist1D+1, src->lHeight-2);
	*/
	int oillevel = INT_MAX;
	GetPeaksValue(NULL, plHist,src->lHeight, pPeakMinValues, &lPeakMinIdxNum, pPeakMaxValues, &lPeakMaxIdxNum, 5);
	for (int i = 0; i < lPeakMaxIdxNum; i++)//极大值
	{
		printf("plHist[%d]=%d\n", pPeakMaxValues[i], plHist[pPeakMaxValues[i]]);
		MaxValue = MAX(MaxValue, plHist[pPeakMaxValues[i]]);
	}
	//int skip = 0;

	//if (pLines->lLineNum > 3)//
	//{
	//	int tmpA = pLines->StartPt[3].y - pLines->StartPt[2].y;
	//	int tmpB = pLines->StartPt[2].y - pLines->StartPt[1].y;
	//	int tmpC = pLines->StartPt[1].y - pLines->StartPt[0].y;
	//	printf("%d %d %d\n",tmpA, tmpB, tmpC);
	//	if (1.0*ABS(tmpA - tmpB) / tmpA < 0.5 && 1.0*(tmpC - tmpB) / tmpB> 1.0)
	//		skip=1;

	//}

	for (int i = 0; i < lPeakMaxIdxNum; i++)//先给一个没有线的预测值
	{
		if (plHist[pPeakMaxValues[i]] > 0.3*MaxValue)
		{ 
			oillevel = pPeakMaxValues[i];
			break;
		}
	}
	/*******/
	int mindis=10;//投影位置与线的位置的max距离
	for(int i=0;i<pLines->lLineNum;i++)
	{
		tmp[i] = -1;
		/*if (skip == 1)
		{
			skip = 0;
			continue;
		}*/
		for (int j = 0; j < lPeakMaxIdxNum; j++)
		{
			if(plHist[pPeakMaxValues[j]]> 0.3*MaxValue)
			{
				int tmpdis = ABS(pLines->StartPt[i].y - pPeakMaxValues[j]) + ABS(pLines->EndPt[i].y - pPeakMaxValues[j]);
				if (mindis > tmpdis)
				{
					mindis = tmpdis;
					tmp[i] = j;
				}
			}
		}
		if(tmp[i] != -1)//线和投影匹配
		{
			oillevel = MIN(pPeakMaxValues[tmp[i]],MIN(pLines->EndPt[i].y, pLines->StartPt[i].y));
			//printf("oillevelfit = %d\n",MIN(pLines->EndPt[i].y,pLines->StartPt[i].y));
			break;
		}
	}
	printf("oillevelfit = %d\n", oillevel);
	/*******/
	oil[0] = oillevel;
EXT:
	FreeVectMem(NULL, pLines->EndPt);
	FreeVectMem(NULL, pLines->StartPt);
	FreeVectMem(NULL, pLines);
	FreeVectMem(NULL, plHist);
	FreeVectMem(NULL, pPeakMaxValues);
	ImgRelease(NULL, &ImageH);
	ImgRelease(NULL, &ImageBinary);
	free(integralImage);
	return res;
}
int HY_whiteBlock(OL_IMAGES *src, int *oil)
{
	int res = 0;
	int oillevel = INT_MAX;
	JOFFSCREEN ImageH = { 0 };
	JOFFSCREEN ImageBinary = { 0 };
	int ImageSize = src->lHeight*src->lWidth;
	double* integralImage;
	integralImage = (double*)malloc(ImageSize * sizeof(double));
	if (integralImage == NULL)
	{
		res = -1;
		goto EXT;
	}
	GO(ImgCreate(NULL, &ImageH, FORMAT_GRAY, src->lWidth, src->lHeight));
	GO(ImgCreate(NULL, &ImageBinary, FORMAT_GRAY, src->lWidth, src->lHeight));
	ImageIntegral1(src, integralImage);//积分图
	//ImageHaarResponse1(integralImage, &ImageH);//haar响应
	//fitImageHaarResponse(integralImage, &ImageH);//haar响应
	if (0 != fitImageHaarResponse(integralImage, &ImageH))
	{
		res = -1;
		goto EXT;
	}



	MLong *plHist = MNull;
	MLong *plHist1D = MNull;
	MByte *pTmp = MNull;
	MLong *pPeakMinValues = MNull;
	MLong *pPeakMaxValues = MNull;
	MLong lPeakMinIdxNum, lPeakMaxIdxNum;
	AllocVectMem(NULL, plHist, src->lHeight * 2, MLong);
	SetVectZero(plHist, src->lHeight * 2 * sizeof(MLong));
	plHist1D = plHist + src->lHeight;
	AllocVectMem(NULL, pPeakMaxValues, src->lHeight * 2, MLong);
	SetVectZero(pPeakMaxValues, src->lHeight * 2 * sizeof(MLong));
	pPeakMinValues = pPeakMaxValues + src->lHeight;
	pTmp = (MByte*)ImageH.pixelArray.chunky.pPixel;
	for (int j = 0; j < src->lHeight; j++)//水平投影
	{
		for (int i = 0; i < src->lWidth; i++)
		{
			plHist[j] += pTmp[j*ImageH.pixelArray.chunky.dwImgLine + i];
		}
	}
	GO(SmoothHist(NULL, plHist, src->lHeight));
	/*
	for(int i=1;i<src->lHeight-1;i++)
	plHist1D[i] = (plHist[i + 1] - plHist[i - 1]) / 2;
	SmoothHist(NULL, plHist1D+1, src->lHeight-2);
	*/
	GO(GetPeaksValue(NULL, plHist, src->lHeight, pPeakMinValues, &lPeakMinIdxNum, pPeakMaxValues, &lPeakMaxIdxNum, 5));
	//PrintBmpEx(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, DATA_U8, ImageH.dwWidth, ImageH.dwHeight, 1, "..\\ImageH.bmp");
	int maxvalue=0;
	int maxpoint=0;
	for (int i = 0; i < lPeakMaxIdxNum; i++)
	{
		if (plHist[pPeakMaxValues[i]] > maxvalue)
		{
			maxvalue = plHist[pPeakMaxValues[i]];
			maxpoint = pPeakMaxValues[i];
		}
	}
	oil[0] = maxpoint;
EXT:
	FreeVectMem(NULL, plHist);
	FreeVectMem(NULL, pPeakMaxValues);
	ImgRelease(NULL, &ImageH);
	ImgRelease(NULL, &ImageBinary);
	if (integralImage)
		free(integralImage);
	return res;
}
int HY_OilLevel(OL_IMAGES *src,int *oil,int max,int min)
{
	int res = 0;
	int oillevel = INT_MAX;
	JOFFSCREEN ImageH = { 0 };
	JOFFSCREEN ImageBinary = { 0 };
	LineSegment* pLines;
	AllocVectMem(NULL, pLines, 1, LineSegment);
	AllocVectMem(NULL, pLines->EndPt, 20, MPOINT);
	AllocVectMem(NULL, pLines->StartPt, 20, MPOINT);
	int ImageSize = src->lHeight*src->lWidth;
	double* integralImage;
	integralImage = (double*)malloc(ImageSize * sizeof(double));
	GO(ImgCreate(NULL, &ImageH, FORMAT_GRAY, src->lWidth, src->lHeight));
	GO(ImgCreate(NULL, &ImageBinary, FORMAT_GRAY, src->lWidth, src->lHeight));
	ImageIntegral1(src, integralImage);//积分图
	ImageHaarResponse1(integralImage, &ImageH);//haar响应
	MLong *plHist = MNull;
	MLong *plHist1D = MNull;
	MByte *pTmp = MNull;
	MLong *pPeakMinValues = MNull;
	MLong *pPeakMaxValues = MNull;
	MLong lPeakMinIdxNum, lPeakMaxIdxNum;
	AllocVectMem(NULL, plHist, src->lHeight * 2, MLong);
	SetVectZero(plHist, src->lHeight * 2 * sizeof(MLong));
	plHist1D = plHist + src->lHeight;
	AllocVectMem(NULL, pPeakMaxValues, src->lHeight * 2, MLong);
	SetVectZero(pPeakMaxValues, src->lHeight * 2 * sizeof(MLong));
	pPeakMinValues = pPeakMaxValues + src->lHeight;
	pTmp = (MByte*)ImageH.pixelArray.chunky.pPixel;
	for (int j = 0; j < src->lHeight; j++)//水平投影
	{
		for (int i = 0; i < src->lWidth; i++)
		{
			plHist[j] += pTmp[j*ImageH.pixelArray.chunky.dwImgLine + i];
		}
	}
	SmoothHist(NULL, plHist, src->lHeight);
	/*
	for(int i=1;i<src->lHeight-1;i++)
	plHist1D[i] = (plHist[i + 1] - plHist[i - 1]) / 2;
	SmoothHist(NULL, plHist1D+1, src->lHeight-2);
	*/
	GetPeaksValue(NULL, plHist, src->lHeight, pPeakMinValues, &lPeakMinIdxNum, pPeakMaxValues, &lPeakMaxIdxNum, 5);

	PrintBmpEx(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, DATA_U8, ImageH.dwWidth, ImageH.dwHeight, 1, "..\\ImageH.bmp");
	ImgThresh(ImageH.pixelArray.chunky.pPixel, ImageH.pixelArray.chunky.dwImgLine, ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine, ImageH.dwWidth, ImageH.dwHeight, 26, LI_THRESH_BINARY);
	PrintBmpEx(ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine, DATA_U8, ImageBinary.dwWidth, ImageBinary.dwHeight, 1, "..\\ImageBinary.bmp");
	ProbabilisticHoughTransform_ls(NULL, (unsigned char*)ImageBinary.pixelArray.chunky.pPixel, ImageBinary.pixelArray.chunky.dwImgLine,
		ImageBinary.dwWidth, ImageBinary.dwHeight,255,1,V_PI/180,20,15,2,4,10, pLines);
	if (pLines->lLineNum < 1)
	{
		oillevel = 0;
	}
	for (int i = 0; i < pLines->lLineNum; i++)
	{
		oillevel =MIN(pLines->StartPt[i].y,MIN(pLines->EndPt[i].y, oillevel));
		printf("%d:start(%d,%d) end(%d,%d)\n", i, pLines->StartPt[i].x, pLines->StartPt[i].y, pLines->EndPt[i].x, pLines->EndPt[i].y);
	}
	printf("oillevel = %d\n",oillevel);
	oil[0]= oillevel;
EXT:
	return res;
}