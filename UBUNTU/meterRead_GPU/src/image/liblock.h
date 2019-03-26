#if !defined(_LI_BLOCK_H_)
#define _LI_BLOCK_H_

#include "licomdef.h"
#include "limem.h"
#include "limask.h"

#define MAX_CHANNEL_NUM		3

#ifdef __cplusplus
extern "C" {
#endif	

typedef MRECT	JRECT_EXT;
typedef PMRECT	PJRECT_EXT;
typedef struct tag_BLOCK_SIZE{
	MLong lWidth, lHeight;
} JSIZE, *PJSIZE, JSTEP, *PJSTEP;

//The basic block type
typedef struct tag_BLOCK_EXIF{
	JTYPE_DATA_A typeDataA;		
	MLong lWidth, lHeight, lBlockLine;
}BLOCKEXIF, *PBLOCKEXIF;
typedef struct tag_BLOCK {
	MVoid *pBlockData;
	JTYPE_DATA_A typeDataA;		
	MLong lWidth, lHeight, lBlockLine;
} BLOCK, *PBLOCK;
//The extern block type
typedef struct tag_BLOCKEXT{
	BLOCK block;	
	JRECT_EXT ext; 	
} BLOCKEXT, *PBLOCKEXT;
//Define the block in a channel
typedef struct tag_BLOCKPOS{
	MLong lValidL, lValidT;
	MLong lImgW, lImgH;
}BLOCKPOS, *PBLOCKPOS;
typedef struct tag_ChlBlock{
	BLOCKEXT blockExt;
	MLong lValidL, lValidT;
	MLong lImgW, lImgH;
} CHL_BLOCK, *PCHL_BLOCK;

/************************************************************************/
#define SetRectExt			PX(SetRectExt)
#define B_Init_Ex			PX(B_Init_Ex)
#define B_Create			PX(B_Create)
#define B_Release			PX(B_Release)
#define B_Init				PX(B_Init)	
#define B_UpdateBlock		PX(B_UpdateBlock)
#define BE_Init_Ex			PX(BE_Init_Ex)
#define BE_Create			PX(BE_Create)
#define BE_Release			PX(BE_Release)
#define BE_Init				PX(BE_Init)
#define BE_MirrorFill		PX(BE_MirrorFill)
#define BE_ValidBlock		PX(BE_ValidBlock)
#define CB_Init_Ex			PX(CB_Init_Ex)
#define CB_Create			PX(CB_Create)
#define CB_Release			PX(CB_Release)
#define CB_Init				PX(CB_Init)
#define CB_UpdateValid		PX(CB_UpdateValid)
#define CB_LoadBlock		PX(CB_LoadBlock)
#define CB_OffSetValidRect	PX(CB_OffSetValidRect)
#define CB_AutoMirrorFill	PX(CB_AutoMirrorFill)
#define MirrorFill			PX(MirrorFill)
#define ReduceYBlock		PX(ReduceYBlock)
#define ExpandYBlock		PX(ExpandYBlock)
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
JRECT_EXT SetRectExt(MLong lExt); 

//////////////////////////////////////////////////////////////////////////
MRESULT B_Init_Ex(MHandle hMemMgr, PBLOCK pBlock, 
				  JTYPE_DATA_A typeDataA, 
				  MLong lWidth, MLong lHeight);
MRESULT B_Create(MHandle hMemMgr, PBLOCK pBlock, 
				 JTYPE_DATA_A typeDataA, 
				 MLong lWidth, MLong lHeight);
MVoid	B_Release(MHandle hMemMgr, PBLOCK pBlock);

MVoid	B_Init(PBLOCK pBlock, JTYPE_DATA_A typeDataA, 
			   MLong lWidth, MLong lHeight);

MVoid	B_UpdateBlock(PBLOCK pBlockRlt, MVoid *pImgData, MLong lImgLine, 
					  JTYPE_DATA_A typeImg, MLong lChannelNum, MLong lChannelCur, 
					  MLong lOffImgX, MLong lOffImgY, MBool bLoadFromImg);

#define B_MirrorFill(pBlock, pRtValid)	 MirrorFill(B_BlockData(pBlock),	\
	B_BlockWidth(pBlock), B_BlockHeight(pBlock),							\
	B_BlockLine(pBlock), B_BlockType(pBlock), pRtValid);

#define B_BlockLine(pBlock)		((PBLOCK)(pBlock))->lBlockLine
#define B_BlockWidth(pBlock)	((PBLOCK)(pBlock))->lWidth
#define B_BlockHeight(pBlock)	((PBLOCK)(pBlock))->lHeight
#define B_BlockTypeA(pBlock)	((PBLOCK)(pBlock))->typeDataA
#define B_BlockType(pBlock)		IF_DATA_TYPE(B_BlockTypeA(pBlock))
#define B_BlockData(pBlock)		((PBLOCK)(pBlock))->pBlockData
#define B_BlockDataEx(pBlock, lOffX, lOffY)	OffSetImg(B_BlockData(pBlock),		\
					  lOffX, lOffY, B_BlockLine(pBlock), B_BlockType(pBlock))

#define B_Set(pBlock, val) SetImgMem(B_BlockData(pBlock),						\
		B_BlockLine(pBlock)*IF_DATA_BYTES(B_BlockType(pBlock)),					\
		B_BlockWidth(pBlock)*IF_DATA_BYTES(B_BlockType(pBlock)),				\
		B_BlockHeight(pBlock), val, MUInt8)
#define B_SameSize(pBlock1, pBlock2) (											\
	(((PBLOCK)pBlock1)->lWidth == ((PBLOCK)pBlock2)->lWidth)					\
	&& (((PBLOCK)pBlock1)->lHeight == ((PBLOCK)pBlock2)->lHeight)				\
	&& (IF_DATA_BYTES(((PBLOCK)pBlock1)->typeDataA)								\
		== IF_DATA_BYTES(((PBLOCK)pBlock2)->typeDataA)) )
#define B_Cpy(pBlockRlt, pBlockSrc)	{											\
	JASSERT(B_SameSize(pBlockSrc, pBlockRlt));									\
	CpyImgMem3(((PBLOCK)pBlockRlt)->pBlockData, ((PBLOCK)pBlockRlt)->lBlockLine,\
		((PBLOCK)pBlockSrc)->pBlockData, ((PBLOCK)pBlockSrc)->lBlockLine,		\
		((PBLOCK)pBlockSrc)->lWidth, ((PBLOCK)pBlockSrc)->lHeight,				\
		((PBLOCK)pBlockSrc)->typeDataA); }

#ifdef ENABLE_DEBUG
#define PrintBlockBmp(pBlock)												\
	PrintBmp(((PBLOCK)(pBlock))->pBlockData, ((PBLOCK)(pBlock))->lBlockLine,\
		((PBLOCK)(pBlock))->typeDataA & ~0x100,								\
		((PBLOCK)(pBlock))->lWidth, ((PBLOCK)(pBlock))->lHeight, 1)
#define PrintBlockTxt(pBlock)												\
	PrintChannel(((PBLOCK)(pBlock))->pBlockData,							\
		((PBLOCK)(pBlock))->lBlockLine,	((PBLOCK)(pBlock))->typeDataA,		\
		((PBLOCK)(pBlock))->lWidth,	((PBLOCK)(pBlock))->lHeight, 1, 0)
#else
#define PrintBlockBmp(pBlock)
#define PrintBlockTxt(pBlock)
#endif
//////////////////////////////////////////////////////////////////////////
//extern block
MRESULT	BE_Init_Ex(MHandle hMemMgr, PBLOCKEXT pBlockExt, 
				   JTYPE_DATA_A typeDataA,  PJRECT_EXT pRectExt, 
				   MLong lWidth, MLong lHeight);
MRESULT	BE_Create(MHandle hMemMgr, PBLOCKEXT pBlockExt, 
				   JTYPE_DATA_A typeDataA,  PJRECT_EXT pRectExt, 
				   MLong lWidth, MLong lHeight);
MVoid	BE_Release(MHandle hMemMgr, PBLOCKEXT pBlockExt);

MVoid	BE_Init(PBLOCKEXT pBlockExt, JTYPE_DATA_A typeDataA,  
				PJRECT_EXT pRectExt, MLong lWidth, MLong lHeight);

MVoid	BE_MirrorFill(PBLOCKEXT pBlockExt);
BLOCK	BE_ValidBlock(const BLOCKEXT* pBlockExt);

#define	BE_LeftExt(pBlockExt)		((PBLOCKEXT)(pBlockExt))->ext.left
#define	BE_TopExt(pBlockExt)		((PBLOCKEXT)(pBlockExt))->ext.top
#define	BE_RightExt(pBlockExt)		((PBLOCKEXT)(pBlockExt))->ext.right
#define	BE_BottomExt(pBlockExt)		((PBLOCKEXT)(pBlockExt))->ext.bottom

#define 	BE_ValidData(pBlockExt)		(MVoid*)(							\
	(MUInt8*)B_BlockData(pBlockExt) + (BE_LeftExt(pBlockExt)				\
	+ BE_TopExt(pBlockExt) * B_BlockLine(pBlockExt))						\
	* IF_DATA_BYTES(B_BlockType(pBlockExt))	)							
#define		BE_ValidW(pBlockExt)	(B_BlockWidth(pBlockExt)				\
	- BE_LeftExt(pBlockExt) - BE_RightExt(pBlockExt))
#define 	BE_ValidH(pBlockExt)	(B_BlockHeight(pBlockExt)				\
	- BE_TopExt(pBlockExt) - BE_BottomExt(pBlockExt))
//////////////////////////////////////////////////////////////////////////
//the block in a channel
MRESULT CB_Init_Ex(MHandle hMemMgr, PCHL_BLOCK pChlBlock, 
					JTYPE_DATA_A typeDataA, 
					const PJRECT_EXT pExt, const PJSIZE pValidSize, 
					MLong lImgW, MLong lImgH);
MRESULT CB_Create(MHandle hMemMgr, PCHL_BLOCK pChlBlock, 
				  JTYPE_DATA_A typeDataA, 
				  const PJRECT_EXT pExt, const PJSIZE pValidSize, 
				  MLong lImgW, MLong lImgH);
MVoid	CB_Release(MHandle hMemMgr, PCHL_BLOCK pChlBlock);

MVoid	CB_Init(PCHL_BLOCK pChlBlock, JTYPE_DATA_A typeDataA, 
				const PJRECT_EXT pExt, const PJSIZE pValidSize, 
				MLong lImgW, MLong lImgH);

#define CB_BlockL(pChlBlock)	(((PCHL_BLOCK)(pChlBlock))->lValidL			\
	- ((PCHL_BLOCK)(pChlBlock))->blockExt.ext.left)
#define CB_BlockT(pChlBlock)	(((PCHL_BLOCK)(pChlBlock))->lValidT			\
	- ((PCHL_BLOCK)(pChlBlock))->blockExt.ext.top)
#define CB_BlockR(pChlBlock)	(CB_BlockL(pChlBlock)						\
	+ ((PCHL_BLOCK)(pChlBlock))->blockExt.block.lWidth)
#define CB_BlockB(pChlBlock)	(CB_BlockT(pChlBlock)						\
	+ ((PCHL_BLOCK)(pChlBlock))->blockExt.block.lHeight)

#define CB_RealData(pChlBlock)	OffSetImg(B_BlockData(pChlBlock),			\
	CB_RealL(pChlBlock)-CB_BlockL(pChlBlock),								\
	CB_RealT(pChlBlock)-CB_BlockT(pChlBlock),								\
	B_BlockLine(pChlBlock), B_BlockType(pChlBlock))
#define CB_RealR(pChlBlock)	MIN(CB_BlockR(pChlBlock), ((PCHL_BLOCK)(pChlBlock))->lImgW)
#define CB_RealB(pChlBlock)	MIN(CB_BlockB(pChlBlock), ((PCHL_BLOCK)(pChlBlock))->lImgH)
#define CB_RealT(pChlBlock)	MAX(0, CB_BlockT(pChlBlock))
#define CB_RealL(pChlBlock)	MAX(0, CB_BlockL(pChlBlock))
#define CB_RealW(pChlBlock)	(CB_RealR(pChlBlock) - CB_RealL(pChlBlock))
#define CB_RealH(pChlBlock)	(CB_RealB(pChlBlock) - CB_RealT(pChlBlock))

#define	CB_ValidL(pChlBlock)	((PCHL_BLOCK)(pChlBlock))->lValidL
#define	CB_ValidT(pChlBlock)	((PCHL_BLOCK)(pChlBlock))->lValidT
#define	CB_ValidR(pChlBlock)	(CB_ValidL(pChlBlock) + BE_ValidW(pChlBlock))
#define	CB_ValidB(pChlBlock)	(CB_ValidT(pChlBlock) + BE_ValidH(pChlBlock))

MVoid CB_UpdateValid(PCHL_BLOCK pChlBlock, MBool bLoadFromImg,
					 MVoid *pImgData, MLong lImgLine, JTYPE_DATA_A typeImg, 
					 MLong lChannelNum, MLong lChannelCur);
MVoid CB_LoadBlock(PCHL_BLOCK pChlBlock, 
				   MVoid *pImgData, MLong lImgLine, JTYPE_DATA_A typeImg, 
				   MLong lChannelNum, MLong lChannelCur);

MVoid CB_OffSetValidRect(PCHL_BLOCK pChlBlock, MLong lValidL, MLong lValidT);

MVoid CB_AutoMirrorFill(PCHL_BLOCK pChlBlock, MLong lROffSet, MLong lBOffSet);

//////////////////////////////////////////////////////////////////////////////////////////
//Other operation
MVoid	MirrorFill(MVoid *pBlockData, MLong lWidth, MLong lHeight, 
				   MLong lBlockLine, JTYPE_DATA typeData, PMRECT prtValid);

//////////////////////////////////////////////////////////////////////////////////////////
MRESULT ReduceYBlock(MHandle hMemMgr, CHL_BLOCK* pblkReduce, const JOFFSCREEN* pImg);
MRESULT ExpandYBlock(JOFFSCREEN* pImg, const CHL_BLOCK* pblkReduce, 
					 MLong lMskReduceW, MLong lMskReduceH, const JMASK* pMask);

#ifdef __cplusplus
}
#endif

#endif // !defined(_LI_BLOCK_H_)