#ifndef _AREA_GRAPH_H_
#define _AREA_GRAPH_H_

#include "licomdef.h"

typedef struct tag_ptlist
{
	MPOINT pt;
	struct tag_ptlist *pNext;
}PTLIST;

typedef struct tag_ptlistHead
{
	PTLIST *ptHead;
	PTLIST *pTailNode;
}PTLISTHEAD;


//#define LIST_TAIL(pptlistHead)	(while(((PTLISTHEAD*)pptlistHead)->pTailNode//

typedef struct  
{
	MLong lBoundaryNum;	
	MLong lEdgeNum;
	MLong lSize;
	MLong lY, lCb, lCr;
	MLong lFlag;
	MRECT rtArea;
	MLong *pBoundaryNum;
	MLong *pGradientSum;

	//new added parameters
	MDWord *pDistanceWithBlobs;////square of distance
	MLong ptNum;
	//MLong lPtLen;
	PTLISTHEAD PtList;

	///对区域进行classify，有关classify的信息
	MLong lLabel;///分类
	MDouble dConfidence;///置信度
}JINFO_AREA;
#define SET_AREA_FLAG(pArea)	{JASSERT(ABS((pArea)->lFlag)<0x3FFFFFFF);(pArea)->lFlag+=0x40000000;}
#define IS_AREA_SET(pArea)		((pArea)->lFlag>0x0FFFFFFF)
#define CLEAR_AREA_FLAG(pArea)	{if(IS_AREA_SET(pArea)) (pArea)->lFlag-=0x40000000;}

typedef struct  
{
	JINFO_AREA *pInfoArea;
	MLong lLabelNum;
}JINFO_AREA_GRAPH;
#define CLEAR_GRAPH_FLAG(pGraph)	{MLong _i; for(_i=0;_i<(pGraph)->lLabelNum;_i++) CLEAR_AREA_FLAG((pGraph)->pInfoArea+_i);}

typedef MShort	JLabelData;

#define GET_VALID_LABEL(pInfoArea, lImgLabel)								\
	(pInfoArea[lImgLabel].lFlag >= 0 ? (lImgLabel) : (-pInfoArea[lImgLabel].lFlag-1))

#define GET_COMPLEX(lBoundaryNum, lSize)		((lBoundaryNum)<=0x7FFF		\
	? SQARE(lBoundaryNum)/(lSize)											\
	: (MLong)(((MUInt64)(lBoundaryNum)*(lBoundaryNum))/(MUInt64)(lSize)))	
#define GET_COMPLEX_EX(lBoundaryNum, lSize, lEdge)							\
	(GET_COMPLEX(lBoundaryNum, lSize)*((lEdge)+(lBoundaryNum)/4)/((lBoundaryNum)/4))

#define IS_STRIP_AREA(pInfoArea)											\
	((pInfoArea->rtArea.right-pInfoArea->rtArea.left)>(pInfoArea->rtArea.bottom-pInfoArea->rtArea.top)*8	\
	||(pInfoArea->rtArea.bottom-pInfoArea->rtArea.top)>(pInfoArea->rtArea.right-pInfoArea->rtArea.left)*8	/*>=8*/\
	||pInfoArea->lSize*8<(pInfoArea->rtArea.right-pInfoArea->rtArea.left)*(pInfoArea->rtArea.bottom-pInfoArea->rtArea.top))

#define MIN_DISTANCE_BW_BLOB						4

#define VALID_NEIGHBOR_BLOB(lNumPixels)				MAX((lNumPixels)/16,MIN_DISTANCE_BW_BLOB)
#define VALID_NEIGHBOR_BOUNDARY(lBoundaryNum)		((lBoundaryNum)/16)
#define MIN_AREA_SIZE(imgSize)						MAX((imgSize)/(128*128), 11)

///
#define MAX_MEAN_OFFSET							35///20%保险一点，尤其是1/4/7与其他的数字会有较大的差别
#define MAX_MEAN_OFFSET_Y_POS					30///Y方向上的位置偏移
#define MIN_RELATIVE_DISTANCE						10
#define MAX_RELATIVE_DISTANCE						100///valid value[0,100]

//#define VALID_RELATIVE_DISTANCE(x,basevalue)			(((x) > MAX(MIN_RELATIVE_DISTANCE*(basevalue)/100, 1))\
//											&&((x)<(MAX_RELATIVE_DISTANCE*(basevalue)/100)))
#define VALID_RELATIVE_DISTANCE(x,basevalue)			(((x) > MIN_DISTANCE_BW_BLOB)\
										&&((x)<(MAX_RELATIVE_DISTANCE*(basevalue)/100)))

#define VALID_RELATIVE_DISTANCE_EX(x, basevalue, minVal)	(((x) > minVal)\
										&&((x)<(MAX_RELATIVE_DISTANCE*(basevalue)/100)))

#define VALID_STD_VARIANCE_OFFSET(lBaseValue, number)	(((lBaseValue)*MAX_MEAN_OFFSET/100)/sqrt(1.0*number))			//(MAX_MEAN_OFFSET)

#define MAX_RT_OVERLAPPINGRATE					50	////valid value[0,100]
#define MAX_COLOR_DIST								1024*8///目前来看32是比较稳妥的一个数字，16的话意味着更多的碎片
#define MAX_AREA_COMPLEX							128
#define MIN_AREA_COMPLEX							16
#define MAX_AREA_GRADIENT							32
#define MIN_AREA_GRADIENT							2


#define MergeArea	PX(MergeArea)
#define RemoveArea	PX(RemoveArea)
#define UpdateGraph	PX(UpdateGraph)
#define UpdateLabel	PX(UpdateLabel)
#define RemoveMargin	PX(RemoveMargin)
#define NeighborWithMinDist	PX(NeighborWithMinDist)
#define NeighborWithMinDist1 PX(NeighborWithMinDist1)
#define NeighborNum	PX(NeighborNum)
#define IsOnEdge		PX(IsOnEdge)
#define BlobDistCalulate	PX(BlobDistCalulate)

#ifdef __cplusplus
extern "C" {
#endif

MRESULT BlobDistCalulate(MHandle hMemMgr, const JINFO_AREA_GRAPH* pInfoGraph,
					const JLabelData* pLabelData, MLong lLabelLine,
					MLong lWidth, MLong lHeight);

MVoid MergeArea(JINFO_AREA* pInfoArea, MLong lAreaNoFrom, MLong lAreaNoTo, 
				MLong lLabelNum);
MVoid RemoveArea(JINFO_AREA* pInfoArea, MLong lLabel, MLong lLabelNum);

MVoid UpdateGraph(JINFO_AREA_GRAPH *pInfoGraph);
MVoid UpdateLabel(MHandle hMemMgr, 
				  JINFO_AREA_GRAPH* pInfoGraph, JLabelData *pLabelData, 
				  MLong lLabelLine, MLong lWidth, MLong lHeight);

MVoid RemoveMargin(JINFO_AREA_GRAPH *pInfoGraph, MLong lWidth, MLong lHeight);

//////////////////////////////////////////////////////////////////////////
MLong NeighborWithMinDist(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, 
						  MLong lBoundaryThres, MLong *plMinDist);

MLong NeighborWithMinDist1(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, MLong *plMinDist);

MLong NeighborNum(const JINFO_AREA_GRAPH* pInfoGraph, MLong lLabelSeed, 
				  MLong lBoundaryThres);
MBool IsOnEdge(const JINFO_AREA *pInfoArea);

#ifdef ENABLE_DEBUG
MVoid CheckAreaGraph(const JOFFSCREEN *pImg,
					 const JINFO_AREA_GRAPH *pInfoGraph, 
					 JLabelData* pLabelData, MLong lLabelLine);
MVoid DrawCurrentArea(const JLabelData* pLabelData, MLong lLabelLine,
					  MLong lWidth, MLong lHeight,  
					  const JINFO_AREA* pInfoArea, MLong lLabelCur, 
					  const MTChar *strName);
#else
#define CheckAreaGraph		(MVoid)
#define DrawCurrentArea		(MVoid)
#endif
	
	
#ifdef __cplusplus
}
#endif

#endif //_AREA_GRAPH_H_
