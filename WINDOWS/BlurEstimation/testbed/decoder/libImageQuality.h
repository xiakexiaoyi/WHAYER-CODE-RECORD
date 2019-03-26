#ifndef __LIB_IMAGE_QUALITY_H__
#define __LIB_IMAGE_QUALITY_H__

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <sys/types.h>
#endif

#include <vector>
#include <map>

using namespace std;

#ifdef __cplusplus
extern "C"{
#endif
    //���Լ�����Ŀ
    enum QualityType{
        LIGHT =  0x01,
        BLUR = 0x02,
        NOISE = 0x04,
        CAST  = 0x08
    };
//������
#define IMAGE_UNBLUR      900
#define IMAGE_TINYBLUR    901
#define IMAGE_MEDBLUR     902
#define IMAGE_HEVBLUR      903
#define IMAGE_UNNOISE     904
#define IMAGE_MEDNOISE    905
#define IMAGE_HEAVENOISE  906
#define IMAGE_TOODARK     907
#define IMAGE_TOOLIGHT    908
#define IMAGE_CAST        909
#define IMAGE_TINYNOISE   910

    void GetImageQuality( const vector<char>& inData,        /*�������,����ԭʼps+h264��Ƶ */ 
        int64_t fileStartTime,                     /*�������,�ļ���utcʱ�� */ 
        int64_t processStartTime,                   /*�������,��ʼ����utcʱ�� */ 
        int64_t length,                       /*�������,����ʱ�䳤�ȣ���λ��*/ 
        int64_t gap,                         /*�������,���ļ��г�ȡ֡�ļ��ʱ�䣬��λ��*/
        int     contion,                        /*�������,��Ҫ������Ŀ*/ 
        map<int,int>& result ) ;               /*�������,�����*/

#ifdef __cplusplus
};
#endif

#endif