/*!
* \file Liimfilter.h
* \brief  the function related to filter the image 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/


#if !defined(_LI_IMFILTER_H_)
#define _LI_IMFILTER_H_

#include "liblock.h"
#include <stdio.h>


/************************************************************************/
#define  ImFilter               PX(ImFilter)


/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	


MRESULT ImFilter(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc,PBLOCK BlockFilter);




#ifdef __cplusplus
}
#endif
#endif // _LIIMFILTER_H