/*!
* \file Lihist.h
* \brief  the function related histogram equalization
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#ifndef _LI_HIST_H_
#define _LI_HIST_H_
#include "licomdef.h"

/************************************************************************/
#define  LIHISTEQUAL                            PX(LIHISTEQUAL)
#define  LIHISTEQUALH                         PX(LIHISTEQUALH)
/************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif	
  MRESULT  LIHISTEQUAL(MUInt8 *datasrc,MLong width,MLong height);
  MRESULT LIHISTEQUALH(MDouble  *datas,MLong width,MLong height);
#ifdef __cplusplus
}
#endif

#endif