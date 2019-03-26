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

//前置声明
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
    //工作线程
    void loop();
    //输出结果
    vector<IQ_IMAGES*>& GetResult();

private:
    //转换函数
    IQ_IMAGES* AVFrame2IQlImage(AVFrame* frame);
private:

private:
    //输入h264视频
    const vector<char>& indata_;


    //处理的时间（相对时间）
    int64_t processTime_; //秒


    //文件的时间长度
    int64_t length_;

    //处理时间间隔
    int64_t gap_;

    //Demux
    Demux*  demux_;

    //decoder
    Decoder* decoder_;


    //输出结果
    vector<IQ_IMAGES*> outdata_;


    //转码
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
//实现
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
	int      type;      //帧的种类 
	vector<char> data; //数据
	vector<char> videoData;          //可解码数据
	bool isMoving;     //移动
	int64_t pts;        //有空改写
	int64_t off;
};





#endif