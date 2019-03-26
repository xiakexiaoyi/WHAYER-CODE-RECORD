#ifndef _SKIN_EXTRACT_H_
#define _SKIN_EXTRACT_H_

#include "licomdef.h"
#include "colorsegment.h"
#include "areagraph.h"
#include "skinarea.h"
#include "limask.h"

typedef struct  
{
	MHandle		hMemMgr;	

	MBool bAreaMerge;
	MBool bFillHole;
	MBool bSaveFaceArea;


	///与word crop相关的参数
	MLong lMinWordLength;
	MLong lMaxWordLength;

	///将相邻的联结起来
	MBool bConnectNeighbor;

	MLong lMinAreaSize;
	MLong lCbOffset, lCrOffset;
	PARAM_SKIN_FILTER *pParamFilter;
	
	MCOLORREF* pcrSeeds;
	MLong lSeedsNum;
	MLong lGrayThres;

	JMASK *pMskFace;
	MBool bRemoveFaceArea;

	JINFO_AREA_GRAPH infoGraph;
	JLabelData*	pLabelData;
	MLong lWidth, lHeight;
}INFO_SKIN;

#ifdef __cplusplus
extern "C" {
#endif


MRESULT SE_Create(MHandle hMemMgr, INFO_SKIN *pSkiner);
MVoid	SE_Release(INFO_SKIN *pSkiner);

MRESULT SE_SkinExtract(INFO_SKIN *pSkiner, const JOFFSCREEN *pImg, 
					   JMASK *pMskSkin);
#ifdef ENABLE_PRIOR
MRESULT SE_SetPriorArea(INFO_SKIN *pSkiner, const JOFFSCREEN *pImg, 
						const MRECT* prtSkinAreas, MLong lSkinAreaNum);
#endif

#ifdef __cplusplus
}
#endif

#endif //_SKIN_EXTRACT_H_
