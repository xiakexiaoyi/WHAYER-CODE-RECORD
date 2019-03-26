#ifndef _LI_COMDEF_H_
#define _LI_COMDEF_H_

#include "amcomdef.h"
#include "litrimfun.h"

/************************************************************************/
/* extern data byte                                                                     */
/************************************************************************/
//Basic type defination
typedef MUInt8		JBool;
typedef struct {
	MShort	x, y;
}JPoint, *PJPoint;
typedef struct {	
	MVoid *pArray;
	MDWord dwArrayLen;
}JARRAY, *PJARRAY;

//////////////////////////////////////////////////////////////////////////
/* Macro for basic data
 *
 *	 0x	    *			*	    		*			*			*			*
 *		Valid bits    Img type   offFloat/Fixed	Signed/UnSigned	ChannelByte	ChannelNum
 */
//////////////////////////////////////////////////////////////////////////
//Basic Macro for basic data type
#define DATA_MASK			0xFF0
#define DATA_MASK_BYTES		0x0F0
#define DATA_FLAG_SIGN		0x100
#define DATA_FLAG_FLOAT		0x200

typedef enum {
	DATA_NONE	= 0x0,	
	DATA_U8		= 0x010,
	DATA_I8		= DATA_FLAG_SIGN | DATA_U8,
	DATA_U16	= 0x020,	
	DATA_I16	= DATA_FLAG_SIGN | DATA_U16,
	DATA_U32	= 0x040,
	DATA_U64	= 0x080,
	DATA_I32	= DATA_FLAG_SIGN | DATA_U32,
	DATA_I64	= DATA_FLAG_SIGN | DATA_U64,
	DATA_F32	= DATA_FLAG_FLOAT| DATA_I32,
	DATA_F64	= DATA_FLAG_FLOAT| DATA_I64
} JTYPE_DATA;

#define IF_DATA_TYPE(imgFmt)		((JTYPE_DATA)((imgFmt) & DATA_MASK))
#define IF_DATA_BYTES(typeData)		(((typeData) & 0xF0)>>4) 
#define IF_IS_SIGNED(typeData)		((typeData) & DATA_FLAG_SIGN)
#define IF_IS_FLOAT(typeData)		((typeData) & DATA_FLAG_FLOAT) 
#define IF_SET_SIGNED(typeData)		((typeData) | DATA_FLAG_SIGN)
#define IF_SET_FLOAT(typeData)		((typeData) | DATA_FLAG_FLOAT)

//Extern data type defination
typedef MInt32 JTYPE_DATA_A;
#define DATABITS_MASK		0xF000

#define DATA_BOOL			(0x7000 | DATA_U8)
#define DATA_U14			(0x2000 | DATA_U16)
#define DATA_I14			(0x2000 | DATA_I16)

#define IF_DATA_BITS(typeData)		((IF_DATA_BYTES(typeData)<<3) - (((typeData)&DATABITS_MASK)>>12))
#define IF_DATA_TYPE_A(imgFmt)		((imgFmt) & (DATA_MASK|DATABITS_MASK))

/************************************************************************/
/*  Image format defination                                                                    */
/************************************************************************/
//Chunky or Planar 
#define FORMAT_P_C_MASK			0x10000000
#define FORMAT_CHUNKY			0x00000000		//Default
#define FORMAT_PLANAR			0x10000000

#define IF_IS_PLANAR_FORMAT(imgFmt)	((imgFmt) & FORMAT_P_C_MASK)

//Lumin VS color 2 4 6 8...
#define FORMAT_L_C_MASK			0xE0000000
#define FORMAT_444				0x00000000		//Default
#define FORMAT_422				0x20000000
#define FORMAT_420				0x40000000

#define IF_LU_CR_RADIO(imgFmt)	((imgFmt) & FORMAT_L_C_MASK)

//Lumin-color ext type
#define FORMAT_L_C_EXT_MASK			0xEF000000
#define FORMAT_888_LC1C2			FORMAT_444		
#define FORMAT_565					(FORMAT_444 | 0x01000000)

#define FORMAT_211_LC1LC2			FORMAT_422
#define FORMAT_211_C1LC2L			(FORMAT_422 | 0x01000000)
#define FORMAT_844					(FORMAT_422 | 0x02000000)
#define FORMAT_211_LLC1C2			(FORMAT_422 | 0x03000000)
#define FORMAT_211_L2C2L1C1			(FORMAT_422 | 0x04000000)

#define FORMAT_411_L_C1_C2			FORMAT_420
#define FORMAT_411_L_C1C2			(FORMAT_420 | 0x01000000)
#define FORMAT_411_L_C2C1			(FORMAT_420 | 0x02000000)

#define IF_LU_CR_EXT(imgFmt)	((imgFmt) & FORMAT_L_C_EXT_MASK)

//Color space format
#define FORMAT_CR_NAME_MASK		0x00F00000
#define FORMAT_MASK_CHANNELNUM	0x0000000F
#define FORMAT_CR_YUV			0x00000003		//Default
#define FORMAT_CR_GRAY			0x00000001
#define FORMAT_CR_YCbCr			0x00100003
#define FORMAT_CR_LAB			0x00200003
#define FORMAT_CR_LUV			0x00300003
#define FORMAT_CR_RGB			0x00400003
#define FORMAT_CR_BGR			0x00500003
#define FORMAT_CR_HSV			0x00600003
#define FORMAT_CR_VECTOR_FIELD	0x00700002
#define FORMAT_CR_YVU			0x00800003

#define IF_FMT_BASE(imgFmt)		((imgFmt) & (FORMAT_CR_NAME_MASK|FORMAT_MASK_CHANNELNUM))
#define IF_CHANNEL_NUM(imgFmt)	((imgFmt) & FORMAT_MASK_CHANNELNUM)

//BASE FORMAT | DATA TYPE
typedef enum{
	FORMAT_NONE			= 0x00000000,	

	FORMAT_BLACKWHITE	= (FORMAT_CR_GRAY|DATA_BOOL| FORMAT_PLANAR),
	FORMAT_GRAY			= (FORMAT_CR_GRAY|DATA_U8| FORMAT_PLANAR),
	FORMAT_GRAY_I8		= (FORMAT_CR_GRAY|DATA_I8| FORMAT_PLANAR),
	FORMAT_GRAY_U32		= (FORMAT_CR_GRAY|DATA_U32| FORMAT_PLANAR),
	
	FORMAT_BGR			= (FORMAT_CR_BGR|DATA_U8),
	FORMAT_RGB			= (FORMAT_CR_RGB|DATA_U8),
	FORMAT_HSV			= (FORMAT_CR_HSV|DATA_U8),
	FORMAT_LUV			= (FORMAT_CR_LUV|DATA_U8),	
	FORMAT_YCbCr		= (FORMAT_CR_YCbCr|DATA_U8),	
	FORMAT_LAB			= (FORMAT_CR_LAB|DATA_U8),
	FORMAT_LAB_PLANAR	= (FORMAT_LAB | FORMAT_PLANAR),
	FORMAT_YUV			= (FORMAT_CR_YUV|DATA_U8),
	FORMAT_YUV_U14		= (FORMAT_CR_YUV|DATA_U14),		

	FORMAT_YUV444PLANAR = FORMAT_CR_YUV	| DATA_U8 | FORMAT_PLANAR | FORMAT_444,
	FORMAT_YUV422PLANAR = FORMAT_CR_YUV	| DATA_U8 | FORMAT_PLANAR | FORMAT_422,
	FORMAT_YUV420PLANAR	= FORMAT_CR_YUV	| DATA_U8 | FORMAT_PLANAR | FORMAT_420,
	FORMAT_YUV420_LP	= FORMAT_CR_YUV | DATA_U8 | FORMAT_PLANAR | FORMAT_411_L_C1C2,
	FORMAT_YUV420_VUVU	= FORMAT_CR_YVU | DATA_U8 | FORMAT_PLANAR | FORMAT_411_L_C2C1,
	FORMAT_YUV420       = FORMAT_CR_YUV|DATA_U8|FORMAT_CHUNKY,
	
	FORMAT_UYVY			= FORMAT_CR_YUV | DATA_U8 | FORMAT_211_C1LC2L,
	FORMAT_YUYV			= FORMAT_CR_YUV | DATA_U8 | FORMAT_211_LC1LC2,
	FORMAT_YUV422_P8	= FORMAT_CR_YUV | DATA_U8 | FORMAT_844,
	FORMAT_YYUV			= FORMAT_CR_YUV | DATA_U8 | FORMAT_211_LLC1C2,
	FORMAT_Y1VY0U		= FORMAT_CR_YUV | DATA_U8 | FORMAT_211_L2C2L1C1,

	FORMAT_RGB565		= FORMAT_CR_RGB | DATA_U8 | FORMAT_565,
	FORMAT_VECTOR_FIELD = FORMAT_CR_VECTOR_FIELD | DATA_U8 | FORMAT_PLANAR
}JIMAGE_FORMAT;
#define IF_UV_WIDTH(imgFmt, width)		(IF_LU_CR_RADIO(imgFmt)==FORMAT_444?(width):(width)/2)
#define IF_UV_HEIGHT(imgFmt, height)	(IF_LU_CR_RADIO(imgFmt)==FORMAT_420?(height)/2:(height))

//Image defination
typedef struct _TAG_OFFSCREEN {
	MDWord		dwWidth;				// Off-screen width
	MDWord		dwHeight;				// Off-screen height
	MLong		fmtImg;					// Format of pixel array
	union
	{
		struct
		{
			MDWord dwImgLine; 
			MVoid *pPixel;
		} chunky;
		struct
		{
			MDWord dwImgLine[4];
			MVoid *pPixel[4];
		} planar;
	} pixelArray;
} JOFFSCREEN, *PJOFFSCREEN;

typedef struct _TAG_IMAGE_EXIF{
	MDWord dwWidth, dwHeight;
	JIMAGE_FORMAT fmtImg;
}JIMAGE_EXIF, *PJIMAGE_EXIF;

#define PIXELVAL(cr1, cr2, cr3)		(((cr1) << 16) | ((cr2) << 8) | (cr3))
#define PIXELVAL_1(cr)				(MByte)(((cr) >> 16) & 0xFF)
#define PIXELVAL_2(cr)				(MByte)(((cr) >> 8 ) & 0xFF)
#define PIXELVAL_3(cr)				(MByte)(((cr))       & 0xFF)

/************************************************************************/
/* Constant defination                                                                     */
/************************************************************************/
#define NORMAL_ZOOM_SHIFT	8
#define NORMAL_ZOOM			(1<<NORMAL_ZOOM_SHIFT)
#define NORMAL_ZOOM_HALF	(NORMAL_ZOOM/2)

/************************************************************************/
/* Other                                                                     */
/************************************************************************/
typedef MRESULT (*JFNPROGRESS) (
	  MInt32	lProgress,			// The percentage of the current operation
	  MInt32	lStatus,			// The current status at the moment
	  MVoid*	pParam				// Caller-defined data
);

#define GO(FUNCTION)		{if((res = FUNCTION) != LI_ERR_NONE) goto EXT;}
#define RETURN(FUNCTION)	{MRESULT res; if((res = FUNCTION) != LI_ERR_NONE) return res;}

#define _BEGIN				{
#define _END				}

#ifdef WIN32
#define JINLINE				__inline
#else
#define JINLINE				
#endif

#define JSTATIC		static

#ifdef WIN32
#pragma   warning(disable:4514)  
#pragma   warning(disable:4127)  
#endif


#endif	//_LI_COMDEF_H_