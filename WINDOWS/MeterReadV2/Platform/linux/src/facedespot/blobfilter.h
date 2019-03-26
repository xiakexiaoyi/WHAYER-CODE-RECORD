#if !defined(_BLOB_FILTER_)
#define _BLOB_FILTER_

#include "licomdef.h"

typedef struct  
{
	PMPOINT pptSeed;
	MByte *pcrSeed;
	MLong lSeedNum;
	MLong lMaxNum;
} JGSEED, *PJGSEEDS;

#define  MAX_LEN_ELEMENT_1 10 
#define  MAX_LEN_ELEMENT_2 8

#define ExtractBlob_4Con			PX(ExtractBlob_4Con)
#define ExtractBlob_8Con_EXT			PX(ExtractBlob_8Con_EXT)
#define ExtractBlob_8Con			PX(ExtractBlob_8Con)
#define FilterBlob					PX(FilterBlob)
#define FilterSimilar				PX(FilterSimilar)
#define FilterBlob4Con				PX(FilterBlob4Con)
#define FilterEdge					PX(FilterEdge)
#define FilterBlob8Con				PX(FilterBlob8Con)
#define DeleteBlob4Con				PX(DeleteBlob4Con)
#define FilterConnect2Mask			PX(FilterConnect2Mask)
#define LocalMax_Circle			PX(LocalMax_Circle)
#define ridgePoint					PX(ridgePoint)

#ifdef __cplusplus
extern "C" {
#endif		
MVoid ExtractBlob_4Con(MVoid* pMaskData, MLong lMaskLine, 
					   MLong lMaskWidth, MLong lMaskHeight, 
					   MVoid *pQueue, MLong lQueueMaxLen, 
					   MLong* plBlobLen, 
					   MByte InFlag, MByte OutFlag, 
					   MRECT* pMaxRect,
					   MPOINT *pSeed);

MVoid ExtractBlob_8Con(MVoid* pMaskData, MLong lMaskLine,
				   MLong lMaskWidth, MLong lMaskHeight, 
				   MVoid *pQueue, MLong lQueueMaxLen,
				   MLong* plBlobLen, MLong* plPerimeter, MLong* plNghLength, 
				   MByte InFlag, MByte OutFlag, 
				   MPOINT *pSeed, MPOINT *pPointList);	

MVoid FilterBlob(MVoid* pMaskSrc, MLong lMaskLine, 
				 MLong lMaskWidth, MLong lMaskHeight, 
				 MVoid *pMemTmp, MLong lMemLen,
				 MByte inLabel,  MByte OutLabel,
				 MLong lFilterSize, MLong lRoundThres,
				 JGSEED *pSeed, MLong maxSeedNum,
				 MBool bIsNose, MLong lFaceAngle);

MVoid FilterSimilar(MVoid *pSrcImg, MLong lImgLine,
					MVoid *pSrc, MLong lSrcLine, 
					MLong lWidth, MLong lHeight, 
					MVoid *pMemTmp, MLong lMemLen,
					MByte inLabel, MByte outLabel,
				JGSEED *pSeed, MLong maxSeedNum);

MVoid FilterBlob4Con(MVoid* pMaskSrc, MLong lMaskLine, MLong lMaskWidth,
					 MLong lMaskHeight, MVoid *pMemTmp, MLong lMemLen, 
							  MLong lFilterSize, MByte inLabel, MByte outLabel);

MVoid FilterEdge(MVoid* pMask, MLong lMaskLine, 
				 MLong lWidth,MLong lHeight, 
				 MVoid* pEdge, MLong lEdgeLine,
				 MLong lEdgeWidth, MLong lEdgeHeight,
				 MLong lXOffSet, MLong lYOffset,
				 MVoid* pMemTmp, MLong lMemLen, 
				 MLong pinLabel, MLong label);

MVoid FilterBlob8Con(MVoid* pMaskSrc, MLong lMaskLine, 
					 MLong lMaskWidth, MLong lMaskHeight, 
					 MVoid *pMemTmp, MLong lMemLen,
					 MByte inLabel,  MByte OutLabel,
					  MLong lFilterSize, MLong lRoundThres,
					  MLong lFaceAngle);

MRESULT DeleteBlob4Con(MHandle hMemMgr,
					 MVoid* pMaskSrc, MLong lMaskLine, 
					 MLong lMaskWidth, MLong lMaskHeight, 
					 MByte inputLabel,
					 MPOINT* pPtSeed);

MVoid FilterConnect2Mask(MVoid* pBlobData, MLong lBlobLine, 
						 MLong lWidth,MLong lHeight, 
						 MVoid* pMask, MLong lMaskLine,
						 MLong lMaskWidth, MLong lMaskHeight,
						 MLong lXOffSet, MLong lYOffset,
						 MVoid* pMemTmp, MLong lMemLen, 
						 MLong maskLabel,
						 MLong inBlobLabel, MLong outBlobLabel,
				 MLong Percentage);
	

MVoid ExtractBlob_8ConEXT(MVoid* pMaskData, MLong lMaskLine, 
    MLong lMaskWidth, MLong lMaskHeight, 
    MVoid *pQueue, MLong lQueueMaxLen, 
    MLong* plBlobLen, 
    MByte InFlag, MByte OutFlag, 
    MPOINT *pSeed);

MRESULT SoftThres(MHandle hMemMgr, MByte* pSrc, MLong lSrcLine, 
	MByte* pRlt, MLong lRltLine,
	MLong lWidth, MLong lHeight, 
	MVoid *pTmpMem, MLong lMemLen, 
	MLong lHighThres, MLong lLowThres, MLong lBaseLowRate,
	MByte rltLabel, JGSEED *pSeeds);

MRESULT LocalMax_Circle(MHandle hMemMgr, MByte* pSrc, MLong lSrcLine, 
						  MByte* pRlt, MLong lRltLine,
						  MLong lWidth, MLong lHeight, 
						  MVoid *pTmpMem, MLong lMemLen, 
						  MLong lHighThres, MLong lLowThres, MLong lBaseLowRate,
						  MByte rltLabel, JGSEED *pSeeds, MLong lRadius);

MRESULT ridgePoint(MHandle hMemMgr, MByte *pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeight,
	MByte *pLabelData, MLong lLabelLine, MLong lRadius,
	MLong lThreshold, MByte ridgeLabel, MByte MaxLabel, JGSEED *pSeeds);

MVoid AdjustPosByGravity(MByte *pRsp, MLong lRspLine, MLong lWidth, MLong lHeight,
	JGSEED *pSeeds, MLong lRadius);

#ifdef __cplusplus
}
#endif	

#endif // !defined(_BLOB_FILTER_)