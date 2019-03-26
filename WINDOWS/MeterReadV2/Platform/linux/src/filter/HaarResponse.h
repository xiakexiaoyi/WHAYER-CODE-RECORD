#ifndef HAAR_RESPONSE_H
#define HAAR_RESPONSE_H
#include"amcomdef.h"
#include "liblock.h"

#include "bbgeometry.h"
#include "blobfilter.h" 

#ifdef __cplusplus
extern "C" {
#endif

//#define MAX_CIRCLE_CANDIDATES				
//#define MAX_PTNUM_PER_CIRCLE_CANDIDATES	


/***********************************************
Haar response for the analog meter 
ClassNumber : 0;
			 _________________
			|		| |		  |
			|		| |		  |
			|		| |		  |
			|       | |       |
			|_______| |_______|
			|_________________|
			|				  |
			|_________________|
ClassNumber : 1;
			 _________________
			|		| |		  |
			|		| |		  |
			|		| |		  |
			|       | |       |
			|_______| |_______|
			|_________________|
ClassNumber : 2;	
		  _____________
		  |	| |		  |
		  |	| |		  |
		  |	| |		  |
		  |	| |       |
		  |	| |_______|
		  |	|_________|
		  |___________|
ClassNumber :3;
			_____________
			|		| | | 
			|		| | |
			|		| | |
			|       | | |
			|_______| | |
			|_________| |
			|___________|

*************************************************/

	typedef struct  
	{
		MLong w;//Haar模板的宽度  width
		MLong h;//Haar模板的高度  height
		MLong lHaarWidth;
		MBool bWhiteBackground;

		MLong lClassNumber;
		//MLong lMethod;
		MLong *pAngleList;
		MLong lAngleNumber;

	} HaarParam, *PHaarParam;

	//积分图结构体
	typedef struct
	{
		MUInt32 *pData;
		MLong lWidth, lHeight, lBlockLine;
	}IntegrogramImage,*PIntegrogramImage;


//TEMPLATE_TYPE_BIT该位为1表示模板1:3:1
//TEMPLATE_TYPE_BIT该位为0表示模板1:1:1
#define TEMPLATE_TYPE_LONG_BIT		0x80000000
#define TEMPLATE_TYPE_BYTE_BIT		0x80
	typedef struct
	{
		MLong *pData;
		MLong lWidth, lHeight, lBlockLine;
	}ResponseResult;
	
	typedef struct
	{
		MPOINT startPoint,endPoint;
		MDouble length;
		MLong Angle;
		MDouble p;
	}LineSegmentPoint;

	typedef struct 
	{
		MPOINT ptPoint;
		MByte   ptValue;
		MDouble dPtAngle;
	}PTINFO;

	typedef struct 
	{
		LINEPARAM lineParam;
		PTINFO *pPtinfo;
		MLong lConfidence;//候选直线的置信度,0-255
		MLong lPtNum;//当前的点数
		MLong lMaxPtNum;//最大的点数
		MLong lMaxLength;	//备选指针线最大连续响应值非零长度
	}LINE_INFO;

	typedef struct
	{
		LINE_INFO *pLineInfo;//直线信息
		MVoid *pMem;//used to store the allocated memory
		MLong lLineNum;//当前的候选直线数量
		MLong lMaxLineNum;//最大的候选直线数量
	}LINE_SET_INFO;

	typedef struct 
	{
		// circle
		CIRCLE  circleParam;//圆参数
		PTINFO *pPtinfo;//圆点列表
		MLong lPtNum;//当前的点数
		MLong lMaxPtNum;//最大的点数

		MLong lConfidence;//候选圆的置信度
		MDouble dFitRadio;	//拟合度
		
		// line
//		LINE_INFO *pLineInfo;
//		MLong lLineNum;
//		MLong lLineMaxNum;
		LINE_INFO lineInfo;//这里只存储一个，不存多余的
		MLong lLineNum;
	}CIRCLE_INFO;

	typedef struct
	{
		CIRCLE_INFO *pCircleInfo;//圆信息
		MVoid *pMem;//used to store the allocated memory
		MLong lParamNum;//当前的候选圆个数
		MLong lMaxParamNum;//最大的候选圆个数
	}CIRCLE_SET_INFO;

	typedef struct 
	{
		MLong lNumPts;
		MLong lMaxArcAngle;

		MBool bWhiteBackground;
		MLong lHarrWidth;
		MLong lPicBlurLevel;

		MBool bUpdated;//
		MBool bExistCircle;
		CIRCLE_INFO *pCircleInfo;
	}PARAM_INFO;
	
	
//xsd
	typedef struct
	{
		MPOINT ptPos;
		MDouble angleVal;
	}QUADRANT;

	typedef enum 
	{
		OIL_METER = 0,
		OIL_TEMPERATURE,
		VOLT_METER,
		DISCHARGE_METER,
		SF6_GAS_METER,
		SWITCH_METER,
		QUALITROL_METER,
		LEAKAGE_CURRENT_METER,
		UNKNOWN_METER
	}HYAR_METER_TYPE;

	typedef enum
	{
		PIC_LOW_BLUR = 0,
		PIC_HIGH_BLUR,
		PIC_OTHER_BLUR
	}BLUR_LEVEL;

//////////////////////////////////////////////////////////////////////////////////
#define ReadNumber		PX(ReadNumber)
#define ReadNumber2		PX(ReadNumber2)
#define GetIndex		            PX(GetIndex)
#define GetLineInfo               PX(GetLineInfo)
#define CreateCircleInfo		PX(CreateCircleInfo)
#define FreeCicleInfo			PX(FreeCicleInfo)
#define PTInfoCmp			PX(PTInfoCmp)

#define HaarResponseAngle PX(HaarResponseAngle)
#define StatisticNonZeroValue PX(StatisticNonZeroValue)

//#define GetLineLocation2		PX(GetLineLocation2)
//#define GetSwitchLineLocation   PX(GetSwitchLineLocation)
	//////////////////////////////////////////////////////////////////////////
//	MRESULT GetPixelLocation(MHandle hMemMgr,BLOCK*pImage,MPOINT *pointGood,MLong *pNum);
	MRESULT ReadNumber(MHandle hMemMgr,BLOCK *image,MPOINT *point,MLong lNUmber,MPOINT startPoint,MPOINT endPoint,MDouble *numberData,MDouble *result);
	MRESULT ReadNumber2(MHandle hMemMgr,BLOCK *image,MPOINT *point,MLong lNUmber,MDouble k,MDouble b,MDouble *numberData,MDouble *result);
	MVoid GetIndex(MDouble *DoubleImageData,MLong n,MLong xc,MLong yc,MLong rc,MLong lWidth,MLong lHeight,MPOINT *pStart,MPOINT *pEnd);
	MRESULT GetLineInfo(MHandle hMemMgr, BLOCK* pImage, BLOCK *pMask, PARAM_INFO *pParam);
    
	MRESULT CreateCircleInfo(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo, MLong lMaxPtNum);
	MVoid FreeCicleInfo(MHandle hMemMgr, CIRCLE_INFO *pCircleInfo);
	MLong PTInfoCmp( const MVoid* p1, const MVoid* p2);

	////////////
	MRESULT HaarResponseAngle(MHandle hMemMgr ,BLOCK *pImage, PHaarParam pParam,BLOCK *pResponseResultImage, BLOCK *pResponseAngle);
	MRESULT StatisticNonZeroValue(MHandle hMemMgr, MByte *pData, MLong lDataLine, MLong lWidth, MLong lHeight,MByte lHorizontalVal,
									MVoid* pMem, MLong length, MLong lPercentage, MLong *retVal);
	MRESULT CreateCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSet, MLong lMaxCircleCandidate,MLong lMaxPtNumPerCircle);
	MVoid FreeCircleSet(MHandle hMemMgr, CIRCLE_SET_INFO *pCircleSetInfo);
	MRESULT RANSAC_CIRCLE(MHandle hMemMgr, BLOCK *pHaarResponse, BLOCK* pHaarAngleResponse,MPOINT *pPtList, MLong lPtNum, 
							CIRCLE_SET_INFO *pCircleSetInfo,MPOINT *pTemp,MLong lTmpNum, MLong lRadiusCons, PARAM_INFO *pParam);
	MVoid MergeSet(MVoid *pDataList,MLong elemSize, MLong *pListSize, 
					MLong (*cmp)(const MVoid*, const MVoid*), MLong (*replace)(const MVoid*, const MVoid*));

	MLong CircleInfoCmp( const MVoid* p1, const MVoid* p2);
	MLong CircleInfoReplace(const MVoid *p1, const MVoid* p2);
	MLong CircleInfoConfidenceCmp(const MVoid* p1, const MVoid* p2);
	MVoid CircleInfoCpy(CIRCLE_INFO *pSrcCircle, CIRCLE_INFO *pDstCircle);

	MRESULT CreateLineSet(MHandle hMemMgr, LINE_SET_INFO *pLineSet, MLong lMaxLineCandidate, MLong lMaxPtNumPerLine);
	MLong LineInfoCmp(const MVoid* p1, const MVoid* p2);
	MLong LineInfoReplace(const MVoid *p1, const MVoid* p2);
	MLong PTInfoCmp_X( const MVoid* p1, const MVoid* p2);
	MLong PTInfoCmp_Y( const MVoid* p1, const MVoid* p2);
	MLong LineInfoConfidenceCmp(const MVoid* p1, const MVoid* p2);

	MVoid AdjustLinePosition(MByte *pResponseData, MLong lRspLine, MLong lWidth, MLong lHeight,
							LINEPARAM *pLineParam,MPOINT *pptAnchor,
							MPOINT* pptStart, MPOINT* pptEnd);
//*********************xsd circleSort ***********************************************
	MLong CmpValUp( const MVoid* p1, const MVoid* p2);
	MLong CmpValDown( const MVoid* p1, const MVoid* p2);
	MLong circleSort(MHandle hMemMgr, MPOINT *ptList, MLong ptLen, MPOINT center, MLong startQuadrant, MBool isClockWise);

//=======================================================================================
	MRESULT CalcCircleCenter(MHandle hMemMgr, BLOCK *pHaarAngle, PJGSEEDS seedsList, MLong *pSeedsAngle, MPOINT *ptCenter, MLong lRadius);
	MRESULT LineVoteInFlag(MLong *pFlag, MLong lWidth, MLong lHeight, MLong lCurX, MLong lCurY, MLong lAngle);
	MRESULT AdjustCenter(BLOCK voteMap, MPOINT *ptCenter, MLong lRadius, MLong lThreshold);
	MRESULT CalcCircleRadius(MHandle hMemMgr, BLOCK *pHarrResponse, PJGSEEDS seedsList, MLong *pSeesAngle, MPOINT ptCenter, MLong *lRadius);

	/*MDouble GetResultMethod1(MPOINT *pPtPos, MDouble *pPtVal, MDouble *pPtDist, LINEPARAM *pLineParam, MLong lPtNum);
	MDouble GetResultMethod2(MPOINT *pPtPos, MDouble *pPtVal, MDouble *pPtDist, LINEPARAM *pLineParam, MLong lPtNum);
	MDouble GetResultMethod3(MPOINT *pPtPos, MDouble *pPtVal, MDouble *pPtDist, LINEPARAM *pLineParam, MLong lPtNum);*/
	MDouble GetSwitchStatus(MPOINT *pPtPos, MDouble *pPtVal, LINEPARAM *pLineParam, MLong lPtNum);
	MDouble CalcMeterResult(MPOINT *pPtPos, MDouble *pPtVal, LINEPARAM *pLineParam, MLong lPtNum);

//======================= line location ========================================================
	MRESULT GetCircleRing(BLOCK *pHaarResponse, CIRCLE *pCircleParam, MLong lDelta, MLong lMeterType);

	MDouble Point2Line(MPOINT pt, MDouble dCoeffK, MDouble dCoeffB, MLong bVert);
	MLong calcLineCrossCirclePt(CIRCLE *pCircleParam, LINEPARAM *pLineParam, MPOINT *ptCross1, MPOINT *ptCross2);
	MLong calcExternLinePtNum(BLOCK *pHaarResponse, LINEPARAM *pLineParam, CIRCLE *pCircleParam, MLong lThreshold, MLong lMaxNumPt);

	MRESULT getRedDiffImage(MByte *pSrc, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getBlackDiffImage(MByte *pSrc, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getBlackDiffImage2(MHandle hMemMgr, BLOCK *pSrcImage, BLOCK *pDiffImage, MLong *pHist);
	MRESULT calcExternImg(MByte *pSrc, MLong lSrcStride, MByte *pDst, MLong lDstStride, MLong lWidth, MLong lHeight);
	MRESULT getRedDiffImage_yuv(MByte *pSrcY, MByte *pSrcV, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getBlackDiffImage_yuv(MByte *pSrcY, MByte *pSrcV, MByte* pSrcU, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getRedDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getBlackDiffImage2_yuv(MByte *pSrcY, MByte *pSrcV, MByte* pSrcU, MLong lSrcStride, BLOCK *pDiffImage, MLong *pHistArray);
	MRESULT getObjImage_yuv(MHandle hMemMgr, JOFFSCREEN srcImage, BLOCK *pDstBlockY, BLOCK *pDstBlockU, BLOCK *pDstBlockV, MLong lObjLeft, MLong lObjTop);

	MRESULT calcAngleMat(MByte *pSrc, MLong lWidth, MLong lHeight, MLong lStride, MDouble *pDstAngle);
	MRESULT GetLineLocation3(MHandle hMemMgr,BLOCK *pImage, PARAM_INFO *pParam, MLong lMeterType);

	MLong isPtInImage(MPOINT pt, MLong lWidth, MLong lHeight);

	MRESULT strechImage(BLOCK *pImage);
	MRESULT inverseImg(BLOCK *pImage);

	MLong calcMaxContinuityLength(MByte *pData, MLong lStride, MLong lWidth, MLong lHeight, MPOINT ptCross1, MPOINT ptCross2, MLong lMask);
	MLong LineInfoLengthCmp(const MVoid* p1, const MVoid* p2);
	MRESULT appendSearch(MByte *pSrcData, MLong lSrcLine, MByte *pRltData, MLong lRltLine, MLong lWidth,
				MLong lHeight, MPOINT tmpSeed,  LINEPARAM lineParam, MLong lStepThres, MPOINT *pRltSeed);
	MRESULT appendSeeds(MByte *pSrcData, MLong lSrcLine, MByte *pRltData, MLong lRltLine, MByte *pAngleData,
					MLong lAngleLine, MLong lWidth, MLong lHeight, MLong lThreshold, MLong lRadius, JGSEED *pSeeds);
					
	MRESULT searchPt(BLOCK *pSrcImg, MPOINT seedPt, MLong lDirection, MLong lSearchThreshold, MPOINT *dstPt, MLong *lLength);
	MRESULT getMaskResponse(BLOCK *pResponseImage, BLOCK *pMaskImage);

#ifdef __cplusplus
}
#endif
#endif