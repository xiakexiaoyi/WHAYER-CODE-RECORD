#include "snake.h"
#include "lidebug.h"
#include "liedge.h"
#include <math.h>
#include "limath.h"
#define  _SNAKE_BIG 2.e+38f
MVoid SnakeOpenRegion(BLOCK*pImage, MPOINT *pPoint,MVoid *pMem,MLong lNum,SnakeParam param,SnakeTermCriteria criteria)
{
	MLong i,j,k;
	MPOINT *pSourcePoint=(MPOINT*)pMem;
	MLong flagConverged=0;
	MLong diffx,diffy;
	MLong left,right,bottom,upper;
	MFloat Econt[MAX_WIDTH*MAX_HEIGHT],Ecurv[MAX_WIDTH*MAX_HEIGHT],Eimg[MAX_WIDTH*MAX_HEIGHT],E[MAX_WIDTH*MAX_HEIGHT];
	MFloat maxEcont,minEcont;
	MFloat maxEcurv,minEcurv;
	MFloat maxEimg,minEimg;
	MFloat Emin;
	MFloat energy,tmp;
	MLong neighbors = param.lWidth * param.lHeight;
	MLong tx,ty;
	MUInt8 *pData=(MUInt8*)(pImage->pBlockData);
	MLong moved=0;
	MLong iteration=0;

	MLong offsetx,offsety;

	MLong centerx = param.lWidth >> 1;
	MLong centery = param.lHeight >> 1;
	
	MDouble ave_d=0;
	for (i=0;i<lNum;i++)
	{
		pSourcePoint[i]=pPoint[i];
	}
	while (!flagConverged)
	{
		//calculate the average of distance 
		moved=0;
		iteration++;
		for (i=1;i<lNum;i++)
		{
			diffx = pPoint[i - 1].x - pPoint[i].x;
			diffy = pPoint[i - 1].y - pPoint[i].y;

			ave_d+=sqrt((MFloat)(diffx*diffx+diffy*diffy));
		}
		ave_d=ave_d/(lNum-1);


		for (i=1;i<lNum-1;i++)
		{
			/* Calculate Econt */
			left = MIN( pSourcePoint[i].x, param.lWidth >> 1 );
			right = MIN( pImage->lWidth - 1 - pSourcePoint[i].x, param.lWidth >> 1 );
			upper = MIN( pSourcePoint[i].y, param.lHeight >> 1 );
			bottom = MIN( pImage->lHeight - 1 - pSourcePoint[i].y, param.lHeight >> 1 );

			maxEcont = 0;
			minEcont = _SNAKE_BIG;
			for( j = -upper; j <= bottom; j++ )
			{
				for( k = -left; k <= right; k++ )
				{
					if( i == 0 )
					{
						//diffx = pt[n - 1].x - (pt[i].x + k);
						//diffy = pt[n - 1].y - (pt[i].y + j);
						diffx=0;
						diffy=0;
					}
					else
					{
						diffx = pPoint[i - 1].x - (pSourcePoint[i].x + k);
						diffy = pPoint[i - 1].y - (pSourcePoint[i].y + j);
					}
					Econt[(j + centery) * param.lWidth + k + centerx] = energy =
						(MFloat) fabs( ave_d -
						sqrt( (MFloat) (diffx * diffx + diffy * diffy) ));

					maxEcont = MAX( maxEcont, energy );
					minEcont = MIN( minEcont, energy );
				}
			}
			tmp = maxEcont - minEcont;
			tmp = (tmp == 0) ? 0 : (1 / tmp);
			for( k = 0; k < neighbors; k++ )
			{
				Econt[k] = (Econt[k] - minEcont) * tmp;
			}

			/*  Calculate Ecurv */
			maxEcurv = 0;
			minEcurv = _SNAKE_BIG;

			for( j = -upper; j <= bottom; j++ )
			{
				for( k = -left; k <= right; k++ )
				{
					if( i == 0 )
					{
						//tx = pt[n - 1].x - 2 * (pt[i].x + k) + pt[i + 1].x;
						//ty = pt[n - 1].y - 2 * (pt[i].y + j) + pt[i + 1].y;
						tx=0;
						ty=0;
					}
					else if( i == lNum - 1 )
					{
						//tx = pt[i - 1].x - 2 * (pt[i].x + k) + pt[0].x;
						//ty = pt[i - 1].y - 2 * (pt[i].y + j) + pt[0].y;
						tx=0;
						ty=0;
					}
					else
					{
						tx = pPoint[i - 1].x - 2 * (pSourcePoint[i].x + k) + pPoint[i + 1].x;
						ty = pPoint[i - 1].y - 2 * (pSourcePoint[i].y + j) + pPoint[i + 1].y;
					}
					Ecurv[(j + centery) * param.lWidth + k + centerx] = energy =
						(float) ((tx * tx + ty * ty));
					maxEcurv = MAX( maxEcurv, energy );
					minEcurv = MIN( minEcurv, energy );
				}
			}
			tmp = maxEcurv - minEcurv;
			tmp = (tmp == 0) ? 0 : (1 / tmp);
			for( k = 0; k < neighbors; k++ )
			{
				Ecurv[k] = (Ecurv[k] - minEcurv) * tmp;
			}

			/* Calculate Eimg */
			maxEimg=0;
			minEimg=_SNAKE_BIG;
			for( j = -upper; j <= bottom; j++ )
			{
				for( k = -left; k <= right; k++ )
				{
					Eimg[(j + centery) * param.lWidth + k + centerx] = energy =
						pData[(pSourcePoint[i].y + j) * (pImage->lBlockLine) + pSourcePoint[i].x + k];
					maxEimg = MAX( maxEimg, energy );
					minEimg = MIN( minEimg, energy );
				}
			}
			tmp = (maxEimg - minEimg);
			tmp = (tmp == 0) ? 0 : (1 / tmp);

			for( k = 0; k < neighbors; k++ )
			{
				Eimg[k] = ( Eimg[k]-maxEimg) * tmp;
			}

			/* Find Minimize point in the neighbors */
			for( k = 0; k < neighbors; k++ )
			{
				E[k] = param.alpha * Econt[k] + param.beta * Ecurv[k] + param.gamma * Eimg[k];
			}
			Emin = _SNAKE_BIG;
			for( j = -upper; j <= bottom; j++ )
			{
				for( k = -left; k <= right; k++ )
				{

					if( E[(j + centery) * param.lWidth + k + centerx] < Emin )
					{
						Emin = E[(j + centery) * param.lWidth + k + centerx];
						offsetx = k;
						offsety = j;
					}
				}
			}

			if( offsetx || offsety )
			{
				pPoint[i].x =pSourcePoint[i].x+offsetx;
				pPoint[i].y =pSourcePoint[i].y+offsety;
				moved++;
			}

		}
		flagConverged = (moved == 0);
		if( (criteria.type & TERMCRIT_ITER) && (iteration >= criteria.max_iter) )
			flagConverged = 1;
		if( (criteria.type & TERMCRIT_EPS) && (moved <= criteria.epsilon) )
			flagConverged = 1;

	}
}