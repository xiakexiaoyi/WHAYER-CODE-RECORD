#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "liimage.h"

#include "blobfilter.h"
#include "facedespot.h"

//ATTENTION: InFlag != OutFlag && InFlag OutFlag != 255 && 255-1
//lNghLength is not accurate
MVoid ExtractBlob_4Con(MVoid* pMaskData, MLong lMaskLine, 
					   MLong lMaskWidth, MLong lMaskHeight, 
					   MVoid *pQueue, MLong lQueueMaxLen, 
					   MLong* plBlobLen, 
					   MByte InFlag, MByte OutFlag, 
					   MRECT* pMaxRect,
					   MPOINT *pSeed)
{
	MLong lPtsNum=0, lBasePos=0;
	MLong north, west, east, south;
	MPOINT *pPtsTmp=(MPOINT*)pQueue;//queue start
	MRECT maxRect={0};
	MBool bIsRect=MFalse;
	MLong lBNorth, lBWest, lBEast, lBSouth;
	maxRect.right=0; maxRect.bottom=0;
	maxRect.left = lMaskWidth;maxRect.top=lMaskHeight;
	lBNorth = -pSeed->y;
	lBWest = -pSeed->x;
	lBEast = lMaskWidth-pSeed->x;
	lBSouth = lMaskHeight-pSeed->y;
	if (pMaxRect!=MNull)
	{
		bIsRect = MTrue;
	}

	pPtsTmp[lPtsNum].x = 0;
	pPtsTmp[lPtsNum].y = 0;
	lPtsNum++;

	do 
	{
		MPOINT ptCur = pPtsTmp[lBasePos];
		MLong maskX = ptCur.x, maskY = ptCur.y;
		MByte *pMaskTmp = (MByte*)pMaskData+lMaskLine*maskY+maskX;

		*pMaskTmp = OutFlag;
		lPtsNum--;lBasePos++;//blob size
		if(maskX< lBWest || maskX>lBEast) continue;
		if(maskY<lBNorth || maskY> lBSouth) continue;
		if (lBasePos+lPtsNum+4>=lQueueMaxLen) continue;
		north = maskY-1;
		west = maskX-1;
		east = maskX+1;
		south = maskY+1;

		if (bIsRect)
		{
			maxRect.left=MIN(maxRect.left, maskX);
			maxRect.right=MAX(maxRect.right,maskX);
			maxRect.top = MIN(maxRect.top, maskY);
			maxRect.bottom = MAX(maxRect.bottom, maskY);
		}

		//north
		if (north >= lBNorth)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine;
			if(*pMaskTmp2==InFlag)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = maskX; pPts->y = north;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
		}

		//west
		if (west >= lBWest)
		{
			MByte *pMaskTmp2 = pMaskTmp - 1;
			if(*pMaskTmp2==InFlag)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = west; pPts->y = maskY;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
		}
		
		//east
		if (east < lBEast)
		{
			MByte *pMaskTmp2 = pMaskTmp + 1;
			if(*pMaskTmp2==InFlag)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = east; pPts->y = maskY;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
		}

		//south
		if (south < lBSouth)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine;
			if(*pMaskTmp2==InFlag)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = maskX; pPts->y = south;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
		}
		
	} while(lPtsNum>0);	
	if (plBlobLen!=MNull)
		*plBlobLen = lBasePos;

	if (bIsRect)
		*pMaxRect = maxRect;
	return;
}

//ATTENTION: InFlag != OutFlag
//lNghLength is not accurate
MVoid ExtractBlob_8Con(MVoid* pMaskData, MLong lMaskLine,
					   MLong lMaskWidth, MLong lMaskHeight, 
					   MVoid *pQueue, MLong lQueueMaxLen,
					   MLong* plBlobLen, MLong* plPerimeter, MLong* plNghLength, 
				       MByte InFlag, MByte OutFlag, 
					   MPOINT *pSeed, MPOINT *pPointList)
{
	MLong lPtsNum=0, lBasePos=0, lSize=0, lPtsNgh=0, lPtsBoundary=0;
	MPOINT *pPtsTmp=(MPOINT*)pQueue;//queue start
	MPOINT PtsCtr; //blob center estimating
	MPOINT PtsLeft, PtsTop, PtsRight, PtsBottom;//blob rectangle
	MBool bBoundary = MFalse;

	PtsCtr.x = PtsCtr.y = 0;
	PtsLeft.x=lMaskLine;PtsRight.x=0;
	PtsTop.y=lMaskHeight;PtsBottom.y=0;

	pPtsTmp[lPtsNum].x = (MWord)(pSeed->x);
	pPtsTmp[lPtsNum].y = (MWord)(pSeed->y);
	lPtsNum++;

	do 
	{
		MPOINT ptCur = pPtsTmp[lBasePos];
		MLong maskX = ptCur.x, maskY = ptCur.y;
		MByte *pMaskTmp = (MByte*)pMaskData+lMaskLine*maskY+maskX;
	
		if(maskX<0 || maskX>lMaskWidth-1) continue;
		if(maskY<0 || maskY>lMaskHeight-1)continue;

		*pMaskTmp = OutFlag;
		lPtsNum--;lBasePos++;lSize++;//blob size
 		PtsCtr.x+=maskX;PtsCtr.y+=maskY;//blob center
		if (PtsLeft.x >= maskX) {PtsLeft.x=maskX; PtsLeft.y=maskY;}//Maximum rectangle
		if (PtsRight.x <= maskX) {PtsRight.x=maskX; PtsRight.y=maskY;}
		if (PtsTop.y >= maskY) {PtsTop.x=maskX; PtsTop.y=maskY;}
		if (PtsBottom.y <= maskY) {PtsBottom.x=maskX; PtsBottom.y=maskY;}
		
		//south
		if (maskY+1 < lMaskHeight)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) maskX;
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY+1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag

			}
			else 
			{
				if (*pMaskTmp2!=OutFlag&&*pMaskTmp2!=255-1)
					bBoundary = MTrue;
			}
		}
		//west
		if (maskX-1 >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX-1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) maskY;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag

			}
			else 
			{
				if (*pMaskTmp2!=OutFlag&&*pMaskTmp2!=255-1)
					bBoundary = MTrue;
			}
		}
		//east
		if (maskX+1 < lMaskWidth)
		{
			MByte *pMaskTmp2 = pMaskTmp + 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX+1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) maskY;
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
				if (*pMaskTmp2!=OutFlag&&*pMaskTmp2!=255-1)
					bBoundary = MTrue;
			}
		}
		//north
		if (maskY-1 >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) maskX;
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY-1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
				if (*pMaskTmp2!=OutFlag&&*pMaskTmp2!=255-1)
					bBoundary = MTrue;
			}
		}
		//southwest
		if (maskY+1 < lMaskHeight && maskX-1>=0)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine - 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX-1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY+1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
			//	bBoundary = MTrue;
			}
		}
		//southeast
		if (maskY+1 < lMaskHeight && maskX+1<lMaskWidth)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine + 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX+1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY+1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
			//	bBoundary = MTrue;
			}
		}
		//northeast
		if (maskY-1 >= 0 && maskX+1<lMaskWidth)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine + 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX+1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY-1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
			//	bBoundary = MTrue;
			}
		}
		//northwest
		if (maskY-1 >= 0 && maskX-1>=0)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine - 1;
			if(*pMaskTmp2==InFlag)
			{
				if(lBasePos+lPtsNum >= lQueueMaxLen)
					continue;
				pPtsTmp[lBasePos+lPtsNum].x = (MWord) (maskX-1);
				pPtsTmp[lBasePos+lPtsNum].y = (MWord) (maskY-1);
				lPtsNum ++;
				*pMaskTmp2=255-1;//temp flag
			}
			else 
			{
			//	bBoundary = MTrue;
			}
		}
		if (bBoundary)
		{
			lPtsBoundary++;
			bBoundary = MFalse;
		}
	} while(lPtsNum>0);	
	if (plBlobLen!=MNull)
		*plBlobLen = lBasePos;
	if (plNghLength!=MNull)
		*plNghLength = lPtsNgh;
	if (pPointList!=MNull)
	{
		pPointList[0] = PtsLeft;
		pPointList[1] = PtsRight;
		pPointList[2] = PtsTop;
		pPointList[3] = PtsBottom;
		pPointList[4].x = PtsCtr.x/lBasePos;
		pPointList[4].y = PtsCtr.y/lBasePos;
	}
	if (plPerimeter!=MNull)
		*plPerimeter = lPtsBoundary;
	return;
}

#define G_ATAN						PX(G_ATAN)
JSTATIC MLong G_ATAN[21] = {0, 4171,8321,12430,16480,20452,24332,28107,31767,35302,
38708, 41980, 45117, 48119, 50987, 53723, 56331, 58815,
					61179, 63429, 65503};

JSTATIC MVoid Size_0Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect);
JSTATIC MVoid Size_180Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect);
JSTATIC MVoid Size_90Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect);
JSTATIC MVoid Size_270Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect);

typedef MVoid (*fnSize)(MLong lWidth, MLong lHeight, MRECT *pVIPRect);

MVoid FilterBlob(MVoid* pMaskSrc, MLong lMaskLine, 
				 MLong lMaskWidth, MLong lMaskHeight, 
				 MVoid *pMemTmp, MLong lMemLen,
				 MByte inLabel,  MByte OutLabel,
				 MLong lFilterSize, MLong lRoundThres,
				 JGSEED *pSeed, MLong maxSeedNum,
				 MBool bIsNose, MLong lFaceAngle)
{
	MLong x,y;
	MPOINT SeedPoint;
	MLong lBlobLen=0;
	//MPOINT PointList[5];//0-left,1-right,2-top,3-bottom,4-center
	MByte *pTmpMask = (MByte*)pMaskSrc;
	MLong lExt=lMaskLine-lMaskWidth;
	MLong maxLenElem, lFilterSize1;
	MRECT VIPRect={0};
	fnSize pfnSize=MNull;

	switch (lFaceAngle)
	{
	case 1:
		pfnSize = &Size_0Degree;
		break;
	case 2:
		pfnSize = &Size_90Degree;
		break;
	case 3:
		pfnSize = &Size_270Degree;
		break;
	case 4:
		pfnSize = &Size_180Degree;
		break;
	default:
		pfnSize = &Size_0Degree;
		break;
	}

	(*pfnSize)(lMaskWidth, lMaskHeight, &VIPRect);


	lRoundThres <<= 16;
// 	if (lMaskWidth>=NORM_SIZE_1) maxLenElem = MAX_LEN_ELEMENT_1;
// 	else /*if (lMaskWidth>=NORM_SIZE_2)*/
//		maxLenElem = MAX_LEN_ELEMENT_2;
	maxLenElem = MAX_LEN_ELEMENT_1;

	if (pSeed!=MNull)
		pSeed->lSeedNum = 0;

	for(y = 0; y < lMaskHeight; y++, pTmpMask+=lExt)
	{
		for(x = 0; x < lMaskWidth; x++, pTmpMask++)
		{
			SeedPoint.x = x, SeedPoint.y = y;
			if(*pTmpMask==inLabel)
			{
				MLong lPhi = 0, lAlpha = 50;
				MLong lDeltaX=0, lDeltaY=0;
				MBool bFilter=MFalse;
				MRECT MaxRect={0};

				if (pSeed!=MNull&&pSeed->lSeedNum>maxSeedNum) continue;
				if(pSeed!=MNull)
				{
					pSeed->pptSeed[pSeed->lSeedNum].x = x;
					pSeed->pptSeed[pSeed->lSeedNum].y = y;
					pSeed->lSeedNum++;
				}
				ExtractBlob_4Con(pTmpMask, lMaskLine, lMaskWidth, lMaskHeight, pMemTmp, lMemLen, \
					&lBlobLen, inLabel, OutLabel, &MaxRect, &SeedPoint);
				//filter round blob(it may be spots)
				lFilterSize1 = lFilterSize;
				if (y>VIPRect.top&&y<VIPRect.bottom&&x>VIPRect.left&&x<VIPRect.right)
				{
					lFilterSize1 = bIsNose?lFilterSize/2:lFilterSize/4;
				}
				if (lBlobLen<3||lBlobLen>lFilterSize1)
					bFilter = MTrue;
				if (!bFilter)
				{
// 					MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
// 					MLong lTmpLen = lBlobLen;
// 					PointList[0].x = PointList[1].x =PointList[2].x = PointList[3].x =pPtsTmp[0].x;
// 					PointList[0].y = PointList[1].y =PointList[2].y = PointList[3].y =pPtsTmp[0].y;
// 					while (lTmpLen-->0)
// 					{
// 						MPOINT ptCur = pPtsTmp[lTmpLen];
// 						if (PointList[0].x>ptCur.x) PointList[0].x=ptCur.x;
// 						if (PointList[1].x<ptCur.x) PointList[1].x=ptCur.x;
// 						if (PointList[2].y>ptCur.y) PointList[2].y=ptCur.y;
// 						if (PointList[3].y<ptCur.y) PointList[3].y=ptCur.y;
//					}
// 					lDeltaX = ABS(PointList[1].x - PointList[0].x)+1;
// 					lDeltaY = ABS(PointList[3].y - PointList[2].y)+1;
					lDeltaX = ABS(MaxRect.right-MaxRect.left)+1;
					lDeltaY = ABS(MaxRect.bottom-MaxRect.top)+1;
					if (lDeltaY>=maxLenElem || lDeltaX>=maxLenElem) 
						bFilter = MTrue;
				}
				
				if (!bFilter)
				{
					MLong lTanVal=0, lDiv=0;
					lDiv = lDeltaX<lDeltaY?lDeltaX*20/lDeltaY:lDeltaY*20/lDeltaX;
					if (lDiv>=20)lDiv = 20;
					lTanVal = G_ATAN[lDiv];
					lPhi =  lAlpha * lTanVal + (100-lAlpha)*(lBlobLen<<16)/(lDeltaX*lDeltaY);
					if (lPhi<lRoundThres)
						bFilter = MTrue;
				}
			
				//if (lBlobLen<3 || lDeltaY>=maxLenElem || lDeltaX>=maxLenElem||lBlobLen > lFilterSize || (lPhi < lRoundThres))
				if (bFilter)
				{
					MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
					while (lBlobLen-->0)
					{
						MPOINT ptCur = pPtsTmp[lBlobLen];
						MLong maskX = ptCur.x, maskY = ptCur.y;
						MByte *pMaskTmp = (MByte*)pTmpMask+lMaskLine*maskY+maskX;
						*pMaskTmp = 255;
					}
				}
			}
		}
	}
}

#define STANDARD_COLOR_DIST 6
MVoid FilterSimilar(MVoid *pSrcImg, MLong lImgLine,
				MVoid *pSrc, MLong lSrcLine, 
				MLong lWidth, MLong lHeight, 
				MVoid *pMemTmp, MLong lMemLen,
				MByte inLabel, MByte outLabel,
				JGSEED *pSeed, MLong maxSeedNum)
{
	MLong x, y;
	MLong lBlobLen=0;
	MBool bFiltered = MTrue;
	MPOINT SeedPoint, PointList[5];//0-left,1-right,2-top,3-bottom,4-center
	MByte *pTmpSrc;
	MLong lSrcExt;
	MRECT maxRect={0};

	if (pSeed!=MNull)
		pSeed->lSeedNum = 0;

	pTmpSrc = (MByte*)pSrc;
	lSrcExt = lSrcLine-lWidth;
	for (y=0; y<lHeight; y++, pTmpSrc+=lSrcExt)
	{
		for (x=0; x<lWidth; x++,pTmpSrc++)
		{
			SeedPoint.x = x; SeedPoint.y = y;

			if (*pTmpSrc==inLabel || *pTmpSrc==64)
			{
				if (pSeed!=MNull&&pSeed->lSeedNum>maxSeedNum) continue;
				if(pSeed!=MNull)
				{
					pSeed->pptSeed[pSeed->lSeedNum].x = x;
					pSeed->pptSeed[pSeed->lSeedNum].y = y;
					pSeed->lSeedNum++;
				}
				if (*pTmpSrc==64) continue;
				ExtractBlob_4Con(pTmpSrc, lSrcLine, lWidth, lHeight, pMemTmp, lMemLen,\
					&lBlobLen, inLabel, outLabel, &maxRect, &SeedPoint);
				bFiltered = MTrue;
 				{
					MLong lValidLen=0;
					{
						MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
						MLong lTmpLen = lBlobLen;
						MLong lCenterX=0, lCenterY=0;
						PointList[0].x = PointList[1].x =PointList[2].x = PointList[3].x =pPtsTmp[0].x;
						PointList[0].y = PointList[1].y =PointList[2].y = PointList[3].y =pPtsTmp[0].y;
						while (lTmpLen-->0)
						{
							MPOINT ptCur = pPtsTmp[lTmpLen];
							if (PointList[0].x>ptCur.x) PointList[0].x=ptCur.x;
							if (PointList[1].x<ptCur.x) PointList[1].x=ptCur.x;
							if (PointList[2].y>ptCur.y) PointList[2].y=ptCur.y;
							if (PointList[3].y<ptCur.y) PointList[3].y=ptCur.y;
							lCenterX += ptCur.x;
							lCenterY += ptCur.y;
						}
						if (PointList[0].x+SeedPoint.x>1) PointList[0].x -= 2;
						if (PointList[1].x+SeedPoint.x<lWidth-1) PointList[1].x +=2;
						if (PointList[2].y+SeedPoint.y>1) PointList[2].y-=2;
						if (PointList[3].y+SeedPoint.y<lHeight-1) PointList[3].y+=2;
						PointList[4].x = lCenterX/lBlobLen;
						PointList[4].y = lCenterY/lBlobLen;
					}

					//compute the color distance between target region and its neighbors
					{
						MLong i, j, lExt, lMskExt;
						MByte *pTmp=MNull, *pTmpMsk=MNull;
						MLong lTmpBlobLen = lBlobLen;
						MPOINT *pPtsTmp = (MPOINT*)(pMemTmp);
						MLong sumY=0, blobY=0, nghborY=0;
						while (lTmpBlobLen-->0)
						{
							MPOINT ptCur = pPtsTmp[lTmpBlobLen];
							MLong maskX = ptCur.x+SeedPoint.x, maskY = ptCur.y+SeedPoint.y;
							pTmp = (MByte*)pSrcImg+maskY*lImgLine+maskX;
							sumY += *pTmp;
						}
						blobY = sumY/lBlobLen;
						sumY = 0;
						lValidLen = 0;
											
						pTmp = (MByte*)pSrcImg+(PointList[2].y+SeedPoint.y)*lImgLine+(PointList[0].x+SeedPoint.x);
						pTmpMsk = (MByte*)pTmpSrc+PointList[2].y*lSrcLine+PointList[0].x;
						lExt = lImgLine-(PointList[1].x-PointList[0].x+1);
						lMskExt = lSrcLine-(PointList[1].x-PointList[0].x+1);
						for (j=PointList[2].y; j<=PointList[3].y; j++, pTmp+=lExt, pTmpMsk+=lMskExt)
						{
							for (i=PointList[0].x; i<=PointList[1].x; i++, pTmp++, pTmpMsk++)
							{
								if (*pTmpMsk == 255)sumY+=*pTmp, lValidLen++;
							}
						}
						//JASSERT(lValidLen);
						if (lValidLen==0) nghborY = 0;
						else 
							nghborY = sumY/lValidLen;

						if (ABS(blobY-nghborY)<STANDARD_COLOR_DIST ) 
							goto EXT;

						bFiltered = MFalse;
						
					}

EXT:
					if (bFiltered)
					{
						MPOINT  *pPtsTmp = (MPOINT*)(pMemTmp);
						while (lBlobLen-->0)
						{
							MPOINT ptCur = pPtsTmp[lBlobLen];
							MByte *pMaskTmp = (MByte*)pTmpSrc +lSrcLine*ptCur.y+ ptCur.x;
							*pMaskTmp = 255;
						}
						continue;
					}
				}
			}
		}
	}
}

MVoid FilterBlob4Con(MVoid* pMaskSrc, MLong lMaskLine, MLong lMaskWidth,
					  MLong lMaskHeight, MVoid *pMemTmp, MLong lMemLen, 
							  MLong lFilterSize, MByte inLabel, MByte outLabel)
{
	MLong x,y;
	MPOINT SeedPoint;
	MLong lBlobLen=0;
	MByte *pTmpSrc = (MByte*)pMaskSrc;
	MLong lExt = lMaskLine-lMaskWidth;

	for(y = 0; y < lMaskHeight; y++, pTmpSrc+=lExt)
	{
		for(x = 0; x < lMaskWidth; x++, pTmpSrc++)
		{
			SeedPoint.x = x, SeedPoint.y = y;
			if(*pTmpSrc==inLabel)
			{
				ExtractBlob_4Con(pTmpSrc, lMaskLine, lMaskWidth, lMaskHeight, pMemTmp, lMemLen, 
					&lBlobLen, inLabel, outLabel, MNull, &SeedPoint);
				//filter
				if (lBlobLen < lFilterSize)
				{
					MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
					while (lBlobLen-->0)
					{
						MPOINT ptCur = pPtsTmp[lBlobLen];
						MLong maskX = ptCur.x, maskY = ptCur.y;
						MByte *pMaskTmp = (MByte*)pTmpSrc+lMaskLine*maskY+maskX;
						*pMaskTmp = 255;
					}
				}
			}
		}
	}
}

JSTATIC MVoid FilterEdge_PIXEL(MVoid* pMaskSrc, MLong lMaskLine, MLong lWidth, MLong lHeight,
								MPOINT* pSeed, MVoid* pMemTmp, MLong lPtsTmpMaxNum, MLong label)
{
	MLong lPtsNum=0;
	MPOINT *pPtsTmp=(MPOINT*)pMemTmp;
	
	pPtsTmp[lPtsNum].x = pSeed->x;
	pPtsTmp[lPtsNum].y = pSeed->y;
	lPtsNum++;
	
	do 
	{
		MPOINT ptCur = pPtsTmp[--lPtsNum];
		MLong maskX = ptCur.x, maskY = ptCur.y;
		MByte *pMaskTmp = (MByte*)pMaskSrc+lMaskLine*maskY+maskX;
		
		if(maskX<0 || maskX>lWidth-1) continue;
		if(maskY<0 || maskY>lHeight-1) continue;
		if(lPtsNum>=lPtsTmpMaxNum-4) continue;
		
		*pMaskTmp = 255/2;
		
		//south
		if (maskY+1 < lHeight)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine;
			//if(*pMaskTmp2==pinLabel||*pMaskTmp2==label)
			if (*pMaskTmp2==label)
			{
				pPtsTmp[lPtsNum].x = maskX;
				pPtsTmp[lPtsNum++].y = maskY+1;
			}
		}
		//west
		if (maskX-1 >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - 1;
			//if(*pMaskTmp2==pinLabel||*pMaskTmp2==label)
			if (*pMaskTmp2==label)
			{
				pPtsTmp[lPtsNum].x = maskX-1;
				pPtsTmp[lPtsNum++].y = maskY;
			}
		}
		//east
		if (maskX+1 < lWidth)
		{
			MByte *pMaskTmp2 = pMaskTmp + 1;
			//if(*pMaskTmp2==pinLabel||*pMaskTmp2==label)
			if (*pMaskTmp2==label)
			{
				pPtsTmp[lPtsNum].x = maskX+1;
				pPtsTmp[lPtsNum++].y = maskY;
			}
		}
		//north
		if (maskY-1 >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine;
			//if(*pMaskTmp2==pinLabel||*pMaskTmp2==label)
			if (*pMaskTmp2==label)
			{
				pPtsTmp[lPtsNum].x = maskX;
				pPtsTmp[lPtsNum++].y = maskY-1;
			}
		}
	} while(lPtsNum>0);
	return;
}

MVoid FilterEdge(MVoid* pMask, MLong lMaskLine, 
				 MLong lWidth,MLong lHeight, 
				 MVoid* pEdge, MLong lEdgeLine,
				 MLong lEdgeWidth, MLong lEdgeHeight,
				 MLong lXOffSet, MLong lYOffset,
				 MVoid* pMemTmp, MLong lMemLen, 
				 MLong pinLabel, MLong label)
{
	MLong x, y;
	MPOINT SeedPoint;
	MByte *pTmpMsk, *pTmpEdge;
	MLong lExt, lEdgeExt;
	pTmpMsk = (MByte*)pMask;
	lExt = lMaskLine - lWidth;
	
	pTmpMsk = (MByte*)pMask + lYOffset*lMaskLine+lXOffSet;
	pTmpEdge = (MByte*)pEdge;
	lExt = lMaskLine - lEdgeWidth;
	lEdgeExt = lEdgeLine - lEdgeWidth;
	
	for (y=0; y<lEdgeHeight; y++, pTmpMsk+=lExt, pTmpEdge+=lEdgeExt)
		for (x=0; x<lEdgeWidth; x++, pTmpMsk++, pTmpEdge++)
		{
			SeedPoint.x = x; SeedPoint.y=y;
			if (*pTmpMsk==label&&*pTmpEdge == pinLabel)
				FilterEdge_PIXEL(pMask, lMaskLine, lWidth, lHeight, &SeedPoint, pMemTmp, lMemLen,label);
		}
}

// filter blob that connected to Label

MVoid FilterConnect2Mask(MVoid* pBlobData, MLong lBlobLine, 
				 MLong lWidth,MLong lHeight, 
				 MVoid* pMask, MLong lMaskLine,
				 MLong lMaskWidth, MLong lMaskHeight,
				 MLong lXOffSet, MLong lYOffset,
				 MVoid* pMemTmp, MLong lMemLen, 
				 MLong maskLabel,
				 MLong inBlobLabel, MLong outBlobLabel,
				 MLong Percentage)//x -- x%
{
	MLong x, y;
	MPOINT SeedPoint;
	MByte *pTmpBlob, *pTmpMask;
	MLong lExt, lEdgeExt;
	lExt = lBlobLine - lWidth;

	pTmpBlob = (MByte*)pBlobData + lYOffset*lBlobLine+lXOffSet;
	pTmpMask = (MByte*)pMask;
	lExt = lBlobLine - lMaskWidth;
	lEdgeExt = lMaskLine - lMaskWidth;
	
	for (y=0; y<lMaskHeight; y++, pTmpBlob+=lExt, pTmpMask+=lEdgeExt)
		for (x=0; x<lMaskWidth; x++, pTmpBlob++, pTmpMask++)
		{
			//SeedPoint.x = x; SeedPoint.y=y;
			if (*pTmpBlob==inBlobLabel/*&&*pTmpMask == maskLabel*/)
			{
				MLong lBlobLen=0, lTmpBlobLen, lInMaskNum=0;
				MPOINT *pPtsTmp;
				MBool bFilter=MFalse;
				SeedPoint.x = x; SeedPoint.y = y;
				ExtractBlob_4Con(pTmpBlob, lBlobLine, lWidth, lHeight, pMemTmp, lMemLen, &lBlobLen, 
					(MByte)inBlobLabel, (MByte)outBlobLabel, MNull, &SeedPoint);
				lTmpBlobLen = lBlobLen;
				pPtsTmp = (MPOINT *) pMemTmp;
				while (lTmpBlobLen-->0)
				{
					MPOINT ptCur = pPtsTmp[lTmpBlobLen];
					MLong maskX = ptCur.x, maskY = ptCur.y;
					MByte *pMaskTmp = (MByte*)pTmpMask+lMaskLine*maskY+maskX;
					if (*pMaskTmp==maskLabel)
						lInMaskNum++;
				}
				if (lInMaskNum*100>lBlobLen*Percentage)
					bFilter = MTrue;
				if (bFilter)
				{
					lTmpBlobLen = lBlobLen;
					while (lTmpBlobLen-->0)
					{
						MPOINT ptCur = pPtsTmp[lTmpBlobLen];
						MLong maskX = ptCur.x, maskY = ptCur.y;
						MByte *pBlobTmp = (MByte*)pTmpBlob+lBlobLine*maskY+maskX;
						*pBlobTmp = 255;
					}
				}
			}
		}
}

///*
//(transforming the label 0 to 1)
MVoid FilterBlob8Con(MVoid* pMaskSrc, MLong lMaskLine, 
					  MLong lMaskWidth, MLong lMaskHeight, 
					  MVoid *pMemTmp, MLong lMemLen,
					  MByte inLabel,  MByte OutLabel,
					  MLong lFilterSize, MLong lRoundThres,
					  MLong lFaceAngle)
{
	MLong x,y;
	MPOINT SeedPoint;
	MLong lBlobLen=0;
//	MPOINT PointList[5];//0-left,1-right,2-top,3-bottom,4-center
	MByte *pTmpMask = (MByte*)pMaskSrc;
	MLong lExt=lMaskLine-lMaskWidth;
	MLong lRoundThres1 =  (lRoundThres<< 16);
	MLong lRoundThres2 = ((lRoundThres*282)>>8)<<16;
	MLong lMaxSize;
	MLong maxLenEle;

	MRECT VIPRect={0};

	fnSize pfnSize=MNull;
	switch (lFaceAngle)
	{
	case 1:
		pfnSize = &Size_0Degree;
		break;
	case 2:
		pfnSize = &Size_90Degree;
		break;
	case 3:
		pfnSize = &Size_270Degree;
		break;
	case 4:
		pfnSize = &Size_180Degree;
		break;
	default:
		pfnSize = &Size_0Degree;
		break;
	}
	
	(*pfnSize)(lMaskWidth, lMaskHeight, &VIPRect);

// 	if (lMaskWidth>=NORM_SIZE_1) maxLenEle = MAX_LEN_ELEMENT_1;
// 	else /*if (lMaskWidth>=NORM_SIZE_2) */
// 		maxLenEle = MAX_LEN_ELEMENT_2;
//	maxLenEle = 3*maxLenEle/2;
	maxLenEle = MAX_LEN_ELEMENT_1;
	maxLenEle = 3*maxLenEle/2;

	for(y = 0; y < lMaskHeight; y++, pTmpMask+=lExt)
	{
		for(x = 0; x < lMaskWidth; x++, pTmpMask++)
		{
			SeedPoint.x = x, SeedPoint.y = y;
			if(*pTmpMask==inLabel)
			{
				MLong bFilter = MFalse;
				MLong lPhi = 0, lAlpha = 40;
				MLong lDeltaX, lDeltaY;
				MRECT maxRect={0};
				ExtractBlob_4Con(pTmpMask, lMaskLine, lMaskWidth, lMaskHeight, pMemTmp, lMemLen, \
					&lBlobLen, inLabel, OutLabel, &maxRect, &SeedPoint);
				//filter round blob(it may be spots)
				if  (y>VIPRect.top&&y<VIPRect.bottom&&x>VIPRect.left&&x<VIPRect.right)
				{
					lMaxSize = lFilterSize/2;
					lRoundThres = lRoundThres2;
				}
				else
				{
					lRoundThres = lRoundThres1;
					lMaxSize = lFilterSize;
				}
				if (lBlobLen < lMaxSize)
					bFilter = MTrue;
				if (!bFilter)
				{
// 					MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
// 					MLong lTmpLen = lBlobLen;
// 					PointList[0].x = PointList[1].x =PointList[2].x = PointList[3].x =pPtsTmp[0].x;
// 					PointList[0].y = PointList[1].y =PointList[2].y = PointList[3].y =pPtsTmp[0].y;
// 					while (lTmpLen-->0)
// 					{
// 						MPOINT ptCur = pPtsTmp[lTmpLen];
// 						if (PointList[0].x>ptCur.x) PointList[0].x=ptCur.x;
// 						if (PointList[1].x<ptCur.x) PointList[1].x=ptCur.x;
// 						if (PointList[2].y>ptCur.y) PointList[2].y=ptCur.y;
// 						if (PointList[3].y<ptCur.y) PointList[3].y=ptCur.y;
//					}
// 					lDeltaX = ABS(PointList[1].x - PointList[0].x)+1;
// 					lDeltaY = ABS(PointList[3].y - PointList[2].y)+1;
					lDeltaX = ABS(maxRect.right-maxRect.left)+1;
					lDeltaY = ABS(maxRect.bottom-maxRect.top)+1;
					if (lDeltaY>=maxLenEle || lDeltaX>=maxLenEle) continue;//too large 
					
					{
						MLong lTanVal=0, lDiv=0;
						lDiv = lDeltaX<lDeltaY?lDeltaX*20/lDeltaY:lDeltaY*20/lDeltaX;
						if (lDiv>=20)lDiv = 20;
						lTanVal = G_ATAN[lDiv];
						lPhi =  lAlpha * lTanVal + (100-lAlpha)*(lBlobLen<<16)/(lDeltaX*lDeltaY);
					}
					if (lPhi > lRoundThres)
						bFilter = MTrue;
				}
				if (bFilter)
				{
					MPOINT *pPtsTmp = (MPOINT *) pMemTmp;
					while (lBlobLen-->0)
					{
						MPOINT ptCur = pPtsTmp[lBlobLen];
						MLong maskX = ptCur.x, maskY = ptCur.y;
						MByte *pMaskTmp = (MByte*)pTmpMask+lMaskLine*maskY+maskX;
						*pMaskTmp = 255;
					}
				}
			}
		}
	}
}

MRESULT DeleteBlob4Con(MHandle hMemMgr,
					 MVoid* pMaskSrc, MLong lMaskLine, 
					 MLong lWidth, MLong lHeight, 
					 MByte inputLabel,
					 MPOINT* pPtSeed)
{
	MRESULT res=LI_ERR_NONE;
	MLong north, west, east, south;

	MPOINT* pMemTmp=MNull, *pPtsTmp;
	MLong lPtsNum, lBasePos;

	AllocVectMem(hMemMgr, pMemTmp, MAX_START_POINT, MPOINT);
	SetVectZero(pMemTmp, MAX_START_POINT*sizeof(MPOINT));
	pPtsTmp = pMemTmp;
	lPtsNum=0,lBasePos=0;
	pPtsTmp->x = pPtSeed->x;
	pPtsTmp->y = pPtSeed->y;
	lPtsNum++;
	
	do 
	{
		MPOINT ptCur = pPtsTmp[lBasePos];
		MLong maskX = ptCur.x, maskY = ptCur.y;
		MByte *pMaskTmp = (MByte*)pMaskSrc+lMaskLine*maskY+maskX;
		
		*pMaskTmp = 255;
		lPtsNum--;lBasePos++;//blob size
		if(maskX< 0 || maskX>lWidth) continue;
		if(maskY<0 || maskY> lHeight) continue;
		if (lPtsNum+4>=MAX_START_POINT) continue;
		north = maskY-1;
		west = maskX-1;
		east = maskX+1;
		south = maskY+1;
		
		//north
		if (north >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - lMaskLine;
			if(*pMaskTmp2==inputLabel)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = maskX; pPts->y = north;
				lPtsNum ++;
				*pMaskTmp2=255;//temp flag
			}
		}
		
		//west
		if (west >= 0)
		{
			MByte *pMaskTmp2 = pMaskTmp - 1;
			if(*pMaskTmp2==inputLabel)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = west; pPts->y = maskY;
				lPtsNum ++;
				*pMaskTmp2=255;//temp flag
			}
		}
		
		//east
		if (east < lWidth)
		{
			MByte *pMaskTmp2 = pMaskTmp + 1;
			if(*pMaskTmp2==inputLabel)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = east; pPts->y = maskY;
				lPtsNum ++;
				*pMaskTmp2=255;//temp flag
			}
		}
		
		//south
		if (south < lHeight)
		{
			MByte *pMaskTmp2 = pMaskTmp + lMaskLine;
			if(*pMaskTmp2==inputLabel)
			{
				MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
				pPts->x = maskX; pPts->y = south;
				lPtsNum ++;
				*pMaskTmp2=255;//temp flag
			}
		}
	} while(lPtsNum>0);
EXT:
	FreeVectMem(hMemMgr, pMemTmp);
	return res;
}

MVoid Size_0Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect)
{
	pVIPRect->left = lWidth/4;
	pVIPRect->right = lWidth*3/4;
	pVIPRect->top = lHeight/2;
	pVIPRect->bottom = lHeight*3/4;
}

MVoid Size_90Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect)
{
	pVIPRect->left = lWidth/2;
	pVIPRect->right = lWidth*3/4;
	pVIPRect->top = lHeight/4;
	pVIPRect->bottom = lHeight*3/4;
} 
MVoid Size_180Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect)
{
	pVIPRect->left = lWidth/4;
	pVIPRect->right = lWidth*3/4;
	pVIPRect->top = lHeight/4;
	pVIPRect->bottom = lHeight/2;
}
MVoid Size_270Degree(MLong lWidth, MLong lHeight, MRECT *pVIPRect)
{
	pVIPRect->left = lWidth/4;
	pVIPRect->right = lWidth/2;
	pVIPRect->top = lHeight/4;
	pVIPRect->bottom = lHeight*3/4;
}

//ATTENTION: InFlag != OutFlag
//lNghLength is not accurate
MVoid ExtractBlob_8ConEXT(MVoid* pMaskData, MLong lMaskLine, 
    MLong lMaskWidth, MLong lMaskHeight, 
    MVoid *pQueue, MLong lQueueMaxLen, 
    MLong* plBlobLen, 
    MByte InFlag, MByte OutFlag, 
    MPOINT *pSeed)
{
    MLong lPtsNum=0, lBasePos=0;
    MLong north, west, east, south;
    MPOINT *pPtsTmp=(MPOINT*)pQueue;//queue start

    MLong lBNorth, lBWest, lBEast, lBSouth;
    lBNorth = -pSeed->y;
    lBWest = -pSeed->x;
    lBEast = lMaskWidth-pSeed->x;
    lBSouth = lMaskHeight-pSeed->y;

    pPtsTmp[lPtsNum].x = 0;
    pPtsTmp[lPtsNum].y = 0;
    lPtsNum++;

    do 
    {
        MPOINT ptCur = pPtsTmp[lBasePos];
        MLong maskX = ptCur.x, maskY = ptCur.y;
        MByte *pMaskTmp = (MByte*)pMaskData+lMaskLine*maskY+maskX;

        *pMaskTmp = OutFlag;
        lPtsNum--;lBasePos++;//blob size
        if(maskX< lBWest || maskX>lBEast) continue;
        if(maskY<lBNorth || maskY> lBSouth) continue;
        if (lBasePos+lPtsNum+4>=lQueueMaxLen) continue;
        north = maskY-1;
        west = maskX-1;
        east = maskX+1;
        south = maskY+1;

        //north
        if (north >= lBNorth)
        {
            MByte *pMaskTmp2 = pMaskTmp - lMaskLine;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = maskX; pPts->y = north;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //northwest
        if (north>=lBNorth&&west>=lBWest)
        {	
            MByte *pMaskTmp2 = pMaskTmp - 1-lMaskLine;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = west; pPts->y = north;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }

        //west
        if (west >= lBWest)
        {
            MByte *pMaskTmp2 = pMaskTmp - 1;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = west; pPts->y = maskY;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //southwest
        if (south < lBSouth && west>=lBWest)
        {
            MByte *pMaskTmp2 = pMaskTmp + lMaskLine-1;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = west; pPts->y = south;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //south
        if (south < lBSouth)
        {
            MByte *pMaskTmp2 = pMaskTmp + lMaskLine;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = maskX; pPts->y = south;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //southeast
        if (south < lBSouth&&east<lBEast)
        {
            MByte *pMaskTmp2 = pMaskTmp + lMaskLine+1;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = east; pPts->y = south;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //east
        if (east < lBEast)
        {
            MByte *pMaskTmp2 = pMaskTmp + 1;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = east; pPts->y = maskY;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }
        //northeast
        if (east < lBEast&&north>= lBNorth)
        {
            MByte *pMaskTmp2 = pMaskTmp + 1-lMaskLine;
            if(*pMaskTmp2==InFlag)
            {
                MPOINT *pPts = pPtsTmp+lBasePos+lPtsNum;
                pPts->x = east; pPts->y = north;
                lPtsNum ++;
                *pMaskTmp2=255-1;//temp flag
            }
        }


    } while(lPtsNum>0);	
    if (plBlobLen!=MNull)
        *plBlobLen = lBasePos;
    return;
}

JSTATIC MVoid SortGSeeds(JGSEED* gSeed)
 {
	 MLong x, y;
	 for (x = 0; x<gSeed->lSeedNum; x++)
	 {
		 MLong lDistMin = gSeed->pcrSeed[x]; 
		 MLong lIndexMin = x;
		 for (y = x+1; y<gSeed->lSeedNum; y++)
		 {
			 if (gSeed->pcrSeed[y] < lDistMin)
			 {
				 lDistMin = gSeed->pcrSeed[y];
				 lIndexMin = y;
			 }
		 }
		 
		 {
			 MPOINT ptTmp = gSeed->pptSeed[x];
			 MByte crTmp = gSeed->pcrSeed[x];
			 gSeed->pptSeed[x] = gSeed->pptSeed[lIndexMin];
			 gSeed->pptSeed[lIndexMin] = ptTmp;
			 
			 gSeed->pcrSeed[x] = gSeed->pcrSeed[lIndexMin];
			 gSeed->pcrSeed[lIndexMin] = crTmp;
		 }
	 }	
}

MRESULT SoftThres(MHandle hMemMgr, MByte* pSrc, MLong lSrcLine, 
				  MByte* pRlt, MLong lRltLine,
						  MLong lWidth, MLong lHeight, 
						  MVoid *pTmpMem, MLong lMemLen, 
						  MLong lHighThres, MLong lLowThres, MLong lBaseLowRate,
						  MByte rltLabel, JGSEED *pSeeds)
{
	MRESULT res = LI_ERR_NONE;
	MLong x, y, lExt, lRltExt;
	MPOINT SeedPoint;
	MByte *pTmpSrc, *pTmpRlt;
	MPOINT* pTmpStack = (MPOINT*)pTmpMem;
	MLong lStackLen=0;

	JASSERT(lHighThres>=lLowThres);

	//lBaseLowRate = ((lBaseLowRate<<8)+50)/100;

	//local maximum
	pTmpSrc = (MByte*)pSrc;
	pTmpRlt  =(MByte*)pRlt;
	lRltExt = lRltLine - lWidth;
	lExt = lSrcLine - lWidth;
	for(y = 0; y < lHeight; y++, pTmpSrc+=lExt, pTmpRlt+=lRltExt)
		for(x = 0; x < lWidth; x++, pTmpSrc++, pTmpRlt++)
		{
			MByte Cur;
			if (x==0||y==0||x==lWidth-1||y==lHeight-1)
			{
				*pTmpRlt=0;
				continue;
			}
			Cur= *pTmpSrc;
			SeedPoint.x = x, SeedPoint.y = y;
			if (Cur > lHighThres )
			{
				if (Cur >= *(pTmpSrc-1)
				  &&Cur >= *(pTmpSrc+1)
				  &&Cur >= *(pTmpSrc-lSrcLine)
				  &&Cur >= *(pTmpSrc+lSrcLine)
				  && lStackLen<lMemLen)
				{
					pSeeds->pptSeed[lStackLen].x = x; 
					pSeeds->pptSeed[lStackLen].y = y;
					pSeeds->pcrSeed[lStackLen] = Cur;
					lStackLen++;
				}
			}
			else if (Cur<lLowThres)
			{	
				*pTmpRlt = 0;
			}

		}
	pSeeds->lSeedNum = lStackLen;

	SortGSeeds(pSeeds);

	while (lStackLen-->0)
	{
		MPOINT *pPtsTmp = (MPOINT *)pTmpStack;
		MLong lPtsNum = 0;
		MLong lUltLowThres;
		MLong lLowRate;
		//MByte rltLabel;
		
		pPtsTmp[lPtsNum].x = pSeeds->pptSeed[lStackLen].x;
		pPtsTmp[lPtsNum].y = pSeeds->pptSeed[lStackLen].y;
		lPtsNum++;

		lUltLowThres = pSeeds->pcrSeed[lStackLen];
// 		if (lUltLowThres>=255) rltLabel = 64;
//		else
//		rltLabel = 0;
		JASSERT(lHighThres<256);
		lLowRate = lBaseLowRate-(lUltLowThres-lHighThres)*10/(256-lHighThres);
		lLowRate = ((lLowRate<<8)+50)/100;
		lUltLowThres = (lUltLowThres*lLowRate+128)>>8;//lLowThres = 0.8*lHighThres
		//lLowThres = (((lLowThres-128)*lLowRate)>>8)+128;
		//lUltLowThres = MAX(lUltLowThres, lLowThres);
		//if (lUltLowThres <=lLowThres) 
		//{
			//*(pRlt+lRltLine*pSeeds->pptSeed[lStackLen].y+pSeeds->pptSeed[lStackLen].x) = 0;
			//continue;
		//	lUltLowThres = lLowThres;
		//}
		
		do 
		{
 			MPOINT ptCur = pPtsTmp[--lPtsNum];
			MLong maskX = ptCur.x, maskY = ptCur.y;
			MByte *pMaskTmp = (MByte*)pRlt+lRltLine*maskY+maskX;
			MByte *pMaskSrc = (MByte*)pSrc+lSrcLine*maskY+maskX;
			
			if(maskX<=0 || maskX>=lWidth-1) continue;
			if(maskY<=0 || maskY>=lHeight-1) continue;
			if(lPtsNum>=lMemLen-4) continue;

			//如果已经被标记了，是否不再标记
			if (*pMaskTmp==rltLabel)
				continue;
			
			*pMaskTmp = rltLabel;
			
			//south
			if (maskY+1 < lHeight)
			{
				MByte *pMaskTmp2 = pMaskTmp + lRltLine;
				MByte *pMaskSrc2 = pMaskSrc + lSrcLine;
				if (*pMaskSrc2>=lUltLowThres && *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{	
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX;
					pPtsTmp[lPtsNum++].y = maskY+1;
				}
			}
			//southwest
			if (maskY+1 < lHeight || maskX-1 >= 0)
			{
				MByte *pMaskTmp2 = pMaskTmp + lRltLine - 1;
				MByte *pMaskSrc2 = pMaskSrc + lSrcLine - 1;
				if (*pMaskSrc2>=lUltLowThres && *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{	
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX-1;
					pPtsTmp[lPtsNum++].y = maskY+1;
				}
			}
			//west
			if (maskX-1 >= 0)
			{
				MByte *pMaskTmp2 = pMaskTmp - 1;
				MByte *pMaskSrc2 = pMaskSrc - 1;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX-1;
					pPtsTmp[lPtsNum++].y = maskY;
				}
			}
			//northwest
			if (maskX-1 >= 0||maskY-1 >= 0)
			{
				MByte *pMaskTmp2 = pMaskTmp - 1 - lRltLine;
				MByte *pMaskSrc2 = pMaskSrc - 1 - lSrcLine;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX-1;
					pPtsTmp[lPtsNum++].y = maskY-1;
				}
			}
			//north
			if (maskY-1 >= 0)
			{
				MByte *pMaskTmp2 = pMaskTmp - lRltLine;
				MByte *pMaskSrc2 = pMaskSrc - lSrcLine;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX;
					pPtsTmp[lPtsNum++].y = maskY-1;
				}
			}
			//northeast
			if (maskY-1 >= 0||maskX+1 < lWidth)
			{
				MByte *pMaskTmp2 = pMaskTmp - lRltLine+1;
				MByte *pMaskSrc2 = pMaskSrc - lSrcLine+1;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX+1;
					pPtsTmp[lPtsNum++].y = maskY-1;
				}
			}
			//east
			if (maskX+1 < lWidth)
			{
				MByte *pMaskTmp2 = pMaskTmp + 1;
				MByte *pMaskSrc2 = pMaskSrc + 1;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX+1;
					pPtsTmp[lPtsNum++].y = maskY;
				}
			}
			//southeast
			if (maskX+1 < lWidth||maskY+1 < lHeight)
			{
				MByte *pMaskTmp2 = pMaskTmp + 1+lRltLine;
				MByte *pMaskSrc2 = pMaskSrc + 1+lSrcLine;
				if (*pMaskSrc2>=lUltLowThres&& *pMaskTmp2!=rltLabel && *pMaskTmp2!=1)
				{
					*pMaskTmp2 = 1;
					pPtsTmp[lPtsNum].x = maskX+1;
					pPtsTmp[lPtsNum++].y = maskY+1;
				}
			}

		} while(lPtsNum>0);
	}
	return res;
} 

MVoid AdjustPosByGravity(MByte *pRsp, MLong lRspLine, MLong lWidth, MLong lHeight,
	JGSEED *pSeeds, MLong lRadius)
{
	MLong i, j, k, m, n;
	MLong lYStart, lYEnd, lXStart, lXEnd;
	MLong lMaxVal, lMaxXPos, lMaxYPos;
	MLong halfRadius;
	JASSERT(pSeeds && pRsp);

	if(lRadius<=1)
		halfRadius = lRadius;
	else
		halfRadius = lRadius / 2;
	for(k=0; k<pSeeds->lSeedNum; k++)
	{
		lYStart = MAX(0, pSeeds->pptSeed[k].y-lRadius);
		lYEnd = MIN(pSeeds->pptSeed[k].y+lRadius, lHeight-1);
		lXStart = MAX(0, pSeeds->pptSeed[k].x-lRadius);
		lXEnd = MIN(pSeeds->pptSeed[k].x+lRadius, lWidth-1);
		lMaxVal = 0;
		for (j=lYStart; j<=lYEnd; j++)
		{
			for (i=lXStart; i<=lXEnd; i++)
			{
				MLong SumVal = 0;
				MLong lPixelNum=0;
				for (m = MAX(0, j-halfRadius); m<MIN(j+halfRadius, lHeight-1); m++)
				{
					for (n = MAX(0, i-halfRadius); n<MIN(i+halfRadius, lWidth-1); n++)
					{
						SumVal+=*(pRsp+(m)*lRspLine+(n));
						lPixelNum++;
					}
				}
				if (lMaxVal < SumVal/lPixelNum)
				{
					lMaxVal = SumVal/lPixelNum;
					lMaxXPos = i;
					lMaxYPos = j;
				}
			}
		}
		if (lMaxVal>0)
		{
			pSeeds->pptSeed[k].x = lMaxXPos;
			pSeeds->pptSeed[k].y = lMaxYPos;
		}
	}

	for(k=0; k<pSeeds->lSeedNum; k++)
	{
		*(pRsp + pSeeds->pptSeed[k].y * lRspLine + pSeeds->pptSeed[k].x) = 255;
	}	
}

//lRadius是用于计算局部区域的半径
MRESULT LocalMax_Circle(MHandle hMemMgr, MByte* pSrc, MLong lSrcLine, 
						  MByte* pRlt, MLong lRltLine,
						  MLong lWidth, MLong lHeight, 
						  MVoid *pTmpMem, MLong lMemLen, 
						  MLong lHighThres, MLong lLowThres, MLong lBaseLowRate,
						  MByte rltLabel, JGSEED *pSeeds, MLong lRadius)
{
	MRESULT res = LI_ERR_NONE;
	MLong i,j,lSrcExt,lRltExt;
	MLong k, l;
	MByte *pSrcData, *pRltData;
	SoftThres(hMemMgr, pSrc, lSrcLine, pRlt, lRltLine, lWidth, lHeight, 
		pTmpMem, lMemLen, lHighThres, lLowThres, lBaseLowRate, rltLabel, pSeeds);

	//求联通区域

	pSeeds->lSeedNum = 0;
	lSrcExt = lSrcLine - lWidth+2*lRadius;
	lRltExt = lRltLine - lWidth+2*lRadius;
	//top少于lRadius的区域

	//中间区域
	pSrcData = pSrc+lSrcLine*lRadius+lRadius;
	pRltData = pRlt+lRltLine*lRadius+lRadius;
	for(j=lRadius; j<lHeight-lRadius; j++, pSrcData+=lSrcExt,pRltData+=lRltExt)
	{
		for (i=lRadius; i<lWidth-lRadius; i++, pSrcData++,pRltData++)
		{
			if (*pRltData == rltLabel)
			{
				//求局部最大
				for (k =-lRadius; k<lRadius; k++)
				{
					for (l=-lRadius; l<lRadius; l++)
					{
						if (*pSrcData<*(pSrcData+k*lSrcLine+l)
							|| *(pRltData+k*lRltLine+l)==250)
							break;
					}
					if(l!=lRadius)break;
				}
				if (k==lRadius&& l==lRadius)
				{//A seed, enqueue
					pSeeds->pptSeed[pSeeds->lSeedNum].x = i;
					pSeeds->pptSeed[pSeeds->lSeedNum].y = j;
					pSeeds->pcrSeed[pSeeds->lSeedNum] = *pSrcData;
					pSeeds->lSeedNum++;

					*(pRltData) = 250;
				}
			}
		}
	}
	//bottom小于lRadius的区域
	
//EXT:
	return res;
}


MRESULT ridgePoint(MHandle hMemMgr, MByte *pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeight,
					MByte *pLabelData, MLong lLabelLine, MLong lRadius,
					MLong lThreshold, MByte ridgeLabel, MByte MaxLabel, JGSEED *pSeeds)
{
	MRESULT res = LI_ERR_NONE;
	MLong i, j, m, n;
	MByte *pTmpSrc = pSrcData;
	MByte* pTmpLabel = pLabelData;
	MByte AccessedLabel = 0;
	MLong lExt = lSrcLine - lWidth+2*lRadius;
	MLong lLabelExt = lLabelLine - lWidth + 2*lRadius;
	
	JGSEED seed = {0};
	MLong lMaxSeedLen = lWidth*lHeight/10;
	AllocVectMem(hMemMgr, seed.pptSeed, lMaxSeedLen, MPOINT);
	AllocVectMem(hMemMgr, seed.pcrSeed, lMaxSeedLen, MByte);
	seed.lSeedNum = 0;

	pTmpSrc += lRadius * lSrcLine + lRadius;
	pTmpLabel += lRadius * lLabelLine+ lRadius;

	for (j=lRadius; j<lHeight-lRadius; j++,pTmpSrc+=lExt, pTmpLabel+=lLabelExt)
	{	
		for (i=lRadius; i<lWidth-lRadius; i++, pTmpSrc++,pTmpLabel++)
		{
			if (*pTmpSrc < lThreshold ) continue;
			if (*pTmpLabel == AccessedLabel || *pTmpLabel == MaxLabel )continue;
			
			//种子点
			seed.pptSeed[0].x = i;
			seed.pptSeed[0].y = j;
			seed.pcrSeed[0] = *pTmpSrc;
			seed.lSeedNum=1;
			while (seed.lSeedNum>0)
			{
				MByte MaxByte = seed.pcrSeed[seed.lSeedNum-1];
				MLong MaxX, MaxY;
				MByte *pTmpLabel2 = pLabelData+seed.pptSeed[seed.lSeedNum-1].y * lLabelLine +seed.pptSeed[seed.lSeedNum-1].x; 
				MByte *pTmpSrc2 = pSrcData+seed.pptSeed[seed.lSeedNum-1].y * lSrcLine+seed.pptSeed[seed.lSeedNum-1].x; 
				MaxX = 0, MaxY = 0;

				for (m =-lRadius; m<lRadius; m++)
				{
					for (n=-lRadius; n<lRadius; n++)
					{
						if (m+seed.pptSeed[seed.lSeedNum-1].x<0 || m+seed.pptSeed[seed.lSeedNum-1].x>=lWidth
							||n+seed.pptSeed[seed.lSeedNum-1].y<0 || n+seed.pptSeed[seed.lSeedNum-1].y>=lHeight)
							continue;
						if (*(pTmpLabel2+m*lLabelLine+n)== AccessedLabel)//所有的不能访问的点和已访问的点都要被标记
							continue;
						
						if (*(pTmpLabel2+m*lLabelLine+n)==MaxLabel)
							break;
						*(pTmpLabel2 +m *lLabelLine+n) =AccessedLabel;
						if (MaxByte<*(pTmpSrc2+m*lSrcLine+n))
						{
							MaxByte = *(pTmpSrc2+m*lSrcLine+n);
							MaxX = n; MaxY = m;
						}
					}
					if (n!=lRadius)
						break;
				}
				if (m!=lRadius)//是break出来的，也就是说到点了
				{
					seed.lSeedNum--;
					break;
				}
				if (seed.lSeedNum>=lMaxSeedLen)
					break;
				if (MaxX == 0 && MaxY==0)
					break;

				//要在法定的范围内
				if (seed.pptSeed[seed.lSeedNum-1].x+MaxX > lRadius && seed.pptSeed[seed.lSeedNum-1].x+MaxX<lWidth-lRadius
					&&seed.pptSeed[seed.lSeedNum-1].y+MaxY>lRadius &&seed.pptSeed[seed.lSeedNum-1].y+MaxY<lHeight-lRadius)
				{
					seed.pptSeed[seed.lSeedNum].x = seed.pptSeed[seed.lSeedNum-1].x+MaxX;
					seed.pptSeed[seed.lSeedNum].y = seed.pptSeed[seed.lSeedNum-1].y+MaxY;
					seed.pcrSeed[seed.lSeedNum] = MaxByte;
					seed.lSeedNum ++;
				}
			}

			if (seed.lSeedNum>2 && pSeeds!=MNull)
			{
				for (m=1; m<seed.lSeedNum;m+=2)
				{
					if (seed.pptSeed[m].y<0||seed.pptSeed[m].y>=lHeight
						|| seed.pptSeed[m].x<0||seed.pptSeed[m].y>=lWidth)
						continue;
					*(pLabelData+seed.pptSeed[m].y*lLabelLine+seed.pptSeed[m].x) = MaxLabel;
					pSeeds->pptSeed[pSeeds->lSeedNum].x = seed.pptSeed[m].x;
					pSeeds->pptSeed[pSeeds->lSeedNum].y = seed.pptSeed[m].y;
					pSeeds->lSeedNum++;
				}
			}
			
		}
	}

EXT:
	FreeVectMem(hMemMgr, seed.pptSeed);
	FreeVectMem(hMemMgr, seed.pcrSeed);
	return res;
}



