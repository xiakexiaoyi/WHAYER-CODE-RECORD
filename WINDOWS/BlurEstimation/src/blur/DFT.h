/*!
* \file DFT.h
* \brief  the function related to the fft and dft 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#ifndef _DFT_H_
#define _DFT_H_

#include "licomdef.h"
#include  "liblock.h"
/************************************************************************/
#define FFT                       PX(FFT)
#define FFT2                     PX(FFT2)
#define IFFT2                    PX(IFFT2)
#define IFFT                      PX(IFFT)
#define FFTconv                PX(FFTconv)
#define FFTADDZeros        PX(FFTADDZeros)
#define DFT                      PX(DFT)
#define DFT2                    PX(DFT2)
#define IDFT2                   PX(IDFT2)
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif	

/// \brief struct for the complex

typedef struct
{
    MDouble re;
    MDouble im;
}COMPLEX;

/// \brief  function add for complex
/// \param c1 complex 1
/// \param c2 complex2
/// \return the sum of the two complex
/// \复数的“加”
static COMPLEX Add(COMPLEX c1, COMPLEX c2)
{
    COMPLEX c;
    c.re=c1.re+c2.re;
    c.im=c1.im+c2.im;
    return c;
}

/// \复数的“减”

/// \brief  sub function  for complex
/// \param c1 complex 1
/// \param c2 complex2
/// \return the differnce of the two complex

static COMPLEX Sub(COMPLEX c1, COMPLEX c2)
{
    COMPLEX c;
    c.re=c1.re-c2.re;
    c.im=c1.im-c2.im;
    return c;
}

/// \brief  multiply  function  for complex
/// \param c1 complex 1
/// \param c2 complex2
/// \return the product of the two complex
/// \复数乘法
static COMPLEX Mul(COMPLEX c1, COMPLEX c2)
{
    COMPLEX c;
    c.re=c1.re*c2.re-c1.im*c2.im;
    c.im=c1.re*c2.im+c2.re*c1.im;
    return c;
}

MRESULT FFT(MHandle  hMemMgr,COMPLEX * TD, COMPLEX * FD, MLong power);
MRESULT IFFT(MHandle  hMemMgr,COMPLEX * TD, COMPLEX * FD, MLong power);
MRESULT FFT2 (MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD);
MRESULT IFFT2(MHandle hMemMgr,COMPLEX *TD,  MLong lWidth, MLong lHeight, COMPLEX * FD);
MRESULT FFTconv(MHandle hMemMgr,PBLOCK blockres, PBLOCK blocksrc1,PBLOCK blocksrc2);
MLong FFTPOW(MLong asrc);
MRESULT FFTADDZeros(MHandle hMemMgr,COMPLEX* TDres,COMPLEX*TDsrc,MLong power);
MRESULT DFT(MHandle  hMemMgr,COMPLEX * TD, COMPLEX * FD,MLong width,MLong sign);
MRESULT DFT2 (MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD);
MRESULT IDFT2 (MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD);
#ifdef __cplusplus
}
#endif

#endif