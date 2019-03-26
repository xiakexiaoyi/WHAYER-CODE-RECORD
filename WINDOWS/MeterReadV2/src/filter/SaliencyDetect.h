#ifndef _SALIENCY_DETECT_H_
#define _SALIENCY_DETECT_H_

#include "amcomdef.h"
#include "liblock.h"

#ifdef __cplusplus
extern "C"{
#endif

	typedef struct
	{
		MDouble mean_L;
		MDouble mean_a;
		MDouble mean_b;

		MLong cx;
		MLong cy;

		MLong lRegionNum;

		MByte lVal;
	}SuperPixelRegion;

	typedef struct
	{
		MDouble val_L;
		MDouble val_a;
		MDouble val_b;

		MLong x;
		MLong y;
	}SuperPixelSeed;
//******************************** superpixel segmentation ********************************
	MRESULT GetSuperpixelSegmentation(MHandle hMemMgr, MByte *pImageData, MLong lWidth, MLong lHeight, MLong lImageLine,
															MLong *pLabelMap, MLong *label_num, SuperPixelRegion *pSuperPixels, MLong spcount, MLong lCompactness);
	MRESULT RGBtoLAB(MByte *pImageData, MLong lWidth, MLong lHeight, MLong lImageLine,
									MDouble *pVectL, MDouble *pVectA, MDouble *pVectB);
	MRESULT RGBtoXYZ(MByte R, MByte G, MByte B, MDouble *X, MDouble *Y, MDouble *Z);
	MRESULT InitSuperPixelSeeds(MDouble *pVectL, MDouble *pVectA, MDouble *pVectB,
													SuperPixelSeed *pSeeds, MLong lWidth, MLong lHeight, MLong lStep, MLong *lNum);
	MRESULT SearchSuperPixel(MHandle hMemMgr, SuperPixelSeed *pSeeds, MDouble *pVectL, MDouble *pVectA, MDouble *pVectB,
											MLong *pLabelMap, MLong lWidth, MLong lHeight, MLong lStep, MLong lSeedsNum, MDouble dCompactness);
	MRESULT EnforceLabelConnectivity(MHandle hMemMgr, MLong *pLabel, MLong *pTmpLabel, MLong lWidth, MLong lHeight,
														MLong *pLabelNum, MLong K);
	MRESULT DrawSuperPixelContours(MHandle hMemMgr, BLOCK *pResultBlock, MLong *pLabel);
	MRESULT StoreSuperPixel(MHandle hMemMgr, MDouble *pVectL, MDouble *pVectA, MDouble *pVectB, 
					MLong *pLabelMap, SuperPixelRegion *pSuperPixels, MLong lWidth, MLong lHeight, MLong lLabelNum);
//************************************ salinecy detection ************************************
	MRESULT getSaliencyImage(MHandle hMemMgr, SuperPixelRegion *pSuperPixels, MLong *pLabelMap, MLong lLabelNum, BLOCK *pDstBlock);
	MRESULT calcSaliencyRatio(SuperPixelRegion *pSuperPixels, MLong lSuperPixelNum,
												MLong lIndex, MPOINT centerPt, MDouble *pResult);
	
	MRESULT optimizeSaliencyImage(MHandle hMemMgr, SuperPixelRegion *pSuperPixels, MLong *pLabelMap, 
														MLong lLabelNum, BLOCK *pDstBlock);
	MRESULT getAdjacentMap(MLong *pLabelMap, MLong lLabelWidth, MLong lLabelHeight,
											MBool *pAdjacentMap, MLong lAdjacentStride);
	MRESULT calcWeightMap(SuperPixelRegion *pSuperPixels, MLong lSuperPixelNum, MDouble *pWeightMap, 
											MDouble *pDMatrix, MBool *pAdjacentMap, MLong lWidth, MLong lHeight);
	MRESULT calcThresholdOtsu(MHandle hMemMgr, BLOCK *pSrcImage, MByte *lThreshold);
	MRESULT invMatrix(MHandle hMemMgr, MDouble *pSrcMatrix, MDouble *pDstMatrix, MLong n);
	MRESULT initOptimizeVect(MDouble *pWeight, MDouble *pDMatrix, MDouble *pAMatrix, MDouble *pStartVect, 
											MDouble *pFinalVect, SuperPixelRegion *pSuperPixels, MDouble alfa, MLong n, MByte lThreshold);
	MRESULT getOptimizeVect(MDouble *pRefMatrix, MDouble *pStartVect, MDouble *pFinalVect, MDouble beta, MLong n);
	MRESULT refreshSaliencyImage(SuperPixelRegion *pSuperPixels, MDouble *pCoeffVect, 
														BLOCK *pDstBlock, MLong *pLabelMap, MLong n);
#ifdef __cplusplus
}
#endif

#endif