#ifndef MatchRigidBody_H
#define MatchRigidBody_H
#include "liblock.h"
#include "licomdef.h"

typedef struct tagSize
{
	MLong lWidth;
	MLong lHeight;
}TSize;
typedef struct tagOutParam
{
	MLong lAngle;
	MLong lHeight;
	MLong lWidth;
	MLong lX;
	MLong lY;
	MLong lIndex;
	MLong lClassNumber;
}TOutParam,*PTOutParam;

/*描述符单元*/
typedef struct tagDescriptor 
{
	MLong lWidth;
	MLong lHeight;
	MLong lRegion;
	MLong lSample;
	MLong lAngle;
	MLong lDescriptorIndex;
	MLong lNumber;
	MUInt8 *pData;	
}TDescriptor,*PTDescriptor;

/*类描述符*/
typedef struct tagDescriptorClass
{
	MLong  lNumDescriptors;
	MLong lClassIndex;	
	MByte pClassName[16];
	PTDescriptor ptDescriptor;
}TDescriptorClass,*PTDescriptorClass;

/*类描述符的集合*/
typedef struct tagTDescriptorsGroup
{
	MLong lNumClass;
	PTDescriptorClass ptDescriptorClass;	
}TDescriptorsGroup,*PTDescriptorsGroup;



#define GetTemplate	            PX(GetTemplate)
#define TrainTemplateMethod	    PX(TrainTemplateMethod)
#define MatchRigidBody	        PX(MatchRigidBody)
#define GetTemp	                PX(GetTemp)
////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
MRESULT GetTemplate(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask);
//MRESULT TrainTemplate(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask,PTDescriptorClass ptDescriptorClass);
MRESULT TrainTemplateMethod(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask,MLong lParam,PTDescriptorClass ptDescriptorClass);
//MRESULT GetTemplateFromText(MHandle hMemMgr,char* path,TDescriptorNum *pDescriptorNum);
MRESULT MatchRigidBody(MHandle hMemMgr,BLOCK *pImage,PTDescriptorClass ptDescriptorClass,MDouble threshold,PTOutParam ptOutParam);
//MVoid WriteTDescriptorNum(const char *path,TDescriptorNum *tDescriptorNum);
MVoid GetTemp(MHandle hMemMgr,BLOCK *pImage,TOutParam tOutParam);
#ifdef __cplusplus
}
#endif
#endif