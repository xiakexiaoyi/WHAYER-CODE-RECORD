#include <stdio.h>
#include <stdlib.h>
#include "wyVAFunc.h"
#include "wyVACommon.h"
#include "wyMatchRigidBody.h"
#include "wyMatchRigidBodyStruct.h"
#include <math.h>
#include <string.h>
#include "sdkMacro.h"


int DspSdk_dmaCopy2d2d(unsigned char *pSrc, unsigned char *pDst, 
                       unsigned int w, unsigned int h, unsigned int srcPitch, unsigned int dstPitch)
{
    unsigned int y;
    unsigned char *pSrcTmp, *pDstTmp;

    pSrcTmp = pSrc;
    pDstTmp = pDst;

    for(y=0;y<h;y++)
    {

        memcpy(pDstTmp, pSrcTmp, w);
        pSrcTmp+=srcPitch;
        pDstTmp+=dstPitch;

    }
    return 0;
}
/********************************************************
/   The start point of objPic should be the up-left point of objMaskPic.
// So, the smoothed pic of objPic would be the same size as objMaskPic.
// The process of smoothing filter need border extension processing.
// This way avoid doing this.
/   ************
/   ************
/   **              **
/   **              **
/   **              **
/   ************
/   ************
/
*********************************************************/
void tiDspCutObjRecFrmPic_wy(wy_maskByteInfo *maskByteInfo,  wy_picInfo *trainPic, wy_picInfo *objMaskPic, 
                                                                 wy_picInfo *objPic, int top, int left)
{
   //we use edma to do this:subFrame   
   //int DspSdk_dmaCopy2d2d(unsigned char *pSrc, unsigned char *pDst, 
   //    unsigned int w, unsigned int h, unsigned int srcPitch, unsigned int dstPitch)
   unsigned char *pSrcObj, *pSrcMaskObj;

#if(DEBUG_WY_DSP)
   if((maskByteInfo->width < objMaskPic->width)||(maskByteInfo->height < objMaskPic->height))
   {
        
        HOST_PRINT("[%s - %s - %d] size not match!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
   }

   if((trainPic->width < objPic->width)||(trainPic->height < objPic->height))
   { 
        
        HOST_PRINT("[%s - %s - %d] size not match!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
   }
#endif
#if(0)
   pSrcObj = trainPic->byteBufAddr + top*trainPic->stride + left;  

   pSrcMaskObj = maskByteInfo->byteBufAddr + (top-maskByteInfo->startY)*maskByteInfo->width + (left - maskByteInfo->startX);

   DspSdk_dmaCopy2d2d(pSrcObj, objPic->byteBufAddr, objPic->width, 
       objPic->height, trainPic->stride, objPic->stride); 

   DspSdk_dmaCopy2d2d(pSrcMaskObj, objMaskPic->byteBufAddr, objMaskPic->width,
       objMaskPic->height, maskByteInfo->width, objMaskPic->stride);
#else
   pSrcObj = trainPic->byteBufAddr + (top-1)*trainPic->stride + left - 1;  

   pSrcMaskObj = maskByteInfo->byteBufAddr + (top-maskByteInfo->startY)*maskByteInfo->width + (left - maskByteInfo->startX);

   DspSdk_dmaCopy2d2d(pSrcObj, objPic->byteBufAddr, objPic->width, 
       objPic->height, trainPic->stride, objPic->stride); 

   DspSdk_dmaCopy2d2d(pSrcMaskObj, objMaskPic->byteBufAddr, objMaskPic->width,
       objMaskPic->height, maskByteInfo->width, objMaskPic->stride);
#endif
  // return WY_SUCCESS;
}

int matchRigBodyTrainStep0(wy_picInfo *pic, 
                                 wy_trainPMemUg *ddrMem, int *angle)
{
    wy_maskByteInfo *maskByteInfo;
    wyVA_picBufA picBufA;
    wy_picInfo *objPic;
    int *lx=NULL, *ly=NULL;
    int  tmp0, tmp1,number = 0;
    int top, left,	right, bot, ret = WY_FAILURE;
    wy_picInfo *objMaskPic, *sfObjPic;
    wyVA_picBufA maskData;
    WYVA_FLOAT sumX=0, sumY=0, meanX, meanY, result;
    WYVA_FLOAT mat[3]={0,0,0};

#if(WY_DEBUG)
    FILE *fileOutFp=NULL;         
#endif

    objMaskPic = &ddrMem->objMaskPic;
    sfObjPic = &ddrMem->sfObjPic;
    lx = ddrMem->lx;
    ly = ddrMem->ly;
    objPic = &ddrMem->objPic;
    
    maskByteInfo = &ddrMem->maskByteInfo;		
  //  maskByteInfo->width = maskBitInfo->width;
  //  maskByteInfo->height = maskBitInfo->height;              
 //   maskByteInfo->startX = maskBitInfo->startX;
  //  maskByteInfo->startY = maskBitInfo->startY;

  // bitMapToByteMap_wyVA(maskBitInfo->bitBufAddr, maskByteInfo->byteBufAddr, maskBitInfo->width, maskBitInfo->height);

  // bitMapToByteMap_wyVA(bitPingPongBuf[0], bytePingPongBuf[0], width, height>>4);
                        
   // tiDspConvertBitMapToByteMap_wy(maskBitInfo, maskByteInfo, l2Sram);

#if(WY_DEBUG)
    fileOutFp = fopen("mask.yuv", "wb");

    if(!fileOutFp)
    {
        printf("file open error!");
    }
    
    if(fileOutFp)
    {
        fwrite(maskByteInfo->byteBufAddr, sizeof(unsigned char), maskByteInfo->width*maskByteInfo->height, fileOutFp);
        fclose(fileOutFp);
    }
#endif

    maskData.pData = maskByteInfo->byteBufAddr;	//divide with 8
    maskData.startY=maskData.startX = 0;
    left  = maskData.width =  maskByteInfo->width;
    top =maskData.height = maskByteInfo->height;	
    maskData.stride = maskByteInfo->width;

    right = bot = 0;
    findRecPosOfObjInMaskPic_wyVA(&maskData, &left, &right, &top, &bot);
    top += maskByteInfo->startY;
    left += maskByteInfo->startX;
    bot += maskByteInfo->startY;
    right += maskByteInfo->startX;
  //  tiDspFindRecPosOfObjInMaskPic_wy(maskByteInfo, &top, &left, &right, &bot, l2Sram);
	HOST_PRINT("top=%d, left=%d, bot=%d, right=%d\n", top, left, bot, right);
    //------------------------------------------------------------------------------------
    
    if( !(top&&left&&(right!=(pic->width-1))&&(bot!=(pic->height-1))) )
    {
        HOST_PRINT("[%s - %s - %d] !(top&&left&&(right!=(pic->width-1))&&(bot!=(pic->height-1))) not meet!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        goto EXIT0;  
    }
        
    tmp0 = bot - top + 1;
    tmp1 = right - left + 1;

    if(tmp0>WYVA_OBJSIZE_MAX||tmp1>WYVA_OBJSIZE_MAX)
    {
        HOST_PRINT("[%s - %s - %d] object width bg 448 or object height bg 448!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        goto EXIT0; 
    }

    if(tmp0<=WYVA_OBJSIZE_MIN||tmp1<=WYVA_OBJSIZE_MIN)
    {
        HOST_PRINT("[%s - %s - %d] object width lt 0 or object height lt 0!\n", 
            __FILE__, __FUNCTION__, __LINE__);//integer
        goto EXIT0; 
    }

    objMaskPic->width = tmp1;
    objMaskPic->height = tmp0;

	objPic->width = tmp1+2;
	objPic->height = tmp0+2;

    tiDspCutObjRecFrmPic_wy(maskByteInfo, pic, objMaskPic, objPic, top, left);
#if(WY_DEBUG)
    fileOutFp = fopen("/tmp/objMask.yuv", "wb");
    if(!fileOutFp)
    {
        printf("file open error!");
    }
    if(fileOutFp)
    {
        fwrite(objMaskPic->byteBufAddr, sizeof(unsigned char), objMaskPic->height*objMaskPic->stride, fileOutFp);
        fclose(fileOutFp);
    }

    fileOutFp = fopen("/tmp/objPic.yuv", "wb");
    if(!fileOutFp)
    {
        printf("file open error!");
    }
    if(fileOutFp)
    {
        fwrite(objPic->byteBufAddr, sizeof(unsigned char), objPic->height*objPic->stride, fileOutFp);
        fclose(fileOutFp);
    }
#endif

    sfObjPic->width = tmp1;
    sfObjPic->height = tmp0;

    smoothFilter_wyVA(objPic->byteBufAddr, sfObjPic->byteBufAddr, 
			objPic->width, objPic->height, objPic->stride, sfObjPic->stride); //00000000000000000

#if(WY_DEBUG)
    fileOutFp = fopen("sfObjPic.yuv", "wb");
    if(!fileOutFp)
    {
        printf("file open error!");
    }
    
    if(fileOutFp)
    {
        fwrite(sfObjPic->byteBufAddr, sizeof(unsigned char), sfObjPic->height*sfObjPic->stride, fileOutFp);
        fclose(fileOutFp);
    }
#endif
    //----------malloc lx ly-------------
    picBufA.pData = objMaskPic->byteBufAddr;  //picA[0].pData + size;
    picBufA.startX = 0;
    picBufA.startY = 0;
    picBufA.stride = objMaskPic->stride;
    picBufA.width = objMaskPic->width;
    picBufA.height = objMaskPic->height; //8x:interger times of 8
    
    picMainDirectionStep1_wyVA(&picBufA, lx, ly, &number, &sumX, &sumY);
    meanX = sumX/number;
    meanY = sumY/number;
    picMainDirectionStep2_wyVA(lx, ly, mat, meanX, meanY, number); 
    //------------------------------------------------------------------------------------
   mat[0] /= number;
   mat[1] /= number;
   mat[2] /= number; 
    //-------------------------------------------------------------------------------------
    if(mat[1])
   {
       result = (mat[2] - mat[0] + WYVA_SQRT((mat[0]-mat[2])*(mat[0]-mat[2])+4*mat[1]*mat[1]))/2/mat[1]; 
       *angle= (int)(WYVA_ATAN(result)/WYVA_PI*180);
   }
   else if(0==mat[1] && (mat[2] - mat[0] + WYVA_SQRT((mat[0]-mat[2])*(mat[0]-mat[2])+4*mat[1]*mat[1]))<0.1)
   {
        *angle = 0;
   }
   else 
        *angle = 90;
    //-------------------------------
   // tiDspPicMainDirection_wy(objMaskPic, angle, lx, ly, l2Sram);
    ret = WY_SUCCESS;	
    
EXIT0:	
    return ret;
}

void hi3516RotateX_wy(wy_picInfo *outPic, wy_picInfo *inPic, int iAngle)
{
   WYVA_FLOAT sinA, cosA;
   WYVA_FLOAT angle;
   int xABC[3], yABC[3];
   int w_dst, h_dst, h_src, w_src;
   wyVA_picBuf picIn;  
   wyVA_picBufA picOut;
   
   angle = iAngle*WYVA_PI/180.0; // angle in radian
   w_src = inPic->width;
   h_src = inPic->height;
   sinA = WYVA_SIN(angle);
   cosA = WYVA_COS(angle);
   w_dst = (int)(h_src * WY_ABS(sinA) + w_src * WY_ABS(cosA)); 
   h_dst = (int)(w_src * WY_ABS(sinA) + h_src * WY_ABS(cosA));

#if(DEBUG_WY_DSP)         
   if(w_dst > outPic->stride || h_dst > outPic->stride)
   {
       HOST_PRINT("[%s - %s - %d] w_dst > outPic->stride || h_dst > outPic->stride!\n", 
                             __FILE__, __FUNCTION__, __LINE__);//integer
      // return WY_IN_PARA_ERR;
   }

   if(h_dst <16)
   {
       HOST_PRINT("[%s - %s - %d] w_dst > outPic->stride || h_dst > outPic->stride!\n", 
                             __FILE__, __FUNCTION__, __LINE__);//integer
     //  return WY_IN_PARA_ERR;
   }

    if(inPic->width < 16 && inPic->height <16)
    {
        HOST_PRINT("[%s - %s - %d] inPic->width < 16 && inPic->height <16!\n", 
                              __FILE__, __FUNCTION__, __LINE__);//integer
    //	return WY_IN_PARA_ERR;
    }

    if(inPic->width > 448 && inPic->height >448)
    {
        HOST_PRINT("[%s - %s - %d] inPic->width < 16 && inPic->height <16!\n", 
                              __FILE__, __FUNCTION__, __LINE__);//integer
    //	return WY_IN_PARA_ERR;
    }
 #endif
 
   xABC[0] = (int)(cosA*FLOAT_SCALOR_NUM);
   xABC[1] = (int)(-sinA*FLOAT_SCALOR_NUM);
   xABC[2] = (int)((w_src/2-cosA*w_dst/2+sinA*h_dst/2)*FLOAT_SCALOR_NUM);

   yABC[0] = (int)(sinA*FLOAT_SCALOR_NUM);
   yABC[1] = (int)(cosA*FLOAT_SCALOR_NUM);
   yABC[2] = (int)((h_src/2-sinA*w_dst/2-cosA*h_dst/2)*FLOAT_SCALOR_NUM);

   picIn.pData = inPic->byteBufAddr;
   picIn.width = inPic->width;
   picIn.height = inPic->height;
   picIn.stride = inPic->stride;
                                                                                           
   picOut.pData = outPic->byteBufAddr;
   picOut.startX = picOut.startY = 0;
   picOut.stride = outPic->stride;
   outPic->width = picOut.width = w_dst;
   picOut.height = outPic->height = h_dst;
   
   rotateX_wyVA(&picIn, &picOut, xABC, yABC);
     
}

//scalor times: 0.3~2.5
void hi3516BiLinearScalor(wy_picInfo *outPic, wy_picInfo *inPic)
{
    //int scalorBLinear_wyVA(wyVA_picBufA *picIn, wyVA_picBufA *picOut, WYVA_FLOAT nXFactor, WYVA_FLOAT nYFactor)
    WYVA_FLOAT nYFactorF;
    int nXFactor, nYFactor;
    int dstW, dstH, srcW, srcH;
    wyVA_picBufA picIn, picOut;


    dstW = outPic->width;
    dstH = outPic->height;
    srcW = inPic->width;
    srcH = inPic->height;
    
    nXFactor = (int)((srcW*1.0/dstW)*FLOAT_SCALOR_NUM); 
    nYFactorF = srcH*1.0/dstH;
   
   // hDiv16 = dstH>>4;
   // hMod16 = dstH&0xF;	
  //  srcScalorH = (int)(nYFactorF*16+ADDH);
    nYFactor = (int)(nYFactorF*FLOAT_SCALOR_NUM);
#if(DEBUG_WY_DSP)  
    if(inPic->width < 16 && inPic->height <16)
    {
        HOST_PRINT("[%s - %s - %d] inPic->width < 16 && inPic->height <16!\n", 
            __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
    }

    if(outPic->width>720 || outPic->height>576)
    {
        HOST_PRINT("[%s - %s - %d] outPic->width>640 || outPic->height>640!\n", 
            __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
    }

    if(srcScalorH>48)
    {
        HOST_PRINT("[%s - %s - %d] nYFactor bg 2!\n", 
            __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
    }	
#endif

    picIn.height = srcH;
    picIn.stride = inPic->stride;
    picIn.width = srcW;
    picIn.startX = picIn.startY = 0;
    
    picOut.height = dstH;
    picOut.width = dstW;
    picOut.stride = outPic->stride;
    picOut.startX = picOut.startY = 0;

    picIn.pData = inPic->byteBufAddr;
    picOut.pData = outPic->byteBufAddr;
    scalorBLinear_wyVA(&picIn, &picOut, nXFactor, nYFactor);   	
}

//pDesData is internal sram
//mask pic after scalor is also internal sram
//all of above condition is based set desData is not big than 128x128
void hi3516RefineDes(int *obPixelNum, wy_picInfo *maskPic, unsigned char *pDesData,int desW, int desH, unsigned char *pMaskDataL2)
{
  //  unsigned char *pMaskDataL2, *srcPingPong[2], *pDDRSrc[2];
    WYVA_FLOAT nYFactorF;
    int nXFactor, nYFactor;
    int srcW, srcH;
    wyVA_picBufA picIn, picOut;

#if(DEBUG_WY_DSP)  	
    if(desW > 128 || desH > 128)
    {
        HOST_PRINT("[%s - %s - %d] desW > 128 || desH > 128!\n", 
                                 __FILE__, __FUNCTION__, __LINE__);//integer
        //return WY_IN_PARA_ERR;
    }
#endif

    //srcStride = maskPic->stride;
    srcW = maskPic->width;
    srcH = maskPic->height;    
    nXFactor = (int)((srcW*1.0/desW)*FLOAT_SCALOR_NUM); 
    nYFactorF = srcH*1.0/desH;   
    nYFactor = (int)(nYFactorF*FLOAT_SCALOR_NUM);
        
    picIn.height = srcH;
    picIn.stride = maskPic->stride;
    picIn.width = srcW;
    picIn.startX = picIn.startY = 0;

    picOut.height = desH;
    picOut.width = desW;
    picOut.stride = WY_OBJDESSIZE_MAX;
    picOut.startX = picOut.startY = 0;
    
    picIn.pData = maskPic->byteBufAddr;
    picOut.pData = pMaskDataL2;
    
    scalorDesBLinear_wyVA(&picIn, &picOut, nXFactor, nYFactor);
   *obPixelNum = refineDes_wyVA(pDesData, desW, desH, pMaskDataL2, WY_OBJDESSIZE_MAX);		
}

//-----------------------------------------------------------------------------------

int matchRigBodyTrainGetDes_wy(int angle, wy_trainPMemUg *ddrMem, 
                  wy_objCharacterDes *objDesArray, int rotateAngle, WYVA_FLOAT magRate)
{
    wy_picInfo *rotateObjPic, *rotateMaskPic, *salorSfObjPic;
    unsigned short *pMag;
    unsigned char *pAngle;
    int ret = WY_SUCCESS, tmpAngle;
    unsigned char *binVal;
    wy_picInfo *objMaskPic, *sfObjPic;

#if(WY_DEBUG)
  //  char name[10];
    static char num[3]={'A','0',0};
    FILE *fileOutFp=NULL; 
    int i,j;
    unsigned char magVal;
#endif

    objMaskPic = &ddrMem->objMaskPic;
    sfObjPic = &ddrMem->sfObjPic;
    rotateObjPic = &ddrMem->rotateObjPic;
    rotateMaskPic = &ddrMem->rotateMaskPic;
    salorSfObjPic = &ddrMem->salorSfObjPic;
    pAngle = ddrMem->pAngle;
    pMag = 	ddrMem->pMag;

    switch(rotateAngle)
    {
        case 0:  //rotate 0 == NO rotate
            rotateObjPic =sfObjPic;
            rotateMaskPic = objMaskPic;
#if(WY_DEBUG)
            fileOutFp = fopen("rotate0.yuv", "wb");

            if(!fileOutFp)
            {
                printf("file open error!");
            }
            printf("stride %d height %d\n",  rotateObjPic->stride, rotateObjPic->height);

            if(fileOutFp)
            {
                fwrite(rotateObjPic->byteBufAddr, sizeof(unsigned char), rotateObjPic->stride*rotateObjPic->height, fileOutFp);
                fclose(fileOutFp);
            }
#endif
            break;
        case 270: //rotate 90

            rotateObjPic->height = sfObjPic->width;
            rotateObjPic->width = sfObjPic->height;
            rotate90_wyVA(sfObjPic->stride, sfObjPic->width, sfObjPic->height, 
                rotateObjPic->stride, sfObjPic->byteBufAddr, rotateObjPic->byteBufAddr);
            //tiDspRotate90_wy(rotateObjPic, sfObjPic, l2Sram);
            rotateMaskPic->height = objMaskPic->width;
            rotateMaskPic->width = objMaskPic->height;
            rotate90_wyVA(objMaskPic->stride, objMaskPic->width, objMaskPic->height, 
                rotateMaskPic->stride, objMaskPic->byteBufAddr, rotateMaskPic->byteBufAddr);
            //tiDspRotate90_wy(rotateMaskPic, objMaskPic, l2Sram);

#if(WY_DEBUG)
            fileOutFp = fopen("rotate90.yuv", "wb");

            if(!fileOutFp)
            {
                printf("file open error!");
            }
            printf("stride %d height %d\n", rotateObjPic->stride, rotateObjPic->height);

            if(fileOutFp)
            {
                fwrite(rotateObjPic->byteBufAddr, sizeof(unsigned char), rotateObjPic->stride*rotateObjPic->height, fileOutFp);
                fclose(fileOutFp);
            }
#endif

            break;
        case 180: //rotate 180

                rotateObjPic->height = sfObjPic->height;
                rotateObjPic->width = sfObjPic->width; 

                rotate180_wyVA(sfObjPic->stride, sfObjPic->width, sfObjPic->height, 
                     rotateObjPic->stride,sfObjPic->byteBufAddr, rotateObjPic->byteBufAddr);

                rotateMaskPic->height = objMaskPic->height;
                rotateMaskPic->width = objMaskPic->width;

                rotate180_wyVA(objMaskPic->stride, objMaskPic->width, objMaskPic->height, 
                     rotateMaskPic->stride,objMaskPic->byteBufAddr, rotateMaskPic->byteBufAddr);

            //rotate180_wyVA(srcWidth, srcWidth, 16, srcWidth, srcPingPong[0], dstPingPong[0]);
            //tiDspRotate180_wy(rotateObjPic, sfObjPic, l2Sram);		
            //tiDspRotate180_wy(rotateMaskPic, objMaskPic, l2Sram);
#if(WY_DEBUG)
            fileOutFp = fopen("rotate180.yuv", "wb");

            if(!fileOutFp)
            {
                printf("file open error!");
            }
            printf("stride %d height %d\n", rotateObjPic->stride, rotateObjPic->height);

            if(fileOutFp)
            {
                fwrite(rotateObjPic->byteBufAddr, sizeof(unsigned char), rotateObjPic->stride*rotateObjPic->height, fileOutFp);
                fclose(fileOutFp);
            }
#endif
            break;
        case 90: //rotate 270	

                rotateObjPic->height = sfObjPic->width;
                rotateObjPic->width = sfObjPic->height; 

                rotate270_wyVA(sfObjPic->stride, sfObjPic->width, sfObjPic->height, 
                     rotateObjPic->stride,sfObjPic->byteBufAddr, rotateObjPic->byteBufAddr);

                rotateMaskPic->height = objMaskPic->width;
                rotateMaskPic->width = objMaskPic->height;

                rotate270_wyVA(objMaskPic->stride, objMaskPic->width, objMaskPic->height, 
                     rotateMaskPic->stride,objMaskPic->byteBufAddr, rotateMaskPic->byteBufAddr);
        //    tiDspRotate270_wy(rotateObjPic, sfObjPic, l2Sram);		
        //    tiDspRotate270_wy(rotateMaskPic, objMaskPic, l2Sram);
#if(WY_DEBUG)            
            fileOutFp = fopen("rotate270.yuv", "wb");

            if(!fileOutFp)
            {
                printf("file open error!");
            }
            printf("stride %d height %d\n", rotateObjPic->stride, rotateObjPic->height);

            if(fileOutFp)
            {
                fwrite(rotateObjPic->byteBufAddr, sizeof(unsigned char), rotateObjPic->stride*rotateObjPic->height, fileOutFp);
                fclose(fileOutFp);
            }
#endif
            break;
        default:	
           // memset(rotateObjPic->byteBufAddr, 0, rotateObjPic->stride*rotateObjPic->stride);
            hi3516RotateX_wy(rotateObjPic, sfObjPic, rotateAngle);	
            hi3516RotateX_wy(rotateMaskPic, objMaskPic, rotateAngle);

#if(WY_DEBUG)
            itoa(rotateAngle, num, 10);

            fileOutFp = fopen(num, "wb");
            if(!fileOutFp)
            {
                printf("file open error!");
            }

            if(fileOutFp)
            {
                printf("%s stride %d height %d\n", num, rotateObjPic->stride, rotateObjPic->height);
                fwrite(rotateObjPic->byteBufAddr, sizeof(unsigned char), rotateObjPic->stride*rotateObjPic->height, fileOutFp);
                fclose(fileOutFp);
            }
#endif
            break;
    }

/*******************************************
ouput file  720x576   rotate.y  sequence 


************************************************/
    tmpAngle = angle - rotateAngle;
    if(tmpAngle<-270)
        tmpAngle += 360;
    salorSfObjPic->width = (rotateObjPic->width/5+1)*5+2;	
    salorSfObjPic->height= (rotateObjPic->height/5+1)*5+2;
    objDesArray->obRecWidthS = (salorSfObjPic->width-2)/5 - 1;
    objDesArray->obRecHeightS = (salorSfObjPic->height-2)/5 - 1;
 //   objDesArray->angle  = tmpAngle;
    objDesArray->angle = tmpAngle < -90 ? (180+tmpAngle):tmpAngle;
   // objDesArray->matchNum = 0;
    binVal = objDesArray->pData;

    hi3516BiLinearScalor(salorSfObjPic, rotateObjPic);	

#if(WY_DEBUG)
    itoa(rotateAngle, num, 10);

    fileOutFp = fopen(num, "wb");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
       // printf("scalor %s stride %d width %d height %d\n", num, salorSfObjPic->stride, salorSfObjPic->width, salorSfObjPic->height);
        fwrite(salorSfObjPic->byteBufAddr, sizeof(unsigned char), salorSfObjPic->stride*salorSfObjPic->height, fileOutFp);
        fclose(fileOutFp);
    }
#endif

#if(HI3516_WY)
    //flush cache to ddr
	HI_MPI_SYS_MmzFlushCache(salorSfObjPic->u32PhyAddr,salorSfObjPic->byteBufAddr,
	                                         salorSfObjPic->stride*salorSfObjPic->height);
	if(WY_SUCCESS!=calAngleAndMagMatrix_wyVA(salorSfObjPic->u32PhyAddr, ddrMem->u32PhyAddrAge, ddrMem->u32PhyAddrMag,
		salorSfObjPic->width, salorSfObjPic->height, salorSfObjPic->stride, ddrMem->magAngStride))
			return WY_FAILURE;
#else
	calAngleAndMagMatrix_wyVA((unsigned int)(salorSfObjPic->byteBufAddr), (unsigned int)(pAngle), (unsigned int)(pMag),
		salorSfObjPic->width, salorSfObjPic->height, salorSfObjPic->stride, ddrMem->magAngStride);
#endif



   // tiDspAngleMagObjPic_wy(salorSfObjPic, pAngle, pMag, ddrMem->magAngStride, l2Sram);

#if(WY_DEBUG)
   // itoa(rotateAngle, num, 10);
    num[0]++;
    fileOutFp = fopen(num, "wb");
    if(fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(i=0;i< (salorSfObjPic->height-2);i++)
        {
            for(j=0;j<(salorSfObjPic->width-2);j++)
            {
                magVal = (unsigned char)(*(pAngle+ (i)*ddrMem->magAngStride + j));
                fwrite(&magVal, sizeof(unsigned char),1, fileOutFp);

            }
        }

        printf("scalor %s width %d height %d\n", num, salorSfObjPic->width-2, salorSfObjPic->height-2);
        //fwrite(pMag, sizeof(unsigned char),ddrMem->magAngStride*(salorSfObjPic->height-2), fileOutFp);
        fclose(fileOutFp);
    }
#endif

#if(HI3516_WY)
    getDesBinMatrix(binVal, pAngle+ddrMem->magAngStride+1, pMag+ddrMem->magAngStride+1, ddrMem->magAngStride, 
                      salorSfObjPic->width-2, salorSfObjPic->height-2, magRate);
#else
    getDesBinMatrix(binVal, pAngle, pMag, ddrMem->magAngStride, 
					 salorSfObjPic->width-2, salorSfObjPic->height-2, magRate);
#endif
    //output 

#if(WY_DEBUG)
    itoa(rotateAngle, num, 10);

    fileOutFp = fopen(num, "wb");
    if(fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {

               // magVal = (unsigned char)(*(binVal + i*objDesArray->obRecWidthS + j));
        fwrite(binVal, sizeof(unsigned char),objDesArray->obRecWidthS*objDesArray->obRecHeightS, fileOutFp);
        printf("binVal %s width %d height %d\n", num, objDesArray->obRecWidthS, objDesArray->obRecHeightS);
        //fwrite(pMag, sizeof(unsigned char),ddrMem->magAngStride*(salorSfObjPic->height-2), fileOutFp);
        fclose(fileOutFp);
    }
#endif
//objDesArray->obPixelNum = objDesArray->obRecWidthS*objDesArray->obRecHeightS;

   hi3516RefineDes(&objDesArray->obPixelNum, rotateMaskPic, binVal,
                   objDesArray->obRecWidthS, objDesArray->obRecHeightS, ddrMem->pMask);	
//EXIT0:
#if(WY_DEBUG)
    num[0]++;

    itoa(rotateAngle, num, 10);

    fileOutFp = fopen(num, "wb");
    if(fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        // magVal = (unsigned char)(*(binVal + i*objDesArray->obRecWidthS + j));
        fwrite(binVal, sizeof(unsigned char),objDesArray->obRecWidthS*objDesArray->obRecHeightS, fileOutFp);
        printf("binVal %s width %d height %d\n", num, objDesArray->obRecWidthS, objDesArray->obRecHeightS);
        //fwrite(pMag, sizeof(unsigned char),ddrMem->magAngStride*(salorSfObjPic->height-2), fileOutFp);
        fclose(fileOutFp);
    }
#endif
    return ret;  	   
}










































