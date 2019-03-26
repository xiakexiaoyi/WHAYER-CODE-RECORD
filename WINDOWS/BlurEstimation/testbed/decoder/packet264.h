#ifndef __PACKET_264_H__
#define __PACKET_264_H__

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <sys/types.h>
#endif
#include <vector>
using namespace std;

struct h264frame;

class Packet264
{
public:
    Packet264();
    ~Packet264();
public:
    //返回 true:找到两个或两个以上的startcode，false：
    //outdata 表示sps+pps+i或p帧
    bool packet(const char* indata, int length, int64_t pts, vector<h264frame*>& outdata);
    
    //demux发现不连续，就调用
    void flush();

private:
    //发现不连续后，丢弃所有数据，回到初始化状态
    void init();
    //找到起始码
    bool resynch(int* pos, int* type);
private:
    //暂存数据
    vector<char> buffer;
    //暂存数据中的帧类型
    vector<int>  vec_type;
    //上一次读的位置
    int64_t          last_read;

    //上一次起始码的位置
    int64_t          last_Startcode;

    //上一次的type
    int          last_type;

    //上一次startcode的时间
    int64_t          lastPts;

    //丢掉无用的包
    bool         drop;

    //暂存sps
    vector<char> sps;

    //暂存pps
    vector<char> pps;

    //




};
#endif