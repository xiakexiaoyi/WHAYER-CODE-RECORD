#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "litrimfun.h"
#include "limath.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"

#include "priorarea.h"
#include "areagraph.h"
#include "colorsegment.h"

//////////////////////////////////////////////////////////////////////////
JSTATIC MLong _MergeSeed(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal);
JSTATIC MLong _MergeSeed_Loose(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal);
JSTATIC MBool _MergeOther(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal, MLong lLabelSeed);
JSTATIC MRESULT _WeightedMask(MHandle hMemMgr, 
							const JOFFSCREEN *pImg, JMASK *pMskGray, 
							const JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed, 
							const JLabelData *pLabelData, MLong lLabelLine, 
							MCOLORREF *pcrSeeds, MLong *plSeedsNum);
JSTATIC MVoid _WeightedMask_Seed(const JOFFSCREEN *pImg, JMASK *pMskGray, 
						   const JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed, 
						   const JLabelData *pLabelData, MLong lLabelLine, 
						   MCOLORREF crSeed, MLong *plGrayNum);

MRESULT SkinMask(MHandle hMemMgr, const JOFFSCREEN *pImg, 
				 const MRECT *prtSkin, JMASK *pMskSkin, 
				 MCOLORREF *pcrSeeds, MLong *plSeedsNum)
{
	JLabelData *pLabelData = MNull;
	MLong lLabelLine;
	JINFO_AREA_GRAPH infoGraph = {0};
	MLong lLabelSeed = 0;
	
	MRESULT res = LI_ERR_NONE;
	MLong x, y, lLabelCur;

	//Initial color segment result
	lLabelLine = JMemLength(pImg->dwWidth);
	AllocVectMem(hMemMgr, pLabelData, pImg->dwHeight*lLabelLine, JLabelData);		
	GO(CreateAreaGraphByImg(hMemMgr, pImg, pLabelData, lLabelLine, 
		&infoGraph));	
	CheckAreaGraph(pImg, &infoGraph, pLabelData, lLabelLine);
	
	{
		//Get seed color
		MCOLORREF crSeed = SeedColorByPriorArea(pImg, prtSkin);
		MLong lSize0 = (prtSkin->right-prtSkin->left)*(prtSkin->bottom-prtSkin->top);
		//Get seed label
		MLong lCenterX = (prtSkin->left+prtSkin->right)/2;
		MLong lCenterY = (prtSkin->top+prtSkin->bottom)/2;
		MLong lR2X = SQARE((prtSkin->right-prtSkin->left)/2);
		MLong lR2Y = SQARE((prtSkin->bottom-prtSkin->top)/2);
		MLong lR2 = 0;
		if(lR2X<lR2Y)	lR2=lR2Y, lR2Y /= lR2X, lR2X = 1;
		else			lR2=lR2X, lR2X /= lR2Y, lR2Y = 1;
		lLabelSeed = -1;
		for(y=prtSkin->top; y<prtSkin->bottom; y++)
		{
			MLong lSqareY = SQARE(y-lCenterY) * lR2X;
			for(x=prtSkin->left; x<prtSkin->right; x++)
			{
				MLong lDist;
				MLong lLabelVal = pLabelData[lLabelLine*y+x]-1;
				JINFO_AREA *pAreaCur = infoGraph.pInfoArea + lLabelVal;
				if(lLabelVal == lLabelSeed)
					continue;
				if(pAreaCur->lFlag <= 0)
					continue;				
				if(SQARE(x-lCenterX)*lR2Y + lSqareY > lR2)
					continue;
				lDist = GetColorDist(pAreaCur->lY/pAreaCur->lSize, pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crSeed);
				if(lDist > MAX_COLOR_DIST*2)
				{
					pAreaCur->lFlag = 0;
					continue;
				}
				else if(pAreaCur->lSize<lSize0*3/4)
				{
					MLong x0 = (pAreaCur->rtArea.right+pAreaCur->rtArea.left)/2;
					MLong y0 = (pAreaCur->rtArea.bottom+pAreaCur->rtArea.top)/2;
					MLong lDist2 = SQARE(x0-lCenterX)*lR2Y+SQARE(y0-lCenterY)*lR2X;
					if(lDist2>lR2 && lDist*lDist2/lR2 > MAX_COLOR_DIST/2)
					{
						pAreaCur->lFlag = 0;
						continue;
					}
				}
				else
				{
					MLong left = MAX(pAreaCur->rtArea.left, prtSkin->left);
					MLong right = MIN(pAreaCur->rtArea.right, prtSkin->right);
					MLong top = MAX(pAreaCur->rtArea.top, prtSkin->top);
					MLong bottom = MIN(pAreaCur->rtArea.bottom, prtSkin->bottom);
					if(lLabelSeed!=-1 && infoGraph.pInfoArea[lLabelSeed].lSize>=lSize0)
					{
						pAreaCur->lFlag = 0;
						continue;
					}
					else if((right-left)*(bottom-top)<lSize0*1/2)
					{
						pAreaCur->lFlag = 0;
						continue;
					}
				}

				if(lLabelSeed == -1)
				{
					lLabelSeed = lLabelVal;
					continue;
				}
				else
				{
					MergeArea(infoGraph.pInfoArea, MAX(lLabelVal, lLabelSeed), 
						MIN(lLabelVal, lLabelSeed), infoGraph.lLabelNum);
					lLabelSeed = MIN(lLabelVal, lLabelSeed);
				}
			}
		}		
		if(lLabelSeed==-1)
			goto EXT;
	}

	//Merge other area	
	do 
	{
		x = MFalse;
		lLabelSeed = _MergeSeed(&infoGraph, lLabelSeed);		
		for(lLabelCur=0; lLabelCur<infoGraph.lLabelNum; lLabelCur++)
		{
			JINFO_AREA *pInfoArea = infoGraph.pInfoArea + lLabelCur;
			if(pInfoArea->lFlag < 0 || lLabelCur==lLabelSeed)
				continue;
			x |= _MergeOther(&infoGraph, lLabelCur, lLabelSeed);			
		}
	} while(x==MTrue);
 	UpdateGraph(&infoGraph);

#ifdef ENABLE_SEEDS_GENERATION
	{
		MLong lSeedsNum=*plSeedsNum;		
		GO(_WeightedMask(hMemMgr, pImg, pMskSkin, &infoGraph, lLabelSeed, 
			pLabelData, lLabelLine, pcrSeeds, plSeedsNum));	
		if(pMskSkin!=MNull)
		{
			lLabelSeed = _MergeSeed_Loose(&infoGraph, lLabelSeed);
			UpdateGraph(&infoGraph);
			for(x=lSeedsNum; x<*plSeedsNum; x++)
			{
				_WeightedMask_Seed(pImg, pMskSkin, &infoGraph, lLabelSeed, 
					pLabelData, lLabelLine, pcrSeeds[x], MNull);
			}
		}
	}
#else
	if(pMskSkin!=MNull)
	{
		for(y=0; y<pMskSkin->lHeight; y++)
		{
			for(x=0; x<pMskSkin->lWidth; x++)
			{
				lLabelCur = GET_VALID_LABEL(infoGraph.pInfoArea, pLabelData[lLabelLine*y+x]-1);
				if(lLabelCur!=lLabelSeed)
					continue;
				pMskSkin->pData[pMskSkin->lMaskLine*y+x] = //MASKV_SKIN;
					//(JMaskData)((pLabelData[lLabelLine*y+x]%255) + 1);
					(JMaskData)((lLabelCur%255) + 1);
			}
		}
	}
#endif
	
EXT:
	FreeVectMem(hMemMgr, pLabelData);
	ReleaseAreaGraph(hMemMgr, &infoGraph);
	return res;
}


JSTATIC MBool _CanMerged_Seed(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
							  MLong lComplexSeed, MCOLORREF crSeed);
JSTATIC MBool _CanMerged_Seed_Loose(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
							  MLong lComplexSeed, MCOLORREF crSeed);
JSTATIC MBool _CanMerged_Other(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
							  MLong lComplexSeed, MCOLORREF crSeed);

MBool _MergeOther(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal, 
					MLong lLabelSeed)
{
	MLong x;
	MBool bMerged = MFalse;
	do
	{
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + lLabelVal;
		MLong lComplex = GET_COMPLEX_EX(pAreaSeed->lBoundaryNum, pAreaSeed->lSize, pAreaSeed->lEdgeNum);	
		MCOLORREF crRef = PIXELVAL(pAreaSeed->lY/pAreaSeed->lSize, pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize);
		for(x=0; x<lLabelVal; x++)
		{
			JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea + x;			
			if(pAreaCur->lFlag<0 || x==lLabelSeed)
				continue;
			if(!_CanMerged_Other(pInfoGraph->pInfoArea, lLabelVal, x, lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, lLabelVal, x, pInfoGraph->lLabelNum);	
			bMerged = MTrue;
			break;
		}
		if(x < lLabelVal)
		{
			lLabelVal = x;		
			continue;
		}
		for(x=lLabelVal+1; x<pInfoGraph->lLabelNum; x++)
		{
			JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea + x;
			if(pAreaCur->lFlag<0 || x==lLabelSeed)
				continue;
			if(!_CanMerged_Other(pInfoGraph->pInfoArea, lLabelVal, x, lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, x, lLabelVal, pInfoGraph->lLabelNum);
			bMerged = MTrue;
			break;
		}
	}while(x<pInfoGraph->lLabelNum);	
	return bMerged;
}

MLong _MergeSeed(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal)
{
	MLong x;
	do
	{
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + lLabelVal;
		MLong lComplex = GET_COMPLEX_EX(pAreaSeed->lBoundaryNum, pAreaSeed->lSize, pAreaSeed->lEdgeNum);
		MCOLORREF crRef = PIXELVAL(pAreaSeed->lY/pAreaSeed->lSize, pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize);		
		for(x=0; x<lLabelVal; x++)
		{
			if(pInfoGraph->pInfoArea[x].lFlag<0)
				continue;
			if(!_CanMerged_Seed(pInfoGraph->pInfoArea, lLabelVal, x, 
				lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, lLabelVal, x, pInfoGraph->lLabelNum);	
			break;
		}
		if(x < lLabelVal)
		{
			lLabelVal = x;		
			continue;
		}
		for(x=lLabelVal+1; x<pInfoGraph->lLabelNum; x++)
		{
			if(pInfoGraph->pInfoArea[x].lFlag<0)
				continue;			
			if(!_CanMerged_Seed(pInfoGraph->pInfoArea, lLabelVal, x, 
				lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, x, lLabelVal, pInfoGraph->lLabelNum);
			break;
		}
	}while(x<pInfoGraph->lLabelNum);	
	return lLabelVal;
}
MLong _MergeSeed_Loose(JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelVal)
{
	MLong x;
	do
	{
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + lLabelVal;
		MLong lComplex = GET_COMPLEX_EX(pAreaSeed->lBoundaryNum, pAreaSeed->lSize, pAreaSeed->lEdgeNum);
		MCOLORREF crRef = PIXELVAL(pAreaSeed->lY/pAreaSeed->lSize, pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize);		
		for(x=0; x<lLabelVal; x++)
		{
			if(pInfoGraph->pInfoArea[x].lFlag<0)
				continue;
			if(!_CanMerged_Seed_Loose(pInfoGraph->pInfoArea, lLabelVal, x, 
				lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, lLabelVal, x, pInfoGraph->lLabelNum);	
			break;
		}
		if(x < lLabelVal)
		{
			lLabelVal = x;		
			continue;
		}
		for(x=lLabelVal+1; x<pInfoGraph->lLabelNum; x++)
		{
			if(pInfoGraph->pInfoArea[x].lFlag<0)
				continue;			
			if(!_CanMerged_Seed_Loose(pInfoGraph->pInfoArea, lLabelVal, x, 
				lComplex, crRef))
				continue;
			MergeArea(pInfoGraph->pInfoArea, x, lLabelVal, pInfoGraph->lLabelNum);
			break;
		}
	}while(x<pInfoGraph->lLabelNum);	
	return lLabelVal;
}

#define MAX_GRADIENT	16
MBool _CanMerged_Seed(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
				 MLong lComplexSeed, MCOLORREF crSeed)
{
	MLong lLabelFrom = MAX(lLabelSeed,lLabelcur);
	MLong lLabelTo = MIN(lLabelSeed, lLabelcur);
	const JINFO_AREA *pAreaFrom = pInfoArea + lLabelFrom;
	const JINFO_AREA *pAreaTo = pInfoArea + lLabelTo;
	const JINFO_AREA *pAreaCur = pInfoArea + lLabelcur;	
	MLong lMerge = 0;
	MLong lNeighBoundary = pAreaFrom->pBoundaryNum[lLabelTo+1];
	
	if(lNeighBoundary*4<=pAreaCur->lBoundaryNum)
		return MFalse;	
	if(pAreaCur->lEdgeNum*3>=lNeighBoundary)
		return MFalse;
// 	if(lNeighBoundary>=pAreaCur->lBoundaryNum*7/8 && pAreaCur->lY/pAreaCur->lSize>pInfoArea[lLabelSeed].lY/pInfoArea[lLabelSeed].lSize)
// 		return MTrue;
	lMerge += lComplexSeed-GET_COMPLEX_EX(pAreaFrom->lBoundaryNum+pAreaTo->lBoundaryNum-pAreaFrom->pBoundaryNum[lLabelTo+1]*2, pAreaFrom->lSize+pAreaTo->lSize, 
		pAreaFrom->lEdgeNum+pAreaTo->lEdgeNum);
	lMerge += 16*(MAX_COLOR_DIST-GetColorDist(pAreaCur->lY/pAreaCur->lSize, pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crSeed))/MAX_COLOR_DIST;
	lMerge += 4*(MAX_GRADIENT-pAreaFrom->pGradientSum[lLabelTo+1]/pAreaFrom->pBoundaryNum[lLabelTo+1])/MAX_GRADIENT;
	if(lMerge < 0)
		return MFalse;
	return MTrue;
}
MBool _CanMerged_Seed_Loose(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
							  MLong lComplexSeed, MCOLORREF crSeed)
{
	MLong lLabelFrom = MAX(lLabelSeed,lLabelcur);
	MLong lLabelTo = MIN(lLabelSeed, lLabelcur);
	const JINFO_AREA *pAreaFrom = pInfoArea + lLabelFrom;
	const JINFO_AREA *pAreaTo = pInfoArea + lLabelTo;
	const JINFO_AREA *pAreaCur = pInfoArea + lLabelcur;	
	MLong lMerge = 0, lMerge2 = 0;
	MLong lNeighBoundary = pAreaFrom->pBoundaryNum[lLabelTo+1];
	
	if(lNeighBoundary*4<=pAreaCur->lBoundaryNum)
		return MFalse;	
	if(pAreaCur->lEdgeNum*3>=lNeighBoundary)
		return MFalse;
	lMerge += lComplexSeed-GET_COMPLEX_EX(pAreaFrom->lBoundaryNum+pAreaTo->lBoundaryNum-pAreaFrom->pBoundaryNum[lLabelTo+1]*2, pAreaFrom->lSize+pAreaTo->lSize, 
		pAreaFrom->lEdgeNum+pAreaTo->lEdgeNum);
	if(lMerge >= 0)
		return MTrue;
	lMerge2 += 16*(MAX_COLOR_DIST-GetColorDist(pAreaCur->lY/pAreaCur->lSize, pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crSeed))/MAX_COLOR_DIST;
	lMerge2 += 4*(MAX_GRADIENT-pAreaFrom->pGradientSum[lLabelTo+1]/pAreaFrom->pBoundaryNum[lLabelTo+1])/MAX_GRADIENT;
	if(lMerge2 >= 0)
		return MTrue;
	return MFalse;
}
MBool _CanMerged_Other(const JINFO_AREA *pInfoArea, MLong lLabelSeed, MLong lLabelcur, 
							  MLong lComplexSeed, MCOLORREF crSeed)
{
	MLong lLabelFrom = MAX(lLabelSeed,lLabelcur);
	MLong lLabelTo = MIN(lLabelSeed, lLabelcur);
	const JINFO_AREA *pAreaFrom = pInfoArea + lLabelFrom;
	const JINFO_AREA *pAreaTo = pInfoArea + lLabelTo;
	const JINFO_AREA *pAreaCur = pInfoArea + lLabelcur;	
	MLong lMerge = 0;
	MLong lNeighBoundary = pAreaFrom->pBoundaryNum[lLabelTo+1];

	if(lNeighBoundary <= pAreaCur->lBoundaryNum/4)
		return MFalse;
	lMerge += lComplexSeed-GET_COMPLEX_EX(pAreaFrom->lBoundaryNum+pAreaTo->lBoundaryNum-pAreaFrom->pBoundaryNum[lLabelTo+1]*2, pAreaFrom->lSize+pAreaTo->lSize, 
		pAreaFrom->lEdgeNum+pAreaTo->lEdgeNum);
	lMerge += 8*((MAX_COLOR_DIST/2)-GetColorDist(pAreaCur->lY/pAreaCur->lSize, pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crSeed))/(MAX_COLOR_DIST/2);
	lMerge += 8*((MAX_GRADIENT/2)-pAreaFrom->pGradientSum[lLabelTo+1]/pAreaFrom->pBoundaryNum[lLabelTo+1])/(MAX_GRADIENT/2);
	if(lMerge < 0)
		return MFalse;
	return MTrue;
}

MRESULT _WeightedMask(MHandle hMemMgr, 
					  const JOFFSCREEN *pImg, JMASK *pMskGray, 
					const JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed, 
					const JLabelData *pLabelData, MLong lLabelLine, 
					MCOLORREF *pcrSeeds, MLong *plSeedsNum)
{
	MLong lValidGray = 160;
	MLong lSizeThres = 64;

	MCOLORREF  crSeed;
	MLong lSeedsNum = 0;
	JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + lLabelSeed;
	MLong *plGrayNum = MNull;	
	JBool *pbUsed = MNull;
	MRESULT res = LI_ERR_NONE;

	JMASK mskGray={0};
	AllocVectMem(hMemMgr, plGrayNum, pInfoGraph->lLabelNum, MLong);	
	AllocVectMem(hMemMgr, pbUsed, pInfoGraph->lLabelNum, JBool);
	SetVectMem(pbUsed, pInfoGraph->lLabelNum, 0, JBool);
	if(pMskGray==MNull)
	{
		GO(MaskCreate(hMemMgr, &mskGray, pImg->dwWidth, pImg->dwHeight));
		MaskSet(&mskGray, 0);
	}
	else
	{
		mskGray = *pMskGray;
	}
	
	if(plSeedsNum != MNull)
		lSeedsNum = *plSeedsNum;
	if(lSeedsNum >= MAX_SEEDS_NUM)
		goto EXT;
	crSeed = PIXELVAL(pAreaSeed->lY/pAreaSeed->lSize, pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize);
	if(pcrSeeds != MNull)
		pcrSeeds[lSeedsNum] = crSeed;
	lSeedsNum++;
	pbUsed[lLabelSeed] = 1;

	do
	{
		MLong lMaxSize = 0, lMaxLabel = -1, x;
		JINFO_AREA *pAreaCur;
		_WeightedMask_Seed(pImg, &mskGray, pInfoGraph, lLabelSeed, 
			pLabelData, lLabelLine, crSeed, plGrayNum);
		for(x=0; x<pInfoGraph->lLabelNum; x++)
		{
			if(pbUsed[x] != 0)
				continue;
			if(GET_VALID_LABEL(pInfoGraph->pInfoArea, x) != lLabelSeed)
				continue;
			if(plGrayNum[x]>=lValidGray*pInfoGraph->pInfoArea[x].lSize)
				continue;
			if(lMaxSize >= pInfoGraph->pInfoArea[x].lSize)
				continue;
			lMaxSize = pInfoGraph->pInfoArea[x].lSize;
			lMaxLabel = x;
		}
// 		if(lInvalidArea<=16 /*&& lMaxSize*lSizeThres <=pAreaSeed->lSize*/)
// 			break;
		if(lMaxSize*lSizeThres <= pAreaSeed->lSize)
			break;
		pAreaCur = pInfoGraph->pInfoArea + lMaxLabel;
		crSeed = PIXELVAL(pAreaCur->lY/lMaxSize, pAreaCur->lCb/lMaxSize, pAreaCur->lCr/lMaxSize);
		if(pcrSeeds != MNull)
			pcrSeeds[lSeedsNum] = crSeed;
		lSeedsNum++;
		pbUsed[lMaxLabel] = 1;
	} while(lSeedsNum < MAX_SEEDS_NUM);

	if(plSeedsNum != MNull)
		*plSeedsNum = lSeedsNum;
EXT:
	FreeVectMem(hMemMgr, pbUsed);
	FreeVectMem(hMemMgr, plGrayNum);
	if(pMskGray==MNull)
		MaskRelease(hMemMgr, &mskGray);
	return res;
}

MVoid _WeightedMask_Seed(const JOFFSCREEN *pImg, JMASK *pMskGray, 
						   const JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed, 
						   const JLabelData *pLabelData, MLong lLabelLine, 
						   MCOLORREF crSeed, MLong *plGrayNum)
{
	MLong lWidth = pImg->dwWidth, lHeight = pImg->dwHeight;
	MLong x, y;
	MCOLORREF crRef;
	if(plGrayNum != MNull)
		SetVectMem(plGrayNum, pInfoGraph->lLabelNum, 0, MLong);
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong lGray, lMskVal;
			MLong lLabelCur = pLabelData[lLabelLine*y+x]-1;
			if(lLabelCur<=0)
				continue;
			if(GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabelCur) != lLabelSeed)
				continue;
			lMskVal = pMskGray->pData[pMskGray->lMaskLine*y+x];	
			if(lMskVal==255)
			{
				if(plGrayNum!=MNull)
					plGrayNum[lLabelCur] += lMskVal;
				continue;
			}
			crRef = ImgGetPixel(pImg, x, y);
			lGray = GRAY_VAL(GetColorDist(PIXELVAL_1(crSeed), PIXELVAL_2(crSeed), PIXELVAL_3(crSeed), crRef));
			if(lMskVal<lGray)	
			{
				lMskVal = lGray;
				pMskGray->pData[pMskGray->lMaskLine*y+x] = (JMaskData)lMskVal;
			}
			if(plGrayNum!=MNull)
				plGrayNum[lLabelCur] += lMskVal;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
MRESULT FaceMask(MHandle hMemMgr, const JOFFSCREEN *pImg, 
				 const MRECT *prtFaces, MLong lFaceNum, 
				 JMASK *pMskSkin, 
				 MCOLORREF *pcrSeeds, MLong *plSeedsNum)
{
	MLong lFaceCur;
	if(pMskSkin!=MNull)
		MaskSet(pMskSkin, 0);
	if(plSeedsNum != MNull)
		*plSeedsNum = 0;
	for(lFaceCur=0; lFaceCur<lFaceNum; lFaceCur++)
	{
		const MRECT* prtFace = prtFaces + lFaceCur;
		MRECT rtOutier, rtInner;
		JOFFSCREEN imgFace = *pImg;
		JMASK mskFace = {0};
		if(pMskSkin!=MNull)
			mskFace = *pMskSkin;

		rtOutier.left = prtFace->left - (prtFace->right-prtFace->left)/4;
		rtOutier.right = prtFace->right + (prtFace->right-prtFace->left)/4;
		rtOutier.top = prtFace->top - (prtFace->bottom-prtFace->top)/4;
		rtOutier.bottom = prtFace->bottom + (prtFace->bottom-prtFace->top)/4;
		if(rtOutier.left < 0)	rtOutier.left = 0;
		if(rtOutier.top < 0)	rtOutier.top = 0;
		if(rtOutier.right > (MLong)pImg->dwWidth)	rtOutier.right = pImg->dwWidth;
		if(rtOutier.bottom > (MLong)pImg->dwHeight)	rtOutier.bottom = pImg->dwHeight;
		switch(pImg->fmtImg)
		{
		case FORMAT_YUV422_P8:
			rtOutier.left = CEIL_8(rtOutier.left), rtOutier.right = FLOOR_8(rtOutier.right);
			break;
		default:
			rtOutier.left = CEIL_2(rtOutier.left), rtOutier.right = FLOOR_2(rtOutier.right);
			break;
		}
		rtInner.left = prtFace->left + (prtFace->right-prtFace->left)/4 - rtOutier.left;
		rtInner.right = prtFace->right - (prtFace->right-prtFace->left)/4 - rtOutier.left;
		rtInner.top = prtFace->top + (prtFace->bottom-prtFace->top)/4 - rtOutier.top;
		rtInner.bottom = prtFace->bottom - (prtFace->bottom-prtFace->top)/4 - rtOutier.top;	
		if(rtInner.left >= rtInner.right || rtInner.top >= rtInner.bottom)
			continue;
		
		ImgOffset(&imgFace, rtOutier.left, rtOutier.top);
		imgFace.dwWidth = rtOutier.right - rtOutier.left;
		imgFace.dwHeight = rtOutier.bottom - rtOutier.top;

		if(pMskSkin != MNull)
		{
			mskFace.pData += rtOutier.top*mskFace.lMaskLine + rtOutier.left;
			mskFace.lWidth = rtOutier.right - rtOutier.left;
			mskFace.lHeight = rtOutier.bottom - rtOutier.top;
		}

		RETURN(SkinMask(hMemMgr, &imgFace, &rtInner, pMskSkin==MNull?MNull:&mskFace, 
			pcrSeeds, plSeedsNum));
	}
	return LI_ERR_NONE;
}

MCOLORREF SeedColorByPriorArea(const JOFFSCREEN* pImg, const MRECT* prtSkin)
{
	MLong x, y;
	MLong lY=0, lCb=0, lCr=0, lSize=0;
	MLong lCenterX = (prtSkin->left+prtSkin->right)/2;
	MLong lCenterY = (prtSkin->top+prtSkin->bottom)/2;
	MLong lR2X = SQARE((prtSkin->right-prtSkin->left)/2);
	MLong lR2Y = SQARE((prtSkin->bottom-prtSkin->top)/2);
	MLong lR2 = 0;
	if(lR2X<lR2Y)	lR2=lR2Y, lR2Y /= lR2X, lR2X = 1;
	else			lR2=lR2X, lR2X /= lR2Y, lR2Y = 1;
	
	for(y=prtSkin->top; y<prtSkin->bottom; y++)
	{
		MLong lSqareY = SQARE(y-lCenterY) * lR2X;
		for(x=prtSkin->left; x<prtSkin->right; x++)
		{
			MCOLORREF crRef;
			if(SQARE(x-lCenterX)*lR2Y + lSqareY > lR2)
				continue;
			crRef = ImgGetPixel(pImg, x, y);
			lY += PIXELVAL_1(crRef);
			lCb += PIXELVAL_2(crRef);
			lCr += PIXELVAL_3(crRef);
			lSize++;
		}
	}
	return PIXELVAL(lY/lSize, lCb/lSize, lCr/lSize);
}

MVoid GrayMaskByColorSeed(const JOFFSCREEN* pImg, MCOLORREF crSeed, 
				JMASK *pMskGray)
{
	MLong x, y;
	JMaskData *pMskData = pMskGray->pData;
	for(y=0; y<pMskGray->lHeight; y++, pMskData+=pMskGray->lMaskLine)
	{
		for(x=0; x<pMskGray->lWidth; x++)
		{
			MLong lDist = GRAY_VAL(GetColorDist(PIXELVAL_1(crSeed), PIXELVAL_2(crSeed),
				PIXELVAL_3(crSeed), ImgGetPixel(pImg, x, y)));
			if(lDist <= 0)
				continue;
			if(pMskData[x]==255)
				continue;
			pMskData[x] = (JMaskData)MAX(pMskData[x], lDist);
		}
	}
}
