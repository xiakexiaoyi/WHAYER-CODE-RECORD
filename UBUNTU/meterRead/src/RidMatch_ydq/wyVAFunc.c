#include <stdio.h>
#include <stdlib.h>
#include "wyVACommon.h"
#include <math.h>
#include "wyVAFunc.h"
#include "sdkMacro.h"
//#include "wy_hi3516.h"

//#include "hikFunc.h"
//#pragma DATA_SECTION(idxVal," InternalData ");
//#pragma DATA_ALIGN(idxVal,16);
#define WY_M_MIN_NORM 15
const unsigned char idxVal[181]={1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   //0
                                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
                                                1, 1, 1, 1, 1, 1 ,2, 2, 2, 2,  
                                 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                 2, 2, 4, 4, 4, 4, 4, 4, 4, 4,
                                 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                 4, 4, 4, 4, 4, 4, 4, 4, 8, 8,
                                 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                 8, 8, 8, 8, 16, 16, 16, 16, 16, 16,
                                 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
                                 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
                                 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 
                                 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 
                                 32, 32, 32, 32, 32, 64, 64, 64, 64, 64,
                                 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                                 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                                 64
                   };

int bitMapToByteMap_wyVA(unsigned char *bitMap, unsigned char *byteMap, unsigned int width, unsigned int heigth)
{
    unsigned int w, h;
    unsigned char *byteMap_p, *bitMap_p;

    //pingPong Buffer
    if(width&0x7)
    {
       // WY_SDK_ERR("width should align to 8!!!\n");
        return WY_IN_PARA_ERR;
    }
    
    for(h=0;h<heigth;h++)
    {   
        byteMap_p = byteMap+h*width;
        bitMap_p = bitMap+h*(width>>3);
        for(w=0;w<(width>>3);w++)
        {              
            *(byteMap_p+7) = (*bitMap_p&0x1) ? 255:0;
            *(byteMap_p+6) = ((*bitMap_p&0x2)>>1) ? 255:0;
            *(byteMap_p+5) = ((*bitMap_p&0x4)>>2) ? 255:0;
            *(byteMap_p+4) = ((*bitMap_p&0x8)>>3) ? 255:0;
            *(byteMap_p+3) = ((*bitMap_p&0x10)>>4) ? 255:0;
            *(byteMap_p+2) = ((*bitMap_p&0x20)>>5) ? 255:0;
            *(byteMap_p+1) = ((*bitMap_p&0x40)>>6) ? 255:0;
            *(byteMap_p+0) = ((*bitMap_p&0x80)>>7) ? 255:0;	
             bitMap_p+=1;
             byteMap_p+=8;			 
        }
    }
    
    return WY_SUCCESS;
}

void findRecPosOfObjInMaskPic_wyVA(wyVA_picBufA *maskData, int *left, int *right, int *top, int *bottom)
{ 
    int yPos, xPos;
    int width, height;
    int curLeft, curRight, curTop, curBot;
    int curPosX, curPosY, startX, startY;
    
    width = maskData->width;
    height = maskData->height;
    startX = maskData->startX;
    startY = maskData->startY;
    
    curLeft  = *left;
    curRight = *right;
    curTop   = *top;
    curBot   = *bottom;

   // printf("maskData:%x, startX:%d, startY:%d \n",  (unsigned int)maskData->pData,startX, startY);
    for(yPos = 0; yPos < height; yPos++)
    {
        curPosY = yPos +startY;
        for(xPos= 0; xPos<width; xPos++)
        {
            curPosX = xPos + startX;
            
            if(*(maskData->pData + yPos*width + xPos))
            {
                 if(curLeft > curPosX)
                    curLeft = curPosX;
                 if(curRight<curPosX)
                    curRight = curPosX;
                 if(curTop>curPosY)
                    curTop = curPosY;
                 if(curBot<curPosY)
                    curBot = curPosY;    
            }
        }
    }

    *left   = curLeft ;
    *right  = curRight;
    *top    = curTop  ;
    *bottom = curBot  ;
    
    //return WY_SUCCESS;
}

void cutObjRecFrmPic_wyVA(unsigned char *src, unsigned char *dst, int top, int bot, int left ,int right, int strideSrc, int strideDst)
{
   int y, x, i=0;
   unsigned char *pSrc, *pDst;
   
   for(y=top;y<bot;y++)
   {    
        i++;
        pSrc = src + (y*strideSrc + left);

        for(x=left;x<right;x++)
        {
            *pDst = *pSrc;
            pDst++;
            pSrc++;
        }
        
        pDst = dst + i*strideDst;
   }

  // return WY_SUCCESS;

}

int smoothFilter_wyVA(unsigned char *src, unsigned char *dst, int width, int height, int strideSrc, int strideDst)
{
   unsigned char *aP0,*aP1,*aP2;  
   unsigned char *bP0,*bP1,*bP2;
   unsigned char *cP0,*cP1,*cP2;
 //  unsigned char *src, *dst;
   unsigned char *aP0T,*aP1T,*aP2T;  
   unsigned char *bP0T,*bP1T,*bP2T;
   unsigned char *cP0T,*cP1T,*cP2T;
   unsigned char *pDst, *pSrc;
   int x, y, tmp0, tmp1, tmp2;
   int h1, h2, h3, h4;

//xsd
/*   for(y=0; y<height; y++)
   {
		pDst = dst + y*strideDst;
		pSrc = src + y*strideSrc;
		memcpy(pDst, pSrc, sizeof(unsigned char)*width);
   }
   return WY_SUCCESS;	*/
   
   pSrc = src + strideSrc +1;
   aP0T = aP0 = pSrc - strideSrc - 1;
   aP1T = aP1 = pSrc - strideSrc;
   aP2T = aP2 = pSrc - strideSrc + 1;
   
   bP0T = bP0 = pSrc - 1;
   bP1T = bP1 = pSrc ;
   bP2T = bP2 = pSrc + 1;
   
   cP0T = cP0 = pSrc + strideSrc - 1;
   cP1T = cP1 = pSrc + strideSrc;
   cP2T = cP2 = pSrc + strideSrc + 1;

   //xsd
   h1 = height/5;
   h2 = 2* height/5;
   h3 = 3 * height/5;
   h4 = 4 * height/5;
   for(y=0;y<(height-2);y++)
   {
		pDst = dst + y*strideDst; 	
        for(x=0;x<(width-2);x++)
        {
			tmp0 = *aP0T + *aP1T + *aP2T;
			tmp1 = *bP0T + *bP1T + *bP2T;
			tmp2 = *cP0T + *cP1T + *cP2T;
			*pDst = (unsigned char)((tmp0 + tmp1 + tmp2 + 4)/9); //div round :4 or 5;
            
            aP0T += 1;
            aP1T += 1;
            aP2T += 1;

            bP0T += 1;
            bP1T += 1;
            bP2T += 1;

            cP0T += 1;
            cP1T += 1;
            cP2T += 1;
            pDst += 1;
        }
        
       
       aP0T = aP0 += strideSrc;
       aP1T = aP1 += strideSrc;
       aP2T = aP2 += strideSrc;
       
       bP0T = bP0 += strideSrc;
       bP1T = bP1 += strideSrc;
       bP2T = bP2 += strideSrc;
       
       cP0T = cP0 += strideSrc;
       cP1T = cP1 += strideSrc;
       cP2T = cP2 += strideSrc;
   }
   
   return WY_SUCCESS;
}

void picMainDirectionStep1_wyVA(wyVA_picBufA *pic, int *lx, int *ly,
                                                 int *number, WYVA_FLOAT *sumX, WYVA_FLOAT *sumY)
{
    int x, y;
    unsigned char *pData;
    int  numberTemp=0;
    WYVA_FLOAT sumXTemp=0,sumYTemp=0;
    int startX, startY, endX, endY;

    startY = pic->startY;
    startX = pic->startX;
    endY = startY+pic->height;
    endX = startX + pic->width;
    pData = pic->pData;
    
    for(y=startY;y<endY;y++)
    {
        pData = pic->pData + (y-startY)*pic->stride;
        for(x=startX;x<endX;x++)
        {
           if(*pData>0)
           {
                sumXTemp+=x;
                sumYTemp+=y;
                
                *lx = x;
                *ly = y;
                lx++;
                ly++;
                numberTemp++;
           }
           pData++;
        }
    }	
    
    *sumX += sumXTemp;
    *sumY += sumYTemp;
    *number += numberTemp;
    
    //return WY_SUCCESS;
}

void picMainDirectionStep2_wyVA(int *lx, int *ly, WYVA_FLOAT *mat,
                                           WYVA_FLOAT meanX, WYVA_FLOAT meanY, int number)
{
  int i;
  WYVA_FLOAT lxD, lyD;
  
  for(i=0;i<number;i++)
  {
    lxD = *lx-meanX;
    lyD = *ly-meanY;
    mat[0] += lxD*lxD;//mat11
    mat[1] += lxD*lyD;//mat12 mat21
    mat[2] += lyD*lyD;//mat22	
    lx++;
    ly++;
  }
  
  //return WY_SUCCESS;
}

void picMainDirection_wyVA(wyVA_picBuf *pic, int *angle, int *lx, int *ly)
{
   wyVA_picBufA picA;
   WYVA_FLOAT sumX=0, sumY=0, meanX, meanY, result;
   WYVA_FLOAT mat[3]={0,0,0};
   int number = 0;
   
   picA.pData = pic->pData;
   picA.startX = 0;
   picA.startY = 0;
   picA.stride = pic->stride;
   picA.width = pic->width;
   picA.height = pic->height;
   
   picMainDirectionStep1_wyVA(&picA, lx, ly, &number, &sumX, &sumY);
   meanX = sumX/number;
   meanY = sumY/number;

   picMainDirectionStep2_wyVA(lx, ly, mat, meanX, meanY, number); 
   mat[0] /= number;
   mat[1] /= number;
   mat[2] /= number;

   result = (mat[2] - mat[0] + WYVA_SQRT((mat[0]-mat[2])*(mat[0]-mat[2])+4*mat[1]*mat[1]))/2/mat[1]; 
   *angle= (int)(WYVA_ATAN(result)/WYVA_PI*180);

   //return WY_SUCCESS;
}
//right----ptrOut

void rotate90_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut)
{
   int i, j;
   unsigned char *pTmpOut, *pTmpIn, *ptrOutEnd;
   
   ptrOutEnd = ptrOut + inH - 1;
   for(i=0;i<inH;i++)  //outW
   { 
        pTmpOut = ptrOutEnd - i;
        pTmpIn =  ptrIn + i*inStride;
        
        for(j=0;j<inW;j++) //outH
        {
            *pTmpOut = *pTmpIn;  
            pTmpOut+=outStride;
            pTmpIn++;
        }
   }
   
   //return WY_SUCCESS;    
}

//		 rotate180_wyVA(srcStride, srcWidth, 16, dstStride, srcPingPong[0], dstPingPong[0]);
void rotate180_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut)
{
    int i, j;
    unsigned char *pTmpOut, *pTmpIn, *ptrOutEnd;

    ptrOutEnd = ptrOut + outStride*(inH-1) + inW -1;

    for(i=0;i<inH;i++) 
    { 
        pTmpOut = ptrOutEnd - i*outStride;
        pTmpIn =  ptrIn + i*inStride;

        for(j=0;j<inW;j++) 
        {
            *pTmpOut = *pTmpIn;  
            pTmpOut--;
            pTmpIn++;
        }
    }
   
  // return WY_SUCCESS;     
}

//right----ptrIn
void rotate270_wyVA(int inStride, int inW, int inH, int outStride, unsigned char *ptrIn, unsigned char *ptrOut)
{
   int i, j;
   unsigned char *pTmpOut, *pTmpIn, *ptrInEnd;
   ptrInEnd =ptrIn + (inW-1);

   for(i=0;i<inH;i++)  //outW
   {                  
        pTmpOut = ptrOut + i;
        pTmpIn =  ptrInEnd + i*inStride;
                 
        for(j=0;j<inW;j++) //outH
        {                
            *pTmpOut = *pTmpIn;  
            pTmpOut+=outStride;
            pTmpIn--;
        }
   }
                              
  // return WY_SUCCESS;  
}

void rotateX_wyVA(wyVA_picBuf *picIn, wyVA_picBufA *picOut, int *xABC, int *yABC)
{
    int x, y, dstW, dstH, srcW, srcH, srcStride;
    int src_x, src_y, strideAdd;
   // WYVA_FLOAT Ax, Bx, Cx;
   // WYVA_FLOAT Ay, By, Cy;
   // WYVA_FLOAT u, v, R, tmpU, tmpV;

    int Ax, Bx, Cx;
    int Ay, By, Cy;
    int u, v, R, tmpU, tmpV;
    unsigned char A, B, C, D;	
    unsigned char *ptrA, *ptrDst;

	Ax = xABC[0];
    Bx = xABC[1];
    Cx = xABC[2];

    Ay = yABC[0];
    By = yABC[1];
    Cy = yABC[2];
	
    srcW = picIn->width;
    srcH = picIn->height;
    dstW = picOut->width;
    dstH = picOut->height;
    srcStride = picIn->stride;
    
    ptrDst = picOut->pData;
    strideAdd = picOut->stride - picOut->width;
    
    //dstW = (int)(srcW * WY_ABS(sinA) + srcH * WY_ABS(cosA));	
    //dstH =
    
    for(y=picOut->startY; y<(picOut->startY+dstH); y++)
    {
        for(x=picOut->startX; x<(picOut->startX+dstW); x++)
        {		
           tmpV = Ax*x + Bx*y + Cx;
           tmpU = Ay*x + By*y + Cy;

           src_x = tmpV>>FLOAT_SHFIT_BITNUM;
           src_y = tmpU>>FLOAT_SHFIT_BITNUM;
		   
           v = tmpV&((1<<FLOAT_SHFIT_BITNUM)-1);
           u = tmpU&((1<<FLOAT_SHFIT_BITNUM)-1);

           *ptrDst = 0;
           if(src_y<srcH-1 && src_x<srcW-1 && src_y>=0 && src_x>=0)
           {				              
                ptrA = picIn->pData + src_y*picIn->stride + src_x;
                A = *ptrA;
                B = *(ptrA+1);
                C = *(ptrA+srcStride);
                D = *(ptrA+srcStride+1);
                R = (u*v*D + u*(FLOAT_SCALOR_NUM-v)*C + v*(FLOAT_SCALOR_NUM-u)*B+(FLOAT_SCALOR_NUM-v)*(FLOAT_SCALOR_NUM-u)*A+
                    ( 1<<(FLOAT_SHFIT_BITNUM*2-1) ) )>>(FLOAT_SHFIT_BITNUM<<1); //max value before shift right:ff800  FLOAT_SHFIT_BITNUM:6bit
                *ptrDst = (unsigned char)(R);
           }
           
           ptrDst++;
        }
        ptrDst +=strideAdd;
    }
    
    //return WY_SUCCESS;
}

void scalorBLinear_wyVA(wyVA_picBufA *picIn, wyVA_picBufA *picOut,  int nXFactorInt, int nYFactorInt)
{
    //WYVA_FLOAT nXFactor,nYFactor;
    int dstW, dstH;
    int src_x, src_y, strideAdd;	
    int x, y, srcW, srcH, srcStride, wClip, hClip;	
    unsigned char *ptrA, *ptrDst;
    int u, v, R, tmpV, tmpU;
    unsigned char A, B, C, D;

    srcW = picIn->width;
    srcH = picIn->height;
    dstW = picOut->width;
    dstH = picOut->height;
    srcStride = picIn->stride;

    ptrDst = picOut->pData;
    strideAdd = picOut->stride - picOut->width;
    //nXFactorInt = (int)(FLOAT_SCALOR_NUM*nXFactor);
   // nYFactorInt = (int)(FLOAT_SCALOR_NUM*nYFactor);
    //nXFactor = w_dst*1.0/w_src;
    //nYFactor = h_dst*1.0/h_src;

    for(y=picOut->startY; y<(picOut->startY+dstH); y++)
    {
        for(x=picOut->startX; x<(picOut->startX+dstW); x++)
        {
            tmpV = x*nXFactorInt;   
            tmpU = y*nYFactorInt;
            src_x = tmpV>>FLOAT_SHFIT_BITNUM;
            src_y = tmpU>>FLOAT_SHFIT_BITNUM;

            v = tmpV&((1<<FLOAT_SHFIT_BITNUM)-1);
            u = tmpU&((1<<FLOAT_SHFIT_BITNUM)-1);
            wClip = src_x==(srcW-1) ? 0:1;
            hClip = src_y==(srcH-1) ? 0:srcStride;

            //   if(src_x<picIn->startX)
            // 	printf("erorrrrrrrrrrrrrrr\n");

            ptrA = picIn->pData + (src_y-picIn->startY)*picIn->stride + (src_x-picIn->startX);
            A = *ptrA;
            B = *(ptrA+wClip);
            C = *(ptrA+hClip);
            D = *(ptrA+hClip+wClip);

            R = (u*v*D + u*(FLOAT_SCALOR_NUM-v)*C + v*(FLOAT_SCALOR_NUM-u)*B+(FLOAT_SCALOR_NUM-v)*(FLOAT_SCALOR_NUM-u)*A+
                ( 1<<(FLOAT_SHFIT_BITNUM*2-1) ) )>>(FLOAT_SHFIT_BITNUM<<1); //max value before shift right:ff800
            *ptrDst = (unsigned char)(R);
            ptrDst++;
        }
        ptrDst +=strideAdd;
    }

    //return WY_SUCCESS;	
}
/*
void scalorBLinear_wyVAD(wyVA_picBufA *picIn, wyVA_picBufA *picOut, int sh, WYVA_FLOAT nXFactor, WYVA_FLOAT nYFactor)
{
    //WYVA_FLOAT nXFactor,nYFactor;
    int dstW, dstH;
    int src_x, src_y, strideAdd;	
    int x, y, srcW, srcH, srcStride, wClip, hClip;	
    unsigned char *ptrA, *ptrDst;
    WYVA_FLOAT u, v, R, tmpV, tmpU;
    unsigned char A, B, C, D;
    
    srcW = picIn->width;
    srcH = picIn->height;
    dstW = picOut->width;
    dstH = picOut->height;
    srcStride = picIn->stride;
    
    ptrDst = picOut->pData;
    strideAdd = picOut->stride - picOut->width;
    
    //nXFactor = w_dst*1.0/w_src;
    //nYFactor = h_dst*1.0/h_src;
    
    for(y=picOut->startY; y<(picOut->startY+dstH); y++)
    {
        for(x=picOut->startX; x<(picOut->startX+dstW); x++)
        {		
           tmpV = x*nXFactor;   
           tmpU = y*nYFactor;
           src_x = (int)(tmpV);
           src_y = (int)(tmpU);
           v = tmpV-src_x;
           u = tmpU-src_y;
           wClip = src_x==(srcW-1) ? 0:1;
           hClip = src_y==(srcH-1) ? 0:srcStride;

		   if((src_y-picIn->startY)>(sh+3))
		   	printf("erorr000000000rrrrrrrrrrrrr\n");

		//   if(src_x<picIn->startX)
		  // 	printf("erorrrrrrrrrrrrrrr\n");
					   
           ptrA = picIn->pData + (src_y-picIn->startY)*picIn->stride + (src_x-picIn->startX);
           A = *ptrA;
           B = *(ptrA+wClip);
           C = *(ptrA+hClip);
           D = *(ptrA+hClip+wClip);
           
           R = u*v*D + u*(1-v)*C + v*(1-u)*B+(1-v)*(1-u)*A;  
           *ptrDst = (unsigned char)(R);
           ptrDst++;
        }
        ptrDst +=strideAdd;
    }
     
    //return WY_SUCCESS;	
}
*/
void scalorDesBLinear_wyVA(wyVA_picBufA *picIn, wyVA_picBufA *picOut, int nXFactorInt, int nYFactorInt)
{
    //WYVA_FLOAT nXFactor,nYFactor;
    int dstW, dstH ;
    int src_x, src_y, strideAdd;	
    int x, y, srcW, srcH, srcStride, wClip, hClip;	
    unsigned char *ptrA, *ptrDst;
    int u, v, R, tmpV, tmpU;

  //  WYVA_FLOAT u, v, R, tmpV, tmpU;
    unsigned char A, B, C, D;
    
    srcW = picIn->width;
    srcH = picIn->height;
    dstW = picOut->width;
    dstH = picOut->height;
    srcStride = picIn->stride;
    
    ptrDst = picOut->pData + picOut->startY*picOut->stride + picOut->startX;
    strideAdd = picOut->stride - picOut->width;
   // nXFactorInt = (int)(FLOAT_SCALOR_NUM*nXFactor);
   // nYFactorInt = (int)(FLOAT_SCALOR_NUM*nYFactor);
    //nXFactor = w_dst*1.0/w_src;
    //nYFactor = h_dst*1.0/h_src;
    
    for(y=picOut->startY; y<(picOut->startY+dstH); y++)
    {
        for(x=picOut->startX; x<(picOut->startX+dstW); x++)
        {		
            tmpV = x*nXFactorInt;   
            tmpU = y*nYFactorInt;
            src_x = tmpV>>FLOAT_SHFIT_BITNUM;
            src_y = tmpU>>FLOAT_SHFIT_BITNUM;

            v = tmpV&((1<<FLOAT_SHFIT_BITNUM)-1);
            u = tmpU&((1<<FLOAT_SHFIT_BITNUM)-1);
           wClip = src_x==(srcW-1) ? 0:1;
           hClip = src_y==(srcH-1) ? 0:srcStride;

           ptrA = picIn->pData + (src_y-picIn->startY)*picIn->stride + (src_x-picIn->startX);
           A = *ptrA;
           B = *(ptrA+wClip);
           C = *(ptrA+hClip);
           D = *(ptrA+hClip+wClip);
           
           R = (u*v*D + u*(FLOAT_SCALOR_NUM-v)*C + v*(FLOAT_SCALOR_NUM-u)*B+(FLOAT_SCALOR_NUM-v)*(FLOAT_SCALOR_NUM-u)*A+
               ( 1<<(FLOAT_SHFIT_BITNUM*2-1) ) )>>(FLOAT_SHFIT_BITNUM<<1); //max value before shift right:ff800
           *ptrDst = (unsigned char)(R);
           ptrDst++;
        }
        ptrDst +=strideAdd;
    }
     
    //return WY_SUCCESS;	
}

int refineDes_wyVA(unsigned char *pDesData, int desW, int desH, unsigned char *maskData, int maskStride)
{
   int x, y, strideAdd, count = 0;
   unsigned char *pDesTmp, *pMaskTmp;

   pDesTmp = pDesData;
   pMaskTmp = maskData;
   
   strideAdd = maskStride - desW;
   
   for(y=0;y<desH;y++)
   {          
        for(x=0;x<desW;x++)
        {
            if(!(*pMaskTmp))
            {
                *pDesTmp = 0;
                count++;
            }
          //  else
            //    *pDesTmp = 255;
           
            pDesTmp++;
            pMaskTmp++;
        }
        pMaskTmp+=strideAdd;
   }

   return (desW*desH-count);   
}

int findMaxIn5x5Matrix_wyVA(unsigned short *pMag, int magStride, int *xPos, int *yPos)
{
   int x, y, lx,ly, strideAdd;
   unsigned short mag=0;   
   unsigned short *magPtr;
   
   magPtr = pMag;
   strideAdd = magStride - 5 ; 
   lx = ly = 0;

   for(y=0;y<5;y++)
   {    
        for(x=0;x<5;x++)
        {		    
            if(*magPtr>mag)
            {
                mag = *magPtr;	
                lx = x;
                ly = y;
            }
            magPtr++;
        }	
        magPtr += strideAdd;
   }

   *xPos = lx;
   *yPos = ly;
   
   return (int)mag;
}

void getDesBinMatrix(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, 
                                               int stride, int width, int height, WYVA_FLOAT magRate)
{
   int x, y,y3;
   unsigned char binCal;
   unsigned short *magPtr;
   unsigned short *magPtr3;
   unsigned char *anglePtr, *anglePtr3;
    
#if(WY_DEBUG)
   //  char name[10];
   //char num[10];
   FILE *fileOutFp=NULL; 
   int i,j;
   unsigned char magVal;
#endif

#if(WY_DEBUG)
  // itoa(rotateAngle, num, 10);

   fileOutFp = fopen("binMag.yuv", "ab+");
   if(!fileOutFp)
   {
       printf("file open error!");
   }

   if(fileOutFp)
   {
       for(i=0;i< (height-4);i++)
       {
           for(j=0;j<(width);j++)
           {
               magVal = (unsigned char)(*(pMag + i*stride + j));
               fwrite(&magVal, sizeof(unsigned char),1, fileOutFp);
           }
       }

       //printf("scalor %s width %d height %d\n", num, salorSfObjPic->width-2, salorSfObjPic->height-2);
       //fwrite(pMag, sizeof(unsigned char),ddrMem->magAngStride*(salorSfObjPic->height-2), fileOutFp);
       fclose(fileOutFp);
   }
#endif

   for(y=0;y<(height-8);y+=5)
   {
        anglePtr = pAngle + stride*y;
        magPtr = pMag + stride*y;
      //  if(85==y)
           // printf("lalallalalalalal\n");

        for(x=0;x<(width-8);x+=5)
        {
            binCal = 0;
            for(y3=0;y3<6;y3+=2)
            {
                anglePtr3 = anglePtr + y3*stride;
                magPtr3 = magPtr + y3*stride;
                                        
                getDesBinIn5x5Matrix_wyVA(&binCal, anglePtr3, magPtr3, stride, magRate);
                getDesBinIn5x5Matrix_wyVA(&binCal, anglePtr3+2, magPtr3+2, stride, magRate);
                getDesBinIn5x5Matrix_wyVA(&binCal, anglePtr3+4, magPtr3+4, stride, magRate);                                                  
            }
            anglePtr += 5;
            magPtr += 5;
            *binVal = binCal;
            binVal++;
        }
   }
   
   //return WY_SUCCESS;
}

void getMaxBinMatrix(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, 
                                               int stride, int width, int height)
{
   int x, y;
  // unsigned char binCal;
   unsigned short *magPtr;
   unsigned char *anglePtr;
   //unsigned short *anglePtr3, *magPtr3;
 
#if(WY_DEBUG)
   //  char name[10];
   //char num[10];
   FILE *fileOutFp=NULL; 
   int i,j;
   unsigned char magVal;
#endif

#if(WY_DEBUG)
   // itoa(rotateAngle, num, 10);

   fileOutFp = fopen("binMag.yuv", "ab+");
   if(!fileOutFp)
   {
       printf("file open error!");
   }

   if(fileOutFp)
   {
       for(i=0;i< (height);i++)
       {
           for(j=0;j<(width);j++)
           {
               magVal = (unsigned char)(*(pMag + i*stride + j));
               fwrite(&magVal, sizeof(unsigned char),1, fileOutFp);
           }
       }

       //printf("scalor %s width %d height %d\n", num, salorSfObjPic->width-2, salorSfObjPic->height-2);
       //fwrite(pMag, sizeof(unsigned char),ddrMem->magAngStride*(salorSfObjPic->height-2), fileOutFp);
       fclose(fileOutFp);
   }
#endif

   for(y=0;y<(height-4);y+=5)
   {
        anglePtr = pAngle + stride*y;
        magPtr = pMag + stride*y;
        
        for(x=0;x<(width-4);x+=5)
        {
            getmaxBinIn5x5Matrix_wyVA(binVal, anglePtr+x, magPtr+x, stride);		   	
            binVal++;
        }
   }
   
   //return WY_SUCCESS;
}


void getDesBinIn5x5Matrix_wyVA(unsigned char *binVal, unsigned char *pAngle, unsigned short *pMag, int stride, WYVA_FLOAT magRate)
{
      unsigned short MagTmp[7][3];// X Y Val 
      int xPos, yPos, p1, pVal, i, j;
      unsigned char angle180, bin;
      unsigned int angleIdx;
	  
      bin = *binVal;
      pVal = p1 = findMaxIn5x5Matrix_wyVA(pMag, stride, &xPos, &yPos);
      MagTmp[0][0] = xPos;
      MagTmp[0][1] = yPos;
      MagTmp[0][2] = p1;
                                                                  
      if(p1<WY_M_MIN_NORM)
        bin |= 0x80;
      else
      {
         for(i=0;i<7;i++)
         {
             if(pVal <	p1*magRate)
                break;
             else
             {
                MagTmp[i][0] = xPos; 
                MagTmp[i][1] = yPos;
                MagTmp[i][2] = pVal;
                angle180 =  *(pAngle + stride*yPos + xPos);

angleIdx = angle180 > 180 ? 180:angle180;

              //  angle180 = angle>=180 ? (angle-180):angle; 
				
                bin |= idxVal[angleIdx];
                *(pMag + stride*yPos + xPos) = 0;
                             
                pVal = findMaxIn5x5Matrix_wyVA(pMag, stride, &xPos, &yPos); //p7
                
             }
         }
                             
         for(j=0;j<i;j++)
         {           
           *(pMag + stride*MagTmp[j][1] + MagTmp[j][0]) = MagTmp[j][2];
         }
 
      }
      *binVal = bin;       	  
}

void getmaxBinIn5x5Matrix_wyVA(unsigned char *bin, unsigned char *pAngle, unsigned short *pMag, int stride)
{
    int  x, y, strideAdd, xPos, yPos;
    unsigned short p1 = 0;
    unsigned short *magPtr;
    unsigned char angle180;
    unsigned int angleIdx;
	
    magPtr = pMag;
    strideAdd = stride - 5 ; 
    
    for(y=0;y<5;y++)
    {    
        for(x=0;x<5;x++)
        {		    
            if(*magPtr>p1)
            {
                p1 = *magPtr;
                xPos = x;
                yPos = y;
            }
            magPtr++;
        }	
        magPtr += strideAdd;
    }
    
    if(p1<WY_M_MIN_NORM)
        *bin = 0x80;
    else
    {
        angle180 =	*(pAngle + stride*yPos + xPos);


angleIdx = angle180 > 180 ? 180:angle180;
	       
        *bin = idxVal[angleIdx];
    }    
}

#if(HI3516_WY)
int calAngleAndMagMatrix_wyVA(unsigned int USrc, unsigned int UAngle,unsigned int UMag, int widthSrc, 
                                                                      int height, int strideSrc, int strideDst)
{
    IVE_SRC_INFO_S stSrc;
	IVE_MEM_INFO_S stDstMag;
	IVE_MEM_INFO_S stDstAng;
	IVE_CANNY_CTRL_S stCannyCtrl;

	IVE_HANDLE IveHandle;
	HI_BOOL bFinish;
	HI_S32 s32Ret; 

	stSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
	stSrc.stSrcMem.u32PhyAddr = USrc;
	stSrc.stSrcMem.u32Stride = strideSrc;
	stSrc.u32Height = height;
	stSrc.u32Width = widthSrc;

	stDstMag.u32PhyAddr = UMag;
	stDstMag.u32Stride = strideDst;

	stDstAng.u32PhyAddr = UAngle;
	stDstAng.u32Stride = strideDst;

	stCannyCtrl.as8Mask[0] = -1;
	stCannyCtrl.as8Mask[1] = 0;
	stCannyCtrl.as8Mask[2] = 1;

	stCannyCtrl.as8Mask[3] = -2;
	stCannyCtrl.as8Mask[4] = 0;
	stCannyCtrl.as8Mask[5] = 2;

	stCannyCtrl.as8Mask[6] = -1;
	stCannyCtrl.as8Mask[7] = 0;
	stCannyCtrl.as8Mask[8] = 1;

	stCannyCtrl.enOutFmt = IVE_CANNY_OUT_FMT_MAG_AND_ANG;


	if(HI_SUCCESS!=HI_MPI_IVE_CANNY(&IveHandle, &stSrc, &stDstMag, &stDstAng, &stCannyCtrl,HI_TRUE))
	{
        HOST_PRINT("[%s - %s - %d] HI_MPI_IVE_CANNY error %x %x %d!\n", 
            __FILE__, __FUNCTION__, __LINE__, UMag, UAngle, strideDst);
		return WY_INNER_ERR;
	}

	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE);
	if (HI_SUCCESS != s32Ret)
	{
        HOST_PRINT("[%s - %s - %d] HI_MPI_IVE_Query error!\n", 
            __FILE__, __FUNCTION__, __LINE__);
		return WY_INNER_ERR;
	}
	
	return WY_SUCCESS;
}
#else
int calAngleAndMagMatrix_wyVA(unsigned int USrc, unsigned int UAngle,unsigned int UMag, int widthSrc, 
                                                                      int height, int strideSrc, int strideDst)
{
   unsigned char *aP0,*aP1,*aP2;  
   unsigned char *bP0,*bP2;
   unsigned char *cP0,*cP1,*cP2,*pDstAngle;
   unsigned short *pDstMag; 
   int x, y, strideAdd;
   int gradX, gradY;
   double angle, angle180;
   unsigned char *pSrc, *pAngle;
   unsigned short *pMag;


   pSrc = (unsigned char *)USrc;
   pAngle = (unsigned char *)UAngle;
   pMag = (unsigned short *)UMag;
          
   aP0 = pSrc;
   aP1 = pSrc + 1;
   aP2 = pSrc + 2;
   
   bP0 = pSrc +strideSrc;
   bP2 = pSrc +strideSrc + 2;
   
   cP0 = pSrc + (strideSrc<<1);
   cP1 = pSrc + (strideSrc<<1) + 1;
   cP2 = pSrc + (strideSrc<<1) + 2;

   strideAdd = strideSrc - widthSrc + 2;
      
   for(y=0;y<(height-2);y++)
   {
        pDstAngle = pAngle + y*strideDst; 	
        pDstMag = pMag + y*strideDst;
        for(x=0;x<(widthSrc-2);x++)
        {
            
            gradX = (*aP2 + (*bP2<<1) + *cP2) - (*aP0 + (*bP0<<1) + *cP0); 
            gradY = (*cP0 + (*cP1<<1) + *cP2) - (*aP0 + (*aP1<<1) + *aP2);
            *pDstMag = (unsigned short)(WY_ABS(gradX) + WY_ABS(gradY));
			
            //*pDstMag = (unsigned short)(*(bP0+1));
           /* 
			if(gradX==0) 
			{ 
				angle=WYVA_ATAN2(gradY,gradX+0.1)/WYVA_PI*12; 
			} 
			else 
			{ 
				angle=WYVA_ATAN2(gradY,gradX)/WYVA_PI*12; 
			} 
			
			if(angle>0) 
			{ 
				angle=24-angle; 
			} 
			else 
			{ 
				angle=-angle; 
			}

                *pDstAngle =  (unsigned char)(angle);
              */
			
			angle = WYVA_ATAN2(gradY, gradX)/WYVA_PI*180+180;
			angle180 = angle>=180 ? (angle-180):angle; 
			
            *pDstAngle =  (unsigned char)(angle180);      

            aP0 += 1;                                            
            aP1 += 1;                               
            aP2 += 1;                           
                
            bP0 += 1;                      
            bP2 += 1;                      
            
            cP0 += 1;                   
            cP1 += 1;                     
            cP2 += 1;                   
            
            pDstAngle += 1;
            pDstMag += 1;
        }
               
       aP0 += strideAdd;
       aP1 += strideAdd;
       aP2 += strideAdd;
       
       bP0 += strideAdd;
       bP2 += strideAdd;
       
       cP0 += strideAdd;
       cP1 += strideAdd;
       cP2 += strideAdd;
   }
     
   return WY_SUCCESS;
}

#endif

int desMatchProcess_wyVA(int maxW, int maxH, unsigned char *maxBin, int desW, int desH, 
                                        unsigned char *desBin, int numMatched, int *retMatchNum)
{
    int max=0, x, y, sum, i, j, fPos=-1;
    unsigned char *startP, *maxBinLine, *desBinLine;
	int h1, h2;

	h1 = (maxH-desH)/3;
	h2 = 2*(maxH-desH)/3;
 
    startP = maxBin;
    for(y=0;y<=(maxH - desH);y++)
    { 
		startP=maxBin + y*maxW;
        for(x=0;x<=(maxW-desW);x++)
        {
            sum = 0;			
            //startP=maxBin + y*maxW + x;
            maxBinLine = startP;
            desBinLine = desBin;
            for(i=0;i<desH;i++)
            {
                
                for(j=0;j<desW;j++)
                {
                    if(maxBinLine[j]&desBinLine[j])
                      sum++;
                }
                maxBinLine += maxW;
                desBinLine += desW;
                                 
            }
            startP++;

            if(sum >= numMatched  && sum>max)
            {
                max = sum;
                fPos = (y<<16)|x;
            }
        }
    }
    *retMatchNum = max;
   // printf("max %d", max);
    return fPos;
}                                                     
                                                                     
                                                                               
                                                                                         




    




































