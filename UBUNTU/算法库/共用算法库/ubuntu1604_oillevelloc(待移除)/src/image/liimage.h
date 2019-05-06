#ifndef _LI_IMAGE_
#define _LI_IMAGE_

#include "licomdef.h"
#include "limask.h"

#ifdef __cplusplus
extern "C" {
#endif	

/********************************************************************************/
#define ImgSubtract				PX(ImgSubtract)
#define ImgAdd					PX(ImgAdd)
#define ImgTune					PX(ImgTune)
#define ImgMulti				PX(ImgMulti)
#define ImgCreate				PX(ImgCreate)
#define ImgRelease				PX(ImgRelease)
#define ImgCpy					PX(ImgCpy)

#define ImgSet					PX(ImgSet)
#define ImgOffset				PX(ImgOffset)
#define ImgZoom					PX(ImgZoom)
#define ImgChunky2Plannar		PX(ImgChunky2Plannar)

#define ImgGetPixel				PX(ImgGetPixel)
#define ImgSetPixel				PX(ImgSetPixel)
#define ImgThresh               PX(ImgThresh)

#define ImgAlign                PX(ImgAlign)
/********************************************************************************/

//////////////////////////////////////////////////////////////////////////
MRESULT ImgSubtract(const MVoid *pImg1, MDWord dwImgLine1, JTYPE_DATA_A typeData1,
					const MVoid *pImg2,  MDWord dwImgLine2,JTYPE_DATA_A typeData2,						
					MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt,
					MDWord dwWidth, MDWord dwHeight);
MRESULT ImgAdd(const MVoid *pImg1, MDWord dwImgLine1, JTYPE_DATA_A typeData1,
			   const MVoid *pImg2, MDWord dwImgLine2, JTYPE_DATA_A typeData2,
			   MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt,
			   MDWord dwWidth, MDWord dwHeight);

MRESULT	ImgTune(const MVoid *pImgSrc, MDWord dwSrcLine, JTYPE_DATA typeSrc, 
				MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA typeRlt, 
				MDWord dwWidth, MDWord dwHeight, MLong lOffset, MLong lZoom);

MRESULT ImgMulti(const MVoid *pImgSrc, MDWord dwSrcLine,JTYPE_DATA_A typeDataSrc, 
				 MVoid *pImgRlt, MDWord dwRltLine, JTYPE_DATA_A typeDataRlt, 
				 MDWord dwWidth, MDWord dwHeight, MLong lZoom);

//////////////////////////////////////////////////////////////////////////
MRESULT ImgCreate(MHandle hMemMgr, PJOFFSCREEN pImg, MLong fmtImg, 
				  MLong lWidth, MLong lHeight);
MVoid	ImgRelease(MHandle hMemMgr, PJOFFSCREEN pImg);

MRESULT ImgCpy(const JOFFSCREEN* pImgSrc, PJOFFSCREEN pImgRlt);
MRESULT	ImgSet(PJOFFSCREEN pImg, MLong lVal);
MRESULT ImgOffset(PJOFFSCREEN pImg, MLong lOffsetX, MLong lOffsetY);
MRESULT	ImgZoom(const JOFFSCREEN* pImgSrc, PJOFFSCREEN pImgRlt);

MRESULT ImgChunky2Plannar(PJOFFSCREEN pImage);

MCOLORREF ImgGetPixel(const JOFFSCREEN* pImg, MLong x, MLong y);
MVoid ImgSetPixel(JOFFSCREEN* pImg, MLong x, MLong y, MCOLORREF color);

MLong ImgAlign(JIMAGE_FORMAT fmtImg, MLong xDisp);

//////////////////////////////////////////////////////////////////////////
typedef enum{
    LI_THRESH_BINARY,       // <=Thresh--->0           && >Thresh--->MAX
    LI_THRESH_BINARY_INV,   // <=Thresh--->MAX         && >Thresh--->0
    LI_THRESH_TOZERO,       // <=Thresh--->0           && >Thresh--->unchanged
    LI_THRESH_TOZERO_INV,   // <=Thresh--->unchanged   && >Thresh--->0
    LI_THRESH_TOMAX,        // <=Thresh--->unchanged   && >Thresh--->MAX
    LI_THRESH_TOMAX_INV     // <=Thresh--->MAX         && >Thresh--->unchanged
} JENUM_THRESH_MODEL;
MRESULT ImgThresh(const MVoid *pDataSrc, MDWord dwSrcLine,
                  MVoid *pDataRlt, MDWord dwRltLine,
                  MDWord dwWidth, MDWord dwHeight, MLong lThresh,
                  JENUM_THRESH_MODEL threshModel);
#ifdef __cplusplus
}
#endif

#endif//_LI_IMAGE_

