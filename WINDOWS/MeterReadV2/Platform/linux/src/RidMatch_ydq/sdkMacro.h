#ifndef _SDKMACRO_
#define _SDKMACRO_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define WY_SDK_ERR	printf

#define MAX_CHAIN_NUM   (32)
#define MAX_STAGE_NUM   (10)
#define HI3516_B001_VECN_GOPSIZE (30)

#define MAX_VB_POOL_NUM  (32)
#define HI_VB_BLK_CNT  (10)
#define WY_SYS_ALIGN_WIDTH (16)

#define HI3516_B001_MAX_CHAIN (5)

#define WY_CHANNEL_ID(part, num) ((part<<16)|(num))
#define WY_PART_ID(channelID) (channelID&0xffff)
#define WY_WHICH_PART(channelID) (channelID>>16)

#define MAX_MASKER_NUM (8)
#define MAX_STRINFO_NUM (4)

#define MAX_VENC_ROI_NUM (8)
#define MAX_VFRM_NUM (3)
#define MAX_MD_REGION_NUM (8)
#define MAX_OD_REGION_NUM (4)

#define MAX_MD_PACKET_NUM (6)
#define MAX_OD_PACKET_NUM (6)
#define MAX_MR_PACKET_NUM (6)
#define MAX_FILE_NAME_LENGTH (64)

typedef enum 
{
    WY_OSD_DOWN = 0,
	WY_OSD_MIDDLE,
    WY_OSD_UPLAYER,
    UNKNOWNPRIORITY   
}WY_OSD_PRIORITY;

typedef enum 
{
    PART_VI = 0,
	PART_ENC,
    PART_VO,
    PART_VDA,
    UNKNOWNPART   
}WY_PLATFORM_PART_ID;

typedef enum 
{
    HI3516_B001 = 0,
    HI3520_B001,
    UNKNOWNPLATFORM,    
}WY_PLATFORM_ID; 

typedef enum 
{
    HI3516_VI_HD = 0,
    HI3516_VI_SD,
    UNKNOWNVI,    
}WY_Hi3516_VI;

typedef enum  
{
    HI3516_VO_HD = 0,
    HI3516_VO_SD = 2,
    UNKNOWNVO,    
}WY_Hi3516_VO;


typedef enum 
{
    WY_YUV420SP = 0, //Y U V
    WY_YUV422SP,
    UNKNOWNPIXELFORMAT,    
}WY_PIXELFORMAT; 

typedef enum 
{
    WY_ARGB1555 = 0, //Y U V
    UNKNOWNLOGOFORMAT,    
}WY_LOGOFORMAT; 

typedef enum 
{
    FCB_CH6300_BT601 = 0,  
	SCM_6200_BT601,
	SCM_5300_BT601,
	FCB_CH3310_BT601,
    MODE_DC, 
    MODE_BT1120,
	MODE_CVBS,
    VIO_MODE_BUTT
} WY_VIOPUT_MODE_E;

typedef enum 
{
	MODE_PAL=0,
	MODE_NTSC,
	MODE_AUTO,
	MODE_BUTT
} WY_VIDEO_ENCODING_MODE;

typedef enum {
	WY_FONT16x16 = 0,
	WY_FONT24x24,
	WY_FONT32x32,
	WY_FONT48x48,
	WY_FONT64x64,
	UNKNOWNFONT
} WY_FONT_TYPE;


typedef enum {
	WY_H264 = 0,
    WY_MPEG4,
    WY_AVS,
    WY_MJPEG,
    WY_JPEG,
    UNKNOWNENC
}WY_VENC_TYPE;

typedef enum {
	WY_BASELINE = 0,
	WY_MAINPROFILE,
    WY_HIGHPROFILE,
    UNKNOWNPROFILE
}WY_H264_PROFILE;

typedef enum {
	WY_CBR = 0,
    WY_VBR,
    WY_FIXEDQP,
    WY_ABR,
    UNKNOWNRC
}WY_RC_TYPE;

typedef enum {
	WY_MD = 0,
    WY_OD,
    WY_OC,
    UNKNOWNVDA
}WY_VDA_TYPE;

typedef enum {
    WY_SUCCESS = 0,
	WY_FAILURE = -1,
    WY_OP_ORDER_ERR = -2,    
	WY_IN_PARA_ERR = -3,
	WY_NOT_SUPPORT = -4,
	WY_NO_RES_ERR = -5,  //no resource is available, ex. encoder channel 
	WY_INNER_ERR = -6
}WY_RET;

typedef enum {
    WY_H264_ENC = 0,    
	WY_JPG_SNAP,
	WY_MD_VDA,
	WY_OD_VDA,  
	WY_VOUT,
	WY_UNKNOWN_USAGE
}WY_CHAIN_USAGE;

typedef enum 
{
    BQJ_MB = 0,
	BQJ_MR,
    UNKNOWNVA   
}WY_VA_ID;

typedef enum
{
   WY_OB_MATCH=8,
   WY_OB_NMATCH=7,
}WY_MATCH;

typedef enum 
{
    WY_TRAIN = 0,
	WY_PROCESS,
    UNKNOWNVAOP   
}WY_VAOP_ID;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif











































