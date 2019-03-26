//use ffmpeg
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
};

#include <vector>
#include "Decoder.h"

//Ç°ÖÃÉùÃ÷
struct AVCodec;
struct AVCodecContext;
struct AVFrame;

//#define  DEMO
#ifdef DEMO
static FILE* logfile = fopen("decoder.txt", "wb");
#endif

Decoder::Decoder():codec(NULL),c(NULL),frame(NULL)
{
    codec = avcodec_find_decoder(CODEC_ID_H264);

#if LIBAVCODEC_VERSION_MAJOR >= 54
    c = avcodec_alloc_context3(codec);
#else
    c = avcodec_alloc_context();
#endif
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        c->flags|= CODEC_FLAG_TRUNCATED;
#if LIBAVCODEC_VERSION_MAJOR >= 54
    avcodec_open2(c, codec, NULL); 
#else
    avcodec_open(c, codec);
#endif
    frame = avcodec_alloc_frame();

}

Decoder::~Decoder()
{
    

    if(c) 
    {
        avcodec_flush_buffers( c );
        avcodec_close(c); 	
        av_free(c);
        
    } 
    if(frame) 
    {   
#if LIBAVCODEC_VERSION_MAJOR >= 54
        avcodec_free_frame(&frame);   
#else
        av_free(frame);
#endif
    }
    codec = NULL;
    c = NULL;
    frame = NULL;
}

AVFrame* Decoder::DecodeVideo( char*data, int length )
{
    AVPacket packet;
    av_init_packet( &packet );
    
    std::vector<char> tmp;
    tmp.insert(tmp.end(), FF_INPUT_BUFFER_PADDING_SIZE, 0);
    tmp.insert(tmp.end(), data, data+length);
    packet.data = (uint8_t *)&tmp[0];
    packet.size = length + FF_INPUT_BUFFER_PADDING_SIZE;
    int got_frame;
    int len = avcodec_decode_video2(c, frame, &got_frame, &packet);
    if(len<0)
    {
#ifdef DEMO
        char tmp[50];
        sprintf(tmp, "legth%x\n", length);
        fprintf(logfile, "%s", tmp);
        fwrite(data, 1, length, logfile);
        fwrite("\n",1,1,logfile);
        fflush(logfile);

#endif
        return NULL;
    }
    else
        return frame;

}

void Decoder::flush()
{
    avcodec_flush_buffers(c);
}
