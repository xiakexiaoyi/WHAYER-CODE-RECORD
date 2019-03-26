#include <math.h>

#include "lierrdef.h"
#include "lidebug.h"
#include "limath.h"
#include "bbgeometry.h"
#include "liimage.h"

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
				MFloat slope, MFloat intercept)
{
	MLong x, y;
	if (slope<1.0&&slope>-1.0)
	{
		for (x = 0; x<lWidth  ; x++)
		{
			MLong y = (MLong)(x*slope+intercept);
			*(pRltData + lRltLine * y + x) = DestFlag;
		}
	}
	else
	{
		for (y = 0; y<lHeight ; y++)
		{
			MLong x = (MLong)((y-intercept)/slope);
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


MVoid vFitLine(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical)
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
		*pCoeff2 = xSum / lListLen;
		return;
	}
	*pCoeff1 = (MDouble)(((MDouble)(xSum)*(ySum)-(MDouble)(xySum)*lListLen)*1.0/Zoom);
	*pCoeff2 = (MDouble)(((MDouble)(xySum)*(xSum)-(MDouble)(ySum)*(x2Sum))*1.0/Zoom);
}

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

MDouble vComputeAngle(MShort Gx, MShort Gy)
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

MLong vPointDistToLine(MPOINT ptOut, MPOINT ptLine1, MPOINT ptLine2)
{
	MLong lGround = vDistance1L(ptLine1, ptLine2);
	MLong lSide1 = vDistance1L(ptOut, ptLine1);
	MLong lSide2 = vDistance1L(ptOut, ptLine2);

	MLong lHalfPerimeter = (lGround+lSide1+lSide2)/2;

	MDouble area=sqrt(lHalfPerimeter*(lHalfPerimeter-lGround)*(lHalfPerimeter-lSide1)*(lHalfPerimeter-lSide2));

	return (MLong)(area*2/lGround);
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
    a = (H*D-E*G)/(C*G-D*D);
    b = (H*C-E*D)/(D*D-G*C);
    c = -(a*X1 + b*Y1 + X2 + Y2)/N;


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