/*!
* \file Lithres.cpp
* \brief  the function related to filter the image 
* \author bqj@whayer
* \version vision 1.0 
* \date 10 Sep 2015
*/

#include "limath.h"
#include "lidebug.h"
#include "liblock.h"
#include "liconv.h"
#include "lithres.h"
#include "limem.h"
#include "liimage.h"
#include "liintegral.h"

#include <math.h>

MRESULT AdaptiveThres(MHandle hMemMgr,PBLOCK pBlockSrc,PBLOCK pBlockDst,
				MLong lBlocksizeW, MLong lBlocksizeH, int maxValue,double delta, int type)
{
	MRESULT res = LI_ERR_NONE;
	MLong i,j;
	JIntegral *pIntegral = MNull;
	MLong half_blocksizeW = lBlocksizeW/2;
	MLong half_blocksizeH = lBlocksizeH/2;

	MLong lIntegralLine = JMemLength(pBlockSrc->lWidth+1);
	if (!pBlockSrc || !pBlockDst || !pBlockSrc->pBlockData || !pBlockDst->pBlockData)
	{res = -1;goto EXT;}
	if ((pBlockSrc->lWidth < half_blocksizeW) || (pBlockSrc->lHeight<half_blocksizeH))
	{res = -1; goto EXT;}
	if (!(lBlocksizeW%2) || !(lBlocksizeH%2) )
	{res = -1; goto EXT;}

	AllocVectMem(hMemMgr, pIntegral, lIntegralLine*(pBlockSrc->lHeight+1), JIntegral);

	Integral(pBlockSrc->pBlockData, pBlockSrc->lBlockLine, DATA_U8, pIntegral, MNull, lIntegralLine, 
		pBlockSrc->lWidth, pBlockSrc->lHeight);

	//mean map
	for (j=0; j<pBlockDst->lHeight; j++)
	{
		MLong lStartY, lEndY;
		if (j<half_blocksizeH)
		{
			lStartY = 0;lEndY = j+half_blocksizeH;
		}else
		if (j>pBlockDst->lHeight-half_blocksizeH-1)
		{
			lStartY = j-half_blocksizeH; lEndY = pBlockSrc->lHeight-1;
		}
		else
		{
			lStartY = j - half_blocksizeH; lEndY = j + half_blocksizeH;
		}
		for (i=0; i<pBlockDst->lWidth; i++)
		{
			MLong lStartX, lEndX, lSum;
			if (i<half_blocksizeW)
			{
				lStartX = 0; lEndX = i+half_blocksizeW;
			}else
			if (i>pBlockDst->lWidth-half_blocksizeW-1)
			{
				lStartX = i-half_blocksizeW; lEndX = pBlockSrc->lWidth-1;
			}
			else
			{
				lStartX = i-half_blocksizeW; lEndX = i+half_blocksizeW;
			}

			lSum = *(pIntegral+lStartY*lIntegralLine+lStartX) + *(pIntegral+lEndY*lIntegralLine+lEndX)
				- *(pIntegral+lStartY*lIntegralLine+lEndX) - *(pIntegral+lEndY*lIntegralLine+lStartX);
			lSum /= ((lEndY-lStartY)*(lEndX-lStartX));
			
			*((MByte*)pBlockDst->pBlockData + pBlockDst->lBlockLine * j + i) = TRIM_UINT8(lSum);
		}
	}

	//PrintBmp(pBlockDst->pBlockData, pBlockDst->lBlockLine, DATA_U8, pBlockDst->lWidth, pBlockDst->lHeight, 1);
	//4
	//goto EXT;
    MByte imaxval = TRIM_UINT8(maxValue);
    int idelta = type == LI_THRESH_BINARY ? (int)(delta+0.99999999) : (int)(delta);//这里就偷懒了，对浮点数的ceil和floor
    MByte tab[768];

    if( type == LI_THRESH_BINARY )
        for( i = 0; i < 768; i++ )
            tab[i] = (MByte)(i - 255 > -idelta ? imaxval : 0);
    else if( type == LI_THRESH_BINARY_INV )
        for( i = 0; i < 768; i++ )
            tab[i] = (MByte)(i - 255 <= -idelta ? imaxval : 0);
	else
		{res = -1; goto EXT;}

	for( i = 0; i < pBlockDst->lHeight; i++ )
    {
		const MByte* sdata = (MByte*)pBlockSrc->pBlockData + pBlockSrc->lBlockLine * i;
		MByte* mdata = (MByte*)pBlockDst->pBlockData + pBlockDst->lBlockLine * i;
 
		for( j = 0; j < pBlockDst->lWidth; j++ )
		{
			mdata[j] = tab[sdata[j] - mdata[j] + 255];
		}
    }
EXT:
	FreeVectMem(hMemMgr, pIntegral);
	return res;
}

#define GRAY_LEVEL	256

MRESULT OtsuThres(PBLOCK pBlockSrc,PBLOCK pBlockDst,int type)
{
	MRESULT res = LI_ERR_NONE;

	MLong hist[GRAY_LEVEL] = {0};
	MDouble prob[GRAY_LEVEL] = {0.0}, omega[GRAY_LEVEL]={0.0};
	MDouble myu[GRAY_LEVEL] = {0.0};

	MDouble max_sigma, sigma[GRAY_LEVEL]={0.0};
	MLong i, x, y;

	MLong threshold;
	if (pBlockSrc->lWidth!=pBlockDst->lWidth || pBlockSrc->lHeight != pBlockDst->lHeight)
		return -1;

	//histogram generation
	MByte *pSrcData = (MByte*)pBlockSrc->pBlockData;
	MLong lExt = pBlockSrc->lBlockLine -  pBlockSrc->lWidth;
	for (y=0; y<pBlockSrc->lHeight; y++, pSrcData+=lExt)
		for (x=0; x<pBlockSrc->lWidth; x++, pSrcData++)
			hist[*pSrcData]++;

	//calculation of probability density
	for (i=0; i<GRAY_LEVEL; i++)
		prob[i] = (MDouble)hist[i] / (pBlockSrc->lWidth * pBlockSrc->lHeight);

	//omega & myu generation
	omega[0] = prob[0];
	myu[0] = 0.0;
	for (i=1; i<GRAY_LEVEL; i++)
	{
		omega[i] = omega[i-1] + prob[i];
		myu[i] = myu[i-1] + i*prob[i];
	}

	//sigma maximization
	//sigma stands for inter-class variance and determines optimal threshold value
	threshold = 0;
	max_sigma = 0.0;
	for (i=0; i<GRAY_LEVEL-1; i++)
	{
		if (omega[i]!=0.0 && omega[i]!=1.0)
		{
			 sigma[i] = pow(myu[GRAY_LEVEL-1]*omega[i] - myu[i], 2) / (omega[i]*(1.0 - omega[i]));
		}
		else
			sigma[i] = 0.0;

		if (sigma[i] > max_sigma)
		{
			max_sigma = sigma[i];
			threshold = i;
		}
	}

	ImgThresh(pBlockSrc->pBlockData, pBlockSrc->lBlockLine, pBlockDst->pBlockData, pBlockDst->lBlockLine,
		pBlockSrc->lWidth, pBlockSrc->lHeight, threshold, (JENUM_THRESH_MODEL)type);

	return res;
}



