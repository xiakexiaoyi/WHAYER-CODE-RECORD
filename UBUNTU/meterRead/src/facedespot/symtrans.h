#ifndef _LI_SYMTRANS_
#define _LI_SYMTRANS_

#include "licomdef.h"

/***********************************************************/
#define RadialSymTrans		PX(RadialSymTrans)
/***********************************************************/

#ifdef __cplusplus
extern "C" {
#endif	
MRESULT RadialSymTrans(MHandle hMemMgr,
					   MUInt8 *pImgSrc, MInt32 lImgLine, 
					   MInt32 lWidth, MInt32 lHeight,
					   MInt32 Alpha, MInt32 Beta, 
					   MInt32 lMinR, MInt32 lMaxR, 
					   MUInt8 *pImgRlt, MBool bPosDirection);
	
#ifdef __cplusplus
}
#endif

#endif