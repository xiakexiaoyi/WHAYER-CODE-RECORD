/*!
* \file Linoise.h
* \brief  the function related to noiselevel 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#if !defined(_LI_NOISE_H_)
#define _LI_NOISE_H_


#include "liblock.h"
#include <stdio.h>

/************************************************************************/
#define NoiseLevel  			PX(NoiseLevel)
#define  Convmtx				PX(Convmtx)
#define  WeakTextSEL		PX(WeakTextSEL)
#define NoiseLevelFrames PX(NoiseLevelFrames)
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	

 
MRESULT NoiseLevel( MHandle hMemMgr, MFloat* Noisel,PBLOCKEXT Blocksrc,MLong patchsize,MLong decim,MFloat condf,MLong itr);
MRESULT Convmtx(PBLOCK Blockres, PBLOCK BlockA,MLong patchsize);
MRESULT WeakTextSEL(PBLOCK blockres1,PBLOCK blockres2,MLong* EffWidth,PBLOCK blocksrc1,PBLOCK blocksrc2,MFloat Threshhold);
MRESULT NoiseLevelFrames(MHandle hMemMgr,MFloat* NoiseL,PBLOCK blocksrc1,PBLOCK blocksrc2);
#ifdef __cplusplus
}
#endif
#endif // _LINOISE_H
