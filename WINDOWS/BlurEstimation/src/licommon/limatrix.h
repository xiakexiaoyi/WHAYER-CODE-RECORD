/*!
* \file Limatrix.h
* \brief  the function related to matrix 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#if !defined(_LI_MATRIX_H_)
#define _LI_MATRIX_H_

#include "liblock.h"
#include"LiErrFunc.h"
#include <stdio.h>

/************************************************************************/
#define  LiRank		            PX(LiRank)
#define  LiTrace					PX(LiTrace)
#define  LiHtrans			    PX(LiHtrans)
#define LiSigular                 PX(LiSigular)
#define TransPose              PX(TransPose)
#define Litrmul                   PX(Litrmul)
#define LiImTcol                PX(LiImTcol)
#define LiVertCat               PX(LiVertCat)
#define LiSum                    PX(LiSum) 
#define LiSVD                    PX(LiSVD)

/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	


	MRESULT LiRank(MHandle hMemMgr,MLong *rank,PBLOCK BlockSrc);
	MRESULT LiRank_U8(MHandle hMemMgr,MLong *rank,PBLOCK BlockSrc);
	MRESULT LiRank_F32(MHandle hMemMgr,MLong *rank,PBLOCK BlockSrc);
	MRESULT LiTrace(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc);
	MRESULT LiTrace_I8(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc);
	MRESULT LiTrace_U8(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc);
	MRESULT LiTrace_F32(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc);
	MRESULT LiHtrans(PBLOCK BlockA);
	MRESULT LiHtrans_U8(PBLOCK BlockA);
	MRESULT LiHtrans_F32(PBLOCK BlockA);
	MRESULT LiSigular(PBLOCK BlockR,PBLOCK BlockV, PBLOCK Blocksrc);
	MRESULT TransPose(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc);
	MRESULT TransPose_I8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc);
	MRESULT TransPose_U8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc);
	MRESULT TransPose_F32(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc);
	MRESULT TransPoseEXT(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc);
	MRESULT TransPoseEXT_I8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc);
	MRESULT TransPoseEXT_U8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc);
	MRESULT TransPoseEXT_F32(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc);
    MRESULT Litrmul(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT Litrmul_I8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT Litrmul_U8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT Litrmul_F32(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT LitrmulEXT(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2);
	MRESULT LitrmulEXT_I8(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2);
	MRESULT LitrmulEXT_U8(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2);
	MRESULT LitrmulEXT_F32(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2);	
	MRESULT LiImTcol(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height);
	MRESULT LiImTcol_I8(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height);
	MRESULT LiImTcol_U8(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height);
	MRESULT LiImTcol_F32(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height);
	MRESULT LiVertCat(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT LiVertCat_U8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT LiVertCat_I8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT LiVertCat_F32(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2);
	MRESULT LiSum(MFloat *sum,PBLOCK blocksrc);
	MRESULT LiSum_I8(MFloat *sum,PBLOCK blocksrc);
	MRESULT LiSum_U8(MFloat *sum,PBLOCK blocksrc);
	MRESULT LiSum_F32(MFloat *sum,PBLOCK blocksrc);
	MRESULT LiSVD(MHandle hMemMgr,PBLOCK blockres,PBLOCK blocksrc);
	
	static void ppp(PBLOCK blockres,MDouble*e,MDouble* s,MLong top,MLong height,MLong width,MLong m,MLong n);
	static void sss(MDouble fg[2],MDouble cs[2]);
    MRESULT blobSort(MDouble *s, MLong n);
    MRESULT blobSort_U8(MUInt8* s,MLong n);
    MRESULT QuickSort_U8(MUInt8 *p,MLong n);
     static void isplit(MUInt8*p,MLong n,MLong *m);



#ifdef __cplusplus
}
#endif
#endif // _LIIMATRIX_H