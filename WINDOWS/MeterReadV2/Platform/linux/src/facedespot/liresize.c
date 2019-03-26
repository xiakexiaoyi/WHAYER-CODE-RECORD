#include "liresize.h"

#include "limem.h"


MVoid Resize(MVoid* pSrc, MLong lSrcLine, 
			 MLong lSrcWidth, MLong lSrcHeight, 
			 MVoid* pRlt, MLong lRltLine,
			 MLong lRltWidth, MLong lRltHeight,
			 INTERPOLATE type)
{
	MLong lNewWidth = lRltWidth;
	MLong lNewHeight = lRltHeight;
	MLong lOldWidth = lSrcWidth;
	MLong lOldHeight = lSrcHeight;
	MLong x, y, i, j;
	
	if (lNewHeight==lOldHeight && lNewWidth==lOldWidth)
	{
		CpyImgMem2(pRlt, lRltLine, pSrc, lSrcLine, lOldWidth, lOldHeight);
	}
	else if (type == BILINEAR)
	{
		MDouble fraction_x, fraction_y, one_minus_x, one_minus_y;
		MDouble nXFactor = lOldWidth*1.0/lNewWidth;
		MDouble nYFactor = lOldHeight*1.0/lNewHeight;
		MLong ceil_x, ceil_y, floor_x, floor_y;
		MByte c1, c2, c3, c4;
		MByte b1, b2;
		MByte *pTmpSrc = (MByte*)pSrc;
		MByte *pTmpRlt = (MByte*)pRlt;
		MLong lRltExt = lRltLine - lNewWidth;
		for (y=0; y<lNewHeight; y++, pTmpRlt+=lRltExt)
		{
			for (x=0; x<lNewWidth; x++, pTmpRlt++)
			{
				floor_x = (MLong)(x*nXFactor);
				floor_y = (MLong)(y*nYFactor);
                ceil_x = floor_x==lOldWidth-1 ? 0 : 1;
                ceil_y = floor_y==lOldHeight-1 ? 0 : 1;
				fraction_x = x * nXFactor - floor_x;
				fraction_y = y * nYFactor - floor_y;
				one_minus_x = 1.0 - fraction_x;
				one_minus_y = 1.0 - fraction_y;
				
				pTmpSrc = (MByte*)pSrc+lSrcLine*floor_y+floor_x;
				c1 = *pTmpSrc;
				c2 = *(pTmpSrc + ceil_x);
				c3 = *(pTmpSrc + lSrcLine*ceil_y);
				c4 = *(pTmpSrc + lSrcLine*ceil_y + ceil_x);

				b1 = (MByte)(one_minus_x*c1+fraction_x*c2);
				b2 = (MByte)(one_minus_x*c3+fraction_x*c4);
				*(pTmpRlt) = (MByte)(one_minus_y*b1+fraction_y*b2);
			}
		}
	}
	else if (type == BICUBIC)
	{

	}
	else if (type == POINTSAMPLE)
	{
		MLong lRltExt;
		MLong lXDelta = ((lOldWidth<<16)+(1<<16)/2)/lNewWidth;
		MLong lYDelta = ((lOldHeight<<16)+(1<<16)/2)/lNewHeight;
		MByte *pTmpSrc, *pTmpRlt;
		{
			y=0;
			pTmpSrc = (MByte*)pSrc, pTmpRlt = (MByte*)pRlt;
			lRltExt = lRltLine-lNewWidth;

			for (j=0; j<lNewHeight; j++, pTmpRlt+=lRltExt,y+=lYDelta)
			{
				pTmpSrc = (MByte*)pSrc + (y>>16)*lSrcLine;
				for (x = 0,i=0; i<lNewWidth; i++, pTmpRlt++,x+=lXDelta)
				{
					*pTmpRlt = pTmpSrc[x>>16];					
				}
			}		
		}
	}
	return;
}

MVoid Reduce(MVoid* pSrc, MLong lSrcLine, 
			 MLong lSrcWidth, MLong lSrcHeight, 
			 MVoid* pRlt, MLong lRltLine,
			 MLong lRltWidth, MLong lRltHeight)
{
	MLong lNewWidth = lRltWidth;
	MLong lNewHeight = lRltHeight;
	MLong lOldWidth = lSrcWidth;
	MLong lOldHeight = lSrcHeight;
	MLong x, y, i, j;
	MLong lRltExt;
	MLong lXDelta = (lOldWidth<<16)/lNewWidth;
	MLong lYDelta = (lOldHeight<<16)/lNewHeight;
	MByte *pTmpSrc, *pTmpRlt;
	y=0;
	pTmpSrc = (MByte*)pSrc, pTmpRlt = (MByte*)pRlt;
	lRltExt = lRltLine-lNewWidth;
	
	for (j=0; j<lNewHeight; j++, pTmpRlt+=lRltExt,y+=lYDelta)
	{
		pTmpSrc = (MByte*)pSrc + (y>>16)*lSrcLine;
		for (x = 0,i=0; i<lNewWidth; i++, pTmpRlt++,x+=lXDelta)
		{
			*pTmpRlt = pTmpSrc[x>>16];					
		}
	}		
}


//
MVoid MapToOrigin(MVoid* pSrc, MLong lSrcLine, 
			 MLong lSrcWidth, MLong lSrcHeight, 
			 MVoid* pRlt, MLong lRltLine,
			 MLong lRltWidth, MLong lRltHeight, MByte flag)
{
	MLong x, y, i, j;
	MLong lRltExt, lSrcExt;
	MLong lXReduced = ((lRltWidth<<16)+(1<<16)/2)/lSrcWidth;
	MLong lYReduced = ((lRltHeight<<16)+(1<<16)/2)/lSrcHeight;
	MByte *pTmpSrc, *pTmpRlt;

	if (lSrcWidth == lRltWidth && lSrcHeight==lRltHeight)
	{
		CpyImgMem2(pRlt, lRltLine, pSrc, lSrcLine, lSrcWidth, lSrcHeight);
		return;
	}

	y=0;
	pTmpSrc = (MByte*)pSrc, pTmpRlt = (MByte*)pRlt;
	lRltExt = lRltLine-lRltWidth;
	lSrcExt = lSrcLine-lSrcWidth;

	for (j=0; j<lSrcHeight; j++, pTmpSrc+=lSrcExt)
	{
		for (i=0; i<lSrcWidth; i++, pTmpSrc++)
		{
			MLong  orgX, orgY;
			if (*pTmpSrc!=flag)continue;
			//map
			orgX = (lXReduced*i)>>16;
			orgY = (lYReduced*j)>>16;
			pTmpRlt = (MByte*)pRlt + (orgY+1)*lRltLine+orgX+1;
			lRltExt = lRltLine-(lXReduced>>16)-1;
			for (y=0; y<(lYReduced>>16)+1; y++, pTmpRlt+=lRltExt)
			{
				for (x=0; x<(lXReduced>>16)+1; x++, pTmpRlt++)
				{
					*pTmpRlt = flag;
				}
			}
		}
	}
}
