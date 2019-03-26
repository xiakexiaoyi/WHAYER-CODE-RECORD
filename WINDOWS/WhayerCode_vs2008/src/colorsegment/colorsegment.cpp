#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "litrimfun.h"
#include "liimage.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "limath.h"
#include "limask.h"
#include "areagraph.h"

#include "colorsegment.h"
//#include "skinarea.h"

#define COLOR_DIST				(MAX_COLOR_DIST/2)

JSTATIC MVoid   _ColorSegment_Seed(const JOFFSCREEN* pImg, 
	JLabelData* pLabelData, MLong lLabelLine, MLong lNewLabel, 
	MLong lSeedX, MLong lSeedY, JINFO_AREA *pInfoArea, 
	JPoint* pPtsStack, MLong lPtsNum);
JSTATIC MLong  _ChangeLabel(JLabelData* pLabelData, MLong lLabelLine, 
	MLong lWidth, MLong lHeight, const MRECT* pRtArea, 
	MLong lLabelOld, MLong lLabelNew);
//////////////////////////////////////////////////////////////////////////
MLong	GetColorDist(MLong cr1, MLong cr2, MLong cr3, MCOLORREF crCenter)
{	
	MLong tmp = 0, dist = 0;
	tmp = cr1 - PIXELVAL_1(crCenter);	
	//dist = SQARE(tmp)>>3;
	dist = SQARE(tmp)>>2;
	tmp = cr2 - PIXELVAL_2(crCenter);
	dist += SQARE(tmp)<<2;
	tmp = cr3 - PIXELVAL_3(crCenter);
	dist += SQARE(tmp)<<2;	
	if(cr1 > PIXELVAL_1(crCenter))
		cr1 = PIXELVAL_1(crCenter); 
	return (dist*128) / (cr1+16);
//	return (dist*128*128) / (cr1*PIXELVAL_1(crCenter)+16);
}
//////////////////////////////////////////////////////////////////////////
//创建 区域分割的图
//根据颜色一致性程度来分割
MRESULT CreateAreaGraphByImg(MHandle hMemMgr, const JOFFSCREEN* pImg, 
					 JLabelData* pLabelData, MLong lLabelLine, 
					 JINFO_AREA_GRAPH *pInfoGraph)
{
	MLong lWidth = pImg->dwWidth, lHeight = pImg->dwHeight;
	MLong x, y;
	MLong lCr=0;
	MRESULT res = LI_ERR_NONE;
	JPoint *pPtsStack = MNull;
	MLong lPtsNum = lWidth*lHeight;

	JINFO_AREA_GRAPH infoGraph = {0};
	MLong lMaxFlag = MIN(lWidth*lHeight, 1024*16);
	lMaxFlag = MIN(lMaxFlag, 0x7FFF/3);

	AllocVectMem(hMemMgr, pPtsStack, lPtsNum, JPoint);
	SetVectMem(pLabelData, lLabelLine*lHeight, 0, JLabelData);

	AllocVectMem(hMemMgr, infoGraph.pInfoArea, lMaxFlag, JINFO_AREA);
	SetVectMem(infoGraph.pInfoArea, lMaxFlag, 0, JINFO_AREA);
	infoGraph.lLabelNum = 0;
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			JINFO_AREA *pInfoArea = infoGraph.pInfoArea + infoGraph.lLabelNum;		
			MCOLORREF crRef = ImgGetPixel(pImg, x, y);
		    lCr = PIXELVAL_3(crRef);
			if(pLabelData[lLabelLine*y+x] > 0)
				continue;
			if(infoGraph.lLabelNum >= lMaxFlag-1)
				continue;
			if(lCr<120 )
				continue;

			if(pInfoArea->pBoundaryNum == MNull)
			{
				AllocVectMem(hMemMgr, pInfoArea->pBoundaryNum, infoGraph.lLabelNum+1, MLong);
				AllocVectMem(hMemMgr, pInfoArea->pGradientSum, infoGraph.lLabelNum+1, MLong);
			}
			_ColorSegment_Seed(pImg, pLabelData, lLabelLine, infoGraph.lLabelNum+1, x, y, 
				pInfoArea, pPtsStack, lPtsNum);
			
		//	PrintChannel(pLabelData, lLabelLine, DATA_I16, lWidth,lHeight, 1, 0);
			//area size大于35 直接单独作为一块
			if(pInfoArea->lSize >= MIN_AREA_SIZE(lWidth*lHeight))
			{
				infoGraph.lLabelNum++;
				continue;
			}
			else
			{
				MLong lMinDist = 0x7FFFFFFF, lMinI = -1;
				MCOLORREF crRef = PIXELVAL(pInfoArea->lY/pInfoArea->lSize, pInfoArea->lCb/pInfoArea->lSize, pInfoArea->lCr/pInfoArea->lSize);
				MLong i;
				//PrintChannel(pLabelData, lLabelLine, DATA_I16, lWidth,lHeight, 1, 0);
				for(i=0; i<infoGraph.lLabelNum; i++)
				{
					MLong lDist;
					JINFO_AREA *pInfoCur= infoGraph.pInfoArea + i;
					if(pInfoArea->pBoundaryNum[i+1]<=VALID_NEIGHBOR_BOUNDARY(pInfoArea->lBoundaryNum))
						continue;
					lDist = GetColorDist(pInfoCur->lY/pInfoCur->lSize, pInfoCur->lCb/pInfoCur->lSize, pInfoCur->lCr/pInfoCur->lSize, crRef);
					if(lDist >= lMinDist)
						continue;
					///颜色足够接近 size足够小 连接紧密 进行合并
					if(lDist<=MAX_COLOR_DIST 
						|| pInfoArea->lSize < MIN_AREA_SIZE(lWidth*lHeight)/4
						|| pInfoArea->pBoundaryNum[i+1]*8>=pInfoArea->lBoundaryNum*7)
					{						
						lMinDist = lDist;
						lMinI = i;
					}
				}
				if(lMinI < 0)
				{
					infoGraph.lLabelNum++;
					continue;
				}
				i = _ChangeLabel(pLabelData, lLabelLine, lWidth, lHeight, 
					&pInfoArea->rtArea, infoGraph.lLabelNum+1, lMinI+1);
				JASSERT(i==pInfoArea->lSize);	
				MergeArea(infoGraph.pInfoArea, infoGraph.lLabelNum, lMinI, infoGraph.lLabelNum+1);
				//PrintChannel(pLabelData, lLabelLine, DATA_I16, lWidth,lHeight, 1, 0);
			}
		}	//x
	}	//y
	for(y=0; y<lHeight; y++)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong lLabel = pLabelData[lLabelLine*y+x];
			if(lLabel<0)
			{
				pLabelData[lLabelLine*y+x]=0;
			}
		}
	}
EXT:

//	PrintChannel(pLabelData, lLabelLine, DATA_I16, lWidth,lHeight, 1, 0);
	if(pInfoGraph == MNull)
		ReleaseAreaGraph(hMemMgr, &infoGraph);
	else
		*pInfoGraph = infoGraph;
	FreeVectMem(hMemMgr, pPtsStack);
	return res;
}

MLong  _ChangeLabel(JLabelData* pLabelData, MLong lLabelLine, 
	MLong lWidth, MLong lHeight, const MRECT* pRtArea, 
	MLong lLabelOld, MLong lLabelNew)
{
	MLong x, y;
	MLong left=pRtArea->left, right = pRtArea->right;
	MLong top=pRtArea->top, bottom = pRtArea->bottom;
	MLong lSize=0;
	if(left > 0)	left--;
	if(top > 0)		top--;
	if(right < lWidth)	right++;
	if(bottom < lHeight)bottom++;
	for(y=top; y<bottom; y++)
	{
		for(x=left; x<right; x++)
		{
			MLong lLabelCur = pLabelData[lLabelLine*y+x];
			if(lLabelCur==lLabelOld)
			{
				pLabelData[lLabelLine*y+x] = (JLabelData)lLabelNew;
				lSize++;
			}
			else if(lLabelCur<0)
			{
				pLabelData[lLabelLine*y+x] = 0;
			}
		}
	}
	return lSize;
}

MVoid   _ColorSegment_Seed(const JOFFSCREEN* pImg, 
	JLabelData* pLabelData, MLong lLabelLine, MLong lNewLabel, 
	MLong lSeedX, MLong lSeedY, JINFO_AREA *pInfoArea, 
	JPoint* pPtsStack, MLong lPtsNum)
{
	MLong lWidth = pImg->dwWidth, lHeight = pImg->dwHeight;
	MLong lYSum=0, lCbSum=0, lCrSum=0;
	MLong lPtsCur = 0;
	MLong lSize = 0;	
	MLong lBoundaryNum = 0, lEdgeNum = 0;
	MRECT rtArea;
	
	if(pInfoArea != MNull)
	{
		SetVectMem(pInfoArea->pBoundaryNum, lNewLabel, 0, MLong);
		SetVectMem(pInfoArea->pGradientSum, lNewLabel, 0, MLong);
	}
	pPtsStack[lPtsCur].x = (MShort)lSeedX;
	pPtsStack[lPtsCur].y = (MShort)lSeedY;
	lPtsCur = 1;
	rtArea.left = rtArea.right = lSeedX;
	rtArea.top = rtArea.bottom = lSeedY;
//	pLabelData[lLabelLine*lSeedY+lSeedX] = 0;
	JASSERT(lNewLabel < 0x7FFF/3);

	while(lPtsCur > 0)
	{
		JPoint ptCur = pPtsStack[--lPtsCur];
		MLong i;
		MLong lLabelCur = pLabelData[lLabelLine*ptCur.y+ptCur.x];
		MCOLORREF crRef = ImgGetPixel(pImg, ptCur.x, ptCur.y);
		JASSERT(lLabelCur==lNewLabel || lLabelCur<=0);
		//Right label
		if(lLabelCur == lNewLabel)
			continue;
		if(-lLabelCur>=lNewLabel*3 && pInfoArea != MNull) //Wrong Boundary
		{
			JASSERT(-lLabelCur-lNewLabel*3<=2);
			lBoundaryNum -= -lLabelCur-lNewLabel*3+1;	
		}
		//New label
		pLabelData[lLabelLine*ptCur.y+ptCur.x] = (JLabelData)lNewLabel;
		lYSum += PIXELVAL_1(crRef);
		lCbSum += PIXELVAL_2(crRef);
		lCrSum += PIXELVAL_3(crRef);
		lSize++;
		if(rtArea.left > ptCur.x)			rtArea.left = ptCur.x;
		else if(rtArea.right < ptCur.x)		rtArea.right = ptCur.x;
		if(rtArea.top > ptCur.y)			rtArea.top = ptCur.y;
		else if(rtArea.bottom < ptCur.y)	rtArea.bottom = ptCur.y;
		if(lPtsCur+4 > lPtsNum)
			continue;
		for(i=0; i<4; i++)
		{
			MLong tx = ptCur.x, ty = ptCur.y;
			MLong lY = lYSum/lSize, lCb = lCbSum/lSize, lCr = lCrSum/lSize;
			//MLong lY=PIXELVAL_1(ImgGetPixel(pImg, lSeedX, lSeedY)), lCb=PIXELVAL_2(ImgGetPixel(pImg, lSeedX, lSeedY)),lCr=PIXELVAL_3(ImgGetPixel(pImg, lSeedX, lSeedY));
			if(i==0)
			{
				if(--tx>=0)
					goto STACK_PUSH;
			}
			else if(i==1)
			{
				if(++tx<lWidth)
					goto STACK_PUSH;
			}
			else if(i==2)
			{
				if(--ty>=0)
					goto STACK_PUSH;
			}
			else if(i==3)
			{
				if(++ty<lHeight)
					goto STACK_PUSH;
			}
			//Edge
			if(pInfoArea != MNull)
			{
				lBoundaryNum++;
				lEdgeNum++;
			}
			continue;
STACK_PUSH:			
			lLabelCur = pLabelData[lLabelLine*ty+tx];	
			if(lLabelCur==lNewLabel)	//Right Label
				continue;			
			if(lLabelCur<=0)			//No label or is boundary
			{
				//当前像素值与平均值差的平方和
				if(GetColorDist(lY, lCb, lCr, ImgGetPixel(pImg, tx, ty)) <= COLOR_DIST)
				{
					//Push
					pPtsStack[lPtsCur].x = (MShort)tx;
					pPtsStack[lPtsCur].y = (MShort)ty;
					lPtsCur ++;
				}
				else if(pInfoArea != MNull)	//Boundary
				{			
					lBoundaryNum++;	
					if(-lLabelCur<lNewLabel*3)
						pLabelData[lLabelLine*ty+tx] = (JLabelData)(-lNewLabel*3);
					else if(-lLabelCur<lNewLabel*3+2)
						pLabelData[lLabelLine*ty+tx] --;
					JASSERT(pLabelData[lLabelLine*ty+tx] < 0);
				}
			}
			else if(pInfoArea != MNull)	//Labeled
			{
				MCOLORREF crTmp = ImgGetPixel(pImg, tx, ty);
				JASSERT(lLabelCur>0 && lLabelCur<lNewLabel);
				pInfoArea->pGradientSum[lLabelCur] += 
					ABS(PIXELVAL_1(crTmp)-PIXELVAL_1(crRef)) 
					+ ABS(PIXELVAL_2(crTmp)-PIXELVAL_2(crRef))*2 
					+ ABS(PIXELVAL_3(crTmp)-PIXELVAL_3(crRef))*2 ;				
				pInfoArea->pBoundaryNum[lLabelCur]++;
				lBoundaryNum++;				
			}
		}//i
	}//lPtsCur

	if(pInfoArea != MNull)
	{
		pInfoArea->lFlag = lNewLabel;
		pInfoArea->lBoundaryNum = lBoundaryNum;
		JASSERT(lBoundaryNum > 0);
		pInfoArea->lEdgeNum = lEdgeNum;
		pInfoArea->lSize = lSize;
		JASSERT(lSize > 0);
		pInfoArea->lY	= lYSum;
		pInfoArea->lCb = lCbSum;
		pInfoArea->lCr = lCrSum;
		pInfoArea->rtArea = rtArea;
		pInfoArea->rtArea.right++, pInfoArea->rtArea.bottom++;
	}
}

MVoid   ReleaseAreaGraph(MHandle hMemMgr, JINFO_AREA_GRAPH *pInfoGraph)
{
	MLong i;
	if(pInfoGraph==MNull || pInfoGraph->pInfoArea==MNull)
		return;
	for(i=0; i<=pInfoGraph->lLabelNum; i++)
	{
		FreeVectMem(hMemMgr, pInfoGraph->pInfoArea[i].pBoundaryNum);
		FreeVectMem(hMemMgr, pInfoGraph->pInfoArea[i].pGradientSum);

		//new added parameters
		FreeVectMem(hMemMgr, pInfoGraph->pInfoArea[i].pDistanceWithBlobs);
	}
	FreeVectMem(hMemMgr, pInfoGraph->pInfoArea);
	pInfoGraph->lLabelNum = 0;
}

MVoid	ShowImgByMask(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
						JMASK *pMskImg, MBool bShowBoundary)
{
	MDWord x, y;
	MLong lStepX = pImgSrc->dwWidth/pMskImg->lWidth;
	MLong lStepY = pImgSrc->dwHeight/pMskImg->lHeight;
	if(pImgSrc->dwWidth-pImgRlt->dwWidth!=0 || pImgSrc->dwHeight-pImgRlt->dwHeight!=0)
		return;
	ImgCpy(pImgSrc, pImgRlt);
	for(y=0; y<pImgSrc->dwHeight-1; y++)
	{
		for(x=0; x<pImgSrc->dwWidth-1; x++)
		{
			MLong lMaskLine = pMskImg->lMaskLine;
			JMaskData *pMskData = pMskImg->pData+lMaskLine*(y/lStepY)+x/lStepX;
			MLong lMskVal = pMskData[0];			
// 			if(lMskVal!=0)
// 				ImgSetPixel(pImgRlt, x, y, PIXELVAL(0, 128, 128));
			if(lMskVal==0)
				ImgSetPixel(pImgRlt, x, y, PIXELVAL(0, 128, 128));
			if(!bShowBoundary)
				continue;
			if(pMskData[1] != lMskVal)
				ImgSetPixel(pImgRlt, x, y, PIXELVAL(255, 255, 0));
			else if(pMskData[lMaskLine] != lMskVal)
				ImgSetPixel(pImgRlt, x, y, PIXELVAL(255, 255, 0));
		}
	}
}

MVoid	ShowLabelMask(const JINFO_AREA_GRAPH* pInfoGraph, 
					  const JLabelData* pLabelData, MLong lLabelLine, 
					  MLong lWidth, MLong lHeight, JMASK *pMask)
{
	MLong x, y;
	MLong lStepX = 1, lStepY = 1;
	JMASK mskSkin = {0};	
	if(pMask==MNull||pMask->lWidth<=0||pMask->lHeight<=0)
		MaskCreate(MNull, &mskSkin, lWidth, lHeight);
	else
		mskSkin = *pMask;
	lStepX = lWidth/mskSkin.lWidth;
	lStepY = lHeight/mskSkin.lHeight;
	MaskSet(&mskSkin, 0);
	for(y=0; y<mskSkin.lHeight; y++)
	{
		for(x=0; x<mskSkin.lWidth; x++)
		{
			MLong lLabelCur = pLabelData[lLabelLine*y*lStepY+x*lStepX];
			if(lLabelCur<=0)
				continue;
			lLabelCur = GET_VALID_LABEL(pInfoGraph->pInfoArea, lLabelCur-1);
			if(pInfoGraph->pInfoArea[lLabelCur].lFlag==0)
				continue;
			lLabelCur = pInfoGraph->pInfoArea[lLabelCur].lFlag%255 + 1;
			mskSkin.pData[mskSkin.lMaskLine*y+x] = (JMaskData)lLabelCur;
		}
	}
	PrintBmp(mskSkin.pData, mskSkin.lMaskLine, DATA_U8, mskSkin.lWidth, mskSkin.lHeight, 1);
	if(pMask==MNull||pMask->lWidth<=0||pMask->lHeight<=0)
		MaskRelease(MNull, &mskSkin);
}
