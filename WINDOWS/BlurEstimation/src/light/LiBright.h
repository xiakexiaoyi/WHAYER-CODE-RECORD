/*!
* \file LiBright.h
* \brief  the function related to hsv and image cast  
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#ifndef _LI_BRIGHT_H_
#define _LI_BRIGHT_H_
#include "licomdef.h"
#include "lichannel.h"
/************************************************************************/
#define liEnhance                   PX(liEnhance)
#define BGRTOHSV               PX(BGRTOHSV)
#define HSVTOBGR               PX(HSVTOBGR)
#define LiCast                         PX(LiCast)
/************************************************************************/

/// \brief BGR struct
typedef struct  
{
    MUInt8* B;
    MUInt8* G;
    MUInt8 *R;
}BGR;

/// \brief HSV struct
typedef struct  
{
    MUInt16* H;
    MDouble *S;
    MDouble *V;
}HSV;

/// \brief image cast ratio struct
typedef struct
{
    MDouble gratio;
    MDouble rratio;
    MDouble bratio;
	MDouble rdev;
	MDouble gdev;
	MDouble bdev;
}Ratio,*PRatio;
#ifdef __cplusplus
extern "C" {
#endif	

   MRESULT liEnhance(MHandle hMemMgr,PJOFFSCREEN imgsrc,PJOFFSCREEN imgres);
   MRESULT BGRTOHSV(BGR bgrsrc,HSV hsvres,MLong width,MLong height);
   MRESULT HSVTOBGR(BGR bgrres,HSV hsvsrc,MLong width,MLong height);
   MRESULT LiCast(MHandle hMemMgr,PJOFFSCREEN imgsrc,PRatio ratiosrc);
#ifdef __cplusplus

}
#endif

#endif