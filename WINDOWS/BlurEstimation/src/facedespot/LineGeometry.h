#ifndef __LINE_GEOMETRY_H__
#define __LINE_GEOMETRY_H__

#include "licomdef.h"

#define vLineTo					PX(vLineTo)
#define vDistance1L				PX(vDistance1L)
#define vFitLine				PX(vFitLine)
#define vPerpendPoint			PX(vPerpendPoint)
#define vFillMultiPolygon		PX(vFillMultiPolygon)

#ifdef __cplusplus
extern "C"{
#endif

MLong vDistance1L(MPOINT Pt1, MPOINT Pt2);
MVoid vLineTo(MByte* pRltData, MLong lRltLine,
			  MLong lWidth, MLong lHeight,
			  MByte DestFlag,
			  MPOINT ptStart, MPOINT ptEnd);

MVoid vPerpendPoint(MPOINT Point1, MPOINT Point2,
					MPOINT* pPoint3, MPOINT* pPoint4,
					MLong PerPendDist);

MVoid vFillMultiPolygon(MByte* pRltData, MLong lRltLine,
					   MLong lWidth, MLong lHeight,
					   MPOINT *PointList, MLong lPtNum,
					   MByte FillFlag);
MVoid vFitLine(MPOINT* pPtLists, MLong lListLen, MFloat* pCoeff1, MFloat* pCoeff2, MBool *pbVertical);
#ifdef __cplusplus
}
#endif


#endif //__LINE_GEOMETRY_H__
