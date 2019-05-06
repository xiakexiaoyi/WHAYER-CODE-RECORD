#ifndef _RESAMPLE_H_
#define _RESAMPLE_H_

#include "licomdef.h"
//#include "litrimfun.h"

//#define Resample				 PX(Resample)
//#define ResampleImage			 PX(ResampleImage)
//#define ResampleImage_YUV        PX(ResampleImage_YUV)
//#define ResampleChannelImage     PX(ResampleChannelImage)

#ifdef __cplusplus
extern "C" {
#endif	

MRESULT Resample(MHandle hMemMgr, JOFFSCREEN* pSrcImg, JOFFSCREEN* pDesImg, MLong mode);
	

MRESULT ResampleImage(MHandle hContext,long srcWidth, long srcHeight, long srcLine, long dstWidth, long dstHeight,
					  long dstLine, unsigned char *srcImg, unsigned char *dstImg, int mode);
MRESULT ResampleImage_YUV(MHandle hContext, unsigned char *srcImg, long srcWidth, long srcHeight,
								unsigned char *dstImg, long dstWidth, long dstHeight);
MRESULT ResampleChannelImage(long srcHei, long srcWid, long srcPitch, unsigned char *srcImg,
						 long dstHei, long dstWid, long dstPitch, unsigned char *dstImg, long mode);
	
					  
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_RESAMPLE_H_
