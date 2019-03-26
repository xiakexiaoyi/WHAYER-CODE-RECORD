#ifndef __FILE_PLAYER_H__
#define __FILE_PLAYER_H__
#ifdef _MSC_VER
#include "stdint.h"
#else
#include <sys/types.h>
#endif
#include <string.h>
#include <vector>
#include "HY_IMAGEQUALITY.h"
using namespace std;

//ǰ������
class   Demux;
class   Decoder;
struct  SwsContext;
struct  AVFrame;

class filePlayer
{
public:
    filePlayer(const vector<char>& indata, int64_t fileStartTime, int64_t processStartTime, int64_t length, int64_t gap);
    ~filePlayer();
public:
    //�����߳�
    void loop();
    //������
    vector<IQ_IMAGES*>& GetResult();

private:
    //ת������
    IQ_IMAGES* AVFrame2IQlImage(AVFrame* frame);
private:

private:
    //����h264��Ƶ
    const vector<char>& indata_;


    //�����ʱ�䣨���ʱ�䣩
    int64_t processTime_; //��


    //�ļ���ʱ�䳤��
    int64_t length_;

    //����ʱ����
    int64_t gap_;

    //Demux
    Demux*  demux_;

    //decoder
    Decoder* decoder_;


    //������
    vector<IQ_IMAGES*> outdata_;


    //ת��
    SwsContext *swsCtx_;
    AVFrame* temp_;
    int      inputWidht_;
    int      inputHeight_;


};

typedef enum h264type_
{
	PES_PACK = 3,
	PES_VIDEO_I = 7,
	PES_VIDEO_P = 1,
}h264type;
//ʵ��
struct h264frame
{
	h264frame()
	{
		type = -1;
		isMoving =false;
		data.clear();
		pts = 0;
		off = 0;
	}
	h264frame(h264frame* p)
	{
		type = p->type;
		isMoving =p->isMoving;
		swap(data, p->data);
		swap(videoData, p->videoData);
		pts = p->pts;
		off = p->off;
	}
	~h264frame()
	{
		data.clear();
	}
	int      type;      //֡������ 
	vector<char> data; //����
	vector<char> videoData;          //�ɽ�������
	bool isMoving;     //�ƶ�
	int64_t pts;        //�пո�д
	int64_t off;
};





#endif