#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "areagraph.h"
#include "litrimfun.h"
#include "liimage.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "limath.h"
#include "limask.h"
#include "bbgeometry.h"
#include "colorsegment.h"

MVoid UpdateGraph(JINFO_AREA_GRAPH *pInfoGraph)
{
	MLong lLabelCur = 0;
	for(lLabelCur=0; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
	{
		MLong lFlag = pInfoGraph->pInfoArea[lLabelCur].lFlag;
		if(lFlag >= 0)
		{
			JASSERT(lFlag==0 || lLabelCur+1 == lFlag);
			continue;
		}		
		while (pInfoGraph->pInfoArea[-lFlag-1].lFlag < 0)
			lFlag = pInfoGraph->pInfoArea[-lFlag-1].lFlag;
		JASSERT(lFlag<0);
		JASSERT(pInfoGraph->pInfoArea[-lFlag-1].lFlag==0 || pInfoGraph->pInfoArea[-lFlag-1].lFlag==-lFlag);
		pInfoGraph->pInfoArea[lLabelCur].lFlag = lFlag;
	}
}

MVoid UpdateLabel(MHandle hMemMgr, 
				  JINFO_AREA_GRAPH* pInfoGraph, JLabelData *pLabelData, 
				  MLong lLabelLine, MLong lWidth, MLong lHeight)
{
	MLong x, y;
	y = 1;
	for(x=0; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea + x;
		if(pAreaCur->lFlag<=0)
			continue;
		JASSERT(pAreaCur->lFlag>=y);
		pAreaCur->lFlag = y++;
	}
	for(y=0; y<lHeight; y++, pLabelData+=lLabelLine)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong lLabelCur = pLabelData[x];
			if(lLabelCur <= 0)
				continue;
			lLabelCur = GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabelCur-1);
			pLabelData[x] = (JLabelData)pInfoGraph->pInfoArea[lLabelCur].lFlag;
		}
	}

	y = 0;
	for(x=0; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea + x;
		JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + y;
		MLong i=1, j=1;
		if(pAreaCur->lFlag<=0)
			continue;
		pAreaSeed->lBoundaryNum = pAreaCur->lBoundaryNum;
		pAreaSeed->lEdgeNum = pAreaCur->lEdgeNum;
		pAreaSeed->lSize = pAreaCur->lSize;
		pAreaSeed->lY = pAreaCur->lY;
		pAreaSeed->lCb = pAreaCur->lCb;
		pAreaSeed->lCr = pAreaCur->lCr;
		pAreaSeed->rtArea = pAreaCur->rtArea;			
		JASSERT(pAreaCur->lFlag==y+1);
		for(i=0; i<x; i++)
		{
			if(pInfoGraph->pInfoArea[i].lFlag<=0)
				continue;
			pAreaSeed->pBoundaryNum[j] = pAreaCur->pBoundaryNum[i+1];
			pAreaSeed->pGradientSum[j] = pAreaCur->pGradientSum[i+1];
			j++; 
		}
		JASSERT(j==y+1); 
		y++;
	}
	for(x=0; x<y; x++)
		pInfoGraph->pInfoArea[x].lFlag = x+1;
	for(x=y; x<=pInfoGraph->lLabelNum; x++)
	{
		FreeVectMem(hMemMgr, pInfoGraph->pInfoArea[x].pBoundaryNum);
		FreeVectMem(hMemMgr, pInfoGraph->pInfoArea[x].pGradientSum);
	}
	pInfoGraph->lLabelNum = y;
}

MVoid RemoveArea(JINFO_AREA* pInfoArea, 
				 MLong lLabel, MLong lLabelNum)
{
	pInfoArea[lLabel].lFlag = 0;
}

MVoid MergeArea(JINFO_AREA* pInfoArea, 
				MLong lAreaNoFrom, MLong lAreaNoTo, MLong lLabelNum)
{
	MLong i;
	JASSERT(lAreaNoFrom > lAreaNoTo);
	pInfoArea[lAreaNoTo].lSize += pInfoArea[lAreaNoFrom].lSize;
	pInfoArea[lAreaNoTo].lY  += pInfoArea[lAreaNoFrom].lY;
	pInfoArea[lAreaNoTo].lCb += pInfoArea[lAreaNoFrom].lCb;
	pInfoArea[lAreaNoTo].lCr += pInfoArea[lAreaNoFrom].lCr;
	pInfoArea[lAreaNoTo].lBoundaryNum += pInfoArea[lAreaNoFrom].lBoundaryNum-pInfoArea[lAreaNoFrom].pBoundaryNum[lAreaNoTo+1]*2;
	pInfoArea[lAreaNoTo].lEdgeNum += pInfoArea[lAreaNoFrom].lEdgeNum;
	if(pInfoArea[lAreaNoTo].rtArea.left > pInfoArea[lAreaNoFrom].rtArea.left)
		pInfoArea[lAreaNoTo].rtArea.left = pInfoArea[lAreaNoFrom].rtArea.left;
	if(pInfoArea[lAreaNoTo].rtArea.right < pInfoArea[lAreaNoFrom].rtArea.right)
		pInfoArea[lAreaNoTo].rtArea.right = pInfoArea[lAreaNoFrom].rtArea.right;
	if(pInfoArea[lAreaNoTo].rtArea.top > pInfoArea[lAreaNoFrom].rtArea.top)
		pInfoArea[lAreaNoTo].rtArea.top = pInfoArea[lAreaNoFrom].rtArea.top;
	if(pInfoArea[lAreaNoTo].rtArea.bottom < pInfoArea[lAreaNoFrom].rtArea.bottom)
		pInfoArea[lAreaNoTo].rtArea.bottom = pInfoArea[lAreaNoFrom].rtArea.bottom;
	
	for(i=0; i<lAreaNoTo; i++)
	{
		pInfoArea[lAreaNoTo].pBoundaryNum[i+1] += pInfoArea[lAreaNoFrom].pBoundaryNum[i+1];
		pInfoArea[lAreaNoTo].pGradientSum[i+1] += pInfoArea[lAreaNoFrom].pGradientSum[i+1];
	}
	for(i=lAreaNoTo+1; i<lAreaNoFrom; i++)
	{
		pInfoArea[i].pBoundaryNum[lAreaNoTo+1] += pInfoArea[lAreaNoFrom].pBoundaryNum[i+1]; 			
		pInfoArea[i].pGradientSum[lAreaNoTo+1] += pInfoArea[lAreaNoFrom].pGradientSum[i+1];
	}
	for(i=lAreaNoFrom+1; i<lLabelNum; i++)
	{
		pInfoArea[i].pBoundaryNum[lAreaNoTo+1] += pInfoArea[i].pBoundaryNum[lAreaNoFrom+1];
		pInfoArea[i].pGradientSum[lAreaNoTo+1] += pInfoArea[i].pGradientSum[lAreaNoFrom+1];
	}

	///update distance 
	if (pInfoArea[lAreaNoTo].pDistanceWithBlobs)
	{
		for (i=0; i<lLabelNum; i++)
		{
			pInfoArea[lAreaNoTo].pDistanceWithBlobs[i] = 
				MIN(pInfoArea[lAreaNoFrom].pDistanceWithBlobs[i], pInfoArea[lAreaNoTo].pDistanceWithBlobs[i]);
		}

		for (i=0; i<lLabelNum;i++)
		{
			pInfoArea[i].pDistanceWithBlobs[lAreaNoTo] = 
				pInfoArea[lAreaNoTo].pDistanceWithBlobs[i];
		}
	}
		
	pInfoArea[lAreaNoFrom].lFlag = -(lAreaNoTo+1);	
}

MVoid RemoveMargin(JINFO_AREA_GRAPH *pInfoGraph, MLong lWidth, MLong lHeight)
{
	JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea;

	//Top/left margin 
	if(pInfoArea->rtArea.left==0 && pInfoArea->rtArea.top==0
		&& (pInfoArea->rtArea.right==lWidth || pInfoArea->rtArea.bottom==lHeight)
		&& pInfoArea->lEdgeNum > pInfoArea->lBoundaryNum/2
		&& pInfoArea->lSize <= lWidth*lHeight/4)
	{
		MLong lLabelCur;
		pInfoArea->lFlag = 0;
		for(lLabelCur=1; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
		{
			//JASSERT(pInfoArea[lLabelCur].lFlag > 0);
			if(pInfoArea[lLabelCur].lFlag <= 0)
				continue;
			pInfoArea[lLabelCur].lEdgeNum += pInfoArea[lLabelCur].pBoundaryNum[1];
			pInfoArea[lLabelCur].pBoundaryNum[1] = 0;
			pInfoArea[lLabelCur].pGradientSum[1] = 0;
		}
	}
}

///该函数与NeighborWithMinDist的区别在于，待Merge的多个blob不是相邻的，
///也就是说boundary可能会=0
///merge的条件:ColorDist 要小，像素距离也要小.对于MinDist的值，至少要覆盖，与WordCrop中的合并形成互补
MLong NeighborWithMinDist1(const JINFO_AREA_GRAPH* pInfoGraph, 
					MLong lLabelSeed, MLong *plMinDist)
{
	MLong x, lDist, lPixelDist;
	MLong lMinDistX=-1;
	MLong lThres;
	JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea + lLabelSeed;
	MCOLORREF crRef = PIXELVAL(pInfoArea->lY/pInfoArea->lSize, pInfoArea->lCb/pInfoArea->lSize, pInfoArea->lCr/pInfoArea->lSize);
//	JASSERT(pInfoArea->lFlag > 0); 
	for(x=0; x<lLabelSeed; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;
		lPixelDist = pAreaCur->pDistanceWithBlobs[lLabelSeed];
		if (lPixelDist>SQARE(MIN_DISTANCE_BW_BLOB)+4)
			continue;

		lDist = GetColorDist(pAreaCur->lY/pAreaCur->lSize, 
			pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crRef);
		if (lDist>MAX_COLOR_DIST*3/4)
			continue;
		
		if (*plMinDist>lPixelDist)
		{
			*plMinDist = lPixelDist;
			lMinDistX = x;
		}
	}
	for(x=lLabelSeed+1; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;

		lPixelDist = pAreaCur->pDistanceWithBlobs[lLabelSeed];
		if (lPixelDist>SQARE(MIN_DISTANCE_BW_BLOB)+4)
			continue;
		
		lDist = GetColorDist(pAreaCur->lY/pAreaCur->lSize, 
			pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crRef);
		if (lDist>MAX_COLOR_DIST*3/4)
			continue;
		
		if (*plMinDist>lPixelDist)
		{
			*plMinDist = lPixelDist;
			lMinDistX = x;
		}
	}			
	return lMinDistX;
}

MLong NeighborWithMinDist(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, 
						  MLong lBoundaryThres, MLong *plMinDist)
{
	MLong x, lDist;
	MLong lMinDistX=-1;
	JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea + lLabelSeed;
	MCOLORREF crRef = PIXELVAL(pInfoArea->lY/pInfoArea->lSize, pInfoArea->lCb/pInfoArea->lSize, pInfoArea->lCr/pInfoArea->lSize);
//	JASSERT(pInfoArea->lFlag > 0);
	for(x=0; x<lLabelSeed; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;
		if(pInfoArea->pBoundaryNum[x+1] <= lBoundaryThres)
			continue;
		lDist = GetColorDist(pAreaCur->lY/pAreaCur->lSize, 
			pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crRef);
		if(lDist >= *plMinDist)
			continue;	
		*plMinDist = lDist;
		lMinDistX = x;
	}
	for(x=lLabelSeed+1; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;
		if(pAreaCur->pBoundaryNum[lLabelSeed+1]<=lBoundaryThres)
			continue;
		lDist = GetColorDist(pAreaCur->lY/pAreaCur->lSize, 
			pAreaCur->lCb/pAreaCur->lSize, pAreaCur->lCr/pAreaCur->lSize, crRef);
		if(lDist >= *plMinDist)
			continue;
		*plMinDist = lDist;
		lMinDistX = x;
	}		
	return lMinDistX;
}

MLong NeighborNum(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, 
				  MLong lBoundaryThres)
{
	MLong x, lNum = 0;
	JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea + lLabelSeed;
	JASSERT(pInfoArea->lFlag > 0);
	for(x=0; x<lLabelSeed; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;
		if(pInfoArea->pBoundaryNum[x+1] <= lBoundaryThres)
			continue;
		lNum++;
	}
	for(x=lLabelSeed+1; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_AREA *pAreaCur = pInfoGraph->pInfoArea  + x;
		if(pAreaCur->lFlag <= 0)
			continue;
		if(pAreaCur->pBoundaryNum[lLabelSeed+1]<=lBoundaryThres)
			continue;
		lNum++;
	}	
	return lNum;
}	

MBool IsOnEdge(const JINFO_AREA *pInfoArea)
{
	if(pInfoArea->lEdgeNum*3 >= pInfoArea->lBoundaryNum)	//<=3
		return MTrue;	
	if(pInfoArea->lEdgeNum >= (pInfoArea->rtArea.right-pInfoArea->rtArea.left
		+ pInfoArea->rtArea.bottom-pInfoArea->rtArea.top)/2)
		return MTrue;
	return MFalse;
}


///output square of min distance 
MDWord CalcMinDist(PTLIST *ptlist1, PTLIST *ptlist2)
{
	MDWord dwMinDist = 0xFFFFFFFF;
	PTLIST *ptcur1, *ptcur2;
	JASSERT(ptlist1);
	JASSERT(ptlist2);

	ptcur1 = ptlist1;

	while (ptcur1)
	{
		ptcur2 = ptlist2;
		while(ptcur2)
		{
			MDWord dwDist = SQARE(ptcur1->pt.x-ptcur2->pt.x)+SQARE(ptcur1->pt.y-ptcur2->pt.y);
			if (dwMinDist>dwDist)
				dwMinDist = dwDist;
			ptcur2 = ptcur2->pNext;
		}
		ptcur1 = ptcur1->pNext;
	}

	return dwMinDist;
}

MRESULT BlobDistCalulate(MHandle hMemMgr, const JINFO_AREA_GRAPH* pInfoGraph,
					const JLabelData* pLabelData, MLong lLabelLine,
					MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;

	PTLIST *pPtListMem=MNull, *pCurPt;
	MLong lMaxPTNum = lWidth * lHeight;

	AllocVectMem(hMemMgr, pPtListMem, lMaxPTNum, PTLIST);
	pCurPt = pPtListMem;

	for (int i=0; i<pInfoGraph->lLabelNum; i++)
	{
		JINFO_AREA *pCurArea = pInfoGraph->pInfoArea + i;
		pCurArea->PtList.ptHead = pCurArea->PtList.pTailNode = MNull;
		pCurArea->ptNum = 0;
	}

	for (int y=0; y<lHeight; y++)
	{
		for (int x=0; x<lWidth; x++)
		{
			MLong lLabelCur = pLabelData[lLabelLine*y+x];
			if(lLabelCur<=0)
				continue;
			lLabelCur = GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabelCur-1);
			if (pInfoGraph->pInfoArea[lLabelCur].lFlag<=0)continue;
			if (x<pInfoGraph->pInfoArea[lLabelCur].rtArea.left || x>pInfoGraph->pInfoArea[lLabelCur].rtArea.right)
				continue;
			if(y<pInfoGraph->pInfoArea[lLabelCur].rtArea.top || y>pInfoGraph->pInfoArea[lLabelCur].rtArea.bottom)
				continue;			
			pCurPt->pt.x = x;
			pCurPt->pt.y = y;
			pCurPt->pNext = MNull;
			if (pInfoGraph->pInfoArea[lLabelCur].PtList.ptHead==MNull)
				{
				pInfoGraph->pInfoArea[lLabelCur].PtList.ptHead = pCurPt;
				pInfoGraph->pInfoArea[lLabelCur].PtList.pTailNode = pCurPt;
				}
			else
				{
				pInfoGraph->pInfoArea[lLabelCur].PtList.pTailNode->pNext = pCurPt;
				pInfoGraph->pInfoArea[lLabelCur].PtList.pTailNode = pCurPt;
				}
			pInfoGraph->pInfoArea[lLabelCur].ptNum++;///used to calculate lthres
			pCurPt++;
		}
	}

	for (int i=0; i<pInfoGraph->lLabelNum; i++)
	{
		JINFO_AREA *pCurArea1 = pInfoGraph->pInfoArea + i;
		if (pCurArea1->lFlag<=0)continue;

		for (int j=0; j<pInfoGraph->lLabelNum; j++)
		{
			JINFO_AREA *pCurArea2 = pInfoGraph->pInfoArea +j;
			if (pCurArea2->lFlag<=0)continue;

			if (i==j)
			{
				pCurArea1->pDistanceWithBlobs[j] = 0xFFFFFFFF;
				continue;
			}
			else	if (i>j)
			{
				pCurArea1->pDistanceWithBlobs[j] = pCurArea2->pDistanceWithBlobs[i];
			}
			else
			{
				pCurArea1->pDistanceWithBlobs[j] = CalcMinDist(pCurArea1->PtList.ptHead,pCurArea2->PtList.ptHead);
			}
		}

	}
EXT:
	FreeVectMem(hMemMgr, pPtListMem);
	return res;
}


#ifdef ENABLE_DEBUG
MVoid DrawCurrentArea(const JLabelData* pLabelData, MLong lLabelLine,
					  MLong lWidth, MLong lHeight,  
					  const JINFO_AREA* pInfoArea, MLong lLabelCur, 
					  const MTChar *strName)
{
	MUInt8* pAreaData = MNull;
	MLong lAreaLine = JMemLength(lWidth);
	const MTChar *strName2 = strName;
	MLong lLabel0 = GET_VALID_LABEL(pInfoArea, lLabelCur);
	
	MLong x, y;
	MRESULT res = LI_ERR_NONE;
	if(strName2 == MNull)
		strName2 = "..\\colorarea.bmp";
	AllocVectMem(MNull, pAreaData, lAreaLine*lHeight, MUInt8);
	SetVectMem(pAreaData, lAreaLine*lHeight, 0, MUInt8);

	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong lLabelCur = pLabelData[lLabelLine*y+x];
			if(lLabelCur<=0)
				continue;
			lLabelCur = GET_VALID_LABEL(pInfoArea, lLabelCur-1);
			if(lLabelCur != lLabel0)
				continue;
			pAreaData[lAreaLine*y+x] = 255;
		}
	}
	PrintBmpEx(pAreaData, lAreaLine, DATA_U8, lWidth, lHeight, 1, strName2);
EXT:
	FreeVectMem(MNull, pAreaData);	
}

JSTATIC MVoid _AreaInfo_Seed(const JOFFSCREEN *pImg, MLong lSeedX, MLong lSeedY, 
	MShort* pLabelData, MLong lLabelLine, JINFO_AREA *pInfoArea, 
	JINFO_AREA *pInfoArea0, MByte *pMemTmp, MLong lMemBytes);
MVoid CheckAreaGraph(const JOFFSCREEN *pImg,
					 const JINFO_AREA_GRAPH *pInfoGraph, 
					 JLabelData* pLabelData, MLong lLabelLine)
{
	JINFO_AREA infoArea = {0};
	MLong lWidth = pImg->dwWidth, lHeight = pImg->dwHeight;
	MLong x, y, i;
	MRESULT res = LI_ERR_NONE;
	MByte *pMemTmp = MNull;
	MLong lMemBytes = lWidth*lHeight*4;	
	
	AllocVectMem(MNull,infoArea.pBoundaryNum, pInfoGraph->lLabelNum, MLong);
	AllocVectMem(MNull,infoArea.pGradientSum, pInfoGraph->lLabelNum, MLong);
	AllocVectMem(MNull, pMemTmp, lMemBytes, MByte);
	
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			JINFO_AREA *pInfoArea = MNull;
			MLong lLabel = pLabelData[lLabelLine*y+x];
			if(lLabel <= 0)
				continue;
			
			lLabel = GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabel-1);
			pInfoArea = pInfoGraph->pInfoArea + lLabel;
// 			if(pInfoArea->lFlag ==0)
// 				continue;
			JASSERT(lLabel < pInfoGraph->lLabelNum);
			JASSERT(pInfoArea->lFlag==0 || pInfoArea->lFlag==lLabel+1);
			_AreaInfo_Seed(pImg, x, y, pLabelData, lLabelLine,
				&infoArea, pInfoGraph->pInfoArea, pMemTmp, lMemBytes);
			JASSERT(pInfoArea->lSize == infoArea.lSize);
			JASSERT(pInfoArea->lY == infoArea.lY);
			JASSERT(pInfoArea->lCb == infoArea.lCb);
			JASSERT(pInfoArea->lCr == infoArea.lCr);
			JASSERT(pInfoArea->rtArea.left == infoArea.rtArea.left);
			JASSERT(pInfoArea->rtArea.right == infoArea.rtArea.right);
			JASSERT(pInfoArea->rtArea.top == infoArea.rtArea.top);
			JASSERT(pInfoArea->rtArea.bottom == infoArea.rtArea.bottom);
//			JASSERT(pInfoArea->lEdgeNum == infoArea.lEdgeNum);
			JASSERT(pInfoArea->lBoundaryNum == infoArea.lBoundaryNum);	
			for(i=1; i<=lLabel; i++)
			{
				if(pInfoGraph->pInfoArea[i-1].lFlag<=0)
					continue;
				JASSERT(pInfoArea->pBoundaryNum[i] == infoArea.pBoundaryNum[i]);
				JASSERT(pInfoArea->pGradientSum[i] == infoArea.pGradientSum[i]);
			}
		}
	}

	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong lLabelCur = pLabelData[lLabelLine*y+x];
			if(lLabelCur>0)
			{
				JASSERT(pInfoGraph->pInfoArea[GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabelCur-1)].lFlag==0);
				continue;
			}
			if(-lLabelCur > pInfoGraph->lLabelNum)
				continue;
			/*if(lLabelCur < 0)
				pLabelData[lLabelLine*y+x]=(JLabelData)-lLabelCur/3;*/
			pLabelData[lLabelLine*y+x] = (JLabelData)-pLabelData[lLabelLine*y+x];
		}
	}
	
EXT:
	FreeVectMem(MNull, pMemTmp);
	FreeVectMem(MNull, infoArea.pGradientSum);
	FreeVectMem(MNull, infoArea.pBoundaryNum);
}

MVoid _AreaInfo_Seed(const JOFFSCREEN *pImg, MLong lSeedX, MLong lSeedY, 
	MShort* pLabelData, MLong lLabelLine, JINFO_AREA *pInfoArea, 
	JINFO_AREA *pInfoArea0, MByte *pMemTmp, MLong lMemBytes)
{
	MLong lWidth = pImg->dwWidth;
	MLong lHeight = pImg->dwHeight;
	MLong lSize = 0;
	MLong lBoundaryNum = 0, lEdgeNum = 0;
	MLong lY=0, lCb=0, lCr=0;
	JPoint *pPtStack = (JPoint*)pMemTmp;
	MLong lPtsNum=0,  lStacksNum = lMemBytes/sizeof(JPoint);
	MRECT rtArea;
	MCOLORREF cr;

	MLong lLabelSeed = GET_VALID_LABEL(pInfoArea0, pLabelData[lSeedX+lLabelLine*lSeedY]-1);	
	SetVectMem(pInfoArea->pBoundaryNum, lLabelSeed+1, 0, MLong);
	SetVectMem(pInfoArea->pGradientSum, lLabelSeed+1, 0, MLong);
	pPtStack[lPtsNum].x = (MShort)lSeedX;
	pPtStack[lPtsNum].y = (MShort)lSeedY;
	lPtsNum++;
	rtArea.left = rtArea.right = lSeedX;
	rtArea.top = rtArea.bottom = lSeedY;
	do 
	{
		JPoint ptCur = pPtStack[--lPtsNum];
		MLong lLabelCur = pLabelData[ptCur.x+lLabelLine*ptCur.y];
		MLong i;
		if(lLabelCur <= 0)
			continue;
		JASSERT(lLabelCur == -(lLabelSeed+1) || 
			GET_VALID_LABEL(pInfoArea0, lLabelCur-1)==lLabelSeed);

		pLabelData[ptCur.x+lLabelLine*ptCur.y] = (MShort)-(lLabelSeed+1);
		lSize ++;
		cr = ImgGetPixel(pImg, ptCur.x, ptCur.y);
		lY += PIXELVAL_1(cr);
		lCb += PIXELVAL_2(cr);
		lCr += PIXELVAL_3(cr);
		if(rtArea.left > ptCur.x)			rtArea.left = ptCur.x;
		else if(rtArea.right < ptCur.x)		rtArea.right = ptCur.x;
		if(rtArea.top > ptCur.y)			rtArea.top = ptCur.y;
		else if(rtArea.bottom < ptCur.y)	rtArea.bottom = ptCur.y;

		if(lPtsNum >= lStacksNum-4)
			continue;
		for(i=0; i<4; i++)
		{
			MLong x = ptCur.x, y = ptCur.y;
			if(i==0)
			{
				if(++x<lWidth)
					goto STACK_PUSH;
			}
			else if(i==1)
			{
				if(--x>=0)
					goto STACK_PUSH;
			}
			else if(i==2)
			{
				if(++y<lHeight)
					goto STACK_PUSH;
			}
			else if(i==3)
			{
				if(--y>=0)
					goto STACK_PUSH;
			}
			lBoundaryNum++;
			lEdgeNum++;
			continue;			
STACK_PUSH:
			lLabelCur = pLabelData[x+lLabelLine*y];
			if(lLabelCur == -(lLabelSeed+1))
				continue;
			if(lLabelCur < 0) 
				lLabelCur = -lLabelCur;
			lLabelCur = GET_VALID_LABEL(pInfoArea0, lLabelCur-1);
			if(lLabelCur==lLabelSeed)
			{				
				pPtStack[lPtsNum].x = (MShort)x;
				pPtStack[lPtsNum].y = (MShort)y;
				lPtsNum++;
				continue;
			}
			lBoundaryNum++;				
			if(lLabelCur > lLabelSeed)
				continue;
			
			JASSERT(lLabelCur < lLabelSeed);
			{
				MCOLORREF crTmp = ImgGetPixel(pImg, x, y);
				pInfoArea->pGradientSum[lLabelCur+1] += 
					ABS(PIXELVAL_1(crTmp)-PIXELVAL_1(cr)) 
					+ ABS(PIXELVAL_2(crTmp)-PIXELVAL_2(cr))*2 
					+ ABS(PIXELVAL_3(crTmp)-PIXELVAL_3(cr))*2 ;
				pInfoArea->pBoundaryNum[lLabelCur+1]++;
			}
		}
	} while(lPtsNum > 0);

	if(pInfoArea != MNull)
	{
		pInfoArea->lBoundaryNum = lBoundaryNum;
		pInfoArea->lEdgeNum = lEdgeNum;
		pInfoArea->lSize = lSize;
		pInfoArea->lY	= lY;
		pInfoArea->lCb = lCb;
		pInfoArea->lCr = lCr;
		pInfoArea->rtArea = rtArea;
		pInfoArea->rtArea.right++, pInfoArea->rtArea.bottom++;
	}	
}
#endif	//ENABLE_DEBUG