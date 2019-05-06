#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif

#include "litrimfun.h"
#include "limath.h"
#include "limem.h"
#include "lidebug.h"

#define		MStrCpy		strcpy
#define		_MMT

#ifdef ENABLE_DEBUG
#include <stdio.h>
#include <string.h>

MTChar g_szTextName[64] = _MMT("D:\\li_debug.txt");

extern FILE *g_fp = MNull;
MBool BeginDebug(MVoid)
{
	g_fp = fopen(g_szTextName, "a+");
	if(g_fp == MNull)
		return MFalse;
	return MTrue;
}
MVoid EndDebug(MVoid)
{
	if(g_fp == MNull)
		return;
	fclose(g_fp);
	g_fp = MNull;
}

//////////////////////////////////////////////////////////////////////////
#define PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight,					\
			dwChannelNum, dwChannelCur, FORMAT, TYPE)						\
{																			\
	MUInt32 _i,_j;															\
	MUInt32 _dwExt = dwImgLine - (dwChannelNum)*(dwWidth);					\
	TYPE*_pImgCur = (TYPE*)pImg + dwChannelCur;								\
	if(!BeginDebug())														\
		return;																\
	fprintf(g_fp, "\n=====Width = %d, Height = %d====\n", dwWidth, dwHeight);\
	for(_i=0; _i<(dwHeight); _i++, _pImgCur += _dwExt)						\
	{																		\
		fprintf(g_fp, "%3d: ", _i);											\
		for(_j=0; _j<(dwWidth); _j++, _pImgCur+=(dwChannelNum))				\
		{																	\
			fprintf(g_fp, FORMAT, _pImgCur[0]);								\
		}																	\
		fprintf(g_fp, "\n");												\
	}																		\
	fprintf(g_fp, "\n");													\
	EndDebug();																\
}
MVoid PrintShortChannel(const MInt16 *pImg, MUInt32 dwImgLine,
						MUInt32 dwWidth, MUInt32 dwHeight, 
						MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%4d,", MInt16);
}
MVoid PrintWordChannel(const MUInt16 *pImg, MUInt32 dwImgLine,
		MUInt32 dwWidth, MUInt32 dwHeight, MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%4d,", MUInt16);
}
MVoid PrintBoolChannel(const MBool *pImg, MUInt32 dwImgLine,
					   MUInt32 dwWidth, MUInt32 dwHeight, 
					   MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%d,", MBool);
}

MVoid PrintByteChannel(const MUInt8 *pImg, MUInt32 dwImgLine,
					   MUInt32 dwWidth, MUInt32 dwHeight, 
					   MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%3d,", MUInt8);
}
MVoid PrintCharChannel(const MInt8 *pImg, MUInt32 dwImgLine,
					   MUInt32 dwWidth, MUInt32 dwHeight, 
					   MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%6d,", MInt8);
}
MVoid PrintLongChannel(const MInt32 *pImg, MUInt32 dwImgLine, 
					   MUInt32 dwWidth, MUInt32 dwHeight, 
					   MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%8d,", MInt32);
}
MVoid PrintDWordChannel(const MUInt32 *pImg, MUInt32 dwImgLine, 
						MUInt32 dwWidth, MUInt32 dwHeight, 
						MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%8d,", MUInt32);
}
MVoid PrintFloatChannel(const MFloat *pImg, MUInt32 dwImgLine, 
						 MUInt32 dwWidth, MUInt32 dwHeight, 
						 MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%.8f,", MFloat);
}
MVoid PrintDoubleChannel(const MDouble *pImg, MUInt32 dwImgLine, 
						MUInt32 dwWidth, MUInt32 dwHeight, 
						MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	PRINTCHANNEL_AD(pImg, dwImgLine, dwWidth, dwHeight, 
		dwChannelNum, dwChannelCur, "%.10f,", MDouble);
}
MVoid PrintChannel(const MVoid *pImg, MUInt32 dwImgLine, JTYPE_DATA_A typeDataA,
				   MUInt32 dwWidth, MUInt32 dwHeight, MUInt32 dwChannelNum, MUInt32 dwChannelCur)
{
	switch(IF_DATA_TYPE(typeDataA)) {
	case DATA_U8:
		PrintByteChannel((const MUInt8*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_I8:
		PrintCharChannel((const MInt8*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_U16:
		PrintWordChannel((const MUInt16*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_I16:
		PrintShortChannel((const MInt16*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_U32:
		PrintDWordChannel((const MUInt32*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_I32:
		PrintLongChannel((const MInt32*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	case DATA_BOOL:
		PrintBoolChannel((const MBool*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);		
		break;
    case DATA_F64:
        PrintDoubleChannel((const MDouble*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);		
        break;
	default:
		PrintByteChannel((const MUInt8*)pImg, dwImgLine, dwWidth, dwHeight, 
			dwChannelNum, dwChannelCur);
		break;
	}
}
//////////////////////////////////////////////////////////////////////////
#ifndef JBIG_ENDIAN
#define CORRECT_ENDIAN_INT(x);
#define CORRECT_ENDIAN_SHORT(x);
#else
#define CORRECT_ENDIAN_INT(x)	((x) = (((MUInt32)(x)) >> 24) | ((((MUInt32)(x)) & 0x00ff0000) >> 8) | ((((MUInt32)(x)) & 0x0000ff00) << 8) | ((((MUInt32)(x)) & 0x000000ff) << 24))
#define CORRECT_ENDIAN_SHORT(x)	((x) = (((MUInt16)(x)) >> 8) | ((((MUInt16)(x)) & 0x00ff) << 8))
#endif

typedef struct tagAM_BMPINFOHEADER
{
    MUInt32	biSize;
    MInt32	biWidth;
    MInt32	biHeight;
    MUInt16	biPlanes;
    MUInt16	biBitCount;
    MUInt32	biCompression;
    MUInt32	biSizeImage;
    MInt32	biXPelsPerMeter;
    MInt32	biYPelsPerMeter;
    MUInt32	biClrUsed;
    MUInt32	biClrImportant;
} AM_BMPINFOHEADER;
typedef struct tagAM_RGBQUAD
{
    MUInt8	rgbBlue;
    MUInt8	rgbGreen;
    MUInt8	rgbRed;
    MUInt8	rgbReserved;
} AM_RGBQUAD, *AM_LPRGBQUAD;
MVoid _Correct_Endian_BMPHeader(AM_BMPINFOHEADER *pbi)
{
    CORRECT_ENDIAN_INT(pbi->biSize);
    CORRECT_ENDIAN_INT(pbi->biWidth);
    CORRECT_ENDIAN_INT(pbi->biHeight);
    CORRECT_ENDIAN_SHORT(pbi->biPlanes);
    CORRECT_ENDIAN_SHORT(pbi->biBitCount);
    CORRECT_ENDIAN_INT(pbi->biCompression);
    CORRECT_ENDIAN_INT(pbi->biSizeImage);
    CORRECT_ENDIAN_INT(pbi->biXPelsPerMeter);
    CORRECT_ENDIAN_INT(pbi->biYPelsPerMeter);
    CORRECT_ENDIAN_INT(pbi->biClrUsed);
    CORRECT_ENDIAN_INT(pbi->biClrImportant);
}

MVoid SAVETOBMP(const MUInt8 *pImgData, MInt32 lImgLine,
		MInt32 lWidth, MInt32 lHeight, MInt32 lChannelNum, 
		const MTChar* szBmpName)
{
	FILE * hStream = 0;
	const MUInt8 *p;
	MUInt16 wTemp;
	MUInt32 dwTemp;
	AM_BMPINFOHEADER bi;
	MLong lLineBytes, i;
	
	hStream = fopen(szBmpName, "wb+");
	if(!hStream)
		return;
	lLineBytes = CEIL_4(lWidth*lChannelNum);
	
	wTemp = 0x4D42;
	dwTemp = 54 + lImgLine * lHeight;
	if(lChannelNum == 2)
		dwTemp += 4*sizeof(MUInt32);
	else if(lChannelNum == 1)
		dwTemp += 256*sizeof(AM_RGBQUAD);
	CORRECT_ENDIAN_SHORT(wTemp);
	fwrite(&wTemp,1, 2,hStream);
	CORRECT_ENDIAN_INT(dwTemp);
	fwrite(&dwTemp,1, 4,hStream);
	wTemp = 0;
	dwTemp = 54;
	if(lChannelNum == 2)
		dwTemp += 4*sizeof(MUInt32);
	else if(lChannelNum == 1)
		dwTemp += 256*sizeof(AM_RGBQUAD);
	CORRECT_ENDIAN_SHORT(wTemp);
	fwrite( &wTemp, 1,2,hStream);
	fwrite(&wTemp,1 ,2,hStream);
	CORRECT_ENDIAN_INT(dwTemp);
	fwrite(&dwTemp,1, 4,hStream);
	memset(&bi, 0, sizeof(AM_BMPINFOHEADER));
	bi.biSize = sizeof(AM_BMPINFOHEADER);
	bi.biWidth = lWidth;
	bi.biHeight = lHeight;
	bi.biBitCount= (MUInt16)(lChannelNum*8);
	bi.biPlanes = 1;
	if(lChannelNum == 2)
		bi.biCompression = 3L;
	_Correct_Endian_BMPHeader(&bi);
	fwrite(&bi,1, sizeof(AM_BMPINFOHEADER),hStream);
	
	if(lChannelNum == 2)
	{
		MUInt32 mask[4] = {0xF800, 0x7E0, 0x1F, 0};
		fwrite(mask, 4, sizeof(MUInt32), hStream);
	}
	else if(lChannelNum == 1)
	{
		AM_RGBQUAD rgb[256];
		for(i=0; i<256; i++)
		{
			rgb[i].rgbBlue	= (MUInt8)i;
			rgb[i].rgbGreen	= (MUInt8)i;
			rgb[i].rgbRed	= (MUInt8)i;
			rgb[i].rgbReserved	= 0;
		}
		fwrite(rgb, 256, sizeof(AM_RGBQUAD), hStream);
	}
	p = pImgData + (lHeight - 1) * lImgLine;
	for(i=0; i<lHeight; i++, p-=lImgLine)
		fwrite(p,1, lLineBytes,hStream);
	
	fclose(hStream);
}
//////////////////////////////////////////////////////////////////////////
#define 	MStreamWrite(hStream, pBuf, lSize) fwrite(pBuf, 1, lSize, hStream)
#define SAVETOBMP_EX(pImgData, lImgLine, lWidth, lHeight, lChannelNum,		\
	lOffSet, lZoom, szBmpName, DATA_TYPE)									\
{																			\
	FILE * hStream = 0;														\
	DATA_TYPE *p = ((DATA_TYPE *)(pImgData))+(lHeight-1)*(lImgLine);		\
	MUInt16 wTemp;															\
	MUInt32 dwTemp;															\
	AM_BMPINFOHEADER bi;													\
	MInt32 lLineBytes, i,j;													\
	MInt32 lImgExt = CEIL_4((lWidth)*(lChannelNum))-(lWidth)*(lChannelNum);	\
	MUInt8 byTmp;															\
	MInt32 lTmp;															\
	hStream = fopen(szBmpName, "wb");										\
	if(hStream == MNull)													\
		return;																\
	lLineBytes = CEIL_4((lWidth) * (lChannelNum));							\
	wTemp = 0x4D42;															\
	dwTemp = 54 + lLineBytes * (lHeight);									\
	if ((lChannelNum) == 2)													\
		dwTemp += 4*sizeof(MUInt32);											\
	else if ((lChannelNum) == 1)												\
		dwTemp += 256*sizeof(AM_RGBQUAD);									\
	MStreamWrite(hStream, &wTemp,2);										\
	MStreamWrite(hStream, &dwTemp,4);										\
	wTemp = 0;																\
	dwTemp = 54;															\
	if ((lChannelNum) == 2)													\
		dwTemp += 4*sizeof(MUInt32);											\
	else if ((lChannelNum) == 1)												\
		dwTemp += 256*sizeof(AM_RGBQUAD);									\
	MStreamWrite(hStream, &wTemp, 2);										\
	MStreamWrite(hStream, &wTemp, 2);										\
	MStreamWrite(hStream, &dwTemp,4);										\
	JMemSet(&bi, 0, sizeof(AM_BMPINFOHEADER));								\
	bi.biSize = sizeof(AM_BMPINFOHEADER);									\
	bi.biWidth = (lWidth);													\
	bi.biHeight = (lHeight);													\
	bi.biBitCount= (MInt16)((lChannelNum)<<3);								\
	bi.biPlanes = 1;														\
	if ((lChannelNum) == 2)													\
		bi.biCompression = 3L;												\
	MStreamWrite(hStream, &bi,sizeof(AM_BMPINFOHEADER));					\
	if ((lChannelNum) == 2)													\
	{																		\
		MUInt32 mask[4] = {0xF800, 0x7E0, 0x1F, 0};							\
		MStreamWrite(hStream, mask, 4*sizeof(MUInt32));						\
	}																		\
	else if ((lChannelNum) == 1)												\
	{																		\
		AM_RGBQUAD rgb[256];												\
		for (i=0; i<256; i++)												\
		{																	\
			rgb[i].rgbBlue	= rgb[i].rgbGreen	= rgb[i].rgbRed	= (MUInt8)i;	\
			rgb[i].rgbReserved	= 0;										\
		}																	\
		MStreamWrite(hStream, rgb,256*sizeof(AM_RGBQUAD));					\
	}																		\
	for (i=0; i<(lHeight); i++, p-=lImgLine)								\
	{																		\
		for(j=0; j<(lWidth)*(lChannelNum); j++)								\
		{																	\
			lTmp = (((p[j])*(lZoom)+NORMAL_ZOOM_HALF)>>NORMAL_ZOOM_SHIFT)+(lOffSet);	\
			byTmp = TRIM_UINT8(lTmp);										\
			MStreamWrite(hStream, &byTmp,1);								\
		}																	\
		MStreamWrite(hStream, p+j,lImgExt);									\
	}																		\
	if(hStream != MNull)													\
		fclose(hStream);												\
}



MVoid PrintBmpEx(const MVoid *pImgData, MInt32 lImgLine, JTYPE_DATA_A typeDataA,
			   MInt32 lWidth, MInt32 lHeight, MInt32 lChannelNum, const MTChar* szBmpName)
{
	switch(typeDataA) {
	case DATA_U8:
		SAVETOBMP((MUInt8*)pImgData, lImgLine, lWidth, lHeight, lChannelNum, szBmpName);
		break;
	case DATA_I8:
		SAVETOBMP_EX(pImgData, lImgLine, lWidth, lHeight, lChannelNum, 128, NORMAL_ZOOM*4, szBmpName, MInt8);
		break;
	case DATA_I16:
		SAVETOBMP_EX(pImgData, lImgLine, lWidth, lHeight, lChannelNum, 0, NORMAL_ZOOM, szBmpName, MInt16);
		break;
	case DATA_U16:
		SAVETOBMP_EX(pImgData, lImgLine, lWidth, lHeight, lChannelNum, 0, NORMAL_ZOOM, szBmpName, MUInt16);
		break;
	case DATA_U14:
		SAVETOBMP_EX(pImgData, lImgLine, lWidth, lHeight, lChannelNum, 0, NORMAL_ZOOM>>6, szBmpName, MUInt16);
		break;
	default:
		JASSERT(MFalse);
		break;
	}
}

MVoid PrintBmp(const MVoid *pImgData, MInt32 lImgLine, JTYPE_DATA_A typeDataA,
		MInt32 lWidth, MInt32 lHeight, MInt32 lChannelNum)
{
//	PrintBmpEx(pImgData, lImgLine, typeDataA, lWidth, lHeight, lChannelNum, "D:\\li_debug.bmp");
}
#endif	//ENABLE_DEBUG

//////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_LOG
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef PLATFORM_SOFTUNE
extern MLong WriteLogFile(const char* path, const MVoid* data, MDWord size);
#define PRINT_STR(str, filename)	WriteLogFile("SD:\\DSC00001.txt", str, strlen(str))
#elif defined PLATFORM_COACH
extern int write_file( const void* data, unsigned long size, const char* path );
#define PRINT_STR(str, filename)	write_file(str, strlen(str),"I:\\DSC00001.txt");
#else
#define PRINT_STR(str, filename)											\
{																			\
	FILE*fp = fopen(filename, "a+");										\
	fprintf(fp, str);														\
	fclose(fp);																\
}
#endif

MVoid	Logger(MTChar *szFormat, ...)
{
	MTChar *szLogName = "C:\\logger.txt";
	MTChar buf[1024];

	va_list ap;	
	va_start(ap, szFormat);
	vsprintf(buf, szFormat, ap);
	va_end(ap);

	PRINT_STR(buf, szLogName);
	JPrintf(buf);
}

// MVoid	MemLogger(MHandle hMemMgr)
// {
// #ifdef PLATFORM_COACH
// 	AMMemShowUsedInfo(hMemMgr);
// #else
// 	MDWord dwTotal=0, dwUsed=0;
// 	MMemInfoStatic(hMemMgr, &dwTotal, &dwUsed);
// 	Logger("Total memory %d K, Used memory %d K\n", dwTotal, dwUsed);
// #endif
// }

#endif	//ENABLE_LOG

#ifdef ENABLE_PRINT
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

MVoid	JPrintf(MTChar *szFormat, ...)
{
	MTChar buf[1024];
	
	va_list ap;	
	va_start(ap, szFormat);
	vsprintf(buf, szFormat, ap);
	va_end(ap);

#ifdef PLATFORM_COACH
	Printf_B(buf);
#elif defined PLATFORM_GREENHILLS
	dbg_printf(buf); 
#else
	printf(buf);
#endif

#endif	//ENABLE_LOG

//===============================================
#ifdef ENABLE_ERR_PRINT
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

	MVoid	Err_Print(MTChar *szFormat, ...)
	{
		MTChar buf[1024];

		va_list ap;	
		va_start(ap, szFormat);
		vsprintf(buf, szFormat, ap);
		va_end(ap);

#ifdef PLATFORM_COACH
		Printf_B(buf);
#elif defined PLATFORM_GREENHILLS
		dbg_printf(buf); 
#else
		printf(buf);
#endif
//===============================================

}
#endif