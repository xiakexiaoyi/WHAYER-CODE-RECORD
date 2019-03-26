#include "Demux.h"
#include "filePlayer.h"

#ifndef _MSC_VER
#include <memory.h>
#endif


#ifndef __MIN
#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif
//#define  DEMO
#ifdef DEMO
static FILE* logfile = fopen("demux.txt", "wb");
#endif

//时间，裸数据
static   bool ps_pkt_parse_pes(const char* p_buffer, int i_buffer, int i_skip_extra, int64_t& i_pts, int64_t& i_dts, unsigned int& i_skip);
//帧大小
static int ps_pkt_size( const unsigned char *p, int i_peek );


Demux::Demux( ):readPos(0),lastSync(-1),b_need_sps(true),videoFrame(NULL), i_step(STEP_SYNC),i_last_step(STEP_SYNC),pi_code(0),temp(NULL)
{

}

Demux::~Demux()
{
    if(videoFrame)
    {
        delete videoFrame;
        videoFrame = NULL;
    }

    if(temp)
    {
        delete temp;
        temp = NULL;
    }
}


bool Demux::resynch(int64_t* pi_code )
{
    for(;readPos +4 <=indata_.size();readPos++)
    {
        const unsigned char* tmp = (const unsigned char*)&indata_[readPos];
        if(tmp[0] == 0 && tmp[1] == 0 && tmp[2] == 1 &&   tmp[3]>=0xb9)
        {
            *pi_code = 0x100 | tmp[3];
            return true;
        }
    }
    return false;
}

int ps_pkt_size( const unsigned char *p, int i_peek )
{
    //assert( i_peek >= 6 );
    if( p[3] == 0xb9 && i_peek >= 4 )
    {
        return 4;
    }
    else if( p[3] == 0xba )
    {
        if( (p[4] >> 6) == 0x01 && i_peek >= 14 )
        {
            return 14 + (p[13]&0x07);
        }
        else if( (p[4] >> 4) == 0x02 && i_peek >= 12 )
        {
            return 12;
        }
        return -1;
    }
    else if( i_peek >= 6 )
    {
        return 6 + ((p[4]<<8) | p[5] );
    }
    return -1;
}

//todo 没考虑丢帧

bool Demux::readPsFrame(bool* broken)
{
    *broken = false;
    const unsigned char *p_peek = (const unsigned char *)&indata_[readPos];
    int64_t remind = indata_.size() - readPos;
    if(remind <14)
        return false;

    int i_size = ps_pkt_size(p_peek, 14);


    if( i_size < 0 || ( i_size <= 6 && p_peek[3] > 0xba ) )
    {
        /* Special case, search the next start code */
        i_size = 6;
        for( ;; )
        {
            if(remind<i_size + 4)
                return false;
            int remindPos = readPos; //错误恢复
            readPos += 4;           //避免重复找一个

            int64_t pi_code = 0;
            if(!resynch(&pi_code)) //再次同步
            {
                readPos = remindPos;
                return false;
            }
            else
                return true;
        }
    }
    else
    {
        if(remind<i_size + 4)
            return false;
        else
        {
             const unsigned char *tmp = (const unsigned char *)&indata_[readPos+i_size];
            if(0 == tmp[0] && 0 == tmp[1] && 1== tmp[2] && tmp[3] >0x90)
            {
                readPos += i_size;
                return true;
            }
            else
            {
                readPos +=4;
                *broken = false;
                return true;
            }
        }
    }
}

void Demux::push_data( const vector<char>& inData )
{
    indata_.insert(indata_.end(), inData.begin(), inData.end());
}


    /* Parse a PES (and skip i_skip_extra in the payload) */
  bool ps_pkt_parse_pes( const char* p_buffer, int i_buffer, int i_skip_extra, int64_t& i_pts, int64_t& i_dts, unsigned int& i_skip )
{
    unsigned char header[34];
    i_skip  = 0;
    i_pts = -1;
    i_dts = -1;

    memcpy( header, p_buffer, __MIN( i_buffer, 34 ) );

    switch( header[3] )
    {
    case 0xBC:  /* Program stream map */
    case 0xBE:  /* Padding */
    case 0xBF:  /* Private stream 2 */
    case 0xB0:  /* ECM */
    case 0xB1:  /* EMM */
    case 0xFF:  /* Program stream directory */
    case 0xF2:  /* DSMCC stream */
    case 0xF8:  /* ITU-T H.222.1 type E stream */
        i_skip = 6;
        break;

    default:
        if( ( header[6]&0xC0 ) == 0x80 )
        {
            /* mpeg2 PES */
            i_skip = header[8] + 9;

            if( header[7]&0x80 )    /* has pts */
            {
                i_pts = ((int64_t)(header[ 9]&0x0e ) << 29)|
                    (int64_t)(header[10] << 22)|
                    ((int64_t)(header[11]&0xfe) << 14)|
                    (int64_t)(header[12] << 7)|
                    (int64_t)(header[13] >> 1);

                if( header[7]&0x40 )    /* has dts */
                {
                    i_dts = ((int64_t)(header[14]&0x0e ) << 29)|
                        (int64_t)(header[15] << 22)|
                        ((int64_t)(header[16]&0xfe) << 14)|
                        (int64_t)(header[17] << 7)|
                        (int64_t)(header[18] >> 1);
                }
            }
        }
        else
        {
            i_skip = 6;
            while( i_skip < 23 && header[i_skip] == 0xff )
            {
                i_skip++;
            }
            if( i_skip == 23 )
            {
                /* msg_Err( p_demux, "too much MPEG-1 stuffing" ); */
                return false;
            }
            if( ( header[i_skip] & 0xC0 ) == 0x40 )
            {
                i_skip += 2;
            }

            if(  header[i_skip]&0x20 )
            {
                i_pts = ((int64_t)(header[i_skip]&0x0e ) << 29)|
                    (int64_t)(header[i_skip+1] << 22)|
                    ((int64_t)(header[i_skip+2]&0xfe) << 14)|
                    (int64_t)(header[i_skip+3] << 7)|
                    (int64_t)(header[i_skip+4] >> 1);

                if( header[i_skip]&0x10 )    /* has dts */
                {
                    i_dts = ((int64_t)(header[i_skip+5]&0x0e ) << 29)|
                        (int64_t)(header[i_skip+6] << 22)|
                        ((int64_t)(header[i_skip+7]&0xfe) << 14)|
                        (int64_t)(header[i_skip+8] << 7)|
                        (int64_t)(header[i_skip+9] >> 1);
                    i_skip += 10;
                }
                else
                {
                    i_skip += 5;
                }
            }
            else
            {
                i_skip += 1;
            }
        }
    }

    if( i_skip_extra >= 0 )
        i_skip += i_skip_extra;
    //else if( i_buffer > i_skip + 3 &&
    //    ( ps_pkt_id( p_pes ) == 0xa001 || ps_pkt_id( p_pes ) == 0xbda1 ) )
    //    i_skip += 4 + p_buffer[i_skip+3];

    if( i_buffer <= i_skip )
    {
        return false;
    }

    p_buffer += i_skip;
    i_buffer -= i_skip;

    if( i_dts >= 0 )
        i_dts = 0 + 100 * i_dts / 9;
    if( i_pts >= 0 )
        i_pts = 0 + 100 * i_pts / 9;

    return true;
}

//不可读时返回false
//读到完整pack pes,直接返回
//同步到000001ba，或者同步到NAL00000001， && 暂存了一个videoframe，返回上一个videoframe

bool Demux::GetNextFrame(h264frame** frame)
{
    //需要更多数据
    bool b_more = false;
    
    do 
    {
        i_last_step = i_step;
        switch(i_step)
        {
        case STEP_SYNC:
            {
                i_step      = STEP_READ;

                if(!resynch(&pi_code))
				{
					i_step      = STEP_SYNC;
                    b_more = true;
					break;
				}
                lastSync = readPos;
                //同步到000001ba而求暂存了一个videoframe，返回上一个videoframe
                if(pi_code==0x1ba&&videoFrame)
                {
                    *frame = videoFrame;
                    i_step = STEP_CHANGE;
                    return true;
                }
                break;
            }

        case STEP_READ:
            {
                //broken, oxba, oxbb, 0xbc,都要重新同步，
                //0xe0要分析type
                i_step      = STEP_SYNC;
                bool broken =false;

                if(!readPsFrame(&broken))
                {
                    b_more = true;
                    i_step      = STEP_READ;
					break;
                }
                if (broken)
                {
                    b_need_sps = true;
                    i_step      = STEP_SYNC;
                    break;
                }

                if (pi_code<=0x1bc)
                {
                    h264frame* p = new h264frame;
                    p->type = PES_PACK;
                    p->data.insert(p->data.end(), &indata_[lastSync], &indata_[readPos]);
                    *frame = p; 
					i_step      = STEP_SYNC;
                    return true;
                }

                 if (pi_code==0x1e0)
				 {
                     i_step      = STEP_TYPE;
					 break;
				 }
                break;
            }

        case STEP_TYPE:
            {
                //取一帧264裸数据，并设置帧类型，时间，偏移
                int64_t i_pts = 0;
                int64_t i_dts = 0;
                unsigned int i_skip = 0;
                if(!ps_pkt_parse_pes(&indata_[lastSync], readPos - lastSync, 0, i_pts, i_dts, i_skip))
                {
                   i_step = STEP_SYNC;
				   break;
                }
                else
                {
                    if(readPos -lastSync <= (int)i_skip+5 )
                    {
                        i_step = STEP_SYNC;
						b_need_sps = true;
						break;
                    }
                    else
                    {
                        //0x00 00 00 01 
                        char* tmp = &indata_[lastSync + i_skip];
                        if (0x00 == tmp[0]&&
                            0x00 == tmp[1]&&
                            0x00 == tmp[2]&&
                            0x01 == tmp[3])
                        {
                            unsigned char i_flag = indata_[lastSync + i_skip + 4] &0x1f ;
							if(9 == i_flag)
							{
								i_skip += 6;
								i_flag = indata_[lastSync + i_skip + 4] &0x1f ;
							}
                            if(7 == i_flag)
                            {
                                b_need_sps = false;
								i_step = STEP_SYNC;
                            }
                            else
                            {
                                if (b_need_sps)
                                {
                                    delete videoFrame;
                                    videoFrame = NULL;
                                    i_step = STEP_SYNC;
                                    break;
                                }
                            }

                            temp = new h264frame;
                            temp->type = i_flag;
							temp->pts = i_pts;
                            vector<char>& data = temp->data;
                            vector<char>& videoData = temp->videoData;
                            data.insert(data.end(), &indata_[lastSync], &indata_[readPos]);
                            videoData.insert(videoData.end(),&indata_[lastSync+i_skip], &indata_[readPos]);
                            
                            if(videoFrame)
                            {
                                //遇到NAL返回videoframe
                                i_step = STEP_CHANGE;
                                *frame = videoFrame;
                                return true;
                            }
                            else
                            {
                                i_step = STEP_SYNC;
                                videoFrame = temp;
                                temp = NULL;
                                break;
                            }
                        }
                        else
                        {
                            //附加
                            if(videoFrame)
                            {   
                                vector<char>& data = videoFrame->data;
                                vector<char>& videoData = videoFrame->videoData;
                                data.insert(data.end(), &indata_[lastSync], &indata_[readPos]);
                                videoData.insert(videoData.end(),&indata_[lastSync+i_skip], &indata_[readPos]);
                                i_step = STEP_CHANGE;
                            }
                            i_step = STEP_SYNC;
                        }


                    }
                }
                break;
            } 
        case STEP_CHANGE:
            {
                delete videoFrame;
                videoFrame = temp;
                temp = NULL;
                i_step = STEP_SYNC;
                break;
            }
            
        }
    } while (!b_more);
   

    //清零
    
    int erase_num = lastSync;
    if(STEP_SYNC == i_last_step)
    {
        if(-1==lastSync)
        {
            //从未同步,剩下两个字节即可
            if(indata_.size()>2)
            {
                char tmp1 = indata_[indata_.size()-2];
                char tmp2 = indata_[indata_.size()-1];
                indata_.clear();
                indata_.push_back(tmp1);
                indata_.push_back(tmp2);
            }
            
            lastSync = -1;
            readPos = 0;
            return false;
        }
    }
    else
    {
        erase_num = lastSync;
        lastSync -= erase_num;
        readPos -= erase_num;
    }
    vector<char>::iterator erase_begin = indata_.begin();
    vector<char>::iterator erase_end = erase_begin + erase_num;
    indata_.erase(erase_begin, erase_end); 
   
   return false;
}

