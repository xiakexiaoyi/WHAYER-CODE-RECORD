#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "amcomdef.h"

#define vDrawLine				PX(vDrawLine)
#define vDrawLine2				PX(vDrawLine2)
//xsd
#define vDrawLine3				PX(vDrawLine3)

#define vGetPointMeanValue		PX(vGetPointMeanValue)
#define vLineTo					PX(vLineTo)
#define vDistance1L				PX(vDistance1L)
#define vDistance2L				PX(vDistance2L)
#define vDistance3L				PX(vDistance3L)
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
#define vProjectToLinePoint		PX(vProjectToLinePoint)
#define vCircleFitting			PX(vCircleFitting)
#define vDrawCircle			PX(vDrawCircle)
#define bOnCircle_EX		PX(bOnCircle_EX)
#define bInCircle_EX		PX(bInCircle_EX)
#define vComputeCrossPt	PX(vComputeCrossPt)
#define bInValidArc		PX(bInValidArc)
#define vComputeIntersactionAngle	PX(vComputeIntersactionAngle)
#define vGetPointWidthParam		PX(vGetPointWidthParam)
#define vGetPointMeanValueBtwPtWithWeight	PX(vGetPointMeanValueBtwPtWithWeight)
#define vGetPointMeanValueBtwPtWithDistWeight	PX(vGetPointMeanValueBtwPtWithDistWeight)

#define GaussianWeight	PX(GaussianWeight)

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

typedef struct  
{
    MDouble Coefk;
    MDouble Coefb;
    MDouble DAngle;//有向线段的方向
    MBool    bVertical;
//xsd
	MPOINT ptStart;
	MPOINT ptEnd;
	MPOINT ptMid;
}LINEPARAM;

typedef struct tag_circle_shape
{
	//3-parameter equation
	MLong xcoord;
	MLong ycoord;
	MLong lRadius;
}CIRCLE, *PCIRCLE;

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
MBool bInValidArc(MPOINT midPt, MPOINT arcPt, MDouble mainAngle, MDouble dValidAngle);
MBool bOnCircle_EX(MLong xc, MLong yc, MLong r, MLong i, MLong j, MLong Bias);
MBool bInCircle_EX(MLong xc, MLong yc, MLong r, MLong i, MLong j);
MVoid vGetLineParam(MPOINT pt1, MPOINT pt2, 
				   MDouble *A, MDouble *B, MDouble *C);

MLong vDistance1L(MPOINT Pt1, MPOINT Pt2);
MLong vDistance2L(MPOINT Pt1, MPOINT Pt2);
MLong  vDistance3L(MPOINT Pt1, MPOINT Pt2);
MDouble vDistance3D(MPOINT Pt1, MPOINT Pt2);	//xsd

MVoid vLineTo(MByte* pRltData, MLong lRltLine,
			  MLong lWidth, MLong lHeight,
			  MByte DestFlag,
			  MPOINT ptStart, MPOINT ptEnd);
MVoid vLineToImg(JOFFSCREEN* pImgSrc, MCOLORREF ref,  MPOINT ptStart, MPOINT ptEnd);

MLong vGetPointMeanValue(MByte* pData, MLong lDataLine,
				MLong lWidth, MLong lHeight,
				MPOINT ptStart, MPOINT ptEnd, MDouble dCoeffK, MDouble dCoeffB, MByte bVertical);

MLong vGetPointMeanValueBtwPt(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask,MLong *pPtNum);

MLong vGetPointMeanValueBtwPt_xsd(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MLong *pPtNum, MLong *lNum,MLong lCircleDist);

MVoid vDrawLine(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
			  MPOINT ptPass, MDouble slope, MLong lLength);

MVoid vDrawLine2(MByte* pRltData, MLong lRltLine,
				 MLong lWidth, MLong lHeight,
				 MByte DestFlag,
				MDouble slope, MDouble intercept);
//xsd
MVoid vDrawLine3(MByte* pRltData, MLong lRltLine, MLong lWidth,
						MLong lHeight, MByte destFlag, MDouble dCoeffK,
						MDouble dCoeffB, MBool bVert);
//
MVoid vDrawArrow(MByte* pRltData, MLong lRltLine,
				 MLong lWidth, MLong lHeight,
				 MByte DestFlag, MPOINT ptStart, MDouble direct, MLong lLength);

MVoid vPerpendPoint(MPOINT Point1, MPOINT Point2,
					MPOINT* pPoint3, MPOINT* pPoint4,
					MLong PerPendDist);
MVoid vFillMultiPolygon(MByte* pRltData, MLong lRltLine,
					   MLong lWidth, MLong lHeight,
					   MPOINT *PointList, MLong lPtNum,
					   MByte FillFlag);
MRESULT vFitLine(MPOINT* pPtLists, MLong lListLen, MDouble* pCoeff1, MDouble* pCoeff2, MBool *pbVertical);
MVoid vDrawRect(MByte* pRltData, MLong lRltLine,
				MLong lWidth, MLong lHeight,
				MByte DestFlag,
				MRECT *pRect);
MVoid vDrawRectImg(JOFFSCREEN* pImgSrc, 
				   MCOLORREF ref, MRECT *pRect);
MDouble vComputeAngle(MLong Gx, MLong Gy);
MVoid vComputeEigen(MATRIX_2D* pMatrix, EIGEN_2D *pEigen);
MVoid vDrawEllipce(MVoid *pShowData, MLong lShowLine, MLong lWidth, MLong lHeigth,
				  ELLIPCE* pEllipce);
MVoid vEllipceFit(MVoid *pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeigth,
				 ELLIPCE* pEllipce);
MVoid vDrawEllipceOnImg(JOFFSCREEN *pShowImg, ELLIPCE* pEllipce, MCOLORREF color, MLong thickness);
MBool vbInEllipce(ELLIPCE* pEllipce, MPOINT pt);

MDouble vCalculateTheta(MPOINT pt1, MPOINT pt2);
MDouble vComputeIntersactionAngle(MPOINT ptLeft, MPOINT ptMid, MPOINT ptRight);
MLong vPointDistToLine(MPOINT ptOut, MPOINT ptLine1, MPOINT ptLine2);
MLong vPointDistToLine2(MPOINT ptOut, MDouble dCoeffK, MDouble dCoeffB, MBool bVertical);
MDouble vPointDistToLine2D(MPOINT pt, MDouble dCoeffK, MDouble dCoeffB, MLong bVert); //xsd

MRESULT vCircleFitting(MPOINT *pPtList, MLong lPtLen, MLong *xc, MLong *yc, MLong *r);

MVoid vDrawCircle(MByte* pShowData, MLong lShowLine, MLong lWidth, MLong lHeight,
				  MLong xc, MLong yc, MLong r, MByte lForeFlag);

MLong vComputeCrossPt(MLong xc, MLong yc, MLong r, MDouble CoeffK, MDouble CoeffB, MBool bVert, 
	MPOINT *pCrossPT1, MPOINT* pCrossPT2);
MVoid vGetPointWidthParam(MDouble dCoeffK, MBool bVert, MPOINT ptAnchor, MPOINT *pPt1, MPOINT *pPt2, MDouble dist);
MLong vGetPointMeanValueBtwPtWithWeight(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MByte *pWeightData, MLong lWeightLine,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MLong *pPtNum);
MDouble vGetPointMeanValueBtwPtWithDistWeight(MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,
	MPOINT ptStart, MPOINT ptEnd, MByte Mask, MDouble *pPtWeightSum);
MVoid vProjectToLinePoint(MDouble CoeffK, MDouble CoeffB, MBool bVert, MPOINT ptOut, MPOINT *pProjectPoint);

MDouble GaussianWeight(MDouble Sigmal, MLong dist);
#ifdef __cplusplus
}
#endif


#endif //__GEOMETRY_H__
