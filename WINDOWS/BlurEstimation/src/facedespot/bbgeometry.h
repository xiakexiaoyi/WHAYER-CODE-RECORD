#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "amcomdef.h"

#define vDrawLine				PX(vDrawLine)
#define vDrawLine2				PX(vDrawLine2)
#define vLineTo					PX(vLineTo)
#define vDistance1L				PX(vDistance1L)
#define vDistance2L				PX(vDistance2L)
#define vFitLine				PX(vFitLine)
#define vPerpendPoint			PX(vPerpendPoint)
#define vFillMultiPolygon		PX(vFillMultiPolygon)
#define vDrawRect				PX(vDrawRect)
#define vComputeAngle			PX(vComputeAngle)
#define vComputeEigen			PX(vComputeEigen)
#define vDrawEllipce			PX(vDrawEllipce)
#define vEllipceFit				PX(vEllipceFit)
#define vbInEllipce				PX(vbInEllipce)
#define vDrawEllipceOnImg		PX(vDrawEllipceOnImg)
#define vDrawArrow				PX(vDrawArrow)
#define vLineToImg				PX(vLineToImg)
#define vDrawRectImg			PX(vDrawRectImg)
#define vGetLineParam			PX(vGetLineParam)
#define vCalculateTheta			PX(vCalculateTheta)
#define vPointDistToLine		PX(vPointDistToLine)
#define vCircleFitting			PX(vCircleFitting)

typedef struct tag_matrix
{
// 	MLong lRows;
// 	MLong lCols;
	union
	{
		MLong ld[2][2];
		MDouble fd[2][2];
	}data;
}MATRIX_2D;

typedef struct tag_eigen
{
	//MDouble *pEigenVal;
	MDouble EigenVal[2];
	MDouble EigenVec[2][2];
}EIGEN_2D;

typedef struct tag_ellipce_shape
{
	//5-parameter equation
	MLong xcoord;
	MLong ycoord;
	MLong semiWidth;
	MLong semiHeight;
	MDouble alpha;// -- relative to horizontal
}ELLIPCE, *PELLIPCE;

typedef struct tag_vhistogram
{
	//
	MDouble *m_lookup;
	MLong lLuminBinNum;
	MLong lColorBinNum;
	MLong lLuminBinSize;
	MLong lColorBinSize;

	MLong lLookupBinNum;
}VHIST;

#ifdef __cplusplus
extern "C"{
#endif

MVoid vGetLineParam(MPOINT pt1, MPOINT pt2, 
				   MDouble *A, MDouble *B, MDouble *C);

MLong vDistance1L(MPOINT Pt1, MPOINT Pt2);
MLong vDistance2L(MPOINT Pt1, MPOINT Pt2);
MVoid vLineTo(MByte* pRltData, MLong lRltLine,
			  MLong lWidth, MLong lHeight,
			  MByte DestFlag,
			  MPOINT ptStart, MPOINT ptEnd);
MVoid vLineToImg(JOFFSCREEN* pImgSrc, MCOLORREF ref,  MPOINT ptStart, MPOINT ptEnd);
MVoid vDrawLine(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
			  MPOINT ptPass, MDouble slope, MLong lLength);

MVoid vDrawLine2(MByte* pRltData, MLong lRltLine,
				 MLong lWidth, MLong lHeight,
				 MByte DestFlag,
				MFloat slope, MFloat intercept);

//
MVoid vDrawArrow(MByte* pRltData, MLong lRltLine,
				 MLong lWidth, MLong lHeight,
				 MByte DestFlag, MPOINT ptStart, MDouble direct, MLong lLength);
//

MVoid vPerpendPoint(MPOINT Point1, MPOINT Point2,
					MPOINT* pPoint3, MPOINT* pPoint4,
					MLong PerPendDist);
MVoid vFillMultiPolygon(MByte* pRltData, MLong lRltLine,
					   MLong lWidth, MLong lHeight,
					   MPOINT *PointList, MLong lPtNum,
					   MByte FillFlag);
MVoid vFitLine(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical);
MVoid vDrawRect(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
				MRECT *pRect);
MVoid vDrawRectImg(JOFFSCREEN* pImgSrc, 
				   MCOLORREF ref, MRECT *pRect);
MDouble vComputeAngle(MShort Gx, MShort Gy);
MVoid vComputeEigen(MATRIX_2D* pMatrix, EIGEN_2D *pEigen);
MVoid vDrawEllipce(MVoid *pShowData, MLong lShowLine, MLong lWidth, MLong lHeigth,
				  ELLIPCE* pEllipce);
MVoid vEllipceFit(MVoid *pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeigth,
				 ELLIPCE* pEllipce);
MVoid vDrawEllipceOnImg(JOFFSCREEN *pShowImg, ELLIPCE* pEllipce, MCOLORREF color, MLong thickness);
MBool vbInEllipce(ELLIPCE* pEllipce, MPOINT pt);

MDouble vCalculateTheta(MPOINT pt1, MPOINT pt2);
MLong vPointDistToLine(MPOINT ptOut, MPOINT ptLine1, MPOINT ptLine2);

MRESULT vCircleFitting(MPOINT *pPtList, MLong lPtLen, MLong *xc, MLong *yc, MLong *r);

MVoid vDrawCircle(MByte* pShowData, MLong lShowLine, MLong lWidth, MLong lHeight,
				  MLong xc, MLong yc, MLong r, MByte lForeFlag);

#ifdef __cplusplus
}
#endif


#endif //__GEOMETRY_H__
