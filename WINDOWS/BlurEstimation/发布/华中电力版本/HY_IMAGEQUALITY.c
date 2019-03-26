/*!
* \file HY_IMAGEQUALITY.C
* \brief  the API function
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#include"HY_IMAGEQUALITY.h"
#include"lidebug.h"
#include "liblock.h"
#include "licomdef.h"
#include"litimer.h"
#include"liimage.h"
#include"liimgfmttrans.h"
#include"liedge.h"
#include"liconv.h"
#include"liimfilter.h"
#include"linoise.h"
#include"limatrix.h"
#include"LiErrFunc.h"
#include"ligaussian.h"
#include"liblur.h"
#include"limath.h"
#include"DFT.h"
#include"LiBright.h"
#include "lichannel.h"
#include"Lihist.h"
#include "FmttransYUV2BGR.h"
#include"LiFrmDif.h"
#include "vl/gmm.h"
#include<time.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#define TYPE float
#define VL_F_TYPE VL_TYPE_FLOAT

#define HY_CHECK_IS_OUTTIME  //是否需要检查客户使用超时
#define max(a,b) ( ((a)>(b)) ? (a):(b) )
///\检查是否超时
MLong HYIQ_CheckOutTime()
{
	time_t seconds= time(NULL);//180天180*24*3600=15552000
	if (seconds>1474689757)//1459137757+15552000=1474689757
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}

/// \brief _TransToInteriorImgFmt trans the out formation to the inner formation
/// \param PIMG the input img;

JOFFSCREEN _TransToInteriorImgFmt(const IQ_IMAGES* pImg)
{
	JOFFSCREEN img = *(JOFFSCREEN*)pImg;
	switch(img.fmtImg)
	{

	case HY_IMAGE_GRAY:
		img.fmtImg = FORMAT_GRAY;
		break;
	case HY_IMAGE_YUYV:
		img.fmtImg = FORMAT_YUYV;
		break;
	case HY_IMAGE_RGB:
		img.fmtImg = FORMAT_RGB;
		break;
	case HY_IMAGE_BGR:
		img.fmtImg = FORMAT_BGR;
		break;
	case HY_IMAGE_YUV420:
		img.fmtImg =FORMAT_YUV420;

#ifdef TRIM_RGB

#endif
	default:
		JASSERT(MFalse);
		break;
	}
	JASSERT(IF_DATA_BYTES(img.fmtImg) == 1);
	return img;
}


/// \brief TransGrayToBlock trans gray image to block
/// \param Image the source gray image
/// \param pBlockImg the block image
/// \return the error code

MVoid TransGrayImgToBlock(JOFFSCREEN Image,PBLOCK pBlockImg)
{
	pBlockImg->lBlockLine=Image.pixelArray.chunky.dwImgLine;
	pBlockImg->lHeight=Image.dwHeight;
	pBlockImg->lWidth=Image.dwWidth;
	pBlockImg->typeDataA=DATA_U8;
	pBlockImg->pBlockData=Image.pixelArray.chunky.pPixel;
}


/// \brief HYIQ_Init  Initial the memory
/// \param hMemMgr Handle
/// \param pIQreg the handle
/// \return the error code
MRESULT HYIQ_Init(MHandle hMemMgr,MHandle *pIQreg)
{
	MRESULT res=0;
	PIMQU PMIQ=MNull;

	AllocVectMem(hMemMgr,PMIQ,1,IMQU);
	SetVectMem(PMIQ,1,0,IMQU);
	PMIQ->hMemMgr=hMemMgr;
	//PMIQ->pImgBufs=InitQueue();
	//PMIQ->nImgBufSize=nImgBufSize;
	*pIQreg=(MHandle)PMIQ;

EXT:
	return res;	
}

//MRESULT HYIQ_CUTIMAGE(IQ_PIMAGES pImage_,IQ_PIMAGES pImage)
//{
//	MRESULT res=0;
//	pImage_->pixelArray.chunky.pPixel=(MPChar)pImage->pixelArray.chunky.pPixel+((int)(0.2*pImage->lHeight)*pImage->pixelArray.chunky.lLineBytes);
//	pImage_->lHeight=pImage->lHeight*0.6;
//	pImage_->lWidth=pImage->lWidth;
//	pImage_->pixelArray.chunky.lLineBytes=pImage->pixelArray.chunky.lLineBytes;
//	pImage_->lPixelArrayFormat=pImage->lPixelArrayFormat;
//
//EXT:
//	return res;
//
//}

/// \brief HYIQ_NOISE  Estimate the noise level
/// \param hMemMgr Handle
/// \param pImage the source image
/// \param pImage1 the source image 
/// \param ptOutParam output parameters 
/// \return the error code
MRESULT HYIQ_NOISE(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1,HYIQ_PTOutParam ptOutParam)
{
	MRESULT res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN ImgSrc  = _TransToInteriorImgFmt(pImage);
	JOFFSCREEN imgSrc1= _TransToInteriorImgFmt(pImage1);
	JOFFSCREEN Imggray={0};
	JOFFSCREEN imggray1={0};
	BLOCK blockimgsrc={0};
	BLOCK blockimgsrc1={0};
	MLong w,h,h1;
	BLOCKEXT blockimg1={0};
	BLOCKEXT blockplain={0};
	BLOCK blockgray1={0};
	BLOCK blockgray={0};
	MFloat NoiseL=0;
	JOFFSCREEN imgbgr={0};
	JOFFSCREEN imgbgr1={0};

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif

	/// \judge if the input is empty or not
	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	/// \if fmtimg=yuv420, transform to bgr
	/// \for single image 对应于单张图像
	///如果驶入是YUV420信号，则将其转化为BGR信号做后续处理，主要针对视频流信号
	if (imgSrc1.pixelArray.chunky.pPixel==MNull)
	{
		switch (ImgSrc.fmtImg)
		{
		case FORMAT_GRAY:
			TransGrayImgToBlock(ImgSrc,&blockimgsrc);
			break;
		case FORMAT_YUV420:
			GO( ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(FmttransYUV2BGR(&ImgSrc,&imgbgr));
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&imgbgr,&Imggray));
			TransGrayImgToBlock(Imggray,&blockimgsrc);
			break;
		case FORMAT_RGB:
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&ImgSrc,&Imggray));
			TransGrayImgToBlock(Imggray,&blockimgsrc);
			break;
		default:
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&ImgSrc,&Imggray));
			TransGrayImgToBlock(Imggray,&blockimgsrc);
			break;       
		}
		///筛选掉图像最上面高度10%和最下面10%的像素，因为会有osd影响判别
		w=blockimgsrc.lWidth;
		h=0.9*blockimgsrc.lHeight;
		h1=0.1*blockimgsrc.lHeight;
		blockimg1.block=blockimgsrc;
		blockimg1.ext.left=0;
		blockimg1.ext.right=blockimgsrc.lWidth;
		blockimg1.ext.top=h1;
		blockimg1.ext.bottom=h;
		blockplain.block=blockimgsrc;
		blockplain.ext.left=0;
		blockplain.ext.right=blockimgsrc.lWidth;
		blockplain.ext.top=h1;
		blockplain.ext.bottom=h;
		/// \choose a block to do the noise level estimate
		/// \选出边缘信息最少的区块，1/16去判别噪声
		GO(SelePlainBlock(hMemMgr,&blockplain,&blockimg1));
		GO(NoiseLevel(hMemMgr,&NoiseL,&blockplain,7,0,0.99,3));///参数由代码matlab版本给出
	}

	/// \noise level estimate for videos
	/// 用帧差信息判别视频图像的噪声
	if (imgSrc1.pixelArray.chunky.pPixel!=MNull)
	{
		/// \format  transformation
		switch (ImgSrc.fmtImg)
		{
		case FORMAT_GRAY:
			TransGrayImgToBlock(ImgSrc,&blockimgsrc);
			break;
		case FORMAT_YUV420:
			GO( ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(FmttransYUV2BGR(&ImgSrc,&imgbgr)); 
			///* PrintBmpEx(imgbgr.pixelArray.chunky.pPixel,imgbgr.pixelArray.chunky.dwImgLine,DATA_U8,imgbgr.dwWidth,imgbgr.dwHeight,3,"F:\\2.bmp");
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&imgbgr,&Imggray));
			////* PrintBmpEx(Imggray.pixelArray.chunky.pPixel,Imggray.pixelArray.chunky.dwImgLine,DATA_U8,Imggray.dwWidth,Imggray.dwHeight,1,"F:\\1.bmp");
			TransGrayImgToBlock(Imggray,&blockimgsrc);
			break;
		case FORMAT_RGB:
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&ImgSrc,&Imggray));
			TransGrayImgToBlock(Imggray,&blockimgsrc);
			break;
		default:
			GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&ImgSrc,&Imggray));
			TransGrayImgToBlock(Imggray,&blockimgsrc);
		}
		switch (imgSrc1.fmtImg)
		{
		case FORMAT_GRAY:
			TransGrayImgToBlock(imgSrc1,&blockimgsrc1);
			break;
		case FORMAT_YUV420:
			GO( ImgCreate(hMemMgr,&imgbgr1,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(FmttransYUV2BGR(&imgSrc1,&imgbgr1));
			GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&imgbgr1,&imggray1));
			TransGrayImgToBlock(imggray1,&blockimgsrc1);
			break;
		case FORMAT_RGB:
			GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&imgSrc1,&imggray1));
			TransGrayImgToBlock(imggray1,&blockimgsrc1);
			break;
		default:
			GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&imgSrc1,&imggray1));
			TransGrayImgToBlock(imggray1,&blockimgsrc1);
		}
		/// \noise level estimate for 2 images 用前后帧图像判别视频噪声
		GO(NoiseLevelFrames(hMemMgr,&NoiseL,&blockimgsrc,&blockimgsrc1));
	}

	////华中电力定制
	if (NoiseL>100)
	{
		NoiseL=100;
	}
	/// \output the noise level
	/// 输出噪声等级
	ptOutParam->NOISE.NOISELEVEL=NoiseL;
EXT:

	/// \release the memory释放内存
	ImgRelease(hMemMgr,&Imggray);
	ImgRelease(hMemMgr,&imggray1);
	ImgRelease(hMemMgr,&imgbgr);
	ImgRelease(hMemMgr,&imgbgr1);

	return res;
}

/// \brief HYIQ_BLUR  Estimate the blur level
/// \param hMemMgr Handle
/// \param pImage the source image
/// \param pImage1 the source image 
/// \param ptOutParam output parameters 
/// \return the error code
MRESULT HYIQ_CLEAR(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1,HYIQ_PTOutParam ptOutParam)
{
	MRESULT res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN ImgSrc  = _TransToInteriorImgFmt(pImage);
	JOFFSCREEN ImgSrc1={0};
	BLOCK BLOCKImage={0}; 
	BLOCK BLOCKImage1={0};
	BLOCKEXT blockimg1={0};
	BLOCKEXT blockplain={0};
	BLOCKEXT blocksel={0};
	BLOCKEXT blockedge={0};
	BLOCK edgemap={0};
	BLOCKEXT BlockP;
	MLong j,w,h,h1,i;
	MFloat NoiseL=0;
	MUInt8 *data2;
	MUInt8 *dataedge;
	MFloat sigma,sigma1,sigma2,sigma3;
	MLong a1,b1,a2,b2,a3,b3;
	BLOCK imga={0};
	BLOCK imgb={0};
	BLOCKEXT ImgA={0};
	BLOCKEXT ImgB={0};
	BLOCK blockp={0};
	MFloat eps=0.0000001;
	JOFFSCREEN imggray={0};
	JOFFSCREEN imggray2={0};
	JOFFSCREEN imgbgr={0};
	JOFFSCREEN imgbgr1={0};

#ifdef _ZMH_DEBUG
	IplImage *showImg=cvCreateImage(cvSize(pImage->lWidth,pImage->lHeight*0.8+1),IPL_DEPTH_8U,1);
	IplImage *subShowImg1=cvCreateImage(cvSize(showImg->width/4,showImg->height/4),IPL_DEPTH_8U,1);
	//IplImage *subShowImg2=cvCreateImage(cvSize(showImg->width/2,showImg->height/2),IPL_DEPTH_8U,1);
#endif

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif
	///如果输入图像为空，则返回错误
	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	///传入句柄
	hMemMgr = pImqu->hMemMgr;

	/// \Initial the image format	 将图像格式转化为block格式的灰度图
	switch(ImgSrc.fmtImg)
	{
	case FORMAT_GRAY:
		TransGrayImgToBlock(ImgSrc,&BLOCKImage);
		break;
	case FORMAT_YUV420:
		GO( ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(FmttransYUV2BGR(&ImgSrc,&imgbgr));
		GO(ImgCreate(hMemMgr,&imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&imgbgr,&imggray));
		TransGrayImgToBlock(imggray,&BLOCKImage);
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&ImgSrc,&imggray));
		TransGrayImgToBlock(imggray,&BLOCKImage);
		break;
	default:
		GO(ImgCreate(hMemMgr,&imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&ImgSrc,&imggray));
		TransGrayImgToBlock(imggray,&BLOCKImage);
		break;       
	}


	/// \preprocessing :select the plain part of the image
	/// \ choose the 0.9height part of the image
	///预处理，如果输入噪声信息为空，则计算单张图像噪声
	///选择中间0.8*h的图像
	w=BLOCKImage.lWidth;
	h=0.9*BLOCKImage.lHeight;
	h1=0.1*BLOCKImage.lHeight;
	blockimg1.block=BLOCKImage;
	blockimg1.ext.left=0;
	blockimg1.ext.right=BLOCKImage.lWidth;
	blockimg1.ext.top=h1;
	blockimg1.ext.bottom=h;
	blockplain.block=BLOCKImage;
	blockplain.ext.left=0;
	blockplain.ext.right=BLOCKImage.lWidth;
	blockplain.ext.top=h1;
	blockplain.ext.bottom=h;
	/// \choose a block to do the noise estimation
	/// \找到图像中较平坦的1/16区域
	GO(SelePlainBlock(hMemMgr,&blockplain,&blockimg1));

	/// \step1 noise level estimation 噪声等级估计
	/// \judge the noise level,if noise level is empty,then do the noise level estimate
	if (ptOutParam->NOISE.NOISELEVEL<eps)
	{
		//pImage1->pixelArray.chunky.pPixel=MNull;//test
		if(pImage1->pixelArray.chunky.pPixel==MNull)
		{
			GO(NoiseLevel(hMemMgr,&NoiseL,&blockplain,7,0,0.99,3));
		}
		if ((pImage1->pixelArray.chunky.pPixel!=MNull))
		{
			ImgSrc1  = _TransToInteriorImgFmt(pImage1);

			switch (ImgSrc1.fmtImg)
			{
			case FORMAT_GRAY:
				TransGrayImgToBlock(ImgSrc1,&BLOCKImage1);
				break;
			case FORMAT_YUV420:
				GO( ImgCreate(hMemMgr,&imgbgr1,FORMAT_BGR,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
				GO(FmttransYUV2BGR(&ImgSrc1,&imgbgr1));
				GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
				GO(ImgFmtTrans(&imgbgr1,&imggray2));
				TransGrayImgToBlock(imggray2,&BLOCKImage1);
			case FORMAT_RGB:
				GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
				GO(ImgFmtTrans(&ImgSrc1,&imggray2));
				TransGrayImgToBlock(imggray2,&BLOCKImage1);
				break;
				break;
			default:
				GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
				GO(ImgFmtTrans(&ImgSrc1,&imggray2));
				TransGrayImgToBlock(imggray2,&BLOCKImage1);
			}
			GO(NoiseLevelFrames(hMemMgr,&NoiseL,&BLOCKImage,&BLOCKImage1));
		}
	}
	/// \if noise level is not empty,use the noise level for further estimation
	if (ptOutParam->NOISE.NOISELEVEL>eps)
	{
		NoiseL=ptOutParam->NOISE.NOISELEVEL;
	}

	/// \step2 Improved canny method 用估算出来的噪声等级检测canny边缘
	GO(B_Create(hMemMgr,&edgemap,DATA_U8,blockimg1.block.lWidth,blockimg1.ext.bottom-blockimg1.ext.top));///边缘图像
	B_Set(&edgemap,0);
	data2=(MUInt8*)blockimg1.block.pBlockData+blockimg1.ext.left+(blockimg1.ext.top)*blockimg1.block.lBlockLine;
	dataedge=(MUInt8*)edgemap.pBlockData;
	///根据噪声的canny边缘检测
	GO(NoiseEdge(hMemMgr,data2,blockimg1.block.lBlockLine,(blockimg1.ext.right-blockimg1.ext.left),(blockimg1.ext.bottom-blockimg1.ext.top), dataedge,edgemap.lBlockLine,TYPE_CANNY,NoiseL));

#ifdef _ZMH_DEBUG
	showImg->imageData=(char*)edgemap.pBlockData;
	cvShowImage("edge1",showImg);
	cvWaitKey(2);
#endif
	blockedge.ext.top=0;
	blockedge.ext.left=0;
	blockedge.ext.bottom=edgemap.lHeight;
	blockedge.ext.right=edgemap.lWidth;
	blockedge.block=edgemap;
	//PrintBmpEx(dataedge,blockedge.block.lBlockLine,DATA_U8,blockedge.ext.right-blockedge.ext.left,blockedge.ext.bottom-blockedge.ext.top,1,"F:\\edge1.bmp");
	///选择边缘信息强烈的1/4区域
	SeleBlock(hMemMgr,&blockimg1,&blockedge);
	data2=(MUInt8*)blockimg1.block.pBlockData+blockimg1.ext.left+(blockimg1.ext.top)*blockimg1.block.lBlockLine;
	dataedge=(MUInt8*)blockedge.block.pBlockData+blockedge.ext.left+(blockedge.ext.top)*(blockedge.block.lBlockLine);

	//#endif
	/// \step3 reblur the image 
	/// 再模糊图像，用sigma=1和sigma=2的参数模糊图像，这两个参数是由实验测得
	/// \dealing with a1，b1
	a1=1;
	b1=2;
	GO(B_Create(hMemMgr,&imga,DATA_U8,blockimg1.ext.right-blockimg1.ext.left,blockimg1.ext.bottom-blockimg1.ext.top));
	GO(B_Create(hMemMgr,&imgb,DATA_U8,blockimg1.ext.right-blockimg1.ext.left,blockimg1.ext.bottom-blockimg1.ext.top));
	B_Set(&imga,0);
	B_Set(&imgb,0);
	ImgA.block=imga;
	ImgA.ext.left=0;
	ImgA.ext.right=imga.lWidth;
	ImgA.ext.top=0;
	ImgA.ext.bottom=imga.lHeight;
	ImgB.block=imgb;
	ImgB.ext.left=0;
	ImgB.ext.right=imgb.lWidth;
	ImgB.ext.top=0;
	ImgB.ext.bottom=imgb.lHeight;

	///用sigma=a1进行在模糊，存储在imgA中
	GO(LiGaussianBlur(hMemMgr,&ImgA,&blockimg1,a1));
	//PrintBmpEx((MUInt8*)imga.pBlockData,imga.lBlockLine,DATA_U8,imga.lWidth,imga.lHeight,1,"c:\\imga.bmp");
	///用sigma=b1进行在模糊，存储在imgB中
	GO(LiGaussianBlur(hMemMgr,&ImgB,&blockimg1,b1));

	GO(B_Create(hMemMgr,&blockp,DATA_F64,blockimg1.ext.right-blockimg1.ext.left,blockimg1.ext.bottom-blockimg1.ext.top));
	//    PrintBmpEx((MUInt8*)imgb.pBlockData,imgb.lBlockLine,DATA_U8,imgb.lWidth,imgb.lHeight,1,"c:\\imgb.bmp");
	B_Set(&blockp,0);
	ImgA.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	ImgB.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.block=blockp;
	BlockP.ext.left=0;
	BlockP.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.ext.top=0;
	BlockP.ext.bottom=ImgA.ext.bottom;

	///计算两幅图像之间的梯度区别，根据matlab算法改编  BlockP 梯度图 blockimg1灰度图  blockedge边缘图 ImgA ImgB模糊图像 这几个图像大小一致
	GO(LiBlur(hMemMgr,&BlockP,&blockimg1,&blockedge,&ImgA,&ImgB));
	///根据两幅图像的梯度信息，还有sigmaa，sigmab获得预测图像的sigma
	GO(Lisigma(hMemMgr,&sigma1,&BlockP,a1,b1));

	/// \dealing with a2，b2
	///进行第二次的在模糊同1
	a2=5;
	b2=7;
	B_Set(&imga,0);
	B_Set(&imgb,0);
	ImgA.block=imga;
	ImgA.ext.left=0;
	ImgA.ext.right=imga.lWidth;
	ImgA.ext.top=0;
	ImgA.ext.bottom=imga.lHeight;
	ImgB.block=imgb;
	ImgB.ext.left=0;
	ImgB.ext.right=imgb.lWidth;
	ImgB.ext.top=0;
	ImgB.ext.bottom=imgb.lHeight;

	GO(LiGaussianBlur(hMemMgr,&ImgA,&blockimg1,a2));
	GO(LiGaussianBlur(hMemMgr,&ImgB,&blockimg1,b2));
	B_Set(&blockp,0);
	ImgA.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	ImgB.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.block=blockp;
	BlockP.ext.left=0;
	BlockP.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.ext.top=0;
	BlockP.ext.bottom=ImgA.ext.bottom;
	GO(LiBlur(hMemMgr,&BlockP,&blockimg1,&blockedge,&ImgA,&ImgB));
	GO(Lisigma(hMemMgr,&sigma2,&BlockP,a2,b2));
	///根据两次在模糊估算出来的sigma值，和阈值进行比较，得到最后的sigma
	GO(SigmaSel(&sigma,sigma1,sigma2));

	////华中电力定制
	if (sigma>100)
	{
		sigma=100;
	}
	///输出结果
	ptOutParam->CLEAR.CLEARLEVEL=sigma;
	ptOutParam->NOISE.NOISELEVEL=NoiseL;
	ptOutParam->Sigma.sigma1=sigma1;
	ptOutParam->Sigma.sigma2=sigma2;
EXT:
	///删除内存
	B_Release(hMemMgr,&edgemap);
	B_Release(hMemMgr,&imga);
	B_Release(hMemMgr,&imgb);
	B_Release(hMemMgr,&blockp);
	ImgRelease(hMemMgr,&imggray);
	ImgRelease(hMemMgr,&imggray2);
	ImgRelease(hMemMgr,&imgbgr);
	ImgRelease(hMemMgr,&imgbgr1);

	return res;
}

/// \brief HYIQ_BRIGHT  check whether the bright is abnormal
/// \param hMemMgr Handle
/// \param pImage the source image
/// \param ptOutParam output parameters 
/// \return the error code

/// 根据图像亮度均值和128之间的差别判别是否存在亮度异常
MRESULT HYIQ_BRIGHT(MHandle hHandle,IQ_PIMAGES pImage, IQ_PIMAGES pImage1,HYIQ_PTOutParam ptOutParam)
{
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	MRESULT res=LI_ERR_NONE;
	JOFFSCREEN ImgSrc  = _TransToInteriorImgFmt(pImage);
	JOFFSCREEN ImgSrc1=_TransToInteriorImgFmt(pImage1);
	MFloat k,k1;
	MLong i,j;
	MLong ext;
	MUInt8 *data;
	MLong sum=0;
	MFloat mean=0.0;
	MLong len=ImgSrc.dwHeight*ImgSrc.dwWidth;
	JOFFSCREEN Imggray={0};
	JOFFSCREEN Imgbgr={0};
	JOFFSCREEN Imgbgr1={0};
	JOFFSCREEN Imggray1={0};
	MFloat temp=0;
	MLong H[256];
#ifdef _ZMH_DEBUG
	IplImage *pHistimg = cvCreateImage( cvSize(320,300), 8, 3 );
	float val=0.0;
	MLong maxH=0;
#endif

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif

	for(i=0;i<256;i++)
	{
		H[i]=0;
	}

	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	///将图像转到灰度图像
	switch(ImgSrc.fmtImg)
	{
	case  FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&Imgbgr,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
		FmttransYUV2BGR(&ImgSrc,&Imgbgr);
		GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&Imgbgr,&Imggray));
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&ImgSrc,&Imggray));
		break;
	default:
		GO(ImgCreate(hMemMgr,&Imggray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
		GO(ImgFmtTrans(&ImgSrc,&Imggray));
	}
	///计算图像1的亮度
	data=(MUInt8*)Imggray.pixelArray.chunky.pPixel;
	ext=Imggray.pixelArray.chunky.dwImgLine-Imggray.dwWidth;
	for(i=0;i<Imggray.dwHeight;i++,data+=ext)
		for(j=0;j<Imggray.dwWidth;j++,data++)
		{
			sum+=*(data);
		}
		mean=(float)sum/len;
		k=(mean-128)/128;

		////华中电力定制
		k=k*50+50;
		ptOutParam->Bright.BrightLevel1=k;
		///计算图像2的亮度
		switch(ImgSrc1.fmtImg)
		{
		case  FORMAT_YUV420:
			GO(ImgCreate(hMemMgr,&Imgbgr1,FORMAT_BGR,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
			FmttransYUV2BGR(&ImgSrc1,&Imgbgr1);
			GO(ImgCreate(hMemMgr,&Imggray1,FORMAT_GRAY,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
			GO(ImgFmtTrans(&Imgbgr1,&Imggray1));
			break;
		case FORMAT_RGB:
			GO(ImgCreate(hMemMgr,&Imggray1,FORMAT_GRAY,ImgSrc1.dwWidth,ImgSrc1.dwHeight));
			GO(ImgFmtTrans(&ImgSrc1,&Imggray1));
			break;
		default:
			GO(ImgCreate(hMemMgr,&Imggray1,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
			GO(ImgFmtTrans(&ImgSrc1,&Imggray1));
		}
		sum=0;
		data=(MUInt8*)Imggray1.pixelArray.chunky.pPixel;
		ext=Imggray1.pixelArray.chunky.dwImgLine-Imggray1.dwWidth;
		for(i=0;i<Imggray1.dwHeight;i++,data+=ext)
			for(j=0;j<Imggray1.dwWidth;j++,data++)
			{
				H[*(data)]=H[*(data)]+1;
				sum+=*(data);
			}

			//#ifdef _ZMH_DEBUG
			//			// 画直方图
			//			for( i= 0; i < 256;i++ )
			//			{
			//				if (H[i]>maxH)
			//				{
			//					maxH=H[i];
			//				}
			//			}
			//			cvSet(pHistimg,cvScalar(0,0,0,0),NULL);
			//			for(i= 0; i < 256;i++ )
			//			{
			//				val =pHistimg->height - (H[i]*300)/maxH;
			//				if (val<0)
			//				{
			//					val=0.0;
			//				}
			//				cvRectangle(pHistimg, cvPoint(i*pHistimg->width/256,pHistimg->height),cvPoint((i+1)*pHistimg->width/256,(int)(val)),CV_RGB(255,255,0,0), 1, 8, 0 );
			//			}
			//			cvShowImage("Hist",pHistimg);
			//			cvWaitKey(0);
			//#endif
			mean=(float)sum/len;
			k1=(mean-128)/128;

			////华中电力定制
			k1=k1*50+50;
			///两帧图像之间的亮度差别，用于后续判断
			temp=fabs((k-k1)/k);
			ptOutParam->Bright.BrightLevel2=k1;
			ptOutParam->Bright.Brightderiv=temp;
EXT:
			ImgRelease(hMemMgr,&Imggray);
			ImgRelease(hMemMgr,&Imgbgr);
			ImgRelease(hMemMgr,&Imggray1);
			ImgRelease(hMemMgr,&Imgbgr1);
			return res;
}

/// \brief HYIQ_CAST  check whether the image is cast
/// \param hMemMgr Handle
/// \param pImage the source image
/// \param ptOutParam output parameters 
/// \return the error code
MRESULT HYIQ_CAST(MHandle hHandle,IQ_PIMAGES pImage, HYIQ_PTOutParam ptOutParam)
{
	MRESULT res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN imgsrc=_TransToInteriorImgFmt(pImage);
	JOFFSCREEN imgrgb={0};
	MLong len=imgsrc.dwHeight*imgsrc.dwWidth;
	Ratio ratiosrc={0};
	double maxRatio=0.0;
	double maxDev=0.0;
	double avgDev=0.0;
	double castValue=0.0;

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif

	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	GO(ImgCreate(hMemMgr,&imgrgb,FORMAT_BGR,imgsrc.dwWidth,imgsrc.dwHeight));

	///判别输入的图像是否是bgr图像
	switch (imgsrc.fmtImg)
	{
	case FORMAT_GRAY:
		res=LI_ERR_DATA_UNSUPPORT;
		return res;
	case FORMAT_YUV420:
		FmttransYUV2BGR(&imgsrc,&imgrgb);
		break;
	case FORMAT_RGB:
		GO(ImgFmtTrans(&imgsrc,&imgrgb));
		break;
	default:
		GO(ImgFmtTrans(&imgsrc,&imgrgb));
	}
	///计算三通道的颜色偏移
	GO(LiCast(hMemMgr,&imgrgb,&ratiosrc));
	///根据三通道的颜色偏移判别是否是偏色，0.35是由实验数据得出的
	//if (ratiosrc.rratio>0.35||ratiosrc.gratio>0.35||ratiosrc.bratio>0.35)
	//{
	//	ptOutParam->ImgCast.flag=1;///偏色
	//}
	//else if(ratiosrc.bdev>0.5||ratiosrc.gdev>0.5||ratiosrc.rdev>0.5)
	//{
	//	ptOutParam->ImgCast.flag=1;///偏色
	//}
	//else
	//{
	//	ptOutParam->ImgCast.flag=0;///正常
	//}

	////华中电力定制
	maxRatio=max(max(ratiosrc.rratio,ratiosrc.gratio),ratiosrc.bratio);
	maxDev=max(max(ratiosrc.bdev,ratiosrc.gdev),ratiosrc.rdev);
    avgDev=(ratiosrc.bdev+ratiosrc.gdev+ratiosrc.rdev)/3;

	if (maxRatio>0.35)
	{
		castValue=maxRatio*30.3+39.4;
	}
	else if (maxDev>0.5)
	{
		castValue=maxDev*100;
	}
	else
	{
         castValue=avgDev*100;
	}

	ptOutParam->ImgCast.Bcastratio=ratiosrc.bratio;
	ptOutParam->ImgCast.Rcastratio=ratiosrc.rratio;
	ptOutParam->ImgCast.Gcastratio=ratiosrc.gratio;
    ptOutParam->ImgCast.CastValue=castValue;
EXT:

	ImgRelease(hMemMgr,&imgrgb);
	return res;
}



/// \brief HYIQ_Enhance  enhance the source image
/// \param hMemMgr Handle
/// \param pImage the source image
/// \param pImageres the enhanced image
/// \return the error code
/// 用于图像的亮度增强
MRESULT HYIQ_ENHANCE(MHandle hHandle,IQ_PIMAGES pImagesrc,IQ_PIMAGES pImageres)
{
	MLong res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN imgsrc=_TransToInteriorImgFmt(pImagesrc);
	JOFFSCREEN imgbgr={0};
	JOFFSCREEN imgres={0};

	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	GO(ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,imgsrc.dwWidth,imgsrc.dwHeight));
	GO(ImgCreate(hMemMgr,&imgres,FORMAT_BGR,imgsrc.dwWidth,imgsrc.dwHeight));
	if(imgsrc.fmtImg==FORMAT_GRAY)
	{
		res=LI_ERR_DATA_UNSUPPORT;
		return res;
	}
	///转成BGR形式进行直方图增强
	GO(ImgFmtTrans(&imgsrc,&imgbgr));
	///直方图增强
	GO(liEnhance(hMemMgr,&imgbgr,&imgres));
	pImageres->lHeight=imgres.dwHeight;
	pImageres->lWidth=imgres.dwWidth;
	pImageres->pixelArray.chunky.lLineBytes=imgres.pixelArray.chunky.dwImgLine;
	pImageres->pixelArray.chunky.pPixel=imgres.pixelArray.chunky.pPixel;
	switch(imgres.fmtImg)
	{
	case FORMAT_YUYV:
		pImageres->lPixelArrayFormat=HY_IMAGE_YUYV;
		break;
	case FORMAT_BGR:
		pImageres->lPixelArrayFormat=HY_IMAGE_BGR;
		break;
	case FORMAT_RGB:
		pImageres->lPixelArrayFormat=HY_IMAGE_RGB;
		break;

	default:
		JASSERT(MFalse);
		break;
	}

EXT:
	ImgRelease(hMemMgr,&imgbgr);

	return res;
}

/// \brief HYIQ_SaveImage
/// \param hMemMgr H
/// \param pImage the source image
/// \存储图像
MRESULT HYIQ_SaveImage(MHandle hHandle,IQ_PIMAGES pImagesrc,const char* filename)
{
	MLong res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN imgsrc=_TransToInteriorImgFmt(pImagesrc);
	JOFFSCREEN imgbgr={0};

	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	switch(imgsrc.fmtImg)
	{

	case FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,imgsrc.dwWidth,imgsrc.dwHeight));
		GO(FmttransYUV2BGR(&imgsrc,&imgbgr));
		PrintBmpEx((MUInt8*)imgbgr.pixelArray.chunky.pPixel,imgbgr.pixelArray.chunky.dwImgLine,DATA_U8,imgbgr.dwWidth,imgbgr.dwHeight,3,filename);
		break;
	case FORMAT_GRAY:
		PrintBmpEx((MUInt8*)imgsrc.pixelArray.chunky.pPixel,imgsrc.pixelArray.chunky.dwImgLine,DATA_U8,imgsrc.dwWidth,imgsrc.dwHeight,1,filename);
		break;
	default:
		GO(ImgCreate(hMemMgr,&imgbgr,FORMAT_BGR,imgsrc.dwWidth,imgsrc.dwHeight));
		GO(ImgFmtTrans(&imgsrc,&imgbgr));
		PrintBmpEx((MUInt8*)imgbgr.pixelArray.chunky.pPixel,imgbgr.pixelArray.chunky.dwImgLine,DATA_U8,imgbgr.dwWidth,imgbgr.dwHeight,3,filename);
	}

EXT:
	ImgRelease(hMemMgr,&imgbgr);
	return res;
}

//  根据帧差判别信号丢失还是画面冻结
MRESULT HYIQ_FROZEN(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam)
{
	MLong res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN imgsrc1=_TransToInteriorImgFmt(pImagesrc1);
	JOFFSCREEN imgsrc2=_TransToInteriorImgFmt(pImagesrc2);
	JOFFSCREEN imgbgr1={0};
	JOFFSCREEN imgbgr2={0};
	JOFFSCREEN imggray1={0};
	JOFFSCREEN imggray2={0};
	JOFFSCREEN imgedge={0};
	MUInt8* psrc=MNull;
	MUInt8 *pedge=MNull;
	MFloat diff=0.0;
	MLong n=0;
	MLong i,j;
	MLong ext=0;
	MFloat ratio=0.0;
	MLong H[256];
	MLong num=imgsrc1.dwHeight*imgsrc1.dwWidth;
	MLong tmp=0;
	MLong Hmax=0;
	MFloat Hration=0.0;

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif

	for(i=0;i<256;i++)
	{
		H[i]=0;
	}
	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	switch(imgsrc1.fmtImg)
	{
	case FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&imgbgr1,FORMAT_BGR,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(FmttransYUV2BGR(&imgsrc1,&imgbgr1));
		GO(ImgFmtTrans(&imgbgr1,&imggray1));
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(ImgFmtTrans(&imgsrc1,&imggray1));
		break;
	default:
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		ImgFmtTrans(&imgsrc1,&imggray1);
	}
	switch(imgsrc2.fmtImg)
	{
	case FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&imgbgr2,FORMAT_BGR,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(FmttransYUV2BGR(&imgsrc2,&imgbgr2));
		GO(ImgFmtTrans(&imgbgr2,&imggray2));
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(ImgFmtTrans(&imgsrc2,&imggray2));

		break;
	default:
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		ImgFmtTrans(&imgsrc2,&imggray2);
	}

	/// \ judge the frames difference，计算两帧图像的帧差
	GO(LiFrmDif(&imggray1,&imggray2,&diff));
	/// \judge for the gradient，如果两帧图像的差别不大，则根据图像的边缘信息区分是信号丢失还是画面冻结
	psrc=(MUInt8*)imggray1.pixelArray.chunky.pPixel;
	GO(ImgCreate(hMemMgr,&imgedge,FORMAT_GRAY,imggray1.dwWidth,imggray1.dwHeight));
	pedge=(MUInt8*)imgedge.pixelArray.chunky.pPixel;
	ext=imggray1.pixelArray.chunky.dwImgLine-imggray1.dwWidth;
	//printf("frozen_diff =%f\n",diff);

	if(diff>0.92) //阈值根据实验测得，代表两帧图像的相似度
	{
		///根据直方图的值计算
		for(i=0;i<imggray1.dwHeight;i++,psrc+=ext)
			for(j=0;j<imggray1.dwWidth;j++,psrc++)
			{
				tmp=*psrc;
				H[tmp]=H[tmp]+1;
			}
			for(i=0;i<256;i++)
			{
				//printf("%d",H[i]);
				if(H[i]>Hmax)
				{
					Hmax=H[i];
					//label=i;//最大频数的像素值 若为黑色 像素值小；LOST图片为其他颜色 label不能使用
				}
			}
			Hration=(MFloat)Hmax/num;

			///如果相似度大，则判别其的边缘信息
			Edge(hMemMgr,psrc,imggray1.pixelArray.chunky.dwImgLine,imggray1.dwWidth,imggray1.dwHeight,pedge,imgedge.pixelArray.chunky.dwImgLine,TYPE_CANNY);
			pedge=(MUInt8*)imgedge.pixelArray.chunky.pPixel;
			// PrintBmpEx(pedge,imgedge.pixelArray.chunky.dwImgLine,DATA_U8,imgedge.dwWidth,imgedge.dwHeight,1,"f:\\test.bmp");
			ext=imgedge.pixelArray.chunky.dwImgLine-imgedge.dwWidth;
			for(i=0;i<imgedge.dwHeight;i++,pedge+=ext)
				for(j=0;j<imgedge.dwWidth;j++,pedge++)
				{
					if(*(pedge)==0) ///EDGE=0,边缘
						n=n+1;
				}
				ratio=(MFloat)n/(imgedge.dwWidth*imgedge.dwHeight);

				if (diff>0.98&&(ratio>0.025||Hration<0.7))
				{
					ptOutParam->ImgFrozen.flag=1;///画面冻结
				}
	}
EXT:
	ImgRelease(hMemMgr,&imgbgr1);
	ImgRelease(hMemMgr,&imgbgr2);
	ImgRelease(hMemMgr,&imggray1);
	ImgRelease(hMemMgr,&imggray2);
	ImgRelease(hMemMgr,&imgedge);
	return res;
}
//MRESULT HYIQ_HorizontalProjection(IplImage *pSubBwImg,MLong *hpData)
//{
//	MLong res=LI_ERR_NONE;
//	//JOFFSCREEN bwImg_joff=_TransToInteriorImgFmt(pBwImage);
//	MLong i,j;
//	MLong tmp=0;
//	MLong sumRow=0;
//	MLong maxValue=0;
//	uchar* pBwImg=(uchar*)pSubBwImg->imageData;
//
//	for (i = 0; i <pSubBwImg->height; i++)
//	{
//		sumRow=0;
//		for (j = 0; j< pSubBwImg->width; j++)
//		{
//			tmp=pBwImg[i*pSubBwImg->widthStep +j*pSubBwImg->nChannels];
//			sumRow= sumRow+tmp;
//			if (tmp>maxValue)
//			{
//				maxValue=tmp;
//			}
//		}
//		hpData[i]=sumRow;
//	}
//
//	if (maxValue<1)
//	{
//		return LI_ERR_UNKNOWN;
//	}
//	for (i=0;i<pSubBwImg->height;i++)
//	{
//		hpData[i]=hpData[i]/maxValue;
//		//printf("%d ",hpData[i]);
//	}
//
//	return res;
//}
//
//MRESULT HYIQ_VerticalProjection(IplImage *pSubBwImg,MLong *vpData)
//{
//	MLong res=LI_ERR_NONE;
//	//JOFFSCREEN bwImg_joff=_TransToInteriorImgFmt(pBwImage);
//	MLong i,j;
//	MLong tmp=0;
//	MLong sumCol=0;
//	MLong maxValue=0;
//	uchar* pBwImg=(uchar*)pSubBwImg->imageData;
//
//	for(j = 0; j< pSubBwImg->width; j++)
//	{
//		sumCol=0;
//		for (i = 0; i <pSubBwImg->height; i++)
//		{
//			tmp=pBwImg[i*pSubBwImg->widthStep +j*pSubBwImg->nChannels];
//			sumCol= sumCol+tmp;
//			if (tmp>maxValue)
//			{
//				maxValue=tmp;
//			}
//		}
//		vpData[i]=sumCol;
//	}
//
//	if (maxValue<1)
//	{
//		return LI_ERR_UNKNOWN;
//	}
//
//	for (i=0;i<pSubBwImg->height;i++)
//	{
//		vpData[i]=vpData[i]/maxValue;
//		//printf("%d ",vpData[i]);
//	}
//
//	return res;
//
//}
//
//MRESULT HYIQ_FindUpBottomBound(MLong *hpData,MLong *pUpBound,MLong *pBottomBound,MLong nBeginY,MLong nImgHeight,double dThreshold)
//{
//	MLong res=LI_ERR_NONE;
//	int nSum5=0;
//	int i;
//	uchar *pLabels=(uchar*)malloc(nImgHeight*sizeof(uchar));
//	uchar *pTemp=pLabels;
//	for (i=0;i<nImgHeight;i++)
//	{
//		if (hpData[i]<=dThreshold)
//		{
//			*pTemp=0;
//		}
//		else
//		{
//			*pTemp=1;
//		}
//		pTemp++;
//	}
//
//	for (i=nBeginY;i>0;i--)
//	{
//		if (i>=2&&i<=nImgHeight-3)
//		{
//			nSum5=pLabels[i-2]+pLabels[i-1]+pLabels[i]+pLabels[i+1]+pLabels[i+2];
//			if (nSum5<=2)
//			{
//				*pUpBound=i;
//				break;
//			}
//		}
//	}  
//
//	for (i=nBeginY;i<nImgHeight;i++)
//	{
//		if (i>=2&&i<=nImgHeight-3)
//		{
//			nSum5=pLabels[i-2]+pLabels[i-1]+pLabels[i]+pLabels[i+1]+pLabels[i+2];
//			if (nSum5<=2)
//			{
//				*pBottomBound=i;
//				break;
//			}
//		}
//	}
//	free(pLabels);
//	pLabels=NULL;
//	return res;
//}
//MRESULT HYIQ_FindLeftRightBound(MLong *hpData,CvPoint2D32f *pCorners,MLong *left,MLong *right,MLong centerX,MLong nImgHeight,double nThreshold)
//{
//	MLong res=LI_ERR_NONE;
//	return res;
//}
//
//MRESULT HYIQ_LostBaseHaaris(pHYIQ_haarisInfo pHaarisInfo,char * pGrayImage,MLong nImgWidth,MLong nImgHeight)
//{
//	MLong res=LI_ERR_NONE;
//	CvSize subImgSize=cvSize(nImgWidth*0.6,nImgHeight*0.6);
//	CvRect rect=cvRect(nImgWidth*0.2,nImgHeight*0.2,nImgWidth*0.6,nImgHeight*0.6);
//	IplImage *pGrayImg=cvCreateImage(cvSize(nImgWidth,nImgHeight),IPL_DEPTH_8U,1);
//	IplImage *pSubGrayImg=cvCreateImage(subImgSize,IPL_DEPTH_8U,1);//cut周围一圈所剩的子区域
//	IplImage *pTempImg1=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);
//	IplImage *pTempImg2=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);
//
//	IplImage *pGrad_x=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
//	IplImage *pAbsGrad_x=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
//	IplImage *pGrad_y=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
//	IplImage *pAbsGrad_y=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
//	IplImage *pGrad=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);
//	IplImage *pSubBwImg=cvCreateImage(subImgSize,IPL_DEPTH_8U,1);//cut周围一圈所剩的子区域的二值图
//	IplImage *pSsBwImg;//字符区域二值图
//	IplImage *pTempBw;
//	CvPoint2D32f *pCorners=(CvPoint2D32f *)malloc(1000*sizeof(CvPoint2D32f));
//	CvPoint2D32f centerPoint=cvPoint2D32f(0.0,0.0);
//	int i=0;
//	int j=0;
//	MLong cornersNum=0;
//
//	CvSeq *pContour = NULL;   
//	CvSeq *pConInner = NULL;   
//	CvMemStorage *pStorage = NULL;   
//
//	//字符区域边界
//	int nUpBound=0;
//	int nBottomBound=0;
//	int nLeft=100000;
//	int nRight=0;
//	double dThreshold=0.0;
//	int nHaarisNum=0;
//	CvRect charRect;//
//	double char_width_height_ratio=0.0;//字符宽高比
//	int nCharNum=0;//字符数目
//	int pCharCentersX[1000];//字符的中心X坐标
//	int nTemp=0;//排序中间值
//	double disTemp=0.0;
//	double dis_thresh=0.0;
//	//IplConvKernel* t2 = cvCreateStructuringElementEx(3, 3, 1,1,CV_SHAPE_ELLIPSE,0);
//
//	MLong *hpData=(MLong*)malloc(subImgSize.height*sizeof(MLong));
//	memset(pCharCentersX,0,1000);
//
//	pGrayImg->imageData= pGrayImage;
//	//从图像中提取子图像
//	cvSetImageROI(pGrayImg, rect);
//	cvCopy(pGrayImg, pSubGrayImg,NULL);
//	cvResetImageROI(pGrayImg);
//	if (NULL!=pGrayImg)
//	{
//		cvReleaseImage(&pGrayImg);
//		pGrayImage=NULL;
//	}
//
//	cvGoodFeaturesToTrack (pSubGrayImg, pTempImg1,pTempImg2, pCorners,&cornersNum, 0.1, 10,0,3,0,0.04); 
//	if (NULL!=pTempImg1)
//	{
//		cvReleaseImage(&pTempImg1);
//		pTempImg1=NULL;
//	}
//	if (NULL!=pTempImg2)
//	{
//		cvReleaseImage(&pTempImg2);
//		pTempImg2=NULL;
//	}
//	for (i=0;i<cornersNum;i++)
//	{
//		centerPoint.x=centerPoint.x+pCorners[i].x;
//		centerPoint.y=centerPoint.y+pCorners[i].y;
//		if (pCorners[i].x>nRight)
//		{
//			nRight=pCorners[i].x;
//		}
//		if (pCorners[i].x<nLeft)
//		{
//			nLeft=pCorners[i].x;
//		}
//	}
//	centerPoint.x=centerPoint.x/cornersNum;
//	centerPoint.y=centerPoint.y/cornersNum;
//	//中心点初始化
//	disTemp=sqrt(pow(pSubGrayImg->width/2-centerPoint.x,2.0)+pow(pSubGrayImg->height/2-centerPoint.y,2.0));
//	dis_thresh=sqrt(pow(nImgHeight,2.0)+pow(nImgWidth,2.0))/9;
//	if (disTemp>=dis_thresh)
//	{
//		centerPoint.x=pSubGrayImg->width/2;
//		centerPoint.y=pSubGrayImg->height/2;
//	}
//
//	//cvCircle(pSubGrayImg,cvPoint((int)centerPoint.x,(int)centerPoint.y),3,cvScalar(255,255,255,0),2,8,0);
//	//cvCircle(pSubGrayImg,cvPoint(pSubGrayImg->width/2,pSubGrayImg->height/2),3,cvScalar(255,255,255,0),2,8,0);
//	//printf("%d  %d\n",nImgHeight,nImgHeight);
//	//printf("%f\n",disTemp);
//	//cvShowImage("center",pSubGrayImg);
//	//cvWaitKey(0);
//
//	//梯度检测 
//	cvSobel(pSubGrayImg,pGrad_x,1,0,3);
//	cvAbs(pGrad_x,pAbsGrad_x);
//	cvSobel(pSubGrayImg,pGrad_y,0,1,3);
//	cvAbs(pGrad_y,pAbsGrad_y);
//	cvAddWeighted(pAbsGrad_x, 0.5,pAbsGrad_y, 0.5, 0,pGrad);
//	cvThreshold(pGrad,pSubBwImg,128,255,CV_THRESH_BINARY);
//	//释放梯度数据内存
//	if (NULL!=pSubGrayImg)
//	{
//		cvReleaseImage(&pSubGrayImg);
//		pSubGrayImg=NULL;
//	}
//	if (NULL!=pGrad_x)
//	{
//		cvReleaseImage(&pGrad_x);
//		pGrad_x=NULL;
//	}
//	if (NULL!=pGrad_y)
//	{
//		cvReleaseImage(&pGrad_y);
//		pGrad_y=NULL;
//	}
//	if (NULL!=pAbsGrad_x)
//	{
//		cvReleaseImage(&pAbsGrad_x);
//		pAbsGrad_x=NULL;
//	}
//	if (NULL!=pAbsGrad_y)
//	{
//		cvReleaseImage(&pAbsGrad_y);
//		pAbsGrad_y=NULL;
//	}
//	if (NULL!=pGrad)
//	{
//		cvReleaseImage(&pGrad);
//		pGrad=NULL;
//	}
//
//	//cvMorphologyEx(pSubBwImg,pSubBwImg,NULL,t2,CV_MOP_CLOSE,1);
//	//cvShowImage("bw_grad",pSubBwImg);
//	//cvWaitKey(0);
//
//	// 查找所有轮廓  并填充
//	pStorage = cvCreateMemStorage(0); 
//	//pTempBw=cvCreateImage(cvSize(pSubBwImg->width,pSubBwImg->height),IPL_DEPTH_8U,1);
//	pTempBw=cvCloneImage(pSubBwImg);
//	cvFindContours(pTempBw, pStorage, &pContour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));    
//	//cvDrawContours(pSubBwImg, pContour, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 2, CV_FILLED, 8, cvPoint(0, 0));  
//	// 外轮廓循环   
//	for (; pContour != NULL; pContour = pContour->h_next)   
//	{   
//		// 内轮廓循环   
//		for (pConInner = pContour->v_next; pConInner != NULL; pConInner = pConInner->h_next)   
//		{   
//			//// 内轮廓面积   
//			//dConArea = fabs(cvContourArea(pConInner, CV_WHOLE_SEQ));   
//			//if (dConArea <= dAreaThre)   
//			//{   
//			cvDrawContours(pSubBwImg, pConInner, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 0, CV_FILLED, 8, cvPoint(0, 0));   
//			//}
//		}   
//	}   
//	if (NULL!=pTempBw)
//	{
//		cvReleaseImage(&pTempBw);
//		pTempBw=NULL;
//	}
//	pConInner=NULL;
//	//cvShowImage("contours",pSubBwImg);
//	//cvWaitKey(0);
//
//	res=HYIQ_HorizontalProjection(pSubBwImg,hpData);
//	if (res!=LI_ERR_NONE)
//	{
//		//释放内存
//		free(hpData);
//		hpData=NULL;
//		free(pCorners);
//		pCorners=NULL;
//		cvReleaseMemStorage(&pStorage);   
//		pStorage = NULL;   
//		if (NULL!=pSubBwImg)
//		{
//			cvReleaseImage(&pSubBwImg);
//			pSubBwImg=NULL;
//		}
//		return res;
//	}
//	dThreshold=pSubBwImg->width*0.3;
//	res=HYIQ_FindUpBottomBound(hpData,&nUpBound,&nBottomBound,centerPoint.y,pSubBwImg->height,dThreshold);
//	if (res!=LI_ERR_NONE)
//	{
//		//释放内存
//		free(hpData);
//		hpData=NULL;
//		free(pCorners);
//		pCorners=NULL;
//		cvReleaseMemStorage(&pStorage);   
//		pStorage = NULL;   
//		if (NULL!=pSubBwImg)
//		{
//			cvReleaseImage(&pSubBwImg);
//			pSubBwImg=NULL;
//		}
//		return res;
//	}
//
//	//字符区域二值子图
//	rect=cvRect(nLeft,nUpBound,nRight-nLeft+1,nBottomBound-nUpBound+1);
//	pSsBwImg=cvCreateImage(cvSize(nRight-nLeft+1,nBottomBound-nUpBound+1),IPL_DEPTH_8U,1);
//	cvSetImageROI(pSubBwImg, rect);
//	cvCopy(pSubBwImg, pSsBwImg,NULL);
//	cvResetImageROI(pSubBwImg);
//	if (NULL!=pSubBwImg)
//	{
//		cvReleaseImage(&pSubBwImg);
//		pSubBwImg=NULL;
//	}
//
//	//找字符轮廓
//	cvClearMemStorage(pStorage);
//	pContour=NULL;
//	//pTempBw=cvCreateImage(cvSize(pSsBwImg->width,pSsBwImg->height),IPL_DEPTH_8U,1);
//	pTempBw=cvCloneImage(pSsBwImg);
//	cvFindContours(pTempBw, pStorage, &pContour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));
//	cvSet(pTempBw,cvScalar(0,0,0,0),NULL);
//	for (;pContour!=NULL;pContour=pContour->h_next)
//	{
//		charRect=cvBoundingRect(pContour,1);
//		//cvRectangle(pSsBwImg,cvPoint(charRect.x,charRect.y),cvPoint(charRect.x+charRect.width,charRect.y+charRect.height),cvScalar(128,128,128,0),1,8,0);
//		//width_height_ratio=rect.width/rect.height;
//		//if (width_height_ratio>=0.5||width_height_ratio<=2)
//		//{
//		//	
//		//}
//		for (i=charRect.y;i<charRect.y+charRect.height;i++)
//		{
//			for (j=charRect.x;j<charRect.x+charRect.width;j++)
//			{
//				*(pTempBw->imageData+i*pTempBw->widthStep +j*pTempBw->nChannels)=255;
//			}
//		}
//	}
//	cvClearMemStorage(pStorage);
//	pContour=NULL;
//	cvFindContours(pTempBw, pStorage, &pContour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));
//	nLeft=10000;
//	nRight=0;
//	for (;pContour!=NULL;pContour=pContour->h_next)
//	{
//		charRect=cvBoundingRect(pContour,1);
//		char_width_height_ratio=(double)charRect.width/charRect.height;
//		//cvRectangle(pSsBwImg,cvPoint(charRect.x,charRect.y),cvPoint(charRect.x+charRect.width,charRect.y+charRect.height),cvScalar(128,128,128,0),1,8,0);
//		//cvShowImage("bw",pSsBwImg);
//		//cvWaitKey(0);
//		if (charRect.y+abs(charRect.y+charRect.height-pSsBwImg->height)<6&&char_width_height_ratio>=0.8
//			&&char_width_height_ratio<=2.5)
//		{
//			if (charRect.x<nLeft)
//			{
//				nLeft=charRect.x;
//			}
//			if (charRect.x+charRect.width>nRight)
//			{
//				nRight=charRect.x+charRect.width;
//			}
//			pCharCentersX[nCharNum]=charRect.x+charRect.width/2;
//			nCharNum++;
//			//cvRectangle(pSsBwImg,cvPoint(charRect.x,charRect.y),cvPoint(charRect.x+charRect.width,charRect.y+charRect.height),cvScalar(128,128,128,0),1,8,0);
//		}	
//	}
//
//	//字符中心从小到大排序
//	for (i=0;i<nCharNum-1;i++)
//	{
//		for (j=i+1;j<nCharNum;j++)
//		{
//			if (pCharCentersX[i]>pCharCentersX[j])
//			{
//				nTemp=pCharCentersX[i];
//				pCharCentersX[i]=pCharCentersX[j];
//				pCharCentersX[j]=nTemp;
//			}
//		}
//	}
//	//三个字符中心宽度是否满足条件
//	if (nCharNum>=3)
//	{
//		for(i=0;i<nCharNum;i++)
//		{
//			nTemp=pCharCentersX[i+2]-pCharCentersX[i];
//			if (nTemp<(0.4*nImgWidth)&&nTemp>(0.16*nImgWidth))
//			{
//				pHaarisInfo->n3CenterX=1;
//				break;
//			}
//		}
//	}
//
//	//cvShowImage("char",pSsBwImg);
//	//cvWaitKey(0);
//	////  
//	//cvLine(pSubGrayImg,cvPoint(0,nUpBound),cvPoint(nImgWidth-1,nUpBound),cvScalar(255,255,255,0),2,8,0);
//	//cvLine(pSubGrayImg,cvPoint(0,nBottomBound),cvPoint(nImgWidth-1,nBottomBound),cvScalar(255,255,255,0),2,8,0);
//	//cvShowImage("gray",pSubGrayImg);
//	//cvWaitKey(0);
//	for (i=0;i<cornersNum;i++)
//	{
//		if (pCorners[i].y>=nUpBound-1&&pCorners[i].y<=nBottomBound+1)
//		{
//			nHaarisNum++;
//		}
//	}
//	pHaarisInfo->nHaarisNum=nHaarisNum;
//	pHaarisInfo->charRegionHeight=pSsBwImg->height;
//	pHaarisInfo->charRegionWidth=nRight-nLeft+1;
//	pHaarisInfo->nCharNum=nCharNum;
//	//printf("\nh=%d num=%ld\n",nBottomBound-nUpBound,*haarisNum);
//
//	//释放内存
//	free(hpData);
//	hpData=NULL;
//	free(pCorners);
//	pCorners=NULL;
//	cvReleaseMemStorage(&pStorage);   
//	pStorage = NULL;   
//	if (NULL!=pSsBwImg)
//	{
//		cvReleaseImage(&pSsBwImg);
//		pSsBwImg=NULL;
//	}
//	if (NULL!=pTempBw)
//	{
//		cvReleaseImage(&pTempBw);
//		pTempBw=NULL;
//	}
//	return res;
//}
//
//IQ_API MRESULT  HYIQ_FindPeak(const MLong H[256], MLong *pPeakLocation,MLong *pValleyLocation)  
//{  
//	MLong res=LI_ERR_NONE;
//
//	MLong diff_v[255];
//	int i=0;
//	int ipeak=0;
//	int ivalley=0;
//	// 计算V的一阶差分和符号函数trend  
//	for ( i = 0; i<255; i++)  
//	{  
//		if (H[i + 1] - H[i]>0)  
//			diff_v[i] = 1;  
//		else if (H[i + 1] - H[i] < 0)  
//			diff_v[i] = -1;  
//		else  
//			diff_v[i] = 0;  
//	}  
//	// 对Trend作了一个遍历  
//	for ( i = 254; i >= 0; i--)  
//	{  
//		if (diff_v[i] == 0 && i ==254)  
//		{  
//			diff_v[i] = 1;  
//		}  
//		else if (diff_v[i] == 0)  
//		{  
//			if (diff_v[i + 1] >= 0)  
//				diff_v[i] = 1;  
//			else  
//				diff_v[i] = -1;  
//		}  
//	}  
//
//	for ( i = 0; i<254; i++)  
//	{  
//		if (diff_v[i + 1] - diff_v[i] == -2)  
//		{
//			*(pPeakLocation+ipeak)=i ;  
//			ipeak++;
//		}
//		else if (diff_v[i + 1] - diff_v[i] == 2)  
//		{
//			*(pValleyLocation+ivalley)=i;  
//			ivalley++;
//		}
//	}  
//
//	return res;
//}  
//
//IQ_API MRESULT HYIQ_FindTwoPeak(const MLong H[256], MLong *pPeakLocation,MLong *pSum11)
//{
//	MLong res=LI_ERR_NONE;
//	MLong H1[246];
//	MLong sum11=0;
//	int i,k;
//	int nLabel=0;
//	int nLabel_max=0;
//	int nMax=0;
//
//	for (i=5;i<251;i++)
//	{
//		sum11=0;
//		for (k=i-5;k<=i+5;k++)
//		{
//			sum11=sum11+H[k];
//		}
//		H1[i-5]=sum11;
//	}
//
//	for (i=0;i<246;i++)
//	{
//		if (H1[i]>nMax)
//		{
//			nMax=H1[i];
//			nLabel=i;
//		}
//	}
//	*pPeakLocation=nLabel;
//	*pSum11=nMax;
//	nLabel_max=nLabel;
//
//	nMax=0;
//	for (i=0;i<246;i++)
//	{
//		if (H1[i]>nMax&&abs(i-nLabel_max)>=10)
//		{
//			nMax=H1[i];
//			nLabel=i;
//		}
//	}
//	*(pPeakLocation+1)=nLabel;
//	*(pSum11+1)=nMax;
//
//	return res;
//}

IQ_API MRESULT HYIQ_LostBaseHist(HYIQ_PTOutParam ptOutParam,char *pGrayImage,MLong nImgWidth,MLong nImgHeight)
{
	MLong res=LI_ERR_NONE;
	//对图像归一化到cif 并只去中间区域进行检测
	CvSize subImgSize=cvSize(176,144);
	CvRect rect=cvRect(88,72,176,144);
	//申请图像内存
	IplImage *pGrayImg=cvCreateImage(cvSize(nImgWidth,nImgHeight),IPL_DEPTH_8U,1);
	IplImage *pGrayImg_Cif=cvCreateImage(cvSize(352,288),IPL_DEPTH_8U,1);
	IplImage *pSubGrayImg=cvCreateImage(subImgSize,IPL_DEPTH_8U,1);//cut周围一圈所剩的子区域
	IplImage *pTempImg1=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);
	IplImage *pTempImg2=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);

	IplImage *pGrad_x=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
	IplImage *pAbsGrad_x=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
	IplImage *pGrad_y=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
	IplImage *pAbsGrad_y=cvCreateImage(subImgSize,IPL_DEPTH_16S,1);
	IplImage *pGrad=cvCreateImage(subImgSize,IPL_DEPTH_32F,1);
	IplImage *pSubBwImg=cvCreateImage(subImgSize,IPL_DEPTH_8U,1);

	//show
#ifdef _ZMH_DEBUG
	MLong maxH=0;
	double val=0.0;
	int H[256];
	IplImage *pHistimg = cvCreateImage( cvSize(320,300), 8, 3 );
	IplImage *pShowGrayImg=cvCreateImage(subImgSize,IPL_DEPTH_8U,1);//cut周围一圈所剩的子区域
#endif

	CvPoint2D32f *pCorners=(CvPoint2D32f *)malloc(1000*sizeof(CvPoint2D32f));
	CvPoint2D32f centerPoint=cvPoint2D32f(0.0,0.0);
	int nCornersNum=0;
	MLong nCornersNum_char=0;
	double disTemp=0.0;
	double dis_thresh=0.0;

	int i=0;
	int j=0;
	int k=0;
	int m=0;
	int nBeignX_const=0;
	int nBeginY_const=0;
	int nBeignX=0;
	int nBeginY=0;
	int nEndX=0;
	int nEndY=0;
	MLong tmp=0;
	int nRectNum=0;
	int nRectIsLostNum=0;

	//gmm
	VlGMM * gmm= vl_gmm_new (VL_F_TYPE, 1,2) ;
	float *means;
	float *sigmas;
	float * priors ;
	float fTemp=0.0;
	TYPE * data = (TYPE *)vl_malloc(sizeof(TYPE)*5000);
	TYPE initMeans[2]={70.0,170.0};
	int dataNum=0;

	float fGradSum=0.0;

#ifdef _ZMH_DEBUG
	for(i=0;i<256;i++)
	{
		H[i]=0;
	}
#endif

	//图像归一化到cif for 实时
	pGrayImg->imageData= pGrayImage;
	cvResize(pGrayImg,pGrayImg_Cif,CV_INTER_LINEAR);
	if (NULL!=pGrayImg)
	{
		cvReleaseImage(&pGrayImg);
		pGrayImg=NULL;
	}

	cvSetImageROI(pGrayImg_Cif, rect);
	cvCopy(pGrayImg_Cif, pSubGrayImg,NULL);
	cvResetImageROI(pGrayImg_Cif);
	if (NULL!=pGrayImg_Cif)
	{
		cvReleaseImage(&pGrayImg_Cif);
		pGrayImg=NULL;
	}

#ifdef _ZMH_DEBUG
	cvCopy(pSubGrayImg,pShowGrayImg,NULL);
#endif
	//找字符区域中心点
	cvGoodFeaturesToTrack (pSubGrayImg, pTempImg1,pTempImg2, pCorners,&nCornersNum, 0.1, 10,0,3,0,0.04); 
	if (NULL!=pTempImg1)
	{
		cvReleaseImage(&pTempImg1);
		pTempImg1=NULL;
	}
	if (NULL!=pTempImg2)
	{
		cvReleaseImage(&pTempImg2);
		pTempImg2=NULL;
	}
	for (i=0;i<nCornersNum;i++)
	{
		centerPoint.x=centerPoint.x+pCorners[i].x;
		centerPoint.y=centerPoint.y+pCorners[i].y;
#ifdef _ZMH_DEBUG
		cvCircle(pShowGrayImg,cvPoint((int)pCorners[i].x,(int)pCorners[i].y),2,cvScalar(255,255,255,255),2,8,0);
#endif
	}
	centerPoint.x=centerPoint.x/nCornersNum;
	centerPoint.y=centerPoint.y/nCornersNum;

	//中心点初始化
	disTemp=sqrt(pow((float)(pSubGrayImg->width/2-centerPoint.x),(float)2.0)+pow((float)pSubGrayImg->height/2-centerPoint.y,(float)2.0));
	dis_thresh=sqrt(pow(288.0,2.0)+pow(352.0,2.0))/9;
	if (disTemp>=dis_thresh)
	{
		centerPoint.x=pSubGrayImg->width/2;
		centerPoint.y=pSubGrayImg->height/2;
	}

	//梯度检测 
	cvSobel(pSubGrayImg,pGrad_x,1,0,3);
	cvAbs(pGrad_x,pAbsGrad_x);
	cvSobel(pSubGrayImg,pGrad_y,0,1,3);
	cvAbs(pGrad_y,pAbsGrad_y);
	cvAddWeighted(pAbsGrad_x, 0.5,pAbsGrad_y, 0.5, 0,pGrad);
	cvThreshold(pGrad,pSubBwImg,128,255,CV_THRESH_BINARY);
	//释放梯度数据内存
	if (NULL!=pGrad_x)
	{
		cvReleaseImage(&pGrad_x);
		pGrad_x=NULL;
	}
	if (NULL!=pGrad_y)
	{
		cvReleaseImage(&pGrad_y);
		pGrad_y=NULL;
	}
	if (NULL!=pAbsGrad_x)
	{
		cvReleaseImage(&pAbsGrad_x);
		pAbsGrad_x=NULL;
	}
	if (NULL!=pAbsGrad_y)
	{
		cvReleaseImage(&pAbsGrad_y);
		pAbsGrad_y=NULL;
	}
	if (NULL!=pGrad)
	{
		cvReleaseImage(&pGrad);
		pGrad=NULL;
	}

	//归一化之后字符大小170x40  xstep为10 ystep为5 x方向遍历中间5次 y方向遍历3次  遍历矩形大小110x20
	if (centerPoint.x-75>=0)
	{
		nBeignX_const=centerPoint.x-75;
	}
	if (centerPoint.y-15>=0)
	{
		nBeginY_const=centerPoint.y-15;
	}

	nBeginY=nBeginY_const;
	for (i=0;i<3;i++)
	{
		nEndY=nBeginY+20;
		if (nEndY>=pSubGrayImg->height)
		{
			break;
		}
		nBeignX=nBeignX_const;
		for (j=0;j<5;j++)
		{
			nEndX=nBeignX+110;//此处nBeignX变为最大值了 ？？？？？？？？
			if (nEndX>=pSubGrayImg->width)
			{
				break;
			}
			dataNum=0;
			memset(data,0,5000);
			fGradSum=0.0;
			//rect process
			for(k=nBeginY;k<nEndY;k++)  
			{
				for(m=nBeignX;m<nEndX;m++)  
				{
					tmp=((uchar *)(pSubGrayImg->imageData + k*pSubGrayImg->widthStep))[m];  
					data[dataNum]=tmp;
					dataNum++;
#ifdef _ZMH_DEBUG
					H[tmp]=H[tmp]+1;
#endif
					fGradSum=fGradSum+((uchar *)(pSubBwImg->imageData + k*pSubBwImg->widthStep))[m];
				}
			}
			fGradSum=fGradSum/255;
			//printf("grad=%f\n",fGradSum);
			nRectNum++;

			vl_gmm_set_means(gmm,initMeans);
			vl_gmm_set_max_num_iterations (gmm, 5) ;
			vl_gmm_cluster (gmm, data, dataNum);

			// get the means, covariances, and priors of the GMM
			means = (float*)vl_gmm_get_means(gmm);
			sigmas = (float*)vl_gmm_get_covariances(gmm);
			priors = (float*)vl_gmm_get_priors(gmm);

			//按照means从小到大排序
			if(means[0]>means[1])
			{
				fTemp=means[0];
				means[0]=means[1];
				means[1]=fTemp;

				fTemp=sigmas[0];
				sigmas[0]=sigmas[1];
				sigmas[1]=fTemp;

				fTemp=priors[0];
				priors[0]=priors[1];
				priors[1]=fTemp;
			}

			//for (k=0;k<2;k++)
			//{
			//	printf("%f ",means[k]);
			//	printf("%f ",sigmas[k]);
			//	printf("%f\n",priors[k]);
			//}
			//printf("-----------------------------------------------\n");

			//判断子区域是否为Lost
			if(means[1]-means[0]>70.0&&sigmas[1]<1500&&sigmas[0]<500&&priors[0]>0.3&&priors[1]>0.3&&fGradSum>770)
			{
				nRectIsLostNum++;
			}
#ifdef _ZMH_DEBUG
			//绘画处理的文字区域
			cvLine(pShowGrayImg,cvPoint(nBeignX,nBeginY),cvPoint(nBeignX,nEndY),cvScalar(255,255,255,255),1,8,0);
			cvLine(pShowGrayImg,cvPoint(nBeignX,nBeginY),cvPoint(nEndX,nBeginY),cvScalar(255,255,255,255),1,8,0);
			cvLine(pShowGrayImg,cvPoint(nEndX,nBeginY),cvPoint(nEndX,nEndY),cvScalar(255,255,255,255),1,8,0);
			cvLine(pShowGrayImg,cvPoint(nEndX,nEndY),cvPoint(nBeignX,nEndY),cvScalar(255,255,255,255),1,8,0);
			cvCircle(pShowGrayImg,cvPoint((int)centerPoint.x,(int)centerPoint.y),2,cvScalar(255,255,255,255),2,8,0);
			cvShowImage("subGray",pShowGrayImg);
			cvShowImage("bw",pSubBwImg);
			cvWaitKey(2);

			// 画直方图
			for( k= 0; k < 256;k++ )
			{
				if (H[k]>maxH)
				{
					maxH=H[k];
				}
			}
			cvSet(pHistimg,cvScalar(0,0,0,0),NULL);

			for( k= 0; k < 256;k++ )
			{
				val =pHistimg->height - H[k];
				if (val<0)
				{
					val=0.0;
				}
				cvRectangle(pHistimg, cvPoint(k*pHistimg->width/256,pHistimg->height),cvPoint((k+1)*pHistimg->width/256,(int)(val)),CV_RGB(255,255,0,0), 1, 8, 0 );
			}
			cvShowImage("Hist",pHistimg);
			cvWaitKey(2);
			//直方图内存置0
			for(k=0;k<256;k++)
			{
				H[k]=0;
			}
#endif
			nBeignX=nBeignX+10;
		}
		nBeginY=nBeginY+5;
	}

	for(k=0;k<nCornersNum;k++)
	{
		if (pCorners[k].x>nBeignX_const-5&&pCorners[k].x<nBeignX_const+155&&pCorners[k].y>nBeginY_const-5&&pCorners[k].y<nBeginY_const+35)
		{
			nCornersNum_char++;
		}
	}

	//printf("%f\n",(float)cornersNum_char);
	if ((double)nRectIsLostNum/nRectNum>0.7&&nCornersNum_char>27)
	{
		ptOutParam->SignalLost.flag=1; ///信号丢失
	}

	vl_gmm_delete(gmm);
	vl_free(data);
	free(pCorners);
	pCorners=NULL;

	if (NULL!=pSubGrayImg)
	{
		cvReleaseImage(&pSubGrayImg);
		pGrad=NULL;
	}
	if (NULL!=pSubBwImg)
	{
		cvReleaseImage(&pSubBwImg);
		pGrad=NULL;
	}

	return res;
}

///  根据帧差判别信号丢失还是画面冻结
MRESULT HYIQ_LOST(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam)
{
	MLong res=LI_ERR_NONE;
	MHandle hMemMgr = MNull;
	IMQU *pImqu = (IMQU *)hHandle;
	JOFFSCREEN imgsrc1=_TransToInteriorImgFmt(pImagesrc1);
	JOFFSCREEN imgsrc2=_TransToInteriorImgFmt(pImagesrc2);
	JOFFSCREEN imgbgr1={0};
	JOFFSCREEN imgbgr2={0};
	JOFFSCREEN imggray1={0};
	JOFFSCREEN imggray2={0};
	MUInt8* psrc=MNull;
	MFloat diff=0.0;
	MLong n=0;
	MLong i,j;
	MLong ext=0;
	MLong H[256];
	MLong num=imgsrc1.dwHeight*imgsrc1.dwWidth;
	MLong tmp=0;
	MLong Hmax=0;
	MFloat Hration=0.0;
	MLong label=0; //最大频数的像素值 若为黑色 像素值小；LOST图片为其他颜色 label不能使用
	MLong grayNum=0; //灰度丰富度 视频Lost灰度丰富度小 区别视频冻结
	MLong grayFreThreshold=pImagesrc1->lHeight*pImagesrc1->lWidth*0.001; //去除噪声
	//double gmm_t;

//#ifdef _WIN32
//	ProcessMetrics processMetrics;
//	int processCPUUsage;
//	//int systemCPUUage;
//	int processMemoryage;
//#endif

#ifdef HY_CHECK_IS_OUTTIME
	if (HYIQ_CheckOutTime()==1)//超时
	{
		goto EXT;
	}
#endif

//#ifdef _WIN32 //win32系统 
//	processCPUUsage=processMetrics.GetCPUUsage();
//	//systemCPUUage=processMetrics.GetSystemCPUUsage();
//	processMemoryage=processMetrics.GetMemoryUsage();
//#endif

	for(i=0;i<256;i++)
	{
		H[i]=0;
	}
	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	switch(imgsrc1.fmtImg)
	{
	case FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&imgbgr1,FORMAT_BGR,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(FmttransYUV2BGR(&imgsrc1,&imgbgr1));
		GO(ImgFmtTrans(&imgbgr1,&imggray1));
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		GO(ImgFmtTrans(&imgsrc1,&imggray1));
		break;
	default:
		GO(ImgCreate(hMemMgr,&imggray1,FORMAT_GRAY,imgsrc1.dwWidth,imgsrc1.dwHeight));
		ImgFmtTrans(&imgsrc1,&imggray1);
	}
	switch(imgsrc2.fmtImg)
	{
	case FORMAT_YUV420:
		GO(ImgCreate(hMemMgr,&imgbgr2,FORMAT_BGR,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(FmttransYUV2BGR(&imgsrc2,&imgbgr2));
		GO(ImgFmtTrans(&imgbgr2,&imggray2));
		break;
	case FORMAT_RGB:
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		GO(ImgFmtTrans(&imgsrc2,&imggray2));
		break;
	default:
		GO(ImgCreate(hMemMgr,&imggray2,FORMAT_GRAY,imgsrc2.dwWidth,imgsrc2.dwHeight));
		ImgFmtTrans(&imgsrc2,&imggray2);
	}

	/// \ judge the frames difference，计算两帧图像的帧差
	GO(LiFrmDif(&imggray1,&imggray2,&diff));
	///// \judge for the gradient，如果两帧图像的差别不大，则根据图像的边缘信息区分是信号丢失还是画面冻结
	psrc=(MUInt8*)imggray1.pixelArray.chunky.pPixel;
	ext=imggray1.pixelArray.chunky.dwImgLine-imggray1.dwWidth;
	//	printf("the similarity is =%f",diff);
	if(diff>0.9) //阈值根据实验测得，代表两帧图像的相似度
	{
		///根据直方图的值计算
		for(i=0;i<imggray1.dwHeight;i++,psrc+=ext)
			for(j=0;j<imggray1.dwWidth;j++,psrc++)
			{
				tmp=*psrc;
				H[tmp]=H[tmp]+1;
			}
			for(i=0;i<256;i++)
			{
				//printf("%d",H[i]);
				if(H[i]>Hmax)
				{
					Hmax=H[i];
					label=i;//最大频数的像素值 若为黑色 像素值小；LOST图片为其他颜色 label不能使用
				}
			}
			Hration=(MFloat)Hmax/num;

			//计算灰度丰富度
			for(i=0;i<256;i++)
			{
				if(H[i]>grayFreThreshold)
				{
					grayNum++;
				}
			}

			//printf("lost_diff=%f  lost_Hration=%f  lost_grayNum=%d label=%d\n",diff,Hration,grayNum,label);
			if(Hration>0.85&&grayNum<35&&label<25)//标准情况<<黑色背景
			{
				ptOutParam->SignalLost.flag=1; ///信号丢失
			}
			else//其他
			{
				//gmm_t=(double)cvGetTickCount();
				res=HYIQ_LostBaseHist(ptOutParam,(char*)imggray2.pixelArray.chunky.pPixel,imggray2.dwWidth,imggray2.dwHeight);
				//gmm_t=(double)cvGetTickCount()-gmm_t;
				//printf("t=%f\n",gmm_t/(cvGetTickFrequency()*1000));
			}
	}

EXT:
	ImgRelease(hMemMgr,&imgbgr1);
	ImgRelease(hMemMgr,&imgbgr2);
	ImgRelease(hMemMgr,&imggray1);
	ImgRelease(hMemMgr,&imggray2);

	return res;
}

/// \brief HYIQ_Uninit  free the memory
/// \param hMemMgr Handle to be free
/// \return the error code
/// 释放句柄内存
MRESULT HYIQ_Uninit(MHandle HYIQhandle )
{
	MLong  res=0;
	PIMQU PIMQ=(PIMQU)HYIQhandle;
	MHandle hMemMgr;
	//DestroyQueue(PIMQ->pImgBufs);
	hMemMgr=PIMQ->hMemMgr;
	FreeVectMem(hMemMgr,PIMQ);
EXT:
	return res;
}


/// \brief HYIQ_MemMgrCreate  allocate the memory for the handle
/// \param pMem the pointer
/// \param the memory length
/// \return the pointer
/// \内存创建
MHandle  HYIQ_MemMgrCreate(MVoid * pMem, MLong lMemSize)
{
	return JMemMgrCreate(pMem, lMemSize);
}


/// \brief HYIQ_MemMgrDestroy  Destroy the handle
/// \param hMemMgr Handle to destroy
/// \return null
/// \内存释放
MVoid HYIQ_MemMgrDestroy(MHandle hMemMgr)
{
	JMemMgrDestroy(hMemMgr);
}
