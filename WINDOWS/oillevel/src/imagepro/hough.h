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
#define ProbabilisticHoughTransform_ls			PX(ProbabilisticHoughTransform_ls)
#define ProbabilisticHoughTransform_level			PX(ProbabilisticHoughTransform_level)

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

	MRESULT ProbabilisticHoughTransform_ls(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
		MLong lWidth, MLong lHeight, MByte ForeFlag,
		MDouble rho, MDouble theta, MLong lThreshold, MLong lLineLength, MLong lMaxLineGap, MLong lMinLineDistance,
		MLong lLinesMax, LineSegment* pLines);
	MRESULT ProbabilisticHoughTransform_level(MHandle hMemMgr, MByte* pBinData, MLong lSrcLine,
		MLong lWidth, MLong lHeight, MByte ForeFlag,
		MDouble rho, MDouble theta, MLong lThreshold, MLong lLineLength, MLong lMaxLineGap, 
		MLong lLinesMax, LineSegment* pLines);
	double dis_3D(MPOINT* a,MPOINT* b,MPOINT* s);
	MDouble dis_line(MDouble k,MDouble b,MPOINT* s);
#ifdef __cplusplus
}
#endif


#endif//__HOUGH_H__