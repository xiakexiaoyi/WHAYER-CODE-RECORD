#ifndef _COLOR_SEGMENT_H_
#define _COLOR_SEGMENT_H_

#include "licomdef.h"
#include "limask.h"
#include "areagraph.h"

#ifdef __cplusplus
extern "C" {
#endif



MLong	GetColorDist(MLong cr1, MLong cr2, MLong cr3, MCOLORREF crCenter);

MRESULT CreateAreaGraphByImg(MHandle hMemMgr, const JOFFSCREEN* pImg, 
					 JLabelData* pLabelData, MLong lLabelLine, 
					 JINFO_AREA_GRAPH *pInfoGraph);
MVoid   ReleaseAreaGraph(MHandle hMemMgr, JINFO_AREA_GRAPH *pInfoGraph);


MVoid	ShowLabelMask(const JINFO_AREA_GRAPH* pInfoGraph, 
					  const JLabelData* pLabelData, MLong lLabelLine, 
					  MLong lWidth, MLong lHeight, JMASK *pMask);
MVoid	ShowImgByMask(const JOFFSCREEN *pImgSrc, JOFFSCREEN *pImgRlt, JMASK *pMskImg, 
						MBool bShowBoundary);


#ifdef __cplusplus
}
#endif

#endif //_COLOR_SEGMENT_H_
