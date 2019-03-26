/*!
* \file Liconv.h
* \brief  the function related to convolution 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#if !defined(_LI_CONV_H_)
#define _LI_CONV_H_


#include "liblock.h"
#include <stdio.h>

/************************************************************************/
#define  LiConv                                 PX( LiConv)

/************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif	

MRESULT LiConv(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB);
MRESULT LiConv_U8(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB);
MRESULT LiConv_FLOAT(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB);

#ifdef __cplusplus
}
#endif
#endif // _LICONV_H