#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "facemask.h"
#include "liintegral.h"

#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "lisort.h"
#include "limath.h"
#include "liimage.h"
#include "litimer.h"
#include "litrimfun.h"

#ifdef TRIM_RGB
#include "lirgb_yuv.h"
#endif

//////////////////////////////////////////////////////////////////////////
#define MAX_SEED_LEN		16
#define MAX_SEED_NUM		(SQARE(MAX_SEED_LEN))

#define MAX_COLOR_DIST		512
#define MASK_EXT			4

#define MAX_DIST			220
#define MIN_DIST			64

#define DIST_THRESHOLD1		120
#define DIST_THRESHOLD2		220

#define HIGH_LIGHT			240

#define FLAG_CLEAR(x)			((x) & 0xFE)
#define FLAG_CLEAR_32(x)		((x) & 0xFEFEFEFE)
#define FLAG_SET(x)				((x) | 0x1)
#define FLAG_ISSET(x)			(((x) & 0x1) == 1)

#ifdef ENABLE_DEBUG
MVoid _DrawSeeds(PJOFFSCREEN pImg, PJSEEDS pSeeds);
MVoid _DrawYUVImg(PJOFFSCREEN pImg);
MVoid _DrawRectToMask(JMASK* pMask, PMRECT pRect);
#else
#define _DrawSeeds			(MVoid)
#define _DrawYUVImg			(MVoid)
#define _DrawRectToMask		(MVoid)
#endif	//ENABLE_DEBUG

//////////////////////////////////////////////////////////////////////////
JSTATIC JINLINE MLong	_GetDist(MLong cr11, MLong cr12, MLong cr13, MCOLORREF cr2);

//////////////////////////////////////////////////////////////////////////
JSTATIC MRESULT _SkinMask(MHandle hMemMgr, const JOFFSCREEN* pImg, MLong lFaceSize, 
	PJSEEDS pSeeds,  JMASK *pMask, MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _SkinMask_Seed(const JOFFSCREEN *pImg, MCOLORREF crRef, JMASK *pMask, 
							 MLong *pMskSize, MLong *pMskSum);

JSTATIC MVoid _GetMaskYUYV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
	MLong lReduceW, MLong lReduceH);
//////////////////////////////////////////////////////////////////////////
JSTATIC MRESULT _SortSeeds(MHandle hMemMgr, JSEEDS* pSeeds, MCOLORREF crMid, JPoint ptMid);
JSTATIC MVoid _SortSeedsByDist(JSEEDS* pSeeds, MLong *pDist);
JSTATIC MRESULT _FaceMask(MHandle hMemMgr, const JOFFSCREEN* pImg, JMASK *pMask, 
	PJSEEDS pSeeds, MLong lFaceSize, PJPoint pPtCenter, MCOLORREF crMid, 
	MLong lFaceDistX, MLong lFaceDistY, JENUM_FACEMASK eFaceMask);
JSTATIC MCOLORREF _MidColor(const MCOLORREF *pcrSeed, MLong lSeedNum);
JSTATIC MRESULT	_GenerateSeeds(const JOFFSCREEN* pImg, const MRECT *pFaceRect,
	PJSEEDS pSeeds, MLong lInterFaceOffset);
//////////////////////////////////////////////////////////////////////////

typedef struct  
{
	MLong lSize;
	MLong lSkinSide;
	MLong lSkinDist;
	MLong lNoSkinSide;
	//MLong lBounder;
	//MLong lWeight;
	
	MPOINT ptCenter;
	MCOLORREF crCenter;
}MASK_PARAM;
JSTATIC MVoid _FaceMask_Seed_Ex(const JOFFSCREEN* pImg, const JPoint* pptSeed, 
	MCOLORREF crSeed, const JPoint *pptCenter, MLong lFaceDistX, MLong lFaceDistY, 
	JMASK *pMask, JMASK *pMask2, MVoid *pMemTmp, MLong lMemTmpBytes, MASK_PARAM *pParam);
JSTATIC MBool _IsSkinArea(MCOLORREF crMid, JPoint ptMid, MLong lFaceSize, 
	MLong lSkinSizePre,  MASK_PARAM *pParamSkin);
JSTATIC MVoid _FaceMask_Seed(const JOFFSCREEN* pImg, const JPoint* pptSeed, 
	MCOLORREF crSeed, const JPoint *pptCenter, MLong lFaceDistX, MLong lFaceDistY, 
	JMASK *pMask, MVoid *pMemTmp, MLong lMemTmpBytes, JMASK *pMskLab);
//////////////////////////////////////////////////////////////////////////
JSTATIC MVoid	_RemoveSmallArea(JMASK *pMask, MLong lThreshold, MLong lHoleSize, 
								 MVoid *pMemTmp, MLong lMemBytes);
JSTATIC MLong _SkinRoundLength(LPJMASK pMskSkin, MLong lMskThreshold);
MRESULT SkinMaskFromFaceRect(MHandle hMemMgr, const JOFFSCREEN* pFaceImg, 
							  const MRECT* pFaceRect, MLong lFacesNum, 
							  MLong lMskReduceW, MLong lMskReduceH, 
							  JMASK *pFaceMask)
{
	JOFFSCREEN imgYUV={0}, imgSrc=*pFaceImg;
	JSEEDS seeds={0};		
	MLong lMemBytes = pFaceMask->lWidth*pFaceMask->lHeight*4;
	MByte *pMemTmp = MNull;
	MLong *pDist = MNull;
	JMASK mskSkin = {0};
	MLong lMinSkinSize = (pFaceMask->lWidth*pFaceMask->lHeight)/512;

	MRESULT res = LI_ERR_NONE;	
	MLong lFaceCur, lTimeCur, lSize, lSum, lStartRow;
	JPoint ptCur;	

	imgYUV.fmtImg = FORMAT_YUV;
// 	if(bBilinearReduce)
// 	{
// 		if(pFaceImg->fmtImg == FORMAT_YUYV)
// 			imgYUV.fmtImg = FORMAT_YUYV;
// 	}
	GO(ImgChunky2Plannar(&imgSrc));
	if(lMskReduceW==1 && lMskReduceH==1)
	{
		imgYUV = imgSrc;
	}
	else
	{		
		GO(ImgCreate(hMemMgr, &imgYUV, imgYUV.fmtImg, pFaceMask->lWidth, pFaceMask->lHeight));	
		ReduceImage(&imgSrc, &imgYUV, lMskReduceW, lMskReduceH);
//		_DrawYUVImg(&imgYUV);
	}

	AllocVectMem(hMemMgr, seeds.pptSeed, MAX_SEED_NUM, JPoint);
	AllocVectMem(hMemMgr, seeds.pcrSeed, MAX_SEED_NUM, MCOLORREF);
	AllocVectMem(hMemMgr, pDist, MAX_SEED_NUM, MLong);
	
	AllocVectMem(hMemMgr, pMemTmp, lMemBytes, MByte);
	//////////////////////////////////////////////////////////////////////////
	//Generate face mask firstly
	MaskSet(pFaceMask, 0);
	for(lFaceCur=0; lFaceCur<lFacesNum; lFaceCur++)
	{
		MRECT rtFace = pFaceRect[lFaceCur];
		MCOLORREF crMid;
		MLong lDistX, lDistY;
		
		rtFace.left /= lMskReduceW, rtFace.right /= lMskReduceW;
		rtFace.top /= lMskReduceH, rtFace.bottom /= lMskReduceH;
		ptCur.x = (MShort)((rtFace.left+rtFace.right)/2);
		ptCur.y = (MShort)((rtFace.bottom+rtFace.top)/2);	
		
		lTimeCur = JGetCurrentTime();
		GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 2));
		if(seeds.lSeedNum <= 0)
			continue;
		crMid = _MidColor(seeds.pcrSeed, seeds.lSeedNum);
		
		GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 0));
		GO(_SortSeeds(hMemMgr, &seeds, crMid, ptCur));
//		JAddToTimer(JGetCurrentTime()-lTimeCur, TIMER_AUTO_SEED);
		
		//_DrawSeeds(&img, &seeds);
		lDistX = SQARE(rtFace.right-ptCur.x);
		lDistX += SQARE(rtFace.bottom-ptCur.y);
		lDistY = lDistX;
		GO(_FaceMask(hMemMgr, &imgYUV, pFaceMask, &seeds, 
			(rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top), 
				&ptCur, crMid, lDistX, lDistY, FM_CLEAN));
//		PrintBmp(pFaceMask->pData, pFaceMask->lMaskLine, DATA_U8, pFaceMask->lWidth, pFaceMask->lHeight, 1);
	}		

	//////////////////////////////////////////////////////////////////////////
	//Generate skin mask
	GO(MaskCreate(hMemMgr, &mskSkin, pFaceMask->lWidth, pFaceMask->lHeight));	
	pFaceMask->rcMask.left = pFaceMask->rcMask.top = 0;
	pFaceMask->rcMask.right = pFaceMask->lWidth;
	pFaceMask->rcMask.bottom = pFaceMask->lHeight;
	for (lFaceCur=0; lFaceCur<lFacesNum; lFaceCur++)
	{
		MRECT rtFace = pFaceRect[lFaceCur];
		MCOLORREF crMid;
		MLong i, j=0;		
		rtFace.left /= lMskReduceW, rtFace.right /= lMskReduceW;
		rtFace.top /= lMskReduceH, rtFace.bottom /= lMskReduceH;
		if(lMinSkinSize < (rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top)/16)
			lMinSkinSize = (rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top)/16;

		GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 2));
		if(seeds.lSeedNum <= 0)
			continue;
		crMid = _MidColor(seeds.pcrSeed, seeds.lSeedNum);
		for (i = 0; i<seeds.lSeedNum; i++)
		{
			MCOLORREF crCur = seeds.pcrSeed[i];
			pDist[i] = _GetDist(PIXELVAL_1(crCur),PIXELVAL_2(crCur), PIXELVAL_3(crCur), crMid);
		}
		_SortSeedsByDist(&seeds, pDist);

		//Select seeds
		//Classify
		j = 0;
		for (i=0; i<seeds.lSeedNum; i++)
		{
			MLong lDist = pDist[i];
			if(lDist > MAX_COLOR_DIST)
				break;
			if(lDist<j*32)
				continue;
			ptCur = seeds.pptSeed[i];
			if(pFaceMask->pData[pFaceMask->lMaskLine*ptCur.y+ptCur.x] < MIN_DIST)
				continue;
			seeds.pptSeed[j] = ptCur;
			seeds.pcrSeed[j] = seeds.pcrSeed[i];
			pDist[j] = pDist[i];
			j++;
		}		
		seeds.lSeedNum = j;
		//Generate the skin mask
		j=0;
		for(i=0; i<seeds.lSeedNum; i++)
		{
			_SkinMask_Seed(&imgYUV, seeds.pcrSeed[i], pFaceMask, &lSize, &lSum);				
			if(lSize <= lMinSkinSize)
				continue;
			if(j==0)
			{
				MaskCpy(pFaceMask, &mskSkin);
				j=1;
				continue;
			}

// 			PrintBmp(pFaceMask->pData, pFaceMask->lMaskLine, DATA_U8, pFaceMask->lWidth, pFaceMask->lHeight, 1);
// 			PrintBmp(mskSkin.pData, mskSkin.lMaskLine, DATA_U8, mskSkin.lWidth, mskSkin.lHeight, 1);			
			lSum /= lSize;
			if( lSum < 150
				|| (lSize>(rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top)*3/4 && lSum<175)
				|| (lSize>(rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top)*2 && lSum<200)
				|| lSize>(rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top)*3	)				
				MaskCpy(&mskSkin, pFaceMask);
			else
				MaskCpy(pFaceMask, &mskSkin);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//Remove small skin area
	_RemoveSmallArea(pFaceMask, MIN_DIST, lMinSkinSize, pMemTmp, lMemBytes);
//	PrintBmp(pFaceMask->pData, pFaceMask->lMaskLine, DATA_U8, pFaceMask->lWidth, pFaceMask->lHeight, 1);

	ClearMaskFlag(pFaceMask);
	lStartRow = 0;
	while ((lSize = CatchEachConnectedMask(hMemMgr, pFaceMask, &mskSkin, &lStartRow, &lSum)) != 0)
	{
		MLong i, j, lBoundaryLen;
// 		PrintBmp(pFaceMask->pData, pFaceMask->lMaskLine, DATA_U8, pFaceMask->lWidth, pFaceMask->lHeight, 1);
// 		PrintBmp(mskSkin.pData, mskSkin.lMaskLine, DATA_U8, mskSkin.lWidth, mskSkin.lHeight, 1);
		//Remove small area
// 		if(lSize < lMinSkinSize*256)
// 		{
// 			MaskSub(pFaceMask, &mskSkin, pFaceMask);
// 			continue;
// 		}
		//Remove stripe area
		i = mskSkin.rcMask.right-mskSkin.rcMask.left;
		j = mskSkin.rcMask.bottom-mskSkin.rcMask.top;
		if((i*j) / lSize >= 8)
		{
			MaskSub(pFaceMask, &mskSkin, pFaceMask);
			continue;
		}
		if(i > j)	i=i/j;
		else		i=j/i;
		if(i >= 8)
		{
			MaskSub(pFaceMask, &mskSkin, pFaceMask);
			continue;
		}

		
		//Save the area override with face rectangle.
		for(lFaceCur=0; lFaceCur<lFacesNum; lFaceCur++)
		{
			MRECT rtFace = pFaceRect[lFaceCur];
			i = (rtFace.left+rtFace.right)/(2*lMskReduceW);
			j = (rtFace.top+rtFace.bottom)/(2*lMskReduceH);				
			if(i<mskSkin.rcMask.left || i>=mskSkin.rcMask.right)
				continue;
			if(j<mskSkin.rcMask.top || j>=mskSkin.rcMask.bottom)
				continue;
			break;
		}
		if(lFaceCur<lFacesNum)
			continue;

		lBoundaryLen = _SkinRoundLength(&mskSkin, MIN_DIST);
		lBoundaryLen = (lBoundaryLen*lBoundaryLen*256)/lSum;
		lSum /= lSize;

		if(lSum<155 || lBoundaryLen>170 || lSum*2 - lBoundaryLen < 155)
			MaskSub(pFaceMask, &mskSkin, pFaceMask);
	}

EXT:
	FreeVectMem(hMemMgr, seeds.pptSeed);
	FreeVectMem(hMemMgr, seeds.pcrSeed);
	FreeVectMem(hMemMgr, pDist);
	FreeVectMem(hMemMgr, pMemTmp);
	MaskRelease(hMemMgr, &mskSkin);
	if(lMskReduceW!=1 || lMskReduceH!=1)
		ImgRelease(hMemMgr, &imgYUV);
	return res;
}

//////////////////////////////////////////////////////////////////////////
MLong _SkinRoundLength(LPJMASK pMskSkin, MLong lMskThreshold)
{
	JMaskData *pMskData = pMskSkin->pData + pMskSkin->rcMask.left
		+ pMskSkin->lMaskLine*pMskSkin->rcMask.top;
	MLong lMskLine = pMskSkin->lMaskLine;
	MLong lWidth = pMskSkin->rcMask.right - pMskSkin->rcMask.left;
	MLong y;
	JPoint ptCur, ptPre;
	MLong lRoundSize = 0;
	ptCur = MaskLineRange(pMskData, lWidth, lMskThreshold);
	lRoundSize += ptCur.y - ptCur.x;

	ptPre = ptCur;
	pMskData += lMskLine;
	for(y=pMskSkin->rcMask.bottom-pMskSkin->rcMask.top-1; y!=0; y--, 
		pMskData+=lMskLine, ptPre = ptCur)
	{
		lRoundSize+=2;
		ptCur = MaskLineRange(pMskData, lWidth, lMskThreshold);		
		JASSERT(ptCur.y > ptCur.x);
		lRoundSize += ABS(ptCur.x - ptPre.x);
		lRoundSize += ABS(ptCur.y - ptPre.y);
	}

	lRoundSize += 2;
	lRoundSize += ptPre.y - ptPre.x;
	return lRoundSize;
}

MVoid	_RemoveSmallArea(JMASK *pMask, MLong lThreshold, MLong lHoleSize, 
						MVoid *pMemTmp, MLong lMemBytes)
{
	MDWord x, y, count, k;
	MInt32 t, c[5];
    TRegionItem *pItems = MNull, *pItemCur = MNull;
	MDWord dwWidth = pMask->rcMask.right - pMask->rcMask.left;
	MDWord dwHeight = pMask->rcMask.bottom - pMask->rcMask.top;
    JMaskData *pMaskCur = pMask->pData+pMask->lMaskLine*pMask->rcMask.top+pMask->rcMask.left;
	JMaskData *pMaskPre = pMaskCur - pMask->lMaskLine;
    MInt16 *pColor=MNull, *pColorPre=MNull;
	MDWord dwMaxItem = (lMemBytes-dwWidth*dwHeight*sizeof(MInt16))/sizeof(TRegionItem);
	MDWord dwCountItem = 0;
	MDWord dwMaskExt = pMask->lMaskLine - dwWidth;

	JASSERT((MDWord)lMemBytes > dwWidth*dwHeight*sizeof(MInt16));
	
	//The pColor is the label image, if unlabel, it is set to zero
	pColor = (MInt16*)pMemTmp;
	JMemSet(pColor, 0, lMemBytes);	
	pColorPre = pColor - dwWidth;
	pItems = (TRegionItem *)((MUInt8 *)pMemTmp + dwWidth*dwHeight*sizeof(MInt16));	
	
    for(y=0; y<dwHeight; y++, pMaskCur+=dwMaskExt, pMaskPre+=dwMaskExt)
	{
        for(x=0;x<dwWidth; x++, pMaskCur++, pMaskPre++, pColor++, pColorPre++)
		{			
			if((MLong)(*pMaskCur) < lThreshold)
				continue;

			count=1;
			if(x!=0 && (MLong)(pMaskCur[-1]) >= lThreshold)
			{
				t=pColor[-1]-1;
				do{
					if(t<0) t=-t-1;
					t=(pItems+t)->Color;
				}while(t<0);
				c[count]=t;
				count++;
			}
			if(y!=0 && (MLong)(*pMaskPre) >= lThreshold)
			{
	      		t=pColorPre[0]-1;
				do{
					if(t<0) t=-t-1;
					t=(pItems+t)->Color;
				}while(t<0);
				c[count]=t;

				for(k=1;k<count;k++)
				{					
					if(c[k]==c[count])
						break;
				}
				if(k==count)	count++;
			}

			if(count==1)
			{
				//Add new item
				JASSERT(dwCountItem < dwMaxItem);
				if(dwCountItem < dwMaxItem)
				{
					pItemCur=pItems+dwCountItem;
	      			pItemCur->Color= (MInt16)dwCountItem;
					pItemCur->area = 1;
					dwCountItem++;
	      			*pColor=(MInt16)dwCountItem;
				}
			}
			else
			{
				//Order by label
	      		for(k=2; k<count; k++)
				{
					if(c[1]<=c[k])
						continue;
		     		t=c[k];
		      		c[k]=c[1];
		      		c[1]=t;
				}
				pItemCur=pItems+c[1];
				//Update the later label
 				for(k=count-1;k>1;k--)
				{
		     		pItemCur->area = (MUInt16)(pItemCur->area + (pItems+c[k])->area);
		     		(pItems+c[k])->Color = (MInt16)-(pItemCur->Color+1);
				}
				//Update the min label
				pItemCur->area++;
				*pColor=(MInt16)(pItemCur->Color+1);
				JASSERT(*pColor > 0);
			}
		}// end for x
	}// end for y
		
	//Update the label
	pItemCur = pItems;
	for (y = 0; y < dwCountItem; y ++, pItemCur ++)
	{
		t = pItemCur->Color;
		if (t >= 0)
			continue;
		do {
			if ( t < 0)		
				t = - (t + 1);
			t = (pItems + t)->Color;
		} while(t < 0);
		pItemCur->Color = (MInt16)t;
		pItemCur->area = pItems[t].area;
	}//end for y

	pMaskCur = pMask->pData+pMask->lMaskLine*pMask->rcMask.top+pMask->rcMask.left;
//	PrintBmp(pMaskCur, pMask->lMaskLine, DATA_U8, dwWidth, dwHeight, 1);
	
	pColor = (MInt16*)pMemTmp;
//	PrintChannel(pColor, dwWidth, DATA_I16, dwWidth, dwHeight, 1, 0);

    for(y=0;y<dwHeight;y++, pMaskCur+=dwMaskExt)
	{
        for(x=0;x<dwWidth;x++,pMaskCur++,pColor++)
		{
			MLong lColor = *pColor;
			if(lColor <= 0)
				continue;
			if(pItems[lColor-1].area < lHoleSize)
				*pMaskCur = 0;
		}
	}
	
//	pMaskCur = pMask->pData+pMask->lMaskLine*pMask->rcMask.top+pMask->rcMask.left;
//	PrintBmp(pMaskCur, pMask->lMaskLine, DATA_U8, dwWidth, dwHeight, 1);
}

MVoid _SkinMask_Seed(const JOFFSCREEN *pImg, MCOLORREF crRef, JMASK *pMask, 
					 MLong *pMskSize, MLong *pMskSum)
{
	MDWord x, y;
	MLong lSize = 0, lSum = 0;
	JMaskData *pMaskData = pMask->pData;
	MByte *pImgData = (MByte*)pImg->pixelArray.chunky.pPixel;
	MLong lMaskExt = pMask->lMaskLine - pMask->lWidth;	
	MLong lImgExt = pImg->pixelArray.chunky.dwImgLine - pMask->lWidth*3;
	JASSERT(pImg->fmtImg == FORMAT_YUV);
	JASSERT(pImg->dwWidth-pMask->lWidth==0 && pImg->dwHeight-pMask->lHeight==0);
	for(y=pImg->dwHeight; y!=0; y--, pMaskData+=lMaskExt, pImgData+=lImgExt)
	{
		for(x=pImg->dwWidth; x!=0; x--, pMaskData++, pImgData+=3)
		{
			MLong lMaskData = *pMaskData;
			MLong lDist;
			if(lMaskData >= MAX_DIST)
				continue;
			if(!IS_SKIN_COLOR(pImgData[0], pImgData[1], pImgData[2]))
				continue;
			lDist = _GetDist(pImgData[0], pImgData[1], pImgData[2], crRef);
			lDist = 255 - lDist;
			if(lDist>lMaskData && lDist>= MIN_DIST)
			{
				*pMaskData = (JMaskData)lDist;	
				lSize++, lSum += lDist;
			}
		}
	}
	if(pMskSum != MNull)	*pMskSum = lSum;
	if(pMskSize != MNull)	*pMskSize = lSize;
}

//////////////////////////////////////////////////////////////////////////
MRESULT FaceMaskFromFaceRect(MHandle hMemMgr, const JOFFSCREEN* pFaceImg, 
							  const MRECT* pFaceRect, MLong lFacesNum, 
							  MLong lMskReduceW, MLong lMskReduceH, 
							  JMASK *pFaceMask, JENUM_FACEMASK eFaceMask)
{
	JOFFSCREEN img = *pFaceImg;
	JOFFSCREEN imgYUV={0};

	JSEEDS seeds={0};	
	
	MRESULT res = LI_ERR_NONE;	
	MLong lFaceCur = 0;
	MLong lTimeCur;

	imgYUV.fmtImg = FORMAT_YUV;
// OPTIMIZATION_THREAD
//#ifdef PLATFORM_COACH
// 	if(pFaceImg->fmtImg == FORMAT_YUYV)
// 		imgYUV.fmtImg = FORMAT_YUYV;
// #endif

#ifndef ENABLE_MASK_ONLY_FACE
	if(lMskReduceW==1 && lMskReduceH==1 && img.fmtImg==FORMAT_YUV)
	{
		imgYUV = img;
	}
	else
	{		
		ImgCreate(hMemMgr, &imgYUV, imgYUV.fmtImg, pFaceMask->lWidth, pFaceMask->lHeight);
		ReduceImage(&img, &imgYUV, lMskReduceW, lMskReduceH);	
//		_DrawYUVImg(&imgYUV);
	}
#endif
	AllocVectMem(hMemMgr, seeds.pptSeed, MAX_SEED_NUM, JPoint);
	AllocVectMem(hMemMgr, seeds.pcrSeed, MAX_SEED_NUM, MCOLORREF);	
	
	MaskSet(pFaceMask, 0);	
	pFaceMask->rcMask.left = pFaceMask->rcMask.top = 0;
	pFaceMask->rcMask.right = pFaceMask->lWidth - 1;
	pFaceMask->rcMask.bottom = pFaceMask->lHeight - 1;
	for (lFaceCur=0; lFaceCur<lFacesNum; lFaceCur++)
	{
		MRECT rtFace = pFaceRect[lFaceCur];	
		JMASK mskTmp = *pFaceMask;
		MPOINT imgOffset = {0};
		img = *pFaceImg;
		ImgChunky2Plannar(&img);	

		Logger( "Face rect: (%d, %d),(%d, %d)\n", 
			rtFace.left, rtFace.top, rtFace.right, rtFace.bottom);

#ifdef ENABLE_MASK_ONLY_FACE
		//The reduced image can be limited to the rectangle round the face.			
		imgOffset.x = (rtFace.left-(rtFace.right-rtFace.left)/4);
		imgOffset.y = rtFace.top-(rtFace.bottom-rtFace.top)/4;		
		if(imgOffset.x<0)	imgOffset.x = 0;
		else	imgOffset.x = (imgOffset.x/lMskReduceW)*lMskReduceW;
		if(imgOffset.y<0)	imgOffset.y = 0;		
		else	imgOffset.y = (imgOffset.y/lMskReduceH)*lMskReduceH;
		imgOffset.x = FLOOR_2(imgOffset.x), imgOffset.y = FLOOR_2(imgOffset.y);
		rtFace.left -= imgOffset.x, rtFace.right -= imgOffset.x;
		rtFace.top -= imgOffset.y, rtFace.bottom -= imgOffset.y;	
		
		ImgOffset(&img, imgOffset.x, imgOffset.y);
		img.dwWidth = rtFace.right-rtFace.left + (rtFace.right-rtFace.left)/2;
		img.dwHeight = rtFace.bottom-rtFace.top + (rtFace.bottom-rtFace.top)/2;	
		//Make the with/height be the time of 4.
		img.dwWidth = ((img.dwWidth+(lMskReduceW*4-1))/(lMskReduceW*4))*(lMskReduceW*4);
		img.dwHeight = ((img.dwHeight+(lMskReduceH*4-1))/(lMskReduceH*4))*(lMskReduceH*4);
		if(img.dwWidth > pFaceImg->dwWidth-imgOffset.x)
			img.dwWidth = ((pFaceImg->dwWidth-imgOffset.x)/(lMskReduceW*4))*(lMskReduceW*4);
		if(img.dwHeight > pFaceImg->dwHeight-imgOffset.y)
			img.dwHeight = ((pFaceImg->dwHeight-imgOffset.y)/(lMskReduceH*4))*(lMskReduceH*4);
		
		mskTmp.pData += (imgOffset.y/lMskReduceH)*mskTmp.lMaskLine+imgOffset.x/lMskReduceW;
		mskTmp.lWidth = img.dwWidth/lMskReduceW, mskTmp.lHeight = img.dwHeight/lMskReduceH;		
		JASSERT(mskTmp.lWidth%4==0 && mskTmp.lHeight%4==0);	
		
		if(lMskReduceW==1 && lMskReduceH==1 && img.fmtImg==FORMAT_YUV)
		{
			imgYUV = img;
		}
		else
		{
			ImgRelease(hMemMgr, &imgYUV);
			GO(ImgCreate(hMemMgr, &imgYUV, imgYUV.fmtImg, mskTmp.lWidth, mskTmp.lHeight));
			ReduceImage(&img, &imgYUV, lMskReduceW, lMskReduceH);	
//			_DrawYUVImg(&imgYUV);
		}
#endif		//ENABLE_MASK_ONLY_FACE		
		rtFace.left /= lMskReduceW, rtFace.right /= lMskReduceW;
		rtFace.top /= lMskReduceH, rtFace.bottom /= lMskReduceH;
		if(rtFace.right > (MInt32)imgYUV.dwWidth)	rtFace.right = imgYUV.dwWidth;
		if(rtFace.bottom > (MInt32)imgYUV.dwHeight)	rtFace.bottom = imgYUV.dwHeight;

		{
			MCOLORREF crMid;
			JPoint ptCenter;
			MLong lDistX, lDistY;
			ptCenter.x = (MShort)((rtFace.left+rtFace.right)/2);
			ptCenter.y = (MShort)((rtFace.bottom+rtFace.top)/2);	

			lTimeCur = JGetCurrentTime();
			if(eFaceMask == FM_ORIGINAL)
			{
				GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 2));
				if(seeds.lSeedNum <= 0)
					continue;
				crMid = _MidColor(seeds.pcrSeed, seeds.lSeedNum);
				GO(FilterSeeds(hMemMgr, crMid, &seeds, 0));
			}
			else
			{
				GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 2));
				if(seeds.lSeedNum <= 0)
					continue;
				crMid = _MidColor(seeds.pcrSeed, seeds.lSeedNum);
				
				GO(_GenerateSeeds(&imgYUV, &rtFace, &seeds, 0));
				GO(_SortSeeds(hMemMgr, &seeds, crMid, ptCenter));
			}
//			JAddToTimer(JGetCurrentTime()-lTimeCur, TIMER_AUTO_SEED);
			if(seeds.lSeedNum <= 0)
				continue;
			
			//_DrawSeeds(&img, &seeds);
			
			if(lFaceCur==0)
			{			
				mskTmp.rcMask.left = mskTmp.rcMask.right = seeds.pptSeed->x;
				mskTmp.rcMask.top = mskTmp.rcMask.bottom = seeds.pptSeed->y;
			}
			lDistX = SQARE(rtFace.right-ptCenter.x);
			lDistX += SQARE(rtFace.bottom-ptCenter.y);
			lDistY = lDistX;
			GO(_FaceMask(hMemMgr, &imgYUV, &mskTmp, &seeds, 
				(rtFace.right-rtFace.left)*(rtFace.bottom-rtFace.top), 
				&ptCenter, crMid, lDistX, lDistY, eFaceMask));
			mskTmp.rcMask.right = MIN(mskTmp.rcMask.right, mskTmp.lWidth-1);
			mskTmp.rcMask.bottom = MIN(mskTmp.rcMask.bottom, mskTmp.lHeight-1);
            CALLBACK_TICK("After _FaceMask, in facemask.c");
			//PrintBmp(mskTmp.pData, mskTmp.lMaskLine, DATA_U8, mskTmp.lWidth, mskTmp.lHeight, 1);
		}
	}		
	
	//PrintBmp(pFaceMask->pData, pFaceMask->lMaskLine, DATA_U8, pFaceMask->lWidth, pFaceMask->lHeight, 1);
	
EXT:
	FreeVectMem(hMemMgr, seeds.pptSeed);
	FreeVectMem(hMemMgr, seeds.pcrSeed);	
	if(lMskReduceW!=1 || lMskReduceH!=1)
		ImgRelease(hMemMgr, &imgYUV);
	return res;
}


#define HIGH_LIGHT2		220
MCOLORREF _MidColor(const MCOLORREF *pcrSeed, MLong lSeedNum)
{
	MLong i;
	MLong cr1=0, cr2=0, cr3=0;
	MLong lSeedNum2=0;
	JASSERT(lSeedNum <= MAX_SEED_NUM);
	JASSERT(lSeedNum > 0);
	
	for(i=0; i<lSeedNum; i++)
	{
		if(PIXELVAL_1(pcrSeed[i]) >= HIGH_LIGHT2)
			continue;
		cr1 += PIXELVAL_1(pcrSeed[i]);
		cr2 += PIXELVAL_2(pcrSeed[i]);
		cr3 += PIXELVAL_3(pcrSeed[i]);
		lSeedNum2++;
	}
	if(lSeedNum2==0)
	{
		for(i=0; i<lSeedNum; i++)
		{
			cr1 += PIXELVAL_1(pcrSeed[i]);
			cr2 += PIXELVAL_2(pcrSeed[i]);
			cr3 += PIXELVAL_3(pcrSeed[i]);
			lSeedNum2++;
		}
	}

	return PIXELVAL(cr1/lSeedNum2, cr2/lSeedNum2, cr3/lSeedNum2);
}

MRESULT _FaceMask(MHandle hMemMgr, const JOFFSCREEN* pImg, JMASK *pMask, 
	PJSEEDS pSeeds, MLong lFaceSize, PJPoint pPtCenter, 
	MCOLORREF crMid, MLong lFaceDistX, MLong lFaceDistY, JENUM_FACEMASK eFaceMask)
{
	MByte *pMemTmp = MNull;
	MLong lMemTmpBytes = pMask->lWidth*pMask->lHeight*4;
    JMASK mskTmp = {0}, mskTmp2 = {0}, mskLab = {0};
	MLong i, lSkinSize=0;
    
	MRESULT res = LI_ERR_NONE;

	if (pSeeds->lSeedNum <= 0)
		return LI_ERR_SEEDS;

    AllocVectMem(hMemMgr, pMemTmp, lMemTmpBytes, MByte);
    if(eFaceMask == FM_ORIGINAL)
    {
        //Original mask algorithm
        GO(MaskCreate(hMemMgr, &mskLab, pMask->lWidth, pMask->lHeight));
        for(i=0; i<pSeeds->lSeedNum; i++)
        {
//          ClearMaskFlag(pMask);
            MaskSet(&mskLab, 0);
            _FaceMask_Seed(pImg, pSeeds->pptSeed+i, pSeeds->pcrSeed[i], pPtCenter, 
                lFaceDistX, lFaceDistY, pMask, pMemTmp, lMemTmpBytes, &mskLab);
            CALLBACK_TICK("In loop of _FaceMask_Seed");
        }
    }
    else
    {
        GO(MaskCreate(hMemMgr, &mskTmp, pMask->lWidth, pMask->lHeight));
        GO(MaskCreate(hMemMgr, &mskTmp2, pMask->lWidth, pMask->lHeight));
        MaskSet(&mskTmp, 0);
        MaskSet(&mskTmp2, 0);
        
        mskTmp2.rcMask.left = mskTmp2.rcMask.right = pSeeds->pptSeed->x;
        mskTmp2.rcMask.top = mskTmp2.rcMask.bottom = pSeeds->pptSeed->y;
        for (i=0; i<pSeeds->lSeedNum; i++)
        {
            MASK_PARAM paramMask = {0};
            if(mskTmp2.pData[pSeeds->pptSeed[i].y*mskTmp2.lMaskLine
                + pSeeds->pptSeed[i].x] >= 240)
                continue;
            mskTmp.rcMask.left = mskTmp.rcMask.right = pSeeds->pptSeed[i].x;
            mskTmp.rcMask.top = mskTmp.rcMask.bottom = pSeeds->pptSeed[i].y;
            //PrintBmp(mskTmp.pData, mskTmp.lMaskLine, DATA_U8, mskTmp.lWidth, mskTmp.lHeight, 1);
            _FaceMask_Seed_Ex(pImg, pSeeds->pptSeed+i, pSeeds->pcrSeed[i], pPtCenter, 
                lFaceDistX, lFaceDistY, &mskTmp, &mskTmp2, pMemTmp, lMemTmpBytes, &paramMask);
            ClearMaskFlag(&mskTmp);
            CALLBACK_TICK("In loop of _FaceMask_Seed");
            //PrintBmp(mskTmp.pData, mskTmp.lMaskLine, DATA_U8, mskTmp.lWidth, mskTmp.lHeight, 1);
            if(paramMask.lSize == 0)
                continue;	
            
            //goto EXTSKIN;
            
            mskTmp2.rcMask = RectUnion(&mskTmp2.rcMask, &mskTmp.rcMask);
            if(_IsSkinArea(crMid, *pPtCenter, lFaceSize, lSkinSize, &paramMask))
            {
				// Modified by che --- for multi_face bug ---- 20091203
				if(i==0)
				{
					CpyImgMem(pMask->pData+pMask->lMaskLine*mskTmp.rcMask.top+mskTmp.rcMask.left, 
						pMask->lMaskLine, mskTmp.pData+mskTmp.lMaskLine*mskTmp.rcMask.top+mskTmp.rcMask.left, 
						mskTmp.lMaskLine, mskTmp.rcMask.right-mskTmp.rcMask.left, 
						mskTmp.rcMask.bottom-mskTmp.rcMask.top, JMaskData);
					pMask->rcMask = RectUnion(&pMask->rcMask, &mskTmp.rcMask);
				}
				else
				{
					MaskUnion(&mskTmp, pMask, pMask);
				}
                lSkinSize += paramMask.lSize;
            }
            else
            {
                CpyImgMem(mskTmp.pData+mskTmp.lMaskLine*mskTmp.rcMask.top+mskTmp.rcMask.left, 
                    mskTmp.lMaskLine, pMask->pData+pMask->lMaskLine*mskTmp.rcMask.top+mskTmp.rcMask.left, 
                    pMask->lMaskLine, mskTmp.rcMask.right-mskTmp.rcMask.left, 
                    mskTmp.rcMask.bottom-mskTmp.rcMask.top, JMaskData);
            }
            //PrintBmp(pMask->pData, pMask->lMaskLine, DATA_U8, pMask->lWidth, pMask->lHeight, 1);
        }
        
        //PrintBmp(mskTmp2.pData, mskTmp2.lMaskLine, DATA_U8, mskTmp2.lWidth, mskTmp2.lHeight, 1);
        //PrintBmp(pMask->pData, pMask->lMaskLine, DATA_U8, pMask->lWidth, pMask->lHeight, 1);
        //JASSERT(lSkinSize == MaskSize_NotZero(pMask)); error!!!
        //JASSERT(lSkinSize >= lFaceSize/2);
        if(lSkinSize < lFaceSize/2)	//Remove too much right skin!
        {
            CpyImgMem(pMask->pData+pMask->lMaskLine*mskTmp2.rcMask.top+mskTmp2.rcMask.left, 
                pMask->lMaskLine, mskTmp2.pData+mskTmp2.lMaskLine*mskTmp2.rcMask.top+mskTmp2.rcMask.left, 
                mskTmp2.lMaskLine, mskTmp2.rcMask.right-mskTmp2.rcMask.left, 
                mskTmp2.rcMask.bottom-mskTmp2.rcMask.top, JMaskData);
            pMask->rcMask = RectUnion(&pMask->rcMask, &mskTmp2.rcMask);

//		lSkinSize = MaskSize_NotZero(pMask);
// 		if(lSkinSize < lFaceSize/2)
// 			res = LI_ERR_SEEDS;
        }
    }
EXT:
    MaskRelease(hMemMgr, &mskLab);
	MaskRelease(hMemMgr, &mskTmp);
	MaskRelease(hMemMgr, &mskTmp2);
	FreeVectMem(hMemMgr, pMemTmp);
	return res;
}

MBool _IsSkinArea(MCOLORREF crMid, JPoint ptMid, MLong lFaceSize, 
				  MLong lSkinSizePre,  MASK_PARAM *pParamSkin)
{
	MLong lColorDist, lSpaceDist;
	
	//Remove the background or hair	
	if(lSkinSizePre>=lFaceSize/2 && pParamSkin->lSize>=lFaceSize/4 )
	{
		if(pParamSkin->lSkinDist>220)	/*<220*/
			return MFalse;
		if(PIXELVAL_1(pParamSkin->crCenter)*3<=PIXELVAL_1(crMid)*2)
			return MFalse;		
	}
	// 	//Remove the bounder mask
	// 	if(pParamSkin->lBounder*2>pParamSkin->lSkinSide)
	// 		return MFalse;
	
	//Save the similar color
	lColorDist = _GetDist(PIXELVAL_1(crMid), PIXELVAL_2(crMid), PIXELVAL_3(crMid), pParamSkin->crCenter);
	if(lColorDist <= 175)		
		return MTrue;
	if(pParamSkin->lSkinDist<250 && pParamSkin->lSkinSide*2>pParamSkin->lNoSkinSide)	/*<2*/
		return MTrue;
	if(pParamSkin->lSkinDist<220 && pParamSkin->lSkinSide*3>pParamSkin->lNoSkinSide)	/*<220 >3*/
		return MTrue;
	
	//Save the large area
	lSpaceDist = SQARE(pParamSkin->ptCenter.x-ptMid.x)+SQARE(pParamSkin->ptCenter.y-ptMid.y);
	if(pParamSkin->lSize > lFaceSize/16)
	{
		/*Save the first big area, exclude the hair*/
		if(lSpaceDist*8<lFaceSize && PIXELVAL_1(pParamSkin->crCenter)*2>PIXELVAL_1(crMid))	
			return MTrue;
		if(lSpaceDist*4<lFaceSize && pParamSkin->lSkinSide>pParamSkin->lNoSkinSide)	/*>6, >400*/
			return MTrue;
	}
	
	//Save the highlight area
	if(PIXELVAL_1(pParamSkin->crCenter)>PIXELVAL_1(crMid))
	{
		if(pParamSkin->lSkinSide>pParamSkin->lNoSkinSide)
			return MTrue;
		if(pParamSkin->lSkinSide*3>pParamSkin->lNoSkinSide && pParamSkin->lSkinDist<420)	/*>3, >420*/
			return MTrue;
	}
	if(PIXELVAL_1(pParamSkin->crCenter)>220)
	{
		if(lSpaceDist<lFaceSize/4 && pParamSkin->lSize*64>=lFaceSize)	
			return MTrue;
		if(pParamSkin->lSkinSide*5 > pParamSkin->lNoSkinSide)		/*>5*/
			return MTrue;
	}
	
	return MFalse;
}

MVoid _FaceMask_Seed(const JOFFSCREEN* pImg, const JPoint* pptSeed, 
	MCOLORREF crSeed, const JPoint *pptCenter, MLong lFaceDistX, MLong lFaceDistY, 
	JMASK *pMask, MVoid *pMemTmp, MLong lMemTmpBytes, JMASK *pMskLab)
{
	MLong lMskWidth = pMask->lWidth, lMskHeight = pMask->lHeight;

	JMaskData  *pMskData = pMask->pData;	
	MLong lMskLine = pMask->lMaskLine;	
	MUInt8 *pImgData = pImg->pixelArray.chunky.pPixel;
	MLong lImgLine = pImg->pixelArray.chunky.dwImgLine;
	
	MLong lPtsNum = 0;
	MLong lPtsTmpMaxNum = lMemTmpBytes/sizeof(JPoint);
	JPoint* pPtsTmp = (JPoint*)pMemTmp;	
	
	MRECT	*prtMask = &pMask->rcMask;	
	MLong  lFaceDist=0;
	
	JASSERT(pImg->dwWidth-pMask->lWidth==0 && pImg->dwHeight-pMask->lHeight==0);
	if(pptCenter != MNull)
	{
		if(lFaceDistX <= lFaceDistY)
		{
			lFaceDist  = lFaceDistY;
			lFaceDistY /= lFaceDistX;
			lFaceDistX = 1;
		}
		else
		{
			lFaceDist  = lFaceDistX;
			lFaceDistX /= lFaceDistY;
			lFaceDistY = 1;
		}
	}

	
	pPtsTmp[lPtsNum] = *pptSeed;
	lPtsNum++;
	do 
	{
		JPoint ptCur = pPtsTmp[--lPtsNum];
		MLong lMaskX = ptCur.x, lMaskY = ptCur.y;
		MLong lDist = 0;
		MByte *pMaskTmp = pMskData + lMskLine*lMaskY + lMaskX;
        MByte *pMskLabCur = pMskLab->pData + pMskLab->lMaskLine*lMaskY + lMaskX;
		MLong lMask = *pMaskTmp;
		MLong lCr1, lCr2, lCr3;
		
// 		if(FLAG_ISSET(lMask))
// 			continue;	
// 		*pMaskTmp = (MByte)FLAG_SET(lMask);	
        if (*pMskLabCur != 0)
            continue;
        *pMskLabCur = 255;
		if(lMask >= MAX_DIST)
			continue;

#ifdef ENABLE_MASK_ONLY_FACE
		if(pptCenter!=MNull && (lMaskX-pptCenter->x)*(lMaskX-pptCenter->x)*lFaceDistY
			+(lMaskY-pptCenter->y)*(lMaskY-pptCenter->y)*lFaceDistX 
			>= lFaceDist)	
			continue;
#endif	

#ifdef ENABLE_YUYV_REDUCE
		if(pImg->fmtImg==FORMAT_YUYV)
		{
			MUInt8 *pImgTmp = pImgData + lImgLine*lMaskY + (lMaskX/2)*4;
			lCr1 = pImgTmp[(lMaskX%2)*2];
			lCr2 = pImgTmp[1], lCr3 = pImgTmp[3];
		}
		else
#endif
		{
			MUInt8 *pImgTmp = pImgData + lImgLine*lMaskY + lMaskX*3;
			lCr1 = pImgTmp[0], lCr2=pImgTmp[1], lCr3=pImgTmp[2];
			JASSERT(pImg->fmtImg == FORMAT_YUV);
		}
        
        lDist = 255 - _GetDist(lCr1, lCr2, lCr3, crSeed);
		if (lDist < MIN_DIST || lDist < lMask)
			continue;
//		*pMaskTmp = (MByte)FLAG_SET(lDist);	
		*pMaskTmp = (MByte)lDist;	

		if(lMaskX < prtMask->left)	prtMask->left = lMaskX;
		else if(lMaskX > prtMask->right)	prtMask->right = lMaskX;
		if(lMaskY < prtMask->top)	prtMask->top = lMaskY;
		else if(lMaskY > prtMask->bottom)	prtMask->bottom = lMaskY;
		
		if(lPtsNum >= lPtsTmpMaxNum-4)
			continue;
		//FOUR CONNECTION
		//if (lMaskY+1<lMskHeight && !FLAG_ISSET(pMaskTmp[lMskLine]))
        if (lMaskY+1<lMskHeight && pMskLabCur[pMskLab->lMaskLine]==0)
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY+1);
			lPtsNum ++;
		}
		//if (lMaskX-1>=0 && !FLAG_ISSET(pMaskTmp[-1]))
        if (lMaskX-1>=0 &&  pMskLabCur[-1]==0)
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX-1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		//if (lMaskX+1<lMskWidth && !FLAG_ISSET(pMaskTmp[1]))
        if (lMaskX+1<lMskWidth &&  pMskLabCur[1]==0)
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX+1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		//if (lMaskY-1>=0 && !FLAG_ISSET(pMaskTmp[-lMskLine]))
        if (lMaskY-1>=0 && pMskLabCur[-pMskLab->lMaskLine]==0)
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY-1);
			lPtsNum ++;
		}
	} while(lPtsNum>0);
}

MVoid _FaceMask_Seed_Ex(const JOFFSCREEN* pImg, const JPoint* pptSeed, MCOLORREF crSeed, 
		const JPoint *pptCenter, MLong lFaceDistX, MLong lFaceDistY, 
		JMASK *pMask, JMASK *pMask2, MVoid *pMemTmp, MLong lMemTmpBytes, MASK_PARAM *pParam)
{
	MLong	lTimerCur = JGetCurrentTime();
	MLong	lMskWidth = pMask->lWidth;
	MLong	lMskHeight = pMask->lHeight;	
	
	JMaskData *pMskData = pMask->pData;
	MLong lMskLine = pMask->lMaskLine;
	MUInt8 *pImgData = pImg->pixelArray.chunky.pPixel;
	MLong lImgLine = pImg->pixelArray.chunky.dwImgLine;
	
	MLong	lPtsNum = 0;
	MLong   lPtsTmpMaxNum = lMemTmpBytes/sizeof(JPoint);
	JPoint  *pPtsTmp = (JPoint*)pMemTmp;
	
	MRECT *prtMask = &pMask->rcMask;
	MLong lSize=0, lSkinSide=0, lNoSkinSide=0, lSkinDist=0/*, lBounder=0*//*, lWeight=0*/; 
	MLong lCrSum1=0, lCrSum2=0, lCrSum3=0, lXSum=0, lYSum=0;
	MLong lFaceDist;
//	MLong lTmp;
	
	JASSERT(pImg->dwWidth-pMask->lWidth==0 && pImg->dwHeight-pMask->lHeight==0);
	JASSERT(pImg->fmtImg == FORMAT_YUV);
	if(lFaceDistX <= lFaceDistY)
	{
		lFaceDist  = lFaceDistY;
		lFaceDistY /= lFaceDistX;
		lFaceDistX = 1;
	}
	else
	{
		lFaceDist  = lFaceDistX;
		lFaceDistX /= lFaceDistY;
		lFaceDistY = 1;
	}
	
	pPtsTmp[lPtsNum] = *pptSeed;
	lPtsNum++;
	do 
	{
		JPoint ptCur = pPtsTmp[--lPtsNum];
		MLong lMaskX = ptCur.x, lMaskY = ptCur.y;
		MLong lDist = 0;
		JMaskData *pMaskTmp = pMskData + lMskLine*lMaskY + lMaskX;
		JMaskData *pMaskTmp2 = pMask2->pData + pMask2->lMaskLine*lMaskY + lMaskX;
		MLong lMask = *pMaskTmp;
		MLong lCr1, lCr2, lCr3;
		
		if(FLAG_ISSET(lMask))
			continue;	
		*pMaskTmp = (JMaskData)FLAG_SET(lMask);	

#ifdef ENABLE_YUYV_REDUCE
		if(pImg->fmtImg==FORMAT_YUYV)
		{
			MUInt8 *pImgTmp = pImgData + lImgLine*lMaskY + (lMaskX/2)*4;
			lCr1 = pImgTmp[(lMaskX%2)*2];
			lCr2 = pImgTmp[1], lCr3 = pImgTmp[3];
		}
		else
#endif
		{
			MUInt8 *pImgTmp = pImgData + lImgLine*lMaskY + lMaskX*3;
			lCr1 = pImgTmp[0], lCr2=pImgTmp[1], lCr3=pImgTmp[2];
			JASSERT(pImg->fmtImg == FORMAT_YUV);
		}
		lDist = 255-_GetDist(lCr1, lCr2, lCr3, crSeed);

#ifdef ENABLE_MASK_ONLY_FACE
		lTmp = (lMaskX-pptCenter->x)*(lMaskX-pptCenter->x)*lFaceDistY
			+(lMaskY-pptCenter->y)*(lMaskY-pptCenter->y)*lFaceDistX;
		if(lTmp > lFaceDist)	
			lDist = (lDist*lFaceDist/4)/(lTmp-lFaceDist*3/4);	//Smooth the bounder
#endif			

		if(lDist<MIN_DIST || lDist<=lMask)
		{
			if(lMask<MIN_DIST)
				lNoSkinSide++;
			else
				lSkinSide++, lSkinDist+=255-lDist;
			continue;
		}	

		*pMaskTmp = (JMaskData)FLAG_SET(lDist);			
		lSize ++;
		//lWeight += lDist;
		lXSum += lMaskX, lYSum += lMaskY;
		lCrSum1 += lCr1, lCrSum2 += lCr2, lCrSum3 += lCr3;		
		if(lMaskX < prtMask->left)	prtMask->left = lMaskX;
		else if(lMaskX > prtMask->right)	prtMask->right = lMaskX;
		if(lMaskY < prtMask->top)	prtMask->top = lMaskY;
		else if(lMaskY > prtMask->bottom)	prtMask->bottom = lMaskY;		
		if(*pMaskTmp2 < lDist)	
			*pMaskTmp2 = (JMaskData)lDist;
		
		if(lPtsNum >= lPtsTmpMaxNum-4)
			continue;
		//FOUR CONNECTION
		if (lMaskY+1<lMskHeight && !FLAG_ISSET(pMaskTmp[lMskLine]))
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY+1);
			lPtsNum ++;
		}
		if (lMaskX-1>=0 && !FLAG_ISSET(pMaskTmp[-1]))
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX-1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		if (lMaskX+1<lMskWidth && !FLAG_ISSET(pMaskTmp[1]))
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX+1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		if (lMaskY-1>=0 && !FLAG_ISSET(pMaskTmp[-lMskLine]))
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY-1);
			lPtsNum ++;
		}
	} while(lPtsNum>0);
	//PrintBmp(pMask->pData, pMask->lMaskLine, DATA_U8, pMask->lWidth, pMask->lHeight, 1);


	pParam->lSize = lSize;
	pParam->lSkinSide = lSkinSide;
	pParam->lNoSkinSide = lNoSkinSide;
	//pParam->lBounder = lBounder;
	if(lSize>0)
	{
		//pParam->lWeight = lWeight/lSize;
		pParam->ptCenter.x = lXSum/lSize;
		pParam->ptCenter.y = lYSum/lSize;
		pParam->crCenter = PIXELVAL(lCrSum1/lSize, lCrSum2/lSize, lCrSum3/lSize);
	}
	if(lSkinSide>0)
		pParam->lSkinDist = lSkinDist/lSkinSide;	
	prtMask->right++, prtMask->bottom++;
//	JAddToTimer(JGetCurrentTime()-lTimerCur, TIMER_COLOR_MASK);
}
MRESULT _SortSeeds(MHandle hMemMgr, JSEEDS* pSeeds, MCOLORREF crMid, JPoint ptMid)
{
	MLong* pDist = MNull;
	MRESULT res = LI_ERR_NONE;
	MLong x;
	AllocVectMem(hMemMgr, pDist, pSeeds->lSeedNum, MLong);
	for(x=0; x<pSeeds->lSeedNum; x++)
		pDist[x] = _GetDist(PIXELVAL_1(crMid), PIXELVAL_2(crMid), PIXELVAL_3(crMid), pSeeds->pcrSeed[x]);
	for(x=0; x<pSeeds->lSeedNum; x++)
		pDist[x] += SQARE(ptMid.x-pSeeds->pptSeed[x].x)+SQARE(ptMid.y-pSeeds->pptSeed[x].y);

	_SortSeedsByDist(pSeeds, pDist);
EXT:
	FreeVectMem(hMemMgr, pDist);
	return res;
}

MVoid _SortSeedsByDist(JSEEDS* pSeeds, MLong *pDist)
{
	MLong x, y;
	for (x = 0; x<pSeeds->lSeedNum; x++)
	{
		MLong lDistMin = pDist[x]; 
		MLong lIndexMin = x;
		for (y = x+1; y<pSeeds->lSeedNum; y++)
		{
			if (pDist[y] < lDistMin)
			{
				lDistMin = pDist[y];
				lIndexMin = y;
			}
		}
		
		{
			JPoint ptTmp = pSeeds->pptSeed[x];
			MCOLORREF crTmp = pSeeds->pcrSeed[x];
			pSeeds->pptSeed[x] = pSeeds->pptSeed[lIndexMin];
			pSeeds->pptSeed[lIndexMin] = ptTmp;
			
			pSeeds->pcrSeed[x] = pSeeds->pcrSeed[lIndexMin];
			pSeeds->pcrSeed[lIndexMin] = crTmp;
			
			pDist[lIndexMin] = pDist[x];
			pDist[x] = lDistMin;
		}
	}	
}



MVoid	ClearMaskFlag(JMASK* pMask)
{
	MLong lTimeCur = JGetCurrentTime();
	MLong x, y;
	MByte *pMaskData = MNull;
	MLong lMaskExt, lWidth;
	MLong left = pMask->rcMask.left - MASK_EXT,
	      right = pMask->rcMask.right + MASK_EXT,
	      top = pMask->rcMask.top -MASK_EXT,
	      bottom = pMask->rcMask.bottom + MASK_EXT;

	if(pMask->rcMask.left >= pMask->rcMask.right || 
		pMask->rcMask.top >= pMask->rcMask.bottom)
		return;

	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(right > pMask->lWidth) right = pMask->lWidth;
	if(bottom > pMask->lHeight) bottom = pMask->lHeight;
	
	pMaskData = pMask->pData + pMask->lMaskLine * top + left;
	lWidth = right - left;
	lMaskExt = pMask->lMaskLine - lWidth;	
	if(lWidth <= 4)
	{
		for(y= bottom-top; y!=0; y--, pMaskData+=lMaskExt)
		{	
			for(x=lWidth; x!=0; x--, pMaskData++)
				*pMaskData = (MByte)FLAG_CLEAR(*pMaskData);	
		}
	}
	else
	{
		for(y= bottom-top; y!=0; y--, pMaskData+=lMaskExt)
		{		 
			MLong lTmp = ((MDWord)(-(MLong)pMaskData))%4;
			JASSERT((pMaskData-pMask->pData)%pMask->lMaskLine==left);
			for(x=lTmp; x!=0; x--, pMaskData++)
				*pMaskData = (MByte)FLAG_CLEAR(*pMaskData);		
			JASSERT((MDWord)pMaskData%4 == 0);	
			lTmp = lWidth - lTmp;
			for(x=lTmp/4; x!=0; x--, pMaskData+=4)
				*((MDWord*)(pMaskData)) = (MDWord)(FLAG_CLEAR_32(*((MDWord*)(pMaskData))));
			for(x=lTmp%4; x!=0; x--, pMaskData++)
				*pMaskData = (MByte)FLAG_CLEAR(*pMaskData);	
		}
	}
//	JAddToTimer(JGetCurrentTime()-lTimeCur, TIMER_MASK_CLEAR);
}

//////////////////////////////////////////////////////////////////////////
JSTATIC MVoid _ReduceImg_YUYV422_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
						   MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_YUV422PLANAR_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_RGB_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_BGR_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_RGB565_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_YUYV422(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
								 MLong lMskReduceW, MLong lMskReduceH);
JSTATIC MVoid _ReduceImg_YUV420VUVU_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
								MLong lMskReduceW, MLong lMskReduceH);
MVoid ReduceImage(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
				  MLong lMskReduceW, MLong lMskReduceH)
{
	if(pImgSrc->fmtImg == FORMAT_YUYV)
	{
		if(pImgRlt->fmtImg == FORMAT_YUYV)
			_ReduceImg_YUYV422(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
		else
			_ReduceImg_YUYV422_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
	}
	else if(pImgSrc->fmtImg == FORMAT_YUV422PLANAR)
	{
		_ReduceImg_YUV422PLANAR_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
	}
	else if(pImgSrc->fmtImg == FORMAT_YUV420_VUVU)
	{
		_ReduceImg_YUV420VUVU_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
	}
#ifdef TRIM_RGB
    else if(pImgSrc->fmtImg == FORMAT_RGB)
    {
        _ReduceImg_RGB_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
    }
	else if(pImgSrc->fmtImg == FORMAT_BGR)
	{
		_ReduceImg_BGR_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
	}
    else if (pImgSrc->fmtImg == FORMAT_RGB565)
    {
        _ReduceImg_RGB565_YUV(pImgSrc, pImgRlt, lMskReduceW, lMskReduceH);
    }
#endif  //TRIM_RGB
	else
	{
		MDWord x, y;
		JASSERT(pImgSrc->dwWidth >= lMskReduceW*pImgRlt->dwWidth 
			&& pImgSrc->dwHeight >= lMskReduceH*pImgRlt->dwHeight);
		JASSERT(IF_FMT_BASE(pImgSrc->fmtImg)==FORMAT_CR_YUV || IF_FMT_BASE(pImgSrc->fmtImg)==FORMAT_CR_YVU);
		for(y=0; y<pImgRlt->dwHeight; y++)	
		{
			for(x=0; x<pImgRlt->dwWidth; x++)
			{
				MCOLORREF crRef = ImgGetPixel(pImgSrc, 
					x*lMskReduceW, y*lMskReduceH);
				ImgSetPixel(pImgRlt, x, y, crRef);
			}
		}
	}
}

MVoid _ReduceImg_YUV420VUVU_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
								MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;	
	MInt32 lRltImgW = pImgRlt->dwWidth, lRltImgH = pImgRlt->dwHeight;
	MByte *pDataSrcY = (MByte*)pImgSrc->pixelArray.planar.pPixel[0];
	MByte *pDataSrcVU = (MByte*)pImgSrc->pixelArray.planar.pPixel[1];

	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lSrcYExt = pImgSrc->pixelArray.planar.dwImgLine[0]*lMskReduceH 
		- lRltImgW*lMskReduceW;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - lRltImgW*3;

	JASSERT(lRltImgW*lMskReduceW<=(MInt32)pImgSrc->dwWidth && 
			lRltImgH*lMskReduceH<=(MInt32)pImgSrc->dwHeight);
	JASSERT(pImgSrc->fmtImg==FORMAT_YUV420_VUVU && pImgRlt->fmtImg == FORMAT_YUV);
	JASSERT(pImgSrc->dwWidth%2==0 && pImgSrc->dwHeight%2==0);

	if(lMskReduceW==1)
	{
		MLong lSrcVUExt = pImgSrc->pixelArray.planar.dwImgLine[1] - lRltImgW;
		JASSERT((MInt32)pImgSrc->dwHeight==lRltImgH && (MInt32)pImgSrc->dwWidth==lRltImgW);
		for(y=lRltImgH; y!=0; y--, pDataSrcY+=lSrcYExt, pDataRlt+=lRltExt)	
		{
			for(x=lRltImgW/2; x!=0; x--, pDataSrcY+=2, pDataSrcVU+=2, pDataRlt+=6)
			{
				pDataRlt[0] = pDataSrcY[0];
				pDataRlt[1] = pDataSrcVU[1];
				pDataRlt[2] = pDataSrcVU[0];	
				pDataRlt[3] = pDataSrcY[1];
				pDataRlt[4] = pDataSrcVU[1];
				pDataRlt[5] = pDataSrcVU[0];
			}
			pDataSrcVU += (y%2==0) ? (-lRltImgW) : lSrcVUExt;
		}
	}
	else
	{
		MLong lSrcVUExt = pImgSrc->pixelArray.planar.dwImgLine[1]*lMskReduceH/2 
						  - lRltImgW*lMskReduceW;
		for(y=lRltImgH; y!=0; y--, pDataSrcY+=lSrcYExt, pDataSrcVU+=lSrcVUExt, pDataRlt+=lRltExt)	
		{
			for(x=lRltImgW; x!=0; x--, pDataSrcY+=lMskReduceW, pDataSrcVU+=lMskReduceW, pDataRlt+=3)
			{
				pDataRlt[0] = pDataSrcY[0];
				pDataRlt[1] = pDataSrcVU[1];
				pDataRlt[2] = pDataSrcVU[0];
			}
		}
	}
}

MVoid _ReduceImg_YUV422PLANAR_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;	
	MByte *pDataSrcY = (MByte*)pImgSrc->pixelArray.planar.pPixel[0];
	MByte *pDataSrcU = (MByte*)pImgSrc->pixelArray.planar.pPixel[1];
	MByte *pDataSrcV = (MByte*)pImgSrc->pixelArray.planar.pPixel[2];
	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lSrcYExt = pImgSrc->pixelArray.planar.dwImgLine[0]*lMskReduceH 
		- pImgRlt->dwWidth*lMskReduceW;
	MLong lSrcUExt = pImgSrc->pixelArray.planar.dwImgLine[1]*lMskReduceH 
		- pImgRlt->dwWidth*lMskReduceW/2;
	MLong lSrcVExt = pImgSrc->pixelArray.planar.dwImgLine[2]*lMskReduceH 
		- pImgRlt->dwWidth*lMskReduceW/2;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - pImgRlt->dwWidth*3;
	JASSERT(pImgRlt->dwWidth*lMskReduceW<=pImgSrc->dwWidth && 
		pImgRlt->dwHeight*lMskReduceH<=pImgSrc->dwHeight);
	JASSERT(pImgRlt->fmtImg==FORMAT_YUV && pImgSrc->fmtImg == FORMAT_YUV422PLANAR);
	JASSERT(lMskReduceW>1 && lMskReduceH>1);
	for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrcY+=lSrcYExt,
		pDataSrcU+=lSrcUExt,pDataSrcV+=lSrcVExt, pDataRlt+=lRltExt)	
	{
		for(x=pImgRlt->dwWidth; x!=0; x--, pDataSrcY+=lMskReduceW, 
			pDataSrcU+=lMskReduceW/2, pDataSrcV+=lMskReduceW/2, pDataRlt+=3)
		{
			pDataRlt[0] = pDataSrcY[0];
			pDataRlt[1] = pDataSrcU[0];
			pDataRlt[2] = pDataSrcV[0];
		}
	}
}

#ifdef TRIM_RGB
MVoid _ReduceImg_RGB_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;
    MByte *pDataSrc = (MByte*)pImgSrc->pixelArray.chunky.pPixel;
	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
 	MLong lSrcExt = pImgSrc->pixelArray.chunky.dwImgLine*lMskReduceH 
 		- (pImgRlt->dwWidth)*lMskReduceW*3;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - (pImgRlt->dwWidth)*3;
	JASSERT(pImgRlt->dwWidth*lMskReduceW<=pImgSrc->dwWidth && 
		pImgRlt->dwHeight*lMskReduceH<=pImgSrc->dwHeight);
	JASSERT(pImgRlt->fmtImg==FORMAT_YUV && pImgSrc->fmtImg == FORMAT_RGB);
	for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)        
	{
        for(x=pImgRlt->dwWidth; x!=0; x--, pDataSrc+=lMskReduceW*3, pDataRlt+=3)
		{
            MCOLORREF yuv;
            MCOLORREF rgb = PIXELVAL(pDataSrc[0], pDataSrc[1], pDataSrc[2]);
            RGB2YUV(rgb, &yuv);
			pDataRlt[0] = PIXELVAL_1(yuv);
			pDataRlt[1] = PIXELVAL_2(yuv);
			pDataRlt[2] = PIXELVAL_3(yuv);
		}
	}
}

MVoid _ReduceImg_BGR_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
						 MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;
    MByte *pDataSrc = (MByte*)pImgSrc->pixelArray.chunky.pPixel;
	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lSrcExt = pImgSrc->pixelArray.chunky.dwImgLine*lMskReduceH 
		- (pImgRlt->dwWidth)*lMskReduceW*3;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - (pImgRlt->dwWidth)*3;
	JASSERT(pImgRlt->dwWidth*lMskReduceW<=pImgSrc->dwWidth && 
		pImgRlt->dwHeight*lMskReduceH<=pImgSrc->dwHeight);
	JASSERT(pImgRlt->fmtImg==FORMAT_YUV && pImgSrc->fmtImg == FORMAT_BGR);
	for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)        
	{
        for(x=pImgRlt->dwWidth; x!=0; x--, pDataSrc+=lMskReduceW*3, pDataRlt+=3)
		{
            MCOLORREF yuv;
            MCOLORREF rgb = PIXELVAL(pDataSrc[2], pDataSrc[1], pDataSrc[0]);
            RGB2YUV(rgb, &yuv);
			pDataRlt[0] = PIXELVAL_1(yuv);
			pDataRlt[1] = PIXELVAL_2(yuv);
			pDataRlt[2] = PIXELVAL_3(yuv);
		}
	}
}
MVoid _ReduceImg_RGB565_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
                            MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;
    MByte *pDataSrc = (MByte*)pImgSrc->pixelArray.chunky.pPixel;
	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
 	MLong lSrcExt = pImgSrc->pixelArray.chunky.dwImgLine*lMskReduceH 
 		- (pImgRlt->dwWidth)*lMskReduceW*2;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - (pImgRlt->dwWidth)*3;
	JASSERT(pImgRlt->dwWidth*lMskReduceW<=pImgSrc->dwWidth && 
		pImgRlt->dwHeight*lMskReduceH<=pImgSrc->dwHeight);
	JASSERT(pImgRlt->fmtImg==FORMAT_YUV && pImgSrc->fmtImg == FORMAT_RGB565);
	for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)        
	{
        for(x=pImgRlt->dwWidth; x!=0; x--, pDataSrc+=lMskReduceW*2, pDataRlt+=3)
		{
            MCOLORREF yuv;
            MDWord cr_tmp = *((MWord*)pDataSrc),
                cr_r = ((cr_tmp >> 11) & 0x001F) << 3,
                cr_g = ((cr_tmp >>  5) & 0x003F) << 2,
                cr_b = (cr_tmp & 0x001F) << 3;
            RGB2YUV(PIXELVAL(cr_r, cr_g, cr_b), &yuv);
			pDataRlt[0] = PIXELVAL_1(yuv);
			pDataRlt[1] = PIXELVAL_2(yuv);
			pDataRlt[2] = PIXELVAL_3(yuv);
		}
	}
}
#endif  //TRIM_RGB
//////////////////////////////////////////////////////////////////////////
#if defined  OPTIMIZATION_HAREWARE && defined PLATFORM_COACH
typedef struct tagSYUVINFO
{
    // Number of pixels per image line.
    unsigned int uiSizeX;
	
    // Number of image lines.
    unsigned int uiSizeY;
	
    // Address of the YUV data buffer.
    unsigned char *pbyBuffer;
	
    // Address of the originally allocated buffer (pbyBuffer points inside)
    unsigned char *pbyAllocatedAddress;
	
    // Number of bytes to jump from beginning of line N to beginning of line
    // N+1.
    unsigned int uiLineJumpInBytes;
	
    // Exposure with which frame was taken.
    unsigned short uwExposure;
	
    // Gain with which frame was taken.
    unsigned short uwGain;
	
    // OPTIONAL BUFFER INFO 
    // the buffer
    unsigned char *pbyOptionalBuffer;
	
    // Number of pixels per image line in Optional buffer.
    unsigned int uiOptionalSizeX;
	
    // Number of image lines in Optional buffer.
    unsigned int uiOptionalSizeY;
	
    // Scale Factor
    unsigned short uwScaleFactor;
	
    // Corresponding number of readout frame.
    unsigned long  ulFrameCount;
	
} SYUVINFO, *PSYUVINFO;
extern unsigned char DirectScaleYUV(PSYUVINFO psInputInfo, 
               unsigned int*     puiDestWidth, 
               unsigned int*     puiDestHeight,
               unsigned char*     pbyDestBuffer);

MVoid _ReduceImg_YUYV422(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
						 MLong lMskReduceW, MLong lMskReduceH)
{		
	MUInt8 *pDataSrc = (MUInt8*)pImgSrc->pixelArray.chunky.pPixel;
	MLong lSrcLine = pImgSrc->pixelArray.chunky.dwImgLine;
	MUInt8 *pDataRlt = (MUInt8*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lWidth = pImgRlt->dwWidth, lHeight = pImgRlt->dwHeight;
	SYUVINFO sInfo={0};
		
	
	JASSERT(pImgSrc->fmtImg==FORMAT_YUYV && pImgRlt->fmtImg==FORMAT_YUYV);
	JASSERT(lWidth*lMskReduceW<=(MLong)pImgSrc->dwWidth);
	JASSERT(lHeight*lMskReduceH<=(MLong)pImgSrc->dwHeight);
	if(lMskReduceW==1 && lMskReduceH==1)
	{
		ImgCpy(pImgSrc, pImgRlt);
		return;
	}
	JASSERT(lMskReduceW%2==0 && lMskReduceH%2==0);
	JASSERT(lWidth%2==0);
	
	sInfo.uiSizeX = pImgSrc->dwWidth;
	sInfo.uiSizeY = pImgSrc->dwHeight;
	sInfo.pbyBuffer = pDataSrc;
	sInfo.uiLineJumpInBytes = lSrcLine;
	DirectScaleYUV(&sInfo, &lWidth, &lHeight, pDataRlt);	
}

#else 	//OPTIMIZATION_HAREWARE
MVoid _ReduceImg_YUYV422(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
						 MLong lMskReduceW, MLong lMskReduceH)
{
	MUInt8 *pDataSrc = (MUInt8*)pImgSrc->pixelArray.chunky.pPixel;
	MLong lSrcLine = pImgSrc->pixelArray.chunky.dwImgLine;
	MUInt8 *pDataRlt = (MUInt8*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lWidth = FLOOR_2(pImgRlt->dwWidth), lHeight = pImgRlt->dwHeight;
	MLong lSrcExt = lSrcLine*lMskReduceH - lWidth*lMskReduceW*2;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine-lWidth*2;
	MLong x, y;
	JASSERT(pImgSrc->fmtImg==FORMAT_YUYV && pImgRlt->fmtImg==FORMAT_YUYV);
	JASSERT(lWidth*lMskReduceW<=(MLong)pImgSrc->dwWidth);
	JASSERT(lHeight*lMskReduceH<=(MLong)pImgSrc->dwHeight);
	if(lMskReduceW==1 && lMskReduceH==1)
	{
		ImgCpy(pImgSrc, pImgRlt);
		return;
	}
	JASSERT(lMskReduceW%2==0 && lMskReduceH%2==0);
	JASSERT(lWidth%2==0);
	for(y=0; y<lHeight; y++, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)
	{
		for(x=0; x<lWidth; x+=2, pDataSrc+=lMskReduceW*4, pDataRlt+=4)
		{
			MLong lY1Sum=0, lY2Sum=0, lUSum=0, lVSum=0;
			MLong i, j;
			for(i=0; i<lMskReduceH; i++, pDataSrc+=lSrcLine)
			{
				for(j=0; j<lMskReduceW; j+=2)
				{
					lY1Sum += pDataSrc[j*2] + pDataSrc[j*2+2];
					lUSum += pDataSrc[j*2+1];
					lVSum += pDataSrc[j*2+3];
				}
				for(j=lMskReduceW; j<lMskReduceW*2; j+=2)
				{
					lY2Sum += pDataSrc[j*2] + pDataSrc[j*2+2];
					lUSum += pDataSrc[j*2+1];
					lVSum += pDataSrc[j*2+3];
				}
			}
			
			pDataRlt[0]   = (MUInt8)(lY1Sum/(lMskReduceW*lMskReduceH));
			pDataRlt[1] = (MUInt8)(lUSum/(lMskReduceW*lMskReduceH));
			pDataRlt[2] = (MUInt8)(lY2Sum/(lMskReduceW*lMskReduceH));
			pDataRlt[3] = (MUInt8)(lVSum/(lMskReduceW*lMskReduceH));
			pDataSrc -= lSrcLine*lMskReduceH;
		}
	}
}
#endif	//OPTIMIZATION_HAREWARE

MVoid _ReduceImg_YUYV422_YUV(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, 
							 MLong lMskReduceW, MLong lMskReduceH)
{
	MDWord x, y;
	MByte *pDataSrc = (MByte*)pImgSrc->pixelArray.chunky.pPixel;
	MByte *pDataRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
	MLong lSrcExt = pImgSrc->pixelArray.chunky.dwImgLine*lMskReduceH 
		- FLOOR_2(pImgRlt->dwWidth)*lMskReduceW*2;
	MLong lRltExt = pImgRlt->pixelArray.chunky.dwImgLine - FLOOR_2(pImgRlt->dwWidth)*3;
	JASSERT(pImgRlt->dwWidth*lMskReduceW<=pImgSrc->dwWidth && 
		pImgRlt->dwHeight*lMskReduceH<=pImgSrc->dwHeight);
	JASSERT(pImgRlt->fmtImg==FORMAT_YUV && pImgSrc->fmtImg == FORMAT_YUYV);
	if(lMskReduceW==1)
	{
		for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)	
		{
			for(x=pImgRlt->dwWidth/2; x!=0; x--, pDataSrc+=lMskReduceW*4, pDataRlt+=6)
			{
				pDataRlt[0] = pDataSrc[0];
				pDataRlt[1] = pDataSrc[1];
				pDataRlt[2] = pDataSrc[3];
				pDataRlt[3] = pDataSrc[2];
				pDataRlt[4] = pDataSrc[1];
				pDataRlt[5] = pDataSrc[3];
			}
		}
	}
	else
	{
		JASSERT(lMskReduceW%2 == 0);
		for(y=pImgRlt->dwHeight; y!=0; y--, pDataSrc+=lSrcExt, pDataRlt+=lRltExt)	
		{
			for(x=pImgRlt->dwWidth/2; x!=0; x--, pDataSrc+=lMskReduceW*4, pDataRlt+=6)
			{
				pDataRlt[0] = pDataSrc[0];
				pDataRlt[1] = pDataSrc[1];
				pDataRlt[2] = pDataSrc[3];
				pDataRlt[3] = pDataSrc[lMskReduceW*2];
				pDataRlt[4] = pDataSrc[lMskReduceW*2+1];
				pDataRlt[5] = pDataSrc[lMskReduceW*2+3];
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
JINLINE MLong	_GetDist(MLong cr1, MLong cr2, MLong cr3, MCOLORREF crCenter)
{	
	MLong tmp = 0, dist = 0;
	tmp = cr1 - PIXELVAL_1(crCenter);	
	dist = SQARE(tmp)>>5;
	tmp = cr2 - PIXELVAL_2(crCenter);
	dist += SQARE(tmp)<<1;
	tmp = cr3 - PIXELVAL_3(crCenter);
	dist += SQARE(tmp)<<2;	
	return dist;
}
////////////////////////////////////////////////////////////////////////////////////
#define SEED_AREA_LEN		4
#define SEED_AREA_RADIO		(SEED_AREA_LEN/2)
#define SEED_AREA_SIZE_ST	4
MRESULT	GenerateSeeds(const JOFFSCREEN* pImg, const MRECT *pFaceRect,
					  PJSEEDS pSeeds, MLong lInterFaceOffset)
{
	MRESULT res = LI_ERR_NONE;
	MLong lSeedNum = 0,
		  lSeedWidth = (pFaceRect->right-pFaceRect->left+MAX_SEED_LEN-1) / MAX_SEED_LEN,
		  lSeedHeight = (pFaceRect->bottom-pFaceRect->top+MAX_SEED_LEN-1) / MAX_SEED_LEN;
	MLong lLeft = pFaceRect->left + lSeedWidth*lInterFaceOffset, 
		  lRight = pFaceRect->right - lSeedWidth*lInterFaceOffset,
		  lTop = pFaceRect->top + lSeedHeight*lInterFaceOffset,
		  lBottom = pFaceRect->bottom- lSeedWidth*lInterFaceOffset;
	MLong lImgW = pImg->dwWidth, lImgH = pImg->dwHeight;
	MLong x, y, i, j;
	MLong numY = 0, numCb = 0, numCr = 0;
	MCOLORREF crRef;

	MLong lCenterX = (lLeft + lRight)/2;
	MLong lCenterY = (lBottom + lTop)/2;
	MLong lDist = 1<<30;
	MLong lCofX, lCofY;

	if(lRight-lCenterX<=0 || lBottom-lCenterY<=0 || lSeedWidth<=0 || lSeedHeight<=0)
	{
		pSeeds->lSeedNum = 0;
		return LI_ERR_NONE;
	}
	lCofX = lDist/((lRight-lCenterX)*(lRight-lCenterX));
	lCofY = lDist/((lBottom-lCenterY)*(lBottom-lCenterY));

	if(lBottom >= lImgH)
		lBottom = lImgH - 1;

	for (y=lTop; y<=lBottom; y+=lSeedHeight)
	{
		if(y-SEED_AREA_RADIO < 0 || y+SEED_AREA_RADIO > lImgH)
			continue;
		for (x=lLeft; x<lRight; x+=lSeedWidth)
		{
			if(x-SEED_AREA_RADIO < 0 || x+SEED_AREA_RADIO > lImgW)
				continue;

			if(lCofX*(x-lCenterX)*(x-lCenterX) + lCofY*(y-lCenterY)*(y-lCenterY) > lDist)
				continue;

			numY = 0, numCb = 0, numCr = 0;
			for(i=-SEED_AREA_RADIO; i<SEED_AREA_RADIO; i++)
			{
				for(j=-SEED_AREA_RADIO; j<SEED_AREA_RADIO; j++)
				{
					MCOLORREF crCur = ImgGetPixel(pImg, x+i, y+j);
					numY += PIXELVAL_1(crCur);
					numCb += PIXELVAL_2(crCur);
					numCr += PIXELVAL_3(crCur);
				}
			}

			numY = DOWN_ROUND(numY, SEED_AREA_SIZE_ST);
			numCb = DOWN_ROUND(numCb, SEED_AREA_SIZE_ST);
			numCr = DOWN_ROUND(numCr, SEED_AREA_SIZE_ST);
#ifdef TRIM_RGB
			if (pImg->fmtImg==FORMAT_RGB || pImg->fmtImg==FORMAT_RGB565)
			{
				MCOLORREF crTmp;
				RGB2YUV(PIXELVAL(numY, numCb, numCr), &crTmp);
				numY = PIXELVAL_1(crTmp);
				numCb = PIXELVAL_2(crTmp);
				numCr = PIXELVAL_3(crTmp);
			}
			else if (pImg->fmtImg==FORMAT_BGR)
			{
				MCOLORREF crTmp;
				RGB2YUV(PIXELVAL(numCr, numCb, numY), &crTmp);
				numY = PIXELVAL_1(crTmp);
				numCb = PIXELVAL_2(crTmp);
				numCr = PIXELVAL_3(crTmp);
			}
#endif

			if (!IS_SKIN_COLOR(numY, numCb, numCr))
				continue;
			pSeeds->pptSeed[lSeedNum].x = (MShort)x;
			pSeeds->pptSeed[lSeedNum].y = (MShort)y;
			crRef = PIXELVAL(numY, numCb, numCr);
			pSeeds->pcrSeed[lSeedNum] = crRef;
			lSeedNum++;
		}
	}
	
	pSeeds->lSeedNum = lSeedNum;
	if(lSeedNum == 0)
		return LI_ERR_SEEDS;
	return res;
}

MRESULT	_GenerateSeeds(const JOFFSCREEN* pImg, const MRECT *pFaceRect,
	PJSEEDS pSeeds, MLong lInterFaceOffset)
{
	MLong lSeedNum = 0,
		lSeedWidth = (pFaceRect->right-pFaceRect->left+MAX_SEED_LEN-1) / MAX_SEED_LEN,
		lSeedHeight = (pFaceRect->bottom-pFaceRect->top+MAX_SEED_LEN-1) / MAX_SEED_LEN;
	MLong lLeft = pFaceRect->left + lSeedWidth*lInterFaceOffset, 
		lRight = pFaceRect->right - lSeedWidth*lInterFaceOffset,
		lTop = pFaceRect->top + lSeedHeight*lInterFaceOffset,
		lBottom = pFaceRect->bottom- lSeedWidth*lInterFaceOffset;
	MLong x, y;
	
	MLong lCenterX = (lLeft + lRight)/2;
	MLong lCenterY = (lBottom + lTop)/2;
	MLong lDist = 1<<30;
	MLong lCofX, lCofY;
	
	if(lRight-lCenterX<=0 || lBottom-lCenterY<=0 || lSeedWidth<=0 || lSeedHeight<=0)
	{
		pSeeds->lSeedNum = 0;
		return LI_ERR_NONE;
	}

	lCofX = lDist/((lRight-lCenterX)*(lRight-lCenterX));
	lCofY = lDist/((lBottom-lCenterY)*(lBottom-lCenterY));		
	for (y=lTop; y<lBottom; y+=lSeedHeight)
	{
		MLong lCofY2 = lCofY*(y-lCenterY)*(y-lCenterY);
		for (x=lLeft; x<lRight; x+=lSeedWidth)
		{		
			MCOLORREF crRef;	
			if(lCofX*(x-lCenterX)*(x-lCenterX) + lCofY2 > lDist)
				continue;
			
			crRef = ImgGetPixel(pImg, x, y);			
#ifdef TRIM_RGB
			if (pImg->fmtImg==FORMAT_RGB || pImg->fmtImg==FORMAT_RGB565)
				RGB2YUV(crRef, &crRef);
			else if(pImg->fmtImg==FORMAT_BGR)
				RGB2YUV(PIXELVAL(PIXELVAL_3(crRef), PIXELVAL_2(crRef), PIXELVAL_1(crRef)), &crRef);
#endif
			
			if (!IS_SKIN_COLOR(PIXELVAL_1(crRef), PIXELVAL_2(crRef), PIXELVAL_3(crRef)))
				continue;
			pSeeds->pptSeed[lSeedNum].x = (MShort)x;
			pSeeds->pptSeed[lSeedNum].y = (MShort)y;
			pSeeds->pcrSeed[lSeedNum] = crRef;
			lSeedNum++;
		}
	}
	
	pSeeds->lSeedNum = lSeedNum;
	if(lSeedNum == 0)
		return LI_ERR_SEEDS;
	return LI_ERR_NONE;
}

MRESULT FilterSeeds(MHandle hMemMgr, MCOLORREF crMid, PJSEEDS pSeeds, 
					MLong lGroupStep)
{
	MRESULT res = LI_ERR_NONE;
	MLong x = 0;
	MLong lSeedNum = pSeeds->lSeedNum;
	MLong *pDist = MNull;
	AllocVectMem(hMemMgr, pDist, lSeedNum, MLong);	
	for (x = 0; x<lSeedNum; x++)
	{
		MCOLORREF crCur = pSeeds->pcrSeed[x];
		pDist[x] = _GetDist(PIXELVAL_1(crCur),PIXELVAL_2(crCur), PIXELVAL_3(crCur), crMid);
	}

	//Sort by Distance.
	_SortSeedsByDist(pSeeds, pDist);
	pSeeds->lSeedNum = lSeedNum = lSeedNum*9/10;


	//Classify
	if(lGroupStep > 0)
	{
		pSeeds->lSeedNum = lSeedNum;
		lSeedNum = 0;
		for (x=0; x<pSeeds->lSeedNum; x++)
		{
			if(pDist[x]<lSeedNum*lGroupStep)
				continue;
			pSeeds->pptSeed[lSeedNum] = pSeeds->pptSeed[x];
			pSeeds->pcrSeed[lSeedNum] = pSeeds->pcrSeed[x];
			pDist[lSeedNum] = pDist[x];
			lSeedNum++;
		}		
		pSeeds->lSeedNum = lSeedNum;
	}

	//Trim the color far away the center skin.	
	while(--lSeedNum>=0 && pDist[lSeedNum] > MAX_COLOR_DIST);	
	// modified by che ---- 20090828
	lSeedNum = (lSeedNum<0)?0:lSeedNum;
	
	//Save bright pixel
	for(x = lSeedNum; x<pSeeds->lSeedNum; x++)
	{
		if(PIXELVAL_1(pSeeds->pcrSeed[x]) <= HIGH_LIGHT)
			continue;
		pSeeds->pcrSeed[lSeedNum] = pSeeds->pcrSeed[x];
		pSeeds->pptSeed[lSeedNum] = pSeeds->pptSeed[x];
		pDist[lSeedNum] = pDist[x];
		lSeedNum ++;
	}
	pSeeds->lSeedNum = lSeedNum+1;

EXT:
	FreeVectMem(hMemMgr, pDist);
	return res;
}

#define COLOR_LEN		4
#define LUMIN_LEN		16
MCOLORREF MidColor(MHandle hMemMgr, const MCOLORREF *pcrSeed, MLong lSeedNum)
{
	MRESULT res = LI_ERR_NONE;
	MVoid *pTmp = MNull;
	MLong i,  lTmpBytes = MAX_SEED_NUM;
	MLong cr1=0, cr2=0, cr3=0;
	JASSERT(lSeedNum <= MAX_SEED_NUM);
	JASSERT(lSeedNum > 0);
	
	if(lTmpBytes < 256*sizeof(MWord))
		lTmpBytes = 256*sizeof(MWord);
	AllocVectMem(hMemMgr,pTmp, lTmpBytes, MByte);
	
	for(i=0; i<lSeedNum; i++)
	{
		((MByte*)pTmp)[i] = (MByte)PIXELVAL_1(pcrSeed[i]);
	}
 	cr1 = FindMidian(pTmp, lSeedNum, DATA_U8);

	SetVectZero(pTmp, 256/COLOR_LEN*sizeof(MWord));
	for(i=0; i<lSeedNum; i++)
		((MWord*)pTmp)[PIXELVAL_2(pcrSeed[i])/COLOR_LEN]++;
	cr2 = FindMaxIndex(pTmp, 256/COLOR_LEN, DATA_U16);
	cr2 = cr2 * COLOR_LEN + COLOR_LEN/2;
	
	SetVectZero(pTmp, 256/COLOR_LEN*sizeof(MWord));
	for(i=0; i<lSeedNum; i++)
		((MWord*)pTmp)[PIXELVAL_3(pcrSeed[i])/COLOR_LEN]++;
	cr3 = FindMaxIndex(pTmp, 256/COLOR_LEN, DATA_U16);
	cr3 = cr3 * COLOR_LEN + COLOR_LEN/2;

EXT:
	FreeVectMem(hMemMgr, pTmp);
	return PIXELVAL(cr1, cr2, cr3);
}

MRESULT SmoothMask(MHandle hMemMgr, 
				   const JMASK* pMaskSrc, 
				   JMASK* pMaskRlt,
                   MLong dwLen)
{
	if(pMaskSrc != pMaskRlt)
		MaskCpy(pMaskSrc, pMaskRlt);
	pMaskRlt->rcMask.left -= dwLen;
	pMaskRlt->rcMask.right += dwLen;
	pMaskRlt->rcMask.top -= dwLen;
	pMaskRlt->rcMask.bottom += dwLen;
	RectTrim(&pMaskRlt->rcMask, 0, 0, pMaskRlt->lWidth, pMaskRlt->lHeight);
	
	return SmoothBlock(hMemMgr, pMaskRlt->pData + pMaskRlt->rcMask.left
		+ pMaskRlt->lMaskLine*pMaskRlt->rcMask.top, pMaskRlt->lMaskLine, DATA_U8, 
		pMaskRlt->pData + pMaskRlt->rcMask.left + pMaskRlt->lMaskLine*pMaskRlt->rcMask.top, 
		pMaskRlt->lMaskLine, DATA_U8, pMaskRlt->rcMask.right-pMaskRlt->rcMask.left, 
		pMaskRlt->rcMask.bottom-pMaskRlt->rcMask.top, dwLen);
}

//////////////////////////////////////////////////////////////////////////
JSTATIC MLong CatchConnectedMask_Seed(JMASK* pMaskFrom, JMASK* pMaskTo, 						 
									  MLong lSeedX, MLong lSeedY, MLong *plSum, 	
									  MByte* pMemTmp, MLong lMemTmpBytes);
MLong	CatchEachConnectedMask(MHandle hMemMgr, JMASK* pMaskFrom, JMASK* pMaskTo, 
							   MLong *plStartRow, MLong *plSum)
{
	MRESULT res = LI_ERR_NONE;
	MLong lMskSize=0;
	MLong x, y;
	JMaskData *pDataFrom = pMaskFrom->pData+pMaskFrom->lMaskLine**plStartRow;
	MLong lMemTmpBytes = (pMaskFrom->lWidth+MASK_EXT*2)
		*(pMaskFrom->lHeight+MASK_EXT*2)*sizeof(JPoint);
	MByte *pMemTmp = MNull;
	MLong lTimerCur = JGetCurrentTime();
	MLong lLeft = pMaskFrom->rcMask.left;
	MLong lRight = pMaskFrom->rcMask.right;
	MLong lBottom = pMaskFrom->rcMask.bottom;
	AllocVectMem(hMemMgr, pMemTmp, lMemTmpBytes, MByte);	
	JASSERT(pMaskFrom->lWidth == pMaskTo->lWidth);
	JASSERT(pMaskFrom->lHeight == pMaskTo->lHeight);
	for(y=*plStartRow; y<lBottom; y++, pDataFrom+=pMaskFrom->lMaskLine)
	{
		for(x=lLeft; x<lRight; x++)
		{
			if(pDataFrom[x]==0 || FLAG_ISSET(pDataFrom[x]))
				continue;
			
			MaskSet(pMaskTo, 0);
			lMskSize = CatchConnectedMask_Seed(pMaskFrom, pMaskTo, x, y, 
				plSum, pMemTmp, lMemTmpBytes);
			*plStartRow = y;
			goto EXT;
		}
	}
//	JAddToTimer(JGetCurrentTime()-lTimerCur, TIMER_CatchEachConnectedMask);
EXT:
	FreeVectMem(hMemMgr, pMemTmp);
	return lMskSize;
}

MLong CatchConnectedMask_Seed(JMASK* pMaskFrom, JMASK* pMaskTo, 						 
							  MLong lSeedX, MLong lSeedY, MLong *plSum, 
							  MByte* pMemTmp, MLong lMemTmpBytes)
{
	MLong lLeft = pMaskFrom->rcMask.left-MASK_EXT;
	MLong lRight = pMaskFrom->rcMask.right+MASK_EXT;
	MLong lTop = pMaskFrom->rcMask.top-MASK_EXT;
	MLong lBottom = pMaskFrom->rcMask.bottom+MASK_EXT;
	MLong 	lMskFromLine = pMaskFrom->lMaskLine;
	
	MLong	lPtsNum = 0;
	MLong   lPtsTmpMaxNum = lMemTmpBytes/sizeof(JPoint);
	JPoint  *pPtsTmp = (JPoint*)pMemTmp;	
	MRECT	*prtMask = &pMaskTo->rcMask;
	MLong lMskSize=0, lMskSum=0;

	//some pixels exterior the original mask rectangle is needed when filter.
	if(lLeft < 0) lLeft = 0; 
	if(lTop < 0) lTop = 0;
	if(lRight > pMaskFrom->lWidth)	lRight = pMaskFrom->lWidth;
	if(lBottom > pMaskFrom->lHeight)	lBottom = pMaskFrom->lHeight;
	
	prtMask->left = prtMask->right = lSeedX;
	prtMask->top = prtMask->bottom = lSeedY;
	
	pPtsTmp[lPtsNum].x = (MWord)(lSeedX);
	pPtsTmp[lPtsNum].y = (MWord)(lSeedY);
	lPtsNum++;
	do 
	{
		JPoint ptCur = pPtsTmp[--lPtsNum];
		MLong lMaskX = ptCur.x, lMaskY = ptCur.y;
		JMaskData *pDataFrom = pMaskFrom->pData + lMskFromLine*lMaskY + lMaskX;
		MLong lDataFrom = *pDataFrom;

		if(lDataFrom==0 || FLAG_ISSET(lDataFrom))
			continue;

		pMaskTo->pData[pMaskTo->lMaskLine*lMaskY+lMaskX] = (JMaskData)lDataFrom;
		*pDataFrom = (JMaskData)FLAG_SET(lDataFrom);
		
		lMskSum+=lDataFrom;
		lMskSize++;
		if(lMaskX < prtMask->left)	prtMask->left = lMaskX;
		else if(lMaskX > prtMask->right)	prtMask->right = lMaskX;
		if(lMaskY < prtMask->top)	prtMask->top = lMaskY;
		else if(lMaskY > prtMask->bottom)	prtMask->bottom = lMaskY;
		
		if(lPtsNum >= lPtsTmpMaxNum-4)
			continue;
		//FOUR CONNECTION
		if (lMaskY+1<lBottom && pDataFrom[lMskFromLine]>0 && !FLAG_ISSET(pDataFrom[lMskFromLine]))
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY+1);
			lPtsNum ++;
		}
		if (lMaskX-1>=lLeft && pDataFrom[-1]>0 && !FLAG_ISSET(pDataFrom[-1]))
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX-1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		if (lMaskX+1<lRight && pDataFrom[1]>0&& !FLAG_ISSET(pDataFrom[1]))
		{
			pPtsTmp[lPtsNum].x = (MWord) (lMaskX+1);
			pPtsTmp[lPtsNum].y = (MWord) lMaskY;
			lPtsNum ++;
		}
		if (lMaskY-1>=lTop && pDataFrom[-lMskFromLine]>0&& !FLAG_ISSET(pDataFrom[-lMskFromLine]))
		{
			pPtsTmp[lPtsNum].x = (MWord) lMaskX;
			pPtsTmp[lPtsNum].y = (MWord) (lMaskY-1);
			lPtsNum ++;
		}
	} while(lPtsNum>0);

	prtMask->right++, prtMask->bottom++;
	if(prtMask->right > pMaskFrom->rcMask.right)
		prtMask->right = pMaskFrom->rcMask.right;
	if(prtMask->bottom > pMaskFrom->rcMask.bottom)
		prtMask->bottom = pMaskFrom->rcMask.bottom;
	if(prtMask->top < pMaskFrom->rcMask.top)
		prtMask->top = pMaskFrom->rcMask.top;
	if(prtMask->left < pMaskFrom->rcMask.left)
		prtMask->left = pMaskFrom->rcMask.left;
	if(plSum != MNull) *plSum = lMskSum;
	return lMskSize;
}


#ifdef ENABLE_DEBUG
#include "liimgfmttrans.h"
MVoid _DrawSeeds(PJOFFSCREEN pImg, PJSEEDS pSeeds)
{
	PJPoint pptSeed = pSeeds->pptSeed;
	MLong x;
	MCOLORREF redCol = PIXELVAL(0, 0, 255);
	
	JOFFSCREEN imgBGR = {0};
	ImgCreate(MNull, &imgBGR, FORMAT_BGR, pImg->dwWidth, pImg->dwHeight);
	ImgFmtTrans(pImg, &imgBGR);

	for (x=0; x<pSeeds->lSeedNum; x++)
	{
		MLong i,j;
		JPoint ptCur = pptSeed[x];
		for(i=-2; i<=2; i++)
		{
			JPoint pt1;
			pt1.y = (MShort)(ptCur.y + i);
			if(pt1.y<0) pt1.y=0;
			if(pt1.y>=(MLong)(pImg->dwHeight))  pt1.y = (MShort)(pImg->dwHeight-1);
			for (j=-2; j<=2; j++)
			{
				pt1.x = (MShort)(ptCur.x + j);
				if (pt1.x < 0)	pt1.x = 0;
				if (pt1.x > (MLong)(pImg->dwWidth-1))	pt1.x = (MShort)(pImg->dwWidth-1);
				ImgSetPixel(&imgBGR, pt1.x, pt1.y, redCol);
			}
		}
	}
	
	PrintBmp(imgBGR.pixelArray.chunky.pPixel, imgBGR.pixelArray.chunky.dwImgLine, DATA_U8, imgBGR.dwWidth, imgBGR.dwHeight, 3);
	ImgRelease(MNull, &imgBGR);
}
MVoid _DrawYUVImg(PJOFFSCREEN pImg)
{
	JOFFSCREEN imgBGR = {0};
	ImgCreate(MNull, &imgBGR, FORMAT_BGR, pImg->dwWidth, pImg->dwHeight);
	ImgFmtTrans(pImg, &imgBGR);
	PrintBmp(imgBGR.pixelArray.chunky.pPixel, imgBGR.pixelArray.chunky.dwImgLine, DATA_U8, imgBGR.dwWidth, imgBGR.dwHeight, 3);
	ImgRelease(MNull, &imgBGR);
}
MVoid _DrawRectToMask(JMASK* pMask, PMRECT pRect)
{
	MLong y;
	if(pRect->left>=pRect->right || pRect->top>=pRect->bottom)
		return;
	SetVectMem(pMask->pData+pMask->lMaskLine*pRect->top+pRect->left, pRect->right-pRect->left, 0, JMaskData);
	SetVectMem(pMask->pData+pMask->lMaskLine*(pRect->bottom-1)+pRect->left, pRect->right-pRect->left, 0, JMaskData);
	for(y=pRect->top; y<pRect->bottom; y++)
	{
		JMaskData *pData = pMask->pData + pMask->lMaskLine*y;
		pData[pRect->left] = pData[pRect->right] = 0;
	}
}
#endif	//ENABLE_DEBUG