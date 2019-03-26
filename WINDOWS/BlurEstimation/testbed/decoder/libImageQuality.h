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
    //可以检测的项目
    enum QualityType{
        LIGHT =  0x01,
        BLUR = 0x02,
        NOISE = 0x04,
        CAST  = 0x08
    };
//处理结果
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

    void GetImageQuality( const vector<char>& inData,        /*输入参数,输入原始ps+h264视频 */ 
        int64_t fileStartTime,                     /*输入参数,文件的utc时间 */ 
        int64_t processStartTime,                   /*输入参数,开始检测的utc时间 */ 
        int64_t length,                       /*输入参数,检测的时间长度，单位秒*/ 
        int64_t gap,                         /*输入参数,从文件中抽取帧的间隔时间，单位秒*/
        int     contion,                        /*输入参数,需要检测的项目*/ 
        map<int,int>& result ) ;               /*输出参数,检测结果*/

#ifdef __cplusplus
};
#endif

#endif