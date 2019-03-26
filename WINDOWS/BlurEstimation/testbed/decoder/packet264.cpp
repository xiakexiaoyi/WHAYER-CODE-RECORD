#include "packet264.h"
#include "filePlayer.h"
#include <algorithm>


enum nal_unit_type_e
{
    NAL_UNKNOWN = 0,
    NAL_SLICE   = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8,
    NAL_AU_DELIMITER= 9
    /* ref_idc == 0 for 6,9,10,11,12 */
};


Packet264::Packet264()
{
    init();
}

Packet264::~Packet264()
{
    init();
}

bool Packet264::packet( const char* indata, int length, int64_t pts, vector<h264frame*>& outdata )
{
    bool ret = false;
    buffer.insert(buffer.end(), indata,  indata + length);

    //起始码位置
    int pos = 0;
    int type = 0;
   while(resynch(&pos, &type))
   {
       if(-1 == last_Startcode)
       {
           last_Startcode = pos;
           last_type      = type;
           if(pts>0)
                lastPts        = pts;
       }
       else
       {
           ret = true;
           //last_startcode到pos之间的东西
           if(NAL_SPS==last_type)
           {
               sps.clear();
               sps.insert(sps.end(), &buffer[last_Startcode], &buffer[pos]);
           }
           else if(NAL_PPS == last_type)
           {
               pps.clear();
               pps.insert(pps.end(), &buffer[last_Startcode], &buffer[pos]);
           }
           else if(NAL_AU_DELIMITER == last_type)
           {

               //丢掉
           }
           else if(NAL_SLICE_IDR == last_type)
           {
                if(!sps.empty() && !pps.empty())
                {
                    drop = false;
                }

                if(drop)
                {
                    
                }
                else
                {
                    //合成i帧
                    h264frame* frame = new h264frame;
                    frame->pts = lastPts;
                    frame->type = last_type;
                    frame->data.insert(frame->data.end(), sps.begin(), sps.end());
                    frame->data.insert(frame->data.end(), pps.begin(), pps.end());
                    frame->data.insert(frame->data.end(), &buffer[last_Startcode], &buffer[pos]);
                    outdata.push_back(frame);
                }
           }
           else
           {
               if(drop)
               {
                   ;
               }
               else
               {
                   //合成p帧
                   h264frame* frame = new h264frame;
                   frame->pts = lastPts;
                   frame->type = last_type;
                   frame->data.insert(frame->data.end(), &buffer[last_Startcode], &buffer[pos]);
                   outdata.push_back(frame);
               }
           }

           last_Startcode = pos;
           last_type      = type;
           if(pts>0)
               lastPts        = pts;
       }
   }


   //偏移向前移动
   if(-1==last_Startcode)
   {
       buffer.clear();
       last_read = 0;
       last_Startcode  = -1;

   }
   else
   {
       char* pstart = &buffer[last_Startcode];
       char* pend    = pstart + buffer.size() - last_Startcode;
        vector<char> tmp(pstart, pend);
        swap(tmp, buffer);
        last_read -= last_Startcode;
        last_Startcode = 0;
   }
   
return ret;
    
}

bool Packet264::resynch(int* pos, int* type)
{
    int i = 0;
    for (i=last_read;i+5<buffer.size();i++)
    {
        //找到起始码
        if(0 == buffer[i] && 0 == buffer[i+1] && 0==buffer[i+2] && 1==buffer[i+3])
        {
            *pos = i;
            *type = buffer[i+4]&0x1f; 
            last_read = i+4;
            return true;
        }
    }
    last_read = i+1;
    return false;
}

void Packet264::flush()
{
    init();
}

void Packet264::init()
{
    buffer.clear();
    vec_type.clear();
    last_read = 0;
    last_Startcode = -1;
    last_type = -1;
    lastPts   = 0;
    drop      = true;
    sps.clear();
    pps.clear();
}
