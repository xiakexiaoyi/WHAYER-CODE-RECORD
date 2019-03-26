#include "HaarWidth.h"
#include "limem.h"
#include <math.h>
#include <stdio.h>
#include "lidebug.h"

#define PI 3.141592653589793238
#define InfSlope 100000.0

MRESULT MergeSegs( MHandle hMemMgr, MInt32 *pNumTuple, MDouble *pSegs, MDouble *pOutSegs );
MRESULT GetAvgPointer(PBLOCK pImage,MPOINT SegMostStart, MPOINT SegMostEnd, MLong *pMean );
MVoid BinarySelectedRegion ( PBLOCK pImage, MLong lxEnd, MPOINT SegMostStart, MLong lMean, BLOCK *pBinaryImage);
MVoid FilterBoundaryPoints(PBLOCK pImage, PBLOCK pBlockImage, MLong lxEnd, BLOCK *pFilterImage);
MVoid CalRegRatWidHgt(PBLOCK pImage, pFilterParam pParam, PBLOCK pBlockImage, MUInt32 lNumRegion);
MVoid FilterOtherRegs( PBLOCK pBlockImage, pFilterParam pParam, MUInt32 lNumRegion, BLOCK *pFilterImage );
MVoid FilterLastRegs(PBLOCK pBlockImage, pFilterParam pParam, MUInt32 lNumRegion, BLOCK *pFilterImage);


 MRESULT GetHaarWdith(MHandle hMemMgr, BLOCK *pImage, MInt32 *pNumTuple, MDouble *pSegs, MLong *pHaarWidth)
 {
     MRESULT res=LI_ERR_NONE;
     BLOCK Image1 = {0}, Image2 = {0}, Image3 = {0}, Image4 = {0}, Image5 = {0};
     BLOCK BlockImage = {0};
     MDouble factor = 0.5;
     MInt32 NumTuple = *pNumTuple;
     MPOINT SegMostStart = {0}, SegMostEnd = {0};
     MPOINT SegStart = {0}, SegEnd = {0};
     MLong lWidth, lHeight, lBlockLine;
     MLong HaarWidth = 0;
     MLong i;
     pFilterParam pParam;
     MLong ldis = 0, lMag, lxDis, lxEnd;
     MLong  lThreshold = 128;
     MLong x = 0, y = 0;
     MLong lMean;
     MUInt32 NumRegion1, NumRegion2, NumRegion3 = 0;
     MUInt32 NumRegionLast = 0;
     MDouble *pOutSegs = MNull;

     if (pImage == MNull || pNumTuple == MNull || pSegs == MNull)
     {
         res = LI_ERR_INVALID_PARAM;
         return res;
     }

     AllocVectMem(hMemMgr, pOutSegs, NumTuple*7, MDouble);
     SetVectMem(pOutSegs, NumTuple*7, 0, MDouble);
    // adjusting the start points and end points to make the x of start point smaller than the end 
     GO(MergeSegs(hMemMgr, &NumTuple, pSegs, pOutSegs));
// seach the most distance segment from start point to end point.
    Image1 = *pImage;
  //  pData1= (MUInt8*)(Image1.pBlockData);
    lWidth = Image1.lWidth;
	lHeight = Image1.lHeight;
	lBlockLine = Image1.lBlockLine;
    for (i = 0; i < NumTuple; ++i)
	{
		SegStart.x = (MLong)(pOutSegs[i*7+0]);
		SegStart.y = (MLong)(pOutSegs[i*7+1]);
		SegEnd.x = (MLong)(pOutSegs[i*7+2]);
		SegEnd.y = (MLong)(pOutSegs[i*7+3]);
        lMag = abs( SegEnd.x - SegStart.x) + abs(SegEnd.y - SegStart.y);
        if ( lMag > ldis)
        {
            SegMostStart.x = SegStart.x;
            SegMostStart.y = SegStart.y;
            SegMostEnd.x = SegEnd.x;
            SegMostEnd.y = SegEnd.y;
            ldis = lMag;
        }
	}
   // vLineTo(Image1.pBlockData, lBlockLine, lWidth, lHeight, 255, SegMostStart, SegMostEnd);
	PrintBmpEx(Image1.pBlockData, Image1.lBlockLine,Image1.typeDataA & ~0x100,Image1.lWidth, Image1.lHeight, 1, "D:\\pointer.bmp");
     // determine the boundary of interested region  
    lxDis = (MLong)floor(abs(SegMostStart.x - SegMostEnd.x) * factor);
    lxEnd = SegMostStart.x + lxDis;
    if (SegMostEnd.x > lWidth)
        lxEnd = lWidth;

        // caculate the average gray pixel value of the pointer (Oil level indicator)
     GO(GetAvgPointer(&Image1, SegMostStart, SegMostEnd, &lMean));

    GO(B_Create(hMemMgr, &Image2, DATA_U8, lWidth, lHeight));
	//CpyVectMem(Image2.pBlockData,Image1.pBlockData,lHeight*lBlockLine,MUInt8);

    // divide the lHeight into 8 block to precisely get the interested region
     BinarySelectedRegion (&Image1, lxEnd, SegMostStart, lMean, &Image2);


     PrintBmpEx(Image2.pBlockData, Image2.lBlockLine, Image2.typeDataA & ~0x100, Image2.lWidth, Image2.lHeight, 1, "D:\\SelectedRegionBinary.bmp");

    // filter the binary image, firstly filtering the boundary points 
     GO(B_Create(hMemMgr, &BlockImage, DATA_U8, lWidth, lHeight));
     ConnectRegion_EX2(hMemMgr, &Image2, lThreshold, &BlockImage, &NumRegion1);
     printf( "Region_num1=%d\n", NumRegion1);

     GO(B_Create(hMemMgr,&Image3,DATA_U8,lWidth,lHeight));
	 CpyVectMem(Image3.pBlockData,Image2.pBlockData,lHeight*lBlockLine,MUInt8);

       FilterBoundaryPoints(&Image2, &BlockImage, lxEnd, &Image3);

       ConnectRegion_EX2(hMemMgr, &Image3, lThreshold, &BlockImage, &NumRegion2);
        printf("k=%d\n", NumRegion2);

       AllocVectMem(hMemMgr, pParam, NumRegion2, FilterParam);

      CalRegRatWidHgt(&Image3, pParam, &BlockImage, NumRegion2);

      GO(B_Create(hMemMgr,&Image4,DATA_U8,lWidth,lHeight));
	  CpyVectMem(Image4.pBlockData,Image3.pBlockData,lHeight*lBlockLine,MUInt8);

      FilterOtherRegs(&BlockImage, pParam, NumRegion2, &Image4);

      PrintBmpEx(Image4.pBlockData, Image4.lBlockLine,Image4.typeDataA & ~0x100,Image4.lWidth, Image4.lHeight, 1, "D:\\FilterBinary.bmp");

      GO(ConnectRegion_EX2(hMemMgr, &Image4, lThreshold, &BlockImage, &NumRegion3));

      GO(B_Create(hMemMgr,&Image5,DATA_U8,lWidth,lHeight));
	  CpyVectMem(Image5.pBlockData, Image4.pBlockData, lHeight*lBlockLine, MUInt8);

      FilterLastRegs(&BlockImage, pParam, NumRegion3, &Image5);
      for ( i = 0; i < (MLong)NumRegion3; ++i)
     { 
         if ( pParam[i].FlagNum != 0)
             NumRegionLast++;
         HaarWidth += pParam[i].FlagNum;
      }
      HaarWidth /= (NumRegionLast*4);
      HaarWidth = (MLong)(sqrt ((MDouble)HaarWidth)+0.5);
      printf("harr_w=%d\n", HaarWidth);

     PrintBmpEx(Image5.pBlockData, Image5.lBlockLine,Image5.typeDataA & ~0x100,Image5.lWidth, Image5.lHeight, 1, "D:\\LastBinary_scale.bmp");

     *pHaarWidth = HaarWidth;

  EXT:
    B_Release(hMemMgr,&Image2);
    B_Release(hMemMgr,&Image3);
    B_Release(hMemMgr,&Image4);
    B_Release(hMemMgr,&Image5);
    B_Release(hMemMgr,&BlockImage);
    FreeVectMem(hMemMgr, pParam);
    FreeVectMem(hMemMgr, pOutSegs);

  return res;
 }

          // adjusting the start points and end points to make the x of start point smaller than the end 
     // caculate the angle of each segment
 MRESULT MergeSegs( MHandle hMemMgr, MInt32 *pNumTuple, MDouble *pSegs, MDouble *pOutSegs )
 {
      MRESULT res=LI_ERR_NONE;
      MInt32 NumTuple = *pNumTuple;
      pSegParam pSegParams = MNull;
      MLong i, j;
       MDouble HeightDiff = 0, WidthDiff = 0;
       MDouble Sign;
       MDouble AngMerge, AngleDiff ;
       MDouble Swapx, Swapy;
       MPOINT start = {0}, end = {0};

      AllocVectMem(hMemMgr, pSegParams, NumTuple, SegParam);
      for(i = 0; i < NumTuple; ++i)
      {
          Swapx = pSegs[i*7+0];
          if (Swapx > pSegs[i*7+2])
          {
              Swapy = pSegs[i*7+1];
              pSegs[i*7+0] = pSegs[i*7+2];
              pSegs[i*7+2] = Swapx;
              pSegs[i*7+1] = pSegs[i*7+3];
              pSegs[i*7+3] = Swapy;
          }
      }
      for(i = 0; i < NumTuple; ++i)
     {
        if ( pSegs[i*7+0] - pSegs[i*7+2] != 0)
            pSegParams[i].kVal = (MDouble) (pSegs[i*7+3] - pSegs[i*7+1])/(pSegs[i*7+2] - pSegs[i*7+0]);
        else
            pSegParams[i].kVal = InfSlope;
        pSegParams[i].Angle = atan(pSegParams[i].kVal)*180/PI;
        pSegParams[i].Mag = fabs(pSegs[i*7+3] - pSegs[i*7+1]) + fabs(pSegs[i*7+2] - pSegs[i*7+0]);
        printf("k=%f, angle=%f\n", pSegParams[i].kVal, pSegParams[i].Angle);
        printf("x=%f, y=%f, x=%f, y=%f\n", pSegs[i*7+0], pSegs[i*7+1], pSegs[i*7+2], pSegs[i*7+3]);
    }
     for(i = 0; i < NumTuple; ++i)
     {
        pOutSegs[i*7+0] = pSegs[i*7+0];
        pOutSegs[i*7+1] = pSegs[i*7+1];
        pOutSegs[i*7+2] = pSegs[i*7+2];
        pOutSegs[i*7+3] = pSegs[i*7+3];
        pOutSegs[i*7+4] = pSegs[i*7+4];
        pOutSegs[i*7+5] = pSegs[i*7+5];
        pOutSegs[i*7+6] = pSegs[i*7+6];
     }
// search segments almost in a line and merge them into a tuple.
     for(i = 0; i < NumTuple; ++i)
     {
          for(j = i+1; j < NumTuple; ++j)
          {
              if ( pOutSegs[i*7+0] > pOutSegs[j*7+2])
              {
                  WidthDiff = pOutSegs[i*7+0] - pOutSegs[j*7+2];
                  HeightDiff = pOutSegs[i*7+1] - pOutSegs[j*7+3];
              }
              else if (pOutSegs[i*7+2] <= pOutSegs[j*7+0] )
              {
                   WidthDiff = pOutSegs[i*7+2] - pOutSegs[j*7+0];
                   HeightDiff = pOutSegs[i*7+3] - pOutSegs[j*7+1];
              }
              Sign = WidthDiff < 0 ? -1 : 1;
              WidthDiff *= Sign;
              Sign = HeightDiff < 0 ? -1 : 1;
              HeightDiff *= Sign;
             if(WidthDiff < 20&&HeightDiff < 20)
             {
                    start.x = pOutSegs[i*7+0];
                    start.y = pOutSegs[i*7+1];
                    end.x = pOutSegs[i*7+0];
                    end.y = pOutSegs[i*7+1];
                     // to decide the start point of the merged line segment
                    // the start point of next line segment.
                    if ( start.x > pOutSegs[j*7+0])
                    {
                        start.x = pOutSegs[j*7+0];
                        start.y = pOutSegs[j*7+1];
                    }
                    // the end point of next line segment.
                    if ( start.x > pOutSegs[j*7+2])
                    {
                        start.x = pOutSegs[j*7+2];
                        start.y = pOutSegs[j*7+3];
                    }
                        // to decide the end point of the merged line segment
                        // the start point of current line segment.
                    if ( end.x < pOutSegs[i*7+2])
                    {
                        end.x = pOutSegs[i*7+2];
                        end.y = pOutSegs[i*7+3];
                    }
                    // the start point of next line segment.
                    if ( end.x < pOutSegs[j*7+0])
                    {
                        end.x = pOutSegs[j*7+0];
                        end.y = pOutSegs[j*7+1];
                    }
                    // the end point of next line segment.
                    if ( end.x < pOutSegs[j*7+2])
                    {
                        end.x = pOutSegs[j*7+2];
                        end.y = pOutSegs[j*7+3];
                    }
                   AngMerge = atan((MDouble)(end.y - start.y)/(end.x - start.x))*180/PI;
                   if (pSegParams[i].Mag > pSegParams[j].Mag)
                    {
                        AngleDiff = pSegParams[i].Angle - AngMerge;
                        Sign = AngleDiff < 0 ? -1 : 1;
                        AngleDiff *= Sign;
                        if ( AngleDiff < 2)
                        {
                            pOutSegs[i*7+0] = start.x;
                            pOutSegs[i*7+1] = start.y;
                            pOutSegs[i*7+2] = end.x;
                            pOutSegs[i*7+3] = end.y;
                        }
                    }
                   else
                   {
                        AngleDiff = pSegParams[j].Angle - AngMerge;
                        Sign = AngleDiff < 0 ? -1 : 1;
                        AngleDiff *= Sign;
                        if ( AngleDiff < 3)
                        {
                            pOutSegs[j*7+0] = start.x;
                            pOutSegs[j*7+1] = start.y;
                            pOutSegs[j*7+2] = end.x;
                            pOutSegs[j*7+3] = end.y;
                        }
                   }
              }
         }
    }

     EXT:
    FreeVectMem(hMemMgr, pSegParams);

  return res;
 }

 // caculate the average gray pixel value of the pointer (Oil level indicator)
 MRESULT GetAvgPointer(PBLOCK pImage, MPOINT SegMostStart, MPOINT SegMostEnd, MLong *pMean )
 {
     MRESULT res = LI_ERR_NONE;
     MLong x = 0, y = 0;
     MLong x1 = 0, y1 = 0;
     MLong lsum = 0;
     MLong lNumUp = 0, lNumDown = 0;
     MLong lMeanDown, lMeanUp;
     MLong lMean;
     MDouble dkSeg;
     MLong lbSeg;
     MUInt8 *pData;
     MLong lBlockLine, lHeight;
     MLong lPtrUpHeight,  lPtrDownHeight;
     
    pData= (MUInt8*)pImage->pBlockData;
    if(SegMostEnd.x > SegMostStart.x)
          dkSeg = (MDouble) (SegMostEnd.y - SegMostStart.y)/( SegMostEnd.x - SegMostStart.x);
    else
    {
        res = LI_ERR_GET_POINT;
        goto EXT;
    }
    lbSeg = SegMostStart.y - dkSeg * SegMostStart.x;
    lBlockLine = pImage->lBlockLine;
    lHeight = pImage->lHeight;

    for(x = SegMostStart.x; x < SegMostEnd.x; ++x)
    {
        y = dkSeg * x + lbSeg;
        lPtrUpHeight = y -14;
        if (lPtrUpHeight > 0)
        {
            for(y1 = -14; y1 <=0; y1++)
            {
                 lsum += pData[x+(y+y1) * lBlockLine];
                 lNumUp++;
              }
        }
        else
        {
            lPtrUpHeight += 14;
            for(y1 = -lPtrUpHeight; y1 <=0; y1++)
            {
                 lsum += pData[x+(y+y1) * lBlockLine];
                 lNumUp++;
              }
        }
    }
    lMeanUp = lsum/lNumUp;
    lsum=0;
        for(x = SegMostStart.x; x < SegMostEnd.x; ++x)
    {
        y =  dkSeg * x + lbSeg;
        lPtrDownHeight = y+15;
        if ( lPtrDownHeight < lHeight)
        {
            for(y1 = 1; y1 <=15; y1++)
            {
                lsum += pData[x+(y+y1) * lBlockLine];
                lNumDown++;
            }
        }
        else
        {
            lPtrDownHeight = lHeight-y;
            for(y1 = 1; y1 <= lPtrDownHeight; y1++)
            {
                lsum += pData[x+(y+y1) * lBlockLine];
                lNumDown++;
            }
        }
    }
    lMeanDown = lsum/lNumDown;

    if ( lMeanUp > lMeanDown)
        lMean = lMeanDown;
    else
        lMean = lMeanUp;
    *pMean = lMean; 
EXT:
    return res;
 }

    // divide the lHeight into 8 block to precisely get the interested region
    // binary the given region
 MVoid BinarySelectedRegion ( PBLOCK pImage, MLong lxEnd, MPOINT SegMostStart, MLong lMean, BLOCK *pBinaryImage)
 {
     MLong lRegHeight;
     MLong x = 0, y = 0;
     MUInt8 *pData;
     MUInt8 *pBinaryData;
     MLong lWidth, lHeight, lBlockLine;
     
     lWidth = pImage->lWidth;
     lHeight = pImage->lHeight;
     lBlockLine =  pImage->lBlockLine;
     pData = (MUInt8*)pImage->pBlockData;
     pBinaryData = (MUInt8*)pBinaryImage->pBlockData;

     lRegHeight = (MLong) (lHeight + 7) /(1<<3);
    // binary the given region
    for(y = 0; y < lHeight; y++)
    {
        if ( SegMostStart.y < 3 * lRegHeight)
        {
            for (x =0; x < lWidth; ++x)
               {
                   if( x < lxEnd && y < lHeight/2 )
                   {
                       if(pData[x + y * lBlockLine] >= lMean)
                           {
                               pBinaryData[x + y * lBlockLine] = 0;
                           }
                      else
                              pBinaryData[x + y * lBlockLine] = 255;
                   }
                else
                  pBinaryData[x + y * lBlockLine] = 0;
            }
        }
       if ( (SegMostStart.y >= 3 * lRegHeight) && (SegMostStart.y <= 5 * lRegHeight))
         {
            for (x = 0; x < lWidth; ++x)
               {
                   if( x < lxEnd && (( y >= 3 * lRegHeight) && (y <= 5 * lRegHeight)))
                   {
                       if(pData[x + y * lBlockLine] >= lMean)
                           {
                               pBinaryData[x + y * lBlockLine] = 0;
                           }
                      else
                              pBinaryData[x + y * lBlockLine] = 255;
                   }
                  else
                     pBinaryData[x + y * lBlockLine] = 0;
            }
        }
      if ( SegMostStart.y > 5 * lRegHeight)
        {
            for (x = 0; x < lWidth; ++x)
               {
                   if( x < lxEnd && (y >(MLong) lHeight/2))
                   {
                       if(pData[x + y * lBlockLine] >= lMean)
                           {
                               pBinaryData[x + y * lBlockLine] = 0;
                           }
                      else
                               pBinaryData[x + y * lBlockLine] = 255;
                   }
                 else
                  pBinaryData[x + y * lBlockLine] = 0;
            }
        }
    }
 }

 //  filter the boundary points which must be the leastsignificent points.
 MVoid FilterBoundaryPoints(PBLOCK pImage, PBLOCK pBlockImage, MLong lxEnd, BLOCK *pFilterImage)
 {
     MLong x = 0, y = 0;
     MLong x1 = 0, y1 = 0;
     MUInt8 *pData;
     MUInt8 *pBlockData;
     MUInt8 *pFilterData;
     MLong lWidth, lHeight, lBlockLine;
     MLong BdyWidth;
     MDouble factor = 0.03;
     
     lWidth = pImage->lWidth;
     lHeight = pImage->lHeight;
     lBlockLine =  pImage->lBlockLine;
     pData= (MUInt8*)pImage->pBlockData;
     pBlockData = (MUInt8*) pBlockImage->pBlockData;
     pFilterData =  (MUInt8*) pFilterImage->pBlockData;
     BdyWidth = (MLong) (factor*((lHeight + lWidth)/2)+0.5);

    for ( y = 0; y < lHeight; ++y)
       for(x = 0; x < lWidth; ++x)
       {
           if (y< BdyWidth || y> lHeight-BdyWidth|| x < BdyWidth || x > (lxEnd-BdyWidth))
           {
               if (pData[x+y*lBlockLine] != 0)
               {
                   for ( y1 = 0; y1 < lHeight; ++y1)
                       for ( x1 = 0; x1 < lxEnd; ++x1)
                       {
                           if(  pBlockData[x+y*lBlockLine] ==  pBlockData[x1+y1*lBlockLine] )
                               pFilterData[x1+y1*lBlockLine]=0;
                       }
               }
           }
       }
 }


 //  calculate the ratio between width and height of nonzero regions
 MVoid CalRegRatWidHgt(PBLOCK pImage, pFilterParam pParam, PBLOCK pBlockImage, MUInt32 lNumRegion)
 {
      MLong bSeg;
      MLong x = 0, y = 0;
      MLong lDiffRegWidth, lDiffRegHeight;
      MLong segy, segx;
      MLong i, j;
      MDouble Sign;
      MUInt8 Flag;
      MUInt8 *pData;
      MUInt8 *pBlockData;
      MUInt32 NumRegPoint;
      MLong lWidth, lHeight, lBlockLine;
     
     lWidth = pImage->lWidth;
     lHeight = pImage->lHeight;
     lBlockLine =  pImage->lBlockLine;
    pData= (MUInt8*)pImage->pBlockData;
    pBlockData = (MUInt8*) pBlockImage->pBlockData;
       for ( i = 0; i < lNumRegion; ++i)
       {
           pParam[i].LeftUp.x=0;
           pParam[i].LeftUp.y=0;
           pParam[i].RightDown.x=0;
           pParam[i].RightDown.y=0;
       }
       for ( i = 0; i < lNumRegion; ++i)
       {
           Flag = (MUInt8)(i+1); 
           for ( y = 0; y < lHeight; ++y)
               for (x = 0; x < lWidth; ++x)
               {
                   if ( pBlockData[x+y*lBlockLine] == Flag)
                   {
                       if ( pParam[i].LeftUp.x == 0 )
                       {
                            pParam[i].LeftUp.x = x;
                       }
                       else 
                       {
                           if(  pParam[i].LeftUp.x > x)
                                 pParam[i].LeftUp.x = x;
                       }
                       if ( pParam[i].LeftUp.y == 0)
                             pParam[i].LeftUp.y = y;
                       else 
                       {
                           if(   pParam[i].LeftUp.y > y)
                                 pParam[i].LeftUp.y = y;
                       }
                       if (pParam[i].RightDown.x == 0)
                           pParam[i].RightDown.x = x;
                       else 
                       {
                           if( pParam[i].RightDown.x < x)
                                pParam[i].RightDown.x = x;
                       }
                       if ( pParam[i].RightDown.y == 0)
                            pParam[i].RightDown.y = y;
                       else 
                       {
                           if( pParam[i].RightDown.y < y)
                                pParam[i].RightDown.y = y;
                       }
                   }
               }
          pParam[i].RegCenter.x = (pParam[i].RightDown.x + pParam[i].LeftUp.x)/2;
          pParam[i].RegCenter.y = (pParam[i].RightDown.y + pParam[i].LeftUp.y)/2;
          if ( pParam[i].RightDown.x - pParam[i].LeftUp.x == 0)
               pParam[i].kSeg = InfSlope;
          else
          {
               if ( pParam[i].RegCenter.y > lHeight/2)
                    pParam[i].kSeg = ((MDouble) (pParam[i].LeftUp.y - pParam[i].RightDown.y))/ (pParam[i].RightDown.x - pParam[i].LeftUp.x);
              else
                    pParam[i].kSeg = ((MDouble) (pParam[i].RightDown.y - pParam[i].LeftUp.y))/ (pParam[i].RightDown.x - pParam[i].LeftUp.x);
          }
            printf("k=%d, k2=%d, k3=%d, k4=%d\n", pParam[i].LeftUp.x, pParam[i].RightDown.x, pParam[i].LeftUp.y, pParam[i].RightDown.y);
       }

       // compute the ratio of width and height
       for ( i = 0; i < lNumRegion; ++i)
       {
           NumRegPoint = 0;
           lDiffRegHeight = pParam[i].RightDown.y - pParam[i].LeftUp.y;
           Sign = lDiffRegHeight < 0 ? -1 : 1;
           lDiffRegHeight *= Sign;
           lDiffRegWidth = pParam[i].RightDown.x - pParam[i].LeftUp.x;
           Sign = lDiffRegWidth < 0 ? -1 : 1;
           lDiffRegWidth *= Sign;
           if ( lDiffRegWidth >= lDiffRegHeight){
                for (segx = (pParam[i].RegCenter.x - lDiffRegWidth/4); segx < (pParam[i].RegCenter.x + lDiffRegWidth/4); ++segx)
                  {
                      Flag = (MUInt8)(i+1);
                      bSeg = pParam[i].RightDown.y + (1/pParam[i].kSeg) * segx;
                      for(y = pParam[i].LeftUp.y; y< pParam[i].RightDown.y; ++y)
                       {
                             x = (MLong) (y - bSeg) / (-1/pParam[i].kSeg);
                             if(pBlockData[x + y*lBlockLine] == Flag)   
                             NumRegPoint++;
                       }
                }
                if( NumRegPoint > 0)
                    pParam[i].RegionHeight =  (MLong) (NumRegPoint/ (lDiffRegWidth/2) +0.5);
                else
                    pParam[i].RegionHeight = lDiffRegWidth+1; 
           }
           else {
               for (segy = pParam[i].RegCenter.y - lDiffRegHeight/4; segy < pParam[i].RegCenter.y + lDiffRegHeight/4; ++segy)
                  {
                      Flag = (MUInt8)(i+1);
                      bSeg = segy + (1/pParam[i].kSeg) * pParam[i].RightDown.x;
                      for(x = pParam[i].LeftUp.x; x< pParam[i].RightDown.x; ++x)
                       {
                             y = (MLong) (-1/pParam[i].kSeg) * x + bSeg;
                             if(pBlockData[x + y*lBlockLine] == Flag)   
                             NumRegPoint++;
                       }
                       if( NumRegPoint > 0)
                            pParam[i].RegionHeight =  (MLong) (NumRegPoint/ (lDiffRegHeight/2) +0.5);
                       else
                            pParam[i].RegionHeight = lDiffRegHeight+1;
                  }
           } 
              pParam[i].RegionWidth = (MLong) (sqrt((((pParam[i].RightDown.y - pParam[i].LeftUp.y)*(pParam[i].RightDown.y - pParam[i].LeftUp.y)+(pParam[i].LeftUp.x - pParam[i].RightDown.x)*(pParam[i].LeftUp.x - pParam[i].RightDown.x))+0.5)));
              pParam[i].Ratio = (MDouble) pParam[i].RegionWidth/pParam[i].RegionHeight;
              printf( "ratio = %f, w=%d, h=%d\n",pParam[i].Ratio, pParam[i].RegionWidth, pParam[i].RegionHeight );
       }
 }

 // filter the regions being not satisfying the ratio (ratio>3) even more get rid of the red rectangle region( which is too big to pecisely get the width of actual scale)
 MVoid FilterOtherRegs(PBLOCK pBlockImage, pFilterParam pParam, MUInt32 lNumRegion, BLOCK *pFilterImage)
 {
      MLong i, j;
      MUInt8 Flag;
      MUInt8 *pBlockData;
      MLong x = 0, y = 0;
      MLong lNumThreshold = 0;
      MUInt8 *pFilterData;
     MLong lWidth, lHeight, lBlockLine;
     
     lWidth = pBlockImage->lWidth;
     lHeight = pBlockImage->lHeight;
     lBlockLine =  pBlockImage->lBlockLine;
     pBlockData = (MUInt8*) pBlockImage->pBlockData;
     pFilterData =  (MUInt8*) pFilterImage->pBlockData;

      for ( i = 0; i < lNumRegion; ++i)
      {
         Flag = (MUInt8)(i+1); 
         if ( pParam[i].Ratio < 3)
         {
            for ( y = 0; y < lHeight; ++y)
               for (x = 0; x < lWidth; ++x)
               {
                   if (pBlockData[x+y*lBlockLine] == Flag)
                   {
                       pFilterData[x+y*lBlockLine] = 0;
                   }
               }
         }
      }
     for ( i = 0; i < lNumRegion; ++i)
     {
         pParam[i].FlagNum = 0;
         Flag = (MUInt8)(i+1);
            for ( y = 0; y < lHeight; ++y)
               for (x = 0; x < lWidth; ++x)
               {
                   if (pBlockData[x+y*lBlockLine] == Flag)
                   {
                       pParam[i].FlagNum++;
                   }
               }
//               printf("n=%d\n", pParam[i].FlagNum);
         if( lNumThreshold < pParam[i].FlagNum )
         {
             lNumThreshold = pParam[i].FlagNum;
             j = i;
         }
     }

    if ( (pParam[j].RightDown.y > 0.9*lHeight && pParam[j].RightDown.x > 0.2*lWidth) || (pParam[j].LeftUp.y < 0.1*lHeight && pParam[j].RightDown.x > 0.2*lWidth))
    {
        Flag = (MUInt8)(j+1);
        for ( y = 0; y < lHeight; ++y)
            for (x = 0; x < lWidth; ++x)
            {
                if ( pBlockData[x+y*lBlockLine] == Flag)
                {
                    pFilterData[x+y*lBlockLine] = 0;
                }
            }
    }
 }

 // filter some regions which is much less than the max value.
 MVoid FilterLastRegs(PBLOCK pBlockImage, pFilterParam pParam, MUInt32 lNumRegion, BLOCK *pFilterImage)
 {
     MLong i;
      MUInt8 Flag;
      MUInt8 *pBlockData;
      MUInt8 *pData;
      MLong x = 0, y = 0;
      MLong lNumThreshold = 0;
      MLong lWidth, lHeight, lBlockLine;

     lWidth = pBlockImage->lWidth;
     lHeight = pBlockImage->lHeight;
     lBlockLine =  pBlockImage->lBlockLine;
     pData= (MUInt8*)pFilterImage->pBlockData;
     pBlockData = (MUInt8*) pBlockImage->pBlockData;

     for ( i = 0; i <  lNumRegion; ++i)
     {
         pParam[i].FlagNum = 0;
         Flag = (MUInt8)(i+1);
            for ( y = 0; y < lHeight; ++y)
               for (x = 0; x < lWidth; ++x)
               {
                   if ( pBlockData[x+y*lBlockLine] == Flag)
                   {
                       pParam[i].FlagNum++;
                   }
               }
               printf("n=%d\n",pParam[i].FlagNum);
     }
        for ( i = 0; i < lNumRegion; ++i)
     {
         if(lNumThreshold < pParam[i].FlagNum )
             lNumThreshold = pParam[i].FlagNum;
     }
         printf("kn=%d\n", lNumThreshold);
        // filter the points which is lower than three quarters of the most leaving region
    for ( i = 0; i < lNumRegion; ++i)
     {
          Flag = (MUInt8)(i+1);
          if (pParam[i].FlagNum < 0.75* lNumThreshold){
              pParam[i].FlagNum = 0;
            for ( y = 0; y < lHeight; ++y)
               for (x = 0; x < lWidth; ++x)
               {
                   if ( pBlockData[x+y*lBlockLine] == Flag)
                   {
                        pData[x+y*lBlockLine] = 0;
                   }
               }
          }
     }
 }