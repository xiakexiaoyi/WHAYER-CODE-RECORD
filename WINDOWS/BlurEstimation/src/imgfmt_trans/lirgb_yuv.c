#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "lirgb_yuv.h"

#include "litrimfun.h"
#include "limath.h"
#include "lidebug.h"


// #ifdef OPTIMIZATION_SSE
// #include "lirgb_yuv_sse.h"
// #endif

//1, 
//Y'= 0.299*R' + 0.587*G' + 0.114*B'
//U'=-0.147*R' - 0.289*G' + 0.436*B' = 0.492*(B'- Y')
//V'= 0.615*R' - 0.515*G' - 0.100*B' = 0.877*(R'- Y')
//
//R' = Y' + 1.140*V'
//G' = Y' - 0.394*U' - 0.581*V'
//B' = Y' + 2.032*U'

//2,
/* r = 0.299; g = 0.587; b = 0.114; 
 * cb = 0.5/(1-b); cr = 0.5/(1-r)
 *
 * RGB --> YUV
 *	r		g		b
 *  -r		-g		1-b		* cb
 *	1-r		-g		-b		* cr
 *
 * YUV --> RGB
 *	1	1/cr				0
 *	1	-r/(cr*g)			-b/(cb*g)
 *	1	0					1/cb
 */


#define ZOOM				32768
#define	ZOOM_SHIFT			15
#define	ZOOM_HALF	   		16384

#define COLOR_OFFSET		(1<<7)
#define COLOR_OFFSET_14		(1<<13)

#define	RGB_YUV_R 			9798	//0.299
#define	RGB_YUV_G 			19235	//0.587
#define	RGB_YUV_B 			3736	//0.114

#define	RGB_YUV_U 			18492	//(0.5/(1-0.114)) = 0.564
#define	RGB_YUV_V 			23372	//(0.5/(1-0.299)) = 0.713

#define	YUV_RGB_R 			45941	//	(1-0.299)/0.5	= 1.402
#define	YUV_RGB_U			-11277	// -0.114/((0.5/(1-0.114))*0.587) = -0.344
#define	YUV_RGB_V			-23401	// -0.299/((0.5/(1-0.299))*0.587) = -0.714
#define	YUV_RGB_B 			58065	//	(1-0.114)/0.5	= 1.772

#define _RGB2YUV_L(R, G, B, lY, lCb, lCr)									\
{																			\
	MLong _b = B, _g = G, _r = R;											\
	lY = RGB_YUV_R*_r + RGB_YUV_G*_g + RGB_YUV_B*_b;						\
	lCb = ((_b<<7) - (lY>>(ZOOM_SHIFT-7)))*RGB_YUV_U >> 7;					\
	lCr = ((_r<<7) - (lY>>(ZOOM_SHIFT-7)))*RGB_YUV_V >> 7;					\
}

#define TRIM_TYPE	TRIM_UINT8
#define _RGB2YUV(R, G, B, Y, U, V)											\
{																			\
	MLong _lY, _lU, _lV;													\
	_RGB2YUV_L(R, G, B, _lY, _lU, _lV);										\
	Y  = (MByte)DOWN_ROUND(_lY, ZOOM_SHIFT);								\
	_lU = DOWN_ROUND(_lU, ZOOM_SHIFT) + COLOR_OFFSET;						\
	U = TRIM_TYPE(_lU);														\
	_lV = DOWN_ROUND(_lV, ZOOM_SHIFT) + COLOR_OFFSET;						\
	V = TRIM_TYPE(_lV);														\
}

#define _YUV2RGB_L(Y, U, V, lHalf, lR, lG, lB)								\
{																			\
	MLong _lY = Y, _lCb = (U)-(lHalf), _lCr = (V)-(lHalf);					\
	lB = (_lY<<ZOOM_SHIFT) + YUV_RGB_B*_lCb;								\
	lG = (_lY<<ZOOM_SHIFT) + YUV_RGB_U*_lCb + YUV_RGB_V*_lCr;				\
	lR = (_lY<<ZOOM_SHIFT) + YUV_RGB_R*_lCr;								\
}

#define _YUV2RGB(Y, U, V, R, G, B)											\
{																			\
	MLong _b, _g, _r;														\
	_YUV2RGB_L(Y, U, V, COLOR_OFFSET,_r, _g, _b);							\
	_r = DOWN_ROUND(_r, ZOOM_SHIFT);										\
	R = TRIM_UINT8(_r);														\
	_g = DOWN_ROUND(_g, ZOOM_SHIFT);										\
	G = TRIM_UINT8(_g);														\
	_b = DOWN_ROUND(_b, ZOOM_SHIFT);										\
	B = TRIM_UINT8(_b);														\
}

//YUV 14
#define TRIM_TYPE14		TRIM_UINT14
#define _RGB2YUV14(R, G, B, Y, U, V)										\
{																			\
	MLong _lY, _lU, _lV;													\
	_RGB2YUV_L(R, G, B, _lY, _lU, _lV);										\
	Y  = (MWord)DOWN_ROUND(_lY, ZOOM_SHIFT-6);								\
	_lU = DOWN_ROUND(_lU, ZOOM_SHIFT-6) + COLOR_OFFSET_14;					\
	U = TRIM_TYPE14(_lU);													\
	_lV = DOWN_ROUND(_lV, ZOOM_SHIFT-6) + COLOR_OFFSET_14;					\
	V = TRIM_TYPE14(_lV);													\
}

#define _YUV142RGB(Y, U, V, R, G, B)										\
{																			\
	MLong _b, _g, _r;														\
	_YUV2RGB_L(Y, U, V, COLOR_OFFSET_14, _r, _g, _b);						\
	_r = DOWN_ROUND(_r, ZOOM_SHIFT+6);										\
	R = TRIM_UINT8(_r);														\
	_g = DOWN_ROUND(_g, ZOOM_SHIFT+6);										\
	G = TRIM_UINT8(_g);														\
	_b = DOWN_ROUND(_b, ZOOM_SHIFT+6);										\
	B = TRIM_UINT8(_b);														\
}

#if defined TRIM_YUV_LAB || defined TRIM_RGB
MVoid YUV2RGB(MCOLORREF crYUV, MCOLORREF *pcrRGB)
{
	MLong Y = PIXELVAL_1(crYUV);
	MLong U = PIXELVAL_2(crYUV);
	MLong V = PIXELVAL_3(crYUV);
	MLong R, G, B;
	_YUV2RGB(Y, U, V, R, G, B);
	*pcrRGB = PIXELVAL(R, G, B);
}
MVoid YUVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				 MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwYUVLine = dwYUVLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVLine, BGR+=dwBGRLine)
	{
		for(x=0; x<dwWidth; x++, YUV+=3, BGR+=3)
		{
			_YUV2RGB(YUV[0], YUV[1], YUV[2], 
				BGR[2], BGR[1], BGR[0]);
		}
	}
}
#endif

#ifdef TRIM_RGB
MVoid RGB2YUV(MCOLORREF crRGB, MCOLORREF *pcrYUV)
{
	MLong R = PIXELVAL_1(crRGB);
	MLong G = PIXELVAL_2(crRGB);
	MLong B = PIXELVAL_3(crRGB);
	MLong Y, U, V;
	_RGB2YUV(R, G, B, Y, U, V);
	*pcrYUV = PIXELVAL(Y, U, V);
}

MVoid BGRIMG2YUV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				   MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwYUVLine = dwYUVLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, BGR+=dwBGRLine, YUV+=dwYUVLine)
	{
		for(x=0; x<dwWidth; x++, BGR+=3, YUV+=3)
		{
			_RGB2YUV(BGR[2], BGR[1], BGR[0], 
				YUV[0], YUV[1], YUV[2]);	
		}
	}
}
MVoid BGRIMG2RGB565(const MByte *pBGR, MDWord dwBGRLine, 
				 MByte *pRGB565, MDWord dwRGB565Line, 
				 MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwRGB565Line = dwRGB565Line - 2*dwWidth;
	for(y=dwHeight; y!=0; y--, pBGR+=dwBGRLine, pRGB565+=dwRGB565Line)
	{
		for(x=dwWidth; x!=0; x--, pBGR+=3, pRGB565+=2)
		{
            MDWord cr_r = ((MDWord)(pBGR[2])) >> 3,
                cr_g = ((MDWord)(pBGR[1])) >> 2,
                cr_b = ((MDWord)(pBGR[0])) >> 3;
            MDWord cr_tmp = (cr_r << 11) | (cr_g << 5) | cr_b;
            ((MWord*)pRGB565)[0] = (MWord)cr_tmp;
		}
	}
}
MVoid RGBIMG2RGB565(const MByte *pRGB, MDWord dwRGBLine, 
				 MByte *pRGB565, MDWord dwRGB565Line, 
				 MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwRGBLine = dwRGBLine - 3*dwWidth;
	dwRGB565Line = dwRGB565Line - 2*dwWidth; 
	for(y=dwHeight; y!=0; y--, pRGB565+=dwRGB565Line, pRGB+=dwRGBLine)
	{
		for(x=dwWidth; x!=0; x--, pRGB565+=2, pRGB+=3)
		{
            MDWord cr_r = ((MDWord)(pRGB[0])) >> 3,
                cr_g = ((MDWord)(pRGB[1])) >> 2,
                cr_b = ((MDWord)(pRGB[2])) >> 3;
            MDWord cr_tmp = (cr_r << 11) | (cr_g << 5) | cr_b;
            ((MWord*)pRGB565)[0] = (MWord)cr_tmp;
		}
	}
}
MVoid RGB565IMG2BGR(const MByte *pRGB565, MDWord dwRGB565Line, 
				 MByte *pBGR, MDWord dwBGRLine, 
				 MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwRGB565Line = dwRGB565Line - 2*dwWidth;
	for(y=dwHeight; y!=0; y--, pBGR+=dwBGRLine, pRGB565+=dwRGB565Line)
	{
		for(x=dwWidth; x!=0; x--, pBGR+=3, pRGB565+=2)
		{
            MDWord cr_tmp = *((MWord*)pRGB565);
            pBGR[2] = (MByte)(((cr_tmp >> 11) & 0x001F) << 3);
            pBGR[1] = (MByte)(((cr_tmp >>  5) & 0x003F) << 2);
            pBGR[0] = (MByte)((cr_tmp & 0x001F) << 3);
		}
	}
}

#ifdef TRIM_DATA_14BITS
MVoid BGRIMG2YUV_U14(MByte *BGR, MDWord dwBGRLine, MWord *YUV, 
				   MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwYUVLine = dwYUVLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, BGR+=dwBGRLine, YUV+=dwYUVLine)
	{
		for(x=0; x<dwWidth; x++, BGR+=3, YUV+=3)
		{													
			_RGB2YUV14(BGR[2], BGR[1], BGR[0], 
				YUV[0], YUV[1], YUV[2]);							
		}
	}
}

MVoid YUV_U14IMG2BGR(MWord *YUV, MDWord dwYUVLine, MByte *BGR, 
				   MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwYUVLine = dwYUVLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVLine, BGR+=dwBGRLine)
	{
		for(x=0; x<dwWidth; x++, YUV+=3, BGR+=3)
		{
			_YUV142RGB(YUV[0], YUV[1], YUV[2], 
				BGR[2], BGR[1], BGR[0]);																
		}
	}
}
#endif		//TRIM_DATA_14BITS

MVoid RGB2Y(MByte R, MByte G, MByte B, MByte *Y)
{
	MLong lY = RGB_YUV_R*R + RGB_YUV_G*G + RGB_YUV_B*B;
	*Y  = (MByte)((lY + ZOOM_HALF) >> ZOOM_SHIFT);
}

MVoid BGRIMG2Y(MByte *IMG_BGR, MDWord dwBGRLine, 
			   MByte *IMG_Y, MDWord dwYLine,
			   MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwBGRExt = dwBGRLine - 3*dwWidth; 
	MDWord dwYExt = dwYLine - dwWidth;
	for(y=dwHeight; y!=0; y--, IMG_BGR+=dwBGRExt, IMG_Y+=dwYExt)
	{
		for(x=dwWidth; x!=0; x--, IMG_BGR+=3, IMG_Y++)
		{
			RGB2Y(IMG_BGR[0], IMG_BGR[1], IMG_BGR[2], IMG_Y);
		}
	}
}

MVoid BGRIMG2UYVY(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
					  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
	MDWord dwBGRExt = dwBGRLine - (dwWidth/2)*6;
	MByte cb1, cr1, cb2, cr2;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, BGR += dwBGRExt)
	{
		for(x=dwWidth/2; x!=0; x--)
		{
			_RGB2YUV(BGR[2], BGR[1], BGR[0], YUV[1], cb1, cr1);			
			_RGB2YUV(BGR[5], BGR[4], BGR[3], YUV[3], cb2, cr2);
			YUV[0] = (MByte)((cb1+cb2+1)>>1);
			YUV[2] = (MByte)((cr1+cr2+1)>>1);
			BGR += 6, YUV +=4;			
		}
	}
}

MVoid UYVYIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
					  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MByte *pYCbCrCur = YUV;
	MByte *pBGRCur   = BGR;
	MDWord dwWidth2 = FLOOR_2(dwWidth);
	MDWord dwYUVExt = dwYUVLine - dwWidth2*2;
	MDWord dwBGRExt = dwBGRLine - dwWidth2*3;
	JASSERT((MDWord)YUV % 4 ==0);
	JASSERT((MDWord)BGR % 4 ==0);
	pYCbCrCur += dwYUVLine*(dwHeight-1) + (dwWidth2-2)*2;
	pBGRCur += dwBGRLine*(dwHeight-1)+(dwWidth2-2)*3;
	for(y=dwHeight; y!=0; y--, pYCbCrCur-=dwYUVExt, pBGRCur -= dwBGRExt)
	{
		JASSERT((pYCbCrCur-YUV)%dwYUVLine == (dwWidth2-2)*2);
		JASSERT((pBGRCur-BGR)%dwBGRLine == (dwWidth2-2)*3);
		for(x=dwWidth2; x!=0; x-=2)
		{			
			MLong lY1 = pYCbCrCur[1], lY2 = pYCbCrCur[3];
			MLong lCb = pYCbCrCur[0], lCr = pYCbCrCur[2];
			_YUV2RGB(lY1, lCb, lCr, pBGRCur[2], pBGRCur[1], pBGRCur[0]);
			_YUV2RGB(lY2, lCb, lCr, pBGRCur[5], pBGRCur[4], pBGRCur[3]);							
			pBGRCur -= 6, pYCbCrCur -=4;
		}
	}
}

MVoid BGRIMG2YUYV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
	MDWord dwBGRExt = dwBGRLine - (dwWidth/2)*6;
	MByte cb1, cr1, cb2, cr2;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, BGR += dwBGRExt)
	{
		for(x=(dwWidth/2); x!=0; x--)
		{
			_RGB2YUV(BGR[2], BGR[1], BGR[0], YUV[0], cb1, cr1);			
			_RGB2YUV(BGR[5], BGR[4], BGR[3], YUV[2], cb2, cr2);
			YUV[1] = (MByte)((cb1+cb2+1)>>1);
			YUV[3] = (MByte)((cr1+cr2+1)>>1);
			BGR += 6, YUV +=4;			
		}
	}
}
MVoid YUYVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MByte *pYCbCrCur = YUV;
	MByte *pBGRCur   = BGR;
	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
	MDWord dwBGRExt = dwBGRLine - (dwWidth/2)*6;
	JASSERT((MDWord)YUV % 4 ==0);
	JASSERT((MDWord)BGR % 4 ==0);
	pYCbCrCur += dwYUVLine*(dwHeight-1) + (dwWidth/2-1)*4;
	pBGRCur += dwBGRLine*(dwHeight-1)+(dwWidth/2-1)*6;
	for(y=dwHeight; y!=0; y--, pYCbCrCur-=dwYUVExt, pBGRCur -= dwBGRExt)
	{
		JASSERT((pYCbCrCur-YUV)%dwYUVLine == (dwWidth/2-1)*4);
		JASSERT((pBGRCur-BGR)%dwBGRLine == (dwWidth/2-1)*6);
		for(x=dwWidth/2; x!=0; x--)
		{
			MLong lY1 = pYCbCrCur[0], lY2 = pYCbCrCur[2];
			MLong lCb = pYCbCrCur[1], lCr = pYCbCrCur[3];
			_YUV2RGB(lY1, lCb, lCr, pBGRCur[2], pBGRCur[1], pBGRCur[0]);
			_YUV2RGB(lY2, lCb, lCr, pBGRCur[5], pBGRCur[4], pBGRCur[3]);							
			pBGRCur -= 6, pYCbCrCur -=4;
		}
	}
}

MVoid BGRIMG2Y1VY0U(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
					MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
	MDWord dwBGRExt = dwBGRLine - (dwWidth/2)*6;
	MByte cb1, cr1, cb2, cr2;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, BGR += dwBGRExt)
	{
		for(x=(dwWidth/2); x!=0; x--)
		{
			_RGB2YUV(BGR[2], BGR[1], BGR[0], YUV[2], cb1, cr1);			
			_RGB2YUV(BGR[5], BGR[4], BGR[3], YUV[0], cb2, cr2);
			YUV[3] = (MByte)((cb1+cb2+1)>>1);
			YUV[1] = (MByte)((cr1+cr2+1)>>1);
			BGR += 6, YUV +=4;			
		}
	}
}
MVoid Y1VY0UIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MByte *pYCbCrCur = YUV;
	MByte *pBGRCur   = BGR;
	MDWord dwWidth2 = FLOOR_2(dwWidth);
	MDWord dwYUVExt = dwYUVLine - dwWidth2*2;
	MDWord dwBGRExt = dwBGRLine - dwWidth2*3;
	JASSERT((MDWord)YUV % 4 ==0);
	JASSERT((MDWord)BGR % 4 ==0);
	pYCbCrCur += dwYUVLine*(dwHeight-1) + (dwWidth2-2)*2;
	pBGRCur += dwBGRLine*(dwHeight-1)+(dwWidth2-2)*3;
	for(y=dwHeight; y!=0; y--, pYCbCrCur-=dwYUVExt, pBGRCur -= dwBGRExt)
	{
		JASSERT((pYCbCrCur-YUV)%dwYUVLine == (dwWidth2-2)*2);
		JASSERT((pBGRCur-BGR)%dwBGRLine == (dwWidth2-2)*3);
		for(x=dwWidth2; x!=0; x-=2)
		{
			MLong lY1 = pYCbCrCur[2], lY2 = pYCbCrCur[0];
			MLong lCb = pYCbCrCur[3], lCr = pYCbCrCur[1];
			_YUV2RGB(lY1, lCb, lCr, pBGRCur[2], pBGRCur[1], pBGRCur[0]);
			_YUV2RGB(lY2, lCb, lCr, pBGRCur[5], pBGRCur[4], pBGRCur[3]);							
			pBGRCur -= 6, pYCbCrCur -=4;
		}
	}
}

MVoid BGRIMG2YYUV(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
				  MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
	MDWord dwBGRExt = dwBGRLine - (dwWidth/2)*6;
	MByte cb1, cr1, cb2, cr2;
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, BGR += dwBGRExt)
	{
		for(x=(dwWidth/2); x!=0; x--)
		{
			_RGB2YUV(BGR[2], BGR[1], BGR[0], YUV[0], cb1, cr1);			
			_RGB2YUV(BGR[5], BGR[4], BGR[3], YUV[1], cb2, cr2);
			YUV[2] = (MByte)((cb1+cb2+1)>>1);
			YUV[3] = (MByte)((cr1+cr2+1)>>1);
			BGR += 6, YUV +=4;			
		}
	}
}
MVoid YYUVIMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
				  MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MByte *pYCbCrCur = YUV;
	MByte *pBGRCur   = BGR;
	MDWord dwWidth2 = FLOOR_2(dwWidth);
	MDWord dwYUVExt = dwYUVLine - dwWidth2*2;
	MDWord dwBGRExt = dwBGRLine - dwWidth2*3;
	JASSERT((MDWord)YUV % 4 ==0);
	JASSERT((MDWord)BGR % 4 ==0);
	pYCbCrCur += dwYUVLine*(dwHeight-1) + (dwWidth2-2)*2;
	pBGRCur += dwBGRLine*(dwHeight-1)+(dwWidth2-2)*3;
	for(y=dwHeight; y!=0; y--, pYCbCrCur-=dwYUVExt, pBGRCur -= dwBGRExt)
	{
		JASSERT((pYCbCrCur-YUV)%dwYUVLine == (dwWidth2-2)*2);
		JASSERT((pBGRCur-BGR)%dwBGRLine == (dwWidth2-2)*3);
		for(x=dwWidth2; x!=0; x-=2)
		{
			MLong lY1 = pYCbCrCur[0], lY2 = pYCbCrCur[1];
			MLong lCb = pYCbCrCur[2], lCr = pYCbCrCur[3];
			_YUV2RGB(lY1, lCb, lCr, pBGRCur[2], pBGRCur[1], pBGRCur[0]);
			_YUV2RGB(lY2, lCb, lCr, pBGRCur[5], pBGRCur[4], pBGRCur[3]);							
			pBGRCur -= 6, pYCbCrCur -=4;
		}
	}
}

MVoid BGRIMG2Y8U4V4(MByte *BGR, MDWord dwBGRLine, MByte *YUV, 
					MDWord dwYUVLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x, y;
	MLong u0, u1, u2, u3;
	MLong v0, v1, v2, v3;
	MLong y0, y1, y2, y3;
	MDWord dwYUVExt = dwYUVLine - dwWidth*2;
	MDWord dwBGRExt = dwBGRLine - dwWidth*3;
	JASSERT(dwWidth % 8 == 0);
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, YUV+=dwBGRExt)
	{
		for(x=dwWidth/8; x!=0; x--)
		{
			_RGB2YUV_L(BGR[2], BGR[1], BGR[0], y0, u0, v0);
			_RGB2YUV_L(BGR[5], BGR[4], BGR[3], y1, u1, v1);
			BGR += 6;
			_RGB2YUV_L(BGR[2], BGR[1], BGR[0], y2, u2, v2);
			_RGB2YUV_L(BGR[5], BGR[4], BGR[3], y3, u3, v3);
			BGR += 6;
			YUV[0] = (MByte)DOWN_ROUND(y0, ZOOM_SHIFT);			
			YUV[1] = (MByte)DOWN_ROUND(y1, ZOOM_SHIFT);
			YUV[2] = (MByte)DOWN_ROUND(y2, ZOOM_SHIFT);			
			YUV[3] = (MByte)DOWN_ROUND(y3, ZOOM_SHIFT);
			u0 += u1, v0 += v1;
			u2 += u3, v2 += v3;
			u0 = DOWN_ROUND(u0, ZOOM_SHIFT+1) + COLOR_OFFSET;
			u2 = DOWN_ROUND(u2, ZOOM_SHIFT+1) + COLOR_OFFSET;
			v0 = DOWN_ROUND(v0, ZOOM_SHIFT+1) + COLOR_OFFSET;
			v2 = DOWN_ROUND(v2, ZOOM_SHIFT+1) + COLOR_OFFSET;
			YUV[8+0] = TRIM_UINT8(u0), YUV[8+1] = TRIM_UINT8(u2);
			YUV[12 + 0] = TRIM_UINT8(v0); YUV[12+1] = TRIM_UINT8(v2);			
			
			_RGB2YUV_L(BGR[2], BGR[1], BGR[0], y0, u0, v0);
			_RGB2YUV_L(BGR[5], BGR[4], BGR[3], y1, u1, v1);
			BGR += 6;
			_RGB2YUV_L(BGR[2], BGR[1], BGR[0], y2, u2, v2);
			_RGB2YUV_L(BGR[5], BGR[4], BGR[3], y3, u3, v3);
			BGR += 6;
			YUV[4] = (MByte)DOWN_ROUND(y0, ZOOM_SHIFT);			
			YUV[5] = (MByte)DOWN_ROUND(y1, ZOOM_SHIFT);
			YUV[6] = (MByte)DOWN_ROUND(y2, ZOOM_SHIFT);			
			YUV[7] = (MByte)DOWN_ROUND(y3, ZOOM_SHIFT);
			u0 += u1, v0 += v1;
			u2 += u3, v2 += v3;
			u0 = DOWN_ROUND(u0, ZOOM_SHIFT+1) + COLOR_OFFSET;
			u2 = DOWN_ROUND(u2, ZOOM_SHIFT+1) + COLOR_OFFSET;
			v0 = DOWN_ROUND(v0, ZOOM_SHIFT+1) + COLOR_OFFSET;
			v2 = DOWN_ROUND(v2, ZOOM_SHIFT+1) + COLOR_OFFSET;
			YUV[8+2] = TRIM_UINT8(u0), YUV[8+3] = TRIM_UINT8(u2);
			YUV[12 + 2] = TRIM_UINT8(v0); YUV[12+3] = TRIM_UINT8(v2);
			
			YUV += 16;
		}
	}
}
MVoid Y8U4V4IMG2BGR(MByte *YUV, MDWord dwYUVLine, MByte *BGR, 
					MDWord dwBGRLine, MDWord dwWidth, MDWord dwHeight)
{
	MDWord x, y;
	MLong u0, u1, u2, u3;
	MLong v0, v1, v2, v3;
	MLong y0, y1, y2, y3;
	MDWord dwYUVExt = dwYUVLine - dwWidth*2;
	MDWord dwBGRExt = dwBGRLine - dwWidth*3;
	JASSERT(dwWidth % 8 == 0);
	for(y=dwHeight; y!=0; y--, YUV+=dwYUVExt, YUV+=dwBGRExt)
	{
		for(x=dwWidth/8; x!=0; x--)
		{
			u0 = YUV[8+0], u1 = YUV[8+1], u2 = YUV[8+2], u3 = YUV[8+3];
			v0 = YUV[12+0], v1 = YUV[12+1], v2 = YUV[12+2], v3 = YUV[12+3];
			y0 = YUV[0], y1 = YUV[1], y2 = YUV[2], y3 = YUV[3];
			YUV += 4;
			_YUV2RGB(y0, u0, v0, BGR[2], BGR[1], BGR[0]);
			_YUV2RGB(y1, u0, v0, BGR[5], BGR[4], BGR[3]);
			BGR += 6;
			_YUV2RGB(y2, u1, v1, BGR[2], BGR[1], BGR[0]);
			_YUV2RGB(y3, u1, v1, BGR[5], BGR[4], BGR[3]);
			BGR += 6;
			y0 = YUV[0], y1 = YUV[1], y2 = YUV[2], y3 = YUV[3];
			YUV += 12;
			_YUV2RGB(y0, u2, v2, BGR[2], BGR[1], BGR[0]);
			_YUV2RGB(y1, u2, v2, BGR[5], BGR[4], BGR[3]);
			BGR += 6;
			_YUV2RGB(y2, u3, v3, BGR[2], BGR[1], BGR[0]);
			_YUV2RGB(y3, u3, v3, BGR[5], BGR[4], BGR[3]);
			BGR += 6;
		}
	}
}

MVoid BGRIMG2YUV420LP(MByte *BGR, MDWord dwBGRLine, 
					  MByte *pYUV[], MDWord dwYUVLine[],
					  MDWord dwWidth, MDWord dwHeight)
{
	MLong dwWidth2 = FLOOR_2(dwWidth);
	MLong dwHeight2 = FLOOR_2(dwHeight);
	MByte *pY1 = pYUV[0], *pY2 = pYUV[0] + dwYUVLine[0];
	MByte *pUV = pYUV[1];
	MDWord x, y;
	MLong cbsum, crsum, Y, U, V;
	MByte *pBGR1 = BGR, *pBGR2 = BGR + dwBGRLine;
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2; 
	MDWord dwBGRExt = dwBGRLine*2-dwWidth2*3;
	MDWord dwUVExt = dwYUVLine[1] - dwWidth2;
	for(y=dwHeight2; y!=0; y-=2, pBGR1 += dwBGRExt, pBGR2 += dwBGRExt, 
		pY1 += dwYExt, pY2 += dwYExt, pUV += dwUVExt)
	{
		for(x=dwWidth2; x!=0; x-=2)
		{
			_RGB2YUV_L(pBGR1[2], pBGR1[1], pBGR1[0], Y, U, V);
			pBGR1 += 3;
			*(pY1++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum = U, crsum = V;
			_RGB2YUV_L(pBGR1[2], pBGR1[1], pBGR1[0], Y, U, V);
			pBGR1 += 3;
			*(pY1++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;

			_RGB2YUV_L(pBGR2[2], pBGR2[1], pBGR2[0], Y, U, V);
			pBGR2 += 3;
			*(pY2++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;
			_RGB2YUV_L(pBGR2[2], pBGR2[1], pBGR2[0], Y, U, V);
			pBGR2 += 3;
			*(pY2++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;

			U = DOWN_ROUND(cbsum, ZOOM_SHIFT+2) + COLOR_OFFSET;
			V = DOWN_ROUND(crsum, ZOOM_SHIFT+2) + COLOR_OFFSET;

			*(pUV++) = TRIM_UINT8(U);
			*(pUV++) = TRIM_UINT8(V);
		}
	}

//	PrintBmp(BGR, dwBGRLine, DATA_U8, dwWidth2, dwHeight2, 3);
//	PrintBmp(YUV, dwYUVLine, DATA_U8, dwWidth2, dwHeight2, 1);
//	PrintBmp(YUV+dwYUVLine*dwHeight2, 
//		dwYUVLine, DATA_U8, dwWidth2, dwHeight2/2, 1);
}
MVoid YUV420LPIMG2BGR(MByte *pYUV[], MDWord dwYUVLine[],
					  MByte *BGR, MDWord dwBGRLine,						
					  MDWord dwWidth, MDWord dwHeight)
{
	MLong dwWidth2 = FLOOR_2(dwWidth);
	MLong dwHeight2 = FLOOR_2(dwHeight);
	MByte *pY1 = pYUV[0], *pY2 = pYUV[0] + dwYUVLine[0];
	MByte *pUV = pYUV[1];
	MDWord x, y;
	MLong Y, U, V;
	MByte *pBGR1 = BGR, *pBGR2 = BGR + dwBGRLine;
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2; 
	MDWord dwBGRExt = dwBGRLine*2-dwWidth2*3;
	MDWord dwUVExt = dwYUVLine[1] - dwWidth2;
	for(y=dwHeight2; y!=0; y-=2, pBGR1 += dwBGRExt, pBGR2 += dwBGRExt, 
		pY1 += dwYExt, pY2 += dwYExt, pUV += dwUVExt)
	{
		for(x=dwWidth2; x!=0; x-=2)
		{
			U = *(pUV++);
			V = *(pUV++);
			
			Y = *(pY1++);
			_YUV2RGB(Y, U, V, pBGR1[2], pBGR1[1], pBGR1[0]);
			pBGR1 += 3;
			Y = *(pY1++);
			_YUV2RGB(Y, U, V, pBGR1[2], pBGR1[1], pBGR1[0]);
			pBGR1 += 3;
			
			Y = *(pY2++);
			_YUV2RGB(Y, U, V, pBGR2[2], pBGR2[1], pBGR2[0]);
			pBGR2 += 3;
			Y = *(pY2++);
			_YUV2RGB(Y, U, V, pBGR2[2], pBGR2[1], pBGR2[0]);
			pBGR2 += 3;
		}
	}
	
//	PrintBmp(YUV, dwYUVLine, DATA_U8, dwWidth, dwHeight, 1);
//	PrintBmp(YUV+dwYUVLine*dwHeight, 
//		dwYUVLine, DATA_U8, dwWidth, dwHeight/2, 1);	
//	PrintBmp(BGR, dwBGRLine, DATA_U8, dwWidth, dwHeight, 3);
}

MVoid BGRIMG2YUV420VU(MByte *BGR, MDWord dwBGRLine, 
					  MByte *pYUV[], MDWord dwYUVLine[],
					  MDWord dwWidth, MDWord dwHeight)
{
	MLong dwWidth2 = FLOOR_2(dwWidth);
	MLong dwHeight2 = FLOOR_2(dwHeight);
	MByte *pY1 = pYUV[0], *pY2 = pYUV[0] + dwYUVLine[0];
	MByte *pUV = pYUV[1];
	MDWord x, y;
	MLong cbsum, crsum, Y, U, V;
	MByte *pBGR1 = BGR, *pBGR2 = BGR + dwBGRLine;
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2; 
	MDWord dwBGRExt = dwBGRLine*2-dwWidth2*3;
	MDWord dwUVExt = dwYUVLine[1] - dwWidth2;
	for(y=dwHeight2; y!=0; y-=2, pBGR1 += dwBGRExt, pBGR2 += dwBGRExt, 
		pY1 += dwYExt, pY2 += dwYExt, pUV += dwUVExt)
	{
		for(x=dwWidth2; x!=0; x-=2)
		{
			_RGB2YUV_L(pBGR1[2], pBGR1[1], pBGR1[0], Y, U, V);
			pBGR1 += 3;
			*(pY1++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum = U, crsum = V;
			_RGB2YUV_L(pBGR1[2], pBGR1[1], pBGR1[0], Y, U, V);
			pBGR1 += 3;
			*(pY1++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;
			
			_RGB2YUV_L(pBGR2[2], pBGR2[1], pBGR2[0], Y, U, V);
			pBGR2 += 3;
			*(pY2++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;
			_RGB2YUV_L(pBGR2[2], pBGR2[1], pBGR2[0], Y, U, V);
			pBGR2 += 3;
			*(pY2++) = (MByte)DOWN_ROUND(Y, ZOOM_SHIFT);
			cbsum += U, crsum += V;
			
			U = DOWN_ROUND(cbsum, ZOOM_SHIFT+2) + COLOR_OFFSET;
			V = DOWN_ROUND(crsum, ZOOM_SHIFT+2) + COLOR_OFFSET;
			
			*(pUV++) = TRIM_UINT8(V);
			*(pUV++) = TRIM_UINT8(U);
		}
	}
}
MVoid YUV420VUIMG2BGR(MByte *pYUV[], MDWord dwYUVLine[],
					  MByte *BGR, MDWord dwBGRLine,						
					  MDWord dwWidth, MDWord dwHeight)
{
	MLong dwWidth2 = FLOOR_2(dwWidth);
	MLong dwHeight2 = FLOOR_2(dwHeight);
	MByte *pY1 = pYUV[0], *pY2 = pYUV[0] + dwYUVLine[0];
	MByte *pUV = pYUV[1];
	MDWord x, y;
	MLong Y, U, V;
	MByte *pBGR1 = BGR, *pBGR2 = BGR + dwBGRLine;
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2; 
	MDWord dwBGRExt = dwBGRLine*2-dwWidth2*3;
	MDWord dwUVExt = dwYUVLine[1] - dwWidth2;
	for(y=dwHeight2; y!=0; y-=2, pBGR1 += dwBGRExt, pBGR2 += dwBGRExt, 
		pY1 += dwYExt, pY2 += dwYExt, pUV += dwUVExt)
	{
		for(x=dwWidth2; x!=0; x-=2)
		{
			V = *(pUV++);
			U = *(pUV++);
			
			Y = *(pY1++);
			_YUV2RGB(Y, U, V, pBGR1[2], pBGR1[1], pBGR1[0]);
			pBGR1 += 3;
			Y = *(pY1++);
			_YUV2RGB(Y, U, V, pBGR1[2], pBGR1[1], pBGR1[0]);
			pBGR1 += 3;
			
			Y = *(pY2++);
			_YUV2RGB(Y, U, V, pBGR2[2], pBGR2[1], pBGR2[0]);
			pBGR2 += 3;
			Y = *(pY2++);
			_YUV2RGB(Y, U, V, pBGR2[2], pBGR2[1], pBGR2[0]);
			pBGR2 += 3;
		}
	}
}

MVoid BGRIMG2YUV422Planar(const MByte *pBGR, MDWord dwBGRLine, 
						  MByte *pYUV[], MDWord dwYUVLine[], 
						  MDWord dwWidth, MDWord dwHeight)
{

	MDWord x,y;
	MDWord dwYExt = dwYUVLine[0] - dwWidth;
	MDWord dwUExt = dwYUVLine[1] - dwWidth/2;
	MDWord dwVExt = dwYUVLine[2] - dwWidth/2;
	MDWord dwBGRExt = dwBGRLine - dwWidth*3;	
	MByte *pY = pYUV[0];
	MByte *pU = pYUV[1];
	MByte *pV = pYUV[2];
	MByte u1, v1, u2, v2;
	JASSERT(dwWidth %2 == 0);
	for(y=dwHeight; y!=0; y--, pBGR += dwBGRExt, 
		pY += dwYExt, pU += dwUExt, pV += dwVExt)
	{
		for(x=dwWidth; x!=0; x-=2, pBGR += 6, pY += 2, pU++, pV++)
		{
			_RGB2YUV(pBGR[2], pBGR[1], pBGR[0], pY[0], u1, v1);			
			_RGB2YUV(pBGR[5], pBGR[4], pBGR[3], pY[1], u2, v2);
			*pU = (MByte)((u1+u2+1)>>1);
			*pV = (MByte)((v1+v2+1)>>1);
		}
	}
}

MVoid YUV422PlanarIMG2BGR(const MByte *pYUV[], MDWord dwYUVLine[], 
						  MByte *pBGR, MDWord dwBGRLine, 
						  MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	MDWord dwYExt = dwYUVLine[0] - dwWidth;
	MDWord dwUExt = dwYUVLine[1] - dwWidth/2;
	MDWord dwVExt = dwYUVLine[2] - dwWidth/2;
	MDWord dwBGRExt = dwBGRLine - dwWidth*3;	
	const MByte *pY = pYUV[0];
	const MByte *pU = pYUV[1];
	const MByte *pV = pYUV[2];
	JASSERT(dwWidth %2 == 0);
	for(y=dwHeight; y!=0; y--, pBGR += dwBGRExt, 
		pY += dwYExt, pU += dwUExt, pV += dwVExt)
	{
		for(x=dwWidth; x!=0; x-=2, pBGR += 6, pY += 2, pU++, pV++)
		{
			_YUV2RGB(pY[0], pU[0], pV[0], pBGR[2], pBGR[1], pBGR[0]);			
			_YUV2RGB(pY[1], pU[0], pV[0], pBGR[5], pBGR[4], pBGR[3]);
		}
	}		
}

MVoid BGRIMG2YUV420Planar(const MByte *pBGR, MDWord dwBGRLine, 
						  MByte *pYUV[], MDWord dwYUVLine[], 
						  MDWord dwWidth, MDWord dwHeight)
{
// #ifdef OPTIMIZATION_SSE
// 	BGRIMG2YUV420Planar_SSE2(pBGR, dwBGRLine, pYUV, dwYUVLine, 
// 		dwWidth, dwHeight);
// #else	//OPTIMIZATION_SSE
	MDWord dwWidth2 = FLOOR_2(dwWidth);
	MDWord dwHeight2 = FLOOR_2(dwHeight);
	MDWord x,y;
	const MByte *pBGR1 = pBGR + dwBGRLine;	
	MDWord dwBGRExt = dwBGRLine*2 - dwWidth2*3;	
	MByte *pY = pYUV[0];
	MByte *pY1 = pY + dwYUVLine[0];
	MByte *pU = pYUV[1];
	MByte *pV = pYUV[2];
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2;
	MDWord dwUExt = dwYUVLine[1] - dwWidth2/2;
	MDWord dwVExt = dwYUVLine[2] - dwWidth2/2;
	MByte u1, v1, u2, v2, u3, v3, u4, v4;
	for(y=dwHeight2; y!=0; y-=2, pBGR += dwBGRExt, pBGR1 += dwBGRExt,
		pY += dwYExt, pY1 += dwYExt, pU += dwUExt, pV += dwVExt)
	{	
		for(x=dwWidth2; x!=0; x-=2, pBGR += 6,pBGR1+=6, pY += 2, pY1+=2, pU++, pV++)
		{
			_RGB2YUV(pBGR[2], pBGR[1], pBGR[0], pY[0], u1, v1);			
			_RGB2YUV(pBGR[5], pBGR[4], pBGR[3], pY[1], u2, v2);
			_RGB2YUV(pBGR1[2], pBGR1[1], pBGR1[0], pY1[0], u3, v3);			
			_RGB2YUV(pBGR1[5], pBGR1[4], pBGR1[3], pY1[1], u4, v4);
			*pU = (MByte)((u1+u2+u3+u4+2)>>2);
			*pV = (MByte)((v1+v2+v3+v4+2)>>2);
		}
	}
//#endif	//OPTIMIZATION_SSE
// 	PrintBmp(pYUV[0], dwYUVLine[0], DATA_U8, dwWidth, dwHeight, 1);
// 	PrintBmp(pYUV[1], dwYUVLine[1], DATA_U8, dwWidth/2, dwHeight/2, 1);
// 	PrintBmp(pYUV[2], dwYUVLine[2], DATA_U8, dwWidth/2, dwHeight/2, 1);
}

MVoid YUV420PlanarIMG2BGR(const MByte *pYUV[], MDWord dwYUVLine[], 
						  MByte *pBGR, MDWord dwBGRLine, 
						  MDWord dwWidth, MDWord dwHeight)
{
// 	PrintBmp(pYUV[0], dwYUVLine[0], DATA_U8, dwWidth, dwHeight, 1);
// 	PrintBmp(pYUV[1], dwYUVLine[1], DATA_U8, dwWidth/2, dwHeight/2, 1);
// 	PrintBmp(pYUV[2], dwYUVLine[2], DATA_U8, dwWidth/2, dwHeight/2, 1);
// #ifdef OPTIMIZATION_SSE
// 	YUV420PlanarIMG2BGR_SSE2(pYUV, dwYUVLine, pBGR, dwBGRLine,  
// 		dwWidth, dwHeight);
// #else //OPTIMIZATION_SSE
	MDWord dwWidth2 = FLOOR_2(dwWidth);
	MDWord dwHeight2 = FLOOR_2(dwHeight);
	MDWord x,y;	
	MByte *pBGR0 = pBGR, *pBGR1 = pBGR + dwBGRLine;	
	MDWord dwBGRExt = dwBGRLine*2 - dwWidth2*3;	
	const MByte *pY = pYUV[0];
	const MByte *pY1 = pY + dwYUVLine[0];
	const MByte *pU = pYUV[1];
	const MByte *pV = pYUV[2];	
	MDWord dwYExt = dwYUVLine[0]*2 - dwWidth2;
	MDWord dwUExt = dwYUVLine[1] - dwWidth2/2;
	MDWord dwVExt = dwYUVLine[2] - dwWidth2/2;
	for(y=dwHeight2; y!=0; y-=2, pBGR0 += dwBGRExt, pBGR1 += dwBGRExt,
		pY += dwYExt, pY1 += dwYExt, pU += dwUExt, pV += dwVExt)
	{
		for(x=dwWidth2; x!=0; x-=2, pBGR0+=6, pBGR1+=6, pY+=2, pY1+=2, pU++, pV++)
		{
			_YUV2RGB(pY[0], pU[0], pV[0], pBGR0[2], pBGR0[1], pBGR0[0]);			
			_YUV2RGB(pY[1], pU[0], pV[0], pBGR0[5], pBGR0[4], pBGR0[3]);
			_YUV2RGB(pY1[0], pU[0], pV[0], pBGR1[2], pBGR1[1], pBGR1[0]);			
			_YUV2RGB(pY1[1], pU[0], pV[0], pBGR1[5], pBGR1[4], pBGR1[3]);
		}
	}
//#endif	

//	PrintChannel(pBGR, dwBGRLine, DATA_U8, dwWidth*3, dwHeight, 1, 0);
}
MVoid BGRIMG2RGB(const MByte *pBGR, MDWord dwBGRLine, 
				 MByte *pRGB, MDWord dwRGBLine, 
				 MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwRGBLine = dwRGBLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, pBGR+=dwBGRLine, pRGB+=dwRGBLine)
	{
		for(x=dwWidth; x!=0; x--, pBGR+=3, pRGB+=3)
		{
			pRGB[0] = pBGR[2];
			pRGB[1] = pBGR[1];
			pRGB[2] = pBGR[0];
		}
	}
}
MVoid RGBIMG2BGR(const MByte *pRGB, MDWord dwRGBLine, 
				 MByte *pBGR, MDWord dwBGRLine, 
				 MDWord dwWidth, MDWord dwHeight)
{
	MDWord x,y;
	dwBGRLine = dwBGRLine - 3*dwWidth; 
	dwRGBLine = dwRGBLine - 3*dwWidth;
	for(y=dwHeight; y!=0; y--, pBGR+=dwBGRLine, pRGB+=dwRGBLine)
	{
		for(x=dwWidth; x!=0; x--, pBGR+=3, pRGB+=3)
		{
			pBGR[0] = pRGB[2];
			pBGR[1] = pRGB[1];
			pBGR[2] = pRGB[0];
		}
	}
}
// MVoid RGBIMG2YUYV(const MByte *pRGB, MDWord dwRGBLine, 
// 				 MByte *pYUV, MDWord dwYUVLine, 
// 				 MDWord dwWidth, MDWord dwHeight)
// {
// 	MDWord x,y;
// 	MDWord dwYUVExt = dwYUVLine - (dwWidth/2)*4;
// 	MDWord dwRGBExt = dwRGBLine - (dwWidth/2)*6;
// 	MByte cb1, cr1, cb2, cr2;
// 	for(y=dwHeight; y!=0; y--, pYUV+=dwYUVExt, pRGB += dwRGBExt)
// 	{
// 		for(x=(dwWidth/2); x!=0; x--)
// 		{
// 			_RGB2YUV(pRGB[0], pRGB[1], pRGB[2], pYUV[0], cb1, cr1);			
// 			_RGB2YUV(pRGB[3], pRGB[4], pRGB[5], pYUV[2], cb2, cr2);
// 			pYUV[1] = (MByte)((cb1+cb2+1)>>1);
// 			pYUV[3] = (MByte)((cr1+cr2+1)>>1);
// 			pRGB += 6, pYUV +=4;			
// 		}
// 	}    
// }
#endif	//TRIM_RGB