#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "liblock.h"
#include "lidebug.h"
#include "lierrdef.h"
#include "limath.h"
#include "licomdef.h"
#include "liimage.h"
#include "limem.h"
#include "lichannel.h"
#include "litimer.h"
#ifdef ENABLE_REDUCE_Y_BLOCK
# include "lipyramid.h"
#endif
/*
 *	Implement
 */
//////////////////////////////////////////////////////////////////////////
JRECT_EXT SetRectExt(MLong lExt)
{
	JRECT_EXT rectExt;
	rectExt.bottom = rectExt.top = lExt;
	rectExt.left = rectExt.right = lExt;
	return rectExt;
}
//////////////////////////////////////////////////////////////////////////
MRESULT B_Init_Ex(MHandle hMemMgr, PBLOCK pBlock, 
				  JTYPE_DATA_A typeDataA, 
				  MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;
	pBlock->lWidth = lWidth;
	pBlock->lHeight = lHeight;
	pBlock->typeDataA = typeDataA;
	
	if(pBlock->pBlockData == MNull)
	{
		pBlock->lBlockLine = JMemLength(lWidth);		
		AllocVectMem(hMemMgr, pBlock->pBlockData, 
			lHeight*pBlock->lBlockLine*IF_DATA_BYTES(typeDataA), MByte);		
	}
	JASSERT(pBlock->lWidth <= pBlock->lBlockLine);
EXT:
	return res;
}
MVoid	B_Init(PBLOCK pBlock, JTYPE_DATA_A typeDataA, 
				  MLong lWidth, MLong lHeight)
{
	JASSERT(pBlock->pBlockData != MNull);
	JASSERT(lWidth <= pBlock->lWidth && lHeight <= pBlock->lHeight);
	B_Init_Ex(MNull, pBlock, typeDataA, lWidth, lHeight);
}
MRESULT B_Create(MHandle hMemMgr, PBLOCK pBlock, 
				 JTYPE_DATA_A typeDataA, 
				 MLong lWidth, MLong lHeight)
{
	JASSERT(pBlock->pBlockData == MNull);
	return B_Init_Ex(hMemMgr, pBlock, typeDataA, lWidth, lHeight);
}
MVoid B_Release(MHandle hMemMgr, PBLOCK pBlock)
{
	FreeVectMem(hMemMgr, pBlock->pBlockData);
}

MVoid B_UpdateBlock(PBLOCK pBlockRlt, MVoid *pImgData, MLong lImgLine, 
					JTYPE_DATA_A typeImg, MLong lChannelNum, MLong lChannelCur, 
					MLong lOffRltX, MLong lOffRltY, MBool bLoadFromImg)
{
	JASSERT(lChannelNum > lChannelCur);

	if(lChannelNum == 1)
	{
		JASSERT(pBlockRlt->typeDataA == typeImg);
		if(bLoadFromImg)
		{
			CpyImgMem_Ex(pBlockRlt->pBlockData, 0, 0, pBlockRlt->lBlockLine, 
				pImgData, lOffRltX, lOffRltY, lImgLine, pBlockRlt->lWidth, pBlockRlt->lHeight, 
				pBlockRlt->typeDataA);	
		}
		else
		{
			CpyImgMem_Ex(pImgData, lOffRltX, lOffRltY, lImgLine, 
				pBlockRlt->pBlockData, 0, 0, pBlockRlt->lBlockLine, 			
				pBlockRlt->lWidth, pBlockRlt->lHeight, pBlockRlt->typeDataA);
		}
	}
	else
	{
		MVoid* pImgCur = (MUInt8*)pImgData + (lOffRltY*lImgLine +
			lOffRltX*lChannelNum)*IF_DATA_BYTES(typeImg);
		AccessChannel(pImgCur, lImgLine, pBlockRlt->pBlockData, 
			pBlockRlt->lBlockLine, pBlockRlt->lWidth, 
			pBlockRlt->lHeight, lChannelNum, lChannelCur, IF_DATA_TYPE(pBlockRlt->typeDataA), bLoadFromImg);
	}
}

//////////////////////////////////////////////////////////////////////////
MRESULT CB_Init_Ex(MHandle hMemMgr, PCHL_BLOCK pChlBlock, 
				   JTYPE_DATA_A typeDataA, 
				   const PJRECT_EXT pExt, const PJSIZE pValidSize, 
				   MLong lImgW, MLong lImgH)
{
	pChlBlock->lImgW = lImgW;
	pChlBlock->lImgH = lImgH;
	pChlBlock->lValidL = pChlBlock->lValidT = 0;	
	return BE_Init_Ex(hMemMgr, &pChlBlock->blockExt, 
		typeDataA, pExt, pValidSize->lWidth, pValidSize->lHeight);
}
MRESULT CB_Create(MHandle hMemMgr, PCHL_BLOCK pChlBlock, 
				  JTYPE_DATA_A typeDataA, 
				  const PJRECT_EXT pExt, const PJSIZE pValidSize, 
				  MLong lImgW, MLong lImgH)
{
	JASSERT(B_BlockData(pChlBlock) == MNull);
	return CB_Init_Ex(hMemMgr, pChlBlock, typeDataA, pExt, pValidSize, 
		lImgW, lImgH);
}
MVoid	CB_Init(PCHL_BLOCK pChlBlock, JTYPE_DATA_A typeDataA, 
				const PJRECT_EXT pExt, const PJSIZE pValidSize, 
				MLong lImgW, MLong lImgH)
{
	JASSERT(B_BlockData(pChlBlock) != MNull);
	CB_Init_Ex(MNull, pChlBlock, typeDataA, pExt, pValidSize, 
		lImgW, lImgH);
}

MVoid	CB_Release(MHandle hMemMgr, PCHL_BLOCK pChlBlock)
{
	BE_Release(hMemMgr, &pChlBlock->blockExt);
}

MVoid CB_UpdateValid(PCHL_BLOCK pChlBlock, MBool bLoadFromImg, 
						MVoid *pImgData, MLong lImgLine, JTYPE_DATA_A typeImg, 
						MLong lChannelNum, MLong lChannelCur)
{
	PBLOCKEXT pBlockExt = &pChlBlock->blockExt;
	BLOCK validb = pBlockExt->block;

	JASSERT(lChannelCur < lChannelNum);
	if(pChlBlock->lValidL >= pChlBlock->lImgW || pChlBlock->lValidT >= pChlBlock->lImgH)
		return;
	
	validb.pBlockData = BE_ValidData(pBlockExt);
	validb.lWidth	= BE_ValidW(pBlockExt);
	validb.lHeight	= BE_ValidH(pBlockExt);
	JASSERT(validb.lWidth + pChlBlock->lValidL <= pChlBlock->lImgW);
	JASSERT(validb.lHeight + pChlBlock->lValidT <= pChlBlock->lImgH);
	B_UpdateBlock(&validb, pImgData, lImgLine, typeImg, lChannelNum, lChannelCur, 
		pChlBlock->lValidL, pChlBlock->lValidT, bLoadFromImg);		
}

MVoid CB_LoadBlock(PCHL_BLOCK pChlBlock, 
				   MVoid *pImgData, MLong lImgLine, JTYPE_DATA_A typeImg, 
				   MLong lChannelNum, MLong lChannelCur)
{
	BLOCK block = pChlBlock->blockExt.block;
	MLong lOffX = CB_RealL(pChlBlock), lOffY = CB_RealT(pChlBlock);
	block.pBlockData = CB_RealData(pChlBlock);
	block.lWidth = CB_RealW(pChlBlock);
	block.lHeight = CB_RealH(pChlBlock);
	B_UpdateBlock(&block, pImgData, lImgLine, typeImg, lChannelNum, lChannelCur, 
		lOffX, lOffY, MTrue);
	CB_AutoMirrorFill(pChlBlock, 0, 0);
}

MVoid CB_OffSetValidRect(PCHL_BLOCK pChlBlock, MLong lValidL, MLong lValidT)
{
	JASSERT(lValidL >= CB_RealL(pChlBlock) && lValidL <= CB_RealR(pChlBlock));
	JASSERT(lValidT >= CB_RealT(pChlBlock) && lValidT <= CB_RealB(pChlBlock));

	pChlBlock->blockExt.ext.left += lValidL - pChlBlock->lValidL;
	pChlBlock->blockExt.ext.top += lValidT - pChlBlock->lValidT;
	pChlBlock->lValidL = lValidL;
	pChlBlock->lValidT = lValidT;
}

MVoid CB_AutoMirrorFill(PCHL_BLOCK pChlBlock, MLong lROffSet, MLong lBOffSet)
{
	PBLOCK pBlock = &pChlBlock->blockExt.block;
	PJRECT_EXT pExt = &pChlBlock->blockExt.ext;
	MRECT rtValid;
	MLong lOffset;
	rtValid.left = rtValid.top = 0;
	rtValid.right = pBlock->lWidth;
	rtValid.bottom = pBlock->lHeight;
	JASSERT(pChlBlock != MNull);
	//Left top
	if((lOffset = pExt->left - pChlBlock->lValidL) > 0)
		rtValid.left = lOffset;
	if((lOffset = pExt->top - pChlBlock->lValidT) > 0)
		rtValid.top = lOffset;
	//right bottom
	if(pBlock->lWidth > (lOffset = pChlBlock->lImgW+lROffSet-pChlBlock->lValidL+pExt->left))
		rtValid.right = lOffset;
	if(pBlock->lHeight > (lOffset = pChlBlock->lImgH+lBOffSet-pChlBlock->lValidT+pExt->top))
		rtValid.bottom = lOffset;
	MirrorFill(pBlock->pBlockData, pBlock->lWidth, pBlock->lHeight, 
		pBlock->lBlockLine, IF_DATA_TYPE(pBlock->typeDataA), &rtValid);
}
//////////////////////////////////////////////////////////////////////////
MRESULT	BE_Init_Ex(MHandle hMemMgr, PBLOCKEXT pBlockExt, 
				   JTYPE_DATA_A typeDataA,  PJRECT_EXT pRectExt, 
				   MLong lWidth, MLong lHeight)
{
	if(pRectExt != MNull)
		pBlockExt->ext = *pRectExt;
	else
		SetVectMem(&pBlockExt->ext, 1, 0, JRECT_EXT);
	return B_Init_Ex(hMemMgr, &pBlockExt->block, typeDataA,		
		lWidth+pBlockExt->ext.left+pBlockExt->ext.right, 
		lHeight+pBlockExt->ext.top+pBlockExt->ext.bottom);
}
MRESULT	BE_Create(MHandle hMemMgr, PBLOCKEXT pBlockExt, 
				  JTYPE_DATA_A typeDataA,  PJRECT_EXT pRectExt, 
				  MLong lWidth, MLong lHeight)
{
	JASSERT(B_BlockData(pBlockExt) == MNull);
	return BE_Init_Ex(hMemMgr, pBlockExt, typeDataA, pRectExt, lWidth, lHeight);
}
MVoid	BE_Init(PBLOCKEXT pBlockExt, JTYPE_DATA_A typeDataA,  
				PJRECT_EXT pRectExt, MLong lWidth, MLong lHeight)
{
	JASSERT(B_BlockData(pBlockExt) != MNull);
	BE_Init_Ex(MNull, pBlockExt, typeDataA, pRectExt, lWidth, lHeight);
}
MVoid	BE_Release(MHandle hMemMgr, PBLOCKEXT pBlockExt)
{
	B_Release(hMemMgr, &pBlockExt->block);
}

MVoid	BE_MirrorFill(PBLOCKEXT pBlockExt)
{
	MRECT rtValid;
	rtValid.left = pBlockExt->ext.left;
	rtValid.top = pBlockExt->ext.top;
	rtValid.right = rtValid.left + BE_ValidW(pBlockExt);
	rtValid.bottom = rtValid.top + BE_ValidH(pBlockExt);	
	B_MirrorFill(pBlockExt, &rtValid);
}

BLOCK	BE_ValidBlock(const BLOCKEXT* pBlockExt)
{
	BLOCK blk = *(PBLOCK)pBlockExt;
	blk.lWidth = BE_ValidW(pBlockExt);
	blk.lHeight = BE_ValidH(pBlockExt);
	blk.pBlockData = BE_ValidData(pBlockExt);
	return blk;
}

//////////////////////////////////////////////////////////////////////////
#define MIRROR_FILL(pBlockData, lWidth, lHeight, lBlockLine, prtValid, TYPE)\
{																			\
	const MLong OFFSET = 1;													\
	JASSERT(pBlockData != MNull && prtValid != MNull);						\
	JASSERT(prtValid->left >=0 && prtValid->top >=0);						\
	JASSERT(prtValid->right <= lWidth && prtValid->bottom <= lHeight);		\
	/*PrintBmp(pBlockData, lBlockLine, DATA_U8, lWidth,lHeight, 1);*/		\
	if(prtValid->left > 0)													\
	{																		\
		MLong x, y;															\
		TYPE* pDataCur1 = (TYPE*)pBlockData + prtValid->left				\
			+ prtValid->top * lBlockLine;									\
		TYPE* pDataCur2 = pDataCur1 - 1;									\
		pDataCur1 += OFFSET;												\
		for(y=prtValid->bottom-prtValid->top; y!=0; y--,					\
			pDataCur1 += lBlockLine, pDataCur2 += lBlockLine)				\
		{																	\
			for(x=prtValid->left-1; x>=0; x--)								\
				pDataCur2[-x] = pDataCur1[x];								\
		}																	\
	}																		\
	if(prtValid->right < lWidth)											\
	{																		\
		MLong x, y;															\
		TYPE* pDataCur1 = (TYPE*)pBlockData + prtValid->right - 1			\
			+ prtValid->top * lBlockLine;									\
		TYPE* pDataCur2 = pDataCur1 + 1;									\
		pDataCur1 -= OFFSET;												\
		for(y=prtValid->bottom-prtValid->top; y!=0; y--,					\
			pDataCur1 += lBlockLine, pDataCur2 += lBlockLine)				\
		{																	\
			for(x=lWidth-prtValid->right-1; x>=0; x--)						\
				pDataCur2[x] = pDataCur1[-x];								\
		}																	\
	}																		\
	if(prtValid->top > 0)													\
	{																		\
		MLong x;															\
		TYPE* pDataCur1 = (TYPE*)pBlockData + (prtValid->top) * lBlockLine;	\
		TYPE* pDataCur2 = pDataCur1 - lBlockLine;							\
		pDataCur1 += OFFSET * lBlockLine;									\
		for(x=prtValid->top; x!=0; x--,										\
			pDataCur1 += lBlockLine, pDataCur2 -= lBlockLine)				\
		{																	\
			CpyVectMem(pDataCur2, pDataCur1, lWidth, TYPE);					\
		}																	\
	}																		\
	if(prtValid->bottom < lHeight)											\
	{																		\
		MLong x;															\
		TYPE* pDataCur1 = (TYPE*)pBlockData									\
			+ (prtValid->bottom-1) * lBlockLine;							\
		TYPE* pDataCur2 = pDataCur1 + lBlockLine;							\
		pDataCur1 -= OFFSET*lBlockLine;										\
		for(x=lHeight-prtValid->bottom; x!=0; x--,							\
			pDataCur1 -= lBlockLine, pDataCur2 += lBlockLine)				\
		{																	\
			CpyVectMem(pDataCur2, pDataCur1, lWidth, TYPE);					\
		}																	\
	}																		\
	/*PrintBmp(pBlockData, lBlockLine, DATA_U8, lWidth,lHeight, 1);*/		\
}

MVoid MirrorFill(MVoid *pBlockData, 
				 MLong lWidth, MLong lHeight, 
				 MLong lBlockLine, JTYPE_DATA typeData, 
				 PMRECT prtValid)
{
	if(prtValid->bottom-prtValid->top < lHeight - prtValid->bottom)
		lHeight = prtValid->bottom + prtValid->bottom-prtValid->top;
	if(prtValid->right-prtValid->left < lWidth - prtValid->right)
		lWidth = prtValid->right + prtValid->right-prtValid->left;

	switch(IF_DATA_BYTES(typeData)) {
	case 1:
		MIRROR_FILL(pBlockData, lWidth, lHeight, lBlockLine, prtValid, MUInt8);
		break;
#ifdef TRIM_DATA_16BITS
	case 2:
		MIRROR_FILL(pBlockData, lWidth, lHeight, lBlockLine, prtValid, MUInt16);
		break;
#endif	//TRIM_DATA_16BITS
	default:
		JASSERT(MFalse);
		break;
	}	
}
#ifdef ENABLE_REDUCE_Y_BLOCK
//////////////////////////////////////////////////////////////////////////////////////
JSTATIC MRESULT _ReduceYBlock_YUYV(CHL_BLOCK* pblkReduce, const JOFFSCREEN* pImg);
JSTATIC MRESULT _ExpandYBlock_YUYV(JOFFSCREEN* pImg, const CHL_BLOCK* pblkReduce, 
								   MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask);
JSTATIC MRESULT _ReduceYBlock_Y1VY0U(CHL_BLOCK* pblkReduce, const JOFFSCREEN* pImg);
JSTATIC MRESULT _ExpandYBlock_Y1VY0U(JOFFSCREEN* pImg, const CHL_BLOCK* pblkReduce, 
								   MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask);

MRESULT ReduceYBlock(MHandle hMemMgr, CHL_BLOCK* pblkReduce, const JOFFSCREEN* pImg)
{
	MRESULT res = LI_ERR_NONE;
    CALLBACK_TICK_RETURN("Begin of _ReduceYBlock");
	if(IF_IS_PLANAR_FORMAT(pImg->fmtImg))
	{
		MLong lLeft	= CB_RealL(pblkReduce), lRight = CB_RealR(pblkReduce);
		MLong lTop	= CB_RealT(pblkReduce), lBottom= CB_RealB(pblkReduce);		
		
		res = ReduceBlock(hMemMgr, (MUInt8*)pImg->pixelArray.planar.pPixel[0], 
			pImg->pixelArray.planar.dwImgLine[0], 
			pImg->dwWidth, pImg->dwHeight, lLeft, lRight, lTop, lBottom, 
			(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
			B_BlockLine(pblkReduce), B_BlockType(pblkReduce));
		
		{
			MRECT rtValid;
			rtValid.left = lLeft - CB_BlockL(pblkReduce);
			rtValid.right = B_BlockWidth(pblkReduce) - (CB_BlockR(pblkReduce) - lRight);
			rtValid.top = lTop - CB_BlockT(pblkReduce);
			rtValid.bottom = B_BlockHeight(pblkReduce) - (CB_BlockB(pblkReduce) - lBottom);
			B_MirrorFill((PBLOCK)pblkReduce, &rtValid);
		}
		return res;
	}

	switch(pImg->fmtImg)
	{
	case FORMAT_YUYV:
		return _ReduceYBlock_YUYV(pblkReduce, pImg);
	case FORMAT_Y1VY0U:
		return _ReduceYBlock_Y1VY0U(pblkReduce, pImg);
	default:
		//JASSERT(MFalse);
	    return LI_ERR_IMAGE_FORMAT;
	}
}

MRESULT ExpandYBlock(JOFFSCREEN* pImg, const CHL_BLOCK* pblkReduce, 
					 MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask)
{
    CALLBACK_TICK_RETURN("Begin of _ExpandYBlock");
	if(IF_IS_PLANAR_FORMAT(pImg->fmtImg))
	{
		MLong lLeft	= CB_ValidL(pblkReduce)*2, lRight = CB_ValidR(pblkReduce)*2;
		MLong lTop	= CB_ValidT(pblkReduce)*2, lBottom= CB_ValidB(pblkReduce)*2;
		
		JASSERT(lRight/lMskReduceW<=pMask->lWidth && lBottom/lMskReduceH<=pMask->lHeight);	
		JASSERT(lTop/lMskReduceH>=pMask->rcMask.top && lBottom/lMskReduceH<=pMask->rcMask.bottom);
		JASSERT(lLeft/lMskReduceW>=pMask->rcMask.left && lRight/lMskReduceW<=pMask->rcMask.right);
		JASSERT(lLeft>=0 && lRight<=(MLong)pImg->dwWidth && lTop>=0 && lBottom<=(MLong)pImg->dwHeight);
		return ExpandBlockByMask((MUInt8*)pImg->pixelArray.planar.pPixel[0], 
			pImg->pixelArray.planar.dwImgLine[0], lLeft, lRight, lTop, lBottom, 
			(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
			B_BlockLine(pblkReduce), CB_BlockR(pblkReduce), CB_BlockB(pblkReduce), 
			pMask->pData, pMask->lMaskLine, lMskReduceW, lMskReduceH);
	}

	switch(pImg->fmtImg)
	{
	case FORMAT_YUYV:
		return _ExpandYBlock_YUYV(pImg, pblkReduce, lMskReduceW, lMskReduceH, pMask);
	case FORMAT_Y1VY0U:
		return _ExpandYBlock_Y1VY0U(pImg, pblkReduce, lMskReduceW, lMskReduceH, pMask);
	default:
		//JASSERT(MFalse);
	    return LI_ERR_IMAGE_FORMAT;
	}
}

MRESULT _ReduceYBlock_YUYV(CHL_BLOCK* pblkReduce, 
						  const JOFFSCREEN* pImg)
{
	MRESULT res = LI_ERR_NONE;
	MLong lLeft	= CB_RealL(pblkReduce), lRight = CB_RealR(pblkReduce);
	MLong lTop	= CB_RealT(pblkReduce), lBottom= CB_RealB(pblkReduce);	
	JASSERT(B_BlockType(pblkReduce) == DATA_U8);
	
	GO(ReduceYBlock_YUYV((MUInt8*)pImg->pixelArray.chunky.pPixel, pImg->pixelArray.chunky.dwImgLine, 
		pImg->dwWidth, pImg->dwHeight, lLeft, lRight, lTop, lBottom, 
		(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
		B_BlockLine(pblkReduce)));

	{
		MRECT rtValid;
		rtValid.left = lLeft - CB_BlockL(pblkReduce);
		rtValid.right = B_BlockWidth(pblkReduce) - (CB_BlockR(pblkReduce) - lRight);
		rtValid.top = lTop - CB_BlockT(pblkReduce);
		rtValid.bottom = B_BlockHeight(pblkReduce) - (CB_BlockB(pblkReduce) - lBottom);
		B_MirrorFill((PBLOCK)pblkReduce, &rtValid);
	}
EXT:
	return res;
}

MRESULT _ExpandYBlock_YUYV(JOFFSCREEN* pImg, 
						  const CHL_BLOCK* pblkReduce, 
						  MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask)
{
	MLong lLeft	= CB_ValidL(pblkReduce)*2, lRight = CB_ValidR(pblkReduce)*2;
	MLong lTop	= CB_ValidT(pblkReduce)*2, lBottom= CB_ValidB(pblkReduce)*2;

	JASSERT(lRight/lMskReduceW<=pMask->lWidth && lBottom/lMskReduceH<=pMask->lHeight);	
	JASSERT(lTop/lMskReduceH>=pMask->rcMask.top && lBottom/lMskReduceH<=pMask->rcMask.bottom);
	JASSERT(lLeft/lMskReduceW>=pMask->rcMask.left && lRight/lMskReduceW<=pMask->rcMask.right);
	JASSERT(lLeft>=0 && lRight<=(MLong)pImg->dwWidth && lTop>=0 && lBottom<=(MLong)pImg->dwHeight);
	return ExpandYBlock_YUYV((MUInt8*)pImg->pixelArray.chunky.pPixel, 
		pImg->pixelArray.chunky.dwImgLine, lLeft, lRight, lTop, lBottom, 
		(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
		B_BlockLine(pblkReduce), CB_BlockR(pblkReduce), CB_BlockB(pblkReduce), 
		pMask->pData, pMask->lMaskLine, lMskReduceW, lMskReduceH);
}

MRESULT _ReduceYBlock_Y1VY0U(CHL_BLOCK* pblkReduce, const JOFFSCREEN* pImg)
{
	MRESULT res = LI_ERR_NONE;
	MLong lLeft	= CB_RealL(pblkReduce), lRight = CB_RealR(pblkReduce);
	MLong lTop	= CB_RealT(pblkReduce), lBottom= CB_RealB(pblkReduce);	
	JASSERT(B_BlockType(pblkReduce) == DATA_U8);
	
	GO(ReduceYBlock_Y1VY0U((MUInt8*)pImg->pixelArray.chunky.pPixel, pImg->pixelArray.chunky.dwImgLine, 
		pImg->dwWidth, pImg->dwHeight, lLeft, lRight, lTop, lBottom, 
		(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
		B_BlockLine(pblkReduce)));
	
	{
		MRECT rtValid;
		rtValid.left = lLeft - CB_BlockL(pblkReduce);
		rtValid.right = B_BlockWidth(pblkReduce) - (CB_BlockR(pblkReduce) - lRight);
		rtValid.top = lTop - CB_BlockT(pblkReduce);
		rtValid.bottom = B_BlockHeight(pblkReduce) - (CB_BlockB(pblkReduce) - lBottom);
		B_MirrorFill((PBLOCK)pblkReduce, &rtValid);
	}
EXT:
	return res;
}

MRESULT _ExpandYBlock_Y1VY0U(JOFFSCREEN* pImg, const CHL_BLOCK* pblkReduce, 
							 MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask)
{
	MLong lLeft	= CB_ValidL(pblkReduce)*2, lRight = CB_ValidR(pblkReduce)*2;
	MLong lTop	= CB_ValidT(pblkReduce)*2, lBottom= CB_ValidB(pblkReduce)*2;
	
	JASSERT(lRight/lMskReduceW<=pMask->lWidth && lBottom/lMskReduceH<=pMask->lHeight);	
	JASSERT(lTop/lMskReduceH>=pMask->rcMask.top && lBottom/lMskReduceH<=pMask->rcMask.bottom);
	JASSERT(lLeft/lMskReduceW>=pMask->rcMask.left && lRight/lMskReduceW<=pMask->rcMask.right);
	JASSERT(lLeft>=0 && lRight<=(MLong)pImg->dwWidth && lTop>=0 && lBottom<=(MLong)pImg->dwHeight);
	return ExpandYBlock_Y1VY0U((MUInt8*)pImg->pixelArray.chunky.pPixel, 
		pImg->pixelArray.chunky.dwImgLine, lLeft, lRight, lTop, lBottom, 
		(MUInt8*)B_BlockDataEx(pblkReduce, -CB_BlockL(pblkReduce), -CB_BlockT(pblkReduce)), 
		B_BlockLine(pblkReduce), CB_BlockR(pblkReduce), CB_BlockB(pblkReduce), 
		pMask->pData, pMask->lMaskLine, lMskReduceW, lMskReduceH);
}
#endif  //ENABLE_REDUCE_Y_BLOCK