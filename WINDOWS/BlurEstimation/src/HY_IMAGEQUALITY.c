/*!
* \file HY_IMAGEQUALITY.C
* \brief  the API function
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#include <stdio.h>
#include <stdlib.h>
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

//#define HY_CHECK_IS_OUTTIME  //是否需要检查客户使用超时
#define NPIECE_WEIGHT 4
//#define _ZMH_DEBUG

#define MAX(a,b)  (a>b)?a:b
#define MIN(a,b)  (a<b)?a:b

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

MRESULT HYIQ_CUTIMAGE(IQ_PIMAGES pImage_,IQ_PIMAGES pImage)
{
	MRESULT res=0;
	pImage_->pixelArray.chunky.pPixel=(MPChar)pImage->pixelArray.chunky.pPixel+((int)(0.2*pImage->lHeight)*pImage->pixelArray.chunky.lLineBytes);
	pImage_->lHeight=pImage->lHeight*0.6;
	pImage_->lWidth=pImage->lWidth;
	pImage_->pixelArray.chunky.lLineBytes=pImage->pixelArray.chunky.lLineBytes;
	pImage_->lPixelArrayFormat=pImage->lPixelArrayFormat;

EXT:
	return res;

}

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
	/// \output the noise level
	/// 输出噪声等级
	ptOutParam->NOISE.NOISELEVEL=NoiseL;
	//printf("HYIQ_NOISE:ptOutParam->NOISE.NOISELEVEL=%f\n", ptOutParam->NOISE.NOISELEVEL);
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
	//BLOCKEXT BlockP;
	MLong j,w,h,h1,i;
	MFloat NoiseL=0;
	MUInt8 *data2;
	MUInt8 *dataedge;
	MFloat sigma,sigma1,sigma2,sigma3;
	MLong a1,b1,a2,b2,a3,b3;
	//BLOCK imga={0};
	//BLOCK imgb={0};
	//BLOCKEXT ImgA={0};
	//BLOCKEXT ImgB={0};
	//BLOCK blockp={0};
	MFloat eps=0.0000001;
	JOFFSCREEN imggray={0};
	JOFFSCREEN imggray2={0};
	JOFFSCREEN imgbgr={0};
	JOFFSCREEN imgbgr1={0};

	//float weight[NPIECE_WEIGHT][NPIECE_WEIGHT]={0.0};
	//float sigma_piece[NPIECE_WEIGHT][NPIECE_WEIGHT]={0.0};
	//float sum_sigma_weight=0.0;
	//int xStep=0;
	//int yStep=0;
	//JRECT_EXT subGray_ext;
	//JRECT_EXT subEdge_ext;
	//int m1,m2,n1,n2;
	//MFloat fTemp=0.0;
	//MFloat sumTemp=0.0;

#ifdef _ZMH_DEBUG
	IplImage *showImg=cvCreateImage(cvSize(pImage->lWidth,pImage->lHeight*0.8),IPL_DEPTH_8U,1);
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
	///blockplain和blockimg1数据地址不变，仅变化blockplain的ext
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
	//edgemap.pBlockData=data2;
	dataedge=(MUInt8*)edgemap.pBlockData;
	///根据噪声的canny边缘检测
	GO(NoiseEdge(hMemMgr,data2,blockimg1.block.lBlockLine,(blockimg1.ext.right-blockimg1.ext.left),(blockimg1.ext.bottom-blockimg1.ext.top), dataedge,edgemap.lBlockLine,TYPE_CANNY,NoiseL));
	//blockedge切掉上下边界获得的边缘(edgemap已经裁剪了吧，这一步是转存图片吧)
	blockedge.ext.top=0;
	blockedge.ext.left=0;
	blockedge.ext.bottom=edgemap.lHeight;
	blockedge.ext.right=edgemap.lWidth;
	blockedge.block=edgemap;
	//PrintBmpEx(dataedge,blockedge.block.lBlockLine,DATA_U8,blockedge.ext.right-blockedge.ext.left,blockedge.ext.bottom-blockedge.ext.top,1,"F:\\edge1.bmp");
	///选择边缘信息强烈的1/4区域
	SeleBlock(hMemMgr,&blockimg1,&blockedge);
	//xStep=(blockimg1.ext.right-blockimg1.ext.left)/NPIECE_WEIGHT;
	//yStep=(blockimg1.ext.bottom-blockimg1.ext.top)/NPIECE_WEIGHT;
	//subGray_ext=blockimg1.ext;
	//subEdge_ext=blockedge.ext;

	////计算每个子区域的权重和sigma
	//for (i=0;i<NPIECE_WEIGHT;i++)//行->yStep
	//{
	//	for (j=0;j<NPIECE_WEIGHT;j++)//列->xStep
	//	{
	//		blockimg1.ext.left=subGray_ext.left+j*xStep;
	//		blockimg1.ext.right=blockimg1.ext.left+xStep;
	//		blockimg1.ext.top=subGray_ext.top+i*yStep;
	//		blockimg1.ext.bottom=blockimg1.ext.top+yStep;
	//		blockedge.ext.left=subEdge_ext.left+j*xStep;
	//		blockedge.ext.right=blockedge.ext.left+xStep;
	//		blockedge.ext.top=subEdge_ext.top+i*yStep;
	//		blockedge.ext.bottom=blockedge.ext.top+yStep;
	//		GO(CalImgPieceWeight(hMemMgr,&weight[i][j],&blockedge));
	//		GO(CalSigmaTwoGaussBlur(hMemMgr,&sigma1,&blockimg1,&blockedge,1,2));
	//		GO(CalSigmaTwoGaussBlur(hMemMgr,&sigma2,&blockimg1,&blockedge,5,7));
	//		GO(SigmaSel(&sigma_piece[i][j],sigma1,sigma2));
	//		sum_sigma_weight=sum_sigma_weight+weight[i][j];
	//	}
	//}

	//for (i=0;i<NPIECE_WEIGHT*NPIECE_WEIGHT-1;i++)
	//{
	//	m1=i/NPIECE_WEIGHT;
	//	n1=i%NPIECE_WEIGHT;
	//	for (j=i+1;j<NPIECE_WEIGHT*NPIECE_WEIGHT;j++)
	//	{
	//		 m2=j/NPIECE_WEIGHT;
	//		 n2=j%NPIECE_WEIGHT;
	//		 if (weight[m1][n1]<weight[m2][n2])
	//		 {
 //                fTemp=weight[m2][n2];
 //                weight[m2][n2]=weight[m1][n1];
 //                weight[m1][n1]=fTemp;
 //                fTemp=sigma_piece[m2][n2];
	//			 sigma_piece[m2][n2]=sigma_piece[m1][n1];
	//			 sigma_piece[m1][n1]=fTemp;
	//		 }      
	//	}
	//}
	//sigma=0.0;
	////权重归一化并计算最终的sigma
	//for (i=0;i<NPIECE_WEIGHT/2;i++)
	//{
	//	for (j=0;j<NPIECE_WEIGHT;j++)
	//	{
	//         //weight[i][j]=weight[i][j]/sum_sigma_weight;
	//		sumTemp+=weight[i][j];
	//		//if (sumTemp<0.8*sum_sigma_weight)
	//		//{
	//			 sigma=sigma+weight[i][j]*sigma_piece[i][j];
	//		//}
	//		 //printf("    w=%4.2f  s=%4.2f",weight[i][j],sigma_piece[i][j]);
	//	}
	//	 printf("\n");
	//}
	//sigma=sigma/sumTemp;

#ifdef _ZMH_DEBUG
	showImg->imageData=(char*)edgemap.pBlockData;
	cvShowImage("edge1",showImg);
	cvWaitKey(0);
#endif

	GO(CalSigmaTwoGaussBlur(hMemMgr,&sigma1,&blockimg1,&blockedge,1,2));
	GO(CalSigmaTwoGaussBlur(hMemMgr,&sigma2,&blockimg1,&blockedge,5,7));
	GO(SigmaSel(&sigma,sigma1,sigma2));
	//printf("sigma1=%.2f sigma2=%.2f sigma=%.2f\n",sigma1,sigma2,sigma);
	///输出结果
	ptOutParam->CLEAR.CLEARLEVEL=sigma;
	ptOutParam->NOISE.NOISELEVEL=NoiseL;
	ptOutParam->Sigma.sigma1=sigma1;
	ptOutParam->Sigma.sigma2=sigma2;
	//printf("ptOutParam->CLEAR.CLEARLEVEL=%f\n",ptOutParam->CLEAR.CLEARLEVEL);
	//printf("ptOutParam->NOISE.NOISELEVEL=%f\n", ptOutParam->NOISE.NOISELEVEL);
EXT:
	///删除内存
	B_Release(hMemMgr,&edgemap);
	//B_Release(hMemMgr,&imga);
	//B_Release(hMemMgr,&imgb);
	//B_Release(hMemMgr,&blockp);
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
			///两帧图像之间的亮度差别，用于后续判断
			temp=fabs((k-k1)/k);
			ptOutParam->Bright.BrightLevel2=k1;
			ptOutParam->Bright.Brightderiv=temp;
			//printf("ptOutParam->Bright.BrightLevel1=%f\n", ptOutParam->Bright.BrightLevel1);
			//printf("ptOutParam->Bright.BrightLevel2=%f\n", ptOutParam->Bright.BrightLevel2);
			//printf("ptOutParam->Bright.Brightderiv=%f\n", ptOutParam->Bright.Brightderiv);
EXT:
			ImgRelease(hMemMgr,&Imggray);
			ImgRelease(hMemMgr,&Imgbgr);
			ImgRelease(hMemMgr,&Imggray1);
			ImgRelease(hMemMgr,&Imgbgr1);
			return res;
}


MRESULT HYIQ_CASTLAB(MHandle hHandle,IQ_PIMAGES pImage, HYIQ_PTOutParam ptOutParam )
{
     MRESULT res=LI_ERR_NONE;
	 IplImage *pRgbImg=cvCreateImage(cvSize(pImage->lWidth,pImage->lHeight),IPL_DEPTH_8U,3);
	 IplImage *pLabImg=cvCreateImage(cvSize(pImage->lWidth,pImage->lHeight),IPL_DEPTH_8U,3);
	// IplImage *pGrayImg=cvCreateImage(cvSize(pImage->lWidth,pImage->lHeight),IPL_DEPTH_8U,1);
	 MLong histA[256]={0};//直方图数据
	 MLong histB[256]={0};
     MFloat fSumA=0.0;
	 MFloat fSumB=0.0;
	 MFloat fAvgA=0.0;
	 MFloat fAvgB=0.0;
     uchar *pData=(uchar*)pLabImg->imageData;//便于直接访问图像数据 速度快
	 MLong lTempA=0;
     MLong lTempB=0;
	 MFloat D=0.0;
	 MFloat Ma=0.0;
	 MFloat Mb=0.0;
	 MFloat M=0.0;
	 MFloat temp=0.0;
	 MLong i=0;
	 MLong j=0;
	 char *pTempData=pRgbImg->imageData;
	 //CvScalar mean_cs,std_cs;  

	 pRgbImg->imageData=(char*)pImage->pixelArray.chunky.pPixel;
	 cvCvtColor(pRgbImg,pLabImg,CV_BGR2Lab);
	// cvCvtColor(pRgbImg,pGrayImg,CV_RGB2GRAY);
     pRgbImg->imageData=pTempData;
	 
	 for (i=0;i<pLabImg->height;i++)
	 {
		 for (j=0;j<pLabImg->width;j++)
		 {
			 lTempA=pData[i*pLabImg->widthStep/sizeof(uchar)+j*pLabImg->nChannels+1];//a
			 lTempB=pData[i*pLabImg->widthStep/sizeof(uchar)+j*pLabImg->nChannels+2];//b
			 fSumA+=(MFloat)lTempA-128.0;
			 fSumB+=(MFloat)lTempB-128.0;
             histA[lTempA]++;
			 histB[lTempB]++;
		 }
	 }

	 fAvgA=fSumA/(pImage->lWidth*pImage->lHeight);
	 fAvgB=fSumB/(pImage->lWidth*pImage->lHeight);
     D=sqrt(fAvgA*fAvgA+fAvgB*fAvgB);
     
	 for (i=0;i<256;i++)
	 {
		 Ma=Ma+fabs((MFloat)i-128.0-fAvgA)*((MFloat)histA[i]);//计算范围-128～127 
		 Mb=Mb+fabs((MFloat)i-128.0-fAvgB)*((MFloat)histB[i]);
	 }
	 //printf("ma=%0.2f fa=%0.2f mb=%0.2f fb=%0.2f \n",Ma,fAvgA,Mb,fAvgB);
	 Ma=Ma/(pImage->lWidth*pImage->lHeight);
	 Mb=Mb/(pImage->lWidth*pImage->lHeight);

     M=sqrt(Ma*Ma+Mb*Mb);
	 //cvAvgSdv(pGrayImg,&mean_cs,&std_cs); 
  //   temp=std_cs.val[0]*std_cs.val[0];
	 //temp=temp-2500;
	 //temp=MIN(temp,1.0);
	 //temp=MAX(fabs(temp),1.0);
	 if (D>=3.0)
	 {
		 
		 ptOutParam->ImgCast.CastValue=D/M;
	 }
	 else
	 {
         ptOutParam->ImgCast.CastValue=0.0;
	 }
	 
     
EXT:
	 if (NULL!=pRgbImg)
	 {
		 cvReleaseImage(&pRgbImg);
		 pRgbImg=NULL;
	 }
	 if (NULL!=pLabImg)
	 {
		 cvReleaseImage(&pLabImg);
		 pLabImg=NULL;
	 }
	 //if (NULL!=pGrayImg)
	 //{
		// cvReleaseImage(&pGrayImg);
		// pLabImg=NULL;
	 //}

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
	/*华中电力定制*/
	double maxRatio = 0.0;
	double maxDev = 0.0;
	double avgDev = 0.0;
	double castValue = 0.0;
	/*华中电力定制*/

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
	/*if (ratiosrc.rratio>0.35||ratiosrc.gratio>0.35||ratiosrc.bratio>0.35)
	{
		ptOutParam->ImgCast.flag=1;///偏色

	}
	else if(ratiosrc.bdev>0.5||ratiosrc.gdev>0.5||ratiosrc.rdev>0.5)
	{
		ptOutParam->ImgCast.flag=1;///偏色
	}
	else
	{
		ptOutParam->ImgCast.flag=0;///正常
	}*/
	/*华中电力定制*/
	maxRatio = MAX(MAX(ratiosrc.rratio, ratiosrc.gratio), ratiosrc.bratio);
	maxDev = MAX(MAX(ratiosrc.bdev, ratiosrc.gdev), ratiosrc.rdev);
	avgDev = (ratiosrc.bdev + ratiosrc.gdev + ratiosrc.rdev) / 3;

	if (maxRatio>0.35)
	{
		castValue = maxRatio*30.3 + 39.4;
	}
	else if (maxDev>0.5)
	{
		castValue = maxDev * 100;
	}
	else
	{
		castValue = avgDev * 100;
	}
	ptOutParam->ImgCast.CastValue = castValue;
	/*华中电力定制*/
	//printf("CastValue=%f\n",ptOutParam->ImgCast.CastValue);
	ptOutParam->ImgCast.Bcastratio=ratiosrc.bratio;
	ptOutParam->ImgCast.Rcastratio=ratiosrc.rratio;
	ptOutParam->ImgCast.Gcastratio=ratiosrc.gratio;
	ptOutParam->ImgCast.rdev=ratiosrc.rdev;
	ptOutParam->ImgCast.gdev=ratiosrc.gdev;
	ptOutParam->ImgCast.bdev=ratiosrc.bdev;

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
MRESULT HYIQ_FROZEN(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam, PCustomParam customp)
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
	if (customp->ImgFrozen.diff1 == MNull)
	{
		customp->ImgFrozen.diff1 = 0.92;
	}
	if (customp->ImgFrozen.diff2 == MNull)
	{
		customp->ImgFrozen.diff2 = 0.98;
	}
	if (customp->ImgFrozen.ratio == MNull)
	{
		customp->ImgFrozen.ratio = 0.025;
	}
	if (customp->ImgFrozen.Hration == MNull)
	{
		customp->ImgFrozen.Hration = 0.7;
	}
	//if(diff>0.92) //阈值根据实验测得，代表两帧图像的相似度
	if (diff>customp->ImgFrozen.diff1)
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
				//printf("ImgFrozen:diff=%f, ratio=%f, Hration=%f\n",diff, ratio, Hration);
				//if (diff>0.98&&(ratio>0.025||Hration<0.7))
				if (diff>customp->ImgFrozen.diff2 && (ratio>customp->ImgFrozen.ratio || Hration<customp->ImgFrozen.Hration))
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

IQ_API MRESULT HYIQ_LostBaseHist(HYIQ_PTOutParam ptOutParam,char *pGrayImage,MLong nImgWidth,MLong nImgHeight, PCustomParam customp)
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
	char * pTempData=NULL;

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
	pTempData=pGrayImg->imageData;
	pGrayImg->imageData= pGrayImage;
	cvResize(pGrayImg,pGrayImg_Cif,CV_INTER_LINEAR);//  pGrayImg_Cif=(352,288)
	if (NULL!=pGrayImg)//释放pGrayImg
	{
        pGrayImg->imageData=pTempData;
		cvReleaseImage(&pGrayImg);
		pGrayImg=NULL;
	}
#ifdef _ZMH_DEBUG
	cvShowImage("pGrayImg_Cif", pGrayImg_Cif);
	cvWaitKey(2);
#endif
	cvSetImageROI(pGrayImg_Cif, rect);  //rect=cvRect(88,72,176,144)
	cvCopy(pGrayImg_Cif, pSubGrayImg,NULL);
	cvResetImageROI(pGrayImg_Cif);
	if (NULL!=pGrayImg_Cif)//释放pGrayImg_Cif
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
	dis_thresh=sqrt(pow(288.0,2.0)+pow(352.0,2.0))/9;//2/9的(176,144)的对角线长度
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
	if (centerPoint.x-75>=0)//如果centerPoint.x-75>=0，矩形宽110在第一次遍历就会越界而退出循环，导致一个矩形框都没有实行
	{
		nBeignX_const=centerPoint.x-75;
	}
	if (centerPoint.y-15>=0)//？字符区域吗
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
			//rect process 通过双高斯模型来模拟双高峰特性
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
			fGradSum=fGradSum/255;//字符边缘所占像素点个数
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

			/*for (k=0;k<2;k++)
			{
				printf("%f ",means[k]);
				printf("%f ",sigmas[k]);
				printf("%f\n",priors[k]);
			}
			printf("-----------------------------------------------\n");*/

			//判断子区域是否为Lost 阈值由实验得出
			//if(means[1]-means[0]>70.0&&sigmas[1]<1500&&sigmas[0]<500&&priors[0]>0.3&&priors[1]>0.3&&fGradSum>770)
			if (means[1] - means[0]>customp->SignalLost.nonblackbackground.meandiff&&sigmas[1]<customp->SignalLost.nonblackbackground.sigma1 && sigmas[0]<customp->SignalLost.nonblackbackground.sigma0  \
				&& priors[0]>customp->SignalLost.nonblackbackground.priors0&&priors[1]>customp->SignalLost.nonblackbackground.priors1&&fGradSum>customp->SignalLost.nonblackbackground.GradSum)
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

	//计算该区域角点数
	for(k=0;k<nCornersNum;k++)
	{
		if (pCorners[k].x>nBeignX_const-5&&pCorners[k].x<nBeignX_const+155&&pCorners[k].y>nBeginY_const-5&&pCorners[k].y                              <nBeginY_const+35)
		{
			nCornersNum_char++;
		}
	}
	//printf("nRectIsLostNum=%d,nRectNum=%d,nCornersNum_char=%d\n", nRectIsLostNum, nRectNum,nCornersNum_char);
	//阈值由实验得出
	//if ((double)nRectIsLostNum/nRectNum>0.7&&nCornersNum_char>27)
	if ((double)nRectIsLostNum / nRectNum>customp->SignalLost.nonblackbackground.lostpercent&&nCornersNum_char>customp->SignalLost.nonblackbackground.charcornersnum)
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
MRESULT HYIQ_LOST(MHandle hHandle,IQ_PIMAGES pImagesrc1,IQ_PIMAGES pImagesrc2,HYIQ_PTOutParam ptOutParam,PCustomParam customp)
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
	//MFloat eps = 0.0000001;
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
	double gmm_t;

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
	switch(imgsrc1.fmtImg)  //转为灰度图像
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
	//printf("the similarity is =%f",diff);
	//printf("lost_diff =%f\n", diff);
	if (customp->SignalLost.diff == MNull)
	{
		customp->SignalLost.diff = 0.92;
	}
	if (customp->SignalLost.blackbackground.grayNum == MNull)
	{
		customp->SignalLost.blackbackground.grayNum = 35;
	}
	if (customp->SignalLost.blackbackground.Hration == MNull)
	{
		customp->SignalLost.blackbackground.Hration = 0.85;
	}
	if (customp->SignalLost.blackbackground.label == MNull)
	{
		customp->SignalLost.blackbackground.label = 25;
	}
	if (customp->SignalLost.nonblackbackground.charcornersnum == MNull)
	{
		customp->SignalLost.nonblackbackground.charcornersnum = 27;
	}
	if (customp->SignalLost.nonblackbackground.GradSum == MNull)
	{
		customp->SignalLost.nonblackbackground.GradSum = 770.0;
	}
	if (customp->SignalLost.nonblackbackground.lostpercent == MNull)
	{
		customp->SignalLost.nonblackbackground.lostpercent = 0.7;
	}
	if (customp->SignalLost.nonblackbackground.meandiff == MNull)
	{
		customp->SignalLost.nonblackbackground.meandiff = 70.0;
	}
	if (customp->SignalLost.nonblackbackground.priors0 == MNull)
	{
		customp->SignalLost.nonblackbackground.priors0 = 0.3;
	}
	if (customp->SignalLost.nonblackbackground.priors1 == MNull)
	{
		customp->SignalLost.nonblackbackground.priors1 = 0.3;
	}
	if (customp->SignalLost.nonblackbackground.sigma0 == MNull)
	{
		customp->SignalLost.nonblackbackground.sigma0 = 500.0;
	}
	if (customp->SignalLost.nonblackbackground.sigma1 == MNull)
	{
		customp->SignalLost.nonblackbackground.sigma1 = 1500.0;
	}
	//if(diff>0.92) //阈值根据实验测得，代表两帧图像的相似度
	if(diff > customp->SignalLost.diff)
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
			//printf(" lost_Hration=%f  lost_grayNum=%d label=%d\n", Hration, grayNum, label);
			//printf("lost_diff=%f  lost_Hration=%f  lost_grayNum=%d label=%d\n",diff,Hration,grayNum,label);
			//if(Hration>0.85&&grayNum<35&&label<25)//标准情况<<黑色背景
			if (Hration>customp->SignalLost.blackbackground.Hration&&grayNum<customp->SignalLost.blackbackground.grayNum && label<customp->SignalLost.blackbackground.label)
			{
				ptOutParam->SignalLost.flag=1; ///信号丢失
			}
			else//其他
			{
				//gmm_t=(double)cvGetTickCount();
				res=HYIQ_LostBaseHist(ptOutParam,(char*)imggray2.pixelArray.chunky.pPixel,imggray2.dwWidth,imggray2.dwHeight,customp);
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

//场景变化检测
MRESULT HYIQ_SceneChange(MHandle hHandle,IQ_PIMAGES pImage,IQ_PIMAGES pImage1, HYIQ_PTOutParam ptOutParam)
{
      MRESULT res=LI_ERR_NONE;
      
      return res;
}


///\brief HYIQ_RESYLT show the result
///判别用户是否输入判别阈值，如果用户没有输入，则使用默认值，如果用户输入了，则采用用户设置的值
MRESULT HYIQ_RESULT(MHandle hHandle,HYIQ_PTOutParam ptOutParam,PCustomParam customp,pHYIQ_result result)
{
	MRESULT res=LI_ERR_NONE;
	IMQU *pImqu = (IMQU *)hHandle;
	MHandle hMemMgr=MNull;
	MFloat eps=0.0000001;
	if (pImqu == MNull )
	{res = LI_ERR_UNKNOWN; goto EXT;}
	hMemMgr = pImqu->hMemMgr;
	/* 模糊部分的阈值是根据matlab给清晰图像添加不同sigma的高斯模糊之后获得的。轻度模糊表示图像存在轻度模糊，不影响图像实际内容,中度模糊表示图像中存在较
	为明显的模糊，并且图像内容受影响，重度模糊表示图像存在明显模糊，图像内容受严重影响无法辨识*/
	if (customp->ClearLevel.ClearLevellowthresh==MNull)
	{
		customp->ClearLevel.ClearLevellowthresh=1.4;//2.5
	}
	if(customp->ClearLevel.ClearLevelhighthresh==MNull)
	{
		customp->ClearLevel.ClearLevelhighthresh=20;//5
	}
	//if(customp->ClearLevel.ClearLevelhighthresh1==MNull)
	//{
	// customp->ClearLevel.ClearLevelhighthresh1=20;
	//}
	/* 噪声部分的阈值是根据matlab给清晰图像添加不同sigma的高斯噪声之后获得的。低度噪声是指存在噪声，但是噪声的存在不影响实际图像内容
	中度噪声等级是指，存在噪声，并且较为明显。重度噪声等级是指，存在噪声，并且较为明显，会对后续检测有所影响。*/
	if (customp->Noiselevel.NoiseLevellowthresh==MNull)
	{
		customp->Noiselevel.NoiseLevellowthresh=3;
	}
	if(customp->Noiselevel.Noiselevelhighthresh==MNull)
	{
		customp->Noiselevel.Noiselevelhighthresh=20;//20
	}
	//if(customp->Noiselevel.Noiselevelhighthresh1==MNull)
	//{
	// customp->Noiselevel.Noiselevelhighthresh1=20;
	//}
	if((customp->castRatioThresh-eps)<0)
	{
		//customp->castRatioThresh=1.5;
		customp->castRatioThresh = 50;
	}
	/*亮度部分的阈值是根据实际数据测试所得。大于过亮阈值代表图像过亮，小于过暗阈值则代表图像过暗*/
	if((customp->Light.darkthresh-eps)<0)
	{
		customp->Light.darkthresh=-0.5;//-0.5
	}
	if((customp->Light.lightthresh-eps)<0)
	{
		customp->Light.lightthresh=0.5;//0.5
	}

	//judge the image status
	/// 将实验值和阈值进行比较，输出实际的判断
	if (ptOutParam->CLEAR.isDetct==1)
	{

		if(ptOutParam->CLEAR.CLEARLEVEL<customp->ClearLevel.ClearLevellowthresh)
		{
			result->clearstatus=IMAGE_CLEAR; /*图像清晰*/
		}
		if((ptOutParam->CLEAR.CLEARLEVEL>customp->ClearLevel.ClearLevellowthresh)&&(ptOutParam->CLEAR.CLEARLEVEL<customp->ClearLevel.ClearLevelhighthresh))
		{
			result->clearstatus=IMAGE_NOTCLEAR;  /*图像不清晰*/
		}
		//if((ptOutParam->ClearLevel>customp->ClearLevel.ClearLevelhighthresh)&&(ptOutParam->ClearLevel<customp->ClearLevel.ClearLevelhighthresh1))
		//{
		// result->blurstatus=IMAGE_MEDBLUR;  /*图像中度模糊*/
		//}
		if(ptOutParam->CLEAR.CLEARLEVEL>customp->ClearLevel.ClearLevelhighthresh)
		{
			result->clearstatus=IMAGE_HEVNOTCLEAR; /*图像重度不清晰*/
		}
	}

	if (ptOutParam->Bright.isDetct==1)
	{
		if(ptOutParam->Bright.BrightLevel1>customp->Light.lightthresh)
		{
			result->lightstatus=IMAGE_TOOLIGHT;  /*图像过亮*/
		}
		if(ptOutParam->Bright.BrightLevel1<customp->Light.darkthresh/*&&ptOutParam->Bright.grayNum>50*/)
		{
			result->lightstatus=IMAGE_TOODARK;   /*图像过暗*/
		}
	}

	if (ptOutParam->NOISE.isDetct==1)
	{
		if(ptOutParam->NOISE.NOISELEVEL<customp->Noiselevel.NoiseLevellowthresh)
		{
			result->Noisestatus=IMAGE_UNNOISE;   /*图像不存在噪声*/
		}
		if((ptOutParam->NOISE.NOISELEVEL>customp->Noiselevel.NoiseLevellowthresh)&&(ptOutParam->NOISE.NOISELEVEL<customp->Noiselevel.Noiselevelhighthresh))
		{
			result->Noisestatus=IMAGE_NOISE;  /*图像存在噪声*/
		}
		//if((ptOutParam->NOISELEVEL>customp->Noiselevel.Noiselevelhighthresh)&&(ptOutParam->NOISELEVEL<customp->Noiselevel.Noiselevelhighthresh1))
		//{
		// result->Noisestatus=IMAGE_MEDNOISE;   /*图像存在中度噪声*/
		//}
		if(ptOutParam->NOISE.NOISELEVEL>customp->Noiselevel.Noiselevelhighthresh)
		{
			result->Noisestatus=IMAGE_HEAVENOISE; /*图像存在重度噪声*/
		}
	}

	if (ptOutParam->SignalLost.flag==1&&(ptOutParam->SignalLost.isDetct==1))
	{
		result->signalLoststatus=IMAGE_SIGNALLOST; /*信号丢失*/
	}
	if(ptOutParam->ImgFrozen.flag==1&&(ptOutParam->ImgFrozen.isDetct==1))
	{
		result->imgFrozenstatus=IMAGE_FROZEN;     /*画面冻结*/
	}
	if(ptOutParam->ImgCast.CastValue>customp->castRatioThresh&&(ptOutParam->ImgCast.isDetct==1))
	{
		result->caststatus=IMAGE_CAST;      /*图像偏色*/	   
	}
	//if(ptOutParam->ImgCast.flag&&(ptOutParam->ImgCast.isDetct==1))
	//{
	//	result->caststatus=IMAGE_CAST;      /*图像偏色*/	   
	//}

EXT:
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
