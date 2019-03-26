#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "liimgfmttrans.h"

#include "litrimfun.h"
#include "lierrdef.h"
#include "lidebug.h"
#include "liimage.h"

#ifdef TRIM_RGB
#include "lirgb_yuv.h"
#endif
#ifdef TRIM_YCbCr
#include "lirgb_ycbcr.h"
#endif
#ifdef TRIM_LAB
#include "lirgb_lab.h"
#endif
// #ifdef TRIM_RGB565
// #include "lirgb565.h"
// #endif
#ifdef TRIM_BGR32
#include "lirgb32.h"
#endif

#ifndef TRIM_REDUNDANCE
MRESULT ImgFmtTrans(PJOFFSCREEN pImgSrc, PJOFFSCREEN pImgRlt)
{
	JOFFSCREEN imgSrc = *pImgSrc;
	JOFFSCREEN imgRlt = *pImgRlt;
	ImgChunky2Plannar(&imgSrc);
	ImgChunky2Plannar(&imgRlt);
	JASSERT(imgSrc.dwWidth == imgRlt.dwWidth
		&& imgSrc.dwHeight == imgRlt.dwHeight);
	if(imgSrc.fmtImg == imgRlt.fmtImg)
	{
		ImgCpy(pImgSrc, pImgRlt);
		return LI_ERR_NONE;
	}
//	SAVETOBMP(pImgSrc, dwSrcLine, dwWidth, dwHeight, 3);
	switch(imgSrc.fmtImg) {
	case FORMAT_BGR: 
		switch(imgRlt.fmtImg) {
#ifdef TRIM_RGB
		case FORMAT_YUV422PLANAR:
			BGRIMG2YUV422Planar((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine,
				(MUInt8**)imgRlt.pixelArray.planar.pPixel, 
				imgRlt.pixelArray.planar.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
		case FORMAT_YUV420PLANAR:
			BGRIMG2YUV420Planar((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine,
				(MUInt8**)imgRlt.pixelArray.planar.pPixel, 
				imgRlt.pixelArray.planar.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
		case FORMAT_YUV:
			BGRIMG2YUV((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;			
		case FORMAT_UYVY:
			BGRIMG2UYVY((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;	
		case FORMAT_YUYV:
			BGRIMG2YUYV((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;	
		case FORMAT_YYUV:
			BGRIMG2YYUV((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;	
		case FORMAT_Y1VY0U:
			BGRIMG2Y1VY0U((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
		case FORMAT_YUV422_P8:
			BGRIMG2Y8U4V4((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;	
		case FORMAT_YUV420_LP:
			BGRIMG2YUV420LP((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8**)imgRlt.pixelArray.planar.pPixel, 
				imgRlt.pixelArray.planar.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
		case FORMAT_YUV420_VUVU:
			BGRIMG2YUV420VU((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8**)imgRlt.pixelArray.planar.pPixel, 
				imgRlt.pixelArray.planar.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
		case FORMAT_GRAY:
			BGRIMG2Y((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth, imgSrc.dwHeight);
			break;
#ifdef TRIM_DATA_14BITS
		case FORMAT_YUV_U14:
			BGRIMG2YUV_U14((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine,
				(MUInt16*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
#endif //TRIM_DATA_14BITS	
		case FORMAT_RGB:
			BGRIMG2RGB((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth, imgSrc.dwHeight);
			break;
#endif //TRIM_RGB
#ifdef TRIM_YCbCr
		case FORMAT_YCbCr:
			BGRIMG2YCbCr((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)pImgRlt, imgRlt.pixelArray.chunky.dwImgLine,  
				imgSrc.dwWidth,imgSrc.dwHeight);
			break;
#endif
#ifdef TRIM_LAB
		case FORMAT_LAB:
			BGRIMG2LAB((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth, imgSrc.dwHeight);
			break;
#endif
#ifdef TRIM_RGB
		case FORMAT_RGB565:
			BGRIMG2RGB565((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth, imgSrc.dwHeight);
			break;
#endif
#ifdef TRIM_BGR32
		case FORMAT_BGR32:
			BGR24toBGR32((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
				imgSrc.pixelArray.chunky.dwImgLine, 
				(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
				imgRlt.pixelArray.chunky.dwImgLine, 
				imgSrc.dwWidth, imgSrc.dwHeight);
			break;
#endif
		case FORMAT_NONE:
		default:
			return LI_ERR_IMAGE_FORMAT;
		}
		break;		
#ifdef TRIM_RGB
	case FORMAT_YUV:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUVIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine,
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_UYVY:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		UYVYIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_YUYV:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUYVIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_YYUV:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YYUVIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_Y1VY0U:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		Y1VY0UIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;
	case FORMAT_YUV422_P8:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		Y8U4V4IMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_YUV420_LP:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUV420LPIMG2BGR((MUInt8**)imgSrc.pixelArray.planar.pPixel, 
			imgSrc.pixelArray.planar.dwImgLine  , 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_YUV420_VUVU:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUV420VUIMG2BGR((MUInt8**)imgSrc.pixelArray.planar.pPixel, 
			imgSrc.pixelArray.planar.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
#ifdef TRIM_DATA_14BITS
	case FORMAT_YUV_U14:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUV_U14IMG2BGR((MUInt16*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
#endif //TRIM_DATA_14BITS
	case FORMAT_YUV422PLANAR:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUV422PlanarIMG2BGR(((const MUInt8**)imgSrc.pixelArray.planar.pPixel),
			imgSrc.pixelArray.planar.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine,
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_YUV420PLANAR:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YUV420PlanarIMG2BGR(((const MUInt8**)imgSrc.pixelArray.planar.pPixel),
			imgSrc.pixelArray.planar.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine,
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;	
	case FORMAT_RGB:
        switch(imgRlt.fmtImg) {
        case FORMAT_BGR:
            RGBIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
                imgSrc.pixelArray.chunky.dwImgLine, 
                (MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
                imgRlt.pixelArray.chunky.dwImgLine, 
                imgSrc.dwWidth, imgSrc.dwHeight);
            break;  
        case FORMAT_RGB565:
            RGBIMG2RGB565((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
                imgSrc.pixelArray.chunky.dwImgLine, 
                (MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
                imgRlt.pixelArray.chunky.dwImgLine, 
                imgSrc.dwWidth, imgSrc.dwHeight);
            break;
        default:
			return LI_ERR_IMAGE_FORMAT;
        }
		break;

#endif //TRIM_RGB
#ifdef TRIM_YCbCr
	case FORMAT_YCbCr:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		YCbCrIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel, 
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, imgSrc.dwWidth,
			imgSrc.dwHeight);
		break;	
#endif
#ifdef TRIM_LAB
	case FORMAT_LAB:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		LABIMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;
#endif
#ifdef TRIM_RGB
	case FORMAT_RGB565:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		RGB565IMG2BGR((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;
#endif
#ifdef TRIM_BGR32
	case FORMAT_BGR32:
		if(imgRlt.fmtImg != FORMAT_BGR)
			return LI_ERR_IMAGE_FORMAT;
		BGR32toBGR24((MUInt8*)imgSrc.pixelArray.chunky.pPixel,
			imgSrc.pixelArray.chunky.dwImgLine, 
			(MUInt8*)imgRlt.pixelArray.chunky.pPixel, 
			imgRlt.pixelArray.chunky.dwImgLine, 
			imgSrc.dwWidth, imgSrc.dwHeight);
		break;
#endif
	case FORMAT_NONE:
	default:
		JASSERT(MFalse);
		return LI_ERR_IMAGE_FORMAT;
	}
//	SAVETOBMP(imgRlt.pixelArray.chunky.pPixel, dwRltLine, imgSrc.dwWidth, imgSrc.dwHeight, 3);
	return LI_ERR_NONE;
}
#endif//TRIM_REDUNDANCE

