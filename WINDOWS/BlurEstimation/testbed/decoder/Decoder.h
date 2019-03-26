#ifndef __DECODER_H__
#define __DECODER_H__
#ifdef _MSC_VER
#include "stdint.h"
#else
#include <sys/types.h>
#endif
//前置声明
struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class  Decoder
{
public:
    Decoder();
    ~Decoder();
public:
    AVFrame*  DecodeVideo(char*data, int length);
    void flush();
private:
    //H.264全局变量
    AVCodec *codec;
    AVCodecContext *c;
    AVFrame *frame;	
};
#endif