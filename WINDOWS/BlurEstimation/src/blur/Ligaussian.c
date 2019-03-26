
/*!
* \file Ligaussian.c
* \brief  the function related to the gaussian blur 
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
#include"DFT.h"



static const MLong s1[7]={443,5401,24204,39905,24204,5401,443};
static const MLong s2[13]={222,877,2702,6482,12111,17621,19968,17621,12111,6482,2702,877,222};
static const MLong s3[31]={89,159,272,449,711,1082,1582,2223,3000,3891,4849,5805,6772,7378,7836,7994,7836,7378,6772,5805,4849,3891,3000,2223,1582,1082,711,449,272,159,89};
static const MLong s4[43]={63,96,144,209,299,419,575,773,1018,1314,1662,2059,2499,2972,3464,3955,4425,4851,5210,5483,5653,5483,5210,4851,4425,3955,3464,2972,2499,2059,1662,
1314,1018,773,575,419,299,209,144,96,63};
static const MLong s5[61]={44,60,79,104,136,176,225,284,356,441,541,658,791,943,1112,1298,1501,1718,1946,2184,2425,2667,2904,3130,3340,3529,3691,3823,3919,3979,3999,
    3979,3919,3823,3691,3529,3340,3130,2904,2667,2425,2184,1946,1718,1501,1298,1112,943,791,658,541,441,356,284,225,176,136,104,79,60,44};
static const MLong s6[85]={32,39,48,59,72,87,105,126,150,178,210,246,288,334,387,445,509,580,657,741,831,927,1030,1137,1250,1367,1487,1610,1733,1856,1978,2098,2213,
    2426,2521,2606,2680,2742,2792,2827,2849,2856,2849,2827,2792,2742,2680,2606,2521,2426,2213,2098,1978,1856,1733,1610,1487,1367,1250,1137,1030,927,831,741,657,
    580,509,445,387,334,288,246,210,178,150,126,105,87,72,59,48,39,32};
//static const MLong s5[51]={176,225,284,356,441,541,658,791,943,1112,1298,1501,1718,1946,2184,2425,2667,2904,3130,3340,3529,3691,3823,3919,3979,3999,
//    3979,3919,3823,3691,3529,3340,3130,2904,2667,2425,2184,1946,1718,1501,1298,1112,943,791,658,541,441,356,284,225,176};
//static const MLong s6[51]={509,580,657,741,831,927,1030,1137,1250,1367,1487,1610,1733,1856,1978,2098,2213,
//    2426,2521,2606,2680,2742,2792,2827,2849,2856,2849,2827,2792,2742,2680,2606,2521,2426,2213,2098,1978,1856,1733,1610,1487,1367,1250,1137,1030,927,831,741,657,
//    580,509};




/// \brief Ligaussian is used to produced a gaussian kernel related to the sigma 
/// \param  hMemMgr handle
/// \param  BlockG The gaussian kernel produced
/// \param sigma the gaussian sigma
/// \return  error code
/// \根据sigma的值产生一个gaussian核
MRESULT LiGaussian(MHandle hMemMgr,PBLOCK BlockG,MFloat sigma)
{
    MLong res=LI_ERR_NONE;
    MFloat siz;
    MFloat std=sigma;
    MLong i,j;
    BLOCK X={0};
    BLOCK Y={0};
    BLOCK Arg={0};
    MFloat* datax;
    MFloat* datay;
    MFloat* dataarg;
    MFloat* datag;
    MFloat tempx;
    MFloat tempy;
    MFloat temparg;
    MFloat sum;
    MLong EXTX,EXTY,EXTArg,EXTG;
    siz=(BlockG->lHeight-1)/2.0;
    GO(B_Create(hMemMgr,&X,DATA_F32,BlockG->lWidth,BlockG->lHeight));
    GO(B_Create(hMemMgr,&Y,DATA_F32,BlockG->lWidth,BlockG->lHeight));
    GO(B_Create(hMemMgr,&Arg,DATA_F32,BlockG->lWidth,BlockG->lHeight));
    datax=(MFloat*)X.pBlockData;
    datay=(MFloat*)Y.pBlockData;
    dataarg=(MFloat*)Arg.pBlockData;
    EXTX=X.lBlockLine-X.lWidth;
    for(i=0;i<X.lHeight;i++,datax+=EXTX)
    for(j=0;j<X.lWidth;j++,datax++)
    {
        *(datax)=-siz+j;
    }

    EXTY=Y.lBlockLine-Y.lWidth;
    for(i=0;i<Y.lHeight;i++,datay+=EXTY)
    for(j=0;j<Y.lWidth;j++,datay++)
    {
        *(datay)=-siz+i;
    }
    datax=(MFloat*)X.pBlockData;
    datay=(MFloat*)Y.pBlockData;
    EXTArg=Arg.lBlockLine-Arg.lWidth;
    for(i=0;i<Arg.lHeight;i++,dataarg+=EXTArg,datax+=EXTX,datay+=EXTY)
    for(j=0;j<Arg.lWidth;j++,datax++,datay++,dataarg++)
    {
        tempx=*(datax)*(*(datax));
        tempy=*(datay)*(*(datay));
        temparg=-(tempx+tempy)/(2*std*std);
        *(dataarg)=(float)exp((float)temparg);
    }
    dataarg=(MFloat*)Arg.pBlockData;
    datag=(MFloat*)BlockG->pBlockData;
    LiSum(&sum,&Arg);
    if (sum!=0)
    {
        EXTG=BlockG->lBlockLine-BlockG->lWidth;
        for(i=0;i<BlockG->lHeight;i++,datag+=EXTG,dataarg+=EXTArg)
        for(j=0;j<BlockG->lWidth;j++,dataarg++,datag++)
        {
            *(datag)=*(dataarg)/sum;
        }
    }

EXT:
  B_Release(hMemMgr,&X);
  B_Release(hMemMgr,&Y);
  B_Release(hMemMgr,&Arg);
  return res;

}


/// \brief LiGaussianBlur is the function which can reblur the source image using the 
///      gaussian kernel related to the sigma
/// \param  hMemMgr handle
/// \param  blockres the image after reblurring
/// \param  blocksrc the image which is needed to be reblurred
/// \param sigma the gaussian sigma
/// \return  error code
/// 根据给定的sigma值再模糊图像

MRESULT LiGaussianBlur(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc,MLong sigma)
{
    MRESULT res=LI_ERR_NONE;
    MLong ksize,kcenter,i,j,x,y;
    MDouble *kernel=MNull;
    MUInt32 *kernel1=MNull;
    MDouble scale,cons;
    MLong sum,mul;
    MUInt8 *datares;
    MUInt8 *datasrc;
    MUInt32 *datatemp;
    MLong hsrc,wsrc,hres,wres;
    BLOCK temp={0};
    MLong flag=0;
    MFloat eps=0.000001;
    MLong exttemp,extsrc,extres;
    MLong  temp1;
    MDouble sum1;
    if(sigma==0)
    {
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }

    /*The size of the kernel is (6*sigma+1)  ksizeΪodd  */
	///给定sigma值，如果是1,2,3,4,5,6，则根据预先分配的，如果是其他的，则生成核
    switch(sigma)
    {
    case 1:
        kernel1=(MUInt32*)s1;
        kcenter=3;
        break;
    case 2:
        kernel1=(MUInt32*)s2;
        kcenter=6;
        break;
    case 5:
        kernel1=(MUInt32*)s3;
        kcenter=15;
        break;
    case 7:
        kernel1=(MUInt32*)s4;
        kcenter=21;
        break;
    case 10:
        kernel1=(MUInt32*)s5;
        kcenter=30;
        break;
    case 14:
        kernel1=(MUInt32*)s6;
        kcenter=42;
        break;
    default:
        ksize = (sigma * 3) * 2 + 1;  
        /// \  produce one dimension gaussian kernel，生成核
        AllocVectMem(hMemMgr,kernel,ksize,MDouble);
        scale = -0.5/(sigma*sigma);  
        cons = 1/sqrt(-scale / V_PI);  
        kcenter = ksize/2;  
        sum1=0;

        for(i = 0; i < ksize; i++)  
        {  
            x = i - kcenter;  
            *(kernel+i) = cons * exp(x * x * scale);///<one dimesion kernel
            sum1 += *(kernel+i);  
        }  
        /// \normailize the kernel to[0,1] 
        for(i = 0; i < ksize; i++)  
        {  
            *(kernel+i) /= sum1;  
        }  
        AllocVectMem(hMemMgr,kernel1,ksize,MUInt32);
        for(i = 0; i < ksize; i++)  
        {  
            *(kernel1+i) =*(kernel+i)*100000;   
        }  
        flag=1;

    }
    

    datasrc=(MUInt8*)blocksrc->block.pBlockData+blocksrc->ext.top*blocksrc->block.lBlockLine+blocksrc->ext.left;
    datares=(MUInt8*)blockres->block.pBlockData+blockres->ext.top*blocksrc->block.lBlockLine+blockres->ext.left;
    hsrc=blocksrc->ext.bottom-blocksrc->ext.top;
    wsrc=blocksrc->ext.right-blocksrc->ext.left;
    hres=blockres->ext.bottom-blockres->ext.top;
    wres=blockres->ext.right-blockres->ext.left;
    GO(B_Create(hMemMgr,&temp,DATA_U32,wres,hres));
    datatemp=(MUInt32*)temp.pBlockData;
    extsrc=blocksrc->block.lBlockLine-(blocksrc->ext.right-blocksrc->ext.left);
    exttemp=temp.lBlockLine-temp.lWidth;
    extres=blockres->block.lBlockLine-(blockres->ext.right-blockres->ext.left);

  ///\do the gaussian blur in the x direction 在x方向做高斯模糊
    for(y = 0; y <hsrc ; y++,datatemp+=exttemp,datasrc+=extsrc)  
    for( x = 0; x < wsrc; x++,datatemp++,datasrc++)  
    {  
        mul = 0;  
        sum = 0;  
        
        for(i = -kcenter; i <= kcenter; i++)  
        {  
            if((x+i) >= 0 && (x+i) < wsrc)  
            {  
                mul += *(datasrc+i)*(*(kernel1+kcenter+i));  
                sum += (*(kernel1+kcenter+i));  
            }  
        }  
            *(datatemp) = mul/sum;  
    }  
    

    /// \do the gaussian blur in y direction 在y方向做高斯模糊
    datatemp=(MUInt32*)temp.pBlockData;
    extres=blockres->block.lBlockLine-(blockres->ext.right-blockres->ext.left);
    datares=(MUInt8*)blockres->block.pBlockData+blockres->ext.top*blocksrc->block.lBlockLine+blockres->ext.left;
    for(y=0;y<temp.lHeight;y++,datares+=extres,datatemp+=exttemp)
    for(x=0;x<temp.lWidth;x++,datares++,datatemp++)
    {
        mul = 0;  
        sum = 0;  
        
        for(i = -kcenter; i <= kcenter; i++)  
        {  
            if((y+i) >= 0 && (y+i) < temp.lHeight)  
            {  
                mul += *(datatemp+i*temp.lBlockLine)*(*(kernel1+kcenter+i));  
                sum += (*(kernel1+kcenter+i));  
            }  
        
        }  	
        temp1=mul/sum;
        *(datares)=temp1;
    }
    
    


EXT:
    FreeVectMem(hMemMgr,kernel);
    if (flag==1)
    {
        FreeVectMem(hMemMgr,kernel1);
    }
    


B_Release(hMemMgr,&temp);
return res;

}


/// \brief LiGaussianBlurF is the function which can reblur the source image using the 
///       gaussian kernel using the fast fourier transform related to the input sigma
/// \param  hMemMgr handle
/// \param  blockres the image after reblurring
/// \param  blocksrc the image which is needed to be reblurred
/// \param sigma the gaussian sigma
/// \return  error code
/// \用快速傅里叶变化进行高斯模糊
MRESULT LiGaussianBlurF(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc,MLong sigma)
{
    MRESULT res=LI_ERR_NONE;
    COMPLEX *s=MNull;
    COMPLEX *sf=MNull;
    COMPLEX *TDres=MNull;
    COMPLEX  *FDres=MNull;
    COMPLEX *TDsrc=MNull;
    COMPLEX *FDsrc=MNull;
    MFloat *data1=MNull;
    MUInt8 *datasrc=MNull;
    MUInt8 *datares=MNull;
    MLong i,j,extsrc,extsig,k,extres;
    MLong width,height;
    MLong ksize,temp;
    BLOCK Blocksigma={0};

    /// \produce the gaussian kernel using the sigma，根据sigma值生成gaussian核
    ksize=6*sigma+1;
    GO(B_Create(hMemMgr,&Blocksigma,DATA_F32,ksize,ksize));
    LiGaussian(hMemMgr,&Blocksigma,sigma);
    
    //// \add zeros according to the size of blocksrc and blocksigma，补零
    width=Blocksigma.lWidth+blocksrc->ext.right-blocksrc->ext.left-1;
    height=Blocksigma.lHeight+blocksrc->ext.bottom-blocksrc->ext.top-1;
    temp=FFTPOW(width);
    width=pow((float)2,temp);
    temp=FFTPOW(height);
    height=pow((float)2,temp);
    AllocVectMem(hMemMgr,s,width*height,COMPLEX);
    AllocVectMem(hMemMgr,sf,width*height,COMPLEX);
    SetVectMem(s,width*height,0,COMPLEX);
    SetVectMem(sf,width*height,0,COMPLEX);
    extsig=Blocksigma.lBlockLine-Blocksigma.lWidth;
    data1=(MFloat*)Blocksigma.pBlockData;
    for(i=0;i<Blocksigma.lHeight;i++,data1+=extsig)
    for(j=0;j<Blocksigma.lWidth;j++,data1++)
    {
        s[i*width+j].re=*(data1);
    }
    AllocVectMem(hMemMgr,TDsrc,width*height,COMPLEX);
    AllocVectMem(hMemMgr,FDsrc,width*height,COMPLEX);
    SetVectMem(TDsrc,width*height,0,COMPLEX);
    SetVectMem(FDsrc,width*height,0,COMPLEX);
    extsrc=blocksrc->block.lBlockLine-blocksrc->ext.right+blocksrc->ext.left;
    datasrc=(MUInt8*)blocksrc->block.pBlockData+blocksrc->ext.top*(blocksrc->block.lBlockLine)+blocksrc->ext.left;
    
    for(i=0;i<(blocksrc->ext.bottom-blocksrc->ext.top);i++,datasrc+=extsrc)
    for(j=0;j<(blocksrc->ext.right-blocksrc->ext.left);j++,datasrc++)
        {
            TDsrc[i*width+j].re=*(datasrc);
        }

     /// \ do the fft  二维的fft
    FFT2(hMemMgr,s,width,height,sf);
    FFT2(hMemMgr,TDsrc,width,height,FDsrc);
    
    /// \do the multiply after fft，经过fft变化，原来的乘法运算变成了加法运算
    AllocVectMem(hMemMgr,TDres,width*height,COMPLEX);
    AllocVectMem(hMemMgr,FDres,width*height,COMPLEX);
    SetVectMem(TDres,width*height,0,COMPLEX);
    SetVectMem(FDres,width*height,0,COMPLEX);
    for(i=0;i<height;i++)
    for(j=0;j<width;j++)
      {
            k=i*width+j;
            FDres[k].re=FDsrc[k].re*sf[k].re-FDsrc[k].im*sf[k].im;
            FDres[k].im=FDsrc[k].re*sf[k].im+FDsrc[k].im*sf[k].re;
        }
    
    /// \ do the fft inverse ，二维fft反变化
    IFFT2(hMemMgr,TDres,width,height,FDres);

    /// \get the useful information，得到有用区域的信息
    datares=(MUInt8*)blockres->block.pBlockData+blockres->ext.top*blockres->block.lBlockLine+blockres->ext.left;
    extres=blockres->block.lBlockLine-blockres->ext.right+blockres->ext.left;
    for(i=0;i<(blockres->ext.bottom-blockres->ext.top);i++,datares+=extres)
    for(j=0;j<(blockres->ext.right-blockres->ext.left);j++,datares++)
    {
            k=i*(blockres->ext.right-blockres->ext.left)+j;
            *(datares)=TDres[k].re;
    }
EXT:
    B_Release(hMemMgr,&Blocksigma);
    FreeVectMem(hMemMgr,s);
    FreeVectMem(hMemMgr,sf);
    FreeVectMem(hMemMgr,TDsrc);
    FreeVectMem(hMemMgr,TDres);
    FreeVectMem(hMemMgr,FDres);
    FreeVectMem(hMemMgr,FDsrc);
    return res;
   
}
