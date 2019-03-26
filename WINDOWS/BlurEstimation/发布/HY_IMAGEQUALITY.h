/*!
* \file HY_IMAGEQUALITY.h
* \brief  the API function
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#ifndef _HY_IMAGEQUALITY_H
#define _HY_IMAGEQUALITY_H

#include"amcomdef.h"
#include <stdio.h>
//#include "deque_whayer.h"

#ifdef ENABLE_DLL
#define IQ_API	__declspec(dllexport)
#else
#define	IQ_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
	/* Defines for image color format*/
#define HY_IMAGE_GRAY				0x00		//chalk original 0x0A 	
#define HY_IMAGE_YUYV               0x02        //y,u,y,v,y,u,y,v......
#define HY_IMAGE_RGB                0x04        //r,g,b,r,g,b.....
#define HY_IMAGE_BGR                0x06        //b,g,r,b,g,r.....
#define HY_IMAGE_YUV420          0x08       //yuv420.....
#define MAX_DESCRITPTOR_NUMBER 1000             //一个类中的最多描述符的数量
#define MAX_CLASS_NUMBER 100                    //描述符集合中最多类的数量

#define IMAGE_CLEAR      900
#define IMAGE_NOTCLEAR    901
#define IMAGE_HEVNOTCLEAR     902

#define IMAGE_UNNOISE     904
#define IMAGE_NOISE    905
#define IMAGE_HEAVENOISE  906

#define IMAGE_TOODARK     907
#define IMAGE_TOOLIGHT    908

#define IMAGE_CAST        909

#define IMAGE_SIGNALLOST  911
#define IMAGE_FROZEN      912

	/* Defines for image data*/
	typedef struct {
		MLong		lWidth;				// Off-screen width
		MLong		lHeight;			// Off-screen height
		MLong		lPixelArrayFormat;	// Format of pixel array
		union
		{
			struct
			{
				MLong lLineBytes; 
				MVoid *pPixel;
			} chunky;
			struct
			{
				MLong lLinebytesArray[4];
				MVoid *pPixelArray[4];
			} planar;
		} pixelArray;
	} IQ_IMAGES, *IQ_PIMAGES;

//#define IMGBUFSIZE 50//图片改为2
	typedef struct  
	{
		MHandle	hMemMgr;
		MLong patchsize;
		MFloat condf;
		MLong decim;
		MLong itr;

		//Whayer_Queue *pImgBufs;//图片buf数据的指针
  //      MLong nImgBufSize;
	}IMQU,*PIMQU;

	/// \brief the image quality parameter struct
	typedef struct
	{
		struct
		{
            MFloat CLEARLEVEL; /// \ClearLevel
			MLong isDetct;//0没有检测 1检测过
		}CLEAR;
		
		struct
		{
			MFloat sigma1;
			MFloat sigma2;
		}Sigma;
		struct
		{
            MFloat NOISELEVEL; /// \noiselevel
			MLong isDetct;//0没有检测 1检测过
		}NOISE;
		
		struct  
		{
			MFloat BrightLevel1;
			MFloat BrightLevel2;
			MFloat Brightderiv;/// \luminace derivation
			MLong grayNum;//计算在Bright函数中
			MLong isDetct;//0没有检测 1检测过
		}Bright;
		struct  
		{
			MFloat Rcastratio; /// \ R castraitio
			MFloat Gcastratio; /// \G castratio
			MFloat Bcastratio; /// \B castratio
			MLong flag;
			MLong isDetct;//0没有检测 1检测过
		}ImgCast;
		struct 
		{
			MLong flag;  /// \while flag=0, signal is normal,flag=1,signal lost
			MLong isDetct;//0没有检测 1检测过
		}SignalLost;
		struct 
		{
			MLong flag;  /// \while flag=0, signal is normal,flag=1,image frozen
			MLong isDetct;//0没有检测 1检测过
		}ImgFrozen;
		//MFloat CoverDustLevel; /// \ClearLevel
	}HYIQ_TOutParam,*HYIQ_PTOutParam;
typedef struct  
{
	struct
	{
		MFloat ClearLevellowthresh;/// if blur level is less than this threshhold,the image is not blur
		MFloat ClearLevelhighthresh;///if blur level is larger than ClearLevellowthresh,but less than this ,the image is a  blurred;
		//MFloat ClearLevelhighthresh1;///if blur level is between ClearLevelhighthresh and ClearLevelhighthresh1,the image is median blurred,when the image is larger than ClearLevelhighthresh1,the image is seriouly blurred.
	}ClearLevel;
	struct
	{
		MFloat NoiseLevellowthresh;/// if blur level is less than this threshhold,the image is not too noise
		MFloat Noiselevelhighthresh;///if blur level is larger than ClearLevellowthresh,but less than this ,the image is a  noise;
		//MFloat Noiselevelhighthresh1;///if blur level is between ClearLevelhighthresh and ClearLevelhighthresh1,the image is median noise,when the image is larger than ClearLevelhighthresh1,the image is seriouly noise.
	}Noiselevel;
	struct
	{
		MFloat darkthresh; ///when small than this thresh,the image is too dark
		MFloat lightthresh;/// when larger than this thresh the image is too light

	}Light;
	MFloat castratio;///when larger than this ratio,the image is cast;

	//MFloat CoverDustThreshold;
}CustomParam,*PCustomParam;
typedef struct 
{
	MLong Noisestatus; //噪声判别状况
	MLong lightstatus; ///亮度判别状况
	MLong clearstatus;  ///清晰判别状况
	MLong caststatus;  ///偏色判别状况
	MLong signalLoststatus;  ///图像信号丢失判别状况
	MLong imgFrozenstatus;  ///图像冻结判别状况
	//MLong coverDuststatus; ///图像蒙尘判别状况
}HYIQ_result,*pHYIQ_result;

	/* Defines for error*/
#define HYIQ_ERR_NONE					0
#define HYIQ_ERR_UNKNOWN				-1
#define HYIQ_ERR_IMAGE_FORMAT			-101
#define HYIQ_ERR_IMAGE_SIZE_UNMATCH	    -102
#define HYIQ_ERR_ALLOC_MEM_FAIL		    -201
#define HYIQ_ERR_NO_FIND				-901

	IQ_API MHandle HYIQ_MemMgrCreate(MVoid * pMem, MLong lMemSize);
	IQ_API MVoid   HYIQ_MemMgrDestroy(MHandle hMemMgr);
	IQ_API MRESULT HYIQ_Init(MHandle hMemMgr,MHandle *pIQreg,MLong nImgBufSize);
	IQ_API MRESULT HYIQ_NOISE(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1,HYIQ_PTOutParam ptOutParam);
	IQ_API MRESULT HYIQ_SaveImage(MHandle hHandle,IQ_PIMAGES pImagesrc,const char* filename);
	IQ_API MRESULT HYIQ_LOST(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam);
	IQ_API MRESULT HYIQ_FROZEN(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam);
	IQ_API MRESULT HYIQ_CLEAR(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1,HYIQ_PTOutParam ptOutParam);

    IQ_API MRESULT HYIQ_CUTIMAGE(IQ_PIMAGES pImage_,IQ_PIMAGES pImage);

	IQ_API MRESULT HYIQ_LostBaseHist(HYIQ_PTOutParam ptOutParam,char * pSrcImg,MLong nImgWidth,MLong nImgHeight);

	IQ_API MRESULT HYIQ_BRIGHT(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1, HYIQ_PTOutParam ptOutParam);
	IQ_API MRESULT HYIQ_ENHANCE(MHandle hHandle,IQ_PIMAGES pImagesrc,IQ_PIMAGES pImageres);
	IQ_API MRESULT HYIQ_CAST(MHandle hHandle,IQ_PIMAGES pImage, HYIQ_PTOutParam ptOutParam );
	IQ_API MRESULT HYIQ_Uninit(MHandle HYIQhandle);
    IQ_API MRESULT HYIQ_RESULT(MHandle hHandle,HYIQ_PTOutParam ptOutParam,PCustomParam customp,pHYIQ_result result);

	typedef struct
	{
		MLong lCodebase;			// Codebase version number 
		MLong lMajor;				// major version number 
		MLong lMinor;				// minor version number
		MLong lBuild;				// Build version number, increasable only
		const MChar *Version;		// version in string form
		const MChar *BuildDate;	// latest build Date
		const MChar *CopyRight;	// copyright 
	} IQ_Version;
	IQ_API const IQ_Version * IQ_GetVersion();


#ifdef __cplusplus
}
#endif

#endif