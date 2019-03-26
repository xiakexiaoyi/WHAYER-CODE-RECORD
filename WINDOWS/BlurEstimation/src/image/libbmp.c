
#include "libbmp.h"


#include "libbmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define JLINE_BYTES(Width, BitCt)    (((long)(Width) * (BitCt) + 31) / 32 * 4)


#ifndef JBIG_ENDIAN
#define CORRECT_ENDIAN_INT(x);
#define CORRECT_ENDIAN_SHORT(x);
#else
#define CORRECT_ENDIAN_INT(x)	((x) = (((MDWord)(x)) >> 24) | ((((MDWord)(x)) & 0x00ff0000) >> 8) | ((((MDWord)(x)) & 0x0000ff00) << 8) | ((((MDWord)(x)) & 0x000000ff) << 24))
#define CORRECT_ENDIAN_SHORT(x)	((x) = (((MWord)(x)) >> 8) | ((((MWord)(x)) & 0x00ff) << 8))
#endif
MVoid Correct_Endian_BMPHeader(AM_BMPINFOHEADER *pbi)
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

long savetobmp(char* szFile, unsigned char *pData, long nWidth, 
    long nHeight, long nBitCount)
{
    FILE * stream = 0;
    unsigned char *p;
    unsigned short wTemp;
    unsigned long dwTemp;
    AM_BMPINFOHEADER bi;
    long nLineBytes, i;

    stream = fopen(szFile, "wb+");
    if(!stream)
        return -1;

    nLineBytes = JLINE_BYTES(nWidth, nBitCount);
    wTemp = 0x4D42;
    dwTemp = 54 + nLineBytes * nHeight;
    if(nBitCount == 16)
        dwTemp += 4*sizeof(unsigned long);
    else if(nBitCount == 8)
        dwTemp += 256*sizeof(AM_RGBQUAD);
    CORRECT_ENDIAN_SHORT(wTemp);
    fwrite(&wTemp,1, 2,stream);
    CORRECT_ENDIAN_INT(dwTemp);
    fwrite(&dwTemp,1, 4,stream);
    wTemp = 0;
    dwTemp = 54;
    if(nBitCount == 16)
        dwTemp += 4*sizeof(unsigned long);
    else if(nBitCount == 8)
        dwTemp += 256*sizeof(AM_RGBQUAD);
    CORRECT_ENDIAN_SHORT(wTemp);
    fwrite( &wTemp, 1,2,stream);
    fwrite(&wTemp,1 ,2,stream);
    CORRECT_ENDIAN_INT(dwTemp);
    fwrite(&dwTemp,1, 4,stream);
    memset(&bi, 0, sizeof(AM_BMPINFOHEADER));
    bi.biSize = sizeof(AM_BMPINFOHEADER);
    bi.biWidth = nWidth;
    bi.biHeight = nHeight;
    bi.biBitCount= (unsigned short)nBitCount;
    bi.biPlanes = 1;
    if(nBitCount == 16)
        bi.biCompression = 3L;
    Correct_Endian_BMPHeader(&bi);
    fwrite(&bi,1, sizeof(AM_BMPINFOHEADER),stream);

    if(nBitCount == 16)
    {
        unsigned long mask[4] = {0xF800, 0x7E0, 0x1F, 0};
        fwrite(mask, 4, sizeof(unsigned long), stream);
    }
    else if(nBitCount == 8)
    {
        AM_RGBQUAD rgb[256];
        for(i=0; i<256; i++)
        {
            rgb[i].rgbBlue	= (unsigned char)i;
            rgb[i].rgbGreen	= (unsigned char)i;
            rgb[i].rgbRed	= (unsigned char)i;
            rgb[i].rgbReserved	= 0;
        }
        fwrite(rgb, 256, sizeof(AM_RGBQUAD), stream);
    }
    p = pData + (nHeight - 1) * nLineBytes;
    for(i=0; i<nHeight; i++, p-=nLineBytes)
        fwrite(p,1, nLineBytes,stream);

    fclose(stream);
    return 0;
}

long LoadBmp(char *szFile, unsigned char **pBuffer, long *lWidth, 
    long *lHeight, long *nBitCount)
{
    long nOffset, nLineBytes, i,nHeight;
    unsigned char *p;
    unsigned short wTemp;
    AM_BMPINFOHEADER bi;

    long lRet = -1;
    FILE * stream=0;
    unsigned char *pImageData = 0;
    stream = fopen(szFile, "rb");
    if(stream == MNull)	
        return lRet;
    fread( &wTemp,1, 2,stream);
    CORRECT_ENDIAN_SHORT(wTemp);
    if(wTemp != 0x4D42)
        goto exit;
    fseek(stream, 10, SEEK_SET);
    fread(&nOffset, 1,4,stream);
    CORRECT_ENDIAN_INT(nOffset);
    fread(&bi,1, sizeof(AM_BMPINFOHEADER),stream);
    Correct_Endian_BMPHeader(&bi);
    if(bi.biBitCount != 24 && bi.biBitCount != 16)
        goto exit;

    nHeight = bi.biHeight > 0 ? bi.biHeight : -bi.biHeight;
    *lWidth = bi.biWidth;
    *lHeight = nHeight;
    *nBitCount = bi.biBitCount;
    nLineBytes = JLINE_BYTES(bi.biWidth, bi.biBitCount);
    pImageData = (unsigned char*)malloc( nLineBytes*nHeight);
    if(!pImageData)
        goto exit;

    fseek(stream, nOffset,SEEK_SET);
    if(bi.biHeight > 0)
    {
        p = pImageData + (nHeight - 1) * nLineBytes;
        for(i=0; i<nHeight; i++, p-=nLineBytes)
            fread(p, 1, nLineBytes, stream);
    }
    else
    {
        p = pImageData;
        for(i=0; i<nHeight; i++, p+=nLineBytes)
            fread(p, 1, nLineBytes, stream);
    }	
    lRet = 0;

exit:
    *pBuffer = pImageData;
    if(stream)
        fclose(stream);
    return lRet;
}

void SplitPathName(char* szPathName, 
    char *szPath, char *szName, char *szExt)
{
    MInt32 lLength = strlen(szPathName);
    MInt32 lCur = 0, lExtPos = lLength;
    for(lCur = lLength-1; lCur>=0; lCur--)
    {
        if(szPathName[lCur] == '.')
        {
            if(szExt != MNull)
            {
                strncpy(szExt, szPathName+lCur+1, lLength-lCur-1);
                szExt[lLength-lCur-1] = 0;
            }
            lExtPos = lCur;
        }
        if(szPathName[lCur] == '\\' || szPathName[lCur] == '/')
        {
            if(szName != MNull)
            {
                strncpy(szName, szPathName+lCur+1, lExtPos-lCur-1);
                szName[lExtPos-lCur-1] = 0;
            }
            if(szPath != MNull)
            {
                strncpy(szPath, szPathName, lCur+1);
                szPath[lCur+1] = 0;
            }
            break;
        }
    }
}
