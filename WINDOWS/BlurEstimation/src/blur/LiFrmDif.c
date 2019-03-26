/*!
* \file LiFrmDif.c
* \brief  the function related to calculate the difference between two frames 
* \author hmy@whayer
* \version vision 1.0 
* \date 18 July 2014
*/

#include"lierrdef.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "LiFrmDif.h"


/// \ 计算两帧图像的帧差
MRESULT LiFrmDif(PJOFFSCREEN srcimg1, PJOFFSCREEN srcimg2,MFloat *Diff)
{
    MLong res=LI_ERR_NONE;
    MLong temp;
    MLong i,j,extsrc1,extsrc2,n;
    MUInt8*datasrc1=MNull;
    MUInt8*datasrc2=MNull;
    n=0;

    extsrc1=srcimg1->pixelArray.chunky.dwImgLine-srcimg1->dwWidth;
    extsrc2=srcimg2->pixelArray.chunky.dwImgLine-srcimg2->dwWidth;
    datasrc1=(MUInt8*)srcimg1->pixelArray.chunky.pPixel;
    datasrc2=(MUInt8*)srcimg2->pixelArray.chunky.pPixel;
	/// \两帧图像相减
    for(i=0;i<srcimg1->dwHeight;i++,datasrc1+=extsrc1,datasrc2+=extsrc2)
    for(j=0;j<srcimg2->dwWidth;j++,datasrc1++,datasrc2++)
    {
        temp=*(datasrc1)-*(datasrc2);
        if(0==temp)
        {
            n++;
        }
       
    }
    *Diff=(MFloat)n/(srcimg1->dwWidth*srcimg2->dwHeight);
    return res;
}
