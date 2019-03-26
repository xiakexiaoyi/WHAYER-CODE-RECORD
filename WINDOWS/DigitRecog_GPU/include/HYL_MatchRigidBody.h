#ifndef _HYL_MATCHRIDGIDBODY_H
#define _HYL_MATCHRIDGIDBODY_H

#include"amcomdef.h"



#ifdef __cplusplus
extern "C" {
#endif

	HYLAPI(MHandle)  HYL_MemMgrCreate(MVoid * pMem, MLong lMemSize);
	HYLAPI(MVoid) HYL_MemMgrDestroy(MHandle hMemMgr);
	HYLAPI(MRESULT) HYL_MatchInit(MHandle hMemMgr, MHandle *pHYEDHandle);
	HYLAPI(MRESULT) HYL_MatchUninit(MHandle HYEDHandle);
	HYLAPI(MRESULT) HYL_TrainTemplateFromMask(MHandle hHYMRDHandle,HYL_PIMAGES pImage,HYL_PIMAGES pMask,MChar *pClassName,MLong lParma);
	HYLAPI(MRESULT) HYL_GetDashboard (MHandle hHYMRDHandle,HYL_PIMAGES pImage,MChar *pClassName,MDouble threshold,PMPOINT offset);
	HYLAPI(MRESULT) HYL_GetDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem);
	HYLAPI(MRESULT) HYL_SaveDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize);
	HYLAPI(MRESULT) HYL_GetTemplateFromText (MHandle hHYMRDHandle,char* path);
	HYLAPI(MRESULT) HYL_SaveTDescriptorsGroup (MHandle hHYMRDHandle,const char *path);

#ifdef __cplusplus
}
#endif

#endif//_HYL_MATCHRIDGIDBODY_H