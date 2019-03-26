#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "skinextract.h"
#include "litrimfun.h"
#include "liimage.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "limath.h"
#include "limask.h"
#include "skinarea.h"
#ifdef ENABLE_PRIOR 
#include "priorarea.h"
#endif


MRESULT SE_Create(MHandle hMemMgr, INFO_SKIN *pSkiner)
{
	MRESULT res = LI_ERR_NONE;
	SetVectMem(pSkiner, 1, 0, INFO_SKIN);
	pSkiner->hMemMgr = hMemMgr;
	AllocVectMem(hMemMgr, pSkiner->pParamFilter, 1, PARAM_SKIN_FILTER);
	pSkiner->bAreaMerge = MTrue;
	pSkiner->bFillHole = MTrue;
	pSkiner->bSaveFaceArea = MFalse;
	pSkiner->bRemoveFaceArea = MFalse; 
	pSkiner->bConnectNeighbor = MTrue;
	//pSkiner->lMinAreaSize = 0;
	pSkiner->lCbOffset = pSkiner->lCrOffset = 0;
	pSkiner->lGrayThres = 60;

	pSkiner->lMinWordLength = 1;
	pSkiner->lMaxWordLength = 4;
	pSkiner->lMinAreaSize=40; //new add 设置最小size过滤
	
	pSkiner->pParamFilter->bRemoveNoSkinColor = MTrue;
	pSkiner->pParamFilter->bWhiteBalance = MTrue;
	pSkiner->pParamFilter->bRemoveAreaOnEdge = MTrue;
	pSkiner->pParamFilter->bRemoveRectangleArea = MFalse;
	pSkiner->pParamFilter->bRemoveSmallArea = MTrue;
	pSkiner->pParamFilter->bRemoveStrip = MTrue;
	pSkiner->pParamFilter->bRemoveComplexArea = MTrue;
	pSkiner->pParamFilter->bRemoveRelatedColor = MFalse;
EXT:
	return res;
}
MVoid SE_Release(INFO_SKIN *pSkiner)
{
	if(pSkiner == MNull)
		return;
	FreeVectMem(pSkiner->hMemMgr, pSkiner->pParamFilter);	
	FreeVectMem(pSkiner->hMemMgr, pSkiner->pcrSeeds);
	MaskRelease(pSkiner->hMemMgr, pSkiner->pMskFace);
	FreeVarMem(pSkiner->hMemMgr, pSkiner->pMskFace);

	ReleaseAreaGraph(pSkiner->hMemMgr, &pSkiner->infoGraph);
	FreeVectMem(pSkiner->hMemMgr, pSkiner->pLabelData);
	pSkiner->lWidth = pSkiner->lHeight = 0;
}

#ifdef ENABLE_DEBUG
JLabelData *g_pLabelData;
MLong g_lLabelLine, g_lWidth, g_lHeight;
#endif

#include "liimgfmttrans.h"
JSTATIC MVoid DrawYUVImg(PJOFFSCREEN pImg, const MChar* pOutStr)
{
	JOFFSCREEN imgBGR = {0};
	ImgCreate(MNull, &imgBGR, FORMAT_BGR, pImg->dwWidth, pImg->dwHeight);
	ImgFmtTrans(pImg, &imgBGR);
	if (pOutStr==MNull)
		PrintBmp(imgBGR.pixelArray.chunky.pPixel, imgBGR.pixelArray.chunky.dwImgLine, DATA_U8, 
		imgBGR.dwWidth, imgBGR.dwHeight, 3);
	else
		PrintBmpEx(imgBGR.pixelArray.chunky.pPixel, imgBGR.pixelArray.chunky.dwImgLine, DATA_U8, 
		imgBGR.dwWidth, imgBGR.dwHeight, 3, pOutStr);
	ImgRelease(MNull, &imgBGR);
}

MRESULT SE_SkinExtract(INFO_SKIN *pSkiner, const JOFFSCREEN *pImg, 
					   JMASK *pMskSkin)
{
	MRESULT res = LI_ERR_NONE;
	JLabelData *pLabelData = pSkiner->pLabelData;
	MLong lLabelLine = JMemLength(pImg->dwWidth);
	JINFO_AREA_GRAPH* pInfoGraph = &pSkiner->infoGraph;

// 	MLong lMinAreaSize = pImg->dwWidth * pImg->dwHeight / 100;
//	pSkiner->lMinAreaSize = lMinAreaSize;

#ifdef ENABLE_VIDEO_SEEDS
	JASSERT(pLabelData==MNull);
	AllocVectMem(pSkiner->hMemMgr, pLabelData, lLabelLine*pImg->dwHeight, JLabelData);
	GO(CreateAreaGraphByImg(pSkiner->hMemMgr, pImg, pLabelData, lLabelLine, pInfoGraph));	
	CheckAreaGraph(pImg, pInfoGraph, pLabelData, lLabelLine);
	pSkiner->lWidth = pImg->dwWidth;
	pSkiner->lHeight = pImg->dwHeight;
#else
	if(pSkiner->pLabelData!=MNull)
	{
		JASSERT(pInfoGraph->lLabelNum!=MNull);
		JASSERT(pImg->dwWidth-pSkiner->lWidth==0 && pImg->dwHeight-pSkiner->lHeight==0);
	}
	else
	{
		AllocVectMem(pSkiner->hMemMgr, pLabelData, lLabelLine*pImg->dwHeight, JLabelData);
		pSkiner->pLabelData = pLabelData;
		GO(CreateAreaGraphByImg(pSkiner->hMemMgr, pImg, pLabelData, lLabelLine, pInfoGraph));
		//PrintChannel(pLabelData, lLabelLine, DATA_I16, pImg->dwWidth,pImg->dwHeight, 1, 0);
		CheckAreaGraph(pImg, pInfoGraph, pLabelData, lLabelLine);
		pSkiner->lWidth = pImg->dwWidth;
		pSkiner->lHeight = pImg->dwHeight;
	}
#endif
	
	RemoveMargin(pInfoGraph, pImg->dwWidth, pImg->dwHeight);	

#ifdef ENABLE_DEBUG
	g_pLabelData = pLabelData;
	g_lLabelLine = lLabelLine;
	g_lWidth = pImg->dwWidth, g_lHeight = pImg->dwHeight;
	///count of the effective blobs
#endif
	//删选无效区域，这步操作需要放在区域合并之前,在后续的合并会少了很多的干扰
	//如果放在之后，组合时会有很多导干扰，
	SkinFilter(pInfoGraph, pSkiner->pParamFilter, pSkiner->lMinAreaSize, 
		pSkiner->lCbOffset, pSkiner->lCrOffset);

	if(pSkiner->bAreaMerge)
 		AreaMerge(pInfoGraph, pSkiner->lMinAreaSize); 

	if(pSkiner->bSaveFaceArea)
		MergeSkinArea(pInfoGraph, pSkiner->lMinAreaSize);
	if(pSkiner->bFillHole)
		FillHoleInSkin(pInfoGraph, pSkiner->lMinAreaSize*4);
	if (pSkiner->bConnectNeighbor)
		ConnectNeighborInSkin(pSkiner->hMemMgr, pInfoGraph, pLabelData, lLabelLine, pImg->dwWidth, pImg->dwHeight);

	if(pSkiner->bSaveFaceArea)
	{
		MLong lLabelCur;
		for(lLabelCur=pInfoGraph->lLabelNum; lLabelCur>=0; lLabelCur--)
		{
			JINFO_AREA *pInfoArea = pInfoGraph->pInfoArea + lLabelCur; 
			MLong lLinkNum = 0, i; 
			if(pInfoArea->lFlag <= 0)
				continue;		
			
			for(i=0; i<lLabelCur; i++)			
			{
				if(pInfoGraph->pInfoArea[i].lFlag > 0)
					lLinkNum += pInfoGraph->pInfoArea[lLabelCur].pBoundaryNum[i+1];
			}
			for(i=lLabelCur+1; i<pInfoGraph->lLabelNum; i++)
			{
				if(pInfoGraph->pInfoArea[i].lFlag > 0)
					lLinkNum += pInfoGraph->pInfoArea[i].pBoundaryNum[lLabelCur+1];
			}
			JASSERT(lLinkNum <= pInfoArea->lBoundaryNum);

			if(lLinkNum > VALID_NEIGHBOR_BOUNDARY(pInfoArea->lBoundaryNum))
				continue;			

			if(pSkiner->pParamFilter->bRemoveSmallArea)
			{
				if(pInfoArea->lSize <= pSkiner->lMinAreaSize)
				{
					RemoveArea(pInfoGraph->pInfoArea, lLabelCur, pInfoGraph->lLabelNum);
					continue;
				}
			}
			if(pSkiner->pParamFilter->bRemoveComplexArea)
			{
				if(MAX_AREA_COMPLEX < GET_COMPLEX_EX(pInfoArea->lBoundaryNum, pInfoArea->lSize, pInfoArea->lEdgeNum))
				{				
					RemoveArea(pInfoGraph->pInfoArea, lLabelCur, pInfoGraph->lLabelNum);
					continue;
				}
			}
			if(pSkiner->pParamFilter->bRemoveStrip)
			{
				if(IS_STRIP_AREA(pInfoArea))
				{
					RemoveArea(pInfoGraph->pInfoArea, lLabelCur, pInfoGraph->lLabelNum);
					continue;
				}
			}

		}
	}
	///如果用pdistanceblob，就不再适用该规则
	//CheckAreaGraph(pImg, pInfoGraph, pLabelData, lLabelLine);	
	
	if(pMskSkin != MNull)
	{
		ShowLabelMask(pInfoGraph, pLabelData, lLabelLine, pImg->dwWidth, 
			pImg->dwHeight, pMskSkin);
		ShowImgByMask(pImg, (PJOFFSCREEN)pImg,pMskSkin, MTrue);
		DrawYUVImg((PJOFFSCREEN)pImg, "..\\segimg.bmp");
	}
EXT:
#ifdef ENABLE_VIDEO_SEEDS
	FreeVectMem(pSkiner->hMemMgr, pLabelData);
	ReleaseAreaGraph(pSkiner->hMemMgr, pInfoGraph);	
#endif

	return res;
}



/////////////////////////////////////////////////////////////
#ifdef ENABLE_PRIOR
MRESULT SE_SetPriorArea(INFO_SKIN *pSkiner, const JOFFSCREEN *pImg, 
						const MRECT* prtSkinAreas, MLong lSkinAreaNum)
{
	MRESULT  res = LI_ERR_NONE;
	MLong lFaceCur;
	if(pSkiner->pcrSeeds == MNull)
		AllocVectMem(pSkiner->hMemMgr, pSkiner->pcrSeeds, MAX_SEEDS_NUM, MCOLORREF);
	
	if(pSkiner->bRemoveFaceArea && pSkiner->pMskFace==MNull)
	{
		AllocVarMem(pSkiner->hMemMgr,pSkiner->pMskFace, 1, JMASK);
		GO(MaskCreate(pSkiner->hMemMgr, pSkiner->pMskFace, pImg->dwWidth, pImg->dwHeight));
	}
	GO(FaceMask(pSkiner->hMemMgr, pImg, prtSkinAreas, lSkinAreaNum, 
		pSkiner->pMskFace, pSkiner->pcrSeeds, &pSkiner->lSeedsNum));

	if(pSkiner->pParamFilter->bWhiteBalance)
	{
		MCOLORREF crRef = pSkiner->pcrSeeds[0];
		if(!IS_SKIN_COLOR(PIXELVAL_1(crRef), PIXELVAL_2(crRef), PIXELVAL_3(crRef)))
		{
			pSkiner->lCbOffset = 110 - PIXELVAL_2(crRef);
			pSkiner->lCrOffset = 158 - PIXELVAL_3(crRef);
		}
	}

	pSkiner->lMinAreaSize = (prtSkinAreas->right-prtSkinAreas->left)*(prtSkinAreas->bottom-prtSkinAreas->top)/16;
	for(lFaceCur=1; lFaceCur<lSkinAreaNum; lFaceCur++)
	{
		const MRECT* prtFace = prtSkinAreas + lFaceCur;
		MLong lSize = (prtFace->right-prtFace->left)*(prtFace->bottom-prtFace->top);
		if(lSize/16 < pSkiner->lMinAreaSize)
			pSkiner->lMinAreaSize = lSize/16;
	}
	if((MDWord)pSkiner->lMinAreaSize < MIN_AREA_SIZE(pImg->dwWidth*pImg->dwHeight))
		pSkiner->lMinAreaSize = MIN_AREA_SIZE(pImg->dwWidth*pImg->dwHeight);
EXT:
	return res;
}
#endif

