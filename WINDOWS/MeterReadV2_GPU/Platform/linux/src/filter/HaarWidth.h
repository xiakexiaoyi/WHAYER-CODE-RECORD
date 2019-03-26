#ifndef HAAR_WIDTH_H
#define HAAR_WIDTH_H
#include"amcomdef.h"
#include "liblock.h"
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
	{
		MDouble  Angle;
        MDouble Mag;
        MDouble kVal;
	}SegParam, *pSegParam;

    typedef struct
	{
		MPOINT RegCenter;
        MPOINT LeftUp, RightDown;
		MDouble kSeg;
        MDouble Ratio;
		MLong RegionWidth;
        MLong RegionHeight;
		MInt32 FlagNum;
	}FilterParam, *pFilterParam;

/*===========================================================
函数名	：HaarWdith
功能		：从图像中获取大刻度值的宽度
输入参数说明：
				hMemMgr :   操作句柄
				pImage :        输入灰度图像
                pNumTuple : 输入检测到的直线段数目(Tuple)
				segs :            输入检测到的直线段
				              
输出参数说明：    pHaarWidth :          输出Haar宽度
返回值说明：错误代码
===========================================================*/
    	//////////////////////////////////////////////////////////////////////////
#define GetHaarWdith		        PX(HaarWdith)
	//////////////////////////////////////////////////////////////////////////
     MRESULT GetHaarWdith(MHandle hMemMgr, BLOCK *pImage, MInt32 *pNumTuple, MDouble *segs, MLong *pHaarWidth);

    #ifdef __cplusplus
}
#endif
#endif