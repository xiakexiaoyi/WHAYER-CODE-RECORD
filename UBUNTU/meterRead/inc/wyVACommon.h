//#include "sdkMacro.h"

#ifndef _WYVACOMMON_
#define _WYVACOMMON_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define DEBUG_WY_DSP 0
#if 1
#define HOST_PRINT printf
#else
#define HOST_PRINT (void)
#endif
#define WYVA_PI (3.1415926)
#define WYVA_SQRT sqrt
#define WYVA_ATAN atan
#define WYVA_ATAN2 atan2

#define WYVA_SIN  sin
#define WYVA_COS  cos
#define WYVA_OBJSIZE_MAX (448)
#define WYVA_ROTATE_SIZE_MAX (640 - 5)
#define WYVA_SCALOR_SIZE_MAX (640 - 5 + 7 + 6) //648%8=0
#define WYVA_PIC_HEIGHT_MAX (576)
#define WYVA_PIC_WIDTH_MAX (720)  

//#define PICWIDTH_MAX (720) 
//#define PICHEIGHT_MAX (576) 
#define WYVA_ALIGN16 (0x0)//(0xF)
#define WYVA_OBJSIZE_MIN (64)
#define WY_DEBUG (0)
#define WY_ABS(a) (a>=0 ? a:(-a))

#define FLOAT_SCALOR_NUM (512)
#define FLOAT_SHFIT_BITNUM (9)
#define WY_OBJDESSIZE_MAX (128)
#define HI3516_WY (0)

typedef double WYVA_FLOAT;
typedef float WYVA_SFLOAT; //short

 typedef enum
 {
    MEM_NO_CACHE=0,
    MEM_CACHE
 }WY_CACHE;
/*
 typedef enum {
     WY_SUCCESS = 0,
     WY_FAILURE = -1,
     WY_OP_ORDER_ERR = -2,    
     WY_IN_PARA_ERR = -3,
     WY_NOT_SUPPORT = -4,
     WY_NO_RES_ERR = -5,  //no resource is available, ex. encoder channel 
     WY_INNER_ERR = -6
 }WY_RET;
 */


typedef struct
{
	unsigned char *pData;
	int startX;
	int startY;
	int width;
	int height;
	int stride;
}wyVA_picBufA;

typedef struct
{
	unsigned char *pData;
	int width;
	int height;
	int stride;
}wyVA_picBuf;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

