#include <math.h>

#include "lidebug.h"
#include "limath.h"
#include "LineGeometry.h"

MLong vDistance1L(MPOINT Pt1, MPOINT Pt2)
{
	return ABS(Pt1.x-Pt2.x)+ABS(Pt1.y-Pt2.y);
	//return sqrt(SQARE(Pt1.x-Pt2.x)+SQARE(Pt1.y-Pt2.y)); 
}

MVoid vLineTo(MByte* pRltData, MLong lRltLine,
			  MLong lWidth, MLong lHeight,
			  MByte DestFlag,
			  MPOINT ptStart, MPOINT ptEnd)
{
	MLong x, y;
	MLong lStart, lEnd;

	MFloat k1, b1;
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

MVoid vPerpendPoint(MPOINT Point1, MPOINT Point2,
					MPOINT* pPoint3, MPOINT* pPoint4,
					MLong PerPendDist)
{
	MFloat k1, k2, b1, b2, bExtra;
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
		
		PT3.x = (-(b2-b1-bExtra)/(k2-k1)+0.5);
		//PT3.y = (k2*(PT3.x)+b2+0.5);
		PT3.y = k1*PT3.x+b1+bExtra;
		
		PT4.x = (-(b2-b1+bExtra)/(k2-k1)-0.5);
		//PT4.y = (k2*PT4.x+b2-0.5);
		PT4.y = k1*PT4.x+b1-bExtra;
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


MVoid vFitLine(MPOINT* pPtLists, MLong lListLen, MFloat* pCoeff1, MFloat* pCoeff2, MBool *pbVertical)
{
	MLong Zoom=0;
	MLong i;
	MLong xSum = 0, x2Sum = 0, ySum =0, xySum = 0;
	JASSERT(pbVertical);

	*pbVertical = MFalse;
	for(i=0; i<lListLen; i++)
	{
		xSum += pPtLists[i].x;
		x2Sum += pPtLists[i].x*pPtLists[i].x;
		ySum += pPtLists[i].y;
		xySum += pPtLists[i].x*pPtLists[i].y;
	}
	
	Zoom = (xSum)*(xSum)- (x2Sum)*lListLen;
	if (Zoom==0) 
	{
		*pbVertical = MTrue;
		return;
	}
	*pCoeff1 = (MFloat)(((xSum)*(ySum)-(xySum)*lListLen)*1.0/Zoom);
	*pCoeff2 = (MFloat)(((xySum)*(xSum)-(ySum)*(x2Sum))*1.0/Zoom);
}