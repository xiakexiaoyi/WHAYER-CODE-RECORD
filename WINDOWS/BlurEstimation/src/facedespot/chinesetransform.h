#ifndef  __CHINESETRANSFORM_H__
#define __CHINESETRANSFORM_H__

#include "amcomdef.h"
#include "litrimfun.h"
#include "blobfilter.h"
#include "bbgeometry.h"

#define ChinTransform		PX(ChinTransform)
#define LocalMaxBlob		PX(LocalMaxBlob)
#define CreateBlobGraphByImg	PX(CreateBlobGraphByImg)
#define ReleaseBlobGraph		PX(ReleaseBlobGraph)
#define BlobMerge				PX(BlobMerge)
#define BlobDelete				PX(BlobDelete)

#define MAX_BLOB_GRADIENT	64
#define MAX_SEED_NUM		65536
#define THRESH_VOTING		60
#define MIN_FINGER_LENTH    
#define COS15	   0.9659
#define COS10	   0.9848
#define COS5	   0.9962  
#define ANGLE_DIST			COS15
#define EDGE_PERCENTAGE		5 

#define MAX_DIST_BW_POINTS 40
#define MIN_DIST_BW_POINTS 20 

#define MAX_COLOR_DIST_BLOB 1024
#define COLOR_DIST_BLOB     (MAX_COLOR_DIST_BLOB/4)


typedef struct  
{
	MLong lGradientSum;
	MLong lBoundaryNum;	
	MLong lEdgeNum;
	MLong lSize;
	MPOINT CenPt;
	EIGEN_2D eigen;

	MLong lY, lCb, lCr;
	MByte WeightToSkin;
	MLong lFlag;
	MPOINT ptSeed;
	MPOINT Endpoint[4];//0--left, 1--top, 2--right, 3--bottom
}JINFO_BLOB;

typedef struct  
{
	JINFO_BLOB *pInfoBlob;
	MLong lLabelNum;
}JINFO_BLOB_GRAPH;

#ifdef __cplusplus
extern "C"{
#endif
MRESULT LocalMaxBlob(MHandle hMemMgr, MByte* pChinTransData, MLong lLine, MLong lWidth, MLong lHeight,
				 MByte* pRltData, MLong lRltLine, JGSEED* pSeeds);

MRESULT ChinTransform(MHandle hMemMgr, 
#ifdef USE_COLOR_GRADIENT
					  JOFFSCREEN *pSrcImg,
#else
					 MByte* pSrcData, MLong lSrcLine, 
#endif
					  MByte* pMaskData, MLong lMaskLine,
					  MLong lWidth, MLong lHeight,
					MByte* pRltData, MLong lRltLine);

MRESULT CreateBlobGraphByImg(MHandle hMemMgr, JOFFSCREEN* pImg, 
							 MByte* pBlobMask, MLong lBlobLine,
							 MLong lWidth, MLong lHeight,
							 JINFO_BLOB_GRAPH **ppInfoGraph, JGSEED *pSeed);

MVoid ReleaseBlobGraph(MHandle hMemMgr, JINFO_BLOB_GRAPH **pInfoGraph);
MVoid BlobMerge(JOFFSCREEN *pImgSrc, JINFO_BLOB_GRAPH* pInfoGraph, MLong lWidth, MLong lHeight);
MVoid BlobDelete(JINFO_BLOB_GRAPH *pInfoGraph, MLong lMinBlobSize);

#ifdef __cplusplus
}
#endif

#endif//__CHINESETRANSFORM_H__