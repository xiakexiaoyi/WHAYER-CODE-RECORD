#include "wyVACommon.h"

#ifndef _WYVAFUNC_
#define _WYVAFUNC_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

int bitMapToByteMap_wyVA(unsigned char *bitMap, unsigned char *byteMap, unsigned int width, unsigned int heigth);
void findRecPosOfObjInMaskPic_wyVA(wyVA_picBufA *maskData, int *left, int *right, int *top, int *bottom);
void cutObjRecFrmPic_wyVA(unsigned char *src, unsigned char *dst, int top, int bot, int left ,int right, int strideSrc, int strideDst);

int smoothFilter_wyVA(unsigned char *src, unsigned char *dst, int width, int height, int strideSrc, int strideDst);

void picMainDirectionStep1_wyVA(wyVA_picBufA *pic, int *lx, int *ly,
	                                                        int *number, WYVA_FLOAT *sumX, WYVA_FLOAT *sumY);

void picMainDirectionStep2_wyVA(int *lx, int *ly, WYVA_FLOAT *mat,
	                                                  WYVA_FLOAT meanX, WYVA_FLOAT meanY, int number);

void picMainDirection_wyVA(wyVA_picBuf *pic, int *angle, int *lx, int *ly);
void rotate90_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut);
void rotate180_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut);
void rotate270_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut);
void rotateX_wyVA(wyVA_picBuf *picIn, wyVA_picBufA *picOut, int *xABC, int *yABC);
void scalorBLinear_wyVA(wyVA_picBufA *picIn, wyVA_picBufA *picOut,  int nXFactorInt, int nYFactorInt);

//void scalorBLinear_wyVAD(wyVA_picBufA *picIn, wyVA_picBufA *picOut, int sh, WYVA_FLOAT nXFactor, WYVA_FLOAT nYFactor);

void scalorDesBLinear_wyVA(wyVA_picBufA *picIn, wyVA_picBufA *picOut, int nXFactorInt, int nYFactorInt);
int refineDes_wyVA(unsigned char *pDesData, int desW, int desH, unsigned char *maskData, int maskStride);
int findMaxIn5x5Matrix_wyVA(unsigned short *pMag, int magStride, int *xPos, int *yPos);
void getDesBinMatrix(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, 
	                                           int stride, int width, int height, WYVA_FLOAT magRate);
void getMaxBinMatrix(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, 
	                                           int stride, int width, int height);
void getDesBinIn5x5Matrix_wyVA(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, int stride, WYVA_FLOAT magRate);
void getmaxBinIn5x5Matrix_wyVA(unsigned char *bin, unsigned char *pAngle, unsigned short *pMag, int stride);

int calAngleAndMagMatrix_wyVA(unsigned int USrc, unsigned int UAngle,unsigned int UMag, int widthSrc, 
                                                                      int height, int strideSrc, int strideDst);
int desMatchProcess_wyVA(int maxW, int maxH, unsigned char *maxBin, int desW, int desH, 
                                        unsigned char *desBin, int numMatched, int *retMatchNum);

											   

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif







