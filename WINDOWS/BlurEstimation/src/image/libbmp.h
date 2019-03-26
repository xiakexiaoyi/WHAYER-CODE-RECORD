
#if !defined(_LI_BBMP_H_)
#define _LI_BBMP_H_

typedef struct tagAM_BMPINFOHEADER
{
    unsigned long	biSize;
    long			biWidth;
    long			biHeight;
    unsigned short	biPlanes;
    unsigned short	biBitCount;
    unsigned long	biCompression;
    unsigned long	biSizeImage;
    long			biXPelsPerMeter;
    long			biYPelsPerMeter;
    unsigned long	biClrUsed;
    unsigned long	biClrImportant;
} AM_BMPINFOHEADER;

typedef struct tagAM_RGBQUAD
{
    unsigned char	rgbBlue;
    unsigned char	rgbGreen;
    unsigned char	rgbRed;
    unsigned char	rgbReserved;
} AM_RGBQUAD, *AM_LPRGBQUAD;

#include "amcomdef.h"
#ifdef __cplusplus
extern "C" {
#endif		

    //Load / Save bmp
    long LoadBmp(char *szFile, unsigned char **pBuffer, long *nWidth, long *nHeight, long *nBitCount);
    long savetobmp(char* szFile, unsigned char *pData, long nWidth, long nHeight, long nBitCount);
    //Basic functions
    void SplitPathName(char* szPathName, 
        char *szPath, char *szName, char *szExt);

#ifdef __cplusplus
}
#endif	

#endif // !defined(_LI_BBMP_H_)