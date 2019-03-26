#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "liimage.h"
#include "limath.h"
#include "lidebug.h"
#include "lierrdef.h"
#include "limem.h"
#include "litrimfun.h"

#ifdef OPTIMIZATION_ARM
#include "liimage_arm.h"
#endif

#define IMGSUBTRACT(pImg1, dwImgLine1, pImg2, dwImgLine2,					\
	pImgRlt, dwRltLine,	dwWidth, dwHeight, TYPE1, TYPE2, TYPE3, D_TRIM)		\
{																			\
	MLong x,y;																\
	MDWord dwImgExt1 = dwImgLine1 - dwWidth;								\
	MDWord dwImgExt2 = dwImgLine2 - dwWidth;								\
	MDWord dwRltExt  = dwRltLine - dwWidth;									\
	TYPE1 *pImg1Cur = (TYPE1*)pImg1;										\
	TYPE2 *pImg2Cur = (TYPE2*)pImg2;										\
	TYPE3 *pImgRltCur = (TYPE3*)pImgRlt;									\
	for(y=dwHeight; y!=0; y--,	pImg1Cur += dwImgExt1,						\
		pImg2Cur += dwImgExt2, pImgRltCur += dwRltExt)						\
	{																		\
		for(x=dwWidth; x!=0; x--)											\
		{																	\
			MLong lDiff = *(pImg1Cur++);									\
			lDiff -= *(pImg2Cur++); 										\
			*(pImgRltCur++) = D_TRIM(lDiff);								\
		}																	\
	}																		\
}

#define IMGADD(pImg1, dwImgLine1, pImg2, dwImgLine2,						\
	pImgRlt, dwRltLine,	dwWidth, dwHeight, TYPE1, TYPE2, TYPE3, D_TRIM)		\
{																			\
	MLong x,y;																\
	MDWord dwImgExt1 = dwImgLine1 - dwWidth;								\
	MDWord dwImgExt2 = dwImgLine2 - dwWidth;								\
	MDWord dwRltExt  = dwRltLine - dwWidth;									\
	TYPE1 *pImg1Cur = (TYPE1*)pImg1;										\
	TYPE2 *pImg2Cur = (TYPE2*)pImg2;										\
	TYPE3 *pImgRltCur = (TYPE3*)pImgRlt;									\
	for(y=dwHeight; y!=0; y--,	pImg1Cur += dwImgExt1,						\
		pImg2Cur += dwImgExt2, pImgRltCur += dwRltExt)						\
	{																		\
		for(x=dwWidth; x!=0; x--)											\
		{																	\
			MLong lAdd = *pImg1Cur++;										\
			MLong lTmp1 = *pImg2Cur++;										\
			lAdd += lTmp1;													\
			*(pImgRltCur++) = D_TRIM(lAdd);									\
		}																	\
	}																		\
}

#define IMGADD_CON(pImg1, dwImgLine1, add,									\
	pImgRlt, dwRltLine,	dwWidth, dwHeight, TYPE1, TYPE3, D_TRIM)			\
{																			\
	MLong x,y;																\
	MDWord dwImgExt1 = dwImgLine1 - dwWidth;								\
	MDWord dwRltExt  = dwRltLine - dwWidth;									\
	TYPE1 *pImg1Cur = (TYPE1*)pImg1;										\
	TYPE3 *pImgRltCur = (TYPE3*)pImgRlt;									\
	for(y=dwHeight; y!=0; y--,	pImg1Cur += dwImgExt1,						\
		pImgRltCur += dwRltExt)												\
	{																		\
		for(x=dwWidth; x!=0; x--)											\
		{																	\
			MLong lTmp = *(pImg1Cur++)+add;									\
			*(pImgRltCur++) = D_TRIM(lTmp);									\
		}																	\
	}																		\
}


#define IMG_CPY(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth, dwHeight,	\
	dwChannelNum, TYPE)														\
{																			\
	TYPE *_pImgSrc = (TYPE *)(pImgSrc);										\
	TYPE *_pImgRlt = (TYPE *)(pImgRlt);										\
	MDWord _y;																\
	MDWord _dwWidth = (dwChannelNum)*(dwWidth);								\
	for(_y=dwHeight; _y!=0; _y--, _pImgRlt+=dwRltLine, _pImgSrc+=dwSrcLine)	\
	{																		\
		CpyVectMem(_pImgRlt, _pImgSrc, _dwWidth, TYPE);						\
	}																		\
}


#define IMG_TUNE(pImgSrc, dwSrcLine, TYPE_SRC, pImgRlt, dwRltLine, TYPE_RLT,\
	dwWidth, dwHeight, lOffset, lZoom,	TYPE_TRIM)							\
{																			\
	TYPE_SRC *pSrcCur = (TYPE_SRC*)pImgSrc;									\
	TYPE_RLT *pRltCur = (TYPE_RLT*)pImgRlt;									\
	MDWord dwSrcExt = dwSrcLine - dwWidth;									\
	MDWord dwRltExt = dwRltLine - dwWidth;									\
	MDWord x, y;															\
	JASSERT((MLong)dwSrcExt >= 0 && (MLong)dwRltExt >= 0);					\
	for(y=dwHeight; y!=0; y--, pRltCur+=dwRltExt, pSrcCur+=dwSrcExt)		\
	{																		\
		for(x=dwWidth; x!=0; x--, pRltCur++, pSrcCur++)						\
		{																	\
			MLong lTune = ((*pSrcCur+lOffset) * lZoom)>>NORMAL_ZOOM_SHIFT;	\
			*pRltCur = TYPE_TRIM(lTune);									\
		}																	\
	}																		\
}

#define IMG_MULTI(pImgSrc, dwSrcLine, TYPE_SRC,								\
	pImgRlt, dwRltLine, TYPE_RLT, dwWidth, dwHeight, lZoom, TYPE_TRIM)		\
{																			\
	TYPE_SRC *pSrcCur = (TYPE_SRC*)pImgSrc;									\
	TYPE_RLT *pRltCur = (TYPE_RLT*)pImgRlt;									\
	MDWord dwSrcExt = dwSrcLine - dwWidth;									\
	MDWord dwRltExt = dwRltLine - dwWidth;									\
	MDWord x, y;															\
	JASSERT((MLong)dwSrcExt >= 0 && (MLong)dwRltExt >= 0);					\
	for(y=dwHeight; y!=0; y--, pRltCur+=dwRltExt, pSrcCur+=dwSrcExt)		\
	{																		\
		for(x=dwWidth; x!=0; x--, pRltCur++, pSrcCur++)						\
		{																	\
			MLong lTune = ((*pSrcCur) * lZoom)>>NORMAL_ZOOM_SHIFT;			\
			*pRltCur = TYPE_TRIM(lTune);									\
		}																	\
	}																		\
}

//////////////////////////////////////////////////////////////////////////
MRESULT ImgSubtract(const MVoid *pImg1, MDWord dwImgLine1, JTYPE_DATA_A typeData1,
					const MVoid *pImg2,  MDWord dwImgLine2,JTYPE_DATA_A typeData2,						
					MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt,
					MDWord dwWidth, MDWord dwHeight)
{
	if(typeData1 != typeData2)
		return LI_ERR_DATA_UNSUPPORT;
	if(IF_DATA_TYPE(typeDataRlt) == DATA_I8)
	{
		if(IF_DATA_TYPE(typeData1) == DATA_U8)
		{
#ifdef OPTIMIZATION_ARM
			ImgSubtract_U8_U8_I8_ARM((MUInt8*)pImg1, dwImgLine1, (MUInt8*)pImg2, dwImgLine2, 
				(MInt8*)pImgRlt, dwRltLine, dwWidth, dwHeight);
#else
			IMGSUBTRACT(pImg1, dwImgLine1, pImg2, dwImgLine2, pImgRlt, dwRltLine, 
				dwWidth, dwHeight, MUInt8, MUInt8, MInt8, TRIM_INT8);
#endif
			return LI_ERR_NONE;
		}
// 		else
// 		{
// 			JASSERT(IF_DATA_TYPE(typeData1) == DATA_I8);
// 			IMGSUBTRACT(pImg1, dwImgLine1, pImg2, dwImgLine2, pImgRlt, dwRltLine, 
// 				dwWidth, dwHeight, MInt8, MInt8, MInt8, TRIM_INT8/*(MInt8)*/);
// 			return LI_ERR_NONE;
// 		}
	}
	JASSERT(MFalse);
	return LI_ERR_DATA_UNSUPPORT;
}

//////////////////////////////////////////////////////////////////////////
MRESULT ImgAdd(const MVoid *pImg1, MDWord dwImgLine1, JTYPE_DATA_A typeData1,
			   const MVoid *pImg2, MDWord dwImgLine2, JTYPE_DATA_A typeData2,
			   MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt,
			   MDWord dwWidth, MDWord dwHeight)
{
	if(typeData1 != typeDataRlt)
		return LI_ERR_DATA_UNSUPPORT;
	if(IF_DATA_TYPE(typeData2) == DATA_I8)
	{
		if(IF_DATA_TYPE(typeData1) == DATA_U8)
		{
			JASSERT(dwImgLine2 >= dwWidth);
#ifdef OPTIMIZATION_ARM
			ImgAdd_U8_I8_U8_ARM((MUInt8*)pImg1, dwImgLine1, (MInt8*)pImg2, dwImgLine2, 
				(MUInt8*)pImgRlt, dwRltLine, dwWidth, dwHeight);	
#else
			IMGADD(pImg1, dwImgLine1, pImg2, dwImgLine2, pImgRlt, dwRltLine, 
				dwWidth, dwHeight, MUInt8, MInt8, MUInt8, TRIM_UINT8);
#endif
			return LI_ERR_NONE;
		}
// 		else
// 		{
// 			JASSERT(IF_DATA_TYPE(typeData1) == DATA_I8);
// 			IMGADD(pImg1, dwImgLine1, pImg2, dwImgLine2, pImgRlt, dwRltLine, 
// 				dwWidth, dwHeight, MInt8, MInt8, MInt8, TRIM_INT8);
// 			return LI_ERR_NONE;
// 		}
	}
#ifdef TRIM_DATA_16BITS
	if(IF_DATA_TYPE(typeData2) == DATA_I16)
	{
		IMGADD(pImg1, dwImgLine1, pImg2, dwImgLine2, pImgRlt, dwRltLine, 
			dwWidth, dwHeight, MUInt16, MInt16, MUInt16, TRIM_UINT16);
		return LI_ERR_NONE;
	}
#endif	//TRIM_DATA_16BITS
	JASSERT(MFalse);
	return LI_ERR_DATA_UNSUPPORT;
}


#define IMGMULTICONST(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth,		\
	dwHeight, lZoom, D_TYPE, D_TRIM)										\
{																			\
	MDWord x, y;															\
	MDWord dwSrcExt = dwSrcLine - dwWidth, dwRltExt = dwRltLine - dwWidth;	\
	D_TYPE *pSrcCur = (D_TYPE*)pImgSrc, *pRltCur = (D_TYPE*)pImgRlt;		\
	for (y=dwHeight; y!=0; y--, pSrcCur+=dwSrcExt, pRltCur+=dwRltExt)		\
	{																		\
		for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)					\
		{																	\
			MLong ltmp = *pSrcCur*lZoom;									\
			ltmp = DOWN_ROUND(ltmp, NORMAL_ZOOM_SHIFT);						\
			*pRltCur = D_TRIM(ltmp);										\
		}																	\
	}																		\
}
MRESULT	ImgMulti(const MVoid *pImgSrc, MDWord dwSrcLine, JTYPE_DATA_A typeDataSrc, 
					  MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt, 
					  MDWord dwWidth, MDWord dwHeight, MLong lZoom)
{
	if(typeDataSrc != typeDataRlt)
		return LI_ERR_DATA_UNSUPPORT;
	if(lZoom == NORMAL_ZOOM)
		return LI_ERR_NONE;
	switch(typeDataSrc)
	{
	case DATA_U8:
		IMGMULTICONST(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth, dwHeight, 
			lZoom, MUInt8, TRIM_UINT8);
		break;
	case DATA_I8:
#ifdef OPTIMIZATION_ARM
		ImgMultiConst_I8_ARM((MInt8*)pImgSrc, dwSrcLine, (MInt8*)pImgRlt, dwRltLine, 
			dwWidth, dwHeight, lZoom);
#else
		IMGMULTICONST(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth, dwHeight, 
			lZoom, MInt8, TRIM_INT8);
#endif
	    break;
#ifdef TRIM_DATA_14BITS
	case DATA_I14:
		IMGMULTICONST(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth, dwHeight, 
			lZoom, MInt16, TRIM_INT14);
		break;
	case DATA_U14:
		IMGMULTICONST(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth, dwHeight, 
			lZoom, MUInt16, TRIM_UINT14);
		break;
#endif	//TRIM_DATA_14BITS
	default:
		return LI_ERR_DATA_UNSUPPORT;
	}

	return LI_ERR_NONE;
}

#define ADD_IMG(pImgSrc, dwSrcLine, pImgRlt, dwRltLine, dwWidth,		\
	dwHeight, lAdd, D_TYPE, D_TRIM)									\
{																			\
	MDWord x, y;															\
	MDWord dwSrcExt = dwSrcLine - dwWidth, dwRltExt = dwRltLine - dwWidth;	\
	D_TYPE *pSrcCur = (D_TYPE*)pImgSrc, *pRltCur = (D_TYPE*)pImgRlt;\
	for (y=dwHeight; y!=0; y--, pSrcCur+=dwSrcExt, pRltCur+=dwRltExt)		\
	{																		\
		for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)					\
		{																	\
			MLong ltmp = *pSrcCur+lAdd;										\
			*pRltCur = D_TRIM(ltmp);										\
		}																	\
	}																		\
}


#define IMG_MULTI_CONST_CENTER(pImgSrc, dwSrcLine, pImgRlt, dwRltLine,		\
	dwWidth, dwHeight, lZoom, lCenter, D_TYPE, D_TRIM)						\
{																			\
	D_TYPE *pSrcCur = (D_TYPE*)pImgSrc;										\
	D_TYPE *pRltCur = (D_TYPE*)pImgRlt;										\
	MDWord x, y;															\
	MDWord dwSrcExt = dwSrcLine - dwWidth, dwRltExt = dwRltLine - dwWidth;	\
	for (y=dwHeight; y!=0; y--, pSrcCur+=dwSrcExt, pRltCur+=dwRltExt)		\
	{																		\
		for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)					\
		{																	\
			MLong lTmp = (*pSrcCur-(lCenter)) * lZoom;						\
			lTmp = DOWN_ROUND(lTmp, NORMAL_ZOOM_SHIFT) + (lCenter);			\
			*pRltCur = D_TRIM(lTmp);										\
		}																	\
	}																		\
}

MRESULT	ImgTune(const MVoid *pImgSrc, MDWord dwSrcLine, JTYPE_DATA typeSrc, 
				MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA typeRlt, 
				MDWord dwWidth, MDWord dwHeight, MLong lOffset, MLong lZoom)
{
	if(typeSrc == typeRlt && lOffset == 0 && lZoom == NORMAL_ZOOM)
	{
		if(pImgSrc == pImgRlt)
			return LI_ERR_NONE;
		CpyImgMem3(pImgRlt, dwRltLine, pImgSrc, dwSrcLine, dwWidth, dwHeight, 
			typeSrc);
		return LI_ERR_NONE;
	}
	switch(typeSrc) {
	case DATA_I8:
		switch(typeRlt) {
		case DATA_I8:
			IMG_TUNE(pImgSrc, dwSrcLine, MInt8, pImgRlt, dwRltLine, MInt8, 
				dwWidth, dwHeight, lOffset, lZoom, TRIM_INT8);
			break;
// 		case DATA_I16:
// 			IMG_TUNE(pImgSrc, dwSrcLine, MInt8, pImgRlt, dwRltLine, MInt16, 
// 				dwWidth, dwHeight, lOffset, lZoom, (MInt16));
// 			break;
		}
		break;
	case DATA_U8:
		switch(typeRlt) {
		case DATA_U8:
			if(lOffset != 0)
			{
				IMG_TUNE(pImgSrc, dwSrcLine, MUInt8, pImgRlt, dwRltLine, MUInt8, 
					dwWidth, dwHeight, lOffset, lZoom, TRIM_UINT8);
			}
			else
			{
				IMG_MULTI(pImgSrc, dwSrcLine, MUInt8, pImgRlt, dwRltLine, MUInt8, 
					dwWidth, dwHeight, lZoom, TRIM_UINT8);
			}
			break;
// 		case DATA_U16:
// 			IMG_TUNE(pImgSrc, dwSrcLine, MUInt8, pImgRlt, dwRltLine, MUInt16, 
// 				dwWidth, dwHeight, lOffset, lZoom, (MUInt16));
// 			break;
// 		case DATA_I16:
// 			IMG_TUNE(pImgSrc, dwSrcLine, MUInt8, pImgRlt, dwRltLine, MInt16, 
// 				dwWidth, dwHeight, lOffset, lZoom, (MInt16));
// 			break;
		}
		break;
#ifdef TRIM_DATA_16BITS
	case DATA_I16:
		switch(typeRlt) {
		case DATA_I8:
			IMG_TUNE(pImgSrc, dwSrcLine, MInt16, pImgRlt, dwRltLine, MInt8, 
				dwWidth, dwHeight, lOffset, lZoom, (MInt8));
			break;
		case DATA_I16:
			IMG_TUNE(pImgSrc, dwSrcLine, MInt16, pImgRlt, dwRltLine, MInt16, 
				dwWidth, dwHeight, lOffset, lZoom, (MInt16));
			break;
		}
		break;
	case DATA_U16:
		switch(typeRlt) {
		case DATA_U8:
			IMG_TUNE(pImgSrc, dwSrcLine, MUInt16, pImgRlt, dwRltLine, MUInt8, 
				dwWidth, dwHeight, lOffset, lZoom, (MUInt8));
			break;
		case DATA_U16:
			IMG_TUNE(pImgSrc, dwSrcLine, MUInt16, pImgRlt, dwRltLine, MUInt16, 
				dwWidth, dwHeight, lOffset, lZoom, (MUInt16));
			break;
		case DATA_I16:
			IMG_TUNE(pImgSrc, dwSrcLine, MUInt16, pImgRlt, dwRltLine, MInt16, 
				dwWidth, dwHeight, lOffset, lZoom, (MInt16));
			break;
		}
		break;
#endif //TRIM_DATA_16BITS
	}

	return LI_ERR_NONE;
//	PrintBmp(pImgRlt, dwRltLine, typeRlt, dwWidth, dwHeight, 1);
}
MRESULT ImgCpy(const JOFFSCREEN* pImgSrc, PJOFFSCREEN pImgRlt)
{
	JOFFSCREEN imgSrc = *pImgSrc, imgRlt = *pImgRlt;
	if(pImgRlt == pImgSrc)
		return LI_ERR_NONE;	
	if(imgRlt.fmtImg != imgSrc.fmtImg)
		return LI_ERR_IMAGE_FORMAT;

	ImgChunky2Plannar(&imgSrc);
	ImgChunky2Plannar(&imgRlt);

	if(IF_IS_PLANAR_FORMAT(imgSrc.fmtImg))
	{
		MLong i = 0; 
		MLong lColorW = imgSrc.dwWidth, lColorH = imgSrc.dwHeight;
		MLong lChannelNum = IF_CHANNEL_NUM(imgSrc.fmtImg);
		switch (IF_LU_CR_RADIO(imgSrc.fmtImg))	{
		case FORMAT_420:
			if(IF_LU_CR_EXT(imgSrc.fmtImg) == FORMAT_411_L_C1_C2)
				lColorH /= 2, lColorW /= 2;
			else
				lChannelNum = 2, lColorH /= 2;
			break;
		case FORMAT_422:
			lColorW /= 2;
			break;
		}
		for(i = 0; i < lChannelNum; i ++)
		{
			MLong lWidth = (i==0 ? imgSrc.dwWidth : lColorW);
			MLong lHeight = (i==0 ? imgSrc.dwHeight : lColorH);
			CpyImgMem2(imgRlt.pixelArray.planar.pPixel[i], 
				imgRlt.pixelArray.planar.dwImgLine[i]*IF_DATA_BYTES(imgSrc.fmtImg),				
				imgSrc.pixelArray.planar.pPixel[i], 
				imgSrc.pixelArray.planar.dwImgLine[i]*IF_DATA_BYTES(imgSrc.fmtImg), 				
				lWidth*IF_DATA_BYTES(imgSrc.fmtImg),lHeight);							
		}
	}
	else
	{		
		MLong dwWidth = imgSrc.dwWidth, dwHeight = imgSrc.dwHeight;
		JASSERT(IF_CHANNEL_NUM(imgSrc.fmtImg)==1 || IF_CHANNEL_NUM(imgSrc.fmtImg)==3);
		if(IF_CHANNEL_NUM(imgSrc.fmtImg) == 3)
		{			
			switch(IF_LU_CR_EXT(imgSrc.fmtImg)) {
			case FORMAT_888_LC1C2:
				dwWidth *= 3;
				break;			
			case FORMAT_211_C1LC2L:
			case FORMAT_211_LC1LC2:
			case FORMAT_211_LLC1C2:
			case FORMAT_211_L2C2L1C1:
			case FORMAT_844:
			case FORMAT_565:				
				dwWidth *= 2;
				break;
			default:
				JASSERT(MFalse);
				return LI_ERR_IMAGE_FORMAT;
				break;
			}	
		}
		CpyImgMem2(imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine*IF_DATA_BYTES(imgSrc.fmtImg), 			
			imgSrc.pixelArray.chunky.pPixel, 
			imgSrc.pixelArray.chunky.dwImgLine*IF_DATA_BYTES(imgSrc.fmtImg), 			
			dwWidth*IF_DATA_BYTES(imgSrc.fmtImg), dwHeight);	
	}
	return LI_ERR_NONE;
}

MRESULT ImgCreate(MHandle hMemMgr, 
				  PJOFFSCREEN pImg, MLong fmtImg, 
				  MLong lWidth, MLong lHeight)
{
	MRESULT res = LI_ERR_NONE;
	pImg->dwWidth = lWidth;
	pImg->dwHeight = lHeight;
	pImg->fmtImg = fmtImg;
	
	if(IF_IS_PLANAR_FORMAT(fmtImg))
	{
		MLong lColorH = lHeight, lColorW = lWidth;
		MLong lChannelNum = IF_CHANNEL_NUM(fmtImg);
		MLong i, lSize = 0;
		switch(IF_LU_CR_RADIO(fmtImg)) {
		case FORMAT_420:
			if(IF_LU_CR_EXT(fmtImg) == FORMAT_411_L_C1_C2)
				lColorH /= 2, lColorW /= 2;
			else
				lChannelNum = 2, lColorH /= 2;
			break;
		case FORMAT_422:
			lColorW /= 2;
			break;
		}	
		
		lSize = JMemLength(lWidth)*lHeight;		
		for(i=1; i<lChannelNum; i++)
			lSize += JMemLength(lColorW)*lColorH;
		pImg->pixelArray.chunky.dwImgLine = JMemLength(lWidth);
		AllocVectMem(hMemMgr, pImg->pixelArray.chunky.pPixel, lSize, MByte);
	}
	else
	{
		switch(IF_LU_CR_EXT(fmtImg)) {
		case FORMAT_888_LC1C2:
			pImg->pixelArray.chunky.dwImgLine = JMemLength(lWidth*IF_CHANNEL_NUM(fmtImg));
			AllocVectMem(hMemMgr, pImg->pixelArray.chunky.pPixel, 
				pImg->pixelArray.chunky.dwImgLine*lHeight*IF_DATA_BYTES(fmtImg), MByte);
			break;
		case FORMAT_211_C1LC2L:
		case FORMAT_211_LC1LC2:
		case FORMAT_211_L2C2L1C1:
		case FORMAT_844:
		case FORMAT_211_LLC1C2:
			pImg->pixelArray.chunky.dwImgLine = JMemLength(lWidth*2);
			AllocVectMem(hMemMgr, pImg->pixelArray.chunky.pPixel, 
				pImg->pixelArray.chunky.dwImgLine*lHeight*IF_DATA_BYTES(fmtImg), MByte);
			break;
		case FORMAT_565:
			pImg->pixelArray.chunky.dwImgLine = JMemLength(lWidth*2);
			AllocVectMem(hMemMgr, pImg->pixelArray.chunky.pPixel,
				pImg->pixelArray.chunky.dwImgLine * lHeight, MByte);
			break;
		default:
			JASSERT(MFalse);
			res = LI_ERR_IMAGE_FORMAT;
			break;
		}		
	}
EXT:
	return res;
}
MVoid ImgRelease(MHandle hMemMgr, PJOFFSCREEN pImg)
{
	if(pImg == MNull)
		return;
	FreeVectMem(hMemMgr, pImg->pixelArray.chunky.pPixel);
}

MRESULT ImgOffset(PJOFFSCREEN pImg, MLong lOffsetX, MLong lOffsetY)
{
	JOFFSCREEN img = *pImg;
	JASSERT(IF_DATA_BYTES(img.fmtImg) == 1);

    if (img.fmtImg == FORMAT_GRAY)
    {
        img.pixelArray.chunky.pPixel = OffSetImg(
            img.pixelArray.chunky.pPixel, lOffsetX, lOffsetY, 
            img.pixelArray.chunky.dwImgLine, IF_DATA_TYPE(img.fmtImg));
        *pImg = img;
        return LI_ERR_NONE;
    }

	ImgChunky2Plannar(&img);
	if(IF_IS_PLANAR_FORMAT(img.fmtImg))
	{
		MLong i = 0; 
		MLong lColorW = img.dwWidth, lColorH = img.dwHeight;
		MLong lColorOffX = lOffsetX, lColorOffY = lOffsetY;
		MLong lChannelNum = IF_CHANNEL_NUM(img.fmtImg);
		switch (IF_LU_CR_RADIO(img.fmtImg))	{
		case FORMAT_420:
			JASSERT(lOffsetY%2==0 && lOffsetX%2==0);
			if(IF_LU_CR_EXT(img.fmtImg) == FORMAT_411_L_C1_C2)
				lColorW /= 2, lColorOffX/= 2;
			else
				lChannelNum = 2;
			lColorH /= 2, lColorOffY /= 2;
			break;
		case FORMAT_422:
			lColorW /= 2, lColorOffX/=2;
			break;
		}
		for(i = 0; i < lChannelNum; i ++)
		{
			if(i!=0)	
				lOffsetX = lColorOffX, lOffsetY = lColorOffY;
			img.pixelArray.planar.pPixel[i] = OffSetImg(
				img.pixelArray.planar.pPixel[i], lOffsetX, lOffsetY, 
				img.pixelArray.planar.dwImgLine[i], IF_DATA_TYPE(img.fmtImg));
		}
	}
	else
	{		
		MLong dwWidth = img.dwWidth;
		if(IF_CHANNEL_NUM(img.fmtImg) == 3)
		{			
			switch(IF_LU_CR_RADIO(img.fmtImg)) {
			case FORMAT_444:
				if(IF_LU_CR_EXT(img.fmtImg) == FORMAT_565)
				{
					dwWidth *= 2;
					lOffsetX *= 2;
				}
				else
				{
					dwWidth *= 3;
					lOffsetX *= 3;
				}
				break;			
			case FORMAT_422:	
				dwWidth *= 2;
				lOffsetX *= 2;
				break;
			default:
				JASSERT(MFalse);
				break;
			}	
		}
		img.pixelArray.chunky.pPixel = OffSetImg(
			img.pixelArray.chunky.pPixel, lOffsetX, lOffsetY, 
			img.pixelArray.chunky.dwImgLine, IF_DATA_TYPE(img.fmtImg));
	}
	
	*pImg = img;
	return LI_ERR_NONE;
}

MRESULT	ImgSet(PJOFFSCREEN pImg, MLong lVal)
{
	JASSERT(IF_DATA_BYTES(pImg->fmtImg) == 1);
	if(IF_IS_PLANAR_FORMAT(pImg->fmtImg))
	{
		MLong i = 0; 
		MLong lColorW = pImg->dwWidth, lColorH = pImg->dwHeight;
		MLong lChannelNum = IF_CHANNEL_NUM(pImg->fmtImg);
		JOFFSCREEN img = *pImg;
		ImgChunky2Plannar(&img);
		switch (IF_LU_CR_RADIO(pImg->fmtImg))	{
		case FORMAT_420:
			if(IF_LU_CR_EXT(pImg->fmtImg) == FORMAT_411_L_C1_C2)
				lColorH /= 2, lColorW /= 2;
			else
				lChannelNum = 2, lColorH /= 2;
			break;
		case FORMAT_422:
			lColorW /= 2;
			break;
		}
		for(i = 0; i < lChannelNum; i ++)
		{
			MLong lHeight = (i==0 ? img.dwHeight : lColorH);
			JMemSet(img.pixelArray.planar.pPixel[i], (MByte)lVal, 
				img.pixelArray.planar.dwImgLine[i]*lHeight);
		}
	}
	else
	{		
		MLong dwWidth = pImg->dwWidth, dwHeight = pImg->dwHeight;
		if(IF_CHANNEL_NUM(pImg->fmtImg) == 3)
		{			
			switch(IF_LU_CR_EXT(pImg->fmtImg)) {
			case FORMAT_888_LC1C2:
				dwWidth *= 3;
				break;			
			case FORMAT_211_C1LC2L:
			case FORMAT_211_LC1LC2:
			case FORMAT_211_L2C2L1C1:
			case FORMAT_844:
			case FORMAT_211_LLC1C2:
			case FORMAT_565:				
				dwWidth *= 2;
				break;
			case FORMAT_411_L_C1_C2:
				dwHeight = (dwHeight*3)/2;
				break;
			default:
				JASSERT(MFalse);
				break;
			}	
		}
		JMemSet(pImg->pixelArray.chunky.pPixel, (MByte)lVal, 
			pImg->pixelArray.chunky.dwImgLine*dwHeight);
	}

	return LI_ERR_NONE;
}

MRESULT ImgChunky2Plannar(PJOFFSCREEN pImage)
{
	if(!IF_IS_PLANAR_FORMAT(pImage->fmtImg))
		return LI_ERR_NONE;
	if(pImage->pixelArray.planar.pPixel[0] != MNull)
		return LI_ERR_NONE;
	if(IF_CHANNEL_NUM(pImage->fmtImg) == 1)
	{
		pImage->pixelArray.planar.pPixel[0] = pImage->pixelArray.chunky.pPixel;
		pImage->pixelArray.planar.dwImgLine[0] = pImage->pixelArray.chunky.dwImgLine;
		return LI_ERR_NONE;
	}

	switch(IF_LU_CR_EXT(pImage->fmtImg))
	{
	case FORMAT_411_L_C1C2:
	case FORMAT_411_L_C2C1:
		pImage->pixelArray.planar.pPixel[0] = pImage->pixelArray.chunky.pPixel;
		pImage->pixelArray.planar.pPixel[1] = OffSetImg(pImage->pixelArray.chunky.pPixel, 
			0, pImage->dwHeight, pImage->pixelArray.chunky.dwImgLine, IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.dwImgLine[0] = pImage->pixelArray.planar.dwImgLine[1] 
			= pImage->pixelArray.chunky.dwImgLine;
		return LI_ERR_NONE;
	case FORMAT_422:
		pImage->pixelArray.planar.pPixel[0] = pImage->pixelArray.chunky.pPixel;
		pImage->pixelArray.planar.pPixel[1] = OffSetImg(pImage->pixelArray.planar.pPixel[0], 
			0, pImage->dwHeight, JMemLength(pImage->dwWidth), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.pPixel[2] = OffSetImg(pImage->pixelArray.planar.pPixel[1], 
			0, pImage->dwHeight, JMemLength(pImage->dwWidth/2), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.dwImgLine[0] = JMemLength(pImage->dwWidth);
		pImage->pixelArray.planar.dwImgLine[1] = JMemLength(pImage->dwWidth/2);
		pImage->pixelArray.planar.dwImgLine[2] = JMemLength(pImage->dwWidth/2);
		return LI_ERR_NONE;
	case FORMAT_420:
		pImage->pixelArray.planar.pPixel[0] = pImage->pixelArray.chunky.pPixel;
		pImage->pixelArray.planar.pPixel[1] = OffSetImg(pImage->pixelArray.planar.pPixel[0], 
			0, pImage->dwHeight, JMemLength(pImage->dwWidth), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.pPixel[2] = OffSetImg(pImage->pixelArray.planar.pPixel[1], 
			0, pImage->dwHeight/2, JMemLength(pImage->dwWidth/2), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.dwImgLine[0] = JMemLength(pImage->dwWidth);
		pImage->pixelArray.planar.dwImgLine[1] = JMemLength(pImage->dwWidth/2);
		pImage->pixelArray.planar.dwImgLine[2] = JMemLength(pImage->dwWidth/2);
		return LI_ERR_NONE;
	case FORMAT_444:
		pImage->pixelArray.planar.pPixel[0] = pImage->pixelArray.chunky.pPixel;
		pImage->pixelArray.planar.pPixel[1] = OffSetImg(pImage->pixelArray.planar.pPixel[0], 
			0, pImage->dwHeight, JMemLength(pImage->dwWidth), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.pPixel[2] = OffSetImg(pImage->pixelArray.planar.pPixel[1], 
			0, pImage->dwHeight, JMemLength(pImage->dwWidth), IF_DATA_TYPE(pImage->fmtImg));
		pImage->pixelArray.planar.dwImgLine[0] = JMemLength(pImage->dwWidth);
		pImage->pixelArray.planar.dwImgLine[1] = JMemLength(pImage->dwWidth);
		pImage->pixelArray.planar.dwImgLine[2] = JMemLength(pImage->dwWidth);
		return LI_ERR_NONE;
	default:
		return LI_ERR_IMAGE_FORMAT;
	}
}


// JSTATIC MVoid ImgZoom_U8(const MByte *pImgSrc, MLong lSrcLine, 
// 						 MByte *pImgRlt, MLong lRltLine, 
// 						 MLong lWidth, MLong lHeight, 
// 						 MLong lZoom, MLong lChannelNum);
// MRESULT	ImgZoom(const JOFFSCREEN* pImgSrc, PJOFFSCREEN pImgRlt)
// {
// 	MLong lZoom = pImgSrc->dwWidth / pImgRlt->dwWidth;
// 	JIMAGE_FORMAT fmtImg = pImgSrc->fmtImg;
// 	if(lZoom - pImgSrc->dwHeight / pImgRlt->dwHeight != 0)
// 		return LI_ERR_IMAGE_SIZE_INVALID;
// 	if(pImgRlt->fmtImg != fmtImg)
// 		return LI_ERR_IMAGE_FORMAT;
// 	if(IF_IS_PLANAR_FORMAT(fmtImg))
// 	{
// 		return LI_ERR_IMAGE_FORMAT;
// 	}
// 	else
// 	{
// 		MVoid *pDataSrc = pImgSrc->pixelArray.chunky.pPixel;
// 		MVoid *pDataRlt = pImgRlt->pixelArray.chunky.pPixel;
// 		MLong lSrcLine = pImgSrc->pixelArray.chunky.dwImgLine;
// 		MLong lRltLine = pImgRlt->pixelArray.chunky.dwImgLine;
// 		switch(IF_LU_CR_EXT(fmtImg)) 
// 		{
// 		case FORMAT_888_LC1C2:
// 			ImgZoom_U8((MByte*)pDataSrc, lSrcLine, (MByte*)pDataRlt, lRltLine, 
// 				pImgSrc->dwWidth, pImgSrc->dwHeight, lZoom, 3);
// 			break;
// 		default:
// 			JASSERT(MFalse);
// 			break;
// 		}
// 	}
// 
// 	return LI_ERR_NONE;
// }
// 
// MVoid ImgZoom_U8(const MByte *pImgSrc, MLong lSrcLine, 
// 				 MByte *pImgRlt, MLong lRltLine, 
// 				 MLong lWidth, MLong lHeight, 
// 				 MLong lZoom, MLong lChannelNum)
// {
// 	MLong x, y;
// 	MLong lWidth2 = lWidth/lZoom, lHeight2 = lHeight/lZoom;
// 	MLong lZoom2 = lZoom/2;
// 	MLong lSrcExt = lZoom*lSrcLine - lWidth2*lZoom*lChannelNum;
// 	MLong lRltExt = lRltLine - lWidth2*lChannelNum;
// 	pImgSrc += lZoom2*(lSrcLine+lChannelNum);
// 	for(y=lHeight2; y!=0; y--, pImgSrc+=lSrcExt, pImgRlt+=lRltExt)
// 	{
// 		for(x=lWidth2; x!=0; x--, pImgSrc+=lChannelNum*lZoom, pImgRlt+=lChannelNum)
// 		{
// 			MLong i;
// 			for(i=0; i<lChannelNum; i++)
// 				pImgRlt[i] = pImgSrc[i];
// 		}
// 	}
// }

MCOLORREF ImgGetPixel(const JOFFSCREEN* pImg, MLong x, MLong y)
{
	MByte *pTmp1, *pTmp2, *pTmp3;
	MLong lImgLine1, lImgLine2, lImgLine3;
    MDWord cr_tmp;
	JOFFSCREEN img = *pImg;
	ImgChunky2Plannar(&img);
	
	switch(img.fmtImg)
	{
	case FORMAT_YUYV:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		return PIXELVAL(pTmp1[x*2], pTmp1[(x/2)*4+1], pTmp1[(x/2)*4+3]);
		
	case FORMAT_UYVY:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		return PIXELVAL(pTmp1[x*2+1], pTmp1[(x/2)*4], pTmp1[(x/2)*4+2]);

	case FORMAT_YYUV:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		return PIXELVAL(pTmp1[(x/2)*4+x%2], pTmp1[(x/2)*4+2], pTmp1[(x/2)*4+3]);

	case FORMAT_Y1VY0U:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		return PIXELVAL(pTmp1[(x/2)*4+2-(x%2)*2], pTmp1[(x/2)*4+3], pTmp1[(x/2)*4+1]);
		
	case FORMAT_YUV:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y;
		return PIXELVAL(pTmp1[x*3], pTmp1[x*3+1], pTmp1[x*3+2]);
		
	case FORMAT_YUV444PLANAR:
	case FORMAT_LAB_PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*y;
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*y;
		return PIXELVAL(pTmp1[x], pTmp2[x], pTmp3[x]);
		
	case FORMAT_YUV420PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*(y/2);
		return PIXELVAL(pTmp1[x], pTmp2[x/2], pTmp3[x/2]);
		
	case FORMAT_YUV422PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*y;
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*y;
		return PIXELVAL(pTmp1[x], pTmp2[x/2], pTmp3[x/2]);
		
	case FORMAT_YUV420_LP:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		return PIXELVAL(pTmp1[x], pTmp2[(x/2)*2], pTmp2[(x/2)*2+1]);
		
	case FORMAT_YUV420_VUVU:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		return PIXELVAL(pTmp1[x], pTmp2[(x/2)*2+1], pTmp2[(x/2)*2]);
		
	case FORMAT_YUV422_P8:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y;
		return PIXELVAL(pTmp1[x+(x/8)*8], pTmp1[(x-x/8*8)/2+x/8*16+8], pTmp1[(x-x/8*8)/2+x/8*16+12]);

	case FORMAT_BGR:
	case FORMAT_HSV:
    case FORMAT_LAB:
    case FORMAT_RGB:
		lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y;
		return PIXELVAL(pTmp1[x*3], pTmp1[x*3+1], pTmp1[x*3+2]);
    case FORMAT_RGB565:
		lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y + x*2;
        cr_tmp = *((MWord*)pTmp1);
		return PIXELVAL(((cr_tmp >> 11) & 0x001F) << 3,
            ((cr_tmp >> 5) & 0x003F) << 2, (cr_tmp & 0x001F) << 3);

    case FORMAT_GRAY:
        lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
        pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y;
        return PIXELVAL(pTmp1[x], 0, 0);
		
	default:
		JASSERT(MFalse);
		break;
	}
	return 0;
}
MVoid ImgSetPixel(JOFFSCREEN* pImg, MLong x, MLong y, MCOLORREF color)
{
	MByte *pTmp1, *pTmp2, *pTmp3;
	MLong lImgLine1, lImgLine2, lImgLine3;
	MLong cr1 = PIXELVAL_1(color),
		cr2 = PIXELVAL_2(color),
		cr3 = PIXELVAL_3(color);
	JOFFSCREEN img = *pImg;
	ImgChunky2Plannar(&img);
	
	switch(img.fmtImg)
	{
	case FORMAT_YUYV:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		pTmp1[x*2] = (MByte)(cr1);
		pTmp1[(x/2)*4+1] = (MByte)(cr2);
		pTmp1[(x/2)*4+3] = (MByte)(cr3);
		break;
		
	case FORMAT_UYVY:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		pTmp1[x*2+1] = (MByte)(cr1);
		pTmp1[(x/2)*4] = (MByte)(cr2);
		pTmp1[(x/2)*4+2] = (MByte)(cr3);
		break;
	case FORMAT_Y1VY0U:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y; 
		pTmp1[(x/2)*4+2-(x%2)*2] = (MByte)(cr1);
		pTmp1[(x/2)*4+3] = (MByte)(cr2);
		pTmp1[(x/2)*4+1] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y;
		pTmp1[x*3] = (MByte)(cr1);
		pTmp1[x*3+1] = (MByte)(cr2);
		pTmp1[x*3+2] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV444PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*y;
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*y;
		pTmp1[x] = (MByte)(cr1);
		pTmp2[x] = (MByte)(cr2);
		pTmp3[x] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV420PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*(y/2);
		pTmp1[x] = (MByte)(cr1);
		pTmp2[x/2] = (MByte)(cr2);
		pTmp3[x/2] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV422PLANAR:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		lImgLine3 = img.pixelArray.planar.dwImgLine[2];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*y;
		pTmp3 = (MByte*)img.pixelArray.planar.pPixel[2] + lImgLine3*y;
		pTmp1[x] = (MByte)(cr1);
		pTmp2[x/2] = (MByte)(cr2);
		pTmp3[x/2] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV420_LP:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		pTmp1[x] = (MByte)(cr1);
		pTmp2[(x/2)*2] = (MByte)(cr2);
		pTmp2[(x/2)*2+1] = (MByte)(cr3);
		break;
		
	case FORMAT_YUV420_VUVU:
		lImgLine1 = img.pixelArray.planar.dwImgLine[0];
		lImgLine2 = img.pixelArray.planar.dwImgLine[1];
		pTmp1 = (MByte*)img.pixelArray.planar.pPixel[0] + lImgLine1*y;
		pTmp2 = (MByte*)img.pixelArray.planar.pPixel[1] + lImgLine2*(y/2);
		pTmp1[x] = (MByte)(cr1);
		pTmp2[(x/2)*2+1] = (MByte)(cr2);
		pTmp2[(x/2)*2] = (MByte)(cr3);
		break;

	case FORMAT_YUV422_P8:
		lImgLine1 = img.pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)img.pixelArray.chunky.pPixel + lImgLine1*y;
		pTmp1[x+(x/8)*8] = (MByte)cr1;
		pTmp1[(x-x/8*8)/2+x/8*16+8] = (MByte)cr2;
		pTmp1[(x-x/8*8)/2+x/8*16+12] = (MByte)cr3;
		break;

	case FORMAT_BGR:
	case FORMAT_HSV:
    case FORMAT_LAB:
    case FORMAT_RGB:
		lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y;
		pTmp1[x*3] = TRIM_UINT8(cr1);
		pTmp1[x*3+1] = TRIM_UINT8(cr2);
		pTmp1[x*3+2] = TRIM_UINT8(cr3);
		break;	
    case FORMAT_RGB565:
		lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
		pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y + x*2;
        ((MWord*)pTmp1)[0] = (MWord)(((cr1>>3)<<11) | ((cr2>>2)<<5) | (cr3>>3));
        break;
          
    case FORMAT_GRAY:
        lImgLine1 = pImg->pixelArray.chunky.dwImgLine;
        pTmp1 = (MByte*)pImg->pixelArray.chunky.pPixel + lImgLine1*y;
        pTmp1[x] = (MByte)cr1;
        break;
		
	default:
		JASSERT(MFalse);
		break;
	}
}
MRESULT ImgThresh(const MVoid *pDataSrc, MDWord dwSrcLine,
                  MVoid *pDataRlt, MDWord dwRltLine,
                  MDWord dwWidth, MDWord dwHeight, MLong lThresh,
                  JENUM_THRESH_MODEL threshModel)
{
    MRESULT res = LI_ERR_NONE;
    MDWord x, y;

    const MByte THRESH_MIN = 0;
    const MByte THRESH_MAX = 255;

    MByte *pSrcCur = (MByte*)pDataSrc;
    MByte *pRltCur = (MByte*)pDataRlt;
    MDWord dwExtSrc = dwSrcLine - dwWidth;
    MDWord dwExtRlt = dwRltLine - dwWidth;

    switch(threshModel)
    {
    default:
    case LI_THRESH_BINARY:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?THRESH_MIN:THRESH_MAX); 
        break;
    case LI_THRESH_BINARY_INV:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?THRESH_MAX:THRESH_MIN); 
        break;
    case LI_THRESH_TOZERO:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?THRESH_MIN:*pSrcCur); 
        break;
    case LI_THRESH_TOZERO_INV:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?*pSrcCur:THRESH_MIN); 
        break;
    case LI_THRESH_TOMAX:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?*pSrcCur:THRESH_MAX); 
        break;
    case LI_THRESH_TOMAX_INV:
        for (y=dwHeight; y!=0; y--, pSrcCur+=dwExtSrc, pRltCur+=dwExtRlt)
            for (x=dwWidth; x!=0; x--, pSrcCur++, pRltCur++)
                *pRltCur = (MByte)((*pSrcCur<=lThresh)?THRESH_MAX:*pSrcCur); 
        break;
    }
    return res;
}
MRESULT	ImgZoom(const JOFFSCREEN* pImgSrc, PJOFFSCREEN pImgRlt)
{
	MLong lRltWidth = pImgRlt->dwWidth,
        lRltHeight = pImgRlt->dwHeight,
        lRltLine = pImgRlt->pixelArray.chunky.dwImgLine,
	    lSrcWidth = pImgSrc->dwWidth,
        lSrcHeight = pImgSrc->dwHeight,
        lSrcLine = pImgSrc->pixelArray.chunky.dwImgLine;
    MByte *pSrc = (MByte*)pImgSrc->pixelArray.chunky.pPixel,
        *pRlt = (MByte*)pImgRlt->pixelArray.chunky.pPixel;
	MLong x, y, i, j, ix;
    MLong nBitCount = IF_CHANNEL_NUM(pImgSrc->fmtImg);

    //Currently only support: FORMAT_GRAY, FORMAT_BGR, FORMAT_RGB, FORMAT_YUV
    JASSERT(IF_LU_CR_RADIO(pImgSrc->fmtImg) == 0);

	if (lRltHeight==lSrcHeight && lRltWidth==lSrcWidth)
	{
        ImgCpy(pImgSrc, pImgRlt);
	}
    else
	{
		MLong lRltExt;
		MLong lXDelta = (lSrcWidth<<16)/lRltWidth;
		MLong lYDelta = (lSrcHeight<<16)/lRltHeight;
		MByte *pTmpSrc, *pTmpRlt;

        y = 0;
		pTmpSrc = (MByte*)pSrc, pTmpRlt = (MByte*)pRlt;
		lRltExt = lRltLine - lRltWidth * nBitCount;

		for (j=0; j<lRltHeight; j++, pTmpRlt+=lRltExt, y+=lYDelta)
		{
			pTmpSrc = (MByte*)pSrc + (y>>16)*lSrcLine;
			for (x=0, i=0; i<lRltWidth; i++, pTmpRlt+=nBitCount, x+=lXDelta)
			{
                for (ix = 0; ix < nBitCount; ix++)
					*(pTmpRlt+ix) = pTmpSrc[(x>>16)*nBitCount + ix];
			}
		}		
	}
	return LI_ERR_NONE;
}
MLong ImgAlign(JIMAGE_FORMAT fmtImg, MLong xDisp)
{
	MLong lAligned = 1;	
    if (xDisp < 0)
        return 0;
	if(fmtImg == FORMAT_YUV || IF_IS_PLANAR_FORMAT(fmtImg))
		lAligned = 2;
	else if(fmtImg == FORMAT_YUV422_P8)
		lAligned = 3;
    return CEIL_AD(xDisp, lAligned);
}

