#include "liedge.h"
#include "limem.h"
#include "limath.h"
#include "lidebug.h"

#ifdef OPTIMIZATION_ARM
//#include "liedge_arm.h"
#endif

JSTATIC MVoid Sobel(  MShort* Gx, MShort* Gy, MLong lGradLine,
			MLong lWidth, MLong lHeight,
			MVoid *pRlt, MLong lRltLine, MLong thres)
{
	MLong i, j;
	MLong lRltExt;
	MByte *pTmpRlt;
	MShort* pTmpGx, *pTmpGy;
	MLong lGradExt;
	
	pTmpRlt = (MByte*)(pRlt);
	lRltExt = lRltLine - lWidth;
	pTmpGx = Gx; pTmpGy = Gy;
	lGradExt = lGradLine - lWidth;
	for (j=0; j<lHeight; j++, pTmpGx+=lGradExt, pTmpGy+=lGradExt, pTmpRlt+=lRltExt)
	{
		for (i=0; i<lWidth; i++, pTmpGx++, pTmpGy++, pTmpRlt++)
		{
			if (ABS(*pTmpGx)+ABS(*pTmpGy) < thres)
				*pTmpRlt = 255;
			else
				*pTmpRlt = 0;
		}
	}
}

/*  1) Convolve the image with a separable Gaussian filter.
*   2) Take the dx and dy the first derivatives using [-1,0,1] and [1,0,-1]'.
*   3) Compute the magnitude: sqrt(dx*dx+dy*dy).
*   4) Perform non-maximal suppression.
*   5) Perform hysteresis.
*/
//#include <math.h>
//#include "liflt_smooth.h"

JSTATIC MVoid Gradient_C(MVoid *pImgSrc, MLong lSrcLine, 
			   MShort *Gx, MShort *Gy, MLong lGraLine,
			   MUInt16 *pMag, MLong lMagLine,
			   MLong lWidth, MLong lHeight, MDWord* plSum,
			   MDWord* pMaxGrad, MDWord *plNum)
{
	MLong i, j;
	MByte *pTmpSrc;
	MShort *tmpGx, *tmpGy,*tmpGxTail,*tmpGyTail;
	MUInt16 *tmpMag,*tmpMagTail;
	MLong lSrcExt, lMagExt, lGradExt;
	MLong nw, n, ne, w, c, e, sw, s, se;
	MDWord dwSum=0;
	MDWord MaxGrad=0;
	MDWord dwNum=0;
	
	pTmpSrc = (MByte*)pImgSrc+lSrcLine+1;
	tmpMag = pMag + lMagLine + 1;
	tmpGx=Gx+lGraLine+1;
	tmpGy=Gy+lGraLine+1;
	lSrcExt = lSrcLine - (lWidth-1-1);
	lGradExt = lGraLine - (lWidth-1-1);
	lMagExt = lMagLine - (lWidth-1-1);
	for (j=1; j<lHeight-1; j++,pTmpSrc+=lSrcExt,tmpGx+=lGradExt, tmpGy+=lGradExt, tmpMag+=lMagExt)
	{
		nw = *(pTmpSrc-lSrcLine-1);
		n = *(pTmpSrc-lSrcLine);
		w = *(pTmpSrc-1);
		c = *(pTmpSrc);
		sw = *(pTmpSrc+lSrcLine-1);
		s = *(pTmpSrc+lSrcLine);
		for (i=1; i<lWidth-1; i++, pTmpSrc++, 
			tmpGx++, tmpGy++, tmpMag++)
		{
			MShort gxVal, gyVal;
			MDWord gSum;
			ne = *(pTmpSrc-lSrcLine+1);
			e = *(pTmpSrc+1);
			se = *(pTmpSrc+lSrcLine+1);
			
			gxVal = (MShort)(((e - w)<<1)+(ne-nw)+(se-sw));
			gyVal = (MShort)(((s-n)<<1)+(se-ne)+(sw-nw));
			gSum = ABS(gxVal)+ABS(gyVal);
			nw=n;w=c;sw=s;n=ne;c=e;s=se;
			
			if (pMag!=MNull)
				*tmpMag = gSum;
			*tmpGx = gxVal; *tmpGy = gyVal;
			dwSum += gSum;
			if (MaxGrad<gSum)
				MaxGrad = gSum;
		}
	}

	tmpGxTail = Gx+(lHeight-1)*lGraLine;
	tmpGyTail = Gy+(lHeight-1)*lGraLine;
	tmpMagTail = pMag+(lHeight-1)*lMagLine;

	SetVectZero(Gx, lGraLine*sizeof(MShort));
	SetVectZero(Gy, lGraLine*sizeof(MShort));
	SetVectZero(tmpGxTail, lGraLine*sizeof(MShort));
	SetVectZero(tmpGyTail, lGraLine*sizeof(MShort));
	if (pMag!=MNull)
	{
		SetVectZero(pMag, lMagLine*sizeof(MUInt16));
		SetVectZero(tmpMagTail, lMagLine*sizeof(MUInt16));
	}
	tmpGx = Gx;
	tmpGxTail = Gx+(lWidth-1);
	tmpGy = Gy;
	tmpGyTail = Gy+(lWidth-1);
	tmpMag = pMag;
	tmpMagTail = pMag+(lWidth-1);
	for (j=0; j<lHeight;j++, tmpGx+=lGraLine, tmpGy+=lGraLine,tmpGxTail+=lGraLine,tmpGyTail+=lGraLine,
							tmpMag+=lMagLine, tmpMagTail+=lMagLine)
	{
		*tmpGx = *tmpGy = *tmpGxTail = *tmpGyTail = 0;
		if (pMag!=MNull)
			*tmpMag = *tmpMagTail = 0;
	}
	if (plSum!=MNull) *plSum = dwSum;
	if (pMaxGrad!=MNull)*pMaxGrad = MaxGrad;
	if (plNum!=MNull)*plNum=dwNum;
}

MVoid Gradient(MVoid *pImgSrc, MLong lSrcLine, 
				 MShort *Gx, MShort *Gy, MLong lGraLine,
				 MUInt16 *pMag, MLong lMagLine,//(ABS(Gx)+ABS(Gy))/2
				 MLong lWidth, MLong lHeight, MDWord* plSum,
				 MDWord* pMaxGrad, MDWord *plNum)
{
// #ifdef OPTIMIZATION_ARM
// 	if (lSrcLine%4==0 && lGraLine%4==0 && (MDWord)pImgSrc%4==0 && lWidth>4)
// 	{
// 		Gradient_ARM(pImgSrc, lSrcLine, Gx, Gy, lGraLine, lWidth, lHeight, plSum, pMaxGrad, plNum);
// 	}
// 	else
//#endif
	{
		Gradient_C(pImgSrc, lSrcLine, Gx, Gy, lGraLine, pMag, lMagLine,
			lWidth, lHeight, plSum, pMaxGrad, plNum);
	}
}

//2--edge dictation, 1-- nonedge dictation.

#define CANNY_PUSH(d)    *(d) = (MByte)2, *stack_top++ = (d)
#define CANNY_POP(d)     (d) = *--stack_top

MRESULT Canny(MHandle hMemMgr, MShort* Gx, 
			  MShort* Gy, MLong lGradLine,
			  MLong lSrcWidth, MLong lSrcHeight, 
			  MVoid *pRlt, MLong lRltLine, 
			  MLong lowThres, MLong highThres)
{
	MRESULT res=LI_ERR_NONE;
	MLong i, j, x, y;
	
	MLong lMagExt, lMagLine;
	MShort *tmpGx, *tmpGy, *tmpGx2, *tmpGy2;

	MLong *buffer=MNull;
	MLong* mag_buf[3];
	MLong maxsize=0;
	MByte **stack_top=0, **stack_bottom = 0;

	MLong *_mag;

	MByte *map = (MByte*)pRlt;
	MLong mapstep = lRltLine;

	SetVectMem(pRlt, lRltLine*lSrcHeight, 1, MByte);

	//lMagLine = CEIL_4(lSrcWidth);
	lMagLine = lGradLine;
// 	AllocVectMem(hMemMgr, Gx, lMagLine*lSrcHeight, MShort);
//	AllocVectMem(hMemMgr, Gy, lMagLine*lSrcHeight, MShort);

//	Gradient(pSrc, lSrcLine, Gx, Gy, lMagLine, MNull, 0, lSrcWidth, lSrcHeight, MNull);

	AllocVectMem(hMemMgr, buffer, lSrcWidth*3, MLong);
	mag_buf[0] = (MLong*)buffer;
	mag_buf[1] = mag_buf[0] + lSrcWidth;
	mag_buf[2] = mag_buf[1] + lSrcWidth;
	
	//maxsize = MAX( 1 << 10, lSrcWidth*lSrcHeight/10 );
	//maxsize = lSrcWidth*lSrcHeight/10;
	maxsize = lSrcWidth*lSrcHeight;
	AllocVectMem(hMemMgr, stack_top, maxsize, MByte*);
	stack_bottom = stack_top;
	SetVectZero(mag_buf[0], lSrcWidth*sizeof(MLong));

 	SetVectMem(map, mapstep, 1, MByte);
 	SetVectMem(map+mapstep*(lSrcHeight-1), mapstep, 1, MByte);

	//MMemInfoStatic(hMemMgr, &dwTotalSize, &dwUsedSize);

	// calculate magnitude and angle of gradient, perform non-maxima supression.
    // fill the map with one of the following values:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge

	_mag = mag_buf[1];
	tmpGx = Gx; tmpGy = Gy;
	for( j = 0; j < lSrcWidth; j++, tmpGx++, tmpGy++)
        _mag[j] = ABS(*tmpGx) + ABS(*tmpGy);
	if( (stack_top - stack_bottom) + lSrcWidth > maxsize )
	{
		MByte** new_stack_bottom=0;
		maxsize = MAX( maxsize * 3/2, maxsize + 8 );
		AllocVectMem(hMemMgr, new_stack_bottom, maxsize, MByte*);
		CpyVectMem(new_stack_bottom, stack_bottom, (stack_top - stack_bottom), MByte*);
		stack_top = new_stack_bottom + (stack_top - stack_bottom);
		FreeVectMem(hMemMgr,stack_bottom);
		stack_bottom = new_stack_bottom;
   }

	lMagExt = lMagLine - (lSrcWidth-2);
    for( i = 2, tmpGx = Gx+i*lMagLine, tmpGy=Gy+i*lMagLine; i < lSrcHeight-2; i++, tmpGx+=lMagLine, tmpGy+=lMagLine)
	{
		MByte* _map;
		MLong magstep1, magstep2;
        MLong prev_flag = 0;
		MShort* dx, *dy;
		_mag = mag_buf[2];

		dx = tmpGx;
		dy = tmpGy;
		for( j = 0; j < lSrcWidth; j++)
            _mag[j] = ABS(dx[j]) + ABS(dy[j]);
		_map = map + mapstep*(i-1);
		_map[0] = _map[lSrcWidth-1]=1;
		_mag = mag_buf[1]; // take the central row
		dx = tmpGx - lMagLine;
		dy = tmpGy - lMagLine;
		magstep1 = (MLong)(mag_buf[2] - mag_buf[1]);
        magstep2 = (MLong)(mag_buf[0] - mag_buf[1]);
		for( j = 2, tmpGx2 = dx+j, tmpGy2=dy+j; j < lSrcWidth-2; j++, tmpGx2++, tmpGy2++)
        {
#define CANNY_SHIFT 15
//#define TG22  (MLong)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5)
#define  TG22 13573
			MLong s, m;
			x = *tmpGx2;
			y = *tmpGy2;
            s = x ^ y;
            m = _mag[j];
			
            x = ABS(x);
            y = ABS(y);
            if( m > lowThres )
            {
                MLong tg22x = x * TG22;
                MLong tg67x = tg22x + ((x + x) << CANNY_SHIFT);
				
                y <<= CANNY_SHIFT;
				
                if( y < tg22x )
                {
                    if( m > _mag[j-1] && m >= _mag[j+1] )
                    {
                        if( m > highThres && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
                else if( y > tg67x )
                {
                    if( m > _mag[j+magstep2] && m >= _mag[j+magstep1] )
                    {
                        if( m > highThres && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
                else
                {
                    s = s < 0 ? -1 : 1;
                    if( m > _mag[j+magstep2-s] && m > _mag[j+magstep1+s] )
                    {
                        if( m > highThres && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
            }
            prev_flag = 0;
            _map[j] = (MByte)1;
        }
		
        // scroll the ring buffer
        _mag = mag_buf[0];
        mag_buf[0] = mag_buf[1];
        mag_buf[1] = mag_buf[2];
        mag_buf[2] = _mag;
	}
	// now track the edges (hysteresis thresholding)
    while( stack_top > stack_bottom )
    {
        MByte* m;
        if( (stack_top - stack_bottom) + 8 > maxsize )
        {
		//	goto EXT;
            MByte** new_stack_bottom=0;
			MLong newSize = MAX( maxsize * 3/2, maxsize + 8 );
            //maxsize = MAX( maxsize * 3/2, maxsize + 8 );
			AllocVectMem(hMemMgr, new_stack_bottom, maxsize, MByte*);
			CpyVectMem(new_stack_bottom, stack_bottom, (stack_top - stack_bottom), MByte*);
            stack_top = new_stack_bottom + (stack_top - stack_bottom);
            FreeVectMem(hMemMgr,stack_bottom);
			maxsize = newSize;
           stack_bottom = new_stack_bottom;
		}
		
        CANNY_POP(m);
		
        if( !m[-1] )
            CANNY_PUSH( m - 1 );
        if( !m[1] )
            CANNY_PUSH( m + 1 );
        if( !m[-mapstep-1] )
            CANNY_PUSH( m - mapstep - 1 );
        if( !m[-mapstep] )
            CANNY_PUSH( m - mapstep );
        if( !m[-mapstep+1] )
            CANNY_PUSH( m - mapstep + 1 );
        if( !m[mapstep-1] )
            CANNY_PUSH( m + mapstep - 1 );
        if( !m[mapstep] )
            CANNY_PUSH( m + mapstep );
        if( !m[mapstep+1] )
            CANNY_PUSH( m + mapstep + 1 );
    }
	{
		MByte* pTmpRlt=MNull, *pTmpTail=MNull;
		MLong lRltExt;

		pTmpRlt = map;
		lRltExt = lRltLine - lSrcWidth;
		for( i = 0; i < lSrcHeight; i++, pTmpRlt+=lRltExt )
		{
			for( j = 0; j < lSrcWidth; j++, pTmpRlt++)
				*pTmpRlt = (MByte)((*pTmpRlt >> 1)-1);
		}
		
		pTmpRlt = (MByte*)pRlt;
		pTmpTail =  (MByte*)pRlt+(lSrcHeight-2)*lRltLine;
		SetVectMem(pTmpRlt, lRltLine*2, 255, MByte);
		SetVectMem(pTmpTail, lRltLine*2, 255, MByte);
		pTmpRlt = (MByte*)pRlt;
		pTmpTail =  (MByte*)pRlt+1;
		for (j=0; j<lSrcHeight;j++, pTmpRlt+=lRltLine, pTmpTail+=lRltLine)
		{
			*pTmpRlt = *pTmpTail = 255;
		}
		pTmpRlt = (MByte*)pRlt+lSrcWidth-2;
		pTmpTail =  (MByte*)pRlt+lSrcWidth-1;
		for (j=0; j<lSrcHeight;j++, pTmpRlt+=lRltLine, pTmpTail+=lRltLine)
		{
			*pTmpRlt = *pTmpTail = 255;
		}
	}

EXT:
	FreeVectMem(hMemMgr, buffer);
	FreeVectMem(hMemMgr, stack_bottom);
// 	FreeVectMem(hMemMgr, Gx);
//	FreeVectMem(hMemMgr, Gy);
	return res;
}

MRESULT SobelCanny(MHandle hMemMgr, MShort* Gx, 
			  MShort* Gy, MLong lGradLine,
			  MVoid *pCanny, MLong lCannyLine, 
			  MVoid *pSobel, MLong lSobelLine,
			  MVoid *pMask, MLong lMaskLine,
			  MLong lWidth, MLong lHeight, 
			  MRECT* pRtProtect,
			  MLong dwSum, MBool bIsLeaveSmall,
			  MLong lowThres, MLong highThres)
{
	MRESULT res=LI_ERR_NONE;
	MLong i, j, x, y;
	
	MLong lMagExt, lMagLine;
	MByte *pTmpSobel, *pTmpMask;
	MShort *tmpGx, *tmpGy, *tmpGx2, *tmpGy2;

	MLong *buffer=MNull;
	MLong* mag_buf[3];
	MLong maxsize=0;
	MByte **stack_top=0, **stack_bottom = 0;

	MLong *_mag;

	MByte *map = (MByte*)pCanny;
	MLong mapstep = lCannyLine;

	MLong  CutOff, Coeff=2;

	MLong lowThresEx, highThresEx;

	SetVectMem(pCanny, lCannyLine*lHeight, 1, MByte);
	if (bIsLeaveSmall)
		CutOff = Coeff*dwSum/((lWidth-1)*(lHeight-1));
	else
		CutOff = 3*dwSum/(2*(lWidth-1)*(lHeight-1));

	//lMagLine = CEIL_4(lSrcWidth);
	lMagLine = lGradLine;

	AllocVectMem(hMemMgr, buffer, lWidth*3, MLong);
	mag_buf[0] = (MLong*)buffer;
	mag_buf[1] = mag_buf[0] + lWidth;
	mag_buf[2] = mag_buf[1] + lWidth;
	
	maxsize = MAX( 1 << 10, lWidth*lHeight/10 );
	//maxsize = lSrcWidth*lSrcHeight/10;
	//maxsize = lWidth*lHeight/10; 
	AllocVectMem(hMemMgr, stack_top, maxsize, MByte*);
	stack_bottom = stack_top;
	SetVectZero(mag_buf[0], lWidth*sizeof(MLong));

 	SetVectMem(map, mapstep, 1, MByte);
 	SetVectMem(map+mapstep*(lHeight-1), mapstep, 1, MByte);

	// calculate magnitude and angle of gradient, perform non-maxima supression.
    // fill the map with one of the following values:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge

	_mag = mag_buf[1];
	tmpGx = Gx; tmpGy = Gy;
	for( j = 0; j < lWidth; j++, tmpGx++, tmpGy++)
        _mag[j] = ABS(*tmpGx) + ABS(*tmpGy);

	pTmpSobel= (MByte*)pSobel;
	SetVectMem(pTmpSobel, lSobelLine*2, 255, MByte);
	pTmpSobel += lSobelLine*(lHeight-2);
	SetVectMem(pTmpSobel, lSobelLine*2, 255, MByte);

	lMagExt = lMagLine - (lWidth-2);
	//pTmpSobel = (MByte*)pSobel+2*lSobelLine+2;
    for( i = 2, tmpGx = Gx+i*lMagLine, tmpGy=Gy+i*lMagLine, 
				pTmpSobel=(MByte*)pSobel+i*lSobelLine,
				pTmpMask = (MByte*)pMask+i*lMaskLine; 
				i < lHeight-2; i++, tmpGx+=lMagLine, tmpGy+=lMagLine, 
									pTmpSobel+=lSobelLine, pTmpMask+=lMaskLine)
	{
		MBool bIsProtect = MFalse;
		MByte* _map;
		MLong magstep1, magstep2;
        MLong prev_flag = 0;
		MShort* dx, *dy;
		_mag = mag_buf[2];

		dx = tmpGx;
		dy = tmpGy;
		for( j = 0; j < lWidth; j++)
        {
			MLong absMag = ABS(dx[j]) + ABS(dy[j]);
			_mag[j] = absMag;
			if (absMag>CutOff && pTmpMask[j]==255)
					pTmpSobel[j] = 0;
			else
				pTmpSobel[j] = 255;
		}

		_map = map + mapstep*(i-1);
		_map[0] = _map[lWidth-1]=1;
		_mag = mag_buf[1]; // take the central row
		dx = tmpGx - lMagLine;
		dy = tmpGy - lMagLine;
		magstep1 = (MLong)(mag_buf[2] - mag_buf[1]);
        magstep2 = (MLong)(mag_buf[0] - mag_buf[1]);

		if( (stack_top - stack_bottom) + lWidth > maxsize )
		{
			MByte** new_stack_bottom=0;
			maxsize = MAX( maxsize * 3/2, maxsize + 8 );
			AllocVectMem(hMemMgr, new_stack_bottom, maxsize, MByte*);
			CpyVectMem(new_stack_bottom, stack_bottom, (stack_top - stack_bottom), MByte*);
			stack_top = new_stack_bottom + (stack_top - stack_bottom);
			FreeVectMem(hMemMgr,stack_bottom);
			stack_bottom = new_stack_bottom;
		}

		if (i>pRtProtect->top&&i<pRtProtect->bottom)
		{
			bIsProtect = MTrue;
		}

		for( j = 2, tmpGx2 = dx+j, tmpGy2=dy+j; j < lWidth-2; j++, tmpGx2++, tmpGy2++)
        {
#define CANNY_SHIFT 15
#define  TG22 13573
			MLong s, m;
			x = *tmpGx2;
			y = *tmpGy2;
            s = x ^ y;
            m = _mag[j];
			
            x = ABS(x);
            y = ABS(y);
			if (bIsProtect&&j>pRtProtect->left&&j<pRtProtect->right)
			{
				lowThresEx = lowThres *3/4;
				highThresEx = highThres*3/4;
			}
			else
			{
				lowThresEx = lowThres;
				highThresEx = highThres;
			}
            if( m > lowThresEx )
            {
                MLong tg22x = x * TG22;
                MLong tg67x = tg22x + ((x + x) << CANNY_SHIFT);
				
                y <<= CANNY_SHIFT;
				
                if( y < tg22x )
                {
                    if( m > _mag[j-1] && m >= _mag[j+1] )
                    {
                        if( m > highThresEx && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
                else if( y > tg67x )
                {
                    if( m > _mag[j+magstep2] && m >= _mag[j+magstep1] )
                    {
                        if( m > highThresEx && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
                else
                {
                    s = s < 0 ? -1 : 1;
                    if( m > _mag[j+magstep2-s] && m > _mag[j+magstep1+s] )
                    {
                        if( m > highThresEx && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (MByte)0;
                        continue;
                    }
                }
            }
            prev_flag = 0;
            _map[j] = (MByte)1;
        }
		
        // scroll the ring buffer
        _mag = mag_buf[0];
        mag_buf[0] = mag_buf[1];
        mag_buf[1] = mag_buf[2];
        mag_buf[2] = _mag;
	}
    while( stack_top > stack_bottom )
    {
        MByte* m;
        if( (stack_top - stack_bottom) + 8 > maxsize )
        {
		//	goto EXT;
            MByte** new_stack_bottom=0;
			MLong newSize = MAX( maxsize * 3/2, maxsize + 8 );
            //maxsize = MAX( maxsize * 3/2, maxsize + 8 );
			AllocVectMem(hMemMgr, new_stack_bottom, maxsize, MByte*);
			CpyVectMem(new_stack_bottom, stack_bottom, (stack_top - stack_bottom), MByte*);
            stack_top = new_stack_bottom + (stack_top - stack_bottom);
            FreeVectMem(hMemMgr,stack_bottom);
			maxsize = newSize;
           stack_bottom = new_stack_bottom;
		}
		
        CANNY_POP(m);
		
        if( !m[-1] )
            CANNY_PUSH( m - 1 );
        if( !m[1] )
            CANNY_PUSH( m + 1 );
        if( !m[-mapstep-1] )
            CANNY_PUSH( m - mapstep - 1 );
        if( !m[-mapstep] )
            CANNY_PUSH( m - mapstep );
        if( !m[-mapstep+1] )
            CANNY_PUSH( m - mapstep + 1 );
        if( !m[mapstep-1] )
            CANNY_PUSH( m + mapstep - 1 );
        if( !m[mapstep] )
            CANNY_PUSH( m + mapstep );
        if( !m[mapstep+1] )
            CANNY_PUSH( m + mapstep + 1 );
    }

EXT:
	FreeVectMem(hMemMgr, buffer);
	FreeVectMem(hMemMgr, stack_bottom);
	return res;
}


//Automatic threshold
MRESULT Edge(MHandle hMemMgr,
			 MByte* pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeight,
			 MByte* pEdgeData, MLong lEdgeLine, EDGE_TYPE typeEdge)
{
	MRESULT res=LI_ERR_NONE;
	MDWord dwSum, dwMaxGrad;

	MLong lThres=0;

	MShort* Gx=MNull, *Gy=MNull;
	MLong lGradLine = JMemLength(lWidth);
	AllocVectMem(hMemMgr, Gx, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, Gy, lGradLine*lHeight, MShort);

	Gradient(pSrcData, lSrcLine, Gx, Gy, lGradLine, MNull, 0,
		lWidth, lHeight, &dwSum, &dwMaxGrad, MNull);

	GO(HistoAnalysis(hMemMgr, Gx, Gy, lGradLine, MNull, 0, lWidth, lHeight, dwMaxGrad, 3, &lThres));

	//Absolute threshold
	if (lThres<20)
	{
		//	lThres = 0;
		SetImgMem(pEdgeData, lEdgeLine, lWidth, lHeight, 255, MByte);
		goto EXT;
	}

	switch(typeEdge)
	{
	case TYPE_SOBEL:
		Sobel(Gx, Gy, lGradLine, lWidth, lHeight, pEdgeData, lEdgeLine, lThres);
		break;
	case TYPE_CANNY:

		//
		GO(Canny(hMemMgr, Gx, Gy, lGradLine, lWidth, lHeight, pEdgeData, lEdgeLine, lThres*60/100, lThres));
		break;
	default:
		res=LI_ERR_INVALID_PARAM;
		break;
	}

EXT:
	FreeVectMem(hMemMgr, Gx);
	FreeVectMem(hMemMgr, Gy);
	return res;
}


#define HIST_LENGTH   256
MRESULT HistoAnalysis(MHandle hMemMgr, MShort *Gx, MShort* Gy, MLong lGradLine,
					  MVoid* pSkinMask, MLong lMaskLine,
					  MLong lWidth, MLong lHeight, 
					  MDWord dwMaxGrad, MLong lPercentage,
					  MLong* plHighThres)
{
	MRESULT res=LI_ERR_NONE;
	MLong i, j, lGradExt, lMaskExt;
	
	//MLong hist[HIST_LENGTH]={0};
	MLong *hist=MNull;
	
	MShort *tmpGx, *tmpGy;
	MByte* pTmpMask = (MByte*)pSkinMask;
	
	MLong HistSum, PartialHistSum;
	MDWord factor=0;
	
	MLong lOutLiers=1, lOutLierNum;//1% outliers, 5% edge number
	MBool key=MTrue;
	
	AllocVectMem(hMemMgr, hist, HIST_LENGTH+1, MLong);
	SetVectZero(hist, (HIST_LENGTH+1)*sizeof(MLong));
    if(dwMaxGrad!=0)
        {
            factor=((HIST_LENGTH)<<16)/dwMaxGrad;
    }
    if(dwMaxGrad==0)
    {
        factor=255;
    }
	tmpGx = Gx, tmpGy = Gy;
	lGradExt = lGradLine*2 - (lWidth/2)*2;
	lMaskExt = lMaskLine*2 - (lWidth/2)*2;
	//lGradExt = lGradLine - lWidth;
	//lMaskExt = lMaskLine - lWidth;
	
	HistSum = 0;
	for (j=lHeight/2; j!=0; j--, tmpGx+=lGradExt, tmpGy+=lGradExt, pTmpMask+=lMaskExt)
	{
		for (i=lWidth/2; i!=0; i--, tmpGx+=2, tmpGy+=2, pTmpMask+=2)
		{
			MDWord cur = ABS(*tmpGx)+ABS(*tmpGy);
			JASSERT((cur*factor)>>16 <= HIST_LENGTH);
			if (pSkinMask==MNull||*pTmpMask==255)
			{
				hist[(cur*factor)>>16]++;
				HistSum++;
			}
		}
	}
	
	lOutLierNum = (HistSum*lOutLiers)/100;
	HistSum = (HistSum*lPercentage)/100;
	
	PartialHistSum = 0;
	key = MTrue;
	for (i=HIST_LENGTH-1; i>=0; i--)
	{
		PartialHistSum+=hist[i];
		if (key&&PartialHistSum>=lOutLierNum)
		{
			PartialHistSum = 0;
			key=MFalse;
		}
		if(PartialHistSum>=HistSum)
			break;
	}
	
	if (plHighThres!=MNull)
		*plHighThres = i*dwMaxGrad/HIST_LENGTH;
	
EXT:
	FreeVectMem(hMemMgr, hist);
	return res;
}

/*****************************************************************************
 函 数 名  : NoiseEdge
 功能描述  : 根据噪声等级求CANNY边缘
 输入参数  : MHandle hMemMgr     
             MByte* pSrcData     
             MLong lSrcLine      
             MLong lWidth        
             MLong lHeight       
             MByte* pEdgeData    
             MLong lEdgeLine     
             EDGE_TYPE typeEdge  
             MFloat noisel       
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月23日
    作    者   : hmy
    修改内容   : 新生成函数

*****************************************************************************/
MRESULT NoiseEdge(MHandle hMemMgr,
	MByte* pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeight,
	MByte* pEdgeData, MLong lEdgeLine, EDGE_TYPE typeEdge,MFloat noisel)
{
	MRESULT res=LI_ERR_NONE;
	MDWord dwSum, dwMaxGrad;

	MLong lThres=0;
	MLong lThres1=0;

	MShort* Gx=MNull, *Gy=MNull;
	MLong lGradLine = JMemLength(lWidth);
	AllocVectMem(hMemMgr, Gx, lGradLine*lHeight, MShort);
	AllocVectMem(hMemMgr, Gy, lGradLine*lHeight, MShort);

	Gradient(pSrcData, lSrcLine, Gx, Gy, lGradLine, MNull, 0,
		lWidth, lHeight, &dwSum, &dwMaxGrad, MNull);

	 lThres=(0.5*noisel*noisel+2*noisel+0.1);
	 GO(HistoAnalysis(hMemMgr, Gx, Gy, lGradLine, MNull, 0, lWidth, lHeight, dwMaxGrad, 3, &lThres1));

	//Absolute threshold
	if (lThres<25)
	{
	     GO(Canny(hMemMgr, Gx, Gy, lGradLine, lWidth, lHeight, pEdgeData, lEdgeLine, lThres1*60/100, lThres1));
	}

	else if(lThres>=25)
	{

		switch(typeEdge)
		{
		case TYPE_CANNY:

			//
			GO(Canny(hMemMgr, Gx, Gy, lGradLine, lWidth, lHeight, pEdgeData, lEdgeLine, lThres*40/100, lThres));
			break;
		default:
			res=LI_ERR_INVALID_PARAM;
			break;

		}
}
		if(lThres1<20)
		{
			SetVectMem(pEdgeData,lEdgeLine*lHeight,255,MUInt8);

		}

		
	

EXT:
	FreeVectMem(hMemMgr, Gx);
	FreeVectMem(hMemMgr, Gy);
	return res;
}