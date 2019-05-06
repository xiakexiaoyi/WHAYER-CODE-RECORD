#ifndef _LI_MATH_H_
#define _LI_MATH_H_

#include "licomdef.h"
#include"math.h"

#ifndef FABS
#define FABS(x)  ((x)>0?(x):-(x))
#endif
//////////////////////////////////////////////////////////////////////////
//Basic mathematical operator
#define ABS(x) (((x)+((x)>>31))^((x)>>31))
#define MIN(x,y) ((x)>(y) ? (y) : (x) )	
#define MAX(x,y) ((x)<(y) ? (y) : (x) )	

#define	SIGN(x)	((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#define SQARE(x)	((x)*(x))

//////////////////////////////////////////////////////////////////////////
//Advanced Trim
#define TRIM(x,thre1,thre2) ((x) > thre2 ? thre2 : ((x) < thre1 ? thre1 : (x)))

#define TRIM_UINT8(x)	(MByte)((x)&(~255) ? ((-(x))>>31) : (x))
#define TRIM_UINT16(x)	(MWord)((x)&(~0xFFFF) ? ((-(x))>>31) : (x))
#define TRIM_INT8(x)	(MChar)(((x)+128)&(~255) ? (((x)>>31)^0x7F) : (x))

//#define TRIM_UINT14(x)	(MUInt16)((x)&(~0x3FFF) ? ((-(x))>>31) : (x))
#define TRIM_UINT14(x)	(MWord)((x)>=(1<<14) ? ((1<<14)-1) : ((x)<0 ? 0 : (x)))
//#define TRIM_INT14(x)	(MInt16)( (x+(1<<13))&(~0x3FFF) ? ((1<<13)-1-((x)>>31)) : (x) )
#define TRIM_INT14(x)	(MShort)((x)>=(1<<13) ? ((1<<13)-1) : ((x)<-(1<<13) ? -(1<<13) : (x)))

//////////////////////////////////////////////////////////////////////////
//SHIFT, DOWN, DIV
#define SHIFT_HALF(shift)		( (1<<(shift))>>1 )

#define DOWN_ROUND(x, shift)	( ( (x) + SHIFT_HALF(shift) ) >> (shift) )
#define DOWN_CEIL(x, shift)		( ( (x) + (1<<(shift)) - 1	) >> (shift) )
#define DOWN_FLOOR(x, shift)	( (x) >> (shift) )

#define DIV_CEIL(x, divisor)	( ( (x) + (divisor) - 1 ) / (divisor) )
#define DIV_FLOOR(x, divisor)	( (x)  / (divisor) )
#define DIV_ROUND(x, divisor)	( ( (x) + ((divisor  )>>1) ) / (divisor) )

#define IS_POW2(x)				( ((x)&(x-1)) == 0	) 
#define LOG2_FLOOR(x, rlt)		{ MLong _i=-1; while(1<<(++_i)<=(MLong)x); rlt=_i-1;}
//////////////////////////////////////////////////////////////////////////
//SNAP
#define CEIL_2(x)			( ((x)+1) & (~0x1) )
#define FLOOR_2(x)			( (x) & (~0x1) )
#define CEIL_4(x)			(( (x) + 3 ) & (~3) )
#define CEIL_16(x)			(( (x) + 15 ) & (~15) )
#define FLOOR_16(x)			( (x) & (~15) )
#define CEIL_32(x)			(( (x) + 31 ) & (~31) )
#define FLOOR_4(x)			( (x) & (~0x3) )
#define CEIL_8(x)			(( (x) + 7 ) & (~0x7) )
#define FLOOR_8(x)			( (x) & (~0x7) )
#define CEIL_AD(x, shift)	(DOWN_CEIL(x, shift) << (shift))
#define FLOOR_AD(x, shift)	(DOWN_FLOOR(x, shift) << (shift))

#define V_PI	3.1415926

#define RAD2ANG	57.297		// 180/PI
#define ANG2RAD	0.01745		// PI/180

#endif//_LI_MATH_H_