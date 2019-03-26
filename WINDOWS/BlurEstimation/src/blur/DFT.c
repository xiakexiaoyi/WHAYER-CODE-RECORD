
/*!
* \file DFT.c
* \brief  the functions related to the FFT and DFT 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/



#include "ligaussian.h"
#include"liblock.h"
#include"limatrix.h"
#include"math.h"
#include"limath.h"
#include"limem.h"
#include"lidebug.h"
#include "DFT.h"



/// \brief FFT means one dimension fast fourier transform
/// \hMemMgr Handle
/// \param TD the time domain parameter
/// \param FD the frequence domain parameter
/// \return the error code
///一维的快速傅里叶变化,TD时域信号，FD频域信号
MRESULT FFT(MHandle  hMemMgr,COMPLEX * TD, COMPLEX * FD, MLong power)
{    
    MLong res=LI_ERR_NONE;
    MLong cout;
    MLong i,j,k,bfsize,p;
    MDouble angle;
    COMPLEX *W,*X1,*X2,*X;

    /// \calculate the number of data to do the FFT
    cout=1<<power;

    ///\memory allocate for the calculation    
    AllocVectMem(hMemMgr,W,cout/2,COMPLEX);
    SetVectMem(W,cout/2,0,COMPLEX);
    AllocVectMem(hMemMgr,X1,cout,COMPLEX);
    SetVectMem(X1,cout,0,COMPLEX);
    AllocVectMem(hMemMgr,X2,cout,COMPLEX);
    SetVectMem(X2,cout,0,COMPLEX);
    
    /// \calculate the weighted coefficient
	/// 计算权重
    for(i=0;i<cout/2;i++)
    {
        angle=-i*V_PI*2/cout;
        W[i].re=cos(angle);
        W[i].im=sin(angle);
    }

    /// \set the time domain data to the memory
    JMemCpy(X1,TD,cout*sizeof(COMPLEX));
       
   /// \butterfly computation 蝶形计算
    for(k=0;k<power;k++)
    {
        for(j=0;j<1<<k;j++)
        {
            bfsize=1<<(power-k);
            for(i=0;i<bfsize/2;i++)
            {
                p=j*bfsize;
                X2[i+p]=Add(X1[i+p],X1[i+p+bfsize/2]);
                X2[i+p+bfsize/2]=Mul(Sub(X1[i+p],X1[i+p+bfsize/2]),W[i*(1<<k)]);
             }
        }
        X=X1;
        X1=X2;
        X2=X;
    }

    ///\rerange the data 中心排列数据
    for(j=0;j<cout;j++)
    {
        p=0;
        for(i=0;i<power;i++)
        {
            if (j&(1<<i)) p+=1<<(power-i-1);
        }
        FD[j]=X1[p];
    }
    /// \release the memory 
    EXT:
    FreeVectMem(hMemMgr,W);
    FreeVectMem(hMemMgr,X1);
    FreeVectMem(hMemMgr,X2);

    return res;
}



/// \brief IFFT means one dimension inverse fast fourier transform
/// \hMemMgr Handle
/// \param TD the time domain parameter
/// \param FD the frequence domain parameter
/// \return the error code
///一维的逆快速傅里叶变化，TD 时域的数据，FD频域的信号，power表示2的阶乘
MRESULT IFFT(MHandle  hMemMgr,COMPLEX * TD, COMPLEX * FD, MLong power)
{
    MLong res=LI_ERR_NONE;
    MLong  i,cout;
    COMPLEX *x;
    
    /// \calculate the number of the ifft
    cout=1<<power;

    ///\ ALLOCATE the memory
    AllocVectMem(hMemMgr,x,cout,COMPLEX);
    SetVectMem(x,cout,0,COMPLEX);

    ///\ write the frequncy domain data into the memory
    JMemCpy(x,FD,cout*sizeof(COMPLEX));

    ///\ find the conjugation of the frequency domain data
	///找到频域的共轭
    for(i=0;i<cout;i++)
    {
        x[i].im=-x[i].im;
    }

    ///\ do the one dimension FFT
    FFT(hMemMgr,x,TD,power);
    ///\ find the conjugation of the time domain data
	///找到时域的共轭
    for(i=0;i<cout;i++)
    {
        TD[i].re/=cout;
        TD[i].im=-TD[i].im/cout;
    }
    
    ///\release the memory
    EXT:
    FreeVectMem(hMemMgr,x);
    return res;
}



 
/// \brief FFT2 means two dimensions  fast fourier transform
/// \hMemMgr Handle
/// \param TD the time domain parameter
/// \param FD the frequence domain parameter
/// \return the error code
///二维的快速傅里叶变化
MRESULT FFT2(MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD)
{	 
    MLong res=LI_ERR_NONE;
    COMPLEX *TempT, *TempF;
    ///< the loop variable
    MLong i,j;
    MLong w,h,wp,hp;
    /// \the width and height to do FFT(which is power of 2)
    w = 1;
    h = 1;
    wp = 0;
    hp = 0;
    /// \calculate the width and height to do FFT(which is power of 2)
    while (w < lWidth)
    {
        w *= 2;
        wp++;
    }
    while (h < lHeight)
    {
        h *= 2;
        hp++;
    }

    ///\ alloc the memory
    AllocVectMem(hMemMgr,TempT,h,COMPLEX);
    SetVectMem(TempT,h,0,COMPLEX);
    AllocVectMem(hMemMgr,TempF,h,COMPLEX);
    SetVectMem(TempF,h,0,COMPLEX);

   ///\ do the fft in y direction，Y方向的FFT
    for (i = 0; i < w ; i++)
    {
        /// \extract the data
        for (j = 0; j < h; j++)
        {	
            TempT[j] = TD[j * w  + i];
        }

        /// \ do fft
        FFT(hMemMgr,TempT, TempF, hp);

        /// \ save the result
        for (j = 0; j < h; j++)
        {	
            TD[j * w  + i] = TempF[j];
        }
    }

    /// \release the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);
    
    ///\ alloc the memory    
    AllocVectMem(hMemMgr,TempT,w,COMPLEX);
    SetVectMem(TempT,w,0,COMPLEX);
    AllocVectMem(hMemMgr,TempF,w,COMPLEX);
    SetVectMem(TempF,w,0,COMPLEX);
    
    ///\ do fft in x direction, X方向的FFT
    for (i = 0; i < h; i++)
    {
        
            ///\ extract data
            for (j = 0; j < w; j++)
            {
                TempT[j] = TD[i * w  + j ];
            }

            ///\do fft
            FFT(hMemMgr,TempT, TempF, wp);

            ///\save the data
            for (j = 0; j < w; j++)
            {	
                FD[i * w  + j ] = TempF[j];
            }
    }
    EXT:
    // release the memory
    FreeVectMem(hMemMgr,TempF);
    FreeVectMem(hMemMgr,TempT);
    
    return res;

}

/// \brief IFFT2 means two dimensions  inverse fast fourier transform
/// \hMemMgr Handle
/// \param TD the time domain parameter
/// \param FD the frequence domain parameter
/// \return the error code
/// 二维傅里叶反变化
MRESULT IFFT2(MHandle hMemMgr,COMPLEX *TD, MLong lWidth, MLong lHeight,COMPLEX * FD)
{
    MLong res=LI_ERR_NONE;
    COMPLEX *TempT, *TempF;
    /// \loop variables
    MLong i,j;
    MLong w,h,wp,hp;

    /// \the width and height to do IFFT(which is power of 2)
    w = 1;
    h = 1;
    wp = 0;
    hp = 0;
    
    /// \ calculate the width and height to do FFT(which is power of 2)
    while(w < lWidth)
    {
        w *= 2;
        wp++;
    }
    while(h < lHeight)
    {
        h *= 2;
        hp++;
    }

   ///\alloc the memory
    AllocVectMem(hMemMgr,TempF,w,COMPLEX);
    SetVectMem(TempF,w,0,COMPLEX);
    AllocVectMem(hMemMgr,TempT,w,COMPLEX);
    SetVectMem(TempT,w,0,COMPLEX);
    
    ///\ do ifft in x direction x方向的逆变化
    for (i = 0; i < h; i++)
    {
        /// \extract the data
        for (j = 0; j < w; j++)
        {
            TempF[j] = FD[i * w  + j ];
        }

            ///\ ifft
            IFFT(hMemMgr,TempT, TempF, wp);

            ///\ save the data
            for (j = 0; j < w; j++)
            {
                FD[i * w + j ] = TempT[j];
            }
    
    }
    ///\free the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);
    AllocVectMem(hMemMgr,TempF,h,COMPLEX);
    SetVectMem(TempF,h,0,COMPLEX);
    AllocVectMem(hMemMgr,TempT,h,COMPLEX);
    SetVectMem(TempT,h,0,COMPLEX);
    /// \ do ifft in y direction y方向的逆变化
    for (i = 0; i < w ; i++)
    {
        /// \extract the data
        for (j = 0; j < h; j++)
        {
            TempF[j] = FD[j * w  + i];
        }
        
        /// \ifft
        IFFT(hMemMgr,TempT, TempF, hp);

        /// \save the result
        for (j = 0; j < h; j++)
        {
            TD[j * w  + i] = TempT[j];
        }
    }
    
EXT:
    ///\ free the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);
    return res;
}


/// \brief FFTconv means calculate the convolution using  FFT
/// \hMemMgr Handle
/// \param blockres the result image after the convolution
/// \param blocksrc1 the block used for convolution
/// \param blocksrc2 the block used for convolution
/// \return the error code
/// 用快速傅里叶方法完成卷积
MRESULT FFTconv(MHandle hMemMgr,PBLOCK blockres, PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MRESULT res=LI_ERR_NONE;
    COMPLEX *TDsrc1,*TDsrc2,*FDsrc1,*FDsrc2,*TDres,*FDres;
    MLong i,j,k;
    MUInt8* datasrc1;
    MDouble *datasrc2;
    MUInt8* datares;
    MLong extsrc1,extsrc2,extres;
    MLong width,height,temp,len;
    TDsrc1=MNull;
    FDsrc1=MNull;
    TDsrc2=MNull;
    FDsrc2=MNull;
    TDres=MNull;
    FDres=MNull;
    /// \caculate the fft for the two source imgaes，计算原图像的fft
    datasrc1=(MUInt8*)blocksrc1->pBlockData;
    datasrc2=(MDouble*)blocksrc2->pBlockData;
    extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    extsrc2=blocksrc2->lBlockLine-blocksrc2->lHeight;
    width=blocksrc1->lWidth+blocksrc2->lWidth-1;
    height=blocksrc2->lHeight+blocksrc2->lHeight-1;
    /// \calculate the width and height needed for adding zeros 
	/// 因为快速傅里叶变化的图像的宽高需要是2的阶乘，所以需要将图像补零
    temp=FFTPOW(width);
    width=pow((long double)2,temp);
    temp=FFTPOW(height);
    height=pow((long double)2,temp);
    len=width*height;
    /// \adding zeros to the original datas
    AllocVectMem(hMemMgr,TDsrc1,len,COMPLEX);
    SetVectMem(TDsrc1,len,0,COMPLEX);
    AllocVectMem(hMemMgr,FDsrc1,len,COMPLEX);
    SetVectMem(FDsrc1,len,0,COMPLEX);
    AllocVectMem(hMemMgr,TDsrc2,len,COMPLEX);
    SetVectMem(TDsrc2,len,0,COMPLEX);
    AllocVectMem(hMemMgr,FDsrc2,len,COMPLEX);
    SetVectMem(FDsrc2,len,0,COMPLEX);
    AllocVectMem(hMemMgr,TDres,len,COMPLEX);
    SetVectMem(TDres,len,0,COMPLEX);
    AllocVectMem(hMemMgr,FDres,len,COMPLEX);
    SetVectMem(FDres,len,0,COMPLEX);
    for(i=0;i<blocksrc1->lHeight;i++,datasrc1+=extsrc1)
    for(j=0;j<blocksrc1->lWidth;j++,datasrc1++)
    {
        k=i*width+j;
        TDsrc1[k].re=*(datasrc1);
    }
    for(i=0;i<blocksrc2->lHeight;i++,datasrc2+=extsrc2)
    for(j=0;j<blocksrc2->lWidth;j++,datasrc2++)
    {
        k=i*width+j;
        TDsrc2[k].re=*(datasrc2);
    }
    /// \DO FFT2 
    FFT2(hMemMgr,TDsrc1,width,height,FDsrc1);
    FFT2(hMemMgr,TDsrc2,width,height,FDsrc2);
    /// \DO multiply
    for(i=0;i<height;i++)
    for(j=0;j<width;j++)
    {
       k=i*width+j;
       FDres[k].re=FDsrc1[k].re*FDsrc2[k].re-FDsrc1[k].im*FDsrc2[k].im;
       FDres[k].im=FDsrc1[k].re*FDsrc2[k].im+FDsrc1[k].im*FDsrc2[k].re;
     }
        /// \DO IFFT to the result after multiplying
        IFFT2(hMemMgr,TDres,width,height,FDres);
        /// \choose the effective area 选择有效区域
        datares=(MUInt8*)blockres->pBlockData;
        extres=blockres->lBlockLine-blockres->lWidth;
        for(i=0;i<blockres->lHeight;i++,datares+=extres)
        for(j=0;j<blockres->lWidth;j++,datares++)
        {
            k=i*width+j;
            temp=TDres[k].re;
            *(datares)=temp;

        }
EXT:
        FreeVectMem(hMemMgr,TDsrc1);
        FreeVectMem(hMemMgr,TDsrc2);
        FreeVectMem(hMemMgr,TDres);
        FreeVectMem(hMemMgr,FDsrc1);
        FreeVectMem(hMemMgr,FDsrc2);
        FreeVectMem(hMemMgr,FDres);
        return res;


}

/// \brief DFTconv means calculate the convolution using DFT
/// \param blockres the result image after the convolution
/// \param blocksrc1 the block used for convolution
/// \param blocksrc2 the block used for convolution
/// \return the error code
/// 用DFT对图像进行快速卷积
MRESULT DFTconv(MHandle hMemMgr,PBLOCK blockres, PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MRESULT res=LI_ERR_NONE;
    COMPLEX *TDsrc1,*TDsrc2,*FDsrc1,*FDsrc2,*TDres,*FDres;
    MLong i,j,k;
    MUInt8* datasrc1;
    MDouble *datasrc2;
    MUInt8* datares;
    MLong extsrc1,extsrc2,extres;
    MLong width,height,dheight,dwidth;
    
     TDsrc1=MNull;
     FDsrc1=MNull;
     TDsrc2=MNull;
     FDsrc2=MNull;
     TDres=MNull;
     FDres=MNull;
    datasrc1=(MUInt8*)blocksrc1->pBlockData;
    datasrc2=(MDouble*)blocksrc2->pBlockData;
    extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    extsrc2=blocksrc2->lBlockLine-blocksrc2->lHeight;
    width=blocksrc1->lWidth+blocksrc2->lWidth-1;
    height=blocksrc2->lHeight+blocksrc2->lHeight-1;
     /// \ALLOCATE THE MEMORY
    AllocVectMem(hMemMgr,TDsrc1,width*height,COMPLEX);
    AllocVectMem(hMemMgr,FDsrc1,width*height,COMPLEX);
    AllocVectMem(hMemMgr,TDsrc2,width*height,COMPLEX);
    AllocVectMem(hMemMgr,FDsrc2,width*height,COMPLEX);
    AllocVectMem(hMemMgr,FDres,width*height,COMPLEX);
    SetVectMem(TDsrc1,width*height,0,COMPLEX);
    SetVectMem(TDsrc2,width*height,0,COMPLEX);
    for(i=0;i<blocksrc1->lHeight;i++,datasrc1+=extsrc1)
    for(j=0;j<blocksrc1->lWidth;j++,datasrc1++)
    {
        k=i*blocksrc1->lWidth+j;
        TDsrc1[k].re=*(datasrc1);
     }
    for(i=0;i<blocksrc2->lHeight;i++,datasrc2+=extsrc2)
    for(j=0;j<blocksrc2->lWidth;j++,datasrc2++)
    {
        k=i*blocksrc2->lWidth+j;
        TDsrc2[k].re=*(datasrc2);

    }

    /// \DO DFT DFT变化
    DFT2(hMemMgr,TDsrc1,blocksrc1->lWidth,blocksrc1->lHeight,FDsrc1);
    DFT2(hMemMgr,TDsrc2,blocksrc2->lWidth,blocksrc2->lHeight,FDsrc2);

    /// \Adding zeros to multiply ，补零
    for(i=0;i<height;i++)
    for(j=0;j<width;j++)
    {
        k=i*width+j;
        FDres[k].re=FDsrc1[k].re*FDsrc2[k].re-FDsrc1[k].im*FDsrc2[k].im;
        FDres[k].im=FDsrc1[k].re*FDsrc2[k].im+FDsrc1[k].im*FDsrc2[k].re;
    }

    ///\ DO IDFT2 DFT反变化
    IDFT2(hMemMgr,TDres,width,height,FDres);
    dheight=floor((long double)(height-blockres->lHeight)/2);
    dwidth=floor((long double)(width-blockres->lWidth)/2);
    datares=(MUInt8*)blockres->pBlockData;
    extres=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blockres->lHeight;i++,datares+=extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
       {
           k=(i+dheight)*width+j+dwidth;
            *(datares)=TDres[k].re;
        }
EXT:
  FreeVectMem(hMemMgr,TDsrc1);
  FreeVectMem(hMemMgr,TDsrc2);
  FreeVectMem(hMemMgr,FDsrc1);
  FreeVectMem(hMemMgr,TDsrc2);
  FreeVectMem(hMemMgr,TDres);
  FreeVectMem(hMemMgr,FDres);

    return res;
    
    
}

/// \brief FFTPOW means calculate the power value which is equal to or bigger
///  than the src data 
/// \param asrc the input data
/// \return the power value
/// \计算大于输入的数的2的阶乘的最小值
MLong  FFTPOW(MLong asrc)
{

    MLong check; 
    MLong temp; 
    MLong rlt;
    /// \check whether the value is the power of 2
    check=IS_POW2(asrc);
  
    if(check==0)
    {
        LOG2_FLOOR(asrc,temp);
        rlt=temp+1;
    }
    if(check==1)
    {
        LOG2_FLOOR(asrc,temp);
        rlt=temp;
    }
    return rlt;
}

/// \brief FFTADDZeros  means adding zeros to the src data
/// \hMemMgr Handle
/// \param TDres the data after adding
/// \param TDsrc the data needed to be added zeros
/// \param power which decided the length of the result data
/// \return the error code
///  \将向量补充到2的power阶乘，方便后续做FFT变化
MRESULT FFTADDZeros(MHandle hMemMgr,COMPLEX* TDres,COMPLEX*TDsrc,MLong power)
{
    MLong res=LI_ERR_NONE;
    MLong n;
    MLong sizeres;
    MLong sizesrc;
    MLong i;
    /// \calculate the length
    n=pow((long double)2,power);

    sizeres=sizeof(TDres)/sizeof(COMPLEX);
    sizesrc=sizeof(TDsrc)/sizeof(COMPLEX);
    if(sizeres!=n)
    {
        res=LI_ERR_INVALID_PARAM;
        return res;
    }
    TDres[0]=TDsrc[0];
    for(i=1;i<(sizeres+1)/2;i++)
    {
        TDres[i]=TDsrc[i];
        TDres[sizeres-i]=TDsrc[sizesrc-i];
    }
    for(i=(sizesrc+1)/2;i<sizeres-(sizesrc-1)/2;i++)
    {
        TDres[i].im=0.0;
        TDres[i].re=0.0;
    }

    return res;
}
/// \brief DFT  means do DFT in one dimension 
/// \hMemMgr Handle
/// \param TD time domain data
/// \param FD frequency domain data
/// \param sign while sign==-1, do inverse dft
/// \return the error code
/// \完成一维DFT变化以及DFT逆变化，当sign=1时，做逆变化
MRESULT DFT(MHandle hMemMgr,COMPLEX * TD, COMPLEX * FD,MLong width,MLong sign)
{
    MLong res=LI_ERR_NONE;
    MLong i,k;
    MDouble c,d,q,w,s;
    q=6.28318530718/width;
    for (k=0;k<width;k++)
        {
            w=k*q;
            FD[k].im=0.0;
            FD[k].re=0.0;
            for (i=0;i<width;i++)
            {
                d=i*w;
                c=cos(d);
                s=sin(d)*sign;
                FD[k].re+=c*TD[i].re+s*TD[i].im;
                FD[k].im+=c*TD[i].im-s*TD[i].re;
            }
        }
    /// \if sign==-1,do inverse dft
        if (sign==-1)
        {
            c=1.0/width;
            for (k=0;k<width;k++)
            {
                FD[k].re=c*FD[k].re;
                FD[k].im=c*FD[k].im;
              
            }
        }
        return res;
}

/// \brief DFT2  means do DFT in two dimensions 
/// \hMemMgr Handle
/// \param TD time domain data
/// \param FD frequency domain data
/// \param lWidth the width of the data
/// \param lHeight the height of the data
/// \return the error code
/// \完成2维DFT变化
MRESULT DFT2(MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD)
{

    MLong res=LI_ERR_NONE;
    COMPLEX *TempT, *TempF;
    /// \loop variables
    MLong i,j;
    
    ///\ allocate the memory
    AllocVectMem(hMemMgr,TempT,lHeight,COMPLEX);
    SetVectMem(TempT,lHeight,0,COMPLEX);
    AllocVectMem(hMemMgr,TempF,lHeight,COMPLEX);
    SetVectMem(TempF,lHeight,0,COMPLEX);
    /// \do dft in y direction
    for (i = 0; i < lWidth ; i++)
    {
        /// \extract the data
        for (j = 0; j < lHeight; j++)
        {	
            TempT[j] = TD[j * lWidth  + i];}

        /// \ do one dimesion dft
        DFT(hMemMgr,TempT,TempF,lHeight,1);
        

       /// \save the result
        for (j = 0; j < lHeight; j++)
        {	
            TD[j *lWidth  + i] = TempF[j];
        }
    }

    /// \release the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);

    /// \allocate the memory

    AllocVectMem(hMemMgr,TempT,lWidth,COMPLEX);
    SetVectMem(TempT,lWidth,0,COMPLEX);
    AllocVectMem(hMemMgr,TempF,lWidth,COMPLEX);
    SetVectMem(TempF,lWidth,0,COMPLEX);

    /// \do dft in x direction，在X方向做dft变化
    for (i = 0; i < lHeight; i++)
    {

        /// \extract data
        for (j = 0; j < lWidth; j++)
        {	TempT[j] = TD[i * lWidth  + j ];}

        /// \one dimension idft
        DFT(hMemMgr,TempT,TempF,lWidth,1);
       

        ///\save the result
        for (j = 0; j < lWidth; j++)
        {	
            FD[i * lWidth  + j ] = TempF[j];
        }
    }
EXT:
    /// \release the memory
    FreeVectMem(hMemMgr,TempF);
    FreeVectMem(hMemMgr,TempT);

    return res;
}

/// \brief IDFT2  means do IDFT in two dimensions 
/// \hMemMgr Handle
/// \param TD time domain data
/// \param FD frequency domain data
/// \param lWidth the width of the data
/// \param lHeight the height of the data
/// \return the error code
/// \二维DFT反变化
MRESULT IDFT2 (MHandle hMemMgr,COMPLEX * TD, MLong lWidth, MLong lHeight, COMPLEX * FD)
{
    MLong res=LI_ERR_NONE;
    COMPLEX *TempT, *TempF;
    /// \loop variabels
    MLong i,j;

    /// \allocate the memory
    AllocVectMem(hMemMgr,TempF,lWidth,COMPLEX);
    SetVectMem(TempF,lWidth,0,COMPLEX);
    AllocVectMem(hMemMgr,TempT,lWidth,COMPLEX);
    SetVectMem(TempT,lWidth,0,COMPLEX);

    /// \do idft in x direction
    for (i = 0; i < lHeight; i++)
    {
        /// \  extract the data
        for (j = 0; j < lWidth; j++)
        {
            TempF[j] = FD[i * lWidth  + j ];
        }

        /// \one dimension idft
        DFT(hMemMgr,TempF,TempT,lWidth,-1);
        
        /// \save the data
        for (j = 0; j < lWidth; j++)
        {
            FD[i * lWidth + j ] = TempT[j];
        }

    }
    /// \release the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);
    AllocVectMem(hMemMgr,TempF,lHeight,COMPLEX);
    SetVectMem(TempF,lHeight,0,COMPLEX);
    AllocVectMem(hMemMgr,TempT,lHeight,COMPLEX);
    SetVectMem(TempT,lHeight,0,COMPLEX);
    /// \do idft in y direction
    for (i = 0; i < lWidth ; i++)
    {
        /// \extract the data
        for (j = 0; j < lHeight; j++)
        {
            TempF[j] = FD[j * lWidth  + i];
        }

        /// \ one dimension dft
        DFT(hMemMgr,TempF,TempT,lHeight,-1);
        

        /// \save the result
        for (j = 0; j < lHeight; j++)
        {
            TD[j * lWidth  + i] = TempT[j];
        }
    }

EXT:
    /// \release the memory
    FreeVectMem(hMemMgr,TempT);
    FreeVectMem(hMemMgr,TempF);
    return res;
}