#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif

#include "limem.h"
#include "litrimfun.h"
#include "lidebug.h"
#include "limath.h"
#include "litimer.h"

JSTATIC MVoid*	JMemAllocStatic(MHandle hContext, MLong lSize);
JSTATIC MVoid	JMemFreeStatic(MHandle hContext, MVoid *pMem);
//////////////////////////////////////////////////////////////////////////
#ifdef OPTIMIZATION_CUDA
#include "limem_cuda.h"
#include <cutil.h>

MVoid*	JMemAlloc(MHandle hMemMgr, MLong lSize){
	if(hMemMgr != MNull)
	{
		return JMemAllocStatic(hMemMgr, lSize);
	}
	else
	{
		MVoid *pMem = MNull;
		cudaMalloc((MVoid**)&pMem, lSize);
		return pMem;
	}
}
MVoid	JMemFree(MHandle hMemMgr, MVoid* pMem){
	if(hMemMgr != MNull)
	{
		JMemFreeStatic(hMemMgr, pMem);
	}
	else
	{
		cudaFree(pMem);
	}
}
MLong	JMemLength(MLong lWidth){
	return CEIL_4(lWidth);
}

MVoid JMemSet(MVoid* pDst, MLong lVal, MLong lSize){	
	JMemSet_CUDA(pDst, lVal, lSize);
}
MVoid JMemCpy(MVoid* pDes, const MVoid* pSrc, MLong len){
	JMemCpy_CUDA(pDes, pSrc, len);
}

#elif defined OPTIMIZATION_SSE

#include "limem_sse.h"
#include <malloc.h>
#include <memory.h>
MVoid*	JMemAlloc(MHandle hMemMgr, MLong lSize){
	if(hMemMgr == MNull)
		return _aligned_malloc(lSize, 16);
	else
		return JMemAllocStatic(hMemMgr, lSize);
}
MVoid	JMemFree(MHandle hMemMgr, MVoid* pMem){
	if(hMemMgr == MNull)
		_aligned_free(pMem);
	else
		JMemFreeStatic(hMemMgr, pMem);
}
MLong	JMemLength(MLong lWidth){
	return CEIL_16(lWidth);
}
MVoid	JMemCpy(MVoid* pDst, const MVoid* pSrc, MLong lSize){
	MMemCpy_SSE2(pDst, pSrc, lSize);
}
MVoid   JMemSet(MVoid* pDst, MLong lVal, MLong lSize){
	if(lVal == 0)
		MemSetZero_SSE2(pDst, lSize);
	else
		memset(pDst, lVal, lSize);
}

#elif defined ENABLE_PLATFORM_DEPENDENT	//(!OPTIMIZATION_SSE && !OPTIMIZATION_CUDA) 

#include "ammem.h"
MVoid*	JMemAlloc(MHandle hMemMgr, MLong lSize){
    return MMemAlloc(hMemMgr, lSize);
}
MVoid	JMemFree(MHandle hMemMgr, MVoid* pMem){
    MMemFree(hMemMgr, pMem);
}
MLong	JMemLength(MLong lWidth){
	return CEIL_4(lWidth);
}
MVoid	JMemCpy(MVoid* pDst, const MVoid* pSrc, MLong lSize){
    MMemCpy(pDst, pSrc, lSize);
}
MVoid   JMemSet(MVoid* pDst, MLong lVal, MLong lSize){
    MMemSet(pDst, (MByte)lVal, lSize);
}

#else //(!ENABLE_PLATFORM_DEPENDENT	&& !OPTIMIZATION_SSE && !OPTIMIZATION_CUDA)

#include <stdlib.h>
#include <string.h>

MVoid*	JMemAlloc(MHandle hMemMgr, MLong lSize){
	if(hMemMgr != MNull)
		return JMemAllocStatic(hMemMgr, lSize);
	else
		return malloc(lSize);
}
MVoid	JMemFree(MHandle hMemMgr, MVoid* pMem){
	if(hMemMgr != MNull)
		JMemFreeStatic(hMemMgr, pMem);
	else
		free(pMem);
}
MLong	JMemLength(MLong lWidth){
	return CEIL_4(lWidth);
}
MVoid	JMemCpy(MVoid* pDst, const MVoid* pSrc, MLong lSize){
	memcpy(pDst, pSrc, lSize);
}
MVoid   JMemSet(MVoid* pDst, MLong lVal, MLong lSize){
	memset(pDst, lVal, lSize);
}

#endif	//defined(OPTIMIZATION_CUDA)

//////////////////////////////////////////////////////////////////////////
extern MLong	Im_Xch_Copy3(MDWord src_addr, MUInt16 src_gl_width, MUInt16 dst_gl_width, 
							 MDWord dst_addr, MUInt16 width, MUInt16 lines, MLong Xch);
JSTATIC MVoid	_JImgMemCpy(MVoid* pDst, MLong lDstLineBytes, const MVoid* pSrc, 
					 MLong lSrcLineBytes, MLong lWidth, MLong lHeight)
{
	MByte* pDstCur = (MByte*)pDst;
	MByte* pSrcCur = (MByte*)pSrc;
	MLong x;
	if(pDst == pSrc)
		return;
	for(x=lHeight; x!=0; x--, pDstCur+=lDstLineBytes, pSrcCur+=lSrcLineBytes)
		JMemCpy(pDstCur, pSrcCur, lWidth);
}

MVoid	JImgMemCpy(MVoid* pDst, MLong lDstLineBytes, const MVoid* pSrc, 
				   MLong lSrcLineBytes, MLong lWidth, MLong lHeight)
{
	if(pDst == pSrc)
		return;
#if defined CLIP_ZORAN_TW
	DirectDmaBlockCopy( pDst, pSrc, lWidth, lDstLineBytes, lSrcLineBytes, lHeight);
#elif defined OPTIMIZATION_FUJITSU_HW
	if((MDWord)pDst%16==0 && (MDWord)pSrc%16==0)
		Im_Xch_Copy3((MDWord)(pSrc), (MUInt16)(lSrcLineBytes), (MUInt16)(lDstLineBytes), 
			(MDWord)(pDst), (MUInt16)(lWidth), (MUInt16)(lHeight), 0);
	else
		_JImgMemCpy(pDst, lDstLineBytes, pSrc, lSrcLineBytes, lWidth, lHeight);
#elif defined OPTIMIZATION_CUDA
	_JImgMemCpy_CUDA(pDst, lDstLineBytes, pSrc, lSrcLineBytes, lWidth, lHeight);
#else 
	_JImgMemCpy(pDst, lDstLineBytes, pSrc, lSrcLineBytes, lWidth, lHeight);
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined OPTIMIZATION_THREAD && defined PLATFORM_WIN32
#include <Windows.h>
static MLong g_lMTMemMgrNum = 0;
static CRITICAL_SECTION g_MTMemMgr = {0};
MVoid JMTMemMgrInit(){
	if(g_lMTMemMgrNum == 0)
		InitializeCriticalSection(&g_MTMemMgr);
	g_lMTMemMgrNum++;
};			
MVoid JMTMemMgrRelease(){
	if(g_lMTMemMgrNum == 1)
		DeleteCriticalSection(&g_MTMemMgr);
	if(g_lMTMemMgrNum > 0)
		g_lMTMemMgrNum --;
};		
MVoid JMTMemMgrEnter(){
	if(g_lMTMemMgrNum > 0)
		EnterCriticalSection(&g_MTMemMgr);	
};
MVoid JMTMemMgrLeave(){
	if(g_lMTMemMgrNum > 0)
		LeaveCriticalSection(&g_MTMemMgr);
};
#else
#define JMTMemMgrInit()		
#define JMTMemMgrRelease()		
#endif


#ifdef ENABLE_RANDOM_MEMORY_MANAGER
//////////////////////////////////////////////////////////////////////////
#ifdef  OPTIMIZATION_SSE
#define ALIGN_SIZE		16
#else
#define ALIGN_SIZE		4
#endif

typedef struct tag_AM_MemPoolCellHeader { 
	MLong	lSize; 
	MBool	bIsUsed; 
#ifdef  OPTIMIZATION_SSE
	MLong	t1,t2;
#endif
}  AM_MEMPOOLCELLHEADER, *LPAM_MEMPOOLCELLHEADER; 
#define MMPOOL_ISENDNODE(pNode)			((pNode)->lSize == -1)
#define MMPOOL_NODESIZE(pNode)			(MUInt64)((pNode)->lSize + sizeof(AM_MEMPOOLCELLHEADER)) 
#define MMPOOL_GETMEMPOINT(pNode)		(MByte*)((MByte*)(pNode) + sizeof(AM_MEMPOOLCELLHEADER))
#define MMPOOL_GETNEXTNODE(pNode)		(LPAM_MEMPOOLCELLHEADER)((MByte*)(pNode) + MMPOOL_NODESIZE(pNode))
#define MMPOOL_MERGE(pNode1, pNode2)	((pNode1)->lSize += MMPOOL_NODESIZE(pNode2))

MHandle JMemMgrCreate(MVoid *pMem, MLong nMemSize)
{
	AM_MEMPOOLCELLHEADER *pCellHeader = MNull , *pEndNode = MNull; 
	
	if(MNull == pMem) 
		return MNull; 
	
	// create the head cell
	pCellHeader = (LPAM_MEMPOOLCELLHEADER)
		(((MUInt64)pMem + ALIGN_SIZE - 1) & (~(ALIGN_SIZE - 1)));
	nMemSize = nMemSize - ((MUInt64)pCellHeader - (MUInt64)pMem);
	nMemSize &= (~(ALIGN_SIZE - 1));

	nMemSize -= 2 * sizeof(AM_MEMPOOLCELLHEADER);
	if(nMemSize <= 0) 
		return MNull;
	
	pCellHeader->lSize = nMemSize; 
	pCellHeader->bIsUsed = MFalse; 
	
	// create the end cell 
	pEndNode = (LPAM_MEMPOOLCELLHEADER)
		((MByte*)pCellHeader + MMPOOL_NODESIZE(pCellHeader));
	pEndNode->lSize = -1; 
	pEndNode->bIsUsed = MFalse; 

	JMTMemMgrInit();
	
	return (MHandle)pCellHeader;
}

MVoid JMemMgrDestroy(MHandle hMemMgr)
{
	JMTMemMgrRelease();
	return; 
}

MVoid* JMemAllocStatic(MHandle hContext, MLong lSize)
{	
	LPAM_MEMPOOLCELLHEADER pCellHeader = (LPAM_MEMPOOLCELLHEADER)hContext; 
	if( MNull == pCellHeader || lSize <= 0 ) 
		return MNull; 
	
	lSize = (lSize + ALIGN_SIZE - 1) & (~(ALIGN_SIZE - 1));
	
	while(!MMPOOL_ISENDNODE(pCellHeader))	// find one cell
	{
		if(!pCellHeader->bIsUsed && pCellHeader->lSize >= (MLong)lSize) {	// find the free space to alloc
			MLong lUnusedSize = pCellHeader->lSize - lSize - sizeof(AM_MEMPOOLCELLHEADER);
			
			// should add new cell?
			if(lUnusedSize >= 32) {	// add new cell
				LPAM_MEMPOOLCELLHEADER pNewCell = (LPAM_MEMPOOLCELLHEADER)
					(MMPOOL_GETMEMPOINT(pCellHeader) + lSize);	
				pNewCell->lSize = lUnusedSize; 
				pNewCell->bIsUsed = MFalse; 
				
				pCellHeader->lSize = lSize;
			}
			pCellHeader->bIsUsed = MTrue;
			
			JASSERT((MDWord)MMPOOL_GETMEMPOINT(pCellHeader)%ALIGN_SIZE == 0);
			return MMPOOL_GETMEMPOINT(pCellHeader);
		} 
		pCellHeader = MMPOOL_GETNEXTNODE(pCellHeader);	// find next
	}
	return MNull; 
}

MVoid JMemFreeStatic(MHandle hContext, MVoid *pMem)
{
	LPAM_MEMPOOLCELLHEADER pCellHeader = (LPAM_MEMPOOLCELLHEADER)hContext; 
	LPAM_MEMPOOLCELLHEADER pPreHeader = MNull;
	LPAM_MEMPOOLCELLHEADER pNextHeader = MNull; 
	
	if(MNull == pCellHeader || MNull == pMem) 
		return; 
	
	// find the memory to free
	while(MMPOOL_GETMEMPOINT(pCellHeader) != pMem) {
		pPreHeader = pCellHeader; 
		pCellHeader = MMPOOL_GETNEXTNODE(pCellHeader);
	}
	
	pCellHeader->bIsUsed = MFalse;
	
	// try to merge with next cell
	pNextHeader = MMPOOL_GETNEXTNODE(pCellHeader); 
	if(!MMPOOL_ISENDNODE(pNextHeader) && !pNextHeader->bIsUsed)
		MMPOOL_MERGE(pCellHeader, pNextHeader); 

	// try to merge with pre cell 
	if(MNull != pPreHeader && !pPreHeader->bIsUsed)
		MMPOOL_MERGE(pPreHeader, pCellHeader);	
}

MVoid JMemInfoStatic(MHandle hContext, MDWord* pdwTotalSize, MDWord* pdwUsedSize)
{	
	LPAM_MEMPOOLCELLHEADER pCellHeader = (LPAM_MEMPOOLCELLHEADER)hContext; 	
	if( MNull == pCellHeader) 		
		return;
	
	pdwTotalSize[0]    = 0;	
	pdwUsedSize[0]     = sizeof(AM_MEMPOOLCELLHEADER);  //end cell	
	while(!MMPOOL_ISENDNODE(pCellHeader)) // find one cell	
	{		
		if(pCellHeader->bIsUsed)			
			pdwUsedSize[0]     += pCellHeader->lSize + sizeof(AM_MEMPOOLCELLHEADER);		
		else			
		{			
			pdwUsedSize[0]     +=  sizeof(AM_MEMPOOLCELLHEADER);			
			pdwTotalSize[0]    += pCellHeader->lSize;			
		}
		
		pCellHeader = MMPOOL_GETNEXTNODE(pCellHeader); // find next		
	}
	
	pdwTotalSize[0]    += pdwUsedSize[0];	
}

#elif defined ENABLE_SEQUENCE_MEMORY_MANAGER

#define MAX_CUDA_MEM_BLOCKS		256
typedef struct{
	MLong *pBlockBytes;
	MByte *pMem;
	MLong lBlockCur;
	MLong lMaxBytes;
}CUDA_MEM;

MHandle JMemMgrCreate(MVoid *pMem, MLong nMemSize)
{
	CUDA_MEM *pCudaMem = MNull;
	MRESULT res = LI_ERR_NONE;
	AllocVarMem(MNull, pCudaMem, 1, CUDA_MEM);
	AllocVarMem(MNull, pCudaMem->pBlockBytes, MAX_CUDA_MEM_BLOCKS, MLong);
	pCudaMem->pMem = (MByte*)pMem;
	pCudaMem->lMaxBytes = nMemSize;
	JMTMemMgrInit();
EXT:
	return pCudaMem;
}
MVoid JMemMgrDestroy(MHandle hMemMgr)
{
	CUDA_MEM *pCudaMem = (CUDA_MEM *)hMemMgr;
	if(pCudaMem == MNull)
		return;
	FreeVarMem(MNull, pCudaMem->pBlockBytes);
	FreeVarMem(MNull, hMemMgr);
	JMTMemMgrRelease();
	return; 
}

MVoid* JMemAllocStatic(MHandle hMemMgr, MLong lSize)
{	
	CUDA_MEM *pCudaMem = (CUDA_MEM*)hMemMgr;	
	if(pCudaMem->lBlockCur+1 >= MAX_CUDA_MEM_BLOCKS)
		return MNull;
	if(pCudaMem->lBlockCur==0)
		pCudaMem->pBlockBytes[0] = JMemLength((MLong)pCudaMem->pMem) - (MLong)pCudaMem->pMem;	
	pCudaMem->lBlockCur++;	
	lSize = JMemLength(lSize);
	if(lSize + pCudaMem->pBlockBytes[pCudaMem->lBlockCur-1] > pCudaMem->lMaxBytes)
		return MNull;
	pCudaMem->pBlockBytes[pCudaMem->lBlockCur] = lSize + pCudaMem->pBlockBytes[pCudaMem->lBlockCur-1];
	return pCudaMem->pMem + pCudaMem->pBlockBytes[pCudaMem->lBlockCur-1];
}

MVoid JMemFreeStatic(MHandle hMemMgr, MVoid *pMem)
{
	CUDA_MEM *pCudaMem = (CUDA_MEM*)hMemMgr;
	if(pCudaMem->lBlockCur == 0 )
		return;
	JASSERT(pMem==pCudaMem->pMem+pCudaMem->pBlockBytes[pCudaMem->lBlockCur-1]) ;
	pCudaMem->lBlockCur--;	
}

#else	//Platform libary dependence

#include "ammem.h"
MHandle JMemMgrCreate(MVoid *pMem, MLong nMemSize){
	return MMemMgrCreate(pMem, nMemSize);
}
MVoid JMemMgrDestroy(MHandle hMemMgr){
	MMemMgrDestroy(hMemMgr);
}
MVoid *JMemAllocStatic(MHandle hMemMgr, MLong lSize){
	return MMemAlloc(hMemMgr, lSize);
}
MVoid JMemFreeStatic(MHandle hMemMgr, MVoid *pMem){
	MMemFree(hMemMgr, pMem);
}
#endif	//OPTIMIZATION_THREAD
