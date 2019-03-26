#ifndef _SORT_H_
#define _SORT_H_

#include "licomdef.h"
#include "litrimfun.h"

#ifdef __cplusplus
extern  "C"{
#endif

MRESULT QuickSort(MHandle hMemMgr,MVoid *pbase, MLong lTotal_elems, 
				  MLong lSize, MLong (*cmp)(const MVoid*, const MVoid*));


#ifdef __cplusplus
}
#endif

#endif