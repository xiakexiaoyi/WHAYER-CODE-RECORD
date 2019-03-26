#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "limask.h"

#include "litrimfun.h"
#include "lierrdef.h"
#include "lidebug.h"
#include "limem.h"
#include "limath.h"
#include "liimage.h"

MVoid	RectTrim(MRECT *pRect, MLong lLeft, MLong lTop, MLong lRight, MLong lBottom)
{
	if(pRect->left < lLeft)		pRect->left = lLeft;
	if(pRect->top < lTop)		pRect->top = lTop;
	if(pRect->right > lRight)	pRect->right = lRight;
	if(pRect->bottom > lBottom)	pRect->bottom = lBottom;
}
MRECT	RectUnion(const MRECT* pRect1, const MRECT* pRect2)
{
	MRECT rt;
	rt.left = MIN(pRect1->left, pRect2->left);
	rt.top = MIN(pRect1->top, pRect2->top);
	rt.right = MAX(pRect1->right, pRect2->right);
	rt.bottom = MAX(pRect1->bottom, pRect2->bottom);
	return rt;
}
MRECT	RectIntersect(const MRECT* pRect1, const MRECT* pRect2)
{
	MRECT rt;
	rt.left = MAX(pRect1->left, pRect2->left);
	rt.top = MAX(pRect1->top, pRect2->top);
	rt.right = MIN(pRect1->right, pRect2->right);
	rt.bottom = MIN(pRect1->bottom, pRect2->bottom);
	return rt;
}
//////////////////////////////////////////////////////////////////////////
MVoid Mask2Img(const JMASK* pMask, PJOFFSCREEN pImg)
{
	pImg->dwWidth = pMask->lWidth;
	pImg->dwHeight = pMask->lHeight;
	pImg->fmtImg = FORMAT_GRAY;
	pImg->pixelArray.chunky.dwImgLine = pMask->lMaskLine;
	pImg->pixelArray.chunky.pPixel = pMask->pData;
}
MVoid	Img2Mask(const JOFFSCREEN* pImg, JMASK* pMask)
{
	JOFFSCREEN img = *pImg;
	if(pImg->fmtImg != FORMAT_GRAY)
	{
		SetVectMem(pMask, 1, 0, JOFFSCREEN);
		return;
	}

	ImgChunky2Plannar(&img);	
	pMask->lWidth = img.dwWidth;
	pMask->lHeight = img.dwHeight;
	pMask->lMaskLine = img.pixelArray.planar.dwImgLine[0];
	pMask->pData = (MByte*)img.pixelArray.planar.pPixel[0];
}

MRESULT MaskCreate(MHandle hMemMgr, LPJMASK pMask, 
				   MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;
	AllocVectMem(hMemMgr, pMask->pData, JMemLength(lWidth)*lHeight, JMaskData);	
	pMask->lWidth = lWidth;
	pMask->lHeight = lHeight;
	pMask->lMaskLine = JMemLength(lWidth);
	pMask->rcMask.left = pMask->rcMask.right = pMask->rcMask.top
		= pMask->rcMask.bottom = 0;
EXT:
	return res;
}
MVoid	MaskRelease(MHandle hMemMgr, LPJMASK pMask)
{
	if(pMask == MNull)
		return;
	FreeVectMem(hMemMgr, pMask->pData);
}

MRESULT	MaskCpy(const JMASK* pMaskSrc, LPJMASK pMaskRlt)
{
	if(pMaskSrc->lWidth!=pMaskRlt->lWidth || pMaskSrc->lHeight!=pMaskRlt->lHeight)
		return LI_ERR_UNKNOWN;
	if(pMaskSrc==pMaskRlt || pMaskSrc->pData==pMaskRlt->pData)
		return LI_ERR_NONE;
	CpyImgMem(pMaskRlt->pData, pMaskRlt->lMaskLine, 
		pMaskSrc->pData, pMaskSrc->lMaskLine, pMaskRlt->lWidth, pMaskRlt->lHeight, 
		JMaskData);
	pMaskRlt->rcMask = pMaskSrc->rcMask;
	return LI_ERR_NONE;
}

MVoid	MaskSet(LPJMASK pMask, JMaskData mskVal)
{
	JASSERT(sizeof(JMaskData) == 1);
//	SetVectMem(pMask->pData, pMask->lHeight*pMask->lMaskLine, mskVal, JMaskData);
    SetImgMem(pMask->pData, pMask->lMaskLine, pMask->lWidth, pMask->lHeight, mskVal, JMaskData);
}
MVoid MaskRange(JMASK *pMask)
{
    MLong x, y;
    MLong left = 0x7FFFFFFF, right = -1,
        top = 0x7FFFFFFF, bottom = -1;
    MBool bAllZero = MTrue;

    if (pMask==MNull)
        return;

    for (y=0; y<pMask->lHeight; y++)
    {
        MByte *pCur = pMask->pData + pMask->lMaskLine*y;
        for (x=0; x<pMask->lWidth; x++)
        {
            if (pCur[x]==0)
                continue;
            bAllZero = MFalse;
            if (x>=right)    right = x+1;
            if (x<left)     left = x;
            if (y>=bottom)   bottom = y+1;
            if (y<top)      top = y;
        }
    }
    if (bAllZero)
    {
        pMask->rcMask.left = pMask->rcMask.right = 0;
        pMask->rcMask.top = pMask->rcMask.bottom = 0;
    }
    else
    {
        pMask->rcMask.left = left, pMask->rcMask.right = right;
        pMask->rcMask.top = top, pMask->rcMask.bottom = bottom;
    }
	pMask->rcMask.right = MIN(pMask->rcMask.right, pMask->lWidth-1);
	pMask->rcMask.bottom = MIN(pMask->rcMask.bottom, pMask->lHeight-1);
}
JPoint	MaskLineRange(JMaskData* pLineData, MLong lWidth, MLong lMskThreshold)
{
	MLong x;
	JPoint ptRange = {0};
	ptRange.x = (MShort)lWidth;
	for(x=0; x<lWidth; x++)
	{
		if(pLineData[x] >= lMskThreshold)
		{
			ptRange.x = (MShort)x;
			break;
		}		
	}
	for(x=lWidth-1; x>=0; x--)
	{
		if(pLineData[x] >= lMskThreshold)
		{
			ptRange.y = (MShort)(x+1);
			break;
		}
	}
	return ptRange;
}
#ifndef TRIM_REDUNDANCE
MLong	MaskSize(const JMASK* pMask, FNMASK_COMPARE fnCompare, MLong lVal0)
{
	MLong x, y;
	JMaskData *pMaskData = pMask->pData+pMask->rcMask.top*pMask->lMaskLine
		+ pMask->rcMask.left;
	MLong lMaskExt = pMask->lMaskLine - (pMask->rcMask.right-pMask->rcMask.left);
	MLong lSize = 0;
	for(y=pMask->rcMask.bottom-pMask->rcMask.top; y!=0; y--, pMaskData+=lMaskExt)
	{
		for(x=pMask->rcMask.right-pMask->rcMask.left; x!=0; x--, pMaskData++)
		{
			if(fnCompare==MNull ? *pMaskData==lVal0 : fnCompare(*pMaskData, lVal0))
				lSize++;
		}
	}
	return lSize;
}

JPoint	MaskCenter(const JMASK *pMask)
{
	MLong lSumX=0, lSumY=0, lSumW=0;
	JMaskData *pMaskData = pMask->pData+pMask->rcMask.top*pMask->lMaskLine;
	MLong x, y;
	for(y=pMask->rcMask.top; y<pMask->rcMask.bottom; y++, pMaskData+=pMask->lMaskLine)
	{
		for(x=pMask->rcMask.left; x<pMask->rcMask.right; x++)
		{
			MLong lMskVal = pMaskData[x];
			if(lMskVal==0)
				continue;
			lSumW += lMskVal;
			lSumX += x*lMskVal;
			lSumY += y*lMskVal;
		}
	}

	{
		JPoint ptCenter = {0};
		if(lSumW > 0)
		{
			lSumX /= lSumW;
			lSumY /= lSumW;
			ptCenter.x = (MShort)lSumX;
			ptCenter.y = (MShort)lSumY;
		}
		return ptCenter;
	}
}
#endif//TRIM_REDUNDANCE

MVoid	MaskSub(const JMASK* pMaskLeft, const JMASK* pMaskRight, JMASK *pMaskRlt)
{
	MLong x, y;
	JMaskData* pLeftData = pMaskLeft->pData + pMaskLeft->rcMask.left
		+ pMaskLeft->lMaskLine*pMaskLeft->rcMask.top;
	JMaskData* pRightData = pMaskRight->pData + pMaskLeft->rcMask.left
		+ pMaskRight->lMaskLine*pMaskLeft->rcMask.top;
	JMaskData* pRltData = pMaskRlt->pData + pMaskLeft->rcMask.left
		+ pMaskRlt->lMaskLine*pMaskLeft->rcMask.top;
	MLong lWidth = pMaskLeft->rcMask.right - pMaskLeft->rcMask.left;
	pMaskRlt->rcMask = pMaskLeft->rcMask;
	MaskCpy(pMaskLeft, pMaskRlt);
	for(y=pMaskLeft->rcMask.bottom-pMaskLeft->rcMask.top; y!=0; y--, 
		pLeftData+=pMaskLeft->lMaskLine, pRightData+=pMaskRight->lMaskLine, 
		pRltData+=pMaskRlt->lMaskLine)
	{
		for(x=0; x<lWidth; x++)
		{
			MLong l, r = pRightData[x];
			if(r==0)
				continue;
			
			l = pLeftData[x];
			if(l > r)
				pRltData[x] = (JMaskData)(l-r);
			else
				pRltData[x] = 0;
		}
	}
}

MRESULT MaskDilate(MHandle hMemMgr, const JMASK *pMskSrc, JMASK *pMskDst, MLong Ksize)
{
    MRESULT res = LI_ERR_NONE;
	MLong k2 = Ksize/2;
	MLong kmax= k2+1;
	MLong x, y;
	MLong ymin = k2, ymax = pMskSrc->lHeight-kmax,
		  xmin = k2, xmax = pMskSrc->lWidth-kmax;
    
    JMASK mskTmp={0};    
    if (pMskSrc == pMskDst)
        MaskCreate(hMemMgr, &mskTmp, pMskSrc->lWidth, pMskSrc->lHeight);
    else
        mskTmp = *pMskDst;
	MaskCpy(pMskSrc, &mskTmp);

	for(y=ymin; y<ymax; y++)
	{
		MByte *pMaskDataSrc = pMskSrc->pData + y*pMskSrc->lMaskLine + xmin,
			  *pMaskDataDst = mskTmp.pData + y*mskTmp.lMaskLine + xmin;
		for(x=xmin; x<xmax; x++, pMaskDataSrc++, pMaskDataDst++)
		{
			MByte curMax = 0;
			MLong j, k;

			for(j=-k2;j<kmax;j++)
			{
				MByte *pMaskData1 = pMaskDataSrc + j*pMskSrc->lMaskLine;
				for(k=-k2;k<kmax;k++)
				{
					if (pMaskData1[k] > curMax)
						curMax = pMaskData1[k];
				}
			}
			*pMaskDataDst = (MByte)curMax;
		}
	}
    if (pMskSrc==pMskDst)
    {
        MaskCpy(&mskTmp, pMskDst);
        MaskRelease(hMemMgr, &mskTmp);
    }
    return res;
}
MRESULT MaskErode(MHandle hMemMgr, const JMASK *pMskSrc, JMASK *pMskDst, MLong Ksize)
{
    MRESULT res = LI_ERR_NONE;
	MLong k2 = Ksize/2;
	MLong kmax= k2+1;
	MLong x, y;
	MLong ymin = k2, ymax = pMskSrc->lHeight-kmax,
		  xmin = k2, xmax = pMskSrc->lWidth-kmax;

    JMASK mskTmp={0};    
    if (pMskSrc == pMskDst)
        MaskCreate(hMemMgr, &mskTmp, pMskSrc->lWidth, pMskSrc->lHeight);
    else
        mskTmp = *pMskDst;
	MaskCpy(pMskSrc, &mskTmp);

	for(y=ymin; y<ymax; y++)
	{
		MByte *pMaskDataSrc = pMskSrc->pData + y*pMskSrc->lMaskLine + xmin,
			  *pMaskDataDst = mskTmp.pData + y*mskTmp.lMaskLine + xmin;
		for(x=xmin; x<xmax; x++, pMaskDataSrc++, pMaskDataDst++)
		{
			MByte curMin = 255;
			MLong j, k;

			for(j=-k2;j<kmax;j++)
			{
				MByte *pMaskData1 = pMaskDataSrc + j*pMskSrc->lMaskLine;
				for(k=-k2;k<kmax;k++)
				{
					if (pMaskData1[k] < curMin)
						curMin = pMaskData1[k];
				}
			}
			*pMaskDataDst = (MByte)curMin;
		}
	}
    if (pMskSrc==pMskDst)
    {
        MaskCpy(&mskTmp, pMskDst);
        MaskRelease(hMemMgr, &mskTmp);
    }
    return res;
}
//////////////////////////////////////////////////////////////////////////
MVoid	MaskUnion(const JMASK* pMask1, const JMASK* pMask2, JMASK *pMaskRlt)
{
	JMASK mskSrc=*pMask1;
	MLong x, y;
	JMaskData *pDataSrc, *pDataRlt;
	JASSERT(pMask1->lWidth==pMask2->lWidth && pMask1->lHeight==pMask2->lHeight);
	if(pMask1==pMaskRlt || pMask1->pData==pMaskRlt->pData)
		mskSrc = *pMask2;
	else if(pMask2!=pMaskRlt && pMask2->pData!=pMaskRlt->pData)
		MaskCpy(pMask2, pMaskRlt);
	
	if(mskSrc.rcMask.left>=mskSrc.rcMask.right || mskSrc.rcMask.top>=mskSrc.rcMask.bottom)
		return;
	if(pMaskRlt->rcMask.left>=pMaskRlt->rcMask.right || pMaskRlt->rcMask.top>=pMaskRlt->rcMask.bottom)
	{
		MaskCpy(&mskSrc, pMaskRlt);
		return;
	}
	
	pMaskRlt->rcMask = RectUnion(&pMaskRlt->rcMask, &mskSrc.rcMask);
	pDataSrc = mskSrc.pData + mskSrc.lMaskLine*mskSrc.rcMask.top;
	pDataRlt = pMaskRlt->pData + pMaskRlt->lMaskLine*mskSrc.rcMask.top;
	for(y=mskSrc.rcMask.bottom-mskSrc.rcMask.top; y!=0; y--, 
		pDataSrc+=mskSrc.lMaskLine, pDataRlt+=pMaskRlt->lMaskLine)
	{
		for(x=mskSrc.rcMask.left; x<mskSrc.rcMask.right; x++)
		{
			if(pDataSrc[x] > pDataRlt[x])
				pDataRlt[x] = pDataSrc[x];
		}
	}
}

#ifdef  TRIM_MASK_OPT_EXT
#define NO_COLOR		-1
#define EDGE_COLOR		0x4000
MVoid MaskColor(const JMaskData* pMskData, MLong lWidth, MLong lHeight, MLong lMskLine, 
				FNMASK_COMPARE fnIsColor, MLong lVal0, JCOLOR* pColor, TRegionItem* pItems, MLong lItemNum)
{
	TRegionItem *pItemCur = MNull;	
	MLong lCountItem = 0;
	MLong x, y;

	SetVectMem(pColor, lWidth*lHeight, 0, JCOLOR);
	for(y=0; y<lHeight; y++, pMskData+=lMskLine)
	{
		for(x=0; x<lWidth; x++, pColor++)
		{	
			MLong t, c1, c2;	
			if(fnIsColor==MNull ? pMskData[x]!=lVal0 : !fnIsColor(pMskData[x], lVal0))
				continue;
			c1 = c2 = NO_COLOR;
			//Find left color
			if(x!=0 && (fnIsColor==MNull ? pMskData[x-1]==lVal0 : fnIsColor(pMskData[x-1], lVal0)))
			{
				t=pColor[-1]-1;
				do{
					if(t<0) t=-(t+1);
					t=(pItems+(t&~EDGE_COLOR))->Color;
				}while(t<0);
				c1=t;
			}
			//Find top color
			if(y!=0 && (fnIsColor==MNull ? pMskData[x-lMskLine]==lVal0 : fnIsColor(pMskData[x-lMskLine], lVal0)))
			{
				t=pColor[-lWidth]-1;
				do{
					if(t<0) t=-(t+1);
					t=(pItems+(t&~EDGE_COLOR))->Color;
				}while(t<0);
				c2=t;
			}

			if(c1==NO_COLOR && c2==NO_COLOR)
			{
				//If no left/top color found, add to new item
				JASSERT(lCountItem < lItemNum);
				if(lCountItem < lItemNum)
				{
					pItemCur=pItems+lCountItem;					
					pItemCur->Color= (JCOLOR)lCountItem;
					pItemCur->area = 1;
					lCountItem++;									
					*pColor=(JCOLOR)lCountItem;
					if(x==0||x==lWidth-1||y==0||y==lHeight-1)
						pItemCur->Color |= EDGE_COLOR;
				}
			}
			else
			{
				//Set min color as current color
				if(c1 == NO_COLOR)
				{
					c1 = c2;
					c2 = NO_COLOR;
				}
				else if(c2 != NO_COLOR)
				{
					if(c2 == c1)
						c2 = NO_COLOR;
					else if((c2&~EDGE_COLOR) < (c1&~EDGE_COLOR))
						t=c1, c1=c2, c2=t;
				}
				pItemCur=pItems+(c1&~EDGE_COLOR);
				if(x==0||x==lWidth-1||y==0||y==lHeight-1)
					pItemCur->Color |= EDGE_COLOR;
				//Integrate the different color, and set the old color as negative.
				if(c2!=NO_COLOR)
				{
					if(c2 >= EDGE_COLOR)
						pItemCur->Color |= EDGE_COLOR;
					c2 &= ~EDGE_COLOR;
					pItemCur->area = (MUInt16)(pItemCur->area + (pItems+c2)->area);
					(pItems+c2)->Color = (MInt16)-(pItemCur->Color+1);
				}
				//Update the min label
				pItemCur->area++;
				*pColor=(MInt16)((pItemCur->Color&~EDGE_COLOR)+1);
				JASSERT(*pColor > 0);
			}			
		}// end for x
	}// end for y

	//Update the negative color.
	pItemCur = pItems;
	for (y = 0; y < lCountItem; y++, pItemCur++)
	{
		MLong t = pItemCur->Color;
		if(t >= 0)
			continue;
		do {
			if( t < 0)	t = -(t + 1);
			t = (pItems + (t&~EDGE_COLOR))->Color;
		} while(t < 0);
		pItemCur->Color = (MInt16)t;
		pItemCur->area = pItems[t&~EDGE_COLOR].area;
	}//end for y
}
MLong MaskFillBySeed(MHandle hMemMgr, JMASK *pMask, MLong lSeedX, MLong lSeedY, 
					 FNMASK_COMPARE fnCompare, MLong lVal0, FNMASK_SET fnFill, MLong lNewVal)
{
	MLong lWidth = pMask->rcMask.right - pMask->rcMask.left;
	MLong lHeight = pMask->rcMask.bottom - pMask->rcMask.top;
	JMaskData *pMaskData = pMask->pData + pMask->rcMask.left 
		+ pMask->lMaskLine * pMask->rcMask.top;
	MLong lSize = 0;
	MRECT rcMask={0};	
	JPoint *pPtStack = MNull;
	MLong lPtsNum=0,  lStacksNum = lWidth*lHeight/2;
	MRESULT res = LI_ERR_NONE;
	AllocVectMem(hMemMgr, pPtStack, lStacksNum, JPoint);		

	lSeedX -= pMask->rcMask.left;
	lSeedY -= pMask->rcMask.top;
	pPtStack[lPtsNum].x = (MShort)lSeedX;
	pPtStack[lPtsNum].y = (MShort)lSeedY;
	rcMask.left = rcMask.right = lSeedX;
	rcMask.top = rcMask.bottom = lSeedY;
	lPtsNum++;
	do 
	{
		JPoint ptCur = pPtStack[--lPtsNum];
		MLong lMaskVal = pMaskData[ptCur.x+pMask->lMaskLine*ptCur.y];		
		if(fnCompare==MNull ? lMaskVal!=lVal0 : !fnCompare(lMaskVal, lVal0))
			continue;
		if(fnFill != MNull)
			lNewVal = fnFill(lMaskVal);
		if(lMaskVal==lNewVal) 
			continue;
		pMaskData[ptCur.x+pMask->lMaskLine*ptCur.y] = (JMaskData)lNewVal;
		if(rcMask.left > ptCur.x)		rcMask.left = ptCur.x;
		else if(rcMask.right < ptCur.x)	rcMask.right = ptCur.x;
		if(rcMask.top > ptCur.y)		rcMask.top = ptCur.y;
		else if(rcMask.bottom < ptCur.y)rcMask.bottom = ptCur.y;
		lSize ++;
		if(lPtsNum >= lStacksNum-4)
			continue;
		if(ptCur.x>0)
		{
			pPtStack[lPtsNum].x = (MShort)(ptCur.x-1);
			pPtStack[lPtsNum].y = ptCur.y;
			lPtsNum++;
		}
		if(ptCur.x<lWidth-1)
		{
			pPtStack[lPtsNum].x = (MShort)(ptCur.x+1);
			pPtStack[lPtsNum].y = ptCur.y;
			lPtsNum++;
		}
		if(ptCur.y>0)
		{
			pPtStack[lPtsNum].x = ptCur.x;
			pPtStack[lPtsNum].y = (MShort)(ptCur.y-1);
			lPtsNum++;
		}
		if(ptCur.y<lHeight-1)
		{
			pPtStack[lPtsNum].x = ptCur.x;
			pPtStack[lPtsNum].y = (MShort)(ptCur.y+1);
			lPtsNum++;
		}
	} while(lPtsNum > 0);
EXT:
	FreeVectMem(hMemMgr, pPtStack);
	rcMask.left += pMask->rcMask.left;
	rcMask.right += pMask->rcMask.left+1;
	rcMask.top += pMask->rcMask.top;
	rcMask.bottom += pMask->rcMask.top+1;
	if(rcMask.right > pMask->lWidth)	rcMask.right = pMask->lWidth;
	if(rcMask.bottom > pMask->lHeight)	rcMask.bottom = pMask->lHeight;
	pMask->rcMask = rcMask;
	return lSize;
}

MLong MaskFill(JMASK *pMask, FNMASK_COMPARE fnCompare, MLong lVal0, 
			   FNMASK_SET fnFill, MLong lNewVal)
{
	MLong x, y;
	MLong lWidth = pMask->rcMask.right - pMask->rcMask.left;
	JMaskData *pMskData = pMask->pData + pMask->rcMask.left
		+ pMask->lMaskLine*pMask->rcMask.top;
	MLong lMskExt = pMask->lMaskLine - lWidth;
	MLong lSize = 0;
	for(y=pMask->rcMask.bottom-pMask->rcMask.top; y!=0; y--, pMskData+=lMskExt)
	{
		for(x=lWidth; x!=0; x--, pMskData++)
		{
			MLong lMaskVal = *pMskData;
			if(fnCompare==MNull ? lMaskVal!=lVal0 : !fnCompare(lMaskVal, lVal0))
				continue;
			if(fnFill != MNull)
				lNewVal = fnFill(lMaskVal);
			*pMskData = (JMaskData)lNewVal;
			lSize ++;
		}
	}
	return lSize;
}

JSTATIC MVoid _MaskLine(JMaskData *pMaskData, MLong lMaskLine, 
	MLong x1, MLong y1, MLong x2, MLong y2, FNMASK_SET fnFill, MLong lNewVal);

MVoid MaskConvex(JMASK *pMask, FNMASK_COMPARE fnCompare, MLong lVal0, 
				 FNMASK_SET fnFill, MLong lNewVal)
{
	MRECT rcMask = {0};
	JPoint left={0}, right={0}, top={0}, bottom={0}, tmp={0};
	JMaskData *pMskData = pMask->pData + pMask->rcMask.left 
		+ pMask->lMaskLine*pMask->rcMask.top;
	MLong lWidth = pMask->rcMask.right - pMask->rcMask.left;
	MLong lHeight = pMask->rcMask.bottom - pMask->rcMask.top;
	JMaskData *pMskCur = MNull;
	MLong x, y;
	rcMask.left = lWidth, rcMask.right = 0;
	rcMask.top = lHeight, rcMask.bottom = 0;
	pMskCur = pMskData;
	for(y=0; y<lHeight; y++, pMskCur+=pMask->lMaskLine)
	{
		for(x=0; x<lWidth; x++)
		{
			if(fnCompare==MNull ? pMskCur[x]!=lVal0 : !fnCompare(pMskCur[x], lVal0))
				continue;
			if(rcMask.left > x)
			{
				rcMask.left = x;
				left.x = left.y = (MShort)y;
			}
			else if(rcMask.left == x)
			{
				if(left.x > y)			left.x = (MShort)y;
				else if(left.y < y)		left.y = (MShort)y;
			}

			if(rcMask.right < x)
			{
				rcMask.right = x;
				right.x = right.y = (MShort)y;
			}
			else if(rcMask.right == x)
			{
				if(right.x > y)			right.x = (MShort)y;
				else if(right.y < y)	right.y = (MShort)y;
			}

			if(rcMask.top > y)
			{
				rcMask.top = y;
				top.x = top.y = (MShort)x;
			}
			else if(rcMask.top == y)
			{
				if(top.x > x)			top.x = (MShort)x;
				else if(top.y < x)		top.y = (MShort)x;
			}
			
			if(rcMask.bottom < y)
			{
				rcMask.bottom = y;
				bottom.x = bottom.y = (MShort)x;
			}
			else if(rcMask.bottom == y)
			{
				if(bottom.x > x)		bottom.x = (MShort)x;
				else if(bottom.y < x)	bottom.y = (MShort)x;
			}
		}
	}

	//Fill the edge
	pMskCur = pMskData + pMask->lMaskLine*rcMask.top;
	for(x=top.x; x<=top.y; x++)
		pMskCur[x] = (JMaskData)(fnFill==MNull? lNewVal : fnFill(pMskCur[x]));
	pMskCur = pMskData + pMask->lMaskLine*rcMask.bottom;
	for(x=bottom.x; x<=bottom.y; x++)
		pMskCur[x] = (JMaskData)(fnFill==MNull? lNewVal : fnFill(pMskCur[x]));
	pMskCur = pMskData + rcMask.left + left.x*pMask->lMaskLine;
	for(x=left.y-left.x; x>=0; x--, pMskCur+=pMask->lMaskLine)
		pMskCur[0] = (JMaskData)(fnFill==MNull? lNewVal : fnFill(pMskCur[x]));
	pMskCur = pMskData + rcMask.right + right.x*pMask->lMaskLine;
	for(x=right.y-right.x; x>=0; x--, pMskCur+=pMask->lMaskLine)
		pMskCur[0] = (JMaskData)(fnFill==MNull? lNewVal : fnFill(pMskCur[x]));
	//Convex line, left&top
	tmp.x = top.x, tmp.y = (MShort)rcMask.top;
	pMskCur = pMskData + (rcMask.top+1)*pMask->lMaskLine;
	for(y=rcMask.top+1; y<=left.x; y++, pMskCur+=pMask->lMaskLine)
	{
		x=rcMask.left;
		while(fnCompare!=MNull && !fnCompare(pMskCur[x], lVal0))	x++;		
		while(fnCompare==MNull && pMskCur[x]!=lVal0)		x++;
		if(x > tmp.x)	continue;
		_MaskLine(pMskData, pMask->lMaskLine, tmp.x, tmp.y, x, y, fnFill, lNewVal);
		tmp.x = (MShort)x, tmp.y = (MShort)y;		
	}
	//left&bottom
	tmp.x = bottom.x, tmp.y = (MShort)rcMask.bottom;
	pMskCur = pMskData + (rcMask.bottom-1)*pMask->lMaskLine;
	for(y=rcMask.bottom-1; y>=left.y; y--, pMskCur-=pMask->lMaskLine)
	{
		x=rcMask.left;
		while(fnCompare!=MNull && !fnCompare(pMskCur[x], lVal0))	x++;	
		while(fnCompare==MNull && pMskCur[x]!=lVal0)		x++;
		if(x > tmp.x)	continue;
		_MaskLine(pMskData, pMask->lMaskLine, tmp.x, tmp.y, x, y, fnFill, lNewVal);
		tmp.x = (MShort)x, tmp.y = (MShort)y;
	}
	//right&top
	tmp.x = top.y, tmp.y = (MShort)rcMask.top;
	pMskCur = pMskData + (rcMask.top+1)*pMask->lMaskLine;
	for(y=rcMask.top+1; y<=right.x; y++, pMskCur+=pMask->lMaskLine)
	{
		x=rcMask.right;
		while(fnCompare!=MNull && !fnCompare(pMskCur[x], lVal0))	x--;		
		while(fnCompare==MNull && pMskCur[x]!=lVal0)		x--;
		if(x < tmp.x)	continue;
		_MaskLine(pMskData, pMask->lMaskLine, tmp.x, tmp.y, x, y, fnFill, lNewVal);
		tmp.x = (MShort)x, tmp.y = (MShort)y;		
	}
	//right&bottom
	tmp.x = bottom.y, tmp.y = (MShort)rcMask.bottom;
	pMskCur = pMskData + (rcMask.bottom-1)*pMask->lMaskLine;
	for(y=rcMask.bottom-1; y>=right.y; y--, pMskCur-=pMask->lMaskLine)
	{
		x=rcMask.right;
		while(fnCompare!=MNull && !fnCompare(pMskCur[x], lVal0))	x--;		
		while(fnCompare==MNull && pMskCur[x]!=lVal0)		x--;
		if(x < tmp.x)	continue;
		_MaskLine(pMskData, pMask->lMaskLine, tmp.x, tmp.y, x, y, fnFill, lNewVal);
		tmp.x = (MShort)x, tmp.y = (MShort)y;
	}


}
MVoid _MaskLine(JMaskData *pMaskData, MLong lMaskLine, MLong x1, MLong y1, 
			   MLong x2, MLong y2, FNMASK_SET fnFill, MLong lNewVal)
{
	MLong x, y;
	if(ABS(y2 - y1) >= ABS(x2 - x1))
	{
		if(y2==y1)
			return;
		if(y1 > y2)	y=y1, y1=y2, y2=y, x=x1, x1=x2, x2=x;
		pMaskData += y1*lMaskLine;
		for(y=y1; y<=y2; y++, pMaskData+=lMaskLine)
		{
			x = ((x1-x2)*y + (y1*x2-x1*y2)) / (y1-y2);
			pMaskData[x]  = (JMaskData)(fnFill==MNull ? lNewVal : fnFill(pMaskData[x]));
		}
	}
	else
	{
		if(x1 > x2)	y=y1, y1=y2, y2=y, x=x1, x1=x2, x2=x;
		pMaskData += x1;
		for(x=x1; x<=x2; x++, pMaskData++)
		{
			y = ((y1-y2)*x + (x1*y2-y1*x2)) / (x1-x2);
			pMaskData[lMaskLine*y] = (JMaskData)(fnFill==MNull ? lNewVal : fnFill(pMaskData[lMaskLine*y]));
		}
	}
}
//////////////////////////////////////////////////////////////////////////
MVoid MaskIntersect(const JMASK* pMask1, const JMASK* pMask2, JMASK *pMaskRlt)
{
	JMASK mskSrc=*pMask1;
	MLong x, y;
	JMaskData *pDataSrc, *pDataRlt;
	JASSERT(pMask1->lWidth==pMask2->lWidth && pMask1->lHeight==pMask2->lHeight);
	if(pMask1==pMaskRlt || pMask1->pData==pMaskRlt->pData)
		mskSrc = *pMask2;
	else if(pMask2!=pMaskRlt && pMask2->pData!=pMaskRlt->pData)
		MaskCpy(pMask2, pMaskRlt);
	
	if(pMaskRlt->rcMask.left>=pMaskRlt->rcMask.right || pMaskRlt->rcMask.top>=pMaskRlt->rcMask.bottom)
	{
		return;
	}	
	if(mskSrc.rcMask.left>=mskSrc.rcMask.right || mskSrc.rcMask.top>=mskSrc.rcMask.bottom)
	{
		MaskSet(pMaskRlt, 0);
		return;
	}	
	pMaskRlt->rcMask = RectIntersect(&pMaskRlt->rcMask, &mskSrc.rcMask);
	// 	SetVectMem(pMaskRlt->pData, pMaskRlt->rcMask.top*pMaskRlt->lMaskLine, 0, JMaskData);
	// 	SetVectMem(pMaskRlt->pData+pMaskRlt->rcMask.bottom*pMaskRlt->lMaskLine, (pMaskRlt->lHeight-pMaskRlt->rcMask.bottom)*pMaskRlt->lMaskLine, 0, JMaskData);
	// 	SetImgMem(pMaskRlt->pData+pMaskRlt->rcMask.top*pMaskRlt->lMaskLine, pMaskRlt->lMaskLine, pMaskRlt->rcMask.left, pMaskRlt->rcMask.bottom-pMaskRlt->rcMask.top, 0, JMaskData);
	// 	SetImgMem(pMaskRlt->pData+pMaskRlt->rcMask.top*pMaskRlt->lMaskLine+pMaskRlt->rcMask.right, pMaskRlt->lMaskLine, pMaskRlt->lWidth-pMaskRlt->rcMask.right, pMaskRlt->rcMask.bottom-pMaskRlt->rcMask.top, 0, JMaskData);
	
	pDataSrc = mskSrc.pData + mskSrc.lMaskLine*pMaskRlt->rcMask.top;
	pDataRlt = pMaskRlt->pData + pMaskRlt->lMaskLine*pMaskRlt->rcMask.top;
	for(y=pMaskRlt->rcMask.bottom-pMaskRlt->rcMask.top; y!=0; y--, 
		pDataSrc+=mskSrc.lMaskLine, pDataRlt+=pMaskRlt->lMaskLine)
	{
		for(x=pMaskRlt->rcMask.left; x<pMaskRlt->rcMask.right; x++)
		{
			if(pDataSrc[x] < pDataRlt[x])
				pDataRlt[x] = pDataSrc[x];
		}
	}
}


#endif	//TRIM_MASK_OPT_EXT
