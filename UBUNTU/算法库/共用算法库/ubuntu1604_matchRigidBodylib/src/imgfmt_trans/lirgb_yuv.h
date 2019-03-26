#if !defined(_LI_YUV_H_)
#define _LI_YUV_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
#define RGB2YUV			PX(RGB2YUV)
#define YUV2RGB			PX(YUV2RGB)

#define BGRIMG2YUV		PX(BGRIMG2YUV)
#define YUVIMG2BGR		PX(YUVIMG2BGR)

#define BGRIMG2YUV_U14	PX(BGRIMG2YUV_U14)
#define YUV_U14IMG2BGR	PX(YUV_U14IMG2BGR)

#define RGB2Y			PX(RGB2Y)
#define BGRIMG2Y		PX(BGRIMG2Y)

#define BGRIMG2YUV420LP	PX(BGRIMG2YUV420LP)
#define YUV420LPIMG2BGR	PX(YUV420LPIMG2BGR)
#define BGRIMG2YUV420VU	PX(BGRIMG2YUV420VU)
#define YUV420VUIMG2BGR	PX(YUV420VUIMG2BGR)

#define BGRIMG2UYVY		PX(BGRIMG2UYVY)
#define UYVYIMG2BGR		PX(UYVYIMG2BGR)

#define BGRIMG2YUYV		PX(BGRIMG2YUYV)
#define YUYVIMG2BGR		PX(YUYVIMG2BGR)

#define BGRIMG2Y8U4V4	PX(BGRIMG2Y8U4V4)
#define Y8U4V4IMG2BGR	PX(Y8U4V4IMG2BGR)

#define BGRIMG2YYUV		PX(BGRIMG2YYUV)
#define YYUVIMG2BGR		PX(YYUVIMG2BGR)

#define BGRIMG2Y1VY0U	PX(BGRIMG2Y1VY0U)
#define Y1VY0UIMG2BGR	PX(Y1VY0UIMG2BGR)

#define BGRIMG2YUV422Planar	PX(BGRIMG2YUV422Planar)
#define YUV422PlanarIMG2BGR	PX(YUV422PlanarIMG2BGR)

#define BGRIMG2YUV420Planar	PX(BGRIMG2YUV420Planar)
#define YUV420PlanarIMG2BGR	PX(YUV420PlanarIMG2BGR)

#define BGRIMG2RGB	PX(BGRIMG2RGB)
#define RGBIMG2BGR	PX(RGBIMG2BGR)

#define BGRIMG2RGB565   PX(BGRIMG2RGB565)
#define RGBIMG2RGB565   PX(RGBIMG2RGB565)
#define RGB565IMG2BGR   PX(RGB565IMG2BGR)
/****************************************************************************/

MVoid RGB2YUV(MCOLORREF crRGB, MCOLORREF *pcrYUV);
MVoid YUV2RGB(MCOLORREF crYUV, MCOLORREF *pcrRGB);
/**
 *	BGRIMG2YUV, YUVIMG2BGR, 
 *				YUV <--> BGR
 *	
 *	Parameter:
 *				BGR							Pointer to BGR image
 *				YUV						Pointer to YUV image
 *				lWidth, lHeight		(in)	the width and height to image
 *				lLineLen			(in)	the line length to image
  */
MVoid BGRIMG2YUV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				 MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid YUVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				 MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

/*
 *	YCbCr14bits
 */
MVoid BGRIMG2YUV_U14(MByte *BGR, MDWord dwBGRLine, MUInt16 *YUV, 
					 MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid YUV_U14IMG2BGR(MUInt16 *YUV, MDWord dwYUVLine, MByte *BGR, 
					 MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

/*
 *	Gray
 */
MVoid RGB2Y(MByte R, MByte G, MByte B, MByte *Y);
MVoid BGRIMG2Y(MByte *IMG_BGR, MDWord dwBGRLine, 
			   MByte *IMG_Y, MDWord dwYLine,
			   MDWord dwWidth, MDWord dwHeight);
//20140812@bqj,新增RGBIMG2Y,作为BGRIMG2Y的补充
MVoid RGBIMG2Y(MByte *IMG_RGB, MDWord dwRGBLine, 
			   MByte *IMG_Y, MDWord dwYLine,
			   MDWord dwWidth, MDWord dwHeight);

/*
 *	YCbCr420 LP
 */
MVoid BGRIMG2YUV420LP(MByte *BGR, MDWord dwBGRLine, 
					  MByte *pYUV[], MDWord dwYUVLine[],
					  MDWord dwWidth, MDWord dwHeight);
MVoid YUV420LPIMG2BGR(MByte *pYUV[], MDWord dwYUVLine[],
					  MByte *BGR, MDWord dwBGRLine,						
					  MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2YUV420VU(MByte *BGR, MDWord dwBGRLine, 
					  MByte *pYUV[], MDWord dwYUVLine[],
					  MDWord dwWidth, MDWord dwHeight);
MVoid YUV420VUIMG2BGR(MByte *pYUV[], MDWord dwYUVLine[],
					  MByte *BGR, MDWord dwBGRLine,						
					  MDWord dwWidth, MDWord dwHeight);

/*
 *	CbYCrYCbYCr...
 */
MVoid BGRIMG2UYVY(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
					  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid UYVYIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
					MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2YUYV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid YUYVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2Y1VY0U(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid Y1VY0UIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2Y8U4V4(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
					MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid Y8U4V4IMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
					MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2YYUV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight);
MVoid YYUVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight);

/*
 *	YYY... (W, H)
 *	UUU... (W/2, H)
 *	VVV... (W/2, H)
 */
MVoid BGRIMG2YUV422Planar(const MByte *pBGR, MDWord dwBGRLine, 
						  MByte *pYUV[], MDWord dwYUVLine[], 
						  MDWord dwWidth, MDWord dwHeight);
MVoid YUV422PlanarIMG2BGR(const MByte *pYUV[], MDWord dwYUVLine[], 
						  MByte *pBGR, MDWord dwBGRLine, 
						  MDWord dwWidth, MDWord dwHeight);
/*
 *	YYY... (W, H)
 *	UUU... (W/2, H/2)
 *	VVV... (W/2, H/2)
 */
MVoid BGRIMG2YUV420Planar(const MByte *pBGR, MDWord dwBGRLine, 
						  MByte *pYUV[], MDWord dwYUVLine[], 
						  MDWord dwWidth, MDWord dwHeight);
MVoid YUV420PlanarIMG2BGR(const MByte *pYUV[], MDWord dwYUVLine[], 
						  MByte *pBGR, MDWord dwBGRLine, 
						  MDWord dwWidth, MDWord dwHeight);

MVoid BGRIMG2RGB(const MByte *pBGR, MDWord dwBGRLine, 
				 MByte *pRGB, MDWord dwRGBLine, 
				 MDWord dwWidth, MDWord dwHeight);
MVoid RGBIMG2BGR(const MByte *pRGB, MDWord dwRGBLine, 
				 MByte *pBGR, MDWord dwBGRLine, 
				 MDWord dwWidth, MDWord dwHeight);

// MVoid RGBIMG2YUYV(const MByte *pRGB, MDWord dwRGBLine, 
// 				 MByte *pYUV, MDWord dwYUVLine, 
//				 MDWord dwWidth, MDWord dwHeight);

//////////////////////////////////////////////////////////////////////////
//MVoid RGB5652YUV(MCOLORREF crRGB, MCOLORREF *pcrYUV);
MVoid BGRIMG2RGB565(const MByte *pBGR, MDWord dwBGRLine, 
				 MByte *pRGB565, MDWord dwRGB565Line, 
				 MDWord dwWidth, MDWord dwHeight);
MVoid RGB565IMG2BGR(const MByte *pRGB565, MDWord dwRGB565Line, 
				 MByte *pBGR, MDWord dwBGRLine, 
				 MDWord dwWidth, MDWord dwHeight);

MVoid RGBIMG2RGB565(const MByte *pRGB, MDWord dwRGBLine, 
				 MByte *pRGB565, MDWord dwRGB565Line, 
				 MDWord dwWidth, MDWord dwHeight);

#ifdef __cplusplus
}
#endif

#endif // !defined(_LI_YUV_H_)
