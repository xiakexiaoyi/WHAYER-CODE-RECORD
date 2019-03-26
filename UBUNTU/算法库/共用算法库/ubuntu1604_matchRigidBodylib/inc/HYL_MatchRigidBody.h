#ifndef _HYL_MATCHRIDGIDBODY_H
#define _HYL_MATCHRIDGIDBODY_H

#include"amcomdef.h"
#if defined(_MSC_VER)
#define YOLOV2DLL_API __declspec(dllexport)
#else
#define YOLOV2DLL_API __attribute ((visibility("default")))
#endif


#ifdef __cplusplus
extern "C" {
#endif
	YOLOV2DLL_API MHandle  HYL_MemMgrCreate (MVoid * pMem, MLong lMemSize);
	YOLOV2DLL_API MVoid HYL_MemMgrDestroy (MHandle hMemMgr);
	YOLOV2DLL_API MRESULT HYL_MatchInit(MHandle hMemMgr, MHandle *pHYEDHandle);
	YOLOV2DLL_API MRESULT HYL_MatchUninit(MHandle HYEDHandle);
	YOLOV2DLL_API MRESULT HYL_TrainTemplateFromMask(MHandle hHYMRDHandle,HYL_PIMAGES pImage,HYL_PIMAGES pMask,MChar *pClassName,MLong lParma);
	YOLOV2DLL_API MRESULT HYL_GetDashboard (MHandle hHYMRDHandle,HYL_PIMAGES pImage,MChar *pClassName,MDouble threshold,PMPOINT offset);
	YOLOV2DLL_API MRESULT HYL_GetDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem);
	YOLOV2DLL_API MRESULT HYL_SaveDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize);
	YOLOV2DLL_API MRESULT HYL_GetTemplateFromText (MHandle hHYMRDHandle,char* path);
	YOLOV2DLL_API MRESULT HYL_SaveTDescriptorsGroup (MHandle hHYMRDHandle,const char *path);

#ifdef __cplusplus
}
#endif

#endif//_HYL_MATCHRIDGIDBODY_H
