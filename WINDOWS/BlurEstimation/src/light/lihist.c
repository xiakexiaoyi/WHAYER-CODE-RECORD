/*!
* \file Lihist.c
* \brief  the function related histogram equalization
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#include"Lihist.h"
#include"lierrdef.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"  

/// \brief  LiHISTEQUAL histogram equalization 
/// \param  datasrc the source data
/// datatype is U8
/// \param   width the data width
/// \param  height the data height
/// \return  error code
MRESULT  LIHISTEQUAL(MUInt8 *datasrc,MLong width,MLong height)
{
   MRESULT res=LI_ERR_NONE;
   MLong NJ[256];
   MDouble PJ[256];
   MLong GI[256];
   MDouble cf=0;
   MLong i,j,n;
   
   for(i=0;i<256;i++)
   {
       NJ[i]=0;
   }
   for(i=0;i<height;i++)
   for(j=0;j<width;j++)
   {
       n=*(datasrc+i*width+j);
       NJ[n]=NJ[n]+1;
   }
   for(i=0;i<256;i++)
   {
       PJ[i]=(double)NJ[i]/(width*height);
   }
   for(i=0;i<256;i++)
   {
       cf=cf+PJ[i];
       GI[i]=(int)(cf*255);

   }
   for( i = 0;i<height ;i++)
   {
   for( j=0; j<width ;j++)
       {
            n=*(datasrc+i*width+j);
           *(datasrc)=GI[n];
         
       }
   }

return res;
}

/// \brief  LiHiSTEQUALH histogram equalization for V of hsv
/// \param  datasrc the source data
/// datatype is double ,and is between[0,1]
/// \param   width the data width
/// \param  height the data height
/// \return  error code
MRESULT LIHISTEQUALH(MDouble *datas,MLong width,MLong height)
{
    MRESULT res=LI_ERR_NONE;
    MLong NJ[256];
    MDouble PJ[256];
    MLong GI[256];
    MDouble cf=0;
    MLong i,j,n;
    for(i=0;i<256;i++)
    {
        NJ[i]=0;
    }
    for(i=0;i<height;i++)
        for(j=0;j<width;j++)
        {
            n=(*(datas+i*width+j))*255;
            NJ[n]=NJ[n]+1;
        }
       
        for(i=0;i<256;i++)
        {
            PJ[i]=(double)NJ[i]/(width*height);
        }
        for(i=0;i<256;i++)
        {
            cf=cf+PJ[i];
            GI[i]=(int)(cf*255);

        }
        for( i = 0; i<height ;i++)
        {
            for( j=0; j<width ;j++)
            {
               n=(*(datas+i*width+j))*255;
                *(datas+i*width+j)=GI[n]/255.0;

            }
        }

        return res;
}