#ifndef _LI_FACESKIN_H_
#define _LI_FACESKIN_H_

#include "licomdef.h"
#include "limask.h"
#include "limath.h"
//#include "arcsoft_face_despot.h"

#define MAX_START_POINT   1024
#define  NORM_SIZE_1       300 
#define  NORM_SIZE_2       256

//denominator
#define  FILTER_EXTRA     512
#define  FILTER_LARGE     1024
#define  FILTER_MEDIUM    2048
#define  FILTER_SMALL	  2500

#define  FaceDefreckle			PX(FaceDefreckle)

typedef enum 
{
	DETECTED_FACE = MFalse,
	EXTENDED_FACE = MTrue
} REGION_OPTION;

typedef enum Inner_FaceOrientCode{
	FOC_0			= 0x1,		// 0 degree
	FOC_90			= 0x2,		// 90 degree
	FOC_270			= 0x3,		// 270 degree
	FOC_180			= 0x4		// 180 degree
}FACE_ORIENT;


//****************************Feature Space Definition**********************//
//   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |// 
//   | eye           | eyebrows      | mouth           |nose|others
//     LL  LR  RL  RR  LL LR   RL  RR  L   T    R    B 
// LL - left of left
// LR - right of left
// RL - left of Right
// RR - right of right
// L  - left
//R  - right
//T  - top
//B  - Bottom
typedef struct  
{
	MPOINT *pPtFeatures;
	MByte *pFlag;
	MLong lValidLen;
}FEATURE_PARAM;

typedef struct _TAG_DSPARAM
{
	MLong lFilterMask;
	MLong lFilterSize;//verify the blob with its size smaller than lfilterSize
	
	//Face Angle
	FACE_ORIENT lFaceAngle;
	
	MLong lMemUsed;
	
	MLong lDetialLevel;//used for canny high threshold
	MLong lMaxSeedNum;
	MLong lReducedSize;
	MLong lFilterDenomi;
	MBool bIsMutualMem; //mutual memory between luminData and spot mask
	//if MoleMatch is split by block, bIsMutualMem assigned MFalse;
	MBool bExtendedFace; //TRUE: extended face; FALSE only detected face
	MLong lReduceW;
	MLong lReduceH;
	MLong lMinFaceSize;
	MBool bIsLeaveSmall;
	
	MBool bIsNose;
	FEATURE_PARAM* pFeatureParam;
}DSPARAM;

#ifdef __cplusplus
extern "C" {
#endif	

MRESULT FaceDefreckle(MHandle hMemMgr, JOFFSCREEN* pImgSrc, MRECT* pFaceRect, MLong lFaceAngle,
					  JMASK *pFaceMask, JOFFSCREEN *pImgRlt, MLong lDetailLevel,
					  FEATURE_PARAM *pFeatureParam,
					  MLong lMemUsed,
					  MBool bIsExtended);
	
#ifdef __cplusplus
}
#endif

#endif//_LI_FACESKIN_H_
