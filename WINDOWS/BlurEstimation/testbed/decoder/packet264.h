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
    //���� true:�ҵ��������������ϵ�startcode��false��
    //outdata ��ʾsps+pps+i��p֡
    bool packet(const char* indata, int length, int64_t pts, vector<h264frame*>& outdata);
    
    //demux���ֲ��������͵���
    void flush();

private:
    //���ֲ������󣬶����������ݣ��ص���ʼ��״̬
    void init();
    //�ҵ���ʼ��
    bool resynch(int* pos, int* type);
private:
    //�ݴ�����
    vector<char> buffer;
    //�ݴ������е�֡����
    vector<int>  vec_type;
    //��һ�ζ���λ��
    int64_t          last_read;

    //��һ����ʼ���λ��
    int64_t          last_Startcode;

    //��һ�ε�type
    int          last_type;

    //��һ��startcode��ʱ��
    int64_t          lastPts;

    //�������õİ�
    bool         drop;

    //�ݴ�sps
    vector<char> sps;

    //�ݴ�pps
    vector<char> pps;

    //




};
#endif