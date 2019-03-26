#ifndef _WYMATCHRIGIDBODYSTRUCT_
#define _WYMATCHRIGIDBODYSTRUCT_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


typedef struct
{
    unsigned char *bitBufAddr;
    int width;
    int height;
    int startX;
    int startY;
}wy_maskBitInfo;

typedef struct
{
    unsigned char *byteBufAddr;
    int width;
    int height;
    int startX;
    int startY;
}wy_maskByteInfo;


typedef struct
{
    unsigned char *byteBufAddr;
    unsigned int u32PhyAddr;
    int width;
    int height;
    int stride;
}wy_picInfo;

typedef struct
{
    int obRecWidthS;
    int obRecHeightS;
    int obPixelNum;
    int angle;
    //int matchNum;
    unsigned char *pData;
}wy_objCharacterDes;

typedef struct
{
    wy_maskByteInfo maskByteInfo;
    wy_picInfo objPic;
    wy_picInfo rotateObjPic; 
    wy_picInfo rotateMaskPic;
    wy_picInfo salorSfObjPic;
    wy_picInfo objMaskPic; 
    wy_picInfo sfObjPic;
    int *lx; 
    int *ly;
    unsigned char *pAngle;
    unsigned short *pMag;
	unsigned int u32PhyAddrAge;
	unsigned int u32PhyAddrMag;	
    int magAngStride;
    unsigned char *pMask;
}wy_trainPMemUg;

typedef struct
{
    wy_picInfo sfPic;
    unsigned char *pAngle;
    unsigned short *pMag;
    unsigned int u32PhyAddrAge;
    unsigned int u32PhyAddrMag;
    int magAngStride;
    unsigned char *pBinVal;
	void *desTemp;
}wy_matchPMemUg;

typedef struct
{
   unsigned char *p60K_0; //divide into 2 parts: p30K_0    p30K_1
   unsigned char *p60K_1; //divide into 2 parts: p30K_2    p30K_3
   
   unsigned char *p16K_0;
   unsigned char *p16K_1;
   
   unsigned char *p16K_Des;
   int p16KDesQdmaId;
   
   unsigned char *p30K_0;
   unsigned char *p30K_1;
   unsigned char *p30K_2;
   unsigned char *p30K_3;                                     
}wy_l2SramUge;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif












































