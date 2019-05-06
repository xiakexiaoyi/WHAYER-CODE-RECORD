#include "hough.h"
#include "limem.h"
#include "sort.h"
#include "limath.h"
#include <math.h>
#include <stdlib.h>
#include<time.h>
#include "lidebug.h"
typedef struct tag_locaMax
{
    MLong lRho;
    MLong lAngle;
    MLong lAccuNum;
}LOCALMAX;

JSTATIC MLong HoughSpaceCmp( const MVoid* p1, const MVoid* p2);

MRESULT HoughTransform(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
    MLong lWidth, MLong lHeight, MByte ForeFlag,
    MDouble rho, MDouble theta, MLong lThreshold, MLong lLineMax, 
    LinePolar* pLines)
{
    MRESULT res=LI_ERR_NONE;
    MLong *accum = 0;
    LOCALMAX *sort_buf=0;
    MDouble *tabSin = 0;
    MDouble *tabCos = 0;
    MLong lNumAngle, lNumRho;
    MLong i, j, n, r, total=0;
    MDouble ang;
    MDouble irho = 1/rho;
    MByte* pTmpBin=MNull;
    MLong lBinExt;
    lNumAngle = (MLong)(V_PI/theta + 0.5);
    lNumRho = (MLong)(((lWidth + lHeight) * 2 + 1) / rho + 0.5);

    AllocVectMem(hMemMgr, accum, (lNumRho+2)*(lNumAngle+2), MLong);
    AllocVectMem(hMemMgr, sort_buf, lNumAngle*lNumRho/4, LOCALMAX);
    AllocVectMem(hMemMgr, tabSin, lNumAngle, MDouble);
    AllocVectMem(hMemMgr, tabCos, lNumAngle, MDouble);

    SetVectZero(accum, sizeof(*accum)*(lNumRho+2)*(lNumAngle+2));

    for( ang = 0, i = 0; i < lNumAngle; ang += theta, i++ )
    {
        tabSin[i] = (MDouble)(sin(ang) * irho);
        tabCos[i] = (MDouble)(cos(ang) * irho);
    }
    // stage 1. fill accumulator
    pTmpBin = pBinData;
    lBinExt = lSrcLine - lWidth;
    for( j = 0; j < lHeight; j++, pTmpBin+=lBinExt)
    {
        for( i = 0; i < lWidth; i++, pTmpBin++)
        {
            if( *pTmpBin == ForeFlag )
                for( n = 0; n < lNumAngle; n++ )
                {
                    MLong r;
                    r = (MLong)( i * tabCos[n] + j * tabSin[n] + 0.5);
                    r += (lNumRho - 1) / 2;
                    accum[(n+1) * (lNumRho+2) + r+1]++;
                }
        }
    }
    // stage 2. find local maximums
    for( r = 0; r < lNumRho; r++ )
    {
        for( n = 0; n < lNumAngle; n++ )
        {
            MLong base = (n+1) * (lNumRho+2) + r+1;
            if( accum[base] > lThreshold &&
                accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                accum[base] > accum[base - lNumRho - 2] && accum[base] >= accum[base + lNumRho + 2] )
            {
                sort_buf[total].lRho = r+1;
                sort_buf[total].lAngle = n+1;
                sort_buf[total++].lAccuNum = accum[base];
            }
        }
    }

    //
    GO(QuickSort(hMemMgr, (MVoid*)sort_buf, total, sizeof(*sort_buf), HoughSpaceCmp));
    lLineMax = MIN(lLineMax, total);
    for( i = 0; i < lLineMax; i++ )	
    {
        //LinePolar* pCurLine = pLines+i;
        *(pLines->rho+i) = (sort_buf[i].lRho -1 - (lNumRho - 1)*0.5f )*rho;
        *(pLines->angle+i) = (sort_buf[i].lAngle-1)*theta;
        pLines->lLineNum++;
    }
EXT:
    FreeVectMem(hMemMgr, accum);
    FreeVectMem(hMemMgr, sort_buf);
    FreeVectMem(hMemMgr, tabSin);
    FreeVectMem(hMemMgr, tabCos);
    return res;
}


MLong HoughSpaceCmp( const MVoid* p1, const MVoid* p2)
{
    LOCALMAX _p1 = *(LOCALMAX*)p1;
    LOCALMAX _p2 = *(LOCALMAX*)p2;

    if( _p1.lAccuNum < _p2.lAccuNum)
        return 1;
    if (_p1.lAccuNum > _p2.lAccuNum)
        return -1;
    return 0;
}

MRESULT ProbabilisticHoughTransform(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
    MLong lWidth, MLong lHeight, MByte ForeFlag,
    MDouble rho, MDouble theta, MLong lThreshold, MLong lLineLength, MLong lMaxLineGap, 
    MLong lLinesMax, LineSegment* pLines)
{
    MRESULT res=LI_ERR_NONE;
    MLong *pAccum = 0;
    MLong *pTmpAccum;
    MDouble *pTabSin = 0;
    MDouble *pTabCos = 0;
    MLong lNumAngle, lNumRho;
    MLong n, r, num = 0, total = 0;
    MLong x, y, idx, count;
    MDouble ang;
    MDouble irho = 1/rho;
    MByte *pTmpBin;
//	MByte *pTmpBinData;
    MLong lBinExt;
    MByte *pMaskData = MNull;
    MByte *pTmpMask, *pCurMaskData;
    MPOINT *pPtData = MNull;
    MPOINT *pPtTmpData;
    MPOINT *pCurPtData;
    MPOINT Line[2];
    MLong lMaxVal, lMaxN, lVal;
    MDouble a, b;
    MLong i, j, k, x0, y0, dx0, dy0, xflag;
    MLong xi, yi, dxi, dyi;
    MLong xj, yj, dxj, dyj;
    MLong i1, j1, i2, j2;
    MLong lGap;
    MLong lGoodLine;
    MLong lAccountNum = 0;
    const MLong shift = 16;

    lNumAngle = (MLong)(V_PI/theta + 0.5);
    lNumRho = (MLong)(((lWidth + lHeight) * 2 + 1) / rho + 0.5);

    AllocVectMem(hMemMgr, pAccum, (lNumRho)*(lNumAngle), MLong);
    AllocVectMem(hMemMgr, pTabSin, lNumAngle, MDouble);
    AllocVectMem(hMemMgr, pTabCos, lNumAngle, MDouble);
    AllocVectMem(hMemMgr, pMaskData, lSrcLine*lHeight, MByte);

    SetVectZero(pAccum, (lNumRho)*(lNumAngle)*sizeof(MLong));
    SetVectZero(pMaskData, (lSrcLine)*(lHeight)*sizeof(MByte));

    for( ang = 0, i = 0; i < lNumAngle; ang += theta, i++ )
    {
        pTabSin[i] = (MDouble)(sin(ang) * irho);
        pTabCos[i] = (MDouble)(cos(ang) * irho);
    }

    // stage 1. collect non-zero image points
    pTmpBin = pBinData;
    pTmpMask = pMaskData;
    lBinExt = lSrcLine - lWidth;
    for(y=0; y<lHeight; y++, pTmpBin+=lBinExt, pTmpMask+=lBinExt)
    {
        for(x=0; x<lWidth; x++, pTmpBin++, pTmpMask++)
        {
            if(*pTmpBin == ForeFlag)
            {
                *pTmpMask = (MByte)1;
                total++;
            }
            else
            {
                *pTmpMask = 0;
            }
        }
    }

    AllocVectMem(hMemMgr, pPtData, total, MPOINT);
    pPtTmpData = pPtData;
    pTmpMask = pMaskData;
    for(y=0; y<lHeight; y++, pTmpMask+=lBinExt)
    {
        for(x=0; x<lWidth; x++, pTmpMask++)
        {
            if(*(pTmpMask))
            {
                pPtTmpData->x = x;
                pPtTmpData->y = y;
                pPtTmpData++;
            }
        }
    }

    // stage 2. process all the points in random order

    srand( (unsigned) time( MNull ) );
    pTmpMask = pMaskData;
    for (count = total; count>0; count--)
    {
        idx = (rand()) % total;
        lMaxVal = lThreshold - 1;
        lMaxN = 0;
        Line[0].x = 0;
        Line[0].y = 0;
        Line[1].x = 0;
        Line[1].y = 0;
        pCurPtData = pPtData + idx;
        i = pCurPtData->y;
        j = pCurPtData->x;

        // "remove" it by overriding it with the last element
        pCurPtData = pPtData + count - 1;

        // check if it has been excluded already (i.e. belongs to some other line)
        if(!pTmpMask[j+i*lSrcLine])
            continue;

        // update accumulator, find the most probable line
        pTmpAccum = pAccum;
        for( n = 0; n < lNumAngle; n++, pTmpAccum += lNumRho )
        {
            r = (MLong)(j * pTabCos[n] + i * pTabSin[n] + 0.5);
            r += (lNumRho - 1) / 2;
            lVal = ++pTmpAccum[r];
            if( lMaxVal < lVal )
            {
                lMaxVal = lVal;
                lMaxN = n;
            }
        }

        // if it is too "weak" candidate, continue with another point
        if( lMaxVal < lThreshold )
            continue;

        // from the current point walk in each direction
        // along the found line and extract the line segment
        a = -pTabSin[lMaxN];
        b = pTabCos[lMaxN];
        x0 = j;
        y0 = i;
        if( fabs(a) > fabs(b) )
        {
            xflag = 1;
            dx0 = a > 0 ? 1 : -1;
            dy0 = (MLong) (b*(1 << shift)/fabs(a) + 0.5);
            y0 = (y0 << shift) + (1 << (shift-1));
        }
        else
        {
            xflag = 0;
            dy0 = b > 0 ? 1 : -1;
            dx0 = (MLong) (a*(1 << shift)/fabs(b) + 0.5);
            x0 = (x0 << shift) + (1 << (shift-1));
        }


        for( k = 0; k < 2; k++ )
        {
            lGap = 0;
            xi = x0;
            yi = y0;
            dxi = dx0;
            dyi = dy0;

            if( k > 0 )
            {
                dxi = -dxi;
                dyi = -dyi;
            }

            // walk along the line using fixed-point arithmetics,
            // stop at the image border or in case of too big gap
            for( ; ; xi += dxi, yi += dyi)
            {
                if( xflag )
                {
                    j1 = xi;
                    i1 = yi >> shift;
                }
                else
                {
                    j1 = xi >> shift;
                    i1 = yi;
                }

                if( j1 < 0 || j1 >= lWidth || i1 < 0 || i1 >= lHeight )
                    break;

                pCurMaskData = pMaskData + i1*lSrcLine + j1;

                // for each non-zero point:
                //    update line end,
                //    clear the mask element
                //    reset the gap
                if( *pCurMaskData )
                {
                    lGap = 0;
                    Line[k].y = i1;
                    Line[k].x = j1;
                }
                else if( ++lGap > lMaxLineGap )
                    break;
            }
        }

        lGoodLine = abs(Line[1].x - Line[0].x) >= lLineLength ||
            abs(Line[1].y - Line[0].y) >= lLineLength;

        for( k = 0; k < 2; k++ )
        {
            xj = x0;
            yj = y0;
            dxj = dx0;
            dyj = dy0;

            if( k > 0 )
            {
                dxj = -dxj;
                dyj = -dyj;
            }

            // walk along the line using fixed-point arithmetics,
            // stop at the image border or in case of too big gap
            for( ;; xj += dxj, yj += dyj )
            {
                if( xflag )
                {
                    j2 = xj;
                    i2 = yj >> shift;
                }
                else
                {
                    j2 = xj >> shift;
                    i2 = yj;
                }

                pCurMaskData = pMaskData + i2*lSrcLine + j2;

                // for each non-zero point:
                //    update line end,
                //    clear the mask element
                //    reset the gap
                if( *pCurMaskData )
                {
                    if( lGoodLine )
                    {
                        pTmpAccum = pAccum;
                        for( n = 0; n < lNumAngle; n++, pTmpAccum += lNumRho )
                        {
                            r =(MLong)( j2 * pTabCos[n] + i2 * pTabSin[n] + 0.5 );
                            r += (lNumRho - 1) / 2;
                            pTmpAccum[r]--;
                        }
                    }
                    *pCurMaskData = 0;
                }

                if( i2 == Line[k].y && j2 == Line[k].x )
                    break;
            }
        }
        if( lGoodLine )
        {
            (pLines->StartPt+lAccountNum)->x = Line[0].x;
            (pLines->StartPt+lAccountNum)->y = Line[0].y;
            (pLines->EndPt+lAccountNum)->x = Line[1].x;
            (pLines->EndPt+lAccountNum)->y = Line[1].y;
            pLines->lLineNum = ++lAccountNum;
            if( pLines->lLineNum >= lLinesMax )
                break;
        }

    }
EXT:
    FreeVectMem(hMemMgr, pAccum);
    FreeVectMem(hMemMgr, pTabSin);
    FreeVectMem(hMemMgr, pTabCos);
    FreeVectMem(hMemMgr, pMaskData);
    FreeVectMem(hMemMgr, pPtData);
    return res;

}