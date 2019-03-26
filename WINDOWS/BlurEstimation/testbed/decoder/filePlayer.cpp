
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
#include "filePlayer.h"
#include "Demux.h"
#include "Decoder.h"


//解码后转换大小，色度空间
#define COMPRESS_WIDTH 704
#define COMPRESS_HEIGHT 576


#define JLINE_BYTES(Width, BitCt)    (((long)(Width) * (BitCt) + 31) / 32 * 4)

filePlayer::filePlayer(const vector<char>& indata, int64_t fileStartTime, int64_t processStartTime, int64_t length, int64_t gap)
                        :indata_(indata),processTime_(processStartTime -fileStartTime),length_(length),gap_(gap),
                        demux_(NULL),decoder_(NULL),temp_(NULL),swsCtx_(NULL)
{
    demux_ = new Demux();
	demux_->push_data(indata_);
    decoder_ = new Decoder();


    //转码
    temp_ = avcodec_alloc_frame();
    temp_->format = PIX_FMT_BGR24;
    temp_->width  = COMPRESS_WIDTH;
    temp_->height = COMPRESS_HEIGHT;
    av_image_alloc(temp_->data, temp_->linesize, COMPRESS_WIDTH, COMPRESS_HEIGHT, PIX_FMT_BGR24, 24);

    inputWidht_  = 0;
    inputHeight_ = 0;

}

filePlayer::~filePlayer()
{
    delete demux_;
    delete decoder_;
    if(temp_)
    {
        av_freep(&temp_->data[0]);
#if LIBAVCODEC_VERSION_MAJOR >= 54
        avcodec_free_frame(&temp_);   
#else
        av_free(temp_);
#endif
    }

    if(swsCtx_)
    {
        sws_freeContext(swsCtx_);       
    }

    demux_      = NULL;
    decoder_    = NULL;
    temp_        = NULL;
    swsCtx_     = NULL; 

    

    for (int i=0;i<outdata_.size();i++)
    {
        delete [](outdata_[i]->pixelArray.chunky.pPixel);
        delete outdata_[i];
    }
    outdata_.clear();

    

}

void filePlayer::loop()
{
        int64_t firstPts = -1; //微秒
        int64_t endPts  = -1;
        int  num = 0;
		//从第一个i帧开始，每个gap_，就是一个处理点，每个处理点取两个帧
		int64_t handlePts = 0;

		do{
			h264frame* info;

			bool ret = demux_->GetNextFrame( &info);
			if(!ret)
				break;
			if(info->videoData.empty())
				continue;
			AVFrame* frame = decoder_->DecodeVideo(&info->videoData[0], info->videoData.size());
			if(frame && frame->width>0 && frame->height>0)
			{
				if (handlePts<=info->pts)
				{
					if(0 == handlePts)
					{
						handlePts = info->pts;
						endPts = info->pts + length_*1000*1000;
					}
					IQ_IMAGES* img = AVFrame2IQlImage(frame);
					outdata_.push_back(img);
					++num;
					if(2 == num)
					{
						num = 0;
						handlePts += gap_*1000*1000;
						if(handlePts>endPts)
							break;
					}
				}
			}
		}while(1);
   

}


 IQ_IMAGES* filePlayer::AVFrame2IQlImage(AVFrame* frame)
{

    if(inputWidht_ != frame->width || inputHeight_ != frame->height)
    {

        if(swsCtx_)
        {
            sws_freeContext(swsCtx_);   
            swsCtx_ = NULL;
        }

        if(!swsCtx_)
        {
            /* create scaling context */
            swsCtx_ = sws_getContext( frame->width, frame->height, PIX_FMT_YUV420P,
                COMPRESS_WIDTH, COMPRESS_HEIGHT, PIX_FMT_BGR24,
                SWS_BICUBIC, NULL, NULL, NULL);
        }
        if (swsCtx_)
        {
            inputWidht_ = frame->width;
            inputHeight_ = frame->height;
        }
    }
    
    //将YUV420格式的图像转换成RGB格式所需要的转换上下文  
    if(swsCtx_)  
    {  
        sws_scale(swsCtx_,frame->data,frame->linesize,0,inputHeight_, temp_->data, temp_->linesize);//图像格式转换   
        IQ_IMAGES* imgiq1 = new IQ_IMAGES;
        char* data = new char[COMPRESS_HEIGHT*temp_->linesize[0]];
        memcpy(data, temp_->data[0], COMPRESS_HEIGHT*temp_->linesize[0]);
        imgiq1->lHeight=COMPRESS_HEIGHT;
        imgiq1->lWidth=COMPRESS_WIDTH;
        imgiq1->lPixelArrayFormat=HY_IMAGE_BGR;
        imgiq1->pixelArray.chunky.lLineBytes=JLINE_BYTES(COMPRESS_WIDTH,24);
        imgiq1->pixelArray.chunky.pPixel = (MVoid*)data;
        return imgiq1;
    }  
    return NULL;
}

vector<IQ_IMAGES*>& filePlayer::GetResult()
{
    return outdata_;
}
