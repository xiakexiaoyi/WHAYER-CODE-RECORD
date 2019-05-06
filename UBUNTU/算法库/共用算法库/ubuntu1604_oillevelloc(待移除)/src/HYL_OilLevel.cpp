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
int HY_whiteBlock(OL_IMAGES *src, int *oil)
{
	int res = 0;
	int oillevel = 2147483647;
        MLong *plHist = MNull;
        MLong *plHist1D = MNull;
	MByte *pTmp = MNull;
	MLong *pPeakMinValues = MNull;
        MLong *pPeakMaxValues = MNull;
        int maxvalue=0;
	int maxpoint=0;
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
