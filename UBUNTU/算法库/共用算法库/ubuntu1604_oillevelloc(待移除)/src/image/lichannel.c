#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "lichannel.h"

#include "litrimfun.h"
#include "liimage.h"
#include "limath.h"
#include "lidebug.h"
#include "lierrdef.h"
#include "limem.h"

#ifdef TRIM_RGB
#include "lirgb_yuv.h"
#endif

#ifdef OPTIMIZATION_SSE
#include "lichannel_sse.h"
#endif

#ifdef OPTIMIZATION_ARM
#include "lichannel_arm.h"
#endif

#define GET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, dwChannelNum,		\
	pChannel, dwChannelLine, dwChannelCur, TYPE)							\
{																			\
	MDWord y, x;															\
	MDWord dwImgExt = dwImgLine - dwWidth*dwChannelNum;					\
	MDWord dwChannelExt = dwChannelLine - dwWidth;							\
	TYPE *pImgCur=(TYPE*)pImg+dwChannelCur, *pChannelCur=(TYPE*)pChannel;	\
	for(y=0; y<dwHeight; y++, pImgCur+=dwImgExt, pChannelCur+=dwChannelExt)	\
	{																		\
		for(x=0; x<dwWidth; x++, pImgCur+=dwChannelNum, pChannelCur++)		\
		{																	\
			*pChannelCur = *pImgCur;										\
		}																	\
	}																		\
}

#define SET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, dwChannelNum,		\
	pChannel, dwChannelLine, dwChannelCur, TYPE)							\
{																			\
	MDWord y, x;															\
	MDWord dwImgExt = dwImgLine - dwWidth*dwChannelNum;					\
	MDWord dwChannelExt = dwChannelLine - dwWidth;							\
	TYPE *pImgCur=(TYPE*)pImg+dwChannelCur, *pChannelCur=(TYPE*)pChannel;	\
	for(y=0; y<dwHeight; y++, pImgCur+=dwImgExt, pChannelCur+=dwChannelExt)	\
	{																		\
		for(x=0; x<dwWidth; x++, pImgCur+=dwChannelNum, pChannelCur++)		\
		{																	\
			*pImgCur = *pChannelCur;										\
		}																	\
	}																		\
}


////////////////////////////////////////////////////////////////////////////
#ifndef	TRIM_ONLY_LUMIN_CHANNEL 
MVoid	AccessChannel_C1YC2Y(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet)
{
#ifdef OPTIMIZATION_ARM
	AccessChannel_C1YC2Y_Arm(pImg, dwImgLine, 
							 pY, dwYLine, pC1, dwC1Line, pC2, dwC2Line, 
							 dwWidth, dwHeight, bGet);
#else	//OPTIMIZATION_ARM
	MDWord x, y;		
	MDWord dwImgExt = dwImgLine - dwWidth*2;
	MDWord dwC1Ext = dwC1Line - dwWidth/2;
	MDWord dwC2Ext = dwC2Line - dwWidth/2;
	MDWord dwYExt = dwYLine - dwWidth;
	JASSERT(dwWidth % 2 == 0);
	dwWidth/=2;
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--)
		{	
			for(x=dwWidth; x!=0; x--)
			{
				*pC1++ = *pImg++;
				*pY++  = *pImg++;
				*pC2++ = *pImg++;
				*pY++  = *pImg++;
			}
			pImg += dwImgExt, pY += dwYExt;
			pC1 += dwC1Ext, pC2 += dwC2Ext;
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--)
		{
			for(x=dwWidth; x!=0; x--)
			{
				*pImg++ = *pC1++;
				*pImg++ = *pY++;
				*pImg++ = *pC2++;
				*pImg++ = *pY++;
			}
			pImg += dwImgExt, pY += dwYExt;
			pC1 += dwC1Ext, pC2 += dwC2Ext;
		}
	}
#endif	//OPTIMIZATION_ARM
}

#define _ACCESSCHANNEL_YC1C2(pImg, dwImgLine, pY, dwYLine, pC1, dwC1Line,	\
	pC2, dwC2Line, dwWidth, dwHeight, bGet, lZoom, TYPE_IMG, TYPE_CHL)		\
{																			\
	MDWord x, y;															\
	MDWord dwImgExt = dwImgLine - dwWidth*3;								\
	MDWord dwC1Ext = dwC1Line - dwWidth;									\
	MDWord dwC2Ext = dwC2Line - dwWidth;									\
	MDWord dwYExt = dwYLine - dwWidth;										\
	TYPE_IMG *pImgCur = (TYPE_IMG*)pImg;									\
	TYPE_CHL *pYCur = (TYPE_CHL*)pY;										\
	TYPE_CHL *pC1Cur = (TYPE_CHL*)pC1;										\
	TYPE_CHL *pC2Cur = (TYPE_CHL*)pC2;										\
	if(bGet)																\
	{																		\
		if(lZoom >= 0)														\
		{																	\
			for(y=dwHeight; y!=0; y--)										\
			{																\
				for(x=dwWidth; x!=0; x--)									\
				{															\
					*pYCur++  = (TYPE_CHL)(*pImgCur++ << lZoom);			\
					*pC1Cur++ = (TYPE_CHL)(*pImgCur++ << lZoom);			\
					*pC2Cur++ = (TYPE_CHL)(*pImgCur++ << lZoom);			\
				}															\
				pImgCur += dwImgExt, pYCur += dwYExt;						\
				pC1Cur += dwC1Ext, pC2Cur += dwC2Ext;						\
			}																\
		}																	\
		else																\
		{																	\
			MLong lZoom2 = -(lZoom);										\
			for(y=dwHeight; y!=0; y--)										\
			{																\
				for(x=dwWidth; x!=0; x--)									\
				{															\
					*pYCur++  = (TYPE_CHL)DOWN_ROUND(*pImgCur++,lZoom2);	\
					*pC1Cur++ = (TYPE_CHL)DOWN_ROUND(*pImgCur++,lZoom2);	\
					*pC2Cur++ = (TYPE_CHL)DOWN_ROUND(*pImgCur++,lZoom2);	\
				}															\
				pImgCur += dwImgExt, pYCur += dwYExt;						\
				pC1Cur += dwC1Ext, pC2Cur += dwC2Ext;						\
			}																\
		}																	\
	}																		\
	else																	\
	{																		\
		if(lZoom >= 0)														\
		{																	\
			for(y=dwHeight; y!=0; y--)										\
			{																\
				for(x=dwWidth; x!=0; x--)									\
				{															\
					*pImgCur++ = (TYPE_IMG)DOWN_ROUND(*pYCur++, lZoom);		\
					*pImgCur++ = (TYPE_IMG)DOWN_ROUND(*pC1Cur++, lZoom);	\
					*pImgCur++ = (TYPE_IMG)DOWN_ROUND(*pC2Cur++, lZoom);	\
				}															\
				pImgCur += dwImgExt, pYCur += dwYExt;						\
				pC1Cur += dwC1Ext, pC2Cur += dwC2Ext;						\
			}																\
		}																	\
		else																\
		{																	\
			MLong lZoom2 = -(lZoom);										\
			for(y=dwHeight; y!=0; y--)										\
			{																\
				for(x=dwWidth; x!=0; x--)									\
				{															\
					*pImgCur++ = (TYPE_IMG)(*pYCur++ << lZoom2);			\
					*pImgCur++ = (TYPE_IMG)(*pC1Cur++ << lZoom2);			\
					*pImgCur++ = (TYPE_IMG)(*pC2Cur++ << lZoom2);			\
				}															\
				pImgCur += dwImgExt, pYCur += dwYExt;						\
				pC1Cur += dwC1Ext, pC2Cur += dwC2Ext;						\
			}																\
		}																	\
	}																		\
}
MVoid	AccessChannel_YC1C2(MVoid *pImg, MDWord dwImgLine, JTYPE_DATA_A typeImg, 
							MVoid *pY, MDWord dwYLine, MVoid *pC1, MDWord dwC1Line, 
							MVoid *pC2, MDWord dwC2Line, JTYPE_DATA_A typeChl, 
							MDWord dwWidth, MDWord dwHeight, MBool bGet)
{
	if(typeChl==DATA_U8 && typeImg==DATA_U8)
	{
		_ACCESSCHANNEL_YC1C2(pImg, dwImgLine, pY, dwYLine, pC1, dwC1Line, 
			pC2, dwC2Line, dwWidth, dwHeight, bGet, 0, MUInt8, MUInt8);
	}
	else if(typeChl==DATA_U16 && typeImg == DATA_U14)
	{
#ifdef OPTIMIZATION_SSE
		AccessChannel_YC1C2_U16_SSE2((MUInt16*)pImg, dwImgLine, 
			(MUInt16*)pY, dwYLine, (MUInt16*)pC1, dwC1Line, 
			(MUInt16*)pC2, dwC2Line, dwWidth, dwHeight, 2, bGet);
#else
		_ACCESSCHANNEL_YC1C2(pImg, dwImgLine, pY, dwYLine, pC1, dwC1Line, 
			pC2, dwC2Line, dwWidth, dwHeight, bGet, 2, MUInt16, MUInt16);
#endif
	}
	else if(typeChl==DATA_U8 && typeImg == DATA_U14)
	{
		_ACCESSCHANNEL_YC1C2(pImg, dwImgLine, pY, dwYLine, pC1, dwC1Line, 
			pC2, dwC2Line, dwWidth, dwHeight, bGet, 6, MUInt16, MUInt8);
	}
	else
	{
		JASSERT(MFalse);
	}
}

MVoid	AccessChannel_YC1YC2(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet)
{
#ifdef REDUCE_COLOR_SIZE
	AccessChannel_YC1YC2_Reduced_Arm(pImg, dwImgLine, 
							 pY, dwYLine, pC1, dwC1Line, pC2, dwC2Line, 
							 dwWidth, dwHeight, bGet);
	return;
#endif	//REDUCE_COLOR_SIZE
	
#ifdef OPTIMIZATION_SSE
	AccessChannel_YC1YC2_SSE2(pImg, dwImgLine, pY, dwYLine, 
		pC1, dwC1Line, pC2, dwC2Line, dwWidth, dwHeight, bGet);
#elif defined OPTIMIZATION_ARM
	AccessChannel_YC1YC2_Arm(pImg, dwImgLine, 
							 pY, dwYLine, pC1, dwC1Line, pC2, dwC2Line, 
							 dwWidth, dwHeight, bGet);
#else
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	MLong lC1Ext = dwC1Line - dwWidth/2;
	MLong lC2Ext = dwC2Line - dwWidth/2;
	MLong x, y;
	JASSERT(dwWidth%2 == 0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY+=lYExt, pC1+=lC1Ext, pC2+=lC2Ext)
		{
			for(x=dwWidth/2; x!=0; x--)
			{
				*pY++	= *pImg++;
				*pC1++	= *pImg++;
				*pY++	= *pImg++;
				*pC2++	= *pImg++;
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY+=lYExt, pC1+=lC1Ext, pC2+=lC2Ext)
		{
			for(x=dwWidth/2; x!=0; x--)
			{
				*pImg++ = *pY++;
				*pImg++ = *pC1++;
				*pImg++ = *pY++;
				*pImg++ = *pC2++;
			}
		}
	}
#endif
}

MVoid	AccessChannel_Y8C44(MByte *pImg, MDWord dwImgLine, 
							MByte *pY, MDWord dwYLine, 
							MByte *pC1, MDWord dwC1Line, 
							MByte *pC2, MDWord dwC2Line, 
							MDWord dwWidth, MDWord dwHeight, 
							MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	MLong lC1Ext = dwC1Line - dwWidth/2;
	MLong lC2Ext = dwC2Line - dwWidth/2;
	JASSERT((MDWord)pImg % 4 == 0 && dwImgLine%4==0);
	JASSERT((MDWord)pY%4==0 && dwYLine%4==0);
	JASSERT((MDWord)pC1%4==0 && dwC1Line%4==0);
	JASSERT((MDWord)pC2%4==0 && dwC2Line%4==0);
	JASSERT(dwWidth%8==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY += lYExt, pC1 += lC1Ext, pC2 += lC2Ext)
		{
			for(x=dwWidth/8; x!=0; x--, pImg+=16, 
				pY += 8, pC1 += 4, pC2 += 4)
			{
				MDWord y0 = ((MDWord*)pImg)[0];
				MDWord y1 = ((MDWord*)pImg)[1];
				MDWord u = ((MDWord*)pImg)[2];
				MDWord v = ((MDWord*)pImg)[3];
				((MDWord*)pY)[0] = y0;
				((MDWord*)pY)[1] = y1;
				((MDWord*)pC1)[0] = u;
				((MDWord*)pC2)[0] = v;

			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY += lYExt, pC1 += lC1Ext, pC2 += lC2Ext)
		{
			for(x=dwWidth/8; x!=0; x--, pImg+=16, 
				pY += 8, pC1 += 4, pC2 += 4)
			{
				MDWord y0 = ((MDWord*)pY)[0];
				MDWord y1 = ((MDWord*)pY)[1];
				MDWord u = ((MDWord*)pC1)[0];
				MDWord v = ((MDWord*)pC2)[0];
				((MDWord*)pImg)[0] = y0;
				((MDWord*)pImg)[1] = y1;
				((MDWord*)pImg)[2] = u;
				((MDWord*)pImg)[3] = v;

			}
		}
	}
}
#ifdef TRIM_RGB565
MVoid	AccessChannel_RGB565(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet)
{
	MDWord x, y;
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=dwImgLine, 
			pY+=dwYLine, pC1+=dwC1Line, pC2+=dwC2Line)
		{
			for(x=0; x<dwWidth; x++)
			{
				MDWord rgb = *((MUInt16*)pImg + x);
				pY[x] =  (MByte)((rgb >> 3) & 0xFC);
				pC1[x] = (MByte)((rgb >> 8) & 0xF8);
				pC2[x] = (MByte)((rgb << 3) );
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=dwImgLine, 
			pY+=dwYLine, pC1+=dwC1Line, pC2+=dwC2Line)
		{
			for(x=0; x<dwWidth; x++)
			{
				*((MUInt16*)pImg + x) = (MUInt16)((pC2[x]>>3) 
					| ((pY[x]>>2) << 5)
					| ((pC1[x]>>3) << 11));
			}
		}
	}
}
#endif	//TRIM_RGB565

MVoid	AccessChannel_C1C2(MByte *pC1C2, MDWord dwImgLine, 
						   MByte *pC1, MDWord dwC1Line, 
						   MByte *pC2, MDWord dwC2Line, 
						   MDWord dwWidth, MDWord dwHeight, 
						   MBool bGet)
{
#ifdef OPTIMIZATION_ARM
	AccessChannel_C1C2_Arm(pC1C2, dwImgLine, pC1, dwC1Line, pC2, dwC2Line, 
		dwWidth, dwHeight, bGet);
#else	//OPTIMIZATION_ARM
	MDWord x,y;	
	MDWord dwImgExt = dwImgLine-dwWidth*2;
	MDWord dwC1Ext = dwC1Line - dwWidth;
	MDWord dwC2Ext = dwC2Line - dwWidth;
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--)
		{	
			for(x=dwWidth; x!=0; x--)
			{
				*pC1++ = *pC1C2++;
				*pC2++ = *pC1C2++;
			}
			pC1C2 += dwImgExt;
			pC1 += dwC1Ext, pC2 += dwC2Ext;
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--)
		{	
			for(x=dwWidth; x!=0; x--)
			{
				*pC1C2++ = *pC1++;
				*pC1C2++ = *pC2++;
			}
			pC1C2 += dwImgExt;
			pC1 += dwC1Ext, pC2 += dwC2Ext;
		}
	}
#endif //OPTIMIZATION_ARM
}
#endif //TRIM_ONLY_LUMIN_CHANNEL

//////////////////////////////////////////////////////////////////////////
MRESULT   AccessChannel(MVoid *pImg, MDWord dwImgLine, 
						MVoid *pChannel, MDWord dwChannelLine, 
						MDWord dwWidth, MDWord dwHeight, 
						MLong lChannelNum, MLong lChannelCur, 
						JTYPE_DATA typeData, MBool bGet)
{
#ifdef OPTIMIZATION_ARM
	if(IF_DATA_BYTES(typeData)==1 && lChannelNum==2 && lChannelCur==0)
	{
		AccessLuminChannel_YUYV_Arm((MByte*)pImg, dwImgLine, 
			(MByte*)pChannel, dwChannelLine, dwWidth, dwHeight, bGet);
		return LI_ERR_NONE;
	}
#endif
	
	switch(IF_DATA_BYTES(typeData)) {
	case 1:
		if(bGet)
		{
			GET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, 
				lChannelNum, pChannel, dwChannelLine, 
				lChannelCur, MByte);
		}
		else
		{
			SET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, 
				lChannelNum, pChannel, dwChannelLine, 
				lChannelCur, MByte);
		}
		break;
	case 2:
		if(bGet)
		{
			GET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, 
				lChannelNum, pChannel, dwChannelLine, 
				lChannelCur, MUInt16);
		}
		else
		{
			SET_CHANNEL(pImg, dwWidth, dwHeight, dwImgLine, 
				lChannelNum, pChannel, dwChannelLine, 
				lChannelCur, MUInt16);
		}
		break;
	case DATA_NONE:
	default:
		JASSERT(MFalse);
		return LI_ERR_DATA_UNSUPPORT;
	}
	//PrintBmp(pChannel, dwChannelLine, typeData, 
	//	dwWidth, dwHeight, 1);
	return LI_ERR_NONE;
}

MVoid	AccessLuminChannel_YC1YC2(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet)
{
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	MLong x, y;
	JASSERT(dwWidth%2 == 0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--)
			{
				*pY++	= *pImg++;
				pImg++;
				*pY++	= *pImg++;
				pImg++;
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, 
			pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--)
			{
				*pImg++ = *pY++;
				pImg++;
				*pImg++ = *pY++;
				pImg++;
			}
		}
	}
}

MVoid	AccessLuminChannel_Y8C44(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	JASSERT((MDWord)pImg%4 == 0 && dwImgLine%4==0);
	JASSERT((MDWord)pY%4==0 && dwYLine%4==0);
	JASSERT(dwWidth%8==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/8; x!=0; x--, pImg+=16, pY+=8)
			{
				*((MDWord*)pY) = *((MDWord*)pImg);
				*((MDWord*)pY+1) = *((MDWord*)pImg+1);
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/8; x!=0; x--, pImg+=16, pY+=8)
			{
				*((MDWord*)pImg) = *((MDWord*)pY);
				*((MDWord*)pImg+1) = *((MDWord*)pY+1);
			}
		}
	}
}

MVoid	AccessLuminChannel_Y1VY0U(MByte *pImg, MDWord dwImgLine, 
								  MByte *pY, MDWord dwYLine, 
								  MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	JASSERT(dwWidth%2==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--, pImg+=4, pY+=2)
			{
				pY[0] = pImg[2];
				pY[1] = pImg[0];
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--, pImg+=4, pY+=2)
			{
				pImg[2] = pY[0];
				pImg[0] = pY[1];
			}
		}
	}
}

MVoid	AccessLuminChannel_Y2C11(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	JASSERT((MDWord)pImg%4 == 0 && dwImgLine%4==0);
	JASSERT((MDWord)pY%4==0 && dwYLine%4==0);
	JASSERT(dwWidth%2==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--, pImg+=4, pY+=2)
			{
				*((MWord*)pY) = *((MWord*)pImg);
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth/2; x!=0; x--, pImg+=4, pY+=2)
			{
				*((MWord*)pImg) = *((MWord*)pY);
			}
		}
	}
}

#ifdef TRIM_RGB
MVoid	AccessLuminChannel_RGB888(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*3;
	MLong lYExt = dwYLine - dwWidth;
// 	JASSERT((MDWord)pImg%4 == 0 && dwImgLine%4==0);
// 	JASSERT((MDWord)pY%4==0 && dwYLine%4==0);
	JASSERT(dwWidth%2==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=3, pY++)
			{
				MLong lY = 9798*pImg[0] + 19235*pImg[1] + 3736*pImg[2];
				*pY = (MByte)DOWN_ROUND(lY, 15);
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=3, pY++)
			{
				MLong lY = 9798*pImg[0] + 19235*pImg[1] + 3736*pImg[2];
				MLong deltY = *pY - DOWN_ROUND(lY, 15);
                if (deltY == 0)
                    continue;
				pImg[0] = TRIM_UINT8((MLong)(pImg[0]) + deltY);
				pImg[1] = TRIM_UINT8((MLong)(pImg[1]) + deltY);
				pImg[2] = TRIM_UINT8((MLong)(pImg[2]) + deltY);
			}
		}
	}
}

MVoid	AccessLuminChannel_BGR888(MByte *pImg, MDWord dwImgLine, 
								  MByte *pY, MDWord dwYLine, 
								  MDWord dwWidth, MDWord dwHeight, 
								  MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*3;
	MLong lYExt = dwYLine - dwWidth;
	// 	JASSERT((MDWord)pImg%4 == 0 && dwImgLine%4==0);
	// 	JASSERT((MDWord)pY%4==0 && dwYLine%4==0);
	JASSERT(dwWidth%2==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=3, pY++)
			{
				MLong lY = 9798*pImg[2] + 19235*pImg[1] + 3736*pImg[0];
				*pY = (MByte)DOWN_ROUND(lY, 15);
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=3, pY++)
			{
				MLong lY = 9798*pImg[2] + 19235*pImg[1] + 3736*pImg[0];
				MLong deltY = *pY - DOWN_ROUND(lY, 15);
                if (deltY == 0)
                    continue;
                pImg[0] = TRIM_UINT8((MLong)(pImg[0]) + deltY);
                pImg[1] = TRIM_UINT8((MLong)(pImg[1]) + deltY);
                pImg[2] = TRIM_UINT8((MLong)(pImg[2]) + deltY);
                
			}
		}
	}
}
MVoid	AccessLuminChannel_RGB565(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet)
{
	MLong x, y;
	MLong lImgExt = dwImgLine - dwWidth*2;
	MLong lYExt = dwYLine - dwWidth;
	JASSERT(dwWidth%2==0);
	if(bGet)
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=2, pY++)
			{
                MDWord cr_tmp = *((MWord*)pImg),
                    cr_r = ((cr_tmp >> 11) & 0x001F) << 3,
                    cr_g = ((cr_tmp >>  5) & 0x003F) << 2,
                    cr_b = (cr_tmp & 0x001F) << 3;
				*pY = (MByte)DOWN_ROUND(9798*cr_r + 19235*cr_g + 3736*cr_b, 15);
			}
		}
	}
	else
	{
		for(y=dwHeight; y!=0; y--, pImg+=lImgExt, pY+=lYExt)
		{
			for(x=dwWidth; x!=0; x--, pImg+=2, pY++)
			{
                MDWord cr_tmp = *((MWord*)pImg),
                    cr_r = ((cr_tmp >> 11) & 0x001F) << 3,
                    cr_g = ((cr_tmp >>  5) & 0x003F) << 2,
                    cr_b = (cr_tmp & 0x001F) << 3;
				MLong deltY = *pY - DOWN_ROUND(9798*cr_r + 19235*cr_g + 3736*cr_b, 15);
                if (deltY == 0)
                    continue;
                cr_r = TRIM_UINT8((MLong)cr_r + deltY) >> 3;
                cr_g = TRIM_UINT8((MLong)cr_g + deltY) >> 2;
                cr_b = TRIM_UINT8((MLong)cr_b + deltY) >> 3;
                cr_tmp = (cr_r << 11) | (cr_g << 5) | cr_b;
                *((MWord*)pImg) = (MWord)cr_tmp;
			}
		}
	}
}
#endif	//TRIM_RGB
