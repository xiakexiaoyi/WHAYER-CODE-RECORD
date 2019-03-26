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

//ǰ������
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
    //һ��ȡһ֡��pes pack or pes video
    bool GetNextFrame(h264frame** frame);
public:
    void push_data(const vector<char>& inData);
private:
    //Ѱ��000001ba
    bool resynch(int64_t* pi_code );
    bool readPsFrame(bool* broken);
private:
    //�ݴ�����h264��Ƶ
    vector<char> indata_;

    //��֡��Ҫ�õ���
    //���α�
    int64_t readPos;

    //000001Pesͬ��
    int64_t lastSync;

    //
    bool b_need_sps;

    //�ݴ�����ֱ��������һ��pack pes
    h264frame*        videoFrame;
    h264frame*        temp;

    int i_step;
    int i_last_step;

    int64_t pi_code;
};

#endif