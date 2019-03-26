#ifndef _LI_SORT_H_
#define _LI_SORT_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************/
#define FindMidian		PX(FindMidian)
#define FindMax			PX(FindMax)
#define FindMaxIndex	PX(FindMaxIndex)
#define FindHistMidian	PX(FindHistMidian)
/***********************************************************/

MLong FindMidian(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData);
MLong FindMax(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData);
MLong FindMaxIndex(MVoid *pVectData, MLong lVectLen, JTYPE_DATA typeData);

MLong FindHistMidian(MVoid *pHistData, MLong lHistLen, JTYPE_DATA typeData);

#ifdef __cplusplus
}
#endif

#endif