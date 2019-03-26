#ifndef _HISI_H
#define _HISI_H

#include "licomdef.h"


#define HISI_createGradAngle	PX(HISI_createGradAngle)
#define HISI_copy	            PX(HISI_copy)
////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
MVoid HISI_createGradAngle( const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MUInt16 *pAngle);
MVoid HISI_copy(MUInt8 * ap_pix_old, MUInt8 * ap_pix_new ,MLong num);
#ifdef __cplusplus
}
#endif
#endif