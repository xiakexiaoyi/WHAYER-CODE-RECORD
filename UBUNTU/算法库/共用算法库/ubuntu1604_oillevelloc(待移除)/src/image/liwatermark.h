/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and 		
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employees 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an intended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**
**	Copyright (c) 2006 by ArcSoft Inc.
**
**	Name			: Watermark.h
**
**	Purpose			: Watermark API 
**
**	Additional		: 
**
**------------------------------------------------------------------------------------------------
**************************************************************************************************/

#ifndef _LI_WATERMARK_H_
#define _LI_WATERMARK_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************/
#define MEmbeddedMark				PX(MEmbeddedMark)
#define JEmbeddedMark				PX(JEmbeddedMark)
/********************************************************************************/

MVoid MEmbeddedMark(MUInt8 *pImage, MLong nWidth, MLong nHeight, 
					MLong nBitCnts, MLong nLineBytes);
MVoid JEmbeddedMark(PJOFFSCREEN pImg);

#ifdef __cplusplus
}
#endif

#endif //_LI_WATERMARK_H_
