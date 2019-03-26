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
������	��HaarWdith
����		����ͼ���л�ȡ��̶�ֵ�Ŀ��
�������˵����
				hMemMgr :   �������
				pImage :        ����Ҷ�ͼ��
                pNumTuple : �����⵽��ֱ�߶���Ŀ(Tuple)
				segs :            �����⵽��ֱ�߶�
				              
�������˵����    pHaarWidth :          ���Haar���
����ֵ˵�����������
===========================================================*/
    	//////////////////////////////////////////////////////////////////////////
#define GetHaarWdith		        PX(HaarWdith)
	//////////////////////////////////////////////////////////////////////////
     MRESULT GetHaarWdith(MHandle hMemMgr, BLOCK *pImage, MInt32 *pNumTuple, MDouble *segs, MLong *pHaarWidth);

    #ifdef __cplusplus
}
#endif
#endif