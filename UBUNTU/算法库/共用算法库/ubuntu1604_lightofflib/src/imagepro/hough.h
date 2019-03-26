#ifndef __HOUGH_H__
#define __HOUGH_H__

#include "licomdef.h"
#include "litrimfun.h"

typedef struct tag_LinePolar
{
    MDouble* rho;
    MDouble* angle;
    MLong lLineNum;
}LinePolar;

typedef struct tag_LineSegment
{
    MPOINT* StartPt;
    MPOINT* EndPt;
    MLong lLineNum;
}LineSegment;

#define HoughTransform			PX(HoughTransform)
#define ProbabilisticHoughTransform			PX(ProbabilisticHoughTransform)

#ifdef __cplusplus
extern "C" {
#endif

    //Binary image input
    MRESULT HoughTransform(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
        MLong lWidth, MLong lHeight, MByte ForeFlag,
        MDouble rho, MDouble theta, MLong lThreshold, MLong lLineMax, 
        LinePolar* pLines);

    MRESULT ProbabilisticHoughTransform(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
        MLong lWidth, MLong lHeight, MByte ForeFlag,
        MDouble rho, MDouble theta, MLong lThreshold, MLong lLineLength, MLong lMaxLineGap, 
        MLong lLinesMax, LineSegment* pLines);


#ifdef __cplusplus
}
#endif


#endif//__HOUGH_H__