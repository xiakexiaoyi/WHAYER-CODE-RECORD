#if !defined(_LI_MEM_H_)
#define _LI_MEM_H_

#include "litrimfun.h"

#include "amcomdef.h"
#include "lierrdef.h"

//////////////////////////////////////////////////////////////////////////
#define JMemAlloc				PX(JMemAlloc)
#define JMemFree				PX(JMemFree)
#define JMemCpy					PX(JMemCpy)
#define JMemSet					PX(JMemSet)
#define JImgMemCpy				PX(JImgMemCpy)
#define JMemLength				PX(JMemLength)
#define JMemInfoStatic			PX(JMemInfoStatic)

#define JMemMgrCreate			PX(JMemMgrCreate)
#define JMemMgrDestroy			PX(JMemMgrDestroy)

//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
MHandle JMemMgrCreate(MVoid* pMem, MLong lMemSize);
MVoid	JMemMgrDestroy(MHandle hMemMgr);

MVoid*	JMemAlloc(MHandle hMemMgr, MLong lSize);
MVoid	JMemFree(MHandle hMemMgr, MVoid* pMem);
MVoid	JMemCpy(MVoid* pDst, const MVoid* pSrc, MLong lSize);
MVoid	JMemSet(MVoid* pDst, MLong lVal, MLong lSize);
MLong	JMemLength(MLong lWidth);
MVoid	JMemInfoStatic(MHandle hContext, MDWord* pdwTotalSize, MDWord* pdwUsedSize);

MVoid	JImgMemCpy(MVoid* pDst, MLong lDstLineBytes, const MVoid* pSrc, 
				   MLong lSrcLineBytes, MLong lWidth, MLong lHeight);

#if defined OPTIMIZATION_THREAD && defined PLATFORM_WIN32
MVoid	JMTMemMgrEnter();
MVoid	JMTMemMgrLeave();
#else
#define JMTMemMgrEnter()	
#define JMTMemMgrLeave()	
#endif
#ifdef __cplusplus
}
#endif

//Alloc/free vect memory
#define AllocVectMem(hMemMgr, pMem, len, TYPE)						\
{																	\
	JASSERT((len) > 0);												\
	JASSERT((pMem) == MNull);										\
	JMTMemMgrEnter();												\
	(pMem) = (TYPE*)JMemAlloc(hMemMgr, (len)*sizeof(TYPE));			\
	JMTMemMgrLeave();												\
	if((pMem) == MNull)												\
	{																\
		res = LI_ERR_ALLOC_MEM_FAIL;								\
		goto EXT;													\
	}																\
}
#define FreeVectMem(hMemMgr, pMem)									\
{																	\
	JMTMemMgrEnter();												\
	if((pMem) != MNull)												\
		JMemFree(hMemMgr,pMem);										\
	(pMem) = MNull;													\
	JMTMemMgrLeave();												\
}
#define SetVectMem(pMem, len, VALUE, TYPE)		JMemSet(pMem,VALUE,(len)*sizeof(TYPE))
#define CpyVectMem(pMem, pMemSrc,len,TYPE)		JMemCpy(pMem,pMemSrc,(len)*sizeof(TYPE))
#define SetVectZero(pDst, lSize)				JMemSet(pDst,0,lSize)		

#define AllocVectMemEx(hMemMgr, pMem, sizeBytes, TYPE)						\
{																			\
	if(sizeBytes > 0)														\
	{																		\
		(pMem) = (TYPE*)JMemAlloc(hMemMgr, sizeBytes);						\
		if((pMem) == MNull)													\
		{																	\
			res = LI_ERR_ALLOC_MEM_FAIL;									\
			goto EXT;														\
		}																	\
	}																		\
}
		
//////////////////////////////////////////////////////////////////////////
//Set/Copy/Clone image memory	
#define OffSetImg(pImgData, lOffsetX, lOffsetY, lImgLine, typeData)			\
	( (MByte*)(pImgData) + ((lOffsetX)+(lOffsetY)*(lImgLine))*IF_DATA_BYTES(typeData) )
#define SetImgMem(pMem, imgLine, width, height, initvalue, TYPE)			\
{																			\
	TYPE *_pDstCur = (TYPE*)(pMem);											\
	MLong _i, _lDstLine = imgLine;											\
	MLong _lWidth = width;													\
	for(_i=height; _i!=0; _i--, _pDstCur+=_lDstLine)						\
		SetVectMem(_pDstCur, _lWidth, initvalue, TYPE);						\
}

#define CpyImgMem(pDst, dstLine, pSrc, srcLine, width,height,TYPE)			\
	JImgMemCpy(pDst, dstLine, pSrc, srcLine, (width)*sizeof(TYPE), height)
#define CpyImgMem2															\
	JImgMemCpy
#define CpyImgMem3(pDst, dstLine, pSrc, srcLine, width,height, typeData)	\
	JImgMemCpy(pDst, (dstLine)*IF_DATA_BYTES(typeData),	pSrc, (srcLine)*IF_DATA_BYTES(typeData),	(width)*IF_DATA_BYTES(typeData), height)

#define CpyImgMem_Ex(pDst, offdstX, offdstY, dstLine, pSrc, offsrcX, offsrcY,	srcLine, width, height, typeData)	\
	CpyImgMem3(OffSetImg(pDst, offdstX, offdstY, dstLine, typeData), dstLine, OffSetImg(pSrc, offsrcX, offsrcY, srcLine, typeData), srcLine, width, height, typeData)

//////////////////////////////////////////////////////////////////////////
//Variant data alloc or free
#ifdef OPTIMIZATION_CUDA
#include "limath.h"
#include <memory.h>
#include <malloc.h>
	#define AllocVarMem(hMemMgr, pMem, len, TYPE)						\
	{																	\
		JASSERT(len > 0);												\
		(pMem) = (TYPE*)malloc((len)*sizeof(TYPE));				\
		if((pMem) == MNull)												\
		{																\
			res = LI_ERR_ALLOC_MEM_FAIL;								\
			goto EXT;													\
		}																\
		memset(pMem, 0, (len)*sizeof(TYPE));							\
	}
	#define FreeVarMem(hMemMgr, pMem)									\
	{																	\
		if((pMem) != MNull)												\
			free(pMem);													\
		(pMem) = MNull;													\
	}
	#define SetVarMem(pMem, len, VALUE, TYPE)	memset(pMem,VALUE,(len)*sizeof(TYPE))
	#define CpyVarMem(pMem, pMemSrc,len,TYPE)	memcpy(pMem,pMemSrc,(len)*sizeof(TYPE))
#else
	#define AllocVarMem(hMemMgr, pMem, num, TYPE)						\
		AllocVectMem(hMemMgr, pMem, (num), TYPE); SetVectZero(pMem, sizeof(TYPE)*(num))
	#define FreeVarMem(hMemMgr, pMem)   FreeVectMem(hMemMgr, pMem)
	#define SetVarMem(pMem, len, VALUE, TYPE)	JMemSet(pMem,VALUE,(len)*sizeof(TYPE))
	#define CpyVarMem(pMem, pMemSrc,len,TYPE)	JMemCpy(pMem,pMemSrc,(len)*sizeof(TYPE))
#endif

//////////////////////////////////////////////////////////////////////////
#if defined OPTIMIZATION_FASTMEMORY && defined OPTIMIZATION_SOFTUNE
	#define AllocFastMemory(hMemMgr, pMem, sizeBytes)						\
		{	(MVoid*)(pMem) = (MVoid*)0x0001F000;							\
			JASSERT(sizeBytes<=(0x00021FFF-0x0001F000));	}
	#define FreeFastMemory(hMemMgr, pMem)
#else
	#define AllocFastMemory(hMemMgr, pMem, sizeBytes)						\
		AllocVectMem(hMemMgr, pMem, sizeBytes, MByte)
	#define FreeFastMemory		FreeVectMem
#endif	//OPTIMIZATION_FASTMEMORY

#if defined OPTIMIZATION_ENABLE_CACHE && defined PLATFORM_SOFTUNE
	#include "fr.h" 
	#define DCACHE_ENABLE  (IO_FR.DCHCR.byte = 3)    //D-Cache flush & Enable 
	#define DCACHE_DISABLE (IO_FR.DCHCR.byte = 2)    //D-Cache Disable 
	#define EnableCache()	__DI(); DCACHE_ENABLE
	#define DisableCache()	DCACHE_DISABLE; __EI()
#else
	#define EnableCache()
	#define DisableCache()
#endif	//OPTIMIZATION_ENABLE_CACHE

#endif // !defined(_LI_MEM_H_)
