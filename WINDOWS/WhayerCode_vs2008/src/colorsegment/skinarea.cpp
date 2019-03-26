#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "skinarea.h"
#include "litrimfun.h"
#include "liimage.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "limath.h"
#include "limask.h"
#include "colorsegment.h"
#include "bbgeometry.h"

#ifdef ENABLE_DEBUG
extern JLabelData *g_pLabelData;
extern MLong g_lLabelLine, g_lWidth, g_lHeight;
#endif

JSTATIC MBool _IsConnectedArea(const JINFO_AREA_GRAPH* pInfoGraph, 
	MLong lLabelSeed, MLong lLabelCur, MLong lMinAreaSize, MLong lColorDist);
JSTATIC MBool _IsConnectedArea_Strict(const JINFO_AREA_GRAPH* pInfoGraph, 
	MLong lLabelSeed, MLong lLabelCur, MLong lMinAreaSize, MLong lColorDist);
JSTATIC MBool _IsConnectedSkin(const JINFO_AREA* pArea, MLong lAreaFrom, MLong lAreaTo, MLong lColorDist, MLong lMinAreaSize);
//////////////////////////////////////////////////////////////////////////

MVoid AreaMerge(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize)
{
	MBool bMerged = MFalse;
	do
	{
		MLong lLabelCur;
		bMerged = MFalse;
		for(lLabelCur=pInfoGraph->lLabelNum-1; lLabelCur>=0; lLabelCur--)
		{
			MLong lMinDistX=0, lMinDist = 0x7FFFFFFF;
			//Save the boundary threshold as a constant when a loop to confirm the mutual.
			MLong lBoundaryThres = VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lLabelCur].lBoundaryNum);	
			
			if(pInfoGraph->pInfoArea[lLabelCur].lFlag <= 0)
				continue;
 			/*DrawCurrentArea(g_pLabelData, g_lLabelLine, g_lWidth, g_lHeight, 
 				pInfoGraph->pInfoArea, lLabelCur, ".\\area_seed.bmp");*/
			lMinDistX = NeighborWithMinDist(pInfoGraph, lLabelCur, lBoundaryThres, &lMinDist);
			if(lMinDistX<0)
				continue;
			if(_IsConnectedArea_Strict(pInfoGraph, lLabelCur, lMinDistX, 
				lMinAreaSize, lMinDist ))
				goto MERGE;
			if(lBoundaryThres > VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lMinDistX].lBoundaryNum))
				lBoundaryThres = VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lMinDistX].lBoundaryNum);
			if(NeighborWithMinDist(pInfoGraph, lMinDistX, lBoundaryThres, &lMinDist) >= 0)
				continue;
 		
			if(!_IsConnectedArea(pInfoGraph, lLabelCur, lMinDistX, lMinAreaSize, lMinDist ))
				continue;
MERGE:
			/*DrawCurrentArea(g_pLabelData, g_lLabelLine, g_lWidth, g_lHeight, 
 			pInfoGraph->pInfoArea, lMinDistX, ".\\area_cur.bmp");*/
			MergeArea(pInfoGraph->pInfoArea,  MAX(lLabelCur, lMinDistX), 
				MIN(lLabelCur, lMinDistX), pInfoGraph->lLabelNum);
			bMerged = MTrue;
		}
	}while(bMerged);
	UpdateGraph(pInfoGraph);
}

MBool _IsConnectedArea_Strict(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, MLong lLabelCur, 
					   MLong lMinAreaSize, MLong lColorDist)
{
	const JINFO_AREA* pInfoArea = pInfoGraph->pInfoArea;
	const JINFO_AREA* pAreaSeed = pInfoArea+lLabelSeed, *pAreaCur = pInfoArea+lLabelCur;
	MLong lLinkNum = pInfoArea[MAX(lLabelSeed, lLabelCur)].pBoundaryNum[MIN(lLabelSeed, lLabelCur)+1];	
	MLong lGradientSum = pInfoArea[MAX(lLabelSeed, lLabelCur)].pGradientSum[MIN(lLabelSeed, lLabelCur)+1];
	//Small color distance
	if(lColorDist*4<=MAX_COLOR_DIST)//或者颜色差距很小
		return MTrue;	
	if(lGradientSum*16<=MAX_AREA_GRADIENT*lLinkNum)//或者blob之间的梯度和很小
		return MTrue;	
	if(pAreaSeed->lSize<=lMinAreaSize && lLinkNum*8>=pAreaSeed->lBoundaryNum*7)//>=7	//或者blob的size吧 很小
		return MTrue;

	return MFalse;
}


MBool _IsConnectedArea(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, MLong lLabelCur, 
					   MLong lMinAreaSize, MLong lColorDist)
{
	const JINFO_AREA* pInfoArea = pInfoGraph->pInfoArea;
	const JINFO_AREA* pAreaSeed = pInfoArea+lLabelSeed, *pAreaCur = pInfoArea+lLabelCur;
	MLong lGradientSum = pInfoArea[MAX(lLabelSeed, lLabelCur)].pGradientSum[MIN(lLabelSeed, lLabelCur)+1];
	MLong lLinkNum = pInfoArea[MAX(lLabelSeed, lLabelCur)].pBoundaryNum[MIN(lLabelSeed, lLabelCur)+1];	
	MLong lUnionComplex, lComplex0, lComplex1;		
	lComplex0 = GET_COMPLEX_EX(pAreaSeed->lBoundaryNum, pAreaSeed->lSize, pAreaSeed->lEdgeNum);
	lComplex1 = GET_COMPLEX_EX(pAreaCur->lBoundaryNum, pAreaCur->lSize, pAreaCur->lEdgeNum);

	//Small color distance
	if(lColorDist*4<=MAX_COLOR_DIST)
		return MTrue;	
	//Smooth boundary
	if(lGradientSum*8<=MAX_AREA_GRADIENT*lLinkNum)
		return MTrue;
	////Small area
	if(pAreaSeed->lSize<=lMinAreaSize/2)
		return MTrue;
	if(lColorDist*2<=MAX_COLOR_DIST && lLinkNum*8>=pAreaSeed->lBoundaryNum*4)//>=6
		return MTrue;
	if(pAreaCur->lSize<=lMinAreaSize && lLinkNum*8>=pAreaCur->lBoundaryNum*4)//>=6
		return MTrue;
	
	//Background
	if(IsOnEdge(pAreaSeed))
	{
		if(IsOnEdge(pAreaCur) /*|| lComplex1>MAX_AREA_COMPLEX*/)
			return MTrue;
		return MFalse;
	}
	if(IsOnEdge(pAreaCur))
	{
		if(IsOnEdge(pAreaSeed) /*|| lComplex0>MAX_AREA_COMPLEX*/)
			return MTrue;
		return MFalse;
	}	
	
	//Single neighbor
	if(1>=NeighborNum(pInfoGraph, lLabelSeed, VALID_NEIGHBOR_BOUNDARY(pAreaSeed->lBoundaryNum)) 
		|| 1>=NeighborNum(pInfoGraph, lLabelCur, VALID_NEIGHBOR_BOUNDARY(pAreaCur->lBoundaryNum)) )
	{
		if(lColorDist*4<=MAX_COLOR_DIST)
			return MTrue;
		//Bright area
		if(pAreaSeed->lY>BRIGHT_Y*pAreaSeed->lSize && lLinkNum*8>=pAreaSeed->lBoundaryNum*6)
			return MTrue;
		if(pAreaCur->lY>BRIGHT_Y*pAreaCur->lSize && lLinkNum*8>=pAreaCur->lBoundaryNum*6)
			return MTrue;
		return MFalse;
	}	

	if(lGradientSum*8<=MAX_AREA_GRADIENT*2*lLinkNum		//<=2
		&& lColorDist*2<=MAX_COLOR_DIST)				//<=2
		return MTrue;
	//Bright area
	if((pAreaSeed->lY>BRIGHT_Y*pAreaSeed->lSize || pAreaCur->lY>BRIGHT_Y*pAreaCur->lSize)
		&& lColorDist<=MAX_COLOR_DIST)
		return MTrue;
 	//Complex
	lUnionComplex = GET_COMPLEX_EX(pAreaSeed->lBoundaryNum+pAreaCur->lBoundaryNum-lLinkNum*2, pAreaSeed->lSize+pAreaCur->lSize, pAreaSeed->lEdgeNum+pAreaCur->lEdgeNum);
	if(lUnionComplex<MAX(lComplex0, lComplex1) && lComplex0>MAX_AREA_COMPLEX && lComplex1>MAX_AREA_COMPLEX)
		return MTrue;
	if(lUnionComplex<MAX(lComplex0, lComplex1) && lColorDist*2<MAX_COLOR_DIST)
		return MTrue;
	if((IS_STRIP_AREA(pAreaSeed)||IS_STRIP_AREA(pAreaCur)) && lColorDist*2<MAX_COLOR_DIST)
		return MTrue;	
	/*if(lUnionComplex<=MIN(lComplex0, lComplex1))
		return MTrue;	*/

	return MFalse;
}

//提前过滤无效不符合的区域
MVoid	SkinFilter(JINFO_AREA_GRAPH *pInfoGraph, const PARAM_SKIN_FILTER *pParam, 
				   MLong lMinAreaSize, MLong lCbOffset, MLong lCrOffset)
{
	MLong lLabelCur=0, lLabelNum = pInfoGraph->lLabelNum;


	for(lLabelCur=0; lLabelCur<lLabelNum; lLabelCur++)
	{
		JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea + lLabelCur; 
		MLong lAreaW = (pInfoArea->rtArea.right-pInfoArea->rtArea.left);
		MLong lAreaH = (pInfoArea->rtArea.bottom-pInfoArea->rtArea.top);
		if(pInfoArea->lFlag <= 0)
			continue;	
	//根据颜色过滤，非红色区域提前过滤掉，		
		if(pParam->bRemoveNoSkinColor)
		{
			MLong lSize = pInfoArea->lSize;
			if(!IS_DIGITAL_RED_COLOR(pInfoArea->lY/lSize, pInfoArea->lCb/lSize, pInfoArea->lCr/lSize)
				&& !IS_DIGITAL_RED_COLOR(pInfoArea->lY/lSize, pInfoArea->lCb/lSize+lCbOffset, pInfoArea->lCr/lSize+lCrOffset)) 
			{
				RemoveArea(pInfoGraph->pInfoArea, lLabelCur, lLabelNum);
				continue;
			}
		}
		if(pParam->bRemoveAreaOnEdge)
		{
			if(IsOnEdge(pInfoArea))
			{
				RemoveArea(pInfoGraph->pInfoArea, lLabelCur, lLabelNum);
				continue;
			}
		}

		if(pParam->bRemoveRectangleArea)
		{
			if(pInfoArea->lSize>=lAreaW*lAreaH*7/8 && pInfoArea->lSize>lMinAreaSize/4)
			{
				RemoveArea(pInfoGraph->pInfoArea, lLabelCur, lLabelNum);
				continue;
			}
		}
	}

}

MRESULT ConnectNeighborInSkin(MHandle hMemMgr, JINFO_AREA_GRAPH *pInfoGraph,
	const JLabelData* pLabelData, MLong lLabelLine, MLong lWidth, MLong lHeight)
{
	MLong lLabelCur, lLabelNum = pInfoGraph->lLabelNum;
	MBool bMerged = MFalse;
//	MLong lThres;
	MRESULT res = LI_ERR_NONE;
	for (int i=0; i<pInfoGraph->lLabelNum; i++)
	{
		AllocVectMem(hMemMgr, pInfoGraph->pInfoArea[i].pDistanceWithBlobs, pInfoGraph->lLabelNum, MDWord);
		SetVectZero(pInfoGraph->pInfoArea[i].pDistanceWithBlobs, pInfoGraph->lLabelNum*sizeof(MDWord));
	}
	 
	GO(BlobDistCalulate(hMemMgr, pInfoGraph, pLabelData, lLabelLine, lWidth,  lHeight));

	do
	{
		bMerged = MFalse;
		for(lLabelCur=0; lLabelCur<lLabelNum; lLabelCur++)
		{			
			MLong lMinDistX=0, lMinDist = 0x7FFFFFFF;
			MLong lMaxOverlappingRate = 100;
			MPOINT ptG1, ptG2;
			MDouble Theta;

			if(pInfoGraph->pInfoArea[lLabelCur].lFlag <= 0)
				continue;
			
			lMinDistX = NeighborWithMinDist1(pInfoGraph, lLabelCur, &lMinDist);
			if(lMinDistX<0)
				continue;	

			MergeArea(pInfoGraph->pInfoArea,  MAX(lMinDistX, lLabelCur), MIN(lMinDistX, lLabelCur), pInfoGraph->lLabelNum);
			bMerged = MTrue;
		}
	}while(bMerged);
	UpdateGraph(pInfoGraph);
EXT:
	
	return res;
}

MVoid	FillHoleInSkin(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize)
{
	MLong lLabelCur;
	for(lLabelCur=0; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
	{			
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea+lLabelCur;
		MLong x;
		if(pAreaSeed->lFlag!=0)
			continue;
		for(x=lLabelCur+1; x<pInfoGraph->lLabelNum; x++)
		{
			JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea + x;
			MLong lThres;
			if(pAreaCur->lFlag!=0)
				continue;
			lThres = MIN(VALID_NEIGHBOR_BOUNDARY(pAreaSeed->lBoundaryNum), 
				VALID_NEIGHBOR_BOUNDARY(pAreaCur->lBoundaryNum));
			if(pAreaCur->pBoundaryNum[lLabelCur+1] <= lThres)
				continue;
			MergeArea(pInfoGraph->pInfoArea, x, lLabelCur, pInfoGraph->lLabelNum);
		}
	}
	for(lLabelCur=0; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
	{
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea+lLabelCur, *pAreaCur=MNull;
		MLong x, lLinkCur;
		MLong lLinkNum=0, lMaxLink=0, lMaxLinkX=-1;
		if(pAreaSeed->lFlag!=0)
			continue;
		if(pAreaSeed->lSize > lMinAreaSize)
			continue;
 		if(!IS_DIGITAL_RED_COLOR(pAreaSeed->lY/pAreaSeed->lSize, 
 			pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize))
 			continue;
		for(x=0; x<lLabelCur; x++)
		{			
			pAreaCur = pInfoGraph->pInfoArea + x;
			lLinkCur = pAreaSeed->pBoundaryNum[x+1];			
			if(pAreaCur->lFlag<=0)
				continue;
			if(lLinkCur==0)
				continue;
			lLinkNum += lLinkCur;
			if(lLinkCur <= lMaxLink)
				continue;
			lMaxLink = lLinkCur;
			lMaxLinkX = x;
		}
		for(x=lLabelCur+1; x<pInfoGraph->lLabelNum; x++)
		{
			pAreaCur = pInfoGraph->pInfoArea + x;
			lLinkCur = pAreaCur->pBoundaryNum[lLabelCur+1];
			if(pAreaCur->lFlag<=0)
				continue;
			if(lLinkCur==0)
				continue;
			lLinkNum += lLinkCur;
			if(lLinkCur <= lMaxLink)
				continue;
			lMaxLink = lLinkCur;
			lMaxLinkX = x;
		}
		if(lMaxLinkX<0)
			continue;
		JASSERT(lMaxLinkX+1==pInfoGraph->pInfoArea[lMaxLinkX].lFlag);

		if(lMaxLink*8>=pAreaSeed->lBoundaryNum*7)
			goto MERGE;
		pAreaCur = pInfoGraph->pInfoArea + lMaxLinkX;
		lLinkCur = GetColorDist(pAreaCur->lY/pAreaCur->lSize, pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, 
			PIXELVAL(pAreaSeed->lY/pAreaSeed->lSize, pAreaSeed->lCb/pAreaSeed->lSize, pAreaSeed->lCr/pAreaSeed->lSize));
		if(lLinkNum*16<pAreaSeed->lBoundaryNum*15 || lLinkCur*3 >= MAX_COLOR_DIST*8)	//>=15, >=3
			continue;
MERGE:
		pInfoGraph->pInfoArea[MIN(lMaxLinkX,lLabelCur)].lFlag = MIN(lMaxLinkX,lLabelCur)+1;
		MergeArea(pInfoGraph->pInfoArea, MAX(lMaxLinkX, lLabelCur), MIN(lMaxLinkX,lLabelCur), pInfoGraph->lLabelNum);
	}
	UpdateGraph(pInfoGraph);
}

JSTATIC MLong  _AreaGrayNum(const JINFO_AREA* pInfoArea, MLong lLabelCur, 
							const JLabelData* pLabelData, MLong lLabelLine, 
							const JMASK *pMskGray);
JSTATIC MLong  _HighGrayNum(const JINFO_AREA* pInfoArea, MLong lLabelCur, 
							const JLabelData* pLabelData, MLong lLabelLine, 
							const JMASK *pMskGray, MLong lHighGray);
MVoid	SkinFilterByGrayMask(JINFO_AREA_GRAPH *pInfoGraph, 
							 const JMASK* pMskGray, MLong lGrayThres, 
							 const JLabelData *pLabelData, MLong lLabelLine)
{
	MLong lLabelCur;
	for(lLabelCur=0; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
	{
		MLong lGrayNum=0;
		if(pInfoGraph->pInfoArea[lLabelCur].lFlag <= 0)
			continue;
#ifdef ENABLE_SAVE_FLASH_AREA
		if(pInfoGraph->pInfoArea[lLabelCur].lY>=BRIGHT_Y*pInfoGraph->pInfoArea[lLabelCur].lSize)
			continue;
#endif
		lGrayNum = _AreaGrayNum(pInfoGraph->pInfoArea, lLabelCur, 
			pLabelData, lLabelLine, pMskGray);
		if(lGrayNum < lGrayThres)
		{
			RemoveArea(pInfoGraph->pInfoArea, lLabelCur, pInfoGraph->lLabelNum);
			continue;
		}
		lGrayNum = _HighGrayNum(pInfoGraph->pInfoArea, lLabelCur, 
			pLabelData, lLabelLine, pMskGray, 180);
		if(lGrayNum < pInfoGraph->pInfoArea[lLabelCur].lSize/16)
		{
			RemoveArea(pInfoGraph->pInfoArea, lLabelCur, pInfoGraph->lLabelNum);
			continue;
		}		
	}
}

MLong  _AreaGrayNum(const JINFO_AREA* pInfoArea, MLong lLabelVal, 
					const JLabelData* pLabelData, MLong lLabelLine, 
					const JMASK *pMskGray)
{
	const MRECT* prtArea = &pInfoArea[lLabelVal].rtArea;
	MLong x, y;
	const JLabelData* pLabelCur = pLabelData+lLabelLine*prtArea->top+prtArea->left;
	const JMaskData* pMskData = pMskGray->pData+pMskGray->lMaskLine*prtArea->top+prtArea->left;
	MLong lWidth = prtArea->right - prtArea->left;
	MLong lHeight = prtArea->bottom - prtArea->top;
	MLong lGrayNum = 0;
	JASSERT(prtArea->right<=pMskGray->lWidth && prtArea->bottom<=pMskGray->lHeight);
	JASSERT(prtArea->left>=0 && prtArea->top>=0);
	//PrintBmp(pMskData, pMskGray->lMaskLine, DATA_U8, lWidth, lHeight, 1);
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			if(GET_VALID_LABEL(pInfoArea, pLabelCur[x]-1)!=lLabelVal)
				continue;
			lGrayNum += pMskData[x];
		}
		pLabelCur+=lLabelLine, pMskData+=pMskGray->lMaskLine;
	}
	JASSERT(pInfoArea[lLabelVal].lSize > 0);
	return lGrayNum/pInfoArea[lLabelVal].lSize;
}
MLong  _HighGrayNum(const JINFO_AREA* pInfoArea, MLong lLabelVal, 
					const JLabelData* pLabelData, MLong lLabelLine,
					const JMASK *pMskGray, MLong lHighGray)
{
	const MRECT* prtArea = &pInfoArea[lLabelVal].rtArea;
	MLong x, y;
	const JLabelData* pLabelCur = pLabelData+lLabelLine*prtArea->top+prtArea->left;
	const JMaskData* pMskData = pMskGray->pData+pMskGray->lMaskLine*prtArea->top+prtArea->left;
	MLong lWidth = prtArea->right - prtArea->left;
	MLong lHeight = prtArea->bottom - prtArea->top;
	MLong lGrayNum = 0;
	JASSERT(prtArea->right<=pMskGray->lWidth && prtArea->bottom<=pMskGray->lHeight);
	JASSERT(prtArea->left>=0 && prtArea->top>=0);
	//PrintBmp(pMskData, pMskGray->lMaskLine, DATA_U8, lWidth, lHeight, 1);
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			if(GET_VALID_LABEL(pInfoArea, pLabelCur[x]-1)!=lLabelVal)
				continue;
			if(pMskData[x] < lHighGray)
				continue;
			lGrayNum ++;
		}
		pLabelCur+=lLabelLine, pMskData+=pMskGray->lMaskLine;
	}
	JASSERT(lGrayNum <= pInfoArea[lLabelVal].lSize);
	return lGrayNum;
}

MVoid	MergeSkinArea(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize)
{
	MLong lLabelCur, lLabelNum = pInfoGraph->lLabelNum;
	MBool bMerged = MFalse;
	do
	{
		bMerged = MFalse;
		for(lLabelCur=0; lLabelCur<lLabelNum; lLabelCur++)
		{			
			MLong lMinDistX=0, lMinDist = 0x7FFFFFFF;
			MLong lBoundaryThres = VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lLabelCur].lBoundaryNum);	
			
			if(pInfoGraph->pInfoArea[lLabelCur].lFlag <= 0)
				continue;
			lMinDistX = NeighborWithMinDist(pInfoGraph, lLabelCur, lBoundaryThres, &lMinDist);
			if(lMinDistX<0)
				continue;
			if(lBoundaryThres > VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lMinDistX].lBoundaryNum))
				lBoundaryThres = VALID_NEIGHBOR_BOUNDARY(pInfoGraph->pInfoArea[lMinDistX].lBoundaryNum);
			if(NeighborWithMinDist(pInfoGraph, lMinDistX, lBoundaryThres, &lMinDist) >= 0)
				continue;			
			if(!_IsConnectedSkin(pInfoGraph->pInfoArea, MAX(lMinDistX, lLabelCur), 
				MIN(lMinDistX, lLabelCur), lMinDist, lMinAreaSize ))
				continue;			
			MergeArea(pInfoGraph->pInfoArea,  MAX(lMinDistX, lLabelCur), MIN(lMinDistX, lLabelCur), 
				pInfoGraph->lLabelNum);
			bMerged = MTrue;
		}
	}while(bMerged);
	UpdateGraph(pInfoGraph);
}

MBool _IsConnectedSkin(const JINFO_AREA* pArea, MLong lAreaFrom, MLong lAreaTo, 
						MLong lColorDist, MLong lMinAreaSize)
{
	const JINFO_AREA* pAreaFrom = pArea+lAreaFrom, *pAreaTo = pArea+lAreaTo;
	MLong lGradientSum = pArea[lAreaFrom].pGradientSum[lAreaTo+1];
	MLong lLinkNum = pArea[lAreaFrom].pBoundaryNum[lAreaTo+1];	
	MLong lUnionComplex, lComplexFrom, lComplexTo;
	JASSERT(lAreaFrom > lAreaTo);

	//Big area
	if(pAreaFrom->lSize>lMinAreaSize*3 && pAreaTo->lSize>lMinAreaSize*3)	//<=3
		return MFalse;	
	//Small area	
	if(pAreaFrom->lSize<=lMinAreaSize && lColorDist<=MAX_COLOR_DIST)
		return MTrue;
	//Gradient
	if(lGradientSum*8<=MAX_AREA_GRADIENT*4*lLinkNum		//<=4
		&& lColorDist*8<=MAX_COLOR_DIST*4)				//<=4
		return MTrue;
	//Included area
	if(lLinkNum*8 >= pAreaFrom->lBoundaryNum*7)			//>=7
		return MTrue;
	//Complex	
	lComplexFrom = GET_COMPLEX_EX(pAreaFrom->lBoundaryNum, pAreaFrom->lSize, pAreaFrom->lEdgeNum);
	lComplexTo = GET_COMPLEX_EX(pAreaTo->lBoundaryNum, pAreaTo->lSize, pAreaTo->lEdgeNum);
	lUnionComplex = GET_COMPLEX_EX(pAreaFrom->lBoundaryNum+pAreaTo->lBoundaryNum-lLinkNum*2, pAreaFrom->lSize+pAreaTo->lSize, pAreaFrom->lEdgeNum+pAreaTo->lEdgeNum);
	if(lUnionComplex<MAX_AREA_COMPLEX && lUnionComplex<MAX(lComplexFrom, lComplexTo) &&
		(lComplexTo>MAX_AREA_COMPLEX||lComplexFrom>MAX_AREA_COMPLEX||lColorDist*8<=MAX_COLOR_DIST*4))	//<=4
		return MTrue;
	//if(lUnionComplex<=MIN(lComplexFrom, lComplexTo) && lColorDist<=MAX_COLOR_DIST*2)
	//	return MTrue;
	return MFalse;
}
