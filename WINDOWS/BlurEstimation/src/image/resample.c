#include "limath.h"
#include "limem.h"
#include "liimage.h"
#include "lierrdef.h"
#include <math.h>

#define DESCALE(x,n)	(((x) +  (1 << ((n) - 1))) >> (n))

// resample image, mode=0: nearest neighbor interpolation, mode=1: bilinear interpolation
//MRESULT ResampleImage(MHandle hContext,long srcWidth, long srcHeight, long dstWidth, long dstHeight,
//				  unsigned char *srcImg, unsigned char *dstImg, int mode)
MRESULT ResampleImage(MHandle hMemMgr,long srcWidth, long srcHeight, long srcLine, long dstWidth, long dstHeight,
					  long dstLine, unsigned char *srcImg, unsigned char *dstImg, int mode)
{
	MRESULT res=LI_ERR_NONE;
	unsigned char *lpt, *lptN;
	long srcx, srcy;
	long newx, newy;
	long LineByte, LineByteN;
	
	if (dstWidth==0 || dstHeight==0)
	{
		res=LI_ERR_IMAGE_SIZE_INVALID;
		goto EXT;
		
	}
				
	srcx = srcWidth;
	srcy = srcHeight;	
	//LineByte = (srcWidth*24+31)/32*4;
	//LineByteN = (dstWidth*24+31)/32*4;
	LineByte = srcLine;
	LineByteN = dstLine;
	newx = dstWidth;
	newy = dstHeight;

	lpt = srcImg;
	if(!lpt) {res=LI_ERR_INVALID_PARAM;goto EXT;}
	lptN = dstImg;
	if(!lptN){res=LI_ERR_INVALID_PARAM;goto EXT;};	

	if (srcx==newx && srcy==newy) {
		long lSizeNew = LineByteN * newy;
		//MMemCpy(lptN, lpt, lSizeNew);		
		//JMemCpy(lptN, lpt, lSizeNew);
		JImgMemCpy(lptN, LineByteN, lpt, LineByte, srcx*3, srcy);
		goto EXT;
	}
	
	if (!(srcx>newx && srcy>newy))  // 图像放大的情况
	{
		long scale_xa=(srcx<<10)/newx;
		long scale_ya=(srcy<<10)/newy;
		long half_scale_xa=scale_xa/2;
		long half_scale_ya=scale_ya/2;
		long xmax = newx, width = newx*3, buf_size;
		void* temp_buf = 0;
        long *buf0, *buf1;
        typedef struct {
			long idx;
			long ialpha;
		} stResizeAlpha;
		stResizeAlpha *xofs, *yofs;
		long fxa, fya;
        long sx,sy,dx,dy,k;
		long prev_sy0 = -1, prev_sy1 = -1;                                           

		buf_size = width*2*sizeof(long) + (width + newy)*sizeof(stResizeAlpha);
		//temp_buf = buf0 = (long *)MMemAlloc(hContext,buf_size);
		AllocVectMem(hMemMgr, temp_buf, buf_size, MByte);
		buf0 = (long*)temp_buf;
		//if(!temp_buf) return SERROR_MEMORY;

        buf1 = buf0 + width;
        xofs = (stResizeAlpha*)(buf1 + width);
        yofs = xofs + width;

        for( dx = 0; dx < newx; dx++ )  
		{
			fxa =  dx*scale_xa + half_scale_xa - 512 ;
			sx = (fxa>>10);
			fxa &= 0x03ff;
            if( sx >= srcx-1 ) {
                fxa = 0, sx = srcx-1;
                if( xmax >= newx )
                    xmax = dx;
            } else if( sx < 0 ){
                fxa = 0, sx = 0;
			}
			for( k = 0, sx *= 3; k < 3; k++ )	{
                xofs[dx*3 + k].idx = sx + k,
                xofs[dx*3 + k].ialpha = fxa;
			}
        }
        for( dy = 0; dy < newy; dy++ )  {
            fya = dy*scale_ya + half_scale_ya - 512;
            sy = (fya>>10);
            fya &= 0x03ff;
            if( sy < 0 )
                sy = 0, fya = 0;
            yofs[dy].idx = sy;
            yofs[dy].ialpha = fya;
        }

		xmax *= 3;                                                                 
                                                                                
		for( dy = 0; dy < newy; dy++, lptN += LineByteN )  {                                                                           
			long fy = yofs[dy].ialpha, *swap_t;
			long sy0 = yofs[dy].idx, sy1 = sy0 + (fy > 0 && sy0 < srcy-1);   
                                                                                
			if( sy0 == prev_sy0 && sy1 == prev_sy1 )  {
				k = 2;                                                              
			}else if( sy0 == prev_sy1 ) {                                                                       
				swap_t = buf0, buf0 = buf1, buf1 = swap_t;
				k = 1;                                                              
			}else                                                                    
				k = 0;                                                              
                                                                                
			for( ; k < 2; k++ )  {                                                                       
				long* _buf = k == 0 ? buf0 : buf1;                              
				const unsigned char* _src;                                                
				long sy = k == 0 ? sy0 : sy1;                                        
				if( k == 1 && sy1 == sy0 ) {                                                                   
					//MMemCpy(buf1, buf0,  width*sizeof(buf0[0]) );
					JMemCpy(buf1, buf0, width*sizeof(buf0[0]));
					continue;                                                       
				}                                                                   
				_src = lpt + sy*LineByte;                                            
				for( dx = 0; dx < xmax; dx++ ) {                                                                   
					long sx = xofs[dx].idx;                                          
					long fx = xofs[dx].ialpha;                             
					long t = _src[sx];                                          
					_buf[dx] = (t<<10) + fx*(_src[sx+3] - t);             
				}                                                                   
	            for( ; dx < width; dx++ )                                     
		            _buf[dx] = (_src[xofs[dx].idx]<<10);                   
			}                                                                       
                                                                                
			prev_sy0 = sy0;                                                         
			prev_sy1 = sy1;                                                         
                                                                                
			if( sy0 == sy1 )                                                        
				for( dx = 0; dx < width; dx++ )                               
					lptN[dx] = TRIM_UINT8(DESCALE( buf0[dx]<<10,20) );     
			else                                                                    
				for( dx = 0; dx < width; dx++ )                               
					lptN[dx] = TRIM_UINT8(DESCALE( (buf0[dx]<<10) + fy*(buf1[dx] - buf0[dx]),20) );    
		}                                                                           
		//MMemFree(hMemMgr, temp_buf);                                                                         
		//if(temp_buf) {MMemFree(hContext,temp_buf); temp_buf=MNull;}
		FreeVectMem(hMemMgr, temp_buf);

	}
	else	//high resolution shrink
	{
		if ((srcx / newx) * newx != srcx || 
			(srcy / newy) * newy != srcy)		
		{
			long nWeightXa, nWeightYa;
			long srcx10, srcy10;
			long *naAccu, *naCarry, *naTemp;
			long scale_xa, scale_ya, nScale;
			unsigned char *pSource, *pDest;
			long fEndXa, fEndYa;
			long i, j;		// index for faValue
			long x, y;		// coordinates in source image	
			long u, v = 0; // coordinates in dest image
			
			//naAccu  = (long*)MMemAlloc(hContext,sizeof(long)*(3*newx+3));
			//naCarry = (long*)MMemAlloc(hContext,sizeof(long)*(3*newx+3));
			AllocVectMem(hMemMgr, naAccu, (3*newx+3), MLong);
			AllocVectMem(hMemMgr, naCarry, (3*newx+3), MLong);	
			//MMemSet(naAccu, 0, sizeof(MLong)*(3*newx+3));
			//MMemSet(naCarry, 0, sizeof(MLong)*(3*newx+3));
			SetVectZero(naAccu, sizeof(long)*(3*newx+3));
			SetVectZero(naCarry,  sizeof(long)*(3*newx+3));
			
			pDest = lptN;

			scale_xa = (srcx<<10) / newx;
			scale_ya = (srcy<<10) / newy;
			nScale = (scale_xa * scale_ya)>>10;
			
			fEndYa = scale_ya - 1024;
			srcx10 = srcx<<10;
			srcy10 = srcy<<10;
			
			for (y = 0; y < srcy10; y+=1024)
			{
				pSource = lpt + (y>>10) * LineByte;
				u = i = 0;
				fEndXa = scale_xa - 1024;
				if (y < fEndYa)        // complete source row goes into dest row
				{
					for (x = 0; x < srcx10; x+=1024)
					{
						if (x < fEndXa)       // complete source pixel goes into dest pixel
						{
						//	for (j = 0; j < 3; j++)	naAccu[i + j] += (*pSource++)<<10;
							naAccu[i  ] += (*pSource++)<<10;
							naAccu[i+1] += (*pSource++)<<10;
							naAccu[i+2] += (*pSource++)<<10;
						}
						else        // source pixel is splitted for 2 dest pixels
						{
							nWeightXa = x - fEndXa;
							for (j = 0; j < 3; j++)
							{
								naAccu[i] += (1024 - nWeightXa) * (*pSource);
								naAccu[3 + i++] += nWeightXa * (*pSource++);
							}
							fEndXa += scale_xa;
							u++;
						}
					}
				}
				else       // source row is splitted for 2 dest rows       
				{
					nWeightYa = y - fEndYa;
					for (x = 0; x < srcx10; x+=1024)
					{
						if (x < fEndXa)       // complete source pixel goes into 2 pixel
						{
						//	for (j = 0; j < 3; j++)
						//	{
						//		naAccu[i + j] += (1024 - nWeightYa) * (*pSource);
						//		naCarry[i + j] += nWeightYa * (*pSource++);
						//	}
							naAccu[i  ] += (1024 - nWeightYa) * (*pSource);
							naCarry[i  ] += nWeightYa * (*pSource++);
							naAccu[i+1] += (1024 - nWeightYa) * (*pSource);
							naCarry[i+1] += nWeightYa * (*pSource++);
							naAccu[i+2] += (1024 - nWeightYa) * (*pSource);
							naCarry[i+2] += nWeightYa * (*pSource++);
						} 
						else        // source pixel is splitted for 4 dest pixels
						{
							nWeightXa = x - fEndXa;
							for (j = 0; j < 3; j++) 
							{
								naAccu[i] += ((1024 - nWeightYa) * (1024 - nWeightXa) * (*pSource))>>10;

								*pDest++ = (unsigned char)(naAccu[i] / nScale);
								//*pDest++ = (unsigned char)(((naAccu[i]<<1) +nScale) / (nScale<<1));  //2008.01.28

								naCarry[i] += (nWeightYa * (1024 - nWeightXa) * (*pSource))>>10;
								naAccu[i + 3] += ((1024 - nWeightYa) * nWeightXa * (*pSource))>>10;
								naCarry[i + 3] = (nWeightYa * nWeightXa * (*pSource))>>10;
								i++;
								pSource++;
							}
							fEndXa += scale_xa;
							u++;
						}
					}
					if (u < newx) // possibly not completed due to rounding errors
					{
						for (j = 0; j < 3; j++) *pDest++ = (unsigned char)(naAccu[i++] / nScale);
						//for (j = 0; j < 3; j++) *pDest++ = (unsigned char)(((naAccu[i++]<<1)+nScale) / (nScale<<1));   //2008.01.28
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					//MMemSet(naCarry, 0, sizeof(long) * 3);    // need only to set first pixel zero
					SetVectZero(naCarry, 3*sizeof(MLong));
					pDest = lptN + (++v * LineByteN);
					fEndYa += scale_ya;
				}
			}
			if (v < newy)	// possibly not completed due to rounding errors
			{
				for (i = 0; i < 3 * newx; i++) 
					*pDest++ = (unsigned char)(naAccu[i] / nScale);
			}

			//MMemFree(hMemMgr, naAccu);
			//MMemFree(hMemMgr, naCarry);
			//if(naAccu) {MMemFree(hContext,naAccu); naAccu=MNull;}
			//if(naCarry) {MMemFree(hContext,naCarry); naCarry=MNull;}
			FreeVectMem(hMemMgr, naAccu);
			FreeVectMem(hMemMgr, naCarry);
		}
		else	// scale integer multiple
		{
			long scale_xa, scale_ya, nScale;
			long fEndXa, fEndYa;
			unsigned char *pSource, *pDest;
			long *naAccu;
			long x, y, i, j;
			
			//naAccu  = (long*)MMemAlloc(hContext,sizeof(long) * 3 * newx);
			AllocVectMem(hMemMgr, naAccu, 3*newx, MLong);
			//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
			SetVectZero(naAccu, sizeof(long)*3*newx);

			pDest = lptN;
			scale_xa = srcx / newx;
			scale_ya = srcy / newy;
			nScale = scale_xa * scale_ya;
			fEndYa = scale_ya;

			for (y = 0; y < srcy; y++)
			{
				pSource = lpt + y * LineByte;
				i = 0;
				fEndXa = scale_xa;

				if (y >= fEndYa)
				{
					for(x = 0, j = 0; x < newx; x++, j+=3)
					{
						pDest[j  ] = (unsigned char)(naAccu[j  ] / nScale);	
						pDest[j+1] = (unsigned char)(naAccu[j+1] / nScale);
						pDest[j+2] = (unsigned char)(naAccu[j+2] / nScale);
					}
					
					pDest += LineByteN;
					//MMemSet(naAccu, 0, sizeof(MLong) * 3 * newx);
					//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
					SetVectZero(naAccu, sizeof(MLong)*3*newx);
					fEndYa += scale_ya;
				}

				for (x = 0; x < srcx; x++)
				{
					if (x >= fEndXa)
					{
						fEndXa += scale_xa;
						i+=3;
					}
					naAccu[i    ] += *pSource++;
					naAccu[i + 1] += *pSource++;
					naAccu[i + 2] += *pSource++;
				}
			}

			//2008.02.22 fix bug
			// set value for the last line 
			for(x = 0, j = 0; x < newx; x++, j+=3)
			{
				pDest[j  ] = (unsigned char)(naAccu[j  ] / nScale);	
				pDest[j+1] = (unsigned char)(naAccu[j+1] / nScale);
				pDest[j+2] = (unsigned char)(naAccu[j+2] / nScale);
			}			
			//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
			SetVectZero(naAccu, sizeof(MLong)*3*newx);


//#ifdef _DEBLUR_DEBUG_
//			SaveToBMP("c:\\deblur.bmp", lptN, newx, newy, 24);
//#endif	// _DEBLUR_DEBUG_

			//MMemFree(hMemMgr, naAccu);
			//if(naAccu) {MMemFree(hContext,naAccu); naAccu=MNull;}
			FreeVectMem(hMemMgr, naAccu);
		}
	}
EXT:
	return res;
}


// resample single-channel image
// bilinear interpolation
// 1999 Steve McMahon (steve@dogma.demon.co.uk)
MRESULT ResampleChannelImage(long srcHei, long srcWid, long srcPitch, unsigned char *srcImg,
						 long dstHei, long dstWid, long dstPitch, unsigned char *dstImg, long mode)
{
	MRESULT res=LI_ERR_NONE;
	unsigned char g, g1, g2, g3, g4;
	long x, y, pt, pt1;
	long ifX, ifY, ifX1, ifY1, xmax, ymax;

	//float ig1, ig2, dx, dy;
	long ig1, ig2, dx, dy; //?		

	//float xScale, yScale, fX, fY;
	long xScale, yScale, fX, fY;  //?
	unsigned char *iDst;

	if (dstWid==0 || dstHei==0) return res=LI_ERR_INVALID_PARAM;

	if (srcWid==dstWid && srcHei==dstWid)
	{
		for(y=0; y<dstHei; y++)
		{
			pt = y*dstPitch;
			pt1 = y*srcPitch;
			for(x=0; x<dstWid; x++)
			{
				dstImg[pt] = srcImg[pt1];
				pt++;
				pt1++;
			}
		}
		return res;
	}

	//xScale = (float)srcWid / (float)dstWid;  
	//yScale = (float)srcHei / (float)dstHei;  
	xScale = ((srcWid<<11) +dstWid) / (dstWid<<1);  //?
	yScale = ((srcHei<<11) +dstHei) / (dstHei<<1);  //?
	
	xmax = srcWid-1;
	ymax = srcHei-1;
	for(y=0; y<dstHei; y++)
	{
		pt = y*dstPitch;

		fY = y * yScale;		// mulitply 1<<10

		//ifY = (int)fY;
		ifY= fY>>10;
		//ifY = ((fY>>10)<<10);	//multiply 1<<10

		ifY1 = MIN(ymax, ifY+1);

		//dy = fY - ifY;
		dy = fY-(ifY<<10);

		for(x=0; x<dstWid; x++)
		{
			fX = x * xScale;	//multiply 1<<10

			//ifX = (int)fX;
			ifX = fX>>10;

			ifX1 = MIN(xmax, ifX+1);

			//dx = fX - ifX;
			dx = fX-(ifX<<10);
			

			iDst = srcImg + ifY*srcPitch + ifX;
			g1 = *iDst;

			iDst = srcImg + ifY*srcPitch + ifX1;
			g2 = *iDst;

			iDst = srcImg + ifY1*srcPitch + ifX;
			g3= *iDst;

			iDst = srcImg + ifY1*srcPitch + ifX1;
			g4 = *iDst;
			
			// Interplate in x direction:
			//ig1 = g1 * (1 - dy) + g3 * dy;			
			//ig2 = g2 * (1 - dy) + g4 * dy;
			ig1 = g1 * (1024 - dy) + g3 * dy;			//ig1 multiply 1<<10
			ig2 = g2 * (1024 - dy) + g4 * dy;			//ig2 multiply 1<<10
			
			// Interpolate in y:
			//g = (unsigned char)(ig1 * (1 - dx) + ig2 * dx);
			g = (unsigned char)(((ig1 * (1024 - dx) + ig2 * dx)+1024)>>20);
			
			// Set output
			dstImg[pt++] = g;				
		}
	}
	
	return res;	
}

MRESULT ResampleImage_YUV(MHandle hContext, unsigned char *srcImg, long srcWidth, long srcHeight,
								unsigned char *dstImg, long dstWidth, long dstHeight)
{
	MRESULT res=LI_ERR_NONE;
	unsigned char *lpt, *lptN;
	long srcx, srcy;
	long newx, newy;
	long LineByte, LineByteN;
	
	if (dstWidth==0 || dstHeight==0)
		return res=LI_ERR_INVALID_PARAM;	
				
	srcx = srcWidth;
	srcy = srcHeight;	
	LineByte = srcWidth;//LineByte = (srcWidth*24+31)/32*4;
	LineByteN = dstWidth;//LineByteN = (dstWidth*24+31)/32*4;
	newx = dstWidth;
	newy = dstHeight;

	lpt = srcImg;
	if(!lpt) return res=LI_ERR_INVALID_PARAM;
	lptN = dstImg;
	if(!lptN) return res=LI_ERR_INVALID_PARAM;	

	if (srcx==newx && srcy==newy) {
		long lSizeNew = LineByteN * newy;
		//MMemCpy(lptN, lpt, lSizeNew);		
		JImgMemCpy(lptN, LineByteN, lpt, LineByte, srcx*2, srcy);
		//JMemCpy(lptN, lpt, lSizeNew);
		return res;
	}
	
	if (!(srcx>newx && srcy>newy))  // 图像放大的情况
	{
		long scale_xa=(srcx<<10)/newx;
		long scale_ya=(srcy<<10)/newy;
		long half_scale_xa=scale_xa/2;
		long half_scale_ya=scale_ya/2;
		long xmax = newx, width = newx, buf_size;//, width = newx*3
		void* temp_buf = 0;
        long *buf0, *buf1;
        typedef struct {
			long idx;
			long ialpha;
		} stResizeAlpha;
		stResizeAlpha *xofs, *yofs;
		long fxa, fya;
        long sx,sy,dx,dy,k;
		long prev_sy0 = -1, prev_sy1 = -1;                                           

		buf_size = width*2*sizeof(long) + (width + newy)*sizeof(stResizeAlpha);
		//temp_buf = buf0 = (long *)MMemAlloc(hContext,buf_size);
		AllocVectMem(hContext, buf0, buf_size, MByte);
		temp_buf = buf0;

        buf1 = buf0 + width;
        xofs = (stResizeAlpha*)(buf1 + width);
        yofs = xofs + width;

        for( dx = 0; dx < newx; dx++ )  
		{
			fxa =  dx*scale_xa + half_scale_xa - 512 ;
			sx = (fxa>>10);
			fxa &= 0x03ff;
            if( sx >= srcx-1 ) {
                fxa = 0, sx = srcx-1;
                if( xmax >= newx )
                    xmax = dx;
            } else if( sx < 0 ){
                fxa = 0, sx = 0;
			}
			for( k = 0, sx *= 3; k < 1; k++ )	{//for( k = 0, sx *= 3; k < 3; k++ )	{
                xofs[dx*3 + k].idx = sx + k,
                xofs[dx*3 + k].ialpha = fxa;
			}
        }
        for( dy = 0; dy < newy; dy++ )  {
            fya = dy*scale_ya + half_scale_ya - 512;
            sy = (fya>>10);
            fya &= 0x03ff;
            if( sy < 0 )
                sy = 0, fya = 0;
            yofs[dy].idx = sy;
            yofs[dy].ialpha = fya;
        }

		//xmax *= 3;                                                                 
                                                                                
		for( dy = 0; dy < newy; dy++, lptN += LineByteN )  {                                                                           
			long fy = yofs[dy].ialpha, *swap_t;
			long sy0 = yofs[dy].idx, sy1 = sy0 + (fy > 0 && sy0 < srcy-1);   
                                                                                
			if( sy0 == prev_sy0 && sy1 == prev_sy1 )  {
				k = 2;                                                              
			}else if( sy0 == prev_sy1 ) {                                                                       
				swap_t = buf0, buf0 = buf1, buf1 = swap_t;
				k = 1;                                                              
			}else                                                                    
				k = 0;                                                              
                                                                                
			for( ; k < 2; k++ )  {                                                                       
				long* _buf = k == 0 ? buf0 : buf1;                              
				const unsigned char* _src;                                                
				long sy = k == 0 ? sy0 : sy1;                                        
				if( k == 1 && sy1 == sy0 ) {                                                                   
					//MMemCpy(buf1, buf0,  width*sizeof(buf0[0]) );
					JMemCpy(buf1, buf0,  width*sizeof(buf0[0]) );
					continue;                                                       
				}                                                                   
				_src = lpt + sy*LineByte;                                            
				for( dx = 0; dx < xmax; dx++ ) {                                                                   
					long sx = xofs[dx].idx;                                          
					long fx = xofs[dx].ialpha;                             
					long t = _src[sx];                                          
					_buf[dx] = (t<<10) + fx*(_src[sx+3] - t);             
				}                                                                   
	            for( ; dx < width; dx++ )                                     
		            _buf[dx] = (_src[xofs[dx].idx]<<10);                   
			}                                                                       
                                                                                
			prev_sy0 = sy0;                                                         
			prev_sy1 = sy1;                                                         
                                                                                
			if( sy0 == sy1 )                                                        
				for( dx = 0; dx < width; dx++ )                               
					lptN[dx] = TRIM_UINT8(DESCALE( buf0[dx]<<10,20) );     
			else                                                                    
				for( dx = 0; dx < width; dx++ )                               
					lptN[dx] = TRIM_UINT8(DESCALE( (buf0[dx]<<10) + fy*(buf1[dx] - buf0[dx]),20) );    
		}                                                                           
                             
		//if(temp_buf) {MMemFree(hContext,temp_buf); temp_buf=MNull;}
		FreeVectMem(hContext, temp_buf);

	}
	else	//high resolution shrink
	{
		if ((srcx / newx) * newx != srcx || 
			(srcy / newy) * newy != srcy)		
		{
			long nWeightXa, nWeightYa;
			long srcx10, srcy10;
			long *naAccu, *naCarry, *naTemp;
			long scale_xa, scale_ya, nScale;
			unsigned char *pSource, *pDest;
			long fEndXa, fEndYa;
			long i, j;		// index for faValue
			long x, y;		// coordinates in source image	
			long u, v = 0; // coordinates in dest image
			
			//naAccu  = (long*)MMemAlloc(hContext,sizeof(long)*(newx+1));//MMemAlloc(hContext,sizeof(long)*(3*newx+3));
			//naCarry = (long*)MMemAlloc(hContext,sizeof(long)*(newx+1));//MMemAlloc(hContext,sizeof(long)*(3*newx+3));
			AllocVectMem(hContext, naAccu, newx+1, long);
			AllocVectMem(hContext, naCarry,newx+1, long);
//			if(!naAccu || !naCarry) {
//				if(naAccu) {MMemFree(hContext,naAccu); naAccu=MNull;}
//				if(naCarry) {MMemFree(hContext,naCarry); naCarry =MNull;}
//				return SERROR_MEMORY;
//			}		

//			MMemSet(naAccu, 0, sizeof(long)*(newx+1));//MMemSet(naAccu, 0, sizeof(long)*(3*newx+3));
//			MMemSet(naCarry, 0, sizeof(long)*(newx+1));//MMemSet(naCarry, 0, sizeof(long)*(3*newx+3));
			SetVectZero(naAccu,  sizeof(long)*(newx+1) );
			SetVectZero(naCarry,  sizeof(long)*(newx+1) );
			
			pDest = lptN;

			scale_xa = (srcx<<10) / newx;
			scale_ya = (srcy<<10) / newy;
			nScale = (scale_xa * scale_ya)>>10;
			
			fEndYa = scale_ya - 1024;
			srcx10 = srcx<<10;
			srcy10 = srcy<<10;
			
			for (y = 0; y < srcy10; y+=1024)
			{
				pSource = lpt + (y>>10) * LineByte;
				u = i = 0;
				fEndXa = scale_xa - 1024;
				if (y < fEndYa)        // complete source row goes into dest row
				{
					for (x = 0; x < srcx10; x+=1024)
					{
						if (x < fEndXa)       // complete source pixel goes into dest pixel
						{
							naAccu[i  ] += (*pSource++)<<10;
							//naAccu[i+1] += (*pSource++)<<10;
							//naAccu[i+2] += (*pSource++)<<10;
						}
						else        // source pixel is splitted for 2 dest pixels
						{
							nWeightXa = x - fEndXa;
							for (j = 0; j < 1; j++)//for (j = 0; j < 3; j++)
							{
								naAccu[i] += (1024 - nWeightXa) * (*pSource);
								naAccu[1 + i++] += nWeightXa * (*pSource++);//naAccu[3 + i++] += nWeightXa * (*pSource++);
							}
							fEndXa += scale_xa;
							u++;
						}
					}
				}
				else       // source row is splitted for 2 dest rows       
				{
					nWeightYa = y - fEndYa;
					for (x = 0; x < srcx10; x+=1024)
					{
						if (x < fEndXa)       // complete source pixel goes into 2 pixel
						{
							naAccu[i  ] += (1024 - nWeightYa) * (*pSource);
							naCarry[i  ] += nWeightYa * (*pSource++);
							//naAccu[i+1] += (1024 - nWeightYa) * (*pSource);
							//naCarry[i+1] += nWeightYa * (*pSource++);
							//naAccu[i+2] += (1024 - nWeightYa) * (*pSource);
							//naCarry[i+2] += nWeightYa * (*pSource++);
						} 
						else        // source pixel is splitted for 4 dest pixels
						{
							nWeightXa = x - fEndXa;
							for (j = 0; j < 1; j++)//for (j = 0; j < 3; j++) 
							{
								naAccu[i] += ((1024 - nWeightYa) * (1024 - nWeightXa) * (*pSource))>>10;
								*pDest++ = (unsigned char)(naAccu[i] / nScale);

								naCarry[i] += (nWeightYa * (1024 - nWeightXa) * (*pSource))>>10;
								naAccu[i+1] += ((1024 - nWeightYa) * nWeightXa * (*pSource))>>10;//naAccu[i+3] += ((1024 - nWeightYa) * nWeightXa * (*pSource))>>10;
								naCarry[i+1] = (nWeightYa * nWeightXa * (*pSource))>>10;//naCarry[i+3] = (nWeightYa * nWeightXa * (*pSource))>>10;
								i++;
								pSource++;
							}
							fEndXa += scale_xa;
							u++;
						}
					}
					if (u < newx) // possibly not completed due to rounding errors
					{
						for (j = 0; j < 1; j++) //for (j = 0; j < 3; j++)
							*pDest++ = (unsigned char)(naAccu[i++] / nScale);
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					//MMemSet(naCarry, 0, sizeof(long));    //MMemSet(naCarry, 0, sizeof(long) * 3); need only to set first pixel zero
					SetVectZero(naCarry, sizeof(long));
					pDest = lptN + (++v * LineByteN);
					fEndYa += scale_ya;
				}
			}
			if (v < newy)	// possibly not completed due to rounding errors
			{
				for (i = 0; i < newx; i++) //for (i = 0; i < 3 * newx; i++) 
					*pDest++ = (unsigned char)(naAccu[i] / nScale);
			}

			//if(naAccu) {MMemFree(hContext,naAccu); naAccu=MNull;}
			//if(naCarry) {MMemFree(hContext,naCarry); naCarry=MNull;}
			FreeVectMem(hContext, naAccu);
			FreeVectMem(hContext, naCarry);
		}
		else	// scale integer multiple
		{
			long scale_xa, scale_ya, nScale;
			long fEndXa, fEndYa;
			unsigned char *pSource, *pDest;
			long *naAccu;
			long x, y, i, j;
			
			//naAccu  = (long*)MMemAlloc(hContext,sizeof(long)*newx);//MMemAlloc(hContext,sizeof(long) * 3 * newx);
			AllocVectMem(hContext, naAccu, newx, long);
			//if(!naAccu)	return SERROR_MEMORY;
			//MMemSet(naAccu, 0, sizeof(long)*newx);//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
			SetVectZero(naAccu, newx*sizeof(long));

			pDest = lptN;
			scale_xa = srcx / newx;
			scale_ya = srcy / newy;
			nScale = scale_xa * scale_ya;
			fEndYa = scale_ya;

			for (y = 0; y < srcy; y++)
			{
				pSource = lpt + y * LineByte;
				i = 0;
				fEndXa = scale_xa;

				if (y >= fEndYa)
				{
					for(x = 0, j = 0; x < newx; x++, j++)//for(x = 0, j = 0; x < newx; x++, j+=3)
					{
						pDest[j] = (unsigned char)(naAccu[j  ] / nScale);	
						//pDest[j+1] = (unsigned char)(naAccu[j+1] / nScale);
						//pDest[j+2] = (unsigned char)(naAccu[j+2] / nScale);
					}
					
					pDest += LineByteN;
					//MMemSet(naAccu, 0, sizeof(long)*newx);//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
					SetVectZero(naAccu, sizeof(long)*newx);
					fEndYa += scale_ya;
				}

				for (x = 0; x < srcx; x++)
				{
					if (x >= fEndXa)
					{
						fEndXa += scale_xa;
						i++;//i+=3;
					}
					naAccu[i] += *pSource++;
					//naAccu[i + 1] += *pSource++;
					//naAccu[i + 2] += *pSource++;
				}
			}

			//2008.02.22 fix bug
			// set value for the last line 
			for(x = 0, j = 0; x < newx; x++, j++)//for(x = 0, j = 0; x < newx; x++, j+=3)
			{
				pDest[j] = (unsigned char)(naAccu[j] / nScale);	
				//pDest[j+1] = (unsigned char)(naAccu[j+1] / nScale);
				//pDest[j+2] = (unsigned char)(naAccu[j+2] / nScale);
			}			
			//MMemSet(naAccu, 0, sizeof(long)*newx);//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);
			SetVectZero(naAccu, sizeof(long)*newx);//MMemSet(naAccu, 0, sizeof(long) * 3 * newx);

			//MMemFree(hMemMgr, naAccu);
			//if(naAccu) {MMemFree(hContext,naAccu); naAccu=MNull;}
			FreeVectMem(hContext, naAccu);
		}
	}
EXT:
	return res;
}
