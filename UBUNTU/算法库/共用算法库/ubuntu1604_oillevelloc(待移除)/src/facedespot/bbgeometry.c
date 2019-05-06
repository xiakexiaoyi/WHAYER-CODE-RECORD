#include <math.h>

#include "lierrdef.h"
#include "lidebug.h"
#include "limath.h"
#include "bbgeometry.h"
#include "liimage.h"


///获取两个rectangle较大的重叠率, 返回百分比
MVoid vGetOverLappingRate(MRECT rt1, MRECT rt2, MLong *minRate, MLong *maxRate)
{
	MLong lSizeRt1, lSizeRt2;
	MLong lMaxYDist = MAX(rt2.bottom,rt1.bottom)-MIN(rt2.top,rt1.top);
	MLong lMaxXDist = MAX(rt2.right,rt1.right)-MIN(rt2.left,rt1.left);
	if (!minRate && !maxRate) return;

	lSizeRt1 = (rt1.bottom-rt1.top)*(rt1.right-rt1.left);
	lSizeRt2 = (rt2.bottom-rt2.top)*(rt2.right-rt2.left);

	if (lSizeRt1<=0 || lSizeRt2 <=0) 
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 100;
	}

	///如果两个矩形有交集，则距离为0
	if (lMaxXDist<= (rt2.right-rt2.left+rt1.right-rt1.left)
	&& lMaxYDist <= (rt2.bottom-rt2.top+rt1.bottom-rt1.top))
	{

		MLong lSizeOverlap = (MIN(rt2.right, rt1.right)-MAX(rt2.left, rt1.left))
						*(MIN(rt2.bottom,rt1.bottom)-MAX(rt2.top, rt1.top));
		if (minRate) *minRate = MIN(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
		if (maxRate) *maxRate = MAX(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
	}
	else
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 0;
	}
	return;
	
}
MLong vDistance1L(MPOINT Pt1, MPOINT Pt2)
{
	return ABS(Pt1.x-Pt2.x)+ABS(Pt1.y-Pt2.y);
	//return sqrt(SQARE(Pt1.x-Pt2.x)+SQARE(Pt1.y-Pt2.y)); 
}

//          A
//          |\
//			|  \
//			|	 \
//			|	   \
//		  	|__ __ __\
//          B         C    (AB+BC)>AC  MAX(AB,AC)<AC, sqrt(x) ~= iteration on t In (MinX, MaxX) to approximate t^2 = x
//						   usually it is enough to approximate sqrt(x) = (MinX+MaxX)/2, especially for x < 30
MLong vDistance2L(MPOINT Pt1, MPOINT Pt2)
{
	//return (MLong)(sqrt(SQARE(Pt1.x-Pt2.x)+SQARE(Pt1.y-Pt2.y))); 
	MLong xdiff = ABS(Pt1.x-Pt2.x);
	MLong ydiff = ABS(Pt1.y-Pt2.y);
	return (MAX(xdiff,ydiff)+xdiff+ydiff)/2;
}

MLong  vDistance3L(MPOINT Pt1, MPOINT Pt2)
{
	return (MLong)(sqrt(SQARE(Pt1.x-Pt2.x)+SQARE(Pt1.y-Pt2.y)));
}

MDouble vDistance3D(MPOINT Pt1, MPOINT Pt2)
{
	return sqrt(SQARE(Pt1.x-Pt2.x)+SQARE(Pt1.y-Pt2.y));
}

MLong vGetPointMeanValueBtwPt(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MLong *pPtNum)
{
	MLong x, y;
	MLong lStart, lEnd;
	MLong lSumValue=0;
	MLong lPtNumTmp = 0;
	MLong lTmpVal = 0;

	MDouble k1, b1;
	if (ptStart.x==ptEnd.x)
	{
		MByte* pTmpRlt = pData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd;y++)
		{
			//lSumValue += (*(pTmpRlt+lDataLine*y))&Mask;
			lTmpVal = (*(pTmpRlt+lDataLine*y))&Mask;
			lSumValue += lTmpVal;
			if(0 != lTmpVal)
				lPtNumTmp++;
		}
		//if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
		//return lSumValue/(lEnd-lStart+1);
		if (pPtNum!=MNull)
			*pPtNum=lPtNumTmp;
		return lSumValue;
	}

	if (ptStart.y==ptEnd.y)
	{
		MByte* pTmpRlt = pData + ptStart.y*lDataLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd;x++)
		{
			//lSumValue += (*(pTmpRlt+x))&Mask;
			lTmpVal = (*(pTmpRlt+x))&Mask;
			lSumValue += lTmpVal;
			if(0 != lTmpVal)
				lPtNumTmp++;
		}
		//if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
		//return lSumValue/(lEnd-lStart+1);
		if (pPtNum!=MNull)
			*pPtNum=lPtNumTmp;
		return lSumValue;
	}
	
	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd; x++)
		{
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			//lSumValue +=(*(pData+x+y*lDataLine))&Mask;
			lTmpVal = (*(pData+x+y*lDataLine))&Mask;
			lSumValue += lTmpVal;
			if(0 != lTmpVal)
				lPtNumTmp++;
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd; y++)
		{
			//y = (MLong)(x*k1+b1);
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			//lSumValue +=(*(pData+x+y*lDataLine))&Mask;
			lTmpVal = (*(pData+x+y*lDataLine))&Mask;
			lSumValue += lTmpVal;
			if(0 != lTmpVal)
				lPtNumTmp++;
		}
	}
	//if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
	//return lSumValue/(lEnd-lStart+1);
	if (pPtNum!=MNull)
		*pPtNum=lPtNumTmp;
	return lSumValue;
	
}

MLong vGetPointMeanValueBtwPt_xsd(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MLong *pPtNum, MLong *lNum)
{
	MLong x, y;
	MLong lStart, lEnd;
	MLong lSumValue=0;
	MDouble k1, b1;
	MLong lTmpNum = 0;
	MLong lTmpVal;

	if (ptStart.x==ptEnd.x)
	{
		MByte* pTmpRlt = pData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd;y++)
		{
			lTmpVal = (*(pTmpRlt+lDataLine*y))&Mask;
			if (lTmpVal > 0)
			{
				lSumValue += lTmpVal;
				lTmpNum++;
			}
		}
		if (pPtNum!=MNull)
			*pPtNum=lEnd-lStart+1;
		if(lNum != MNull)
			*lNum = lTmpNum;
		return lSumValue/(lEnd-lStart+1);
	}

	if (ptStart.y==ptEnd.y)
	{
		MByte* pTmpRlt = pData + ptStart.y*lDataLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd;x++)
		{
			lTmpVal = (*(pTmpRlt+x))&Mask;
			if (lTmpVal > 0)
			{
				lSumValue += lTmpVal;
				lTmpNum++;
			}
		}
		if (pPtNum!=MNull)
			*pPtNum=lEnd-lStart+1;
		if(lNum != MNull)
			*lNum = lTmpNum;
		return lSumValue/(lEnd-lStart+1);
	}

	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd; x++)
		{
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			lTmpVal = (*(pData+x+y*lDataLine))&Mask;
			if (lTmpVal > 0)
			{
				lSumValue += lTmpVal;
				lTmpNum++;
			}
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd; y++)
		{
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			lTmpVal = (*(pData+x+y*lDataLine))&Mask;
			if (lTmpVal > 0)
			{
				lSumValue += lTmpVal;
				lTmpNum++;
			}
		}
	}
	if (pPtNum!=MNull)
		*pPtNum=lEnd-lStart+1;
	if(lNum != MNull)
		*lNum = lTmpNum;
	return lSumValue/(lEnd-lStart+1);

}

//guassian weight
//1/sqrt(2*V_PI) = 0.3989
//
//5*sigma 处最大值是最小值的3.7倍左右
//sigma = dist/5;
//3*sigma处最大值是最小值的3.3倍
//1.1774*sigma 处最大值是最小值的2倍
MDouble GaussianWeight(MDouble Sigmal, MLong dist)
{
	return exp(-(dist*dist)/(2*Sigmal*Sigmal));
	//return 1;
}

//MDouble GaussianWeight

MDouble vGetPointMeanValueBtwPtWithDistWeight(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
												MPOINT ptStart, MPOINT ptEnd, MByte Mask, MDouble *pPtWeightSum)
{
	MLong x, y;
	MLong lStart, lEnd, lMid;
	MDouble lSumValue=0;

	MLong lDist;
	MDouble weightSum = 0;

	MDouble k1, b1;
	if (ptStart.x==ptEnd.x)
	{
		MByte* pTmpRlt = pData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		lDist = (lEnd-lStart+1)/2;
		lMid = (lStart+lEnd)/2;
		for (y=lStart; y<=lEnd;y++)
		{
//			MDouble Weight = GaussianWeight(lDist/5, ABS(y-lMid));
			MDouble Weight = 1;
			lSumValue += (*(pTmpRlt+lDataLine*y)&Mask) * Weight;
			weightSum += Weight;
		}
		if (pPtWeightSum!=MNull)*pPtWeightSum=weightSum;
		return lSumValue/weightSum;
	}

	if (ptStart.y==ptEnd.y)
	{
		MByte* pTmpRlt = pData + ptStart.y*lDataLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		lDist = (lEnd-lStart+1)/2;
		lMid = (lStart+lEnd)/2;
		for (x=lStart; x<=lEnd;x++)
		{
			//MDouble Weight = GaussianWeight(lDist/5, ABS(x-lMid));
			MDouble Weight = 1;
			lSumValue += ((*(pTmpRlt+x))&Mask)*Weight;
			weightSum+=Weight;
		}
		if (pPtWeightSum!=MNull)*pPtWeightSum=weightSum;
		return lSumValue/weightSum;
	}
	
	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		lDist = (lEnd-lStart+1)/2;
		lMid = (lStart+lEnd)/2;
		for (x=lStart; x<=lEnd; x++)
		{
			//MDouble Weight = GaussianWeight(lDist/5, ABS(x-lMid));
			MDouble Weight = 1;
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			lSumValue +=((*(pData+x+y*lDataLine))&Mask)*Weight;
			weightSum+=Weight;
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		lDist = (lEnd-lStart+1)/2;
		lMid = (lStart+lEnd)/2;
		for (y=lStart; y<=lEnd; y++)
		{
			//y = (MLong)(x*k1+b1);
			//MDouble Weight = GaussianWeight(lDist/5, ABS(y-lMid));
			MDouble Weight = 1;
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			lSumValue +=((*(pData+x+y*lDataLine))&Mask)*Weight;
			weightSum+=Weight;
		}
	}
	if (pPtWeightSum!=MNull)*pPtWeightSum=weightSum;
	return lSumValue/weightSum;
	
}

MLong vGetPointMeanValueBtwPtWithWeight(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MByte *pWeightData, MLong lWeightLine,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MLong *pPtNum)
{
	MLong x, y;
	MLong lStart, lEnd;
	MLong lSumValue=0;

	MDouble k1, b1;
	if (ptStart.x==ptEnd.x)
	{
		MByte* pTmpRlt = pData + ptStart.x;
		MByte* pWeightRlt = pWeightData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd;y++)
		{
			if (*(pWeightRlt+lWeightLine*y)&0x80)
				lSumValue += 2*(*(pTmpRlt+lDataLine*y))&Mask;
			else
				lSumValue += (*(pTmpRlt+lDataLine*y))&Mask;
		}
		if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
		return lSumValue/(lEnd-lStart+1);
	}

	if (ptStart.y==ptEnd.y)
	{
		MByte* pTmpRlt = pData + ptStart.y*lDataLine;
		MByte* pWeightRlt = pWeightData + ptStart.y*lDataLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd;x++)
		{
			if (*(pWeightRlt+x)&0x80)
				lSumValue += 2*(*(pTmpRlt+x))&Mask;
			else
				lSumValue += (*(pTmpRlt+x))&Mask;
		}
		if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
		return lSumValue/(lEnd-lStart+1);
	}
	
	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd; x++)
		{
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			if (*(pWeightData+x+y*lWeightLine)&0x80)
				lSumValue +=2*(*(pData+x+y*lDataLine))&Mask;
			else
				lSumValue +=(*(pData+x+y*lDataLine))&Mask;
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd; y++)
		{
			//y = (MLong)(x*k1+b1);
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			if (*(pWeightData+x+y*lWeightLine)&0x80)
				lSumValue +=2*(*(pData+x+y*lDataLine))&Mask;
			else
				lSumValue +=(*(pData+x+y*lDataLine))&Mask;
		}
	}
	if (pPtNum!=MNull)*pPtNum=lEnd-lStart+1;
	return lSumValue/(lEnd-lStart+1);
	
}

MLong vGetPointMeanValue(MByte* pData, MLong lDataLine,
				MLong lWidth, MLong lHeight,
				MPOINT ptStart, MPOINT ptEnd, MDouble dCoeffK, MDouble dCoeffB, MByte bVertical)
{
	MLong x, y;
	MLong lSumValue=0;
	MByte *pTmpData;
	MLong minValue, maxValue;
	if (bVertical)//是垂直的line
	{
		 minValue = ptStart.y>ptEnd.y?ptEnd.y:ptStart.y;
		 maxValue = ptStart.y>ptEnd.y?ptStart.y:ptEnd.y;
		//vertical 的话,x 坐标是一致的
		pTmpData = pData + minValue * lDataLine  + ptStart.x;
		for (y = minValue; y<=maxValue; y++, pTmpData+=lDataLine)
			lSumValue += *pTmpData;
		return lSumValue/(maxValue-minValue+1);
	}

	if (dCoeffK<1.0 && dCoeffK>-1.0)
	{
		minValue = ptStart.x > ptEnd.x ?ptEnd.x:ptStart.x;
		maxValue = ptStart.x>ptEnd.x?ptStart.x:ptEnd.x;
		
		for (x= minValue; x<=maxValue; x++)	
		{
			//根据x坐标求y坐标
			y = (MLong)(dCoeffK * x + dCoeffB);
			lSumValue += *(pData + y * lDataLine  + x);
		}
		return lSumValue / (maxValue-minValue+1);
	}
	else
	{
		minValue = ptStart.y > ptEnd.y ?ptEnd.y:ptStart.y;
		maxValue = ptStart.y>ptEnd.y?ptStart.y:ptEnd.y;
		
		for (y= minValue; y<=maxValue; y++)	
		{
			//根据y坐标求x坐标
			x = (MLong)((y-dCoeffB)/dCoeffK);
			lSumValue += *(pData + y * lDataLine  + x);
		}
		return lSumValue / (maxValue-minValue+1);
	}
	
}

MVoid vDrawLine(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
			  MPOINT ptPass, MDouble slope, MLong lLength)
{
	MLong x, y;
	//y = x*k + b
	MDouble b = ptPass.y - ptPass.x * slope;

	if (slope<1.0&&slope>-1.0)
	{
		MLong lCount = lLength/2;
		for (x = ptPass.x; x<lWidth && lCount>=0 ; x++, lCount--)
		{
			MLong y = (MLong)(x*slope+b);
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
		lCount = lLength/2;
		for (x = ptPass.x; x>=0 && lCount>=0 ; x--, lCount--)
		{
			MLong y =(MLong)( x*slope+b);
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
	}
	else
	{
		MLong lCount = lLength/2;
		for (y = ptPass.y; y<lHeight && lCount>=0 ; y++, lCount--)
		{
			//MLong y = x*slope+b;
			MLong x = (MLong)((y-b)/slope);
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
		lCount = lLength/2;
		for (x = ptPass.x; x>=0 && lCount>=0 ; x--, lCount--)
		{
			MLong x = (MLong)((y-b)/slope);
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
	}
}

//Theta (-pi/2, pi/2)
MVoid vDrawLine2(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
				MDouble slope, MDouble intercept)
{
	MLong x, y;
	if (slope<1.0&&slope>-1.0)
	{
		for (x = 0; x<lWidth  ; x++)
		{
			MLong y = (MLong)(x*slope+intercept);
			if (y<0 || y>=lHeight)continue;
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
	}
	else
	{
		for (y = 0; y<lHeight ; y++)
		{
			MLong x = (MLong)((y-intercept)/slope);
			if (x<0 || x>=lWidth)continue;
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
	}
}

MVoid vLineTo(MByte* pRltData, MLong lRltLine,
			  MLong lWidth, MLong lHeight,
			  MByte DestFlag,
			  MPOINT ptStart, MPOINT ptEnd)
{
	MLong x, y;
	MLong lStart, lEnd;

	MDouble k1, b1;
	if (ptStart.x==ptEnd.x)
	{
		MByte* pTmpRlt = pRltData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd;y++)
		{
			*(pTmpRlt+lRltLine*y) = DestFlag;
		}
		return;
	}

	if (ptStart.y==ptEnd.y)
	{
		MByte* pTmpRlt = pRltData + ptStart.y*lRltLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd;x++)
		{
			*(pTmpRlt+x) = DestFlag;
		}
		return;
	}
	
	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd; x++)
		{
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			*(pRltData+x+y*lRltLine) = DestFlag;
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd; y++)
		{
			//y = (MLong)(x*k1+b1);
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			*(pRltData+x+y*lRltLine) = DestFlag;
		}
	}
	return;
}

MVoid vLineToImg(JOFFSCREEN* pImgSrc, MCOLORREF ref,  MPOINT ptStart, MPOINT ptEnd)
{
	MLong x, y;
	MLong lStart, lEnd;

	MLong lWidth = pImgSrc->dwWidth;
	MLong lHeight = pImgSrc->dwHeight;
	
	MDouble k1, b1;
	if (ptStart.x==ptEnd.x)
	{
		//MByte* pTmpRlt = pRltData + ptStart.x;
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd;y++)
		{
			//*(pTmpRlt+lRltLine*y) = DestFlag;
			ImgSetPixel(pImgSrc, ptStart.x, y, ref);
		}
		return;
	}
	
	if (ptStart.y==ptEnd.y)
	{
		//MByte* pTmpRlt = pRltData + ptStart.y*lRltLine;
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd;x++)
		{
		//	*(pTmpRlt+x) = DestFlag;
			ImgSetPixel(pImgSrc, x, ptStart.y, ref);
		}
		return;
	}
	
	k1 = (ptStart.y-ptEnd.y)*1.0/(ptStart.x-ptEnd.x);
	b1 = ptStart.y - k1*ptStart.x;
	
	if ((k1)<1.0&&k1>-1.0)
	{
		lStart = MIN(MAX(MIN(ptStart.x, ptEnd.x), 0),lWidth-1);
		lEnd = MIN(MAX(MAX(ptStart.x, ptEnd.x), 0),lWidth-1);
		for (x=lStart; x<=lEnd; x++)
		{
			y = (MLong)(x*k1+b1);
			if (y<0||y>=lHeight)continue;
			//*(pRltData+x+y*lRltLine) = DestFlag;
			ImgSetPixel(pImgSrc, x,y,ref);
		}
	}
	else
	{
		lStart = MIN(MAX(MIN(ptStart.y, ptEnd.y), 0),lHeight-1);
		lEnd = MIN(MAX(MAX(ptStart.y, ptEnd.y), 0),lHeight-1);
		for (y=lStart; y<=lEnd; y++)
		{
			//y = (MLong)(x*k1+b1);
			x = (MLong)((y-b1)/k1);
			if (x<0||x>=lWidth)continue;
			//*(pRltData+x+y*lRltLine) = DestFlag;
			ImgSetPixel(pImgSrc, x,y,ref);
		}
	}
	return;
}

MVoid vProjectToLinePoint(MDouble CoeffK, MDouble CoeffB, MBool bVert,MPOINT ptOut, MPOINT *pProjectPoint)
{
	MDouble CoeffK1,CoeffB1;
	if(pProjectPoint==MNull)return;
	if (bVert)
	{
		pProjectPoint->x = (MLong)(-CoeffB);
		pProjectPoint->y = ptOut.y;
	}
	else
	{
		CoeffK1 = -1/CoeffK;
		CoeffB1 = ptOut.y - ptOut.x*CoeffK1;
		pProjectPoint->x = (MLong)(-(CoeffB-CoeffB1)/(CoeffK-CoeffK1));
		pProjectPoint->y = (MLong)(CoeffK*pProjectPoint->x+CoeffB);
	}
	return;
}

MVoid vPerpendPoint(MPOINT Point1, MPOINT Point2,
					MPOINT* pPoint3, MPOINT* pPoint4,
					MLong PerPendDist)
{
	MDouble k1, k2, b1, b2, bExtra;
	//MLong k1, k2, b1, b2;//fix-pointed
	MPOINT ptCen={0};
	MPOINT PT3, PT4;
	MLong DistMinor = PerPendDist;

	ptCen.x = (Point1.x+Point2.x)/2;
	ptCen.y = (Point1.y+Point2.y)/2;
	if (Point1.x==Point2.x)
	{
		PT4.y = PT3.y = ptCen.y;
		PT3.x = ptCen.x - (DistMinor/2);
		PT4.x = ptCen.x + (DistMinor/2);
	}
	else if (Point1.y==Point2.y)
	{
		PT3.x = PT4.x = ptCen.x;
		PT3.y = ptCen.y - (DistMinor/2);
		PT4.y= ptCen.y+(DistMinor/2);
	}
	else
	{
		//k1 = (Point1.y - Point2.y)*1.0/(Point1.x - Point2.x);
		k1 = (Point1.y - Point2.y)*1.0/(Point1.x - Point2.x); 
		b1 = ptCen.y-(k1*ptCen.x);
		k2 = -1 / k1;
		b2 = ptCen.y - k2*ptCen.x;
		bExtra = (DistMinor/2)/(1/sqrt(k1*k1+1));
		
		PT3.x = (MLong)(-(b2-b1-bExtra)/(k2-k1)+0.5);
		//PT3.y = (k2*(PT3.x)+b2+0.5);
		PT3.y = (MLong)(k1*PT3.x+b1+bExtra);
		
		PT4.x = (MLong)(-(b2-b1+bExtra)/(k2-k1)-0.5);
		//PT4.y = (k2*PT4.x+b2-0.5);
		PT4.y = (MLong)(k1*PT4.x+b1-bExtra);
	}
	if (pPoint3&&pPoint4)
	{
		*pPoint3 = PT3;
		*pPoint4 = PT4;
	}
}


//PointList is sequential list;
MVoid vFillMultiPolygon(MByte* pRltData, MLong lRltLine,
					   MLong lWidth, MLong lHeight,
					   MPOINT *PointList, MLong lPtNum,
					   MByte FillFlag)
{
	MLong y, i;
	MRECT ValidRt={0};
	MByte* pTmpRlt;

	JASSERT(lPtNum>0);

	if (lPtNum==1) 
	{
		*(pRltData+lRltLine*PointList[0].y+PointList[0].x)=FillFlag;
		return;
	}
	if (lPtNum==2) 
	{
		vLineTo(pRltData, lRltLine, lWidth, lHeight, FillFlag, PointList[0], PointList[1]);
		return;
	}

	vLineTo(pRltData, lRltLine, lWidth, lHeight, FillFlag, PointList[0], PointList[lPtNum-1]);
	for (i=1; i<lPtNum; i++)
	{
		vLineTo(pRltData, lRltLine, lWidth, lHeight, FillFlag, PointList[i], PointList[i-1]);
	}

	ValidRt.left = lWidth;
	ValidRt.top = lHeight;
	ValidRt.bottom = ValidRt.right = 0;
	for (i=0; i<lPtNum; i++)
	{
		MPOINT *pPtCur = &(PointList[i]);
		if (ValidRt.left>pPtCur->x)ValidRt.left=pPtCur->x;
		if (ValidRt.right<pPtCur->x)ValidRt.right=pPtCur->x;
		if (ValidRt.top>pPtCur->y)ValidRt.top=pPtCur->y;
		if (ValidRt.bottom<pPtCur->y)ValidRt.bottom=pPtCur->y;
	}
	
//	PrintBmp(pRltData, lRltLine, DATA_U8, lWidth, lHeight, 1);
	pTmpRlt = pRltData+lRltLine*ValidRt.top/*+ValidRt.left*/;
	//lTmpExt = lRltLine - (ValidRt.right - ValidRt.left);

	for (y=ValidRt.top; y<ValidRt.bottom; y++, pTmpRlt+=lRltLine)
	{
		MLong lStart, lEnd;
		MByte* pStart = pTmpRlt+ValidRt.left;
		MByte* PEnd = pTmpRlt + ValidRt.right-1;
		for (lStart = ValidRt.left; lStart<ValidRt.right; lStart++, pStart++)
		{
			if (*pStart==FillFlag) break;
		}
		for (lEnd = ValidRt.right; lEnd>ValidRt.left; lEnd--, PEnd--)
		{
			if (*PEnd == FillFlag)break;
		}
		if (lStart>=lEnd)continue;

		for (i=lStart; i<=lEnd; i++)
		{
			*(pTmpRlt+i) = FillFlag;
		}
	}
	
	return;
}


//vFitLine2只适用于两个点的拟合
static MRESULT vFitLine2(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical);

//直线拟合增加出错的返回值
MRESULT vFitLine(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical)
{
	MRESULT res = LI_ERR_NONE;
	MLong i;
	MLong n=0, sX = 0, sY = 0;
	MDouble mX, mY, sXX=0.0, sXY=0.0, sYY=0.0;
	JASSERT(pCoeff1 && pCoeff2 && pbVertical);

	if (lListLen<2)return LI_ERR_UNKNOWN;
	
	if (lListLen == 2)
	{
		vFitLine2(pPtLists, 2, pCoeff1, pCoeff2, pbVertical);
		return LI_ERR_NONE;
	}
		
	for (i=0;i<lListLen;i++) 
	{
		sX += pPtLists[i].x;
		sY += pPtLists[i].y;
	}

	mX = sX/lListLen;
	mY = sY/lListLen;

	for (i=0;i<lListLen;i++) 
	{
		sXX += (pPtLists[i].x-mX)*(pPtLists[i].x-mX);
		sXY += (pPtLists[i].x-mX)*(pPtLists[i].y-mY);
		sYY += (pPtLists[i].y-mY)*(pPtLists[i].y-mY);
	}

	if (sXY==0 && sXX<sYY)
		{//vertical
		*pbVertical = MTrue;
		*pCoeff1 = 1;
		*pCoeff2 = -mX;
		}
	else if (sXY==0 && sXX>sYY)
		{//horizontal
		*pbVertical = MFalse;
		*pCoeff1 = 0;
		*pCoeff2 = mY;
		}
	else if (sXY==0&&sXX==sYY)
		{//Indeterminate
		res = LI_ERR_UNKNOWN;
		}
	else
		{
		*pCoeff1 = (sYY - sXX + sqrt((sYY - sXX) * (sYY - sXX) + 4.0 * sXY * sXY)) / (2.0 * sXY);
       	*pCoeff2 = mY - *pCoeff1  * mX;
		*pbVertical = MFalse; 
		}

	return res;
}

MRESULT vFitLine2(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical)
{
	MLong i;
	MDouble t,sxoss,sx=0.0,sy=0.0,st2=0.0,ss;
	
	MDouble a=0.0, b=0.0;

	JASSERT(pCoeff1 && pCoeff2 && pbVertical);

	if (lListLen<2)return LI_ERR_UNKNOWN;
	
	for (i=0;i<lListLen;i++) 
	{
		sx += pPtLists[i].x;
		sy += pPtLists[i].y;
	}
	ss=lListLen;

	sxoss=sx/ss;
	
	for (i=0;i<lListLen;i++) 
	{
		t=pPtLists[i].x-sxoss;
		st2 += t*t;
		b += t*pPtLists[i].y;
	}
	if (st2==0)
	{
		*pbVertical = MTrue;
		*pCoeff1 = 1;
		*pCoeff2 = -sx / lListLen;
		return LI_ERR_NONE;
	}
	b /= st2;
	a=(sy-sx*b)/ss;
	*pbVertical = MFalse;
	*pCoeff1 = b;
	*pCoeff2 = a;
	return LI_ERR_NONE;
}
/*
MRESULT vFitLine(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical)
{
	MDouble Zoom=0;
	MLong i;
	MLong xSum = 0, x2Sum = 0, ySum =0, xySum = 0;
	MDouble tmpInt;
	JASSERT(pbVertical);

	*pbVertical = MFalse;
	for(i=0; i<lListLen; i++)
	{
		xSum += pPtLists[i].x;
		x2Sum += pPtLists[i].x*pPtLists[i].x;
		ySum += pPtLists[i].y;
		xySum += pPtLists[i].x*pPtLists[i].y;
	}
	Zoom = (MDouble)(xSum)*xSum - (MDouble)(x2Sum)*lListLen;
	//Zoom = (xSum)*(xSum)- (x2Sum)*lListLen;
	//Zoom = (MLong)tmpInt;
	if (Zoom==0.0) 
	{
		*pbVertical = MTrue;
		*pCoeff1 = 1;
		*pCoeff2 = -xSum / lListLen;
		return;
	}
	*pCoeff1 = (MDouble)(((MDouble)(xSum)*(ySum)-(MDouble)(xySum)*lListLen)*1.0/Zoom);
	*pCoeff2 = (MDouble)(((MDouble)(xySum)*(xSum)-(MDouble)(ySum)*(x2Sum))*1.0/Zoom);

	return 1;
}
*/
MVoid vDrawRect(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
				MRECT *pRect)
{
	MPOINT leftTop, rightTop, leftBot, rightBot;
	leftTop.x = pRect->left;
	leftTop.y = pRect->top;
	rightTop.x = pRect->right;
	rightTop.y = pRect->top;

	leftBot.x = pRect->left;
	leftBot.y = pRect->bottom;
	rightBot.x = pRect->right;
	rightBot.y = pRect->bottom;

	vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftTop, leftBot);
	vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftTop, rightTop);
	vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftBot, rightBot);
	vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, rightTop, rightBot);
}


//0 -- 2*pi

MDouble vComputeAngle(MLong Gx, MLong Gy)
{
	MDouble res;
	JASSERT(!(Gx==0 && Gy==0));
	if (Gx==0) 
	{
		if (Gy>0) res=V_PI/2;
		else
			res= 3*V_PI/2;
		return res;
	}

	res = atan(Gy*1.0/Gx);

	if (res>=0 && Gx<0) res += V_PI;

	if (res<0)
	{
		if (Gx<0) res += V_PI;
		else
			res += 2*V_PI;
	}
		
	return res;
}

MVoid vComputeEigen(MATRIX_2D* pMatrix, EIGEN_2D *pEigen)
{
	MLong lDims = 2;
	MDouble trace, determ;
	//MDouble pData[2][2] = pMatrix->data.fd;
	MDouble lamda0, lamda1, tmpsqrt;
	JASSERT(pMatrix);

	if (pEigen==MNull)  return;

	if (lDims == 2)
	{
		//pData = pMatrix->data.fd;
		trace = pMatrix->data.fd[0][0] + pMatrix->data.fd[1][1];
		
		determ = pMatrix->data.fd[0][0]*pMatrix->data.fd[1][1]-pMatrix->data.fd[0][1]*pMatrix->data.fd[1][0];
		tmpsqrt = sqrt(trace*trace-4*determ);
		lamda0 = (trace+tmpsqrt)/2;
		lamda1 = (trace-tmpsqrt)/2; 

		tmpsqrt = MIN(lamda0, lamda1);
		lamda0 = MAX(lamda0, lamda1);
		lamda1 = tmpsqrt;
		
		pEigen->EigenVal[0] = (lamda0 );
		pEigen->EigenVal[1] = ( lamda1);

		pEigen->EigenVec[0][0] = -pMatrix->data.fd[0][1];
		pEigen->EigenVec[0][1] = pMatrix->data.fd[0][0] - lamda0;

		pEigen->EigenVec[1][0] = pMatrix->data.fd[1][0]-lamda1;
		pEigen->EigenVec[1][1] = -pMatrix->data.fd[1][0];
	}

}

MVoid vEllipceFit(MVoid *pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeigth,
				 ELLIPCE* pEllipce)
{
	MLong xcoord,ycoord;
	MLong semiWidth, semiHeight;
	MDouble alpha;

	MDouble ellipceArea;
	MDouble eccentric;

	MLong i,j, lExt;
	MLong sumx=0, sumy=0, pixelNum=0;
	MLong eta11=0, eta20=0, eta02=0;
	MDouble Ix=0.0, Iy=0.0;
	MDouble NormIx, NormIy;
	MByte *pTmpSrc = (MByte*) pSrcData;
	MDouble cosThet, sinThet;
	lExt = lSrcLine-lWidth;
	
	for (j=0; j<lHeigth; j++, pTmpSrc+=lExt)
	{
		for (i=0; i<lWidth; i++, pTmpSrc++)
		{
			if (*pTmpSrc==0) continue;
			pixelNum++;
			sumx+=i;sumy+=j;
		}
	}
	xcoord = sumx/pixelNum;
	ycoord = sumy/pixelNum;

	pTmpSrc = (MByte*) pSrcData;
	for (j=0; j<lHeigth; j++, pTmpSrc+=lExt)
	{
		for (i=0; i<lWidth; i++, pTmpSrc++)
		{
			if (*pTmpSrc==0) continue;
			eta11 += (i-xcoord)*(j-ycoord);
			eta02 += (j-ycoord)*(j-ycoord);
			eta20 += (i-xcoord)*(i-xcoord);
		}
	}
	alpha = 1.0/2*atan(2.0*eta11/(eta02*eta20));
	cosThet = cos(alpha);
	sinThet = sin(alpha);
	pTmpSrc = (MByte*) pSrcData;
	for (j=0; j<lHeigth; j++, pTmpSrc+=lExt)
	{
		for (i=0; i<lWidth; i++, pTmpSrc++)
		{
			if (*pTmpSrc==0) continue;
			Ix += ((j-ycoord)*cosThet-(i-xcoord)*sinThet)*((j-ycoord)*cosThet-(i-xcoord)*sinThet);
			Iy += ((i-xcoord)*cosThet+(j-ycoord)*sinThet)*((i-xcoord)*cosThet+(j-ycoord)*sinThet);
		}
	}
	NormIx = Ix/(MAX(Ix,Iy));
	NormIy = Iy/(MAX(Ix,Iy));
	semiWidth= (MLong)(pow(4/V_PI, 1.0/4)*pow(pow(Iy, 3.0/4)/pow(Ix,1.0/4), 1.0/2));
	semiHeight= (MLong)(pow(4/V_PI, 1.0/4)*pow(pow(Ix, 3.0/4)/pow(Iy,1.0/4), 1.0/2));

	eccentric = semiWidth*1.0/semiHeight;
	ellipceArea = V_PI*semiWidth*semiHeight;

	while (ellipceArea>pixelNum*0.80)
	{
		ellipceArea *= 0.9;
	}

	semiHeight = (MLong)(sqrt(ellipceArea/(V_PI*eccentric)));
	semiWidth = (MLong)(eccentric*semiHeight);

	if (MNull!=pEllipce)
	{
		pEllipce->alpha = alpha;
		pEllipce->xcoord = xcoord;
		pEllipce->ycoord = ycoord;
		pEllipce->semiWidth = semiWidth;
		pEllipce->semiHeight = semiHeight;
	}
}

MVoid vDrawEllipce(MVoid *pShowData, MLong lShowLine, MLong lWidth, MLong lHeigth,
				  ELLIPCE* pEllipce)
{
	MLong i;
	MLong steps = 72*4;

	MDouble beta = -(pEllipce->alpha)*V_PI/180;
	MDouble sinbeta = sin(beta);
	MDouble cosbeta = cos(beta);

	MByte* pTmpData = (MByte*)pShowData;

	for (i=0; i<360; i+=360/steps)
	{
		MDouble alpha = i*V_PI/180;
		MDouble sinalpha=sin(alpha);
		MDouble cosalpha=cos(alpha);

		MLong  x = (MLong)(pEllipce->xcoord
			+(pEllipce->semiWidth*cosalpha*cosbeta-pEllipce->semiHeight*sinalpha*sinbeta));
		MLong  y = (MLong)(pEllipce->ycoord
			+(pEllipce->semiWidth*cosalpha*sinbeta+pEllipce->semiHeight*sinalpha*cosbeta));
		*(pTmpData+y*lShowLine+x) = 255;
	}
}

MVoid vDrawEllipceOnImg(JOFFSCREEN *pShowImg, ELLIPCE* pEllipce, MCOLORREF color, MLong thickness)
{
	MLong i, j;
	MLong steps = 72*4;
	MLong lWidth = (MLong)(pShowImg->dwWidth);
	MLong lHeight = (MLong)(pShowImg->dwHeight);
	
	//MDouble beta = -(pEllipce->alpha)*V_PI/180;
	MDouble beta = pEllipce->alpha-V_PI/2;
	MDouble sinbeta = sin(beta);
	MDouble cosbeta = cos(beta);
	
	//MByte* pTmpData = (MByte*)pShowData;
	JASSERT(thickness>0&&thickness<=MIN(MIN(pEllipce->semiHeight,pEllipce->semiWidth),5));
	for (j=0; j<thickness; j++)
	{
		MLong semiWidth = pEllipce->semiWidth - j;
		MLong semiHeight =pEllipce->semiHeight - j;
		for (i=0; i<360; i+=360/steps)
		{
			MDouble alpha = i*V_PI/180;
			MDouble sinalpha=sin(alpha);
			MDouble cosalpha=cos(alpha);
			
			MLong  x = (MLong)(pEllipce->xcoord
				+(pEllipce->semiWidth*cosalpha*cosbeta-pEllipce->semiHeight*sinalpha*sinbeta));
			MLong  y = (MLong)(pEllipce->ycoord
				+(pEllipce->semiWidth*cosalpha*sinbeta+pEllipce->semiHeight*sinalpha*cosbeta));
			//*(pTmpData+y*lShowLine+x) = 255;
			if (x<0||y<0||x>=lWidth||y>=lHeight)continue;
			ImgSetPixel(pShowImg, x, y, color);
		}
	}
}

//(r, s)
//((r-xc)*cos(theta)+(s-yc)sin*(theta))2/w2+((s-yc)*cos(theta)-(r-xc)*sin(theta))2/h2
MBool vbInEllipce(ELLIPCE* pEllipce, MPOINT pt)
{
 	MLong xc = pEllipce->xcoord;
 	MLong yc = pEllipce->ycoord;
 	MDouble theta = pEllipce->alpha;
// 	MLong w2 = pEllipce->semiHeight*pEllipce->semiHeight;
// 	MLong h2 = pEllipce->semiWidth*pEllipce->semiWidth;
// 	MDouble bVal = ((pt.x-xc)*cos(theta)+(pt.y-yc)*sin(theta))*((pt.x-xc)*cos(theta)+(pt.y-yc)*sin(theta))/w2
// 		+((pt.x-yc)*cos(theta)-(pt.y-xc)*sin(theta))*((pt.x-yc)*cos(theta)-(pt.y-xc)*sin(theta))/h2;
//	return (bVal<=1);
	MDouble d1, d2;
	MDouble a = pEllipce->semiHeight;
	MDouble b = pEllipce->semiWidth;
	MDouble c = sqrt(a*a-b*b);
	MPOINT focus1,focus2;
	focus1.x = xc + (MLong)(c*cos(theta));
	focus1.y = yc + (MLong)(c*sin(theta));
	focus2.x = xc - (MLong)(c*cos(theta));
	focus2.y = yc - (MLong)(c*sin(theta));
	d1=(float)sqrt((focus1.x-pt.x)*(focus1.x-pt.x)+(focus1.y-pt.y)*(focus1.y-pt.y));//到左焦点的距离 
	d2=(float)sqrt((focus2.x-pt.x)*(focus2.x-pt.x)+(focus2.y-pt.y)*(focus2.y-pt.y));//到右焦点的距离 

	return (d1+d2 < 2*a); 
	
}


MVoid vDrawArrow(MByte* pRltData, MLong lRltLine,
				 MLong lWidth, MLong lHeight,
				 MByte DestFlag, MPOINT ptStart, MDouble direct, MLong lLength)
{
	MPOINT ptEnd = {0};
	MPOINT ptArrow1 = {0}, ptArrow2 = {0};
	ptEnd.x = ptStart.x + (MLong)(cos(direct) * lLength);
	ptEnd.y = ptStart.y + (MLong)(sin(direct) * lLength);
	
	ptArrow1.x = ptEnd.x - (MLong)(cos(direct-22.5*V_PI/180)*2);
	ptArrow1.y = ptEnd.y - (MLong)(sin(direct-22.5*V_PI/180)*2);

	ptArrow2.x = ptEnd.x - (MLong)(cos(direct+22.5*V_PI/180)*2);
	ptArrow2.y = ptEnd.y - (MLong)(sin(direct+22.5*V_PI/180)*2);

	vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, ptStart, ptEnd);
	vLineTo(pRltData, lRltLine,lWidth,lHeight,DestFlag,ptEnd,ptArrow1);
	vLineTo(pRltData, lRltLine,lWidth,lHeight,DestFlag,ptEnd,ptArrow2);
}

MVoid vDrawRectImg(JOFFSCREEN* pImgSrc, 
				   MCOLORREF ref, MRECT *pRect)
{
	MLong lWidth = pImgSrc->dwWidth;
	MLong lHeight = pImgSrc->dwHeight;
	MPOINT leftTop, rightTop, leftBot, rightBot;
	leftTop.x = pRect->left;
	leftTop.y = pRect->top;
	rightTop.x = pRect->right;
	rightTop.y = pRect->top;
	
	leftBot.x = pRect->left;
	leftBot.y = pRect->bottom;
	rightBot.x = pRect->right;
	rightBot.y = pRect->bottom;
	
	//vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftTop, leftBot);
	//vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftTop, rightTop);
	//vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, leftBot, rightBot);
	//vLineTo(pRltData, lRltLine, lWidth, lHeight, DestFlag, rightTop, rightBot);	
	vLineToImg(pImgSrc, ref, leftTop, leftBot);
	vLineToImg(pImgSrc, ref, leftTop, rightTop);
	vLineToImg(pImgSrc, ref, leftBot, rightBot);
	vLineToImg(pImgSrc, ref, rightTop, rightBot);
}


MVoid vGetLineParam(MPOINT pt1, MPOINT pt2, 
				   MDouble *A, MDouble *B, MDouble *C)
{
	MDouble InnerA, InnerB, InnerC;
	if (pt1.x == pt2.x )
	{
		if (pt1.y==pt2.y) InnerA = InnerB = InnerC = 0;
		else
		{
			InnerB = 0;
			InnerC = 1;
			InnerA = -InnerC/pt1.x;
		}
	}
	else
	{
		InnerA = (pt2.y-pt1.y)*1.0/(pt2.x-pt1.x);
		InnerB = -1;
		InnerC = pt2.y - InnerA*pt2.x;
	}

	if (A!=MNull) *A = InnerA;
	if (B!=MNull) *B = InnerB;
	if (C!=MNull) *C = InnerC;
}

MDouble vCalculateTheta(MPOINT pt1, MPOINT pt2)
{
	MDouble theta;
	if (pt2.x == pt1.x)
		return theta = V_PI/2;
	
	theta = atan((pt2.y-pt1.y)*1.0/(pt2.x-pt1.x));

	if (theta<0)
		theta += V_PI;
	return theta;

}

//余弦定理
//              b    /\   c
//                 /    \
//                 ----
//                    a(对角theta)
//a^2 = b^2+c^2-2b*c*cos(theta)
MDouble vComputeIntersactionAngle(MPOINT ptLeft, MPOINT ptMid, MPOINT ptRight)
{
	MDouble a2, b2, c2;
	a2 = SQARE(ptLeft.x-ptRight.x)+SQARE(ptLeft.y-ptRight.y);
	b2 = SQARE(ptLeft.x-ptMid.x)+SQARE(ptLeft.y-ptMid.y);
	c2 = SQARE(ptRight.x-ptMid.x)+SQARE(ptRight.y-ptMid.y);
	return acos((b2+c2-a2)/(2*sqrt(b2*c2)));
}

MLong vPointDistToLine(MPOINT ptOut, MPOINT ptLine1, MPOINT ptLine2)
{
	MLong lGround = vDistance1L(ptLine1, ptLine2);
	MLong lSide1 = vDistance1L(ptOut, ptLine1);
	MLong lSide2 = vDistance1L(ptOut, ptLine2);

	MLong lHalfPerimeter = (lGround+lSide1+lSide2)/2;

	MDouble area=sqrt(lHalfPerimeter*(lHalfPerimeter-lGround)*(lHalfPerimeter-lSide1)*(lHalfPerimeter-lSide2));

	return (MLong)(area*2/lGround);
}

MLong vPointDistToLine2(MPOINT ptOut, MDouble dCoeffK, MDouble dCoeffB, MBool bVertical)
{
	if (bVertical)
	{
		return (MLong)(fabs(ptOut.x + dCoeffB));//这里要注意，我们采用的是0 = -x+b的方程
	}
	return (MLong)(fabs(ptOut.x*dCoeffK+dCoeffB-ptOut.y)/sqrt(dCoeffK*dCoeffK+1));
}

MDouble vPointDistToLine2D(MPOINT pt, MDouble dCoeffK, MDouble dCoeffB, MLong bVert)	//xsd
{
	if (bVert)
	{
		return (pt.x + dCoeffB);//这里要注意，我们采用的是0 = -x+b的方程
	}
	return (pt.x*dCoeffK+dCoeffB-pt.y);
}

MRESULT vCircleFitting(MPOINT *pPtList, MLong lPtLen, 
					   MLong *xc, MLong *yc, MLong *r)
{
	MLong i;
	MRESULT res=LI_ERR_NONE;
	MDouble X1=0,Y1=0,X2=0,Y2=0,X3=0,Y3=0,X1Y1=0,X1Y2=0,X2Y1=0;
	MDouble C,D,E,G,H,N;
    MDouble a,b,c;
	if (lPtLen<3) 
		return res=-1;

	for (i=0;i<lPtLen;i++)
    {
        X1 = X1 + pPtList[i].x;
        Y1 = Y1 + pPtList[i].y;
        X2 = X2 + pPtList[i].x*pPtList[i].x;
        Y2 = Y2 + pPtList[i].y*pPtList[i].y;
        X3 = X3 + pPtList[i].x*pPtList[i].x*pPtList[i].x;
        Y3 = Y3 + pPtList[i].y*pPtList[i].y*pPtList[i].y;
        X1Y1 = X1Y1 + pPtList[i].x*pPtList[i].y;
        X1Y2 = X1Y2 + pPtList[i].x*pPtList[i].y*pPtList[i].y;
        X2Y1 = X2Y1 + pPtList[i].x*pPtList[i].x*pPtList[i].y;
    }

	N = lPtLen;
    C = N*X2 - X1*X1;
    D = N*X1Y1 - X1*Y1;
    E = N*X3 + N*X1Y2 - (X2+Y2)*X1;
    G = N*Y2 - Y1*Y1;
    H = N*X2Y1 + N*Y3 - (X2+Y2)*Y1;
	if (C*G-D*D!=0)
	{
		a = (H*D-E*G)/(C*G-D*D);
		b = (H*C-E*D)/(D*D-G*C);
		c = -(a*X1 + b*Y1 + X2 + Y2)/N;
	}
	else
	{
		//拟合不出来
		a = 0;
		b = 0;
		c = 0;
		res = -1;
	}


	if (xc)
	{
		*xc=(MLong)(a/(-2));
	}
	if (yc)
	{
		*yc=(MLong)(b/(-2));
	}
	if (r)
	{
		*r=(MLong)(sqrt(a*a+b*b-4*c)/2);
	}

	return res;	
}

MBool bOnCircle(MLong xc, MLong yc, MLong r, MLong i, MLong j)
{
	MLong lVal = SQARE(i-xc)+SQARE(j-yc);
	return (lVal>SQARE(r-1))&&(lVal<SQARE(r+1));
}

MBool bOnCircle_EX(MLong xc, MLong yc, MLong r, MLong i, MLong j, MLong Bias)
{
	MLong lVal = SQARE(i-xc)+SQARE(j-yc);
	return (lVal>SQARE(r-Bias))&&(lVal<SQARE(r+Bias));
	
}

MBool bInCircle_EX(MLong xc, MLong yc, MLong r, MLong i, MLong j)
{
	MLong lVal = SQARE(i-xc)+SQARE(j-yc);
	MBool ret = lVal <= SQARE(r);
	return ret;
}


MVoid vDrawCircle(MByte* pShowData, MLong lShowLine, MLong lWidth, MLong lHeight,
				  MLong xc, MLong yc, MLong r, MByte lForeFlag)
{
	MLong i, j;
	MLong lLineExt;
	MByte *pTmpShow=MNull;

	pTmpShow = pShowData;
	lLineExt = lShowLine - lWidth;
	for (j=0; j<lHeight; j++, pTmpShow+=lLineExt)
	{
		for (i=0; i<lWidth; i++, pTmpShow++)
		{
			if (bOnCircle(xc,yc,r,i,j))
				*pTmpShow = lForeFlag;
		}
	}
}

//计算圆与直线的交点
MLong vComputeCrossPt(MLong xc, MLong yc, MLong r, MDouble CoeffK, MDouble CoeffB, MBool bVert, 
	MPOINT *pCrossPT1, MPOINT* pCrossPT2)
{
	MLong x1, y1, x2, y2;
	MDouble c, a, b1,tmp;

	if (bVert)
	{
		x1 = x2 = (MLong)(-CoeffB);
		y1 = (MLong)(yc + sqrt(r*r-xc*xc));
		y2 = (MLong)(yc - sqrt(r*r-xc*xc));
	}
	else
	{
		c = xc*xc + (CoeffB- yc)*(CoeffB- yc) -r*r;
		a = 1+ CoeffK*CoeffK;
		b1 = 2*xc-2*CoeffK*(CoeffB-yc);	//-b
		tmp = b1*b1-4*a*c; 
		if (tmp<0)
			return 0;//no cross
		tmp = sqrt(tmp);	// sqrt
		x1 = (MLong)((b1+tmp)/(2*a));
		y1 = (MLong)(CoeffK*x1 + CoeffB);

		x2 = (MLong)((b1-tmp)/(2*a));
		y2 = (MLong)(CoeffK*x2 + CoeffB);
	}
	if (pCrossPT1!=MNull)
		pCrossPT1->x = x1,pCrossPT1->y=y1;
	if (pCrossPT2!=MNull)
		pCrossPT2->x = x2, pCrossPT2->y=y2;
	
	return 1;//one or two cross
}

MBool bInValidArc(MPOINT midPt, MPOINT arcPt, MDouble mainAngle, MDouble dValidAngle)
{
	MDouble dDiffAngle;
	MDouble dTargetAngle = vComputeAngle(arcPt.x-midPt.x, arcPt.y-midPt.y);
	if (mainAngle>V_PI)
		dDiffAngle = fabs(dTargetAngle - (mainAngle-V_PI));
	else
		dDiffAngle = fabs(dTargetAngle - (mainAngle+V_PI));
	
	dDiffAngle = MIN(dDiffAngle, 2*V_PI-dDiffAngle);

	return dDiffAngle<=dValidAngle;
}

//数学公式:   deltaY = deltaX * tan(theta)
//      deltaX = dist/(sqrt(1+k*k)
//deltaY = deltaX *tan(theta)
MVoid vGetPointWidthParam(MDouble dCoeffK, MBool bVert, 
	MPOINT ptAnchor, MPOINT *pPt1, MPOINT *pPt2, MDouble dist)
{
	MLong deltY, deltX;

	if (bVert)
	{
		deltX = 0;
		deltY = (MLong)dist;
	}
	else
	{
		deltX = (MLong)(dist/(sqrt(1+dCoeffK*dCoeffK)));
		deltY = (MLong)(deltX *dCoeffK);	// dCoeffK maybe >=0 or <0, so deltY maybe >=0 or <0
	}
	
	if (pPt1!=MNull)
	{
		pPt1->x = ptAnchor.x + deltX;
		pPt1->y = ptAnchor.y + deltY;
		}
	if (pPt2!=MNull)

		{
		pPt2->x = ptAnchor.x - deltX;
		pPt2->y = ptAnchor.y - deltY;
		}
}