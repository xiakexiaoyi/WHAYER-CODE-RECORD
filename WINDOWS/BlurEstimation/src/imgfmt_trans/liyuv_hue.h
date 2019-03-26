#ifndef _LI_YUV_HUE_H_
#define _LI_YUV_HUE_H_

#include "limath.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/****************************************************************************/
#define YUV2Hue             PX(YUV2Hue)
#define YUVIMG2Hue			PX(YUVIMG2Hue)
#define YUYVIMG2Hue			PX(YUYVIMG2Hue)
#define Y1VY0UIMG2Hue		PX(Y1VY0UIMG2Hue)
/****************************************************************************/

MVoid YUV2Hue(MLong Y, MLong U, MLong V, MUInt8 *pHue);    

//////////////////////////////////////////////////////////////////////////
// Conversion of YUV to Red_Hue
MVoid YUVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				 MInt32 lWidth, MInt32 lHeight);
MVoid YUYVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				  MInt32 lWidth, MInt32 lHeight);
MVoid Y1VY0UIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				    MInt32 lWidth, MInt32 lHeight);

#ifdef __cplusplus
}
#endif
#endif //_LI_YUV_HUE_H_
