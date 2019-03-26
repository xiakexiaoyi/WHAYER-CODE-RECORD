/*!
* \file LiBright.c
* \brief  the function related to hsv and image cast  
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#include"LiBright.h"
#include"lierrdef.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include  "Lihist.h"
#include"limatrix.h"


/// \获取某一个通道的数据
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

/// \设置某一个通道的值
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





/// \brief  LiEnhance  enhance the image
/// \param  hMemMgr  Handle
/// \param  imgres the image after enhancement
/// \param  imgsrc the source image 
/// \return  error code
/// \根据直方图增强图像
MRESULT liEnhance(MHandle hMemMgr,PJOFFSCREEN imgsrc,PJOFFSCREEN imgres)
{
    MLong res=LI_ERR_NONE;
    BGR bgrsrc={0};
    HSV HSVsrc={0};
    MLong len,width,height;
    len=(imgsrc->dwWidth)*(imgsrc->dwHeight);
    width=imgsrc->dwWidth;
    height=imgsrc->dwHeight;
    imgsrc->pixelArray.chunky.dwImgLine;
    AllocVectMem(hMemMgr,bgrsrc.B,len,MUInt8);
    AllocVectMem(hMemMgr,bgrsrc.G,len,MUInt8);
    AllocVectMem(hMemMgr,bgrsrc.R,len,MUInt8);
    AllocVectMem(hMemMgr,HSVsrc.H,len,MUInt16);
    AllocVectMem(hMemMgr,HSVsrc.S,len,MDouble);
    AllocVectMem(hMemMgr,HSVsrc.V,len,MDouble);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.B,width,0,MUInt8);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.G,width,1,MUInt8);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.R,width,2,MUInt8);
    BGRTOHSV(bgrsrc,HSVsrc,width,height);
    LIHISTEQUALH(HSVsrc.V,width,height);
    HSVTOBGR(bgrsrc,HSVsrc,width,height);
    SET_CHANNEL(imgres->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.B,width,0,MUInt8);
    SET_CHANNEL(imgres->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.G,width,1,MUInt8);
    SET_CHANNEL(imgres->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgrsrc.R,width,2,MUInt8);
EXT:
    FreeVectMem(hMemMgr,bgrsrc.B);
    FreeVectMem(hMemMgr,bgrsrc.G);
    FreeVectMem(hMemMgr,bgrsrc.R);
    FreeVectMem(hMemMgr,HSVsrc.H);
    FreeVectMem(hMemMgr,HSVsrc.S);
    FreeVectMem(hMemMgr,HSVsrc.V);
    return res;

}

/// \brief  BGRTOHSV  transform the BGR TO HSV
/// \param bgrsrc the source bgr
/// \param  hsvres the wanted hsv
/// \param  width the data width
/// \param height the data height
/// \return  error code
/// \将BGR图像转成HSV格式的图像
MRESULT BGRTOHSV(BGR bgrsrc,HSV hsvres,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong i,j,n,h;
    MUInt8*dataR;
    MUInt8 *dataG;
    MUInt8 *dataB;
    MUInt16 *dataH;
    MDouble *dataS;
    MDouble *dataV;
   
    MDouble maxrgb,minrgb;
    MDouble S,V,R,G,B,H,temp;
    dataB=bgrsrc.B;
    dataG=bgrsrc.G;
    dataR=bgrsrc.R;
    dataH=hsvres.H;
    dataS=hsvres.S;
    dataV=hsvres.V;
    maxrgb=0;
    minrgb=1;
   
    for(i=0;i<height;i++)
    for(j=0;j<width;j++)
    {
        n=i*width+j;
        R=*(dataR+n)/255.0;
        G=*(dataG+n)/255.0;
        B=*(dataB+n)/255.0;
        maxrgb=MAX(R,G);
        maxrgb=MAX(maxrgb,B);
        minrgb=MIN(R,G);
        minrgb=MIN(minrgb,B);
       V=maxrgb;
        temp=maxrgb-minrgb;
        if (maxrgb==0)
        {
            S=0.0;
         }
        else
        {
            S=temp/maxrgb;
        }
        if (maxrgb==minrgb)
        {
            H=0;
        }
        else
        {
            if(R==maxrgb&&G>=B)
            {
               H=60*(G-B)/temp;
            }
            else if(R==maxrgb &&G<B)
                {
                    H=60*(G-B)/temp+360;
            }
            else if(G==maxrgb)
            {
                H=60*(B-R)/temp+120;
            }
            else if(B==maxrgb)
            {
              H=60*(R-G)/temp+240;
            }

        }

       H=(int)(H+0.5);
       H=(H>359)?(H-360):H;
       H=(H<0)?(H+360):H;
       h=H;
       *(dataH+n)=h;
       *(dataS+n)=S;
       *(dataV+n)=V;

    }
    


    return res;
}

/// \brief  HSVTOBGR  transform HSV TO BGR
/// \param bgrres the wanted bgr
/// \param  hsvsrc the source hsv
/// \param  width the data width
/// \param height the data height
/// \return  error code
/// \将HSV格式转回BGR格式
MRESULT HSVTOBGR(BGR bgrres,HSV hsvsrc,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong H,i,j,n,flag;
    MDouble S,V,R,G,B,f,q,p,t;
    MUInt8 *dataR,*dataG,*dataB;
    MUInt16*dataH;
    MDouble *dataS,*dataV;
    MLong Blue,green,red;

    dataB=bgrres.B;
    dataG=bgrres.G;
    dataR=bgrres.R;
    dataH=hsvsrc.H;
    dataS=hsvsrc.S;
    dataV=hsvsrc.V;
    B=0;
    G=0;
    R=0;
    for(i=0;i<height;i++)
    for(j=0;j<width;j++)
    {
        n=i*width+j;
        H=*(dataH+n);
        S=*(dataS+n);
        V=*(dataV+n);
        flag=(int)abs(H/60.0);
        f=H/60.0-flag;
        p=V*(1-S);
        q=V*(1-f*S);
        t=V*(1-(1-f)*S);
        
        switch(flag)
        {
        case 0:
            B = p;
            G = t;
            R = V;
            break;
        case 1:
            B = p;
            G= V;
            R = q;
            break;
        case 2:
            B = t;
            G= V;
            R = p;
            break;
        case 3:
            B= V;
            G= q;
            R = p;
            break;
        case 4:
            B= V;
            G = q;
            R= t;
            break;
        case 5:
            B = q;
            G = p;
            R = V;
            break;
        default:
            break;
        }
     Blue=(MLong)(B*255);
     Blue=(Blue>255)?255:Blue;
     Blue=(Blue<0)?0:Blue;
     *(dataB+n)=Blue;
     green=(MLong)(G*255);
     green=(green>255)?255:green;
     green=(green<0)?0:green;
     *(dataG+n)=green;
     red=(MLong)(R*255);
     red=(red>255)?255:red;
     red=(red<0)?0:red;
     *(dataR+n)=red;

    }


    return res;

}
/// \brief  LiCast  check the cast ratio of the image
/// \param imgsrc the source image
/// \param  hMemMgr Handle
/// \param  ratiosrc the cast ratio
/// \return  error code
/// \ 判别BGR格式的图像的偏色程度
MRESULT LiCast(MHandle hMemMgr,PJOFFSCREEN imgsrc,PRatio ratiosrc)
{
    MLong res=LI_ERR_NONE;
    BGR bgr;
	MLong sumbr,sumbg,sumbb;
    MLong len=imgsrc->dwHeight*imgsrc->dwWidth;
    MLong width=imgsrc->dwWidth;
    MLong height=imgsrc->dwHeight;
    MLong n,sumr,sumg,sumb,sum,i,tempr,tempg,tempb;
    MDouble meanr,meang,meanb,meant;
	MDouble meanbr,meanbg,meanbb;
    MLong B[256],G[256],R[256];

    if (imgsrc->fmtImg!=FORMAT_BGR)
    {
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }
/// \get r,g,b data;
    AllocVectMem(hMemMgr,bgr.B,len,MUInt8);
    AllocVectMem(hMemMgr,bgr.G,len,MUInt8);
    AllocVectMem(hMemMgr,bgr.R,len,MUInt8);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgr.B,width,0,MUInt8);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgr.G,width,1,MUInt8);
    GET_CHANNEL(imgsrc->pixelArray.chunky.pPixel,width,height, imgsrc->pixelArray.chunky.dwImgLine,3,bgr.R,width,2,MUInt8);
    n=len*0.40;
    sumr=0;
    sumg=0;
    sumb=0;
	sumbb=0;
	sumbr=0;
	sumbg=0;
    for(i=0;i<256;i++)
    {
        B[i]=0;
        G[i]=0;
        R[i]=0;
    }
    for(i=0;i<len;i++)
    {
        tempb=*(bgr.B+i);
        tempg=*(bgr.G+i);
        tempr=*(bgr.R+i);
        B[tempb]=B[tempb]+1;
        G[tempg]=G[tempg]+1;
        R[tempr]=R[tempr]+1;
    }

    tempb=0;
    tempg=0;
    tempr=0;
///计算全局平均值
	meanbb=0.0;
	meanbg=0.0;
	meanbr=0.0;
   for (i=0;i<len;i++)
   {
	   sumbb=sumbb+*(bgr.B+i);
	   sumbg=sumbg+*(bgr.G+i);
	   sumbr=sumbr+*(bgr.R+i);
   }
   meanbb=(MFloat)sumbb/len;
   meanbg=(MFloat)sumbg/len;
   meanbr=(MFloat)sumbr/len;
   ratiosrc->bdev=fabs((meanbb-128)/128);
   ratiosrc->gdev=fabs((meanbg-128)/128);
   ratiosrc->rdev=fabs((meanbr-128)/128);
///计算最大的40%的像素值
    for (i=255;i>-1;i--)
    {
        if ((tempb<n)&&((n-tempb)>=B[i])) //剩余位置大于当前直方图端点频数
        {
            sumb=sumb+i*B[i];
            tempb=B[i]+tempb;
        }
        else  if(tempb<n&&(n-tempb)<B[i])//剩余位置小于当前直方图端点频数
        {
            sumb=sumb+(n-tempb)*i;
            tempb=n;
        }
        if (tempr<n&&(n-tempr)>=R[i])
        {
            sumr=sumr+i*R[i];
            tempr=tempr+R[i];
        }
        else if(tempr<n&&(n-tempr)<R[i])
        {
            sumr=sumr+(n-tempr)*i;
            tempr=n;
        } 
        if (tempg<n&&(n-tempg)>=G[i])
        {
            sumg=sumg+i*G[i];
            tempg=G[i]+tempg;
        }
        else if(tempg<n&&(n-tempg)<G[i])
        {
            sumg=sumg+(n-tempg)*i;
            tempg=n;
        }

    }
	///计算颜色偏差
    sum=sumg+sumb+sumr;
    meanr=(double)sumr/n;
    meang=(double)sumg/n;
    meanb=(double)sumb/n;
    meant=(double)sum/(3*n);
    ratiosrc->bratio=fabs((meanb-meant)/meant);
    ratiosrc->gratio=fabs((meang-meant)/meant);
    ratiosrc->rratio=fabs((meanr-meant)/meant);
	
EXT:
    FreeVectMem(hMemMgr,bgr.B);
    FreeVectMem(hMemMgr,bgr.G);
    FreeVectMem(hMemMgr,bgr.R);
    return res;
}