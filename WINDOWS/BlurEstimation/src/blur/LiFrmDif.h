/*!
* \file LiFrmDif.h
* \brief  the function related to calculate the difference between two frames 
* \author hmy@whayer
* \version vision 1.0 
* \date 18 July 2014
*/
#ifndef _LI_FRMDIF_H_
#define _LI_FRMDIF_H_
#include "licomdef.h"
/************************************************************************/
#define LiFrmDif                   PX(LiFrmDif)

/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif	
    MRESULT LiFrmDif(PJOFFSCREEN srcimg1, PJOFFSCREEN srcimg2,MFloat *Diff);
#ifdef __cplusplus

}
#endif


#endif