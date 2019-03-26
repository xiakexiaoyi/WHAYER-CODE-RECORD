#ifndef _HYL_MATCHRIDGIDBODY_H
#define _HYL_MATCHRIDGIDBODY_H

#include"amcomdef.h"



#ifdef __cplusplus
extern "C" {
#endif
	MHandle  HYL_MemMgrCreate (MVoid * pMem, MLong lMemSize);
	MVoid HYL_MemMgrDestroy (MHandle hMemMgr);
	MRESULT HYL_MatchInit(MHandle hMemMgr, MHandle *pHYEDHandle);
	MRESULT HYL_MatchUninit(MHandle HYEDHandle);
	MRESULT HYL_TrainTemplateFromMask(MHandle hHYMRDHandle,HYL_PIMAGES pImage,HYL_PIMAGES pMask,MChar *pClassName,MLong lParma);
	MRESULT HYL_GetDashboard (MHandle hHYMRDHandle,HYL_PIMAGES pImage,MChar *pClassName,MDouble threshold,PMPOINT offset);
	MRESULT HYL_GetDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem);
	MRESULT HYL_SaveDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize);
	MRESULT HYL_GetTemplateFromText (MHandle hHYMRDHandle,char* path);
	MRESULT HYL_SaveTDescriptorsGroup (MHandle hHYMRDHandle,const char *path);

#ifdef __cplusplus
}
#endif

#endif//_HYL_MATCHRIDGIDBODY_H
