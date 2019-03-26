#ifndef _SKIN_AREA_H_
#define _SKIN_AREA_H_

#include "licomdef.h"
#include "liimage.h"
#include "limask.h"
#include "areagraph.h"


// #define MIN_CB				(85/*-10*/)
// #define MAX_CB				(135/*+6*/)
// #define MIN_CR				(135/*-7*/)
// #define MAX_CR				(180/*+10*/)
#define BRIGHT_Y				200				//<=200
#define MIN_Y					17

#define IS_SKIN_COLOR(y, cb, cr)					\
		(((cb)-128)*1816<=((cr)-128)*1437			\
		&& ((cr)-128)*1437+587*((cb)-128)>=0)

#define IS_DIGITAL_RED_COLOR(y,cb,cr)	\
	((cr)>170)

//#define IS_DIGITAL_RED_COLOR(hue)	(((hue)>=0&&(hue)<30) || ((hue)>=180 && (hue)<240))
//#define IS_DIGITAL_GREEN_COLOR(hue)

// #define IS_SKIN_COLOR_STRICT(y, cb, cr)			\
// 	((cb) >= MIN_CB && (cb) < MAX_CB &&			\
// 	(cr) >= MIN_CR && (cr) < MAX_CR &&			\
// 	(y)  >= MIN_Y  &&  (cb) <= (cr))

typedef struct  
{
	MBool bWhiteBalance;
	MBool bRemoveNoSkinColor;
	MBool bRemoveAreaOnEdge;
	MBool bRemoveRectangleArea;
	MBool bRemoveComplexArea;	
	MBool bRemoveSmallArea;
	MBool bRemoveStrip;
	MBool bRemoveRelatedColor;	
}PARAM_SKIN_FILTER;

#ifdef __cplusplus
extern "C" {
#endif

//MVoid	AreaMerge(JINFO_AREA_GRAPH *pInfoGraph);
MVoid	AreaMerge(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize);
MVoid	SkinFilter(JINFO_AREA_GRAPH *pInfoGraph, const PARAM_SKIN_FILTER *pParam, 
				   MLong lMinAreaSize, MLong lCbOffset, MLong lCrOffset);
MVoid	FillHoleInSkin(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize);

MRESULT ConnectNeighborInSkin(MHandle hMemMgr, JINFO_AREA_GRAPH *pInfoGraph,
					const JLabelData* pLabelData, MLong lLabelLine,
					MLong lWidth, MLong lHeight);

MVoid	SkinFilterByGrayMask(JINFO_AREA_GRAPH *pInfoGraph, 
							 const JMASK* pMskGray, MLong lGrayThres, 
							 const JLabelData *pLabelData, MLong lLabelLine);

//MVoid	MergeFaceArea(JINFO_AREA_GRAPH *pInfoGraph);
MVoid	MergeSkinArea(JINFO_AREA_GRAPH *pInfoGraph, MLong lMinAreaSize);


#ifdef __cplusplus
}
#endif

#endif //_SKIN_AREA_H_
