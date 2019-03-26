#include "libImageQuality.h"
#include "filePlayer.h"

#include"amcomdef.h"
#include"HY_IMAGEQUALITY.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <map>
#include <algorithm>
#include <stdio.h>

//#define  LOGINFO
#ifdef LOGINFO
#include "libbmp.h"
#endif

#define WORK_BUFFER 2560*1920*5
#define STATIC_MEM
#define JLINE_BYTES(Width, BitCt)    (((long)(Width) * (BitCt) + 31) / 32 * 4)

void sortMapByValue(std::map<int, int>& tMap, std::vector<std::pair<int, int> >& tVector);

int cmp(const std::pair<int, int>& x, const std::pair<int, int>& y);


void GetImageQuality( const vector<char>& inData, /*输入参数,输入原始ps+h264视频 */ int64_t fileStartTime, /*输入参数,文件的utc时间 */ int64_t processStartTime, /*输入参数,开始检测的utc时间 */ int64_t length, /*输入参数,文件的时间长度，单?秒*/ int64_t gap, /*输入参数,从文件中抽取帧的间隔时间，单?秒*/ int contion, /*输入参数,需要检测的项目*/  map<int,int>& result )
{
    if(inData.size()<=0)
        return;
#ifdef LOGINFO
    fprintf(stderr, "Reversion 539\r\n");
    fprintf(stderr, "start imagequality\r\n");
#endif
    vector<HYIQ_result> vec_result;
    filePlayer player(inData,fileStartTime, processStartTime, length, gap);
#ifdef LOGINFO
    fprintf(stderr, "player.loop();\r\n");
#endif
    player.loop();
#ifdef LOGINFO
    fprintf(stderr, "player.GetResult();\r\n");
#endif
    vector<IQ_IMAGES*>& vec_bmps = player.GetResult();


    MHandle HYIQHandle=MNull;
#ifdef STATIC_MEM
#ifdef LOGINFO
    fprintf(stderr, "HYIQ_MemMgrCreate\r\n");
#endif
    MVoid *pMem=malloc(WORK_BUFFER);
    MHandle hMemMgr = HYIQ_MemMgrCreate(pMem,WORK_BUFFER);
#else
    MHandle hMemMgr = MNull;
#endif

#ifdef LOGINFO
    fprintf(stderr, "HYIQ_Init\r\n");
#endif
    HYIQ_Init(hMemMgr,&HYIQHandle);
  
    

    for (int i=0;i+2<vec_bmps.size();i+=2)
    {
        HYIQ_TOutParam p1={0};
        CustomParam custopar={0};
        HYIQ_result testresult={0};

        IQ_IMAGES* imgiq1 = vec_bmps[i];
        IQ_IMAGES* imgiq2 = vec_bmps[i+1];
#ifdef LOGINFO
        {
            char path[MAX_PATH];
            sprintf(path, "test/%d.bmp", i);
            savetobmp(path, (unsigned char*)imgiq1->pixelArray.chunky.pPixel, imgiq1->lWidth, imgiq1->lHeight, 24);
            sprintf(path, "test/%d.bmp", i+1);
            savetobmp(path, (unsigned char*)imgiq2->pixelArray.chunky.pPixel, imgiq2->lWidth, imgiq2->lHeight, 24);
        }
#endif
        
#ifdef LOGINFO
        fprintf(stderr, "HYIQ_BRIGHT\r\n");
#endif
        HYIQ_BRIGHT(HYIQHandle,imgiq1,imgiq2,&p1);
        if (p1.Bright.Brightderiv<0.3)
        {
#ifdef LOGINFO
            fprintf(stderr, "HYIQ_NOISE\r\n");
#endif
            HYIQ_NOISE(HYIQHandle,imgiq1,imgiq2,&p1);
            if (p1.Bright.BrightLevel1>-0.45&&p1.Bright.BrightLevel2>-0.45)
            {
#ifdef LOGINFO
                fprintf(stderr, "HYIQ_BLUR\r\n");
#endif
                HYIQ_BLUR(HYIQHandle,imgiq1,imgiq2,&p1);
#ifdef LOGINFO
                fprintf(stderr, "HYIQ_CAST\r\n");
#endif
                HYIQ_CAST(HYIQHandle,imgiq1,&p1);
            }
        }
#ifdef LOGINFO
        fprintf(stderr, "HYIQ_RESULT\r\n");
#endif
        HYIQ_RESULT(HYIQHandle,&p1,&custopar,&testresult);

        vec_result.push_back(testresult);
    }
#ifdef LOGINFO
    fprintf(stderr, "HYIQ_Uninit\r\n");
#endif

    HYIQ_Uninit(HYIQHandle);


#ifdef STATIC_MEM
#ifdef LOGINFO
    fprintf(stderr, "HYIQ_MemMgrDestroy\r\n");
#endif
    HYIQ_MemMgrDestroy(hMemMgr);
    free(pMem);
#endif

    //后处理算法
#ifdef LOGINFO
    fprintf(stderr, "afterprocess");
#endif

    int total = vec_result.size();
    if(total<=0)
        return;
    else
    {
        map<int,int> mapResultLight;
        map<int,int> mapResultBlur;
        map<int,int> mapResultNoise;
        map<int,int> mapResultCast;
        for (int i=0;i<vec_result.size();i++)
        {
            mapResultLight[vec_result[i].lightstatus]++;
            mapResultBlur[vec_result[i].blurstatus]++;
            mapResultNoise[vec_result[i].Noisestatus]++;
            mapResultCast[vec_result[i].caststatus]++;
        }

         std::vector<std::pair<int, int> > tVector;
        if(contion&LIGHT)
        {
           
            sortMapByValue(mapResultLight,tVector);
            result[(int)LIGHT] = tVector[0].first;
        }
        if(contion&BLUR)
        {
            tVector.clear();
            sortMapByValue(mapResultBlur,tVector);
            result[(int)BLUR] = tVector[0].first;
        }

        if(contion&NOISE)
        {
            tVector.clear();
            sortMapByValue(mapResultNoise,tVector);
            result[(int)NOISE] = tVector[0].first;
        }

        if(contion&CAST)
        {
            tVector.clear();
            sortMapByValue(mapResultCast,tVector);
            result[(int)CAST] = tVector[0].first;
        }


    }

#ifdef LOGINFO
    fprintf(stderr, "end imagequality\r\n");
#endif
 
}



int cmp(const std::pair<int, int>& x, const std::pair<int, int>& y)

{

    return x.second > y.second;

}



void sortMapByValue(std::map<int, int>& tMap, std::vector<std::pair<int, int> >& tVector)

{

    for (std::map<int, int>::iterator curr = tMap.begin(); curr != tMap.end(); curr++)

    {

        tVector.push_back(std::make_pair(curr->first, curr->second));

    }



    std::sort(tVector.begin(), tVector.end(), cmp);

}
