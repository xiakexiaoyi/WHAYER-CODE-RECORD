#include "SaliencyDetect.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include <math.h>
#include <stdio.h>

//*********************************************
static MVoid funcForRGB2XYZ(MDouble sVal, MDouble *val);
static MVoid funcForXYZ2Lab(MDouble val, MDouble *fVal);
//*********************************************
//******************************** superpixel segmentation ********************************
MRESULT GetSuperpixelSegmentation(MHandle hMemMgr, MByte *pImageData, MLong lWidth, MLong lHeight, MLong lImageLine,
													MLong *pLabelMap, MLong *label_num, SuperPixelRegion *pSuperPixels, MLong spcount, MLong lCompactness)
{
	MRESULT res = LI_ERR_NONE;
	MLong lSuperPixelSize;
	MDouble dCompactness;
	MLong lStep;
	MLong lImageSize;
	MDouble *pVect_l = MNull, *pVect_a = MNull, *pVect_b = MNull;
	SuperPixelSeed *pSeeds = MNull;
	MLong lSeedsNum;
	//MLong xstrips, ystrips;
	MLong K;
	MLong *pTmpLabel = MNull;
	MLong lIndex;
	
	if (MNull==pImageData || MNull==pLabelMap || 0>=spcount || 0>=lCompactness)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	dCompactness = (MDouble)lCompactness;
	lSuperPixelSize = (MLong)(0.5 + (MDouble)(lWidth * lHeight) / (MDouble)spcount);
	lStep = (MLong)(sqrt((MDouble)lSuperPixelSize) + 0.5);
	lImageSize = lWidth * lHeight;

	AllocVectMem(hMemMgr, pVect_l, lImageSize, MDouble);
	AllocVectMem(hMemMgr, pVect_a, lImageSize, MDouble);
	AllocVectMem(hMemMgr, pVect_b, lImageSize, MDouble);
	// rgb to Lab
	RGBtoLAB(pImageData, lWidth, lHeight, lImageLine, pVect_l, pVect_a, pVect_b);

	if (0==lStep)
		lStep = 1;

	lSeedsNum = spcount;
	AllocVectMem(hMemMgr, pSeeds, lSeedsNum, SuperPixelSeed);
	InitSuperPixelSeeds(pVect_l, pVect_a, pVect_b, pSeeds, lWidth, lHeight, lStep, &lSeedsNum);

	SearchSuperPixel(hMemMgr, pSeeds, pVect_l, pVect_a, pVect_b, pLabelMap, lWidth, lHeight, lStep, lSeedsNum, dCompactness);

	AllocVectMem(hMemMgr,pTmpLabel, lImageSize, MLong);
	SetVectMem(pTmpLabel, lImageSize, -1, MLong);
	K = lStep * lStep;
	EnforceLabelConnectivity(hMemMgr, pLabelMap, pTmpLabel, lWidth, lHeight, &lSeedsNum, K);
	for (lIndex=0; lIndex<lImageSize; lIndex++)
	{
		pLabelMap[lIndex] = pTmpLabel[lIndex];
	}
	*label_num = lSeedsNum;

	for (lIndex=0; lIndex<lSeedsNum; lIndex++)
	{
		pSuperPixels[lIndex].mean_L = 0;
		pSuperPixels[lIndex].mean_a = 0;
		pSuperPixels[lIndex].mean_b = 0;
		pSuperPixels[lIndex].cx = 0;
		pSuperPixels[lIndex].cy = 0;
		pSuperPixels[lIndex].lRegionNum = 0;
	}
	StoreSuperPixel(hMemMgr, pVect_l, pVect_a, pVect_b, pLabelMap, pSuperPixels, lWidth, lHeight, lSeedsNum);
	/*{
		FILE *fp = MNull;
		fp = fopen("D:\\sp.dat", "ab+");
		for (lIndex=0; lIndex<lSeedsNum; lIndex++)
		{
			fprintf(fp, "(%3d, %3d), (%.3f, %.3f, %.3f)\n", pSuperPixels[lIndex].cx, pSuperPixels[lIndex].cy,
				pSuperPixels[lIndex].mean_L, pSuperPixels[lIndex].mean_a, pSuperPixels[lIndex].mean_b);
		}
		fclose(fp);
	}*/

EXT:
	FreeVectMem(hMemMgr, pVect_l);
	FreeVectMem(hMemMgr, pVect_a);
	FreeVectMem(hMemMgr, pVect_b);
	FreeVectMem(hMemMgr, pSeeds);
	FreeVectMem(hMemMgr, pTmpLabel);
	return res;
}

MRESULT RGBtoLAB(MByte *pImageData, MLong lWidth, MLong lHeight, MLong lImageLine, 
								MDouble *pVectL, MDouble *pVectA, MDouble *pVectB)
{
	MRESULT res = LI_ERR_NONE;
	MByte *pBGR_R, *pBGR_G, *pBGR_B;
	MDouble *pLab_L, *pLab_A, *pLab_B;
	MLong i, j;
	MDouble X, Y, Z;
	MDouble Xr, Yr, Zr;
	MDouble xr, yr, zr;
	MDouble fx, fy, fz;

	if (MNull==pImageData || MNull==pVectL || MNull==pVectA || MNull==pVectB)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	Xr = 0.950456;	//reference white
	Yr = 1.0;		//reference white
	Zr = 1.088754;	//reference white

	for (j=0; j<lHeight; j++)
	{
		pBGR_B = pImageData + j * lImageLine;
		pBGR_G = pBGR_B + 1;
		pBGR_R = pBGR_B + 2;
		pLab_L = pVectL + j * lWidth;
		pLab_A = pVectA + j * lWidth;
		pLab_B = pVectB + j * lWidth;
		for (i=0; i<lWidth; i++)
		{
			RGBtoXYZ(*pBGR_R, *pBGR_G, *pBGR_B, &X, &Y, &Z);
			xr = X / Xr;
			yr = Y / Yr;
			zr = Z / Zr;
			funcForXYZ2Lab(xr, &fx);
			funcForXYZ2Lab(yr, &fy);
			funcForXYZ2Lab(zr, &fz);
			*pLab_L = 116.0 * fy - 16.0;
			*pLab_A = 500.0 * (fx - fy);
			*pLab_B = 200.0 * (fy - fz);

			pBGR_B += 3;
			pBGR_G += 3;
			pBGR_R += 3;
			pLab_L++;
			pLab_A++;
			pLab_B++;
		}
	}

EXT:
	return res;
}

MVoid funcForRGB2XYZ(MDouble sVal, MDouble *val)
{
	MDouble scaleThreshold;

	scaleThreshold = 0.0405;
	
	if (sVal < scaleThreshold)
	{
		*val = sVal / 12.92;
	}
	else
	{
		*val = pow((sVal + 0.055) / 1.055, 2.4);
	}
}

MVoid funcForXYZ2Lab(MDouble val, MDouble *fVal)
{
	MDouble epsilon, kappa;
	
	epsilon = 0.008856;		//actual CIE standard
	kappa = 903.3;				//actual CIE standard

	if (val > epsilon)
	{
		*fVal = pow(val, 1.0/3.0);
	}
	else
	{
		*fVal = (kappa * val + 16.0) / 116.0;
	}
}
MRESULT RGBtoXYZ(MByte R, MByte G, MByte B, MDouble *X, MDouble *Y, MDouble *Z)
{
	MRESULT res = LI_ERR_NONE;
	MDouble sR, sG, sB;
	MDouble r, g, b;
	MDouble scaleThreshold;

	scaleThreshold = 0.04045;

	sR = R / 255.0;
	sG = G / 255.0;
	sB = B / 255.0;

	funcForRGB2XYZ(sR, &r);
	funcForRGB2XYZ(sG, &g);
	funcForRGB2XYZ(sB, &b);

	*X = r*0.4124564 + g*0.3575761 + b*0.1804375;
	*Y = r*0.2126729 + g*0.7151522 + b*0.0721750;
	*Z = r*0.0193339 + g*0.1191920 + b*0.9503041;

	return res;
}

MRESULT InitSuperPixelSeeds(MDouble *pVectL, MDouble *pVectA, MDouble *pVectB, SuperPixelSeed *pSeeds, 
												MLong lWidth, MLong lHeight, MLong lStep, MLong *lNum)
{
	MRESULT res = LI_ERR_NONE;
	MLong lSeedsNum;
	MLong xstrips, ystrips, xerr, yerr;
	MDouble xerrPerStrip, yerrPerStrip;
	MLong xoff, yoff, xe, ye;
	MLong xx, yy, seedx, seedy;
	MLong lIndex;
	MLong lCount;

	if (0==lStep)
		lStep = 1;
	xstrips = (MLong)(0.5 + (MDouble)lWidth / (MDouble)lStep);
	if (0==xstrips)
		xstrips = 1;
	xerr = lWidth - xstrips * lStep;
	if (xerr < 0)
	{
		xstrips--;
		xerr = lWidth - xstrips * lStep;
	}
	xerrPerStrip = (MDouble)xerr / (MDouble)xstrips;
	ystrips = (MLong)(0.5 + (MDouble)lHeight / (MDouble)lStep);
	if(0==ystrips)
		ystrips = 1;
	yerr = lHeight - ystrips * lStep;
	if (yerr < 0)
	{
		ystrips--;
		yerr = lHeight - ystrips * lStep;
	}
	yerrPerStrip = (MDouble)yerr / (MDouble)ystrips;

	lSeedsNum = xstrips * ystrips;
	lSeedsNum = MIN(lSeedsNum, *lNum);
	xoff = lStep>>1;
	yoff = lStep>>1;

	lCount = 0;
	for (yy=0; yy<ystrips; yy++)
	{
		ye = (MLong)(yy * yerrPerStrip);
		for (xx=0; xx<xstrips; xx++)
		{
			xe = (MLong)(xx * xerrPerStrip);
			seedx = xx * lStep + xoff + xe;
			seedx = MIN(seedx, lWidth-1);
			seedy = yy * lStep + yoff + ye;
			seedy = MIN(seedy, lHeight-1);

			lIndex = seedy * lWidth + seedx;

			(pSeeds + lCount)->val_L = *(pVectL + lIndex);
			(pSeeds + lCount)->val_a = *(pVectA + lIndex);
			(pSeeds + lCount)->val_b = *(pVectB + lIndex);
			(pSeeds + lCount)->x = seedx;
			(pSeeds + lCount)->y = seedy;
			lCount++;
			if(lCount>=lSeedsNum)
				break;
		}
	}

	*lNum = lSeedsNum;

	return res;
}

MRESULT SearchSuperPixel(MHandle hMemMgr, SuperPixelSeed *pSeeds, MDouble *pVectL, MDouble *pVectA, MDouble *pVectB, 
									MLong *pLabelMap, MLong lWidth, MLong lHeight, MLong lStep, MLong lSeedsNum, MDouble dCompactness)
{
	MRESULT res = LI_ERR_NONE;

	MLong iter;
	MLong lIndex, label;
	MLong lImageSize;
	MDouble invwt, dTmpCoeff;
	MLong lLeft, lRight, lTop, lBottom;
	MDouble dist, dist_lab, dist_xy, maxDistance;
	MLong xx, yy, nn;
	SuperPixelSeed *pTmpSeeds = MNull;
	MDouble *pClusterSize = MNull, *pInv = MNull, *pDistVec = MNull;

	lImageSize = lWidth * lHeight;

	AllocVectMem(hMemMgr, pTmpSeeds, lSeedsNum, SuperPixelSeed);
	AllocVectMem(hMemMgr, pClusterSize, lSeedsNum, MDouble);
	AllocVectMem(hMemMgr, pInv, lSeedsNum, MDouble);
	AllocVectMem(hMemMgr, pDistVec, lImageSize, MDouble);

	maxDistance = sqrt((MDouble)(lWidth*lWidth + lHeight*lHeight));
	dTmpCoeff = SQARE(lStep / dCompactness);
	invwt = 1.0 / dTmpCoeff;
	for (iter=0; iter<10; iter++)		// 基本上迭代10次左右能收敛到一个比较好的结果
	{
		for (lIndex=0; lIndex<lImageSize; lIndex++)
		{
			pDistVec[lIndex] = maxDistance;
		}
		for (nn=0; nn<lSeedsNum; nn++)
		{
			lLeft = MAX(0, (pSeeds+nn)->x - lStep);
			lRight = MIN(lWidth-1, (pSeeds+nn)->x + lStep);
			lTop = MAX(0, (pSeeds+nn)->y - lStep);
			lBottom = MIN(lHeight-1, (pSeeds+nn)->y + lStep);

			for (yy=lTop; yy<=lBottom; yy++)
			{
				for (xx=lLeft; xx<=lRight; xx++)
				{
					lIndex = yy * lWidth + xx;
					dist_lab = SQARE(*(pVectL+lIndex) - (pSeeds+nn)->val_L) +
									SQARE(*(pVectA+lIndex) - (pSeeds+nn)->val_a) +
									SQARE(*(pVectB+lIndex) - (pSeeds+nn)->val_b);
					dist_xy = SQARE(xx - (pSeeds+nn)->x) + SQARE(yy - (pSeeds+nn)->y);
					dist = dist_lab + dist_xy * invwt;
					if (dist<pDistVec[lIndex])
					{
						pDistVec[lIndex] = dist;
						pLabelMap[lIndex] = nn;
					}
				}
			}
		}

		SetVectZero(pTmpSeeds, lSeedsNum);
		SetVectZero(pClusterSize, lSeedsNum);
		lIndex = 0;
		for (yy=0; yy<lHeight; yy++)
		{
			for (xx=0; xx<lWidth; xx++)
			{
				label = pLabelMap[lIndex];
				if(label < 0)
					continue;
				pTmpSeeds[label].val_L += pVectL[lIndex];
				pTmpSeeds[label].val_a += pVectA[lIndex];
				pTmpSeeds[label].val_b += pVectB[lIndex];
				pTmpSeeds[label].x += xx;
				pTmpSeeds[label].y += yy;

				pClusterSize[label] += 1;

				lIndex++;
			}
		}

		for (lIndex=0; lIndex<lSeedsNum; lIndex++)
		{
			if(pClusterSize[lIndex] <= 0)
				pClusterSize[lIndex] = 1;
			
			pSeeds[lIndex].val_L = pTmpSeeds[lIndex].val_L / pClusterSize[lIndex];
			pSeeds[lIndex].val_a = pTmpSeeds[lIndex].val_a / pClusterSize[lIndex];
			pSeeds[lIndex].val_b = pTmpSeeds[lIndex].val_b / pClusterSize[lIndex];
			pSeeds[lIndex].x = (MLong)(pTmpSeeds[lIndex].x / pClusterSize[lIndex]);
			pSeeds[lIndex].y = (MLong)(pTmpSeeds[lIndex].y / pClusterSize[lIndex]);
		}
	}

EXT:
	FreeVectMem(hMemMgr, pTmpSeeds);
	FreeVectMem(hMemMgr, pClusterSize);
	FreeVectMem(hMemMgr, pInv);
	FreeVectMem(hMemMgr, pDistVec);
	return res;
}

MRESULT EnforceLabelConnectivity(MHandle hMemMgr, MLong *pLabel, MLong *pTmpLabel, MLong lWidth,
														MLong lHeight, MLong *pLabelNum, MLong K)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, c;
	MLong lImageSize, lRegionNumThres;
	MLong lIndex, lOriginIndex, lNeighbour;
	MLong lCurX, lCurY;
	MLong lCount, lTmpLabel;
	MLong lAdjcent;
	MLong *pxVec = MNull, *pyVec = MNull;
	MLong dx4[4] = {-1, 0, 1, 0};
	MLong dy4[4] = {0, -1, 0, 1};

	lImageSize = lWidth * lHeight;
	AllocVectMem(hMemMgr, pxVec, lImageSize, MLong);
	AllocVectMem(hMemMgr, pyVec, lImageSize, MLong);
	lRegionNumThres = K>>2;
	lTmpLabel = 0;
	lAdjcent = 0;
	lOriginIndex = 0;

	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			if (pTmpLabel[lOriginIndex] < 0)
			{
				pTmpLabel[lOriginIndex] = lTmpLabel;
				pxVec[0] = i;
				pyVec[0] = j;

				for (lNeighbour=0; lNeighbour<4; lNeighbour++)
				{
					lCurX = pxVec[0] + dx4[lNeighbour];
					lCurY = pyVec[0] + dy4[lNeighbour];
					if ((lCurX>=0 && lCurX<lWidth) && (lCurY>=0 && lCurY<lHeight))
					{
						lIndex = lCurY * lWidth + lCurX;
						if (pTmpLabel[lIndex] >= 0)
						{
							lAdjcent = pTmpLabel[lIndex];
						}
					}
				}

				lCount = 1;
				for (c=0; c<lCount; c++)
				{
					for (lNeighbour=0; lNeighbour<4; lNeighbour++)
					{
						lCurX = pxVec[c] + dx4[lNeighbour];
						lCurY = pyVec[c] + dy4[lNeighbour];
						if ((lCurX>=0 && lCurX<lWidth) && (lCurY>=0 && lCurY<lHeight))
						{
							lIndex = lCurY * lWidth + lCurX;
							if (pTmpLabel[lIndex] < 0 && pLabel[lIndex] == pLabel[lOriginIndex])
							{
								pxVec[lCount] = lCurX;
								pyVec[lCount] = lCurY;
								pTmpLabel[lIndex] = lTmpLabel;
								lCount++;
							}
						}
					}
				}

				if (lCount <= lRegionNumThres)
				{
					for (c=0; c<lCount; c++)
					{
						lIndex = pyVec[c] * lWidth + pxVec[c];
						pTmpLabel[lIndex] = lAdjcent;
					}
					lTmpLabel--;
				}

				lTmpLabel++;
			}

			lOriginIndex++;
		}
	}

	*pLabelNum = lTmpLabel;

EXT:
	FreeVectMem(hMemMgr, pxVec);
	FreeVectMem(hMemMgr, pyVec);
	return res;
}

MRESULT DrawSuperPixelContours(MHandle hMemMgr, BLOCK *pResultBlock, MLong *pLabel)
{
	MRESULT res = LI_ERR_NONE;
	MLong lWidth, lHeight, lStride, lImageSize;
	MLong lIndex, lSearchIndex, lContoursIndex;
	MLong i, j, k, x, y;
	MLong lCount;
	MLong *pContours_x = MNull, *pContours_y = MNull;
	MByte *pResultData = MNull;
	MLong dx8[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
	MLong dy8[8] = {0, -1, -1, -1, 0, 1, 1, 1};

	lWidth = pResultBlock->lWidth;
	lHeight = pResultBlock->lHeight;
	lStride = pResultBlock->lBlockLine;
	pResultData = (MByte*)pResultBlock->pBlockData;
	lImageSize = lWidth * lHeight;
	AllocVectMem(hMemMgr, pContours_x, lImageSize, MLong);
	AllocVectMem(hMemMgr, pContours_y, lImageSize, MLong);

	lSearchIndex = 0;
	lContoursIndex = 0;
	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			lCount = 0;
			for (k=0; k<8; k++)
			{
				x = i + dx8[k];
				y = j + dy8[k];
				if ((x>=0 && x<lWidth)&&(y>=0 && y<lHeight))
				{
					lIndex = y * lWidth + x;
					if (pLabel[lIndex] != pLabel[lSearchIndex])	//contours
					{
						lCount++;
					}
				}
			}

			if (lCount>1)
			{
				if (lContoursIndex >= lImageSize)
				{
					res = LI_ERR_UNKNOWN;
					goto EXT;
				}
				pContours_x[lContoursIndex] = i;
				pContours_y[lContoursIndex] = j;
				lContoursIndex++;
			}

			lSearchIndex++;
		}
	}

	for (i=0; i<lContoursIndex; i++)
	{
		lIndex = pContours_y[i] * lStride + pContours_x[i];
		*(pResultData + lIndex) = 255;
	}

EXT:
	FreeVectMem(hMemMgr, pContours_x);
	FreeVectMem(hMemMgr, pContours_y);
	return res;
}

MRESULT StoreSuperPixel(MHandle hMemMgr, MDouble *pVectL, MDouble *pVectA, MDouble *pVectB, MLong *pLabelMap, 
										SuperPixelRegion *pSuperPixels, MLong lWidth, MLong lHeight, MLong lLabelNum)
{
	MRESULT res = LI_ERR_NONE;
	MLong i, j;
	MLong lIndex;
	MLong lLabel;

	if (MNull == pSuperPixels)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			lIndex = j * lWidth + i;
			lLabel = pLabelMap[lIndex];
			pSuperPixels[lLabel].mean_L += pVectL[lIndex];
			pSuperPixels[lLabel].mean_a += pVectA[lIndex];
			pSuperPixels[lLabel].mean_b += pVectB[lIndex];
			pSuperPixels[lLabel].cx += i;
			pSuperPixels[lLabel].cy += j;
			pSuperPixels[lLabel].lRegionNum++;
		}
	}

	for (i=0; i<lLabelNum; i++)
	{
		if (0!=pSuperPixels[i].lRegionNum)
		{
			pSuperPixels[i].mean_L /= pSuperPixels[i].lRegionNum;
			pSuperPixels[i].mean_a /= pSuperPixels[i].lRegionNum;
			pSuperPixels[i].mean_b /= pSuperPixels[i].lRegionNum;
			pSuperPixels[i].cx /= pSuperPixels[i].lRegionNum;
			pSuperPixels[i].cy /= pSuperPixels[i].lRegionNum;
		}
		else
		{
			pSuperPixels[i].mean_L = 0;
			pSuperPixels[i].mean_a = 0;
			pSuperPixels[i].mean_b = 0;
			pSuperPixels[i].cx = 0;
			pSuperPixels[i].cy = 0;
		}
	}

EXT:
	return res;
}
//************************************ salinecy detection ************************************
MRESULT getSaliencyImage(MHandle hMemMgr, SuperPixelRegion *pSuperPixels, 
										MLong *pLabelMap, MLong lLabelNum, BLOCK *pDstBlock)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lIndex;
	MPOINT centerPt;
	MLong lWidth, lHeight, lBlockLine;
	MByte *pDstData;
	MDouble *pSaliencyRatio = MNull;
	MDouble *pSaliencyResult = MNull;
	MLong lOffSet;
	MLong lLabel;

	lWidth = pDstBlock->lWidth;
	lHeight = pDstBlock->lHeight;
	lBlockLine = pDstBlock->lBlockLine;
	pDstData = (MByte*)(pDstBlock->pBlockData);
	centerPt.x = lWidth>>1;
	centerPt.y = lHeight>>1;

	AllocVectMem(hMemMgr, pSaliencyRatio, lLabelNum, MDouble);
	AllocVectMem(hMemMgr, pSaliencyResult, lLabelNum, MDouble);
	for (lIndex=0; lIndex<lLabelNum; lIndex++)
	{
		pSaliencyRatio[lIndex] = 0;
		calcSaliencyRatio(pSuperPixels, lLabelNum, lIndex, centerPt, &pSaliencyRatio[lIndex]);
		pSaliencyResult[lIndex] = 1 - exp(-pSaliencyRatio[lIndex]);
		 pSuperPixels[lIndex].lVal = (MByte)(255 * pSaliencyResult[lIndex]);
	}

	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			lIndex = j * lWidth + i;
			lOffSet = j * lBlockLine + i;
			lLabel = pLabelMap[lIndex];
			*(pDstData + lOffSet) = pSuperPixels[lLabel].lVal;
		}
	}

EXT:
	FreeVectMem(hMemMgr, pSaliencyRatio);
	FreeVectMem(hMemMgr, pSaliencyResult);
	return res;
}

MRESULT calcSaliencyRatio(SuperPixelRegion *pSuperPixels, MLong lSuperPixelNum, 
						  MLong lIndex, MPOINT centerPt, MDouble *pResult)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	MDouble labDistance;
	MLong lDistance, lDistanceToCenter;
	MDouble tmpSumVal;
	MDouble tmpSqare;

	tmpSumVal = 0;
	tmpSqare = SQARE(centerPt.x - pSuperPixels[lIndex].cx) + SQARE(centerPt.y - pSuperPixels[lIndex].cy);
	lDistanceToCenter = (MLong)sqrt(tmpSqare);
	for (i=0; i<lSuperPixelNum; i++)
	{
		tmpSqare = SQARE(pSuperPixels[i].cx - pSuperPixels[lIndex].cx) + SQARE(pSuperPixels[i].cy - pSuperPixels[lIndex].cy);
		lDistance =  (MLong)sqrt(tmpSqare);
		if (0>=lDistance)
			lDistance = 0;
		tmpSqare = SQARE(pSuperPixels[i].mean_L - pSuperPixels[lIndex].mean_L)
								+ SQARE(pSuperPixels[i].mean_a - pSuperPixels[lIndex].mean_a)
								+ SQARE(pSuperPixels[i].mean_b - pSuperPixels[lIndex].mean_b);
		labDistance =  sqrt(tmpSqare);
		tmpSumVal += (labDistance / (1 + lDistance));
	}
	*pResult = tmpSumVal / (1 + lDistanceToCenter);

//EXT:
	return res;
}

/***基于无向图排序的显著性检测***/
MRESULT optimizeSaliencyImage(MHandle hMemMgr, SuperPixelRegion *pSuperPixels, MLong *pLabelMap,
														MLong lLabelNum, BLOCK *pDstBlock)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight;
	MLong lSize;
	MByte lOtsuThreshold;
	MDouble alfa, beta;
	MBool *pAdjacentMap = MNull;
	MDouble *pWeightMap = MNull, *pDMatrix = MNull, *pAMatrix = MNull, *pBMatrix = MNull;
	MDouble *pStartVect = MNull, *pFinalVect = MNull;

	lWidth = pDstBlock->lWidth;
	lHeight = pDstBlock->lHeight;
	lSize = lLabelNum * lLabelNum;

	AllocVectMem(hMemMgr, pAdjacentMap, lSize, MBool);
	SetVectMem(pAdjacentMap, lSize, MFalse, MBool);

	AllocVectMem(hMemMgr, pWeightMap, lSize, MDouble);
	AllocVectMem(hMemMgr, pDMatrix, lSize, MDouble);
	AllocVectMem(hMemMgr, pAMatrix, lSize, MDouble);
	AllocVectMem(hMemMgr, pBMatrix, lSize, MDouble);

	AllocVectMem(hMemMgr, pStartVect, lLabelNum, MDouble);
	AllocVectMem(hMemMgr, pFinalVect, lLabelNum, MDouble);

	getAdjacentMap(pLabelMap, lWidth, lHeight, pAdjacentMap, lLabelNum);
	calcWeightMap(pSuperPixels, lLabelNum, pWeightMap, pDMatrix, pAdjacentMap, lLabelNum, lLabelNum);
	calcThresholdOtsu(hMemMgr, pDstBlock, &lOtsuThreshold);

	// init
	alfa = 0.2;
	beta = 1 - alfa;
	initOptimizeVect(pWeightMap, pDMatrix, pAMatrix, pStartVect, pFinalVect, pSuperPixels, alfa, lLabelNum, lOtsuThreshold);
	GO(invMatrix(hMemMgr, pAMatrix, pBMatrix, lLabelNum));
	getOptimizeVect(pBMatrix, pStartVect, pFinalVect, beta, lLabelNum);
	refreshSaliencyImage(pSuperPixels, pFinalVect, pDstBlock, pLabelMap, lLabelNum);

EXT:
	FreeVectMem(hMemMgr, pAdjacentMap);
	FreeVectMem(hMemMgr, pWeightMap);
	FreeVectMem(hMemMgr, pDMatrix);
	FreeVectMem(hMemMgr, pAMatrix);
	FreeVectMem(hMemMgr, pBMatrix);
	FreeVectMem(hMemMgr, pStartVect);
	FreeVectMem(hMemMgr, pFinalVect);
	return res;
}

MRESULT getAdjacentMap(MLong *pLabelMap, MLong lLabelWidth, MLong lLabelHeight, 
											MBool *pAdjacentMap, MLong lAdjacentStride)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, k, x, y;
	MLong lIndex, lSearchIndex, lAdjacentIndex;
	MLong dx8[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
	MLong dy8[8] = {0, -1, -1, -1, 0, 1, 1, 1};

	if (MNull==pLabelMap || MNull==pAdjacentMap)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	for (j=0; j<lLabelHeight; j++)
	{
		for (i=0; i<lLabelWidth; i++)
		{
			lIndex = j * lLabelWidth + i;
			for (k=0; k<8; k++)
			{
				x = i + dx8[k];
				y = j + dy8[k];
				if(x<0 || x>=lLabelWidth || y<0 || y>=lLabelHeight)
					continue;
				lSearchIndex = y * lLabelWidth + x;
				if (pLabelMap[lIndex] != pLabelMap[lSearchIndex])
				{
					lAdjacentIndex = pLabelMap[lIndex] * lAdjacentStride + pLabelMap[lSearchIndex];		// ij
					pAdjacentMap[lAdjacentIndex] = MTrue;
					lAdjacentIndex = pLabelMap[lSearchIndex] * lAdjacentStride + pLabelMap[lIndex];		// ji
					pAdjacentMap[lAdjacentIndex] = MTrue;
				}
				else
				{
					lAdjacentIndex = pLabelMap[lIndex] * lAdjacentStride + pLabelMap[lSearchIndex];
					pAdjacentMap[lAdjacentIndex] = MFalse;
					lAdjacentIndex = pLabelMap[lSearchIndex] * lAdjacentStride + pLabelMap[lIndex];
					pAdjacentMap[lAdjacentIndex] = MFalse;
				}
			}
		}
	}

EXT:
	return res;
}

MRESULT calcWeightMap(SuperPixelRegion *pSuperPixels, MLong lSuperPixelNum, MDouble *pWeightMap, 
										MDouble *pDMatrix, MBool *pAdjacentMap, MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lIndex, lSearchIndex;
	MDouble dTmpVal, dColorDistance;
	MDouble sqareSigma, dCoeff;

	if (MNull==pSuperPixels || MNull==pWeightMap || MNull==pDMatrix || MNull==pAdjacentMap)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	sqareSigma = 10;
	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			lIndex = j * lWidth + i;
			if (MTrue == pAdjacentMap[lIndex])
			{
				dTmpVal = SQARE(pSuperPixels[i].mean_L - pSuperPixels[j].mean_L)
								+ SQARE(pSuperPixels[i].mean_a - pSuperPixels[j].mean_a)
								+ SQARE(pSuperPixels[i].mean_b - pSuperPixels[j].mean_b);
				dColorDistance = sqrt(dTmpVal);
				dCoeff = dColorDistance / sqareSigma;
				pWeightMap[lIndex] = exp(-dCoeff);
			}
			else
			{
				pWeightMap[lIndex] = 0;
			}
			pDMatrix[lIndex] = 0;
		}
	}

	/*{
		FILE *fp = MNull;
		fp = fopen("D:\\weight.dat", "ab+");
		if (MNull != fp)
		{
			for (j=0; j<lHeight; j++)
			{
				fprintf(fp, "(%3d) ", j);
				for (i=0; i<lWidth; i++)
				{
					lIndex = j * lWidth + i;
					fprintf(fp, "%.3f ", pWeightMap[lIndex]);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}
	}*/

	for (i=0; i<lHeight; i++)
	{
		lIndex = i * lWidth + i;
		for (j=0; j<lWidth; j++)
		{
			lSearchIndex = i * lWidth + j;
			pDMatrix[lIndex] += pWeightMap[lSearchIndex];
		}
	}

	/*{
		FILE *fp = MNull;
		fp = fopen("D:\\dMatrix.dat", "ab+");
		if (MNull != fp)
		{
			for (j=0; j<lHeight; j++)
			{
				fprintf(fp, "(%3d) ", j);
				for (i=0; i<lWidth; i++)
				{
					lIndex = j * lWidth + i;
					fprintf(fp, "%.3f ", pDMatrix[lIndex]);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}
	}*/

	/*{
		FILE *fp = MNull;
		fp = fopen("D:\\AAMatrix.dat", "ab+");
		if (MNull != fp)
		{
			for (j=0; j<lHeight; j++)
			{
				fprintf(fp, "(%3d) ", j);
				for (i=0; i<lWidth; i++)
				{
					lIndex = j * lWidth + i;
					fprintf(fp, "%.3f ", pDMatrix[lIndex] - 0.2 * pWeightMap[lIndex]);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}
	}*/

EXT:
	return res;
}

MRESULT calcThresholdOtsu(MHandle hMemMgr, BLOCK *pSrcImage, MByte *lThreshold)
{
	MRESULT res = LI_ERR_NONE;

	MLong lWidth, lHeight, lStride, lExt;
	MByte *pData, *pSrcImageData;
	MLong i, j;
	MLong lImageSize;
	MDouble w1, w2, u, u1, u2, u1Tmp, u2Tmp, delta, dMaxDelta;
	MDouble epsilon;
	MLong lPixelCount[256] = {0};
	MDouble dPixelPro[256] = {0};
	BLOCK tmpImage = {0};

	lWidth = pSrcImage->lWidth;
	lHeight = pSrcImage->lHeight;
	lStride = pSrcImage->lBlockLine;
	lExt = lStride - lWidth;
	pSrcImageData = (MByte*)pSrcImage->pBlockData;
	lImageSize = lWidth * lHeight;
	epsilon = 0.00001;

	B_Create(hMemMgr, &tmpImage, DATA_U8, lWidth, lHeight);

	if (0 == lImageSize)
	{
		res = LI_ERR_UNKNOWN;
		goto EXT;
	}

	for (i=0; i<256; i++)
	{
		lPixelCount[i] = 0;
		dPixelPro[i] = 0;
	}

	for (j=0; j<lHeight; j++)
	{
		pData = pSrcImageData + j * lStride;
		for (i=0; i<lWidth; i++)
		{
			lPixelCount[*pData++]++;
		}
	}

	for (i=0; i<256; i++)
	{
		dPixelPro[i] = ((MDouble)lPixelCount[i]) / ((MDouble)lImageSize);
	}

	dMaxDelta = 0;
	for (i=0; i<256; i++)
	{
		w1 = w2 = 0;
		u1Tmp = u2Tmp = 0;
		u = u1 = u2 = 0;
		delta = 0;
		for (j=0; j<256; j++)
		{
			if (j<=i)			// background
			{
				w1 += dPixelPro[j];
				u1Tmp += j * dPixelPro[j];
			}
			else				// foreground
			{
				w2 += dPixelPro[j];
				u2Tmp += j * dPixelPro[j];
			}

			u1 = u1Tmp / (w1 + epsilon);
			u2 = u2Tmp / (w2 + epsilon);
			u = u1 + u2;
			delta = w1 * (u1 - u) * (u1 - u) + w2 * (u2 - u) * (u2 - u);
			if (delta > dMaxDelta)
			{
				dMaxDelta = delta;
				*lThreshold = (MByte)j;
			}
		}
	}

	pData = (MByte*)tmpImage.pBlockData;
	pSrcImageData = (MByte*)pSrcImage->pBlockData;
	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			if (*pSrcImageData < *lThreshold)
			{
				*pData = 255;
			}
			else
			{
				*pData = 0;
			}
			pSrcImageData++;
			pData++;
		}
		pSrcImageData += lExt;
		pData += lExt;
	}
	PrintBmpEx(tmpImage.pBlockData, tmpImage.lBlockLine, DATA_U8, tmpImage.lWidth, tmpImage.lHeight, 1, ".\\log\\OTSU.bmp");

EXT:
	B_Release(hMemMgr, &tmpImage);
	return res;
}

MRESULT invMatrix(MHandle hMemMgr, MDouble *pSrcMatrix, MDouble *pDstMatrix, MLong n)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j, k;
	MDouble tmp, maxVal;
	MLong lIndex, lSwapIndex;
	MDouble epsilon;
	MDouble *pTmpMatrix = MNull;

	if (MNull==pSrcMatrix || MNull==pDstMatrix)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	epsilon = 0.0000001;
	
	AllocVectMem(hMemMgr, pTmpMatrix, n*n, MDouble);
	lIndex = 0;
	for (j=0; j<n; j++)		// copy srcMatrix data
	{
		for (i=0; i<n; i++)
		{
			pTmpMatrix[lIndex] = pSrcMatrix[lIndex];
			if (i==j)
			{
				*(pDstMatrix + lIndex) = 1;
			}
			else
			{
				*(pDstMatrix + lIndex) = 0;
			}
			lIndex++;
		}
	}

	for (i=0; i<n; i++)
	{
		maxVal = pTmpMatrix[i*n + i];
		k = i;
		j = i + 1;
		lIndex = j * n + i;
		for (; j<n; j++)
		{
			if (fabs(pTmpMatrix[lIndex]) > fabs(maxVal))
			{
				maxVal = pTmpMatrix[lIndex];
				k = j;
			}
			lIndex++;
		}

		if (k != i)
		{
			for (j=0; j<n; j++)
			{
				lIndex = i*n + j;
				lSwapIndex = k*n + j;
				tmp = pTmpMatrix[lIndex];
				pTmpMatrix[lIndex] = pTmpMatrix[lSwapIndex];
				pTmpMatrix[lSwapIndex] = tmp;

				tmp = pDstMatrix[lIndex];
				pDstMatrix[lIndex] = pDstMatrix[lSwapIndex];
				pDstMatrix[lSwapIndex] = tmp;
			}
		}

		lIndex = i*n + i;
		if (fabs(pTmpMatrix[lIndex]) < epsilon)	// pTmpMatrix[lIndex]==0
		{
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}

		tmp = pTmpMatrix[lIndex];
		for (j=0; j<n; j++)
		{
			lIndex = i * n + j;
			pTmpMatrix[lIndex] = pTmpMatrix[lIndex] / tmp;
			pDstMatrix[lIndex] = pDstMatrix[lIndex] / tmp;
		}

		for (j=0; j<n; j++)
		{
			if (j != i)
			{
				tmp = pTmpMatrix[j*n + i];
				for (k=0; k<n; k++)
				{
					lIndex = j * n + k;
					lSwapIndex = i * n + k;
					pTmpMatrix[lIndex] = pTmpMatrix[lIndex] - pTmpMatrix[lSwapIndex] * tmp;
					pDstMatrix[lIndex] = pDstMatrix[lIndex] - pDstMatrix[lSwapIndex] * tmp;
				}
			}
		}
	}

EXT:
	FreeVectMem(hMemMgr, pTmpMatrix);
	return res;
}

MRESULT initOptimizeVect(MDouble *pWeight, MDouble *pDMatrix, MDouble *pAMatrix, 
											MDouble *pStartVect, MDouble *pFinalVect, SuperPixelRegion *pSuperPixels, 
											MDouble alfa, MLong n, MByte lThreshold)
{
	MRESULT res = LI_ERR_NONE;

	MLong lIndex;
	MLong i, j;
	MDouble *pStartData = MNull, *pFinalData = MNull;
	SuperPixelRegion *pSuperPixelData = MNull;

	if (MNull==pWeight ||MNull==pDMatrix 
		|| MNull==pAMatrix || MNull==pStartVect || MNull==pFinalVect)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	pStartData = pStartVect;
	pFinalData = pFinalVect;
	pSuperPixelData = pSuperPixels;
	for (lIndex=0; lIndex<n; lIndex++)
	{
		if (pSuperPixelData->lVal >= lThreshold)
		{
			*pStartData = 1;
		}
		else
		{
			*pStartData = 0;
		}
		*pFinalData = 0;

		pStartData++;
		pFinalData++;
		pSuperPixelData++;
	}

	lIndex = 0;
	for (j=0; j<n; j++)
	{
		for (i=0; i<n; i++)
		{
			*(pAMatrix + lIndex) = *(pDMatrix + lIndex) - *(pWeight + lIndex) * alfa;
			lIndex++;
		}
	}

EXT:
	return res;
}

MRESULT getOptimizeVect(MDouble *pRefMatrix, MDouble *pStartVect, MDouble *pFinalVect, MDouble beta, MLong n)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MDouble *pRefMatrixData = MNull;
	MDouble *pStartVectData = MNull, *pFinalVectData = MNull;

	if (MNull==pRefMatrix || MNull==pStartVect || MNull==pFinalVect)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	pRefMatrixData = pRefMatrix;
	pFinalVectData = pFinalVect;
	for (j=0; j<n; j++)
	{
		pStartVectData = pStartVect;
		for (i=0; i<n; i++)
		{
			*pFinalVectData += beta * (*pRefMatrixData) * (*pStartVectData);
			pRefMatrixData++;
			pStartVectData++;
		}
		pFinalVectData++;
	}

EXT:
	return res;
}

MRESULT refreshSaliencyImage(SuperPixelRegion *pSuperPixels, MDouble *pCoeffVect, 
													BLOCK *pDstBlock, MLong *pLabelMap, MLong n)
{
	MRESULT res = LI_ERR_NONE;

	MLong i, j;
	MLong lIndex, lOffSet;
	MLong lWidth, lHeight, lBlockLine;
	MLong lLabel;
	MDouble ratio;
	MByte *pDstData = MNull;

	lWidth = pDstBlock->lWidth;
	lHeight = pDstBlock->lHeight;
	lBlockLine = pDstBlock->lBlockLine;
	pDstData = (MByte *)pDstBlock->pBlockData;

	for (lIndex=0; lIndex<n; lIndex++)
	{
		ratio = 1 - exp(-pCoeffVect[lIndex]);
		pSuperPixels[lIndex].lVal = (MByte)(255 * ratio);
	}

	for (j=0; j<lHeight; j++)
	{
		for (i=0; i<lWidth; i++)
		{
			lIndex = j * lWidth + i;
			lOffSet = j * lBlockLine + i;
			lLabel = pLabelMap[lIndex];
			*(pDstData + lOffSet) = pSuperPixels[lLabel].lVal;
		}
	}

//EXT:
	return res;
}