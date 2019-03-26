/*!
* \file FmttransYUV2BGR.h
* \brief  YUV420 transform to BGR
* \author hmy@whayer
* \version vision 1.0 
* \date 4 July 2014
*/

#include"lierrdef.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include"limatrix.h"
#include "FmttransYUV2BGR.h"


/// \brief  trans YUV420 TO BGR FORMAT
/// \param Imgsrc the input img;
/// \param Imgrlt the output img;
/// \return the error code
/// \将YUV420格式的图像转成为BGR
 MRESULT FmttransYUV2BGR(PJOFFSCREEN Imgsrc,PJOFFSCREEN  Imgrlt )
 {
     MLong res=LI_ERR_NONE;
     MLong width=Imgsrc->dwWidth;
     MLong height=Imgsrc->dwHeight;
     MUInt8 *datasrcy=MNull; ///y通道
     MUInt8 *datasrcu=MNull; ///u通道
     MUInt8 *datasrcv=MNull; ///v通道
     MUInt8 *datarltb=MNull; ///b通道
     MUInt8 *datarltg=MNull;   ///g通道
     MUInt8 *datarltr=MNull;    ///r通道
     MLong i,j,n,m;
     MLong tempy,tempu,tempv,tempb,tempg,tempr;
     tempy=0;
     tempu=0;
     tempv=0;
     if (Imgsrc->fmtImg!=FORMAT_YUV420||Imgrlt->fmtImg !=FORMAT_BGR)
     {
         res=LI_ERR_DATA_UNSUPPORT;
         return res;
     }
     datasrcy=(MUInt8*)Imgsrc->pixelArray.chunky.pPixel;
     datasrcu=(MUInt8*)Imgsrc->pixelArray.chunky.pPixel+(width*height);
     datasrcv=(MUInt8*)Imgsrc->pixelArray.chunky.pPixel+(width*height)+(width*height/4);
     datarltb=(MUInt8*)Imgrlt->pixelArray.chunky.pPixel;
     datarltg=(MUInt8*)Imgrlt->pixelArray.chunky.pPixel+1;
     datarltr=(MUInt8*)Imgrlt->pixelArray.chunky.pPixel+2;
     for(i=0;i<height;i++)
     for(j=0;j<width;j++)
     {
         
         tempy=*(datasrcy+i*width+j);
         n=i%2;
         m=(j/2);
         if(n==0)
         {
             tempu=*(datasrcu+(MLong)(i/2)*width/2+m);
             tempv=*(datasrcv+(MLong)(i/2)*width/2+m);
         }
         if(n==1)
         {
             tempu=*(datasrcu+(MLong)((i-1)/2)*width/2+m);
             tempv=*(datasrcv+(MLong)((i-1)/2)*width/2+m);

         }
		 ///根据YUV420转化成BGR格式的图像的公式
         tempb=tempy+1.779*(tempu-128);
         tempg=tempy-0.3455*(tempu-128)-0.7169*(tempv-128);
         tempr=tempy+1.4075*(tempv-128);
         tempb=tempb>255?255:tempb;
         tempb=tempb<0?0:tempb;
         tempg=tempg>255?255:tempg;
         tempg=tempg<0?0:tempg;
         tempr=tempr>255?255:tempr;
         tempr=tempr<0?0:tempr;
         *(datarltb+j*3+i*width*3)=tempb;
         *(datarltg+j*3+i*width*3)=tempg;
         *(datarltr+j*3+i*width*3)=tempr;

     }

     return res;
 }