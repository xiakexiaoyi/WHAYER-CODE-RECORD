#ifndef LOCATION_H
#define LOCATION_H
#include"amcomdef.h"
#include "liblock.h"
#include "bbgeometry.h"
#include "HaarResponse.h"
#include "blobfilter.h"

#ifdef __cplusplus
extern "C" {
#endif

//************************ struct *****************************
typedef struct
{
	MPOINT seedPos;
	MDouble seedAngle;
	MLong seedArea;
	MByte seedVal;	
}SEEDINFO, *PSEEDINFO;

typedef struct
{
	SEEDINFO *pSeedInfo;
	MLong lSeedNum;
	MLong lMaxNum;
}SEEDSETINFO, *PSEEDSETINFO;

typedef struct
{
	MPOINT *pRegCenter;
	MLong *pRegArea;
	MLong *pRegMeanVal;
	MLong *pRegWidth;

	MLong lRegNum;
	MLong lMaxNum;
	MLong lRegSize;
	MLong lHaarWidth;
}RegionInfo;

typedef struct
{
	//直线参数
	MDouble dCoeffK;
	MDouble dCoeffB;
	MBool bVert;
	MDouble dAngle;

	MPOINT ptStart;
	MPOINT ptEnd;
}LINE_PARA;
//*************************************************************

//************************* define ****************************
#if 1
#define LOCATION_ERR  printf
#else
#define LOCATION_ERR
#endif

#define ANGLE_STEP	10
//*************************************************************

//************************ function ***************************
#define GetCalibLocation               PX(GetCalibLocation)
#define GetLineLocation				   PX(GetLineLocation)
//*************************************************************
MRESULT GetCalibLocation(MHandle hMemMgr, BLOCK* pImage, PARAM_INFO *pParam);

MRESULT GetRegions(MHandle hMemMgr, BLOCK *pSrc, BLOCK *pDst, MLong *pMark, MVoid *pTmpMem, MLong lMemLen, 
					MLong lThres, MByte dstLabel, SEEDSETINFO *pSeeds, RegionInfo *pRegInfo);

MRESULT RegionGrow(MHandle hMemMgr, BLOCK *pSrc, BLOCK *pDst, MLong *pMark, MPOINT ptSeed, MByte ptVal, 
					MLong lThresVal, MByte label, RegionInfo *pRegInfo, SEEDSETINFO *pSeeds);

MDouble CalcRegionDirection(MPOINT *ptList, MLong lPtNum, MPOINT center);

MRESULT CircleRank(MHandle hMemMgr, BLOCK *pHaarResponse, MLong *pMark, SEEDSETINFO *pSeeds,
	RegionInfo *pRegion, CIRCLE_SET_INFO *pCircleSetInfo, MPOINT *ptTmp, MLong lNum);

MRESULT GetAllCircles(MHandle hMemMgr, BLOCK *pHaarResponse, SEEDSETINFO *pSeeds,
						CIRCLE_SET_INFO *pCircleSetInfo, MPOINT *ptTmp, MLong lNum);

MDouble CalcCircleRadio(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo);

MLong CircleInfoFitRadioCmp(const MVoid* p1, const MVoid* p2);
MLong SeedInfoCmp_Y( const MVoid* p1, const MVoid* p2);
MLong CircleInfoPtNumCmp(const MVoid* p1, const MVoid *p2);

MRESULT CalcCircleConfidence(MHandle hMemMgr, BLOCK *pHaarResponse, SEEDSETINFO *pSeeds,
	RegionInfo *pRegion, MLong *pMark, CIRCLE_INFO *pCircleInfo);

MLong notExistInStack(MLong *pStack, MLong lStack, MLong lData);

MLong calcRectWidth(MPOINT center, BLOCK *pSrc, MByte label, MDouble dAngle);

MRESULT OptimizePtList(MHandle hMemMgr, BLOCK *pHaarResponse, CIRCLE_INFO *pCircleInfo, 
						MLong *pMark, RegionInfo *pRegion, MPOINT *ptTmp, MLong lNum);

MRESULT SearchPtOnCircle(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, RegionInfo *pRegion, MLong *pMark, 
						MLong *pStack, MPOINT *ptTmp, MLong *lPtNum, MLong lWidth, MLong lHeight);
MRESULT SearchPtOnCircle_step0(MHandle hMemMgr, RegionInfo *pRegion, CIRCLE cirParam, MLong lMarkData, 
								MPOINT *ptTmp, MLong *lPtNum, MPOINT srcPt, MLong *pStack);

MRESULT AddCrossPt(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, RegionInfo *pRegion, MLong *pMark, MPOINT *ptTmp, 
					MLong *lPtNum, MLong *pDistTmp, MLong lPtDist, MLong lWidth, MLong lHeight);
MRESULT AdjustPtPos(MHandle hMemMgr, RegionInfo *pRegion, MLong *pMark, MPOINT *ptCross, MLong lWidth, MLong lHeight);

MLong FindTwoCirclePt(CIRCLE cirParam1, CIRCLE cirParam2, MPOINT *pt1, MPOINT *pt2, MLong lWidth, MLong lHeight);

MVoid CircleInfoCpy_xsd(CIRCLE_INFO *pSrcCircle, CIRCLE_INFO *pDstCircle);
//*************************************************************

MRESULT GetLineLocation(MHandle hMemMgr, BLOCK *pImage, PARAM_INFO *pParam, MLong lHaarWidth, MLong lPercentage);

MRESULT LinesRank(MHandle hMemMgr, BLOCK *pResponse, JGSEED *pSeedList, CIRCLE cirParam,  MPOINT *ptTmp, MLong lNum, LINE_SET_INFO *pLineSetInfo);
MRESULT GetAllLines(MHandle hMemMgr, JGSEED *pSeedList, CIRCLE cirParam, MPOINT *ptTmp, MLong lNum, LINE_SET_INFO *pLineSetInfo);
MRESULT CalcLineConfidence(MHandle hMemMgr, BLOCK *pResponse, CIRCLE cirParam, LINE_INFO *pLineInfo);
//*************************************************************
#ifdef __cplusplus
}
#endif
#endif