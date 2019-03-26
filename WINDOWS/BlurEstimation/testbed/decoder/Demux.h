#ifndef __DEMUX_H__
#define __DEMUX_H__
#ifdef _MSC_VER
#include "stdint.h"
#else
#include <sys/types.h>
#endif
#include <vector>
#include <queue>

using namespace std;

//前置声明
struct h264frame;
enum  DEMUX_STEP
{
    STEP_SYNC = 1,
    STEP_READ,
    STEP_TYPE,
    STEP_CHANGE,
};


class Demux
{
public:
    Demux();
    ~Demux();

public:
    //一次取一帧，pes pack or pes video
    bool GetNextFrame(h264frame** frame);
public:
    void push_data(const vector<char>& inData);
private:
    //寻找000001ba
    bool resynch(int64_t* pi_code );
    bool readPsFrame(bool* broken);
private:
    //暂存输入h264视频
    vector<char> indata_;

    //分帧需要用到的
    //读游标
    int64_t readPos;

    //000001Pes同步
    int64_t lastSync;

    //
    bool b_need_sps;

    //暂存数据直到遇到下一个pack pes
    h264frame*        videoFrame;
    h264frame*        temp;

    int i_step;
    int i_last_step;

    int64_t pi_code;
};

#endif