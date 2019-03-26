#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "liedge.h"
#include "liresize.h"
//#include "bbmorph.h"
#include "liimage.h"
#include "bbgeometry.h"
#include "blobfilter.h"
#include "chinesetransform.h"
#include <math.h>

#define __TEST_CHINTRANS__

typedef struct tag_OrientList
{
	MLong lSegIndex;
	MLong lVectLen;
	MPOINT *pPointVect;
}ORIENT_LIST;

typedef struct tag_PointClass
{
	MLong lClassNum;
	ORIENT_LIST* pClass;
}POINTCLASS;

JSTATIC MBool bIsInValidBlob(JOFFSCREEN *pImgSrc, MPOINT* pPtStart, MPOINT* pPtEnd, MCOLORREF ref);
JSTATIC MLong vComputeAngleDist(MShort Gx, MShort Gy, MLong classNum);
JSTATIC MRESULT SoftThres(MHandle hMemMgr, MByte* pSrc, MLong lSrcLine, 
						  MByte* pRlt, MLong lRltLine,
						  MLong lWidth, MLong lHeight, 
						  MVoid *pTmpMem, MLong lMemLen, 
						  MLong lHighThres, MLong lLowThres, MLong lBaseLowRate,
						  MByte rltLabel, JGSEED *pSeeds);
JSTATIC MDouble GetAngleDist(MDouble *eigenCur, MDouble *eigenRef, MLong lVectLen);
JSTATIC MLong	GetColorDist(MLong cr1, MLong cr2, MLong cr3, MCOLORREF crCenter);

JSTATIC MLong NeighborLieOnSameDirect(JOFFSCREEN* pImgSrc, JINFO_BLOB_GRAPH* pInfoGraph, MLong lLabelSeed, MDouble* pfMinDirDiff, MLong minLDist);
JSTATIC MVoid MergeBlob(JINFO_BLOB* pInfoBlob, 
				MLong lBlobNoFrom, MLong lBlobNoTo, MLong lLabelNum);
JSTATIC MVoid UpdateBlobGraph(JINFO_BLOB_GRAPH *pInfoGraph);
JSTATIC MLong BlobDist(JINFO_BLOB* pInfoBlob, JINFO_BLOB* pInfoRef);

#ifndef __TEST_CHINTRANS__
#define CLASS_NUM	8
MRESULT ChinTransform(MHandle hMemMgr, 
#ifdef USE_COLOR_GRADIENT
					  JOFFSCREEN *pSrcImg,
#else
					  MByte* pSrcData, MLong lSrcLine, 
#endif
					  MByte* pMaskData, MLong lMaskLine,
					  MLong lWidth, MLong lHeight,
					  MByte* pRltData, MLong lRltLine)
{
	MLong x,y,i;
	MRESULT res=LI_ERR_NONE;
	MShort* Gx=MNull, *Gy=MNull, *pMag=MNull;
	MShort* pTmpGx, *pTmpGy;
	MLong lGradExt,thres=0;
	MLong lGradLine = JMemLength(lWidth);
	MDWord MaxGrad=0, GradPtNum=0;
	POINTCLASS Class={0};
	MLong lPreFixLen;
	MUInt16* pBuffer=MNull, *pTmpBuf, *pTmpD0, *pTmpD1, *pTmpD2, *pTmpD3;
	MLong lBufLine, lBufExt;
	MLong MaxResponse=-1;
	MByte* pTmpRlt;
	MLong lRltExt, factor;
	MUInt16 *D[CLASS_NUM/2]={0};
	MLong hist[256]={0};

	MLong lBinSize;
	MLong lTotalLen;
	MLong lThres;

	JASSERT(lRltLine==lMaskLine);

	lBufLine = JMemLength(lWidth);
	AllocVectMem(hMemMgr, Gx, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, Gy, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, pBuffer, lBufLine*lHeight, MUInt16);
	SetVectZero(Gx, lGradLine*lHeight*sizeof(MShort));
	SetVectZero(Gy, lGradLine*lHeight*sizeof(MShort));
	SetVectZero(pBuffer, lBufLine*lHeight*sizeof(MUInt16));

//	PrintChannel(pMaskData, lMaskLine,DATA_U8, lWidth, lHeight, 1, 0);
#ifdef USE_COLOR_GRADIENT
	ImgGradient(pSrcImg, Gx, Gy, lGradLine, pMaskData, lMaskLine, lWidth, lHeight, MNull, &MaxGrad, MNull);
#else
	Gradient_WithMask(pSrcData, lSrcLine, Gx, Gy, lGradLine,
		pMaskData, lMaskLine, lWidth, lHeight, MNull, &MaxGrad, &GradPtNum);
#endif
	//Display the gradient directions
	if (MaxGrad<=10) goto EXT;
	GO(HistoAnalysis(hMemMgr, Gx, Gy, lGradLine, pMaskData, lMaskLine, lWidth, lHeight, MaxGrad, EDGE_PERCENTAGE, &thres));
	if (thres<=10) goto EXT;

// 	pTmpGx = Gx;
// 	pTmpGy = Gy;
// 	pTmpRlt = pRltData;
// 	lGradExt = lGradLine- lWidth;
// 	lRltExt = lRltLine - lWidth;
// 	for (y=0;y<lHeight;y++,pTmpGx+=lGradExt,pTmpGy+=lGradExt,pTmpRlt+=lRltExt)
// 	{
// 		for (x=0; x<lWidth;x++,pTmpGx++,pTmpGy++,pTmpRlt++)
// 		{
// 			if(ABS(*(pTmpGx))+ABS(*(pTmpGy))>thres)
// 				*pTmpRlt = 255;
// 			else 
// 				*pTmpRlt = 0;
// 		}
// 	}
// 	PrintBmpEx(pRltData,lRltLine,DATA_U8,lWidth,lHeight,1,"C:\\edge.bmp");
//
	Class.lClassNum = CLASS_NUM;
	AllocVectMem(hMemMgr, Class.pClass, Class.lClassNum, ORIENT_LIST);
	SetVectZero(Class.pClass, Class.lClassNum*sizeof(ORIENT_LIST));

	lPreFixLen = lWidth*lHeight/10;
	for (x=0; x<Class.lClassNum; x++)
	{	
		ORIENT_LIST* pCurOrientList=Class.pClass+x;
		pCurOrientList->lSegIndex = x;
		pCurOrientList->lVectLen = 0;
		AllocVectMem(hMemMgr, pCurOrientList->pPointVect, lPreFixLen, MPOINT);
		SetVectZero(pCurOrientList->pPointVect, lPreFixLen*sizeof(MPOINT));
	}
	
	lGradExt = lGradLine - lWidth;
	pTmpGx = Gx, pTmpGy = Gy;
	for (y=0; y<lHeight; y++, pTmpGx+=lGradExt, pTmpGy+=lGradExt)
	{
		for (x=0; x<lWidth; x++,pTmpGx++, pTmpGy++)
		{
			MLong lIndex2;
			MShort gxVal=*pTmpGx, gyVal=*pTmpGy;
			ORIENT_LIST* pCurOrient;
			if (ABS(gxVal)+ABS(gyVal)<thres) continue;

			//orient classification
			lIndex2 = vComputeAngleDist(gxVal, gyVal, CLASS_NUM);
			pCurOrient = (Class.pClass+lIndex2);
			pCurOrient->pPointVect[pCurOrient->lVectLen].x = x;
			pCurOrient->pPointVect[pCurOrient->lVectLen].y = y;
			pCurOrient->lVectLen++;			
		}
	}

	//four pair

	AllocVectMem(hMemMgr, D[0], lRltLine*lHeight, MUInt16);
	AllocVectMem(hMemMgr, D[1], lRltLine*lHeight, MUInt16);
	AllocVectMem(hMemMgr, D[2], lRltLine*lHeight, MUInt16);
	AllocVectMem(hMemMgr, D[3], lRltLine*lHeight, MUInt16);
	SetVectZero(D[0], lRltLine*lHeight*sizeof(MUInt16));
	SetVectZero(D[1], lRltLine*lHeight*sizeof(MUInt16));
	SetVectZero(D[2], lRltLine*lHeight*sizeof(MUInt16));
	SetVectZero(D[3], lRltLine*lHeight*sizeof(MUInt16));
	for (i=0; i<Class.lClassNum/2; i++)
	{
		ORIENT_LIST* pCurList, *pPairList;
		MLong StartIndex=0, EndIndex=0;
		MUInt16* pDes = D[i];
		MUInt16* pSumDes = pBuffer;
		pCurList = Class.pClass+i;
		pPairList = pCurList+Class.lClassNum/2;
		
		for (y=0; y<pCurList->lVectLen; y++)
		{
			MBool bFirst=MTrue;
			MPOINT* pCVect = pCurList->pPointVect + y;
			for (x=StartIndex; x<pPairList->lVectLen; x++)
			{
				MPOINT* pPVect = pPairList->pPointVect+x;
				MLong lDist = vDistance2L(*pCVect, *pPVect);
				MPOINT midPoint;
				MLong offset;

				MLong index;
				if (pPVect->y-pCVect->y>MAX_DIST_BW_POINTS)
					break;

				if(bFirst&&pCVect->y-pPVect->y<MAX_DIST_BW_POINTS)
					StartIndex = MAX(x,0),bFirst = MFalse;

				if (lDist>MAX_DIST_BW_POINTS)
					continue;
				
				if (lDist<MIN_DIST_BW_POINTS)
					continue;

				//voting for midpoint
				midPoint.x = (pCVect->x + pPVect->x)/2;
				midPoint.y = (pCVect->y + pPVect->y)/2;
				offset = lBufLine*midPoint.y+midPoint.x;//ATTENTION!!!! here lRltLine == lMaskLine
				if (*(pMaskData+offset)==255)
				{
					(*(pDes+offset))++;
					(*(pSumDes+offset))++;
					if (MaxResponse<*(pDes+offset))
						MaxResponse = *(pDes+offset);
				}
			}
		}
	}
// 	PrintBmpEx(D[0], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\0.bmp");
// 	PrintBmpEx(D[1], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\1.bmp");
// 	PrintBmpEx(D[2], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\2.bmp");
// 	PrintBmpEx(D[3], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\3.bmp");
	pTmpBuf = pBuffer;
	pTmpD0 = D[0];
	pTmpD1 = D[1];
	pTmpD2 = D[2];
	pTmpD3 = D[3];
	lBufExt = lBufLine - lWidth;
	pTmpRlt = pRltData;
	lRltExt = lRltLine - lWidth;
	if (MaxResponse == 0)goto EXT;
	factor = ((255<<16)+MaxResponse/2)/MaxResponse;

	lBinSize = MAX(1,(MaxResponse+254)/255);
	lTotalLen = 0;

	pTmpGx = Gx, pTmpGy = Gy; 
	for (y = 0; y<lHeight; y++, pTmpBuf+=lBufExt, pTmpRlt+=lRltExt,
		pTmpD0+=lBufExt, pTmpD1+=lBufExt, pTmpD2+=lBufExt, pTmpD3+=lBufExt,
		pTmpGx+=lGradExt, pTmpGy+=lGradExt)
	{
		for (x=0; x<lWidth; x++, pTmpBuf++, pTmpRlt++, pTmpD0++,pTmpD1++,pTmpD2++,pTmpD3++,
			pTmpGx++,pTmpGy++)
		{
			MLong val = *pTmpBuf;
			if (val == 0 || (ABS(*pTmpGx)+ABS(*pTmpGy))>thres)//high thres is deliminished
				//*pTmpRlt = 0;
				*pTmpBuf = 0;
			else
			{
				//Normalize the bin level
				MUInt16 lD0 = *pTmpD0;
				MUInt16 lD1 = *pTmpD1;
				MUInt16 lD2 = *pTmpD2;
				MUInt16 lD3 = *pTmpD3;
				MLong Sum = lD0+lD1+lD2+lD3;
				MLong Sum2 = lD0*lD0+lD1*lD1+lD2*lD2+lD3*lD3;
				double Var = (Sum2*1.0/(Sum*Sum)- 1.0/4)/3;
				
				MLong MaxD = MAX(MAX(MAX(lD0, lD1), lD2), lD3);
				if (Var<0.1)
					//*pTmpRlt = 0;
					*pTmpBuf = 0;
				else
					//*pTmpRlt = TRIM_UINT8((MaxD*factor)>>16);
				{
					*pTmpBuf = MaxD;
					hist[MaxD/lBinSize]++;
					lTotalLen ++ ;
				}
				
			}
		}
	}
	
//	lThres = lTotalLen/100;
	lTotalLen = 0;
	for (y=255; y>=0; y--)
	{
		lTotalLen += hist[y];

		if (lTotalLen>=5)break;
	}
	//lThres = y*MAX(1,MaxResponse*/256);
	lThres = y * lBinSize;
	lThres = (lThres+MaxResponse)/2;
	//factor = 256/lThres;
	factor = ((255<<16)+lThres/2)/lThres;
	//
	pTmpBuf = pBuffer;
	pTmpRlt = pRltData;
	for (y=0; y<lHeight;y++,pTmpBuf+=lBufExt,pTmpRlt+=lRltExt)
	{
		for (x=0; x<lWidth; x++,pTmpBuf++,pTmpRlt++)
		{
			if (*pTmpBuf==0)
			{
				*pTmpRlt = 0;
			}
			else
			{
				*pTmpRlt = TRIM_UINT8(((*pTmpBuf)*factor)>>16);
			}
		}
	}
	//
	//CpyImgMem(pRltData, lRltLine, pBuffer, lBufLine, lWidth, lHeight, MShort);
EXT:

	for (x=0; x<Class.lClassNum; x++)
	{	
		ORIENT_LIST* pCurOrientList=Class.pClass+x;
		FreeVectMem(hMemMgr, pCurOrientList->pPointVect);
	}

	FreeVectMem(hMemMgr, D[0]);
	FreeVectMem(hMemMgr, D[1]);
	FreeVectMem(hMemMgr, D[2]);
	FreeVectMem(hMemMgr, D[3]);
	FreeVectMem(hMemMgr, Class.pClass);
	FreeVectMem(hMemMgr, pBuffer);
	FreeVectMem(hMemMgr, Gx);
	FreeVectMem(hMemMgr, Gy);

	return res;
}
#endif

#ifdef __TEST_CHINTRANS__
#define CLASS_NUM			32
MRESULT ChinTransform(MHandle hMemMgr, 
#ifdef USE_COLOR_GRADIENT
					  JOFFSCREEN *pSrcImg,
#else
					  MByte* pSrcData, MLong lSrcLine, 
#endif
					  MByte* pMaskData, MLong lMaskLine,
					  MLong lWidth, MLong lHeight,
					  MByte* pRltData, MLong lRltLine)
{
	MLong x,y,i;
	MRESULT res=LI_ERR_NONE;
	MShort* Gx=MNull, *Gy=MNull, *pMag=MNull;
	MShort* pTmpGx, *pTmpGy;
	MLong lGradExt,thres=0;
	MLong lGradLine = JMemLength(lWidth);
	MDWord MaxGrad=0, GradPtNum=0;
	POINTCLASS Class={0};
	MLong lPreFixLen;
	MUInt16* pBuffer=MNull, *pTmpBuf, *pTmpD0, *pTmpD1, *pTmpD2, *pTmpD3;
	MLong lBufLine, lBufExt;
	MLong MaxResponse=-1;
	MByte* pTmpRlt;
	MLong lRltExt, factor;
	MUInt16 *D[CLASS_NUM/2]={0};
	MLong hist[256]={0};

	MLong lBinSize;
	MLong lTotalLen;
	MLong lThres;

	JASSERT(lRltLine==lMaskLine);
	lBufLine = JMemLength(lWidth);
	AllocVectMem(hMemMgr, Gx, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, Gy, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, pBuffer, lBufLine*lHeight, MUInt16);
	SetVectZero(Gx, lGradLine*lHeight*sizeof(MShort));
	SetVectZero(Gy, lGradLine*lHeight*sizeof(MShort));
	SetVectZero(pBuffer, lBufLine*lHeight*sizeof(MUInt16));

//	PrintChannel(pMaskData, lMaskLine,DATA_U8, lWidth, lHeight, 1, 0);
#ifdef USE_COLOR_GRADIENT
	ImgGradient(pSrcImg, Gx, Gy, lGradLine, pMaskData, lMaskLine, lWidth, lHeight, MNull, &MaxGrad, MNull);
#else
	Gradient(pSrcData, lSrcLine, Gx, Gy, lGradLine, MNull, 0, lWidth, lHeight, MNull, &MaxGrad, &GradPtNum);
#endif
// 	PrintChannel(Gx, lGradLine,DATA_I16,lWidth,lHeight,1,0);
//	PrintChannel(Gy, lGradLine,DATA_I16,lWidth,lHeight,1,0);

	//Display the gradient directions
	if (MaxGrad<=10) goto EXT;
	GO(HistoAnalysis(hMemMgr, Gx, Gy, lGradLine, pMaskData, lMaskLine, lWidth, lHeight, MaxGrad, EDGE_PERCENTAGE, &thres));
	if (thres<=10) goto EXT;
//
	Class.lClassNum = CLASS_NUM;
	AllocVectMem(hMemMgr, Class.pClass, Class.lClassNum, ORIENT_LIST);
	SetVectZero(Class.pClass, Class.lClassNum*sizeof(ORIENT_LIST));

	lPreFixLen = lWidth*lHeight/10;
	for (x=0; x<Class.lClassNum; x++)
	{	
		ORIENT_LIST* pCurOrientList=Class.pClass+x;
		pCurOrientList->lSegIndex = x;
		pCurOrientList->lVectLen = 0;
		AllocVectMem(hMemMgr, pCurOrientList->pPointVect, lPreFixLen, MPOINT);
		SetVectZero(pCurOrientList->pPointVect, lPreFixLen*sizeof(MPOINT));
	}
	
	lGradExt = lGradLine - lWidth;
	pTmpGx = Gx, pTmpGy = Gy;
	for (y=0; y<lHeight; y++, pTmpGx+=lGradExt, pTmpGy+=lGradExt)
	{
		for (x=0; x<lWidth; x++,pTmpGx++, pTmpGy++)
		{
			MLong lIndex2;
			MShort gxVal=*pTmpGx, gyVal=*pTmpGy;
			ORIENT_LIST* pCurOrient;
			if (ABS(gxVal)+ABS(gyVal)<thres) continue;

			//orient classification
			lIndex2 = vComputeAngleDist(gxVal, gyVal, CLASS_NUM);
			pCurOrient = (Class.pClass+lIndex2);
			pCurOrient->pPointVect[pCurOrient->lVectLen].x = x;
			pCurOrient->pPointVect[pCurOrient->lVectLen].y = y;
			pCurOrient->lVectLen++;			
		}
	}

	//four pair
// 	AllocVectMem(hMemMgr, D[0], lRltLine*lHeight, MUInt16);
// 	AllocVectMem(hMemMgr, D[1], lRltLine*lHeight, MUInt16);
// 	AllocVectMem(hMemMgr, D[2], lRltLine*lHeight, MUInt16);
// 	AllocVectMem(hMemMgr, D[3], lRltLine*lHeight, MUInt16);
// 	SetVectZero(D[0], lRltLine*lHeight*sizeof(MUInt16));
// 	SetVectZero(D[1], lRltLine*lHeight*sizeof(MUInt16));
// 	SetVectZero(D[2], lRltLine*lHeight*sizeof(MUInt16));
// 	SetVectZero(D[3], lRltLine*lHeight*sizeof(MUInt16));
	for (i=0; i<Class.lClassNum/2+1; i++)
	{
		//ORIENT_LIST* pCurList, *pPairList;
		ORIENT_LIST* pCurList, *pList1, *pList2, *pList3;
		MLong StartIndex1=0, StartIndex2=0, StartIndex3=0;
		MUInt16* pDes = D[(i/2)%4];
		MUInt16* pSumDes = pBuffer;
		pCurList = Class.pClass+i;
		//pPairList = pCurList+Class.lClassNum/2;
		pList1 = Class.pClass+ (i+Class.lClassNum/2-1)%Class.lClassNum;
		pList2 = Class.pClass+(i+Class.lClassNum/2)%Class.lClassNum;
		pList3 = Class.pClass+(i+Class.lClassNum/2+1)%Class.lClassNum;
		
		for (y=0; y<pCurList->lVectLen; y++)
		{
			MBool bFirst=MTrue;
			MPOINT* pCVect = pCurList->pPointVect + y;
			for (x=StartIndex1; x<pList1->lVectLen; x++)
			{
				MPOINT* pPVect = pList1->pPointVect+x;
				MLong lDist,offset;
				MPOINT midPoint;

				if (pPVect->y-pCVect->y>MAX_DIST_BW_POINTS)
					break;

				if(bFirst&&pCVect->y-pPVect->y<MAX_DIST_BW_POINTS)
					StartIndex1 = MAX(x,0),bFirst = MFalse;

				lDist = vDistance2L(*pCVect, *pPVect);
				if (lDist>MAX_DIST_BW_POINTS)
					continue;
				
				if (lDist<MIN_DIST_BW_POINTS)
					continue;

				//voting for midpoint
				midPoint.x = (pCVect->x + pPVect->x)/2;
				midPoint.y = (pCVect->y + pPVect->y)/2;
				offset = lBufLine*midPoint.y+midPoint.x;//ATTENTION!!!! here lRltLine == lMaskLine
				if (*(pMaskData+offset)==255)
				{
					(*(pSumDes+offset))++;
					//(*(pDes+offset))++;
					if (MaxResponse<*(pSumDes+offset))
						MaxResponse = *(pSumDes+offset);
				}
			}
		}
		if (i==Class.lClassNum/2) continue;
		for (y=0; y<pCurList->lVectLen; y++)
		{
			MBool bFirst=MTrue;
			MPOINT* pCVect = pCurList->pPointVect + y;
			for (x=StartIndex2; x<pList2->lVectLen; x++)
			{
				MPOINT* pPVect = pList2->pPointVect+x;
				MLong lDist, offset;
				MPOINT midPoint;
				if (pPVect->y-pCVect->y>MAX_DIST_BW_POINTS)
					break;
				
				if(bFirst&&pCVect->y-pPVect->y<MAX_DIST_BW_POINTS)
					StartIndex2 = MAX(x,0),bFirst = MFalse;

				lDist = vDistance2L(*pCVect, *pPVect);

				if (lDist>MAX_DIST_BW_POINTS)
					continue;
				
				if (lDist<MIN_DIST_BW_POINTS)
					continue;
				
				//voting for midpoint
				midPoint.x = (pCVect->x + pPVect->x)/2;
				midPoint.y = (pCVect->y + pPVect->y)/2;
				offset = lBufLine*midPoint.y+midPoint.x;//ATTENTION!!!! here lRltLine == lMaskLine
				if (*(pMaskData+offset)==255)
				{
					(*(pSumDes+offset))++;
					//(*(pDes+offset))++;
					if (MaxResponse<*(pSumDes+offset))
						MaxResponse = *(pSumDes+offset);
				}
			}
		}
		if (i==Class.lClassNum/2-1) continue;
		for (y=0; y<pCurList->lVectLen; y++)
		{
			MBool bFirst=MTrue;
			MPOINT* pCVect = pCurList->pPointVect + y;
			for (x=StartIndex3; x<pList3->lVectLen; x++)
			{
				MPOINT* pPVect = pList3->pPointVect+x;
				MLong lDist, offset;
				MPOINT midPoint;
				if (pPVect->y-pCVect->y>MAX_DIST_BW_POINTS)
					break;
				
				if(bFirst&&pCVect->y-pPVect->y<MAX_DIST_BW_POINTS)
					StartIndex3 = MAX(x,0),bFirst = MFalse;
				
				lDist = vDistance2L(*pCVect, *pPVect);
				if (lDist>MAX_DIST_BW_POINTS)
					continue;
				
				if (lDist<MIN_DIST_BW_POINTS)
					continue;
				
				//voting for midpoint
				midPoint.x = (pCVect->x + pPVect->x)/2;
				midPoint.y = (pCVect->y + pPVect->y)/2;
				offset = lBufLine*midPoint.y+midPoint.x;//ATTENTION!!!! here lRltLine == lMaskLine
				if (*(pMaskData+offset)==255)
				{
					(*(pSumDes+offset))++;
					//(*(pDes+offset))++;
					if (MaxResponse<*(pSumDes+offset))
						MaxResponse = *(pSumDes+offset);
				}
			}
		}
	}
// 	PrintBmpEx(D[0], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\0.bmp");
// 	PrintBmpEx(D[1], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\1.bmp");
// 	PrintBmpEx(D[2], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\2.bmp");
// 	PrintBmpEx(D[3], lRltLine, DATA_U16, lWidth, lHeight, 1, "C:\\3.bmp");
	pTmpBuf = pBuffer;
	pTmpD0 = D[0];
	pTmpD1 = D[1];
	pTmpD2 = D[2];
	pTmpD3 = D[3];
	lBufExt = lBufLine - lWidth;
	pTmpRlt = pRltData;
	lRltExt = lRltLine - lWidth;
	if (MaxResponse == 0)goto EXT;
	factor = ((255<<16)+MaxResponse/2)/MaxResponse;

	lBinSize = MAX(1,(MaxResponse+254)/255);
	lTotalLen = 0;

	pTmpGx = Gx, pTmpGy = Gy; 
	for (y = 0; y<lHeight; y++, pTmpBuf+=lBufExt, pTmpRlt+=lRltExt,
		pTmpD0+=lBufExt, pTmpD1+=lBufExt, pTmpD2+=lBufExt, pTmpD3+=lBufExt,
		pTmpGx+=lGradExt, pTmpGy+=lGradExt)
	{
		for (x=0; x<lWidth; x++, pTmpBuf++, pTmpRlt++, pTmpD0++,pTmpD1++,pTmpD2++,pTmpD3++,
			pTmpGx++,pTmpGy++)
		{
			MLong val = *pTmpBuf;
			if (val == 0 || (ABS(*pTmpGx)+ABS(*pTmpGy))>thres)//high thres is deliminished
				//*pTmpRlt = 0;
				*pTmpBuf = 0;
			else
			{
				//Normalize the bin level
// 				MUInt16 lD0 = *pTmpD0;
// 				MUInt16 lD1 = *pTmpD1;
// 				MUInt16 lD2 = *pTmpD2;
// 				MUInt16 lD3 = *pTmpD3;
// 				MLong Sum = lD0+lD1+lD2+lD3;
// 				MLong Sum2 = lD0*lD0+lD1*lD1+lD2*lD2+lD3*lD3;
// 				double Var = (Sum2*1.0/(Sum*Sum)- 1.0/4)/3;
// 				
// 				MLong MaxD = MAX(MAX(MAX(lD0, lD1), lD2), lD3);
// 				if (Var<0.1)
// 					//*pTmpRlt = 0;
// 					*pTmpBuf = 0;
//				else
					//*pTmpRlt = TRIM_UINT8((MaxD*factor)>>16);
				{
					//*pTmpBuf = MaxD;
					hist[val/lBinSize]++;
					lTotalLen ++ ;
				}
				
			}
		}
	}
	
//	lThres = lTotalLen/100;
	lTotalLen = 0;
	for (y=255; y>=0; y--)
	{
		lTotalLen += hist[y];

		if (lTotalLen>=5)break;
	}
	//lThres = y*MAX(1,MaxResponse*/256);
	lThres = y * lBinSize;
	lThres = (lThres+MaxResponse)/2;
	//factor = 256/lThres;
	factor = ((255<<16)+lThres/2)/lThres;
	//
	pTmpBuf = pBuffer;
	pTmpRlt = pRltData;
	for (y=0; y<lHeight;y++,pTmpBuf+=lBufExt,pTmpRlt+=lRltExt)
	{
		for (x=0; x<lWidth; x++,pTmpBuf++,pTmpRlt++)
		{
			if (*pTmpBuf==0)
			{
				*pTmpRlt = 0;
			}
			else
			{
				*pTmpRlt = TRIM_UINT8(((*pTmpBuf)*factor)>>16);
			}
		}
	}
	//
	//CpyImgMem(pRltData, lRltLine, pBuffer, lBufLine, lWidth, lHeight, MShort);
EXT:

	for (x=0; x<Class.lClassNum; x++)
	{	
		ORIENT_LIST* pCurOrientList=Class.pClass+x;
		FreeVectMem(hMemMgr, pCurOrientList->pPointVect);
	}

// 	FreeVectMem(hMemMgr, D[0]);
// 	FreeVectMem(hMemMgr, D[1]);
// 	FreeVectMem(hMemMgr, D[2]);
//	FreeVectMem(hMemMgr, D[3]);
	FreeVectMem(hMemMgr, Class.pClass);
	FreeVectMem(hMemMgr, pBuffer);
	FreeVectMem(hMemMgr, Gx);
	FreeVectMem(hMemMgr, Gy);

	return res;
}
#endif

MRESULT LocalMaxBlob(MHandle hMemMgr, MByte* pChinTransData, MLong lLine, MLong lWidth, MLong lHeight,
				 MByte* pRltData, MLong lRltLine, JGSEED* pSeeds)
{
	MRESULT res=LI_ERR_NONE;
	MLong lExt=lLine-lWidth;
	MByte* pBuffer=MNull;
	MLong lBufLine = JMemLength(lWidth);
	MLong lBufExt = lBufLine-lWidth;
	MLong MaxVal = 0, MinVal=0;
	MVoid* pMemTmp=MNull;
	MLong lMemLen, lTmpSeedLen;
	MLong lHighThres, lLowThres;

	lMemLen = lWidth*lHeight/8;
	AllocVectMem(hMemMgr, pMemTmp, lMemLen, MPOINT);

	if (pChinTransData!=pRltData)
	{
		pBuffer = pRltData;
		lBufLine = lRltLine;
	}
	else
	{
		JASSERT(lRltLine==lLine);
		AllocVectMem(hMemMgr, pBuffer, lBufLine*lHeight, MByte);
		SetVectZero(pBuffer, lBufLine*lHeight*sizeof(MByte));
	}
	
	lHighThres = THRESH_VOTING;
	lLowThres = lHighThres/2;
//	PrintBmpEx(pChinTransData, lLine, DATA_U8, lWidth, lHeight, 1, "C:\\pre.bmp");
	SoftThres(hMemMgr, pChinTransData, lLine, pBuffer, lBufLine, 
		lWidth, lHeight, pMemTmp, lMemLen, lHighThres, lLowThres,
		50, 255, pSeeds);

	//PrintBmpEx(pBuffer, lBufLine, DATA_U8, lWidth, lHeight, 1, "C:\\blob.bmp");
	lTmpSeedLen = 0;
// 	while (lTmpSeedLen<seeds.lSeedNum)
// 	{
// 		MPOINT ptCur = seeds.pptSeed[lTmpSeedLen];
// 		MByte* pTmpData = pBuffer+lBufLine*ptCur.y+ptCur.x;
// 		MLong lBlobLen;
// 		if (*pTmpData==0)
// 		{
// 			ExtractBlob_8Con(pTmpData, lLine, lWidth, lHeight, pMemTmp, lMemLen,
// 				&lBlobLen, 255, 0, &ptCur);
// 			*pTmpData = 255;
// 		}
// 		lTmpSeedLen++;
//	}
	
	EXT:
	if (pRltData==pChinTransData)
	{
		CpyImgMem3(pRltData, lRltLine, pBuffer, lBufLine, lWidth, lHeight, DATA_U8);
		FreeVectMem(hMemMgr, pBuffer);
	}
	FreeVectMem(hMemMgr, pMemTmp);
	return res;
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
				  && lStackLen<MAX_SEED_NUM)
				{
					pSeeds->pptSeed[lStackLen].x = x; 
					pSeeds->pptSeed[lStackLen].y = y;
					pSeeds->pcrSeed[lStackLen] = Cur;
					lStackLen++;
				}
			}
			else if (Cur<lLowThres)
			{	
				//*pTmpSrc = 128;
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
		lUltLowThres = MAX(lUltLowThres, lLowThres);
		if (lUltLowThres <=lLowThres) 
		{
			//*(pRlt+lRltLine*pSeeds->pptSeed[lStackLen].y+pSeeds->pptSeed[lStackLen].x) = 0;
			//continue;
			lUltLowThres = lLowThres;
		}
		
		do 
		{
 			MPOINT ptCur = pPtsTmp[--lPtsNum];
			MLong maskX = ptCur.x, maskY = ptCur.y;
			MByte *pMaskTmp = (MByte*)pRlt+lRltLine*maskY+maskX;
			MByte *pMaskSrc = (MByte*)pSrc+lSrcLine*maskY+maskX;
			
			if(maskX<0 || maskX>lWidth-1) continue;
			if(maskY<0 || maskY>lHeight-1) continue;
			if(lPtsNum>=lMemLen-4) continue;
			
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
#define TAN11_25	51
#define TAN22_5		106  //0.414*255
#define TAN33_75    170
#define TAN45       255
#define TAN56_25	382
#define TAN67_5		616  //2.414*255
#define TAN78_75	1282
//CLASS_NUM 8
#ifndef __TEST_CHINTRANS__

MLong vComputeAngleDist(MShort Gx, MShort Gy, MLong classNum)
{
// 	MDouble alpha = atan2((MDouble)(Gy), (MDouble)(Gx));
// 	MLong index;
// 	alpha = alpha*180/VPI;
// 	if (alpha<0) alpha+=360;
// 	index = (alpha+22.5)/45;
// 	index %= 8;
//	return index;

	
	MLong res;
	MLong TANVal;
	JASSERT(!(Gx==0 && Gy==0));
	if (Gx==0) 
	{
		if (Gy>0) res=2;//third distance
		else
			res= 6;//Seventh distance
		return res;
	}
	
	TANVal = Gy*256/Gx;

	if (TANVal<TAN22_5&&TANVal>-TAN22_5) res=0;
	else if (TANVal>=TAN22_5&&TANVal<=TAN67_5) res=1;
	else if (TANVal<=-TAN22_5&&TANVal>=-TAN67_5) res=-1;
	else if (TANVal>TAN67_5)res=2;
	else if (TANVal<-TAN67_5)res=-2;

	if (Gx<0)
	{
		res += 4;
	}

	if (Gx>0&&res<0) res+=8;

	return res;

}
#endif

#ifdef __TEST_CHINTRANS__

//range from 0 to pi/2
JSTATIC MLong ATAN4Bins(MLong x)
{
	if (x<TAN22_5) return 0;
	if (x<TAN45) return 1;
	if (x<TAN67_5) return 2;
	return 3;
}

JSTATIC MLong ATAN8Bins(MLong x)
{
	if (x<TAN11_25)return 0;
	if (x<TAN22_5) return 1;
	if (x<TAN33_75) return 2;
	if (x<TAN45)return 3;
	if (x<TAN56_25)return 4;
	if (x<TAN67_5) return 5;
	if (x<TAN78_75)return 6;
	return 7;
}

MLong vComputeAngleDist(MShort Gx, MShort Gy, MLong classNum)
{
	MLong divVal,index,quad;

	switch(classNum)
	{
	case 16:
		{
			if (Gx == 0) 
			{
				if (Gy >=0) return 3;
				else return 3+8;
			}
			
			divVal = Gy*256/Gx;
			index = ATAN4Bins(ABS(divVal));
			quad = ((Gx<0)?1:0) + ((Gy<0)?1:0) * 2;
			switch (quad)
			{
			case 0://the first quadratic
				return index;
			case 1://the second quadratic
				return 7-index;
			case 3://the third quadratic
				return 8+index;
			case 2://the forth quadratic
				return 15-index;
			default:
				return -1;
			}
			break;
		}
	case 32:
		{
			if (Gx == 0) 
			{
				if (Gy >=0) return 7;
				else return 7+16;
			}
			
			divVal = Gy*256/Gx;
			index = ATAN8Bins(ABS(divVal));
			quad = ((Gx<0)?1:0) + ((Gy<0)?1:0) * 2;
			switch (quad)
			{
			case 0://the first quadratic
				return index;
			case 1://the second quadratic
				return 15-index;
			case 3://the third quadratic
				return 16+index;
			case 2://the forth quadratic
				return 31-index;
			default:
				return -1;
			}
			break;
		}
	default:
		return -1;
	}	
// 
// 	MDouble alpha = atan2((MDouble)(Gy), (MDouble)(Gx));
// 	MLong index;
// 	MDouble binsize = 2*VPI/classNum;
// 	//alpha = alpha*180/VPI;
// 	//if (alpha<0) alpha+=360;
// 	alpha += VPI*2;
// 	//index = (alpha+22.5)/45;
// 	///index = alpha/classNum;
// 	//index %= classNum;
// 	index =(MLong)(alpha / binsize);
//	return index%classNum;	
}
#endif

#define MAX_STACK_LEN  65536

//pBlobMask: Foreground -- 255;
//		   : Background -- 0
/*
MRESULT CreateBlobGraphByImg(MHandle hMemMgr, JOFFSCREEN* pImg, 
							 MByte* pBlobMask, MLong lBlobLine,
							 MLong lWidth, MLong lHeight,
							 JINFO_BLOB_GRAPH **ppInfoGraph, JGSEED *pSeed)
{
	MRESULT res=LI_ERR_NONE;
	JINFO_BLOB_GRAPH *pGraph=MNull;
	MLong lTmpLen;
	MVoid* pMemTmp = MNull;
	MLong lMemLen= MAX_STACK_LEN;

	MLong lMaxFlag = MIN(lWidth*lHeight, 1024*16);
	lMaxFlag = MIN(lMaxFlag, 0x7FFF/3);

	if (ppInfoGraph==MNull) return res;
	if (*ppInfoGraph!=MNull) ReleaseBlobGraph(hMemMgr, ppInfoGraph);

	JASSERT((MLong)(pImg->dwWidth) == lWidth && (MLong)(pImg->dwHeight) == lHeight);

	AllocVectMem(hMemMgr, pGraph, 1, JINFO_BLOB_GRAPH);
	SetVectZero(pGraph, sizeof(JINFO_BLOB_GRAPH));

	AllocVectMem(hMemMgr, pGraph->pInfoBlob, lMaxFlag, JINFO_BLOB);
	SetVectMem(pGraph->pInfoBlob, lMaxFlag, 0, JINFO_BLOB);
	pGraph->lLabelNum = 0;

	AllocVectMem(hMemMgr, pMemTmp, lMemLen, MByte);
	
	lTmpLen = pSeed->lSeedNum;
	while (lTmpLen-->0)
	{
		MLong lBlobLen,lGradSum;
		MPOINT * pTmpPT;
		MPOINT Pt = pSeed->pptSeed[lTmpLen];
		MByte *pTmpMask = pBlobMask+lBlobLine*Pt.y+Pt.x;
		MDWord SumY=0, SumCb=0, SumCr=0;
		MDWord xSum=0, ySum=0, x2Sum=0, y2Sum=0,xySum=0;
		MLong lSize=0;

		MATRIX_2D covMatrix={0};
		JINFO_BLOB *pInfoBlob;

//		MLong i;
//		MLong tx, ty;
		MPOINT Endpoints[4]={0};
		
		if (*pTmpMask != 255)
			continue;
		
		ExtractBlob_8Con(pTmpMask, lBlobLine, lWidth, lHeight, pMemTmp, lMemLen,
			&lBlobLen, 255, 128, &Pt);
		pTmpPT = (MPOINT*)pMemTmp;
		lSize = lBlobLen;
		
		lGradSum = 0;
		Endpoints[0].x=lWidth;
		Endpoints[2].x=0;
		Endpoints[1].y=lHeight;
		Endpoints[3].y=0;
		while (lBlobLen-->0)
		{
			MPOINT PtCur=pTmpPT[lBlobLen];
			MCOLORREF val, crTmp ;
			PtCur.x += Pt.x; PtCur.y += Pt.y;
			val = ImgGetPixel(pImg, PtCur.x, PtCur.y);
			SumY += PIXELVAL_1(val);
			SumCb += PIXELVAL_2(val);
			SumCr += PIXELVAL_3(val);
			xSum += PtCur.x;
			ySum += PtCur.y;
			x2Sum += PtCur.x*PtCur.x;
			y2Sum += PtCur.y*PtCur.y;
			xySum += PtCur.x*PtCur.y;
			
			if (Endpoints[0].x>PtCur.x)
				Endpoints[0].x=PtCur.x,Endpoints[0].y=PtCur.y;
			if (Endpoints[1].y>PtCur.y)
				Endpoints[1].x=PtCur.x,Endpoints[1].y=PtCur.y;
			if (Endpoints[2].x<PtCur.x)
				Endpoints[2].x=PtCur.x,Endpoints[2].y=PtCur.y;
			if (Endpoints[3].y<PtCur.y)
				Endpoints[3].x=PtCur.x,Endpoints[3].y=PtCur.y;
			
			
// 			for (i=0; i<4; i++)
// 			{
// 				tx = PtCur.x; ty = PtCur.y;
// 				if (i==0)
// 				{
// 					tx--; goto STACK;
// 				}
// 				if (i==1)
// 				{
// 					tx++; goto STACK;
// 				}
// 				if (i==2)
// 				{
// 					ty--; goto STACK;
// 				}
// 				if (i==3)
// 				{
// 					ty++; goto STACK;
// 				}
// STACK:			
// 				crTmp = ImgGetPixel(pImg, tx, ty);
// 				lGradSum += ABS(PIXELVAL_1(crTmp)-PIXELVAL_1(val)) 
// 					+ ABS(PIXELVAL_2(crTmp)-PIXELVAL_2(val))*2 
// 					+ ABS(PIXELVAL_3(crTmp)-PIXELVAL_3(val))*2 ;
//			}
			//*(pBlobMask+PtCur.y*lBlobLine+PtCur.x) = 128;
		}

		covMatrix.data.fd[0][0] = (x2Sum-(xSum*1.0*xSum*1.0)/lSize)/lSize;
		covMatrix.data.fd[0][1] = (xySum-(xSum*1.0*ySum*1.0)/lSize)/lSize;
		covMatrix.data.fd[1][0] = covMatrix.data.fd[0][1];
		covMatrix.data.fd[1][1] = (y2Sum-(ySum*1.0*ySum*1.0)/lSize)/lSize;
		
		pInfoBlob = pGraph->pInfoBlob+pGraph->lLabelNum;
		pInfoBlob->lGradientSum = lGradSum;
		vComputeEigen(&covMatrix, &pInfoBlob->eigen);
		pInfoBlob->lY = SumY;
		pInfoBlob->lCb = SumCb;
		pInfoBlob->lCr = SumCr;
		pInfoBlob->lSize = lSize;

		pInfoBlob->CenPt.x = xSum/lSize;
		pInfoBlob->CenPt.y = ySum/lSize;
		pInfoBlob->lFlag = pGraph->lLabelNum+1;
		pInfoBlob->ptSeed = Pt;
		pInfoBlob->Endpoint[0] = Endpoints[0];
		pInfoBlob->Endpoint[1] = Endpoints[1];
		pInfoBlob->Endpoint[2] = Endpoints[2];
		pInfoBlob->Endpoint[3] = Endpoints[3];
		pGraph->lLabelNum++;
	}
//#define TEST_DEBUG

#ifdef TEST_DEBUG
	{
		MLong i, j;
		MByte *pTmpMask = MNull; //= pBlobMask+lBlobLine*Pt.y+Pt.x;
		for (i=0; i<pGraph->lLabelNum; i++)
		{
			JINFO_BLOB *pInfoBlob = pGraph->pInfoBlob+i;
			MDouble slope;
			if (pInfoBlob->lFlag<=0) continue;

			pTmpMask = pBlobMask+lBlobLine*pInfoBlob->ptSeed.y+pInfoBlob->ptSeed.x;
			if (*pTmpMask!=128) continue;
			if (pInfoBlob->eigen.EigenVec[0][0]<0.0001&&pInfoBlob->eigen.EigenVec[0][0]>-0.0001)
				slope = pInfoBlob->eigen.EigenVec[0][1]/(pInfoBlob->eigen.EigenVec[0][0]+0.0001);
			else
				slope = pInfoBlob->eigen.EigenVec[0][1]/(pInfoBlob->eigen.EigenVec[0][0]);
			vDrawLine(pBlobMask, lBlobLine, lWidth, lHeight, 255, pInfoBlob->ptSeed, slope, 10);
		}
	}
	PrintBmp(pBlobMask, lBlobLine, DATA_U8, lWidth, lHeight, 1);
#endif

EXT:
	if (res==LI_ERR_NONE)
		*ppInfoGraph = pGraph;
	else
		ReleaseBlobGraph(hMemMgr, &pGraph);
	FreeVectMem(hMemMgr, pMemTmp);
	return res;
}

MVoid ReleaseBlobGraph(MHandle hMemMgr, JINFO_BLOB_GRAPH **ppInfoGraph)
{
	JINFO_BLOB_GRAPH *pGraph;
	if (ppInfoGraph==MNull || *ppInfoGraph == MNull) return;

	pGraph = *ppInfoGraph;
	FreeVectMem(hMemMgr, pGraph->pInfoBlob);
	FreeVectMem(hMemMgr, pGraph);
	*ppInfoGraph = MNull;
}

MVoid UpdateBlobGraph(JINFO_BLOB_GRAPH *pInfoGraph)
{
	MLong lLabelCur = 0;
	for(lLabelCur=0; lLabelCur<pInfoGraph->lLabelNum; lLabelCur++)
	{
		MLong lFlag = pInfoGraph->pInfoBlob[lLabelCur].lFlag;
		if(lFlag >= 0)
		{
			JASSERT(lFlag==0 || lLabelCur+1 == lFlag);
			continue;
		}		
		while (pInfoGraph->pInfoBlob[-lFlag-1].lFlag < 0)
			lFlag = pInfoGraph->pInfoBlob[-lFlag-1].lFlag;
		JASSERT(lFlag<0);
		JASSERT(pInfoGraph->pInfoBlob[-lFlag-1].lFlag==0 || pInfoGraph->pInfoBlob[-lFlag-1].lFlag==-lFlag);
		pInfoGraph->pInfoBlob[lLabelCur].lFlag = lFlag;
	}
}

MVoid BlobMerge(JOFFSCREEN *pImgSrc, JINFO_BLOB_GRAPH* pInfoGraph, MLong lWidth, MLong lHeight)
{
	MBool bMerged = MFalse;
	
	do 
	{
		MLong lLabelCur;
		MLong lMaxDist = (lWidth>lHeight?lWidth:lHeight)/8;
		bMerged = MFalse;
		for(lLabelCur=pInfoGraph->lLabelNum-1; lLabelCur>=0; lLabelCur--)
		{
			MLong lMinDistX=0;
			MDouble fMinDist = ANGLE_DIST;//Cos(15)
			
			JINFO_BLOB* pInfoBlob = pInfoGraph->pInfoBlob+lLabelCur;
			if (pInfoBlob->lFlag<=0) 
				continue;
			
			lMinDistX = NeighborLieOnSameDirect(pImgSrc, pInfoGraph, lLabelCur,  &fMinDist,lMaxDist);
			if(lMinDistX<0)
				continue;
			MergeBlob(pInfoGraph->pInfoBlob,  MAX(lLabelCur, lMinDistX), 
				MIN(lLabelCur, lMinDistX), pInfoGraph->lLabelNum);
			bMerged = MTrue;
		}
	} while(bMerged);
	UpdateBlobGraph(pInfoGraph);
}


MLong NeighborLieOnSameDirect(JOFFSCREEN *pImgSrc, JINFO_BLOB_GRAPH* pInfoGraph, 
							  MLong lLabelSeed, MDouble* pfMinDirDiff, MLong maxLDist)
{
	MLong x, lColorDist;
	MDouble fAngleDist, fDiffAngle;
	MLong lMinDistX=-1;
	JINFO_BLOB *pInfoBlob = pInfoGraph->pInfoBlob + lLabelSeed;
	MCOLORREF crRef = PIXELVAL(pInfoBlob->lY/pInfoBlob->lSize, pInfoBlob->lCb/pInfoBlob->lSize, pInfoBlob->lCr/pInfoBlob->lSize);
	MDouble refEigen[2], refEigenVal;
	MPOINT refCenPt = pInfoBlob->CenPt;

	//the larger is reference
	refEigen[0] = pInfoBlob->eigen.EigenVec[0][0];
	refEigen[1] = pInfoBlob->eigen.EigenVec[0][1];
	refEigenVal = pInfoBlob->eigen.EigenVal[0];

	//if (refEigenVal < 2) return lMinDistX;

	//	JASSERT(pInfoBlob->lFlag > 0);
	for(x=0; x<lLabelSeed; x++)
	{
		JINFO_BLOB *pBlobCur = pInfoGraph->pInfoBlob  + x;
		MDouble posVect[2];
		MCOLORREF valCur;
		if(pBlobCur->lFlag <= 0)
			continue;

		//if (vDistance2L(pInfoBlob->CenPt, pBlobCur->CenPt)>maxLDist)
		if (BlobDist(pInfoBlob, pBlobCur)>maxLDist)
			continue;

		if (pBlobCur->eigen.EigenVal[0]<2 ) continue;

		//Color Distance
		valCur = PIXELVAL(pBlobCur->lY/pBlobCur->lSize,pBlobCur->lCb/pBlobCur->lSize,pBlobCur->lCr/pBlobCur->lSize);
		lColorDist = GetColorDist(PIXELVAL_1(valCur), PIXELVAL_2(valCur), PIXELVAL_3(valCur) ,crRef);
		if (lColorDist>=COLOR_DIST_BLOB) 
			continue;

		//Blob Principle Component Angle
		fAngleDist =GetAngleDist(pBlobCur->eigen.EigenVec[0], refEigen, 2);
		if (fAngleDist<ANGLE_DIST)
			continue;

		posVect[0] = refCenPt.x - pBlobCur->CenPt.x;
		posVect[1] = refCenPt.y - pBlobCur->CenPt.y;
		fDiffAngle = GetAngleDist(posVect,refEigen,2);
		if (fDiffAngle <ANGLE_DIST) continue;


		valCur = PIXELVAL((PIXELVAL_1(valCur)+PIXELVAL_1(crRef))/2,
			              (PIXELVAL_2(valCur)+PIXELVAL_2(crRef))/2,
						  (PIXELVAL_3(valCur)+PIXELVAL_3(crRef))/2);
		if (!bIsInValidBlob(pImgSrc, &(pInfoBlob->ptSeed), &(pBlobCur->ptSeed), valCur))
			continue;

		//Center Distance
//		fDiffAngle = vDistance1L(pBlobCur->CenPt, pInfoBlob->CenPt);
		if(fDiffAngle<= *pfMinDirDiff)
			continue;	
		*pfMinDirDiff = fDiffAngle;
		lMinDistX = x;
	}
	for(x=lLabelSeed+1; x<pInfoGraph->lLabelNum; x++)
	{
		JINFO_BLOB *pBlobCur = pInfoGraph->pInfoBlob  + x;
		MDouble posVect[2];
		MCOLORREF valCur;
		if(pBlobCur->lFlag <= 0)
			continue;
		//if (vDistance2L(pInfoBlob->CenPt, pBlobCur->CenPt)>maxLDist)
		if (BlobDist(pInfoBlob, pBlobCur)>maxLDist)
			continue;
		if (pBlobCur->eigen.EigenVal[0]<2 ) continue;
		lColorDist = GetColorDist(pBlobCur->lY/pBlobCur->lSize, 
			pBlobCur->lCb/pBlobCur->lSize, pBlobCur->lCr/pBlobCur->lSize, crRef);
		if (lColorDist>=COLOR_DIST_BLOB) 
			continue;
		fAngleDist =GetAngleDist(pBlobCur->eigen.EigenVec[0], refEigen, 2);
		if (fAngleDist<ANGLE_DIST)
			continue;
//		lCenDist = vDistance1L(pBlobCur->CenPt, pInfoBlob->CenPt);
		posVect[0] = refCenPt.x - pBlobCur->CenPt.x;
		posVect[1] = refCenPt.y - pBlobCur->CenPt.y;
		fDiffAngle = GetAngleDist(posVect,refEigen,2);
		if (fDiffAngle <ANGLE_DIST) continue;

		valCur = PIXELVAL((PIXELVAL_1(valCur)+PIXELVAL_1(crRef))/2,
			(PIXELVAL_2(valCur)+PIXELVAL_2(crRef))/2,
			(PIXELVAL_3(valCur)+PIXELVAL_3(crRef))/2);
		if (!bIsInValidBlob(pImgSrc, &(pInfoBlob->ptSeed), &(pBlobCur->ptSeed), valCur))
			continue;

		if(fDiffAngle <= *pfMinDirDiff)
			continue;
		*pfMinDirDiff = fDiffAngle;
		lMinDistX = x;
	}		
	
	return lMinDistX;
}

//COS(15)  0.9659
//COS(10)  0.9848
//COS(5)   0.9962
MDouble GetAngleDist(MDouble *eigenCur, MDouble *eigenRef, MLong lVectLen)
{
	MLong i;

	MDouble CosAngle;
	//Normalize
	MDouble sum2Cur = 0.0, sum2Ref=0.0;
	for (i=0; i<lVectLen; i++)
	{
		sum2Cur += SQARE(eigenCur[i]);
		sum2Ref += SQARE(eigenRef[i]);
	}
	sum2Cur = sqrt(sum2Cur);
	sum2Ref = sqrt(sum2Ref);

	if (sum2Cur<=0.001 || sum2Ref<=0.001)
		return 0.0;
	
	for (i=0; i<lVectLen; i++)
	{
		eigenRef[i] /= sum2Ref;
		eigenCur[i] /= sum2Cur;
	}
	
	//dot product
	CosAngle = 0.0;
	for (i=0; i<lVectLen; i++)
	{
		CosAngle += eigenRef[i]*eigenCur[i];
	}
	return fabs(CosAngle);
}

MLong	GetColorDist(MLong cr1, MLong cr2, MLong cr3, MCOLORREF crCenter)
{	
	MLong tmp = 0, dist = 0;
	tmp = cr1 - PIXELVAL_1(crCenter);	
	dist = SQARE(tmp)>>3;
	tmp = cr2 - PIXELVAL_2(crCenter);
	dist += SQARE(tmp)<<2;
	tmp = cr3 - PIXELVAL_3(crCenter);
	dist += SQARE(tmp)<<2;	
	if(cr1 > PIXELVAL_1(crCenter))
		cr1 = PIXELVAL_1(crCenter); 
	return (dist*128) / (cr1+16);
	//	return (dist*128*128) / (cr1*PIXELVAL_1(crCenter)+16);
}

MVoid MergeBlob(JINFO_BLOB* pInfoBlob, 
				MLong lBlobNoFrom, MLong lBlobNoTo, MLong lLabelNum)
{
	JASSERT(lBlobNoFrom > lBlobNoTo);
	pInfoBlob[lBlobNoTo].lSize += pInfoBlob[lBlobNoFrom].lSize;
	pInfoBlob[lBlobNoTo].lY  += pInfoBlob[lBlobNoFrom].lY;
	pInfoBlob[lBlobNoTo].lCb += pInfoBlob[lBlobNoFrom].lCb;
	pInfoBlob[lBlobNoTo].lCr += pInfoBlob[lBlobNoFrom].lCr;

	pInfoBlob[lBlobNoTo].CenPt.x = (pInfoBlob[lBlobNoTo].CenPt.x +pInfoBlob[lBlobNoFrom].CenPt.x)/2;
	pInfoBlob[lBlobNoTo].CenPt.y = (pInfoBlob[lBlobNoTo].CenPt.y +pInfoBlob[lBlobNoFrom].CenPt.y)/2;

	pInfoBlob[lBlobNoTo].eigen.EigenVec[0][0] = 
		(pInfoBlob[lBlobNoTo].eigen.EigenVec[0][0]+pInfoBlob[lBlobNoFrom].eigen.EigenVec[0][0])/2;
	pInfoBlob[lBlobNoTo].eigen.EigenVec[0][1] = 
		(pInfoBlob[lBlobNoTo].eigen.EigenVec[0][1]+pInfoBlob[lBlobNoFrom].eigen.EigenVec[0][1])/2;
	pInfoBlob[lBlobNoTo].eigen.EigenVec[1][0] = 
		(pInfoBlob[lBlobNoTo].eigen.EigenVec[1][0]+pInfoBlob[lBlobNoFrom].eigen.EigenVec[1][0])/2;
	pInfoBlob[lBlobNoTo].eigen.EigenVec[1][1] = 
		(pInfoBlob[lBlobNoTo].eigen.EigenVec[1][1]+pInfoBlob[lBlobNoFrom].eigen.EigenVec[1][1])/2;

	pInfoBlob[lBlobNoTo].Endpoint[0] = (pInfoBlob[lBlobNoTo].Endpoint[0].x<pInfoBlob[lBlobNoFrom].Endpoint[0].x)?
		pInfoBlob[lBlobNoTo].Endpoint[0]:pInfoBlob[lBlobNoFrom].Endpoint[0];
	pInfoBlob[lBlobNoTo].Endpoint[1] = (pInfoBlob[lBlobNoTo].Endpoint[1].y<pInfoBlob[lBlobNoFrom].Endpoint[1].y)?
		pInfoBlob[lBlobNoTo].Endpoint[1]:pInfoBlob[lBlobNoFrom].Endpoint[1];
	pInfoBlob[lBlobNoTo].Endpoint[2] = (pInfoBlob[lBlobNoTo].Endpoint[2].x>pInfoBlob[lBlobNoFrom].Endpoint[2].x)?
		pInfoBlob[lBlobNoTo].Endpoint[2]:pInfoBlob[lBlobNoFrom].Endpoint[2];
	pInfoBlob[lBlobNoTo].Endpoint[3] = (pInfoBlob[lBlobNoTo].Endpoint[3].y>pInfoBlob[lBlobNoFrom].Endpoint[3].y)?
		pInfoBlob[lBlobNoTo].Endpoint[3]:pInfoBlob[lBlobNoFrom].Endpoint[3];
	
	pInfoBlob[lBlobNoFrom].lFlag = -(lBlobNoTo+1);	
}

MVoid BlobDelete(JINFO_BLOB_GRAPH *pInfoGraph, MLong lMinBlobSize)
{
	MLong i;
	JINFO_BLOB* pInfoBlob = pInfoGraph->pInfoBlob;
	for (i=0; i<pInfoGraph->lLabelNum; i++,pInfoBlob++)
	{
		if (pInfoBlob->lFlag<=0)
			continue;
		if (pInfoBlob->eigen.EigenVal[0]/pInfoBlob->eigen.EigenVal[1] < 4||pInfoBlob->lSize<=lMinBlobSize)
			pInfoBlob->lFlag=0;
	}
}

MLong BlobDist(JINFO_BLOB* pInfoBlob, JINFO_BLOB* pInfoRef)
{
//	MLong i, j;
	MPOINT PtBlobEnd[2];
	MPOINT PtBlobRef[2];
	
	MLong flag[2];

	if (pInfoBlob->eigen.EigenVec[0][0]==0||pInfoBlob->eigen.EigenVec[0][1]/pInfoBlob->eigen.EigenVec[0][0]>1
		||pInfoBlob->eigen.EigenVec[0][1]/pInfoBlob->eigen.EigenVec[0][0]<-1)
	{
		PtBlobEnd[0] = pInfoBlob->Endpoint[1], PtBlobEnd[1] = pInfoBlob->Endpoint[3];
		flag[0] = 1;
	}	
	else
	{
		flag[0] = -1;
		PtBlobEnd[0] = pInfoBlob->Endpoint[0], PtBlobEnd[1] = pInfoBlob->Endpoint[2];
	}

	if (pInfoRef->eigen.EigenVec[0][0]==0||pInfoRef->eigen.EigenVec[0][1]/pInfoRef->eigen.EigenVec[0][0]>1
		||pInfoRef->eigen.EigenVec[0][1]/pInfoRef->eigen.EigenVec[0][0]<-1)
	{
		flag[1] = 1;
		PtBlobRef[0] = pInfoRef->Endpoint[1], PtBlobRef[1] = pInfoRef->Endpoint[3];
	}	
	else
	{
		flag[1] = -1;
		PtBlobRef[0] = pInfoRef->Endpoint[0], PtBlobRef[1] = pInfoRef->Endpoint[2];
	}

	if (flag[0] != flag[1]) return 0x7fffffff;

	return MIN(vDistance1L(PtBlobEnd[0], PtBlobRef[1]), vDistance1L(PtBlobEnd[1], PtBlobRef[0]));
}

MBool bIsInValidBlob(JOFFSCREEN *pImgSrc, MPOINT* pPtStart, MPOINT* pPtEnd, MCOLORREF ref)
{
	MDouble k, b;
	MLong lDist, ltmpdist, lInSkin;
	MPOINT* pPtLT, *pPtRB;
	MPOINT ptCur;

	if (pPtStart->x<0||pPtStart->x>pImgSrc->dwWidth-1
		||pPtStart->y<0||pPtStart->y>pImgSrc->dwHeight-1
		||pPtEnd->x<0||pPtEnd->x>pImgSrc->dwWidth-1
		||pPtEnd->y<0||pPtEnd->y>pImgSrc->dwHeight-1)
		return MFalse;

	lDist = vDistance2L(*pPtStart,*pPtEnd);
	if (pPtStart->x==pPtEnd->x)
	{
		pPtRB = (pPtEnd->y>pPtStart->y)?pPtEnd:pPtStart;
		pPtLT = (pPtEnd->y>pPtStart->y)?pPtStart:pPtEnd;
		ltmpdist = lDist;
		lInSkin = 0;
		ptCur = *(pPtLT);
		while (ltmpdist-->0)
		{
			MCOLORREF val;
			ptCur.y++;
			//if (*(pSkinData+lSkinLine*ptCur.y+ptCur.x)==255)
			val = ImgGetPixel(pImgSrc, ptCur.x, ptCur.y);

			if (GetColorDist(PIXELVAL_1(val), PIXELVAL_2(val), PIXELVAL_3(val), ref)<COLOR_DIST_BLOB)
				lInSkin++;
		}
		if (lInSkin*1.0/lDist>=0.9) return MTrue;
		else
			return MFalse;
	}
	else if (pPtStart->y==pPtEnd->y)
	{
		pPtRB = (pPtEnd->x>pPtStart->x)?pPtEnd:pPtStart;
		pPtLT = (pPtEnd->x>pPtStart->x)?pPtStart:pPtEnd;
		ltmpdist =lDist;
		lInSkin = 0;
		ptCur = *(pPtLT);
		while (ltmpdist-->0)
		{
			MCOLORREF val;
			ptCur.x++;
			val = ImgGetPixel(pImgSrc, ptCur.x, ptCur.y);
			
			if (GetColorDist(PIXELVAL_1(val), PIXELVAL_2(val), PIXELVAL_3(val), ref)<COLOR_DIST_BLOB)
				lInSkin++;
		}
		if (lInSkin*1.0/lDist>=0.9) return MTrue;
		return MFalse;
	}
	else if ((k = (pPtEnd->y-pPtStart->y)*1.0/(pPtEnd->x-pPtStart->x))>=-1&&k<=1)
	{
		pPtRB = (pPtEnd->x>pPtStart->x)?pPtEnd:pPtStart;
		pPtLT = (pPtEnd->x>pPtStart->x)?pPtStart:pPtEnd;
		ltmpdist = lDist;
		lInSkin = 0;
		ptCur = *(pPtLT);
		b = ptCur.y - ptCur.x*k;
		while (ltmpdist-->0)
		{
			MCOLORREF val;
			ptCur.x++;
			ptCur.y = (MLong)(ptCur.x*k+b);
			val = ImgGetPixel(pImgSrc, ptCur.x, ptCur.y);
			
			if (GetColorDist(PIXELVAL_1(val), PIXELVAL_2(val), PIXELVAL_3(val), ref)<COLOR_DIST_BLOB)
				lInSkin++;
		}
		if (lInSkin*1.0/lDist>=0.9) return MTrue;
		return MFalse;
	}
	else
	{
		pPtRB = (pPtEnd->y>pPtStart->y)?pPtEnd:pPtStart;
		pPtLT = (pPtEnd->y>pPtStart->y)?pPtStart:pPtEnd;
		ltmpdist = lDist;
		lInSkin = 0;
		ptCur = *(pPtLT);
		b = ptCur.y - ptCur.x*k;
		while (ltmpdist-->0)
		{
			MCOLORREF val;
			ptCur.y++;
			ptCur.x = (MLong)((ptCur.y - b)/k);
			val = ImgGetPixel(pImgSrc, ptCur.x, ptCur.y);
			
			if (GetColorDist(PIXELVAL_1(val), PIXELVAL_2(val), PIXELVAL_3(val), ref)<COLOR_DIST_BLOB)
				lInSkin++;
		}
		if (lInSkin*1.0/lDist>=0.9) return MTrue;
		return MFalse;
	}
}*/
