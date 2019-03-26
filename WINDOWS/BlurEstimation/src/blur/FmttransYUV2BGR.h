/*!
* \file FmttransYUV2BGR.h
* \brief  YUV420 transform to BGR
* \author hmy@whayer
* \version vision 1.0 
* \date 4 July 2014
*/

#ifndef _FMTTRANSYUV2BGR_H_
#define _FMTTRANSYUV2BGR_H_
#include "licomdef.h"
/************************************************************************/
#define FmttransYUV2BGR                   PX(FmttransYUV2BGR)

/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif	
    MRESULT FmttransYUV2BGR(PJOFFSCREEN Imgsrc,PJOFFSCREEN Imgrlt );
#ifdef __cplusplus

}
#endif


#endif