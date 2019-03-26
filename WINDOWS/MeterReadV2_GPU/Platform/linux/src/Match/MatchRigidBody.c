
#include "licomdef.h"
#include "limem.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "lidebug.h"
#include "liresize.h"
#include"liintegral.h"
#include"MatchRigidBody.h"
//#include "LineGeometry.h"
#include "litimer.h"
#include "gradient.h"
#include "liedge.h"
#include"hisi.h"
#include "rotateBlock.h"

//#define PROCESS_RADIUS 1
//#define QUASI_SQRT(x, y, rlt) {MInt32 x1=ABS(x); MInt32 y1=ABS(y); MInt32 mn=MIN(x1,y1); rlt=x1+y1-(mn>>1)-(mn>>2)+(mn>>4);}
#define pi 3.14159265358979323846
//MVoid _CreateGradImg_Match(const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MDouble *pMag, MUInt16 *pAngle);
static MVoid WriteMInt16Data(const char *path, const MInt16 *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight);
static  MRESULT GetRectImage(MHandle hMemMgr,BLOCK *pImage,BLOCK *pDst,MRECT rect);
static  MVoid WriteMUInt16Data(const char *path, const MUInt16 *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight);
//MRESULT RotateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MLong lAngle);
static  MRESULT GetRectFromMask(BLOCK *pMask,PMRECT pMaskRect);
static  MRESULT GetDirector(MHandle hMemMgr,BLOCK *pMask,MLong *ldegree);
static  MRESULT GetOperateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MRECT rect,MLong lSample);
//MVoid Max_32f_C1R(const MDouble* pSrc, MLong lBlockLine, TSize roiSize, MDouble* pMax);
static  MVoid Max_32f_C1R(const MUInt16* pSrc, MLong lBlockLine, TSize roiSize, MUInt16* pMax);
//MVoid MaxIndx_32f_C1R(const MDouble* pSrc, MLong lBlockLine, TSize roiSize, MDouble* pMax,MLong *pInX,MLong *pInY);
static MVoid MaxIndx_32f_C1R(const MUInt16* pSrc, MLong lBlockLine, TSize roiSize, MUInt16* pMax,MLong *pInX,MLong *pInY);
static MRESULT create_template_fast(MHandle hMemMgr,	BLOCK * pImage, MLong a_grad_num,MDouble a_thres ,TDescriptor* pDescriptor);
//MVoid WriteMDOubleData(const char *path, const MDouble *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight);
//MVoid WriteMUInt8Data(const char *path, const MUInt8 *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight);
static MRESULT ComputeGradients(MHandle hMemMgr,BLOCK *pImage,MLong lSample,MUInt8 **GradientData);
static MVoid shift_copy(MUInt8 * ap_pix_old, MUInt8 * ap_pix_new ,MLong num);
//MVoid shift_right(	MUInt8 * ap_img, MUInt8 * ap_list,MLong a_width, MLong a_row, MLong a_col,TSize tsize );
static MVoid shift_right(	MUInt8 * ap_img, MUInt8 * ap_list,MUInt8*ap_tmp,MLong a_width, MLong a_row, MLong a_col,TSize tsize );
//MVoid shift_down(MUInt8 * ap_img, MUInt8 * ap_list,MLong a_width, MLong a_row, MLong a_col ,TSize tsize);
static MVoid shift_down(MUInt8 * ap_img, MUInt8 * ap_list,MUInt8 *ap_tmp,MLong a_width, MLong a_row, MLong a_col ,TSize tsize);
static MVoid shift_init(MUInt8 *ap_img,MUInt8* ap_pix_new,MLong lWidth, MLong lHeight ,TSize tsize);
//MVoid MatchRigidBody(MHandle hMemMgr,const MUInt8 *ap_img,MDouble a_thres,MLong a_width,MLong l_height,TDescriptor *tDescriptor);
//MRESULT ComputeGradients(MHandle hMemMgr,BLOCK *pImage,MLong lSample,MUInt8 *GradientData);
static MRESULT MaskTemplate(MHandle hMemMgr,TDescriptor *pDescriptor,BLOCK *pMask,MRECT rect);
static MVoid DrawLine(BLOCK *pImage,MLong lx,MLong ly,MLong lWidth,MLong lHeight);
static MRESULT TrainTemplate(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask,PTDescriptorClass ptDescriptorClass);
static MRESULT MatchProcess(MHandle hMemMgr,MUInt8 *ap_img,MDouble a_thres,MLong a_width,MLong a_height,TDescriptor *tDescriptor,MLong *pX,MLong *pY);
//MVoid DrawLine(BLOCK *pImage,MLong lx,MLong ly,MLong lWidth,MLong lHeight);
/*===========================================================
函数名	：TrainTemplateMethod
功能		：根据不同的输入方法，选择不同的训练方式
输入参数说明：			
				
				hMemMgr：操作句柄
				pImage：输入图像
				pMask：输入Mask图像
				lParam：训练方法，1为对图像进行旋转，0为对图像不进行旋转
输出参数说明：ptDescriptorClass：描述符类              
返回值说明：错误的类别
===========================================================*/
MRESULT TrainTemplateMethod(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask,MLong lParam,PTDescriptorClass ptDescriptorClass)
{

	MRESULT res = LI_ERR_NONE;
	BLOCK imageMean={0};
	BLOCK imageRotate={0};
	BLOCK maskRotate={0};
	MUInt8 *pImgWorkBuf=MNull;
	MUInt8 *pMaskWorkBuf=MNull;
	MLong i=0;
	MLong lWidth=pImage->lWidth;
	MLong lHeight=pImage->lHeight;
	MLong lRotatedWidth,lRotatedHeight;
	//GO(SmoothBlock())
	GO(B_Create(hMemMgr,&imageMean,DATA_U8,pImage->lWidth,pImage->lHeight));
	/*
	HI_MPI_IVE_FILTER 
	pImage->pBlockData为源数据指针
	imageMean.pBlockData为输出数据指针
	功能为平滑处理
	*/
	GO(SmoothBlock(hMemMgr,
		pImage->pBlockData,pImage->lBlockLine,pImage->typeDataA,
		imageMean.pBlockData,imageMean.lBlockLine,imageMean.typeDataA,
		imageMean.lWidth,imageMean.lHeight,4));
	if (lParam==0)//不对图片进行旋转处理
	{
		GO(TrainTemplate(hMemMgr,&imageMean,pMask,ptDescriptorClass));
	}
	if(lParam==1)//对图像进行旋转处理
	{
		//AllocVectMem(hMemMgr,pImgWorkBuf,10000000,MUInt8);
		//AllocVectMem(hMemMgr,pMaskWorkBuf,10000000,MUInt8);
		lRotatedWidth=lRotatedHeight=(MLong) (sqrt((MDouble)lWidth*lWidth+lHeight*lHeight));
		GO(B_Create(hMemMgr,&imageRotate,DATA_U8,lRotatedWidth,lRotatedHeight));
		GO(B_Create(hMemMgr,&maskRotate,DATA_U8,lRotatedWidth,lRotatedHeight));

		for (i=0;i<36;i++)
		{
			//GO(RotateImage(&imageMean,&imageRotate,10*i));
			//GO(RotateImage(pMask,&maskRotate,10*i));
			RotateImage_EX(&imageMean,&imageRotate,10*i);
			//PrintBmpEx(imageRotate.pBlockData, imageRotate.lBlockLine,imageRotate.typeDataA & ~0x100,imageRotate.lWidth, imageRotate.lHeight, 1, "D:\\imageRotate2.bmp");
			RotateImage_EX(pMask,&maskRotate,10*i);
			//PrintBmpEx(maskRotate.pBlockData, maskRotate.lBlockLine,maskRotate.typeDataA & ~0x100,maskRotate.lWidth, maskRotate.lHeight, 1, "D:\\maskRotate2.bmp");

			GO(TrainTemplate(hMemMgr,&imageRotate,&maskRotate,ptDescriptorClass));
			//B_Release(hMemMgr,&imageRotate);
			//B_Release(hMemMgr,&maskRotate);
		}
		
	}
EXT:
	
	B_Release(hMemMgr,&imageMean);
	//FreeVectMem(hMemMgr,pImgWorkBuf);
	//FreeVectMem(hMemMgr,pMaskWorkBuf);
	B_Release(hMemMgr,&imageRotate);
	B_Release(hMemMgr,&maskRotate);
	return res;
}
/*===========================================================
函数名	：TrainTemplate
功能		：对输入的图像进行训练
输入参数说明：			
				
				hMemMgr：操作句柄
				pImage：输入图像
				pMask：输入Mask图像
输出参数说明：ptDescriptorClass：描述符类              
返回值说明：错误的类别
===========================================================*/
MRESULT TrainTemplate(MHandle hMemMgr,BLOCK *pImage,BLOCK *pMask,PTDescriptorClass ptDescriptorClass)
{
	//MInt16 lSpreedRange=7;
	MLong lSample=5;

	MInt16 *pImgX=NULL;
	MInt16 *pImgY=NULL;
	MUInt16 *pMag=NULL;
	MUInt16 *pAngle=NULL;
	//MUInt8 *pBinaryString=NULL;
	//MUInt8 *pBinaryStringSpreed=NULL;
	MUInt8 *pTemplateData=MNull;
	MUInt8 *pImageData=MNull;
	MLong ldegree;
	MRESULT res = LI_ERR_NONE;
	//TDescriptor tDescriptor;
	BLOCK imageRotate={0};
	BLOCK maskRotate={0};
	BLOCK operateImage={0};
	BLOCK imageMean={0};
	MRECT rect;
	TDescriptor tDescriptor={0};

	

//    PrintBmpEx(pMask->pBlockData, pMask->lBlockLine,pMask->typeDataA & ~0x100,pMask->lWidth, pMask->lHeight, 1, "D:\\maskimage.bmp");

	res = GetRectFromMask(pMask,&rect);
	if(LI_ERR_NONE!=res)
		return res;
	GO(GetDirector(hMemMgr,pMask,&ldegree));
	GO(GetOperateImage(hMemMgr,pImage,&operateImage,rect,lSample));

//	PrintBmpEx(operateImage.pBlockData, operateImage.lBlockLine,operateImage.typeDataA & ~0x100,operateImage.lWidth, operateImage.lHeight, 1, "D:\\operateimg.bmp");

	tDescriptor.lHeight=(operateImage.lHeight-2)/lSample-1;
	tDescriptor.lWidth=(operateImage.lWidth-2)/lSample-1;
	tDescriptor.lSample=lSample;
	tDescriptor.lRegion=(tDescriptor.lHeight)*(tDescriptor.lWidth);
	tDescriptor.lAngle=ldegree;
	tDescriptor.lDescriptorIndex=ptDescriptorClass->lNumDescriptors;
	tDescriptor.lNumber=0;

	GO(create_template_fast(hMemMgr,&operateImage, 7,0.9 ,&tDescriptor));
	GO(MaskTemplate(hMemMgr,&tDescriptor,pMask,rect));
	//PrintChannel()
	ptDescriptorClass->lNumDescriptors++;
	*(ptDescriptorClass->ptDescriptor+ptDescriptorClass->lNumDescriptors-1)=tDescriptor;
		//tDescriptorNum->num++;
		//*(tDescriptorNum->tDescriptor+i)=tDescriptor;
		//B_Release(hMemMgr,&imageMean);
		//PrintBmpEx(pImage->pBlockData, pImage->lBlockLine,pImage->typeDataA & ~0x100,pImage->lWidth, pImage->lHeight, 1, "D:\\image.bmp");
		
		//PrintBmpEx(pMask->pBlockData, pMask->lBlockLine,pMask->typeDataA & ~0x100,pMask->lWidth, pMask->lHeight, 1, "D:\\mask.bmp");
		//PrintBmpEx(maskRotate.pBlockData, maskRotate.lBlockLine,maskRotate.typeDataA & ~0x100,maskRotate.lWidth, maskRotate.lHeight, 1, "D:\\maskRotate.bmp");
		//B_Release(hMemMgr,&maskRotate);
		//B_Release(hMemMgr,&imageRotate);
	
//	}
	//WriteTDescriptorNum("D:\\descriptor.txt",tDescriptorNum);



	/*
	WriteMInt16Data("D:\\gx.txt",pImgX,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	WriteMInt16Data("D:\\gy.txt",pImgY,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	WriteMUInt16Data("D:\\Mag.txt",pMag,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	WriteMUInt16Data("D:\\Angle.txt",pAngle,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	*/


EXT:
	//FreeVectMem(hMemMgr,pImgX);
	//FreeVectMem(hMemMgr,pImgY);
	//FreeVectMem(hMemMgr,pMag);
	//FreeVectMem(hMemMgr,pAngle);
	B_Release(hMemMgr,&imageMean);
	B_Release(hMemMgr,&operateImage);
	return res;

}


MVoid WriteMInt16Data(const char *path, const MInt16 *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight)
{
	int y,x;
	FILE *fp=fopen(path,"w");
	for (y=0;y<lHeight;y++)
	{
		for (x=0;x<lWidth;x++)
		{
			fprintf(fp,"%d ",*(pData+lImgLine*y+x));
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
}

MVoid WriteMUInt16Data(const char *path, const MUInt16 *pData, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight)
{
	int y,x;
	FILE *fp=fopen(path,"w");
	for (y=0;y<lHeight;y++)
	{
		for (x=0;x<lWidth;x++)
		{
			fprintf(fp,"%d ",*(pData+lImgLine*y+x));
		}
		fprintf(fp,"\n");
	}
	fclose(fp);

}


MRESULT GetRectFromMask(BLOCK *pMask,PMRECT pMaskRect)
{
	MRECT rect;
	MInt16 x,y;
	MUInt8* pMaskData=(MUInt8*)(pMask->pBlockData);
	int count = 0;
	MRESULT res = LI_ERR_NONE;
	rect.left=pMask->lWidth;
	rect.right=0;
	rect.top=pMask->lHeight;
	rect.bottom=0;
	
	for (y=0;y<pMask->lHeight;y++)
	{
		for (x=0;x<pMask->lWidth;x++)
		{
			if (*(pMaskData+y*pMask->lBlockLine+x)>100)
			{
				count++;
				if(rect.left>x) 
				{
					rect.left=x;
				}
				if(rect.right<x)
				{
					rect.right=x;
				}
				if (rect.top>y)
				{
					rect.top=y;
				}
				if(rect.bottom<y)
				{
					rect.bottom=y;
				}
			}
		}
	}
	if (0==count)
	{
		res = LI_ERR_NO_FIND;
		return res;
	}
	pMaskRect->bottom=rect.bottom;
	pMaskRect->left=rect.left;
	pMaskRect->right=rect.right;
	pMaskRect->top=rect.top;
	return res;
}


/*===========================================================
函数名	：GetDirector
功能		：获取图像的主方向
输入参数说明：			
				hMemMgr：操作句柄
				pMask：输入Mask图像
输出参数说明：ldegree：主方向的大小
返回值说明：错误的类别
===========================================================*/
MRESULT GetDirector(MHandle hMemMgr,BLOCK *pMask,MLong *ldegree)
{
	MLong x,y;
	MUInt8 *pData;
	//int *pointx=(int*)malloc(*sizeof (int));
	//int *pointy=(int*)malloc(*sizeof (int));
	MLong *pointx=MNull;
	MLong *pointy=MNull;
	MLong *lx;
	MLong *ly;
	MLong number=0;
	MLong i;
	MDouble sumx=0,sumy=0,meanx,meany;
	MDouble mat11=0,mat12=0,mat21=0,mat22=0;
	MDouble result;
	MRESULT res = LI_ERR_NONE;
	MLong lExt;

	AllocVectMem(hMemMgr,pointx,pMask->lHeight*pMask->lWidth,MLong);
	AllocVectMem(hMemMgr,pointy,pMask->lHeight*pMask->lWidth,MLong);
	lx=pointx;
	ly=pointy;
	lExt=pMask->lBlockLine-pMask->lWidth;
	pData=(MUInt8*)(pMask->pBlockData);
	for (y=0;y<pMask->lHeight;y++,pData+=lExt)
	{
		//pData=(MUInt8*)(pMask->pBlockData)+y*pMask->lBlockLine;

		for (x=0;x<pMask->lWidth;x++,pData++)
		{
			if (*(pData)>0)
			{
				*lx=x;
				*ly=y;
				lx++;
				ly++;
				number++;
			}
		}
	}
	lx=pointx;
	ly=pointy;
	for (i=0;i<number;i++)
	{
		sumx+=(*lx);
		sumy+=(*ly);
		lx++;
		ly++;
	}

	meanx=sumx/number;
	meany=sumy/number;

	lx=pointx;
	ly=pointy;
	for(i=0;i<number;i++)
	{
		*lx=(MLong)(*lx-meanx);
		*ly=(MLong)(*ly-meany);
		lx++;
		ly++;
	}


	lx=pointx;
	ly=pointy;	
	for(i=0;i<number;i++)
	{
		mat11+=(*lx)*(*lx);
		mat12+=(*lx)*(*ly);
		mat21+=(*lx)*(*ly);
		mat22+=(*ly)*(*ly);
		lx++;
		ly++;
	}

	mat11/=number;
	mat12/=number;
	mat21/=number;
	mat22/=number;

	result=(mat22-mat11+sqrt((mat11-mat22)*(mat11-mat22)+4*mat12*mat21))/2/mat12;
	*ldegree=(MLong)(atan(result)/pi*180);
EXT:
	FreeVectMem(hMemMgr,pointx);
	FreeVectMem(hMemMgr,pointy);
	return res;
}

MRESULT GetOperateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MRECT rect,MLong lSample)
{
	BLOCK pImage={0};
	MLong x,y;
	MLong lHeight=rect.bottom-rect.top+1;
	MLong lWidth=rect.right-rect.left+1;
	MUInt8 *pImageData;
	MUInt8 *pSrcData=(MUInt8*)(pSrc->pBlockData);
	MRESULT res = LI_ERR_NONE;
	MLong lOperateImageWidth=lSample*(lWidth/lSample+1)+2;
	MLong lOperateImageHeight=lSample*(lHeight/lSample+1)+2;
	MLong lImgExt,lSrcExt;

	GO(B_Create(hMemMgr,&pImage,DATA_U8,lWidth,lHeight));
	GO(B_Create(hMemMgr,pDst,DATA_U8,lOperateImageWidth,lOperateImageHeight));

	pSrcData+=rect.top*(pSrc->lBlockLine)+rect.left;//原图的矩形框左上角的数据
	pImageData=(MUInt8*)(pImage.pBlockData);
	lSrcExt=pSrc->lBlockLine-lWidth;//由于需获取矩形框中的图像信息，所以其有效的长度为lWidth
	lImgExt=pImage.lBlockLine-lWidth;
	//lImgTmp=0;
	//lSrcTmp=0;

	for (y=rect.top;y<=rect.bottom;y++,pImageData+=lImgExt,pSrcData+=lSrcExt)
	{
		for (x=rect.left;x<=rect.right;x++,pImageData++,pSrcData++)
		{
			*pImageData=*pSrcData;
		}
	}

	//PrintBmpEx(pImage.pBlockData, pImage.lBlockLine,pImage.typeDataA & ~0x100,pImage.lWidth, pImage.lHeight, 1, "D:\\operate.bmp");
	Resize(pImage.pBlockData,pImage.lBlockLine,pImage.lWidth,pImage.lHeight,pDst->pBlockData,pDst->lBlockLine,pDst->lWidth,pDst->lHeight,BILINEAR);
	//PrintBmpEx(pDst->pBlockData,pDst->lBlockLine,pDst->typeDataA& ~0x100,pDst->lWidth,pDst->lHeight,1,"D:\\operate2.bmp");


EXT:
	B_Release(hMemMgr,&pImage);
	return res;


}
MRESULT create_template_fast(MHandle hMemMgr,	BLOCK * pImage, MLong a_grad_num,MDouble a_thres ,TDescriptor * pDescriptor)
{
	MRESULT res = LI_ERR_NONE;
	MUInt16 m_min_norm=10;
	MLong l_r,l_c,l_x,l_y;
	MLong M=pDescriptor->lHeight;
	MLong N=pDescriptor->lWidth;
	MLong S=pDescriptor->lSample;
	MLong G=pDescriptor->lRegion;

	MLong l_off_x=1+S/2;
	MLong l_off_y=1+S/2;
	MLong m_bins=8;

	MLong l_mstep=pImage->lBlockLine;
	//MLong l_mstep = lp_sobel_mg->widthStep/sizeof(float);
	//MLong l_istep = ap_img->widthStep/sizeof(float);

	MDouble l_divisor = 12.0/((m_bins-1));

	MUInt32 l_gmax_gra=0;
	MUInt16 l_lmax_gra=0;
	MLong l_yall,l_xall;
	MLong l_counter;
	MLong l_inx,l_iny;

	MUInt32 *lp_strn_ptr=MNull;
	MUInt32 *lp_strn_tmp=MNull;
	MUInt8 *lp_peak_ptr=MNull;
	MUInt8 *lp_peak_tmp=MNull;
	MInt16 *pImgX=NULL;
	MInt16 *pImgY=NULL;
	MUInt16 *pMag=NULL;
	MUInt16 *pAngle=NULL;
	MInt16 *pxcord=MNull;
	MInt16 *pycord=MNull;
	MUInt16 *pvalcord=MNull;

	MLong l_k;
	MUInt16 l_ang1;
	MLong l_bin1;
	MUInt16 l_max=0;
	TSize l_size;
	l_size.lHeight=S;
	l_size.lWidth=S;

	
	AllocVectMem(hMemMgr,pImgX,(pImage->lBlockLine)*(pImage->lHeight),MInt16);
	AllocVectMem(hMemMgr,pImgY,(pImage->lBlockLine)*(pImage->lHeight),MInt16);
	AllocVectMem(hMemMgr,pMag,(pImage->lBlockLine)*(pImage->lHeight),MUInt16);
	AllocVectMem(hMemMgr,pAngle,(pImage->lBlockLine)*(pImage->lHeight),MUInt16);
	memset(pMag,0,(pImage->lBlockLine)*(pImage->lHeight)*sizeof(MUInt16));
	//_CreateGradImg_Match(pImage->pBlockData,pImage->lBlockLine,pImage->lWidth,pImage->lHeight,pImgX,pImgY,pMag,pAngle);

	/*
	HI_MPI_IVE_SOBEL
	pImage->pBlockData为源数据指针，
	pImgX为水平方向输出数据指针
	pImgY为竖直方向输出数据指针，
	功能为获取梯度信息，
	pMag为梯度值大小，pAngle为梯度角度信息。
	梯度值大小与梯度角度信息在HI_MPI_IVE_SOBEL之后再计算
	*/

	HISI_createGradAngle(pImage->pBlockData,pImage->lBlockLine,pImage->lWidth,pImage->lHeight,pImgX,pImgY,pMag,pAngle);
	//WriteMInt16Data("D:\\gx.txt",pImgX,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMInt16Data("D:\\gy.txt",pImgY,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMUInt16Data("D:\\Mag.txt",pMag,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMUInt16Data("D:\\Angle.txt",pAngle,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);

	AllocVectMem(hMemMgr,lp_strn_tmp,M*N,MUInt32);
	AllocVectMem(hMemMgr,lp_peak_tmp,M*N,MUInt8);
	AllocVectMem(hMemMgr,pxcord,a_grad_num,MInt16);
	AllocVectMem(hMemMgr,pycord,a_grad_num,MInt16);
	AllocVectMem(hMemMgr,pvalcord,a_grad_num,MUInt16);


	memset(lp_peak_tmp,0,M*N);
	lp_strn_ptr=lp_strn_tmp;
	lp_peak_ptr=lp_peak_tmp;

	for(  l_r=0; l_r<M; ++l_r )
	{
		for( l_c=0; l_c<N; ++l_c )
		{
			*lp_strn_ptr=0;
			for( l_y=-S/2; l_y<=S/2; l_y+=S/2 )
			{
				l_yall = l_off_y+l_y;

				for( l_x=-S/2; l_x<=S/2; l_x+=S/2 )
				{
					l_xall		= l_off_x+l_x;
					l_counter	= 0;

					Max_32f_C1R( pMag+l_yall*l_mstep+l_xall, l_mstep, l_size, &l_lmax_gra);


					(*lp_strn_ptr) = l_lmax_gra+(*lp_strn_ptr);

					while( 1 )
					{
						MaxIndx_32f_C1R( pMag+l_yall*l_mstep+l_xall, l_mstep, l_size, &l_max,&l_inx,&l_iny);
				

						if( l_lmax_gra < m_min_norm )
						{
							*lp_peak_ptr |= 1 << (m_bins-1);

							break;
						}
						if( l_lmax_gra*a_thres > l_max || l_counter >= a_grad_num ) break; 
						//if( l_lmax_gra-m_min_norm > l_max || l_counter >= a_grad_num ) break; 
						//if( l_lmax_gra-m_min_norm > l_max  ) break; 
						//if( l_counter >= 1 ) break;

						++l_counter;

						//l_ang1 = (CV_IMAGE_ELEM(lp_sobel_ag,float,l_iny+l_yall,l_inx+l_xall)-0.5);
						l_ang1 = *(pAngle+(l_iny+l_yall)*l_mstep+l_inx+l_xall);
						l_bin1 = (MUInt8)((l_ang1>=12?l_ang1-12:l_ang1)/l_divisor);
						//l_bin1 = (MUInt8)(l_ang1/l_divisor);

						*lp_peak_ptr |= 1 << l_bin1;

						*(pxcord+l_counter-1)=(MInt16)(l_inx+l_xall);
						*(pycord+l_counter-1)=(MInt16)(l_iny+l_yall);
						*(pvalcord+l_counter-1)=l_max;

						*(pMag+(l_iny+l_yall)*l_mstep+l_inx+l_xall)=0;

						//l_xcord.push_back(l_inx+l_xall);
						//l_ycord.push_back(l_iny+l_yall);
						//l_val.push_back(l_max);

						//CV_IMAGE_ELEM(lp_sobel_mg,float,l_iny+l_yall,l_inx+l_xall) = -1;
					}
					for (l_k=0;l_k<l_counter;l_k++)
					{
						*(pMag+(pycord[l_k])*l_mstep+pxcord[l_k])=pvalcord[l_k];
					}							
					
				}
			}
			++lp_strn_ptr;
			++lp_peak_ptr;
			l_off_x += S;
		}
		l_off_y += S;
		l_off_x  = S/2+1;
	}
	//WriteMUInt8Data("D:\\templateData2.txt",lp_peak_tmp,N,N,M);
	pDescriptor->pData=lp_peak_tmp;
	//for (x=0;)
	
EXT:
	if(res==LI_ERR_ALLOC_MEM_FAIL)
	{
		FreeVectMem(hMemMgr,lp_peak_tmp);
	}
	FreeVectMem(hMemMgr,lp_strn_tmp);
	FreeVectMem(hMemMgr,pImgX);
	FreeVectMem(hMemMgr,pImgY);
	FreeVectMem(hMemMgr,pMag);
	FreeVectMem(hMemMgr,pAngle);
	FreeVectMem(hMemMgr,pxcord);
	FreeVectMem(hMemMgr,pycord);
	FreeVectMem(hMemMgr,pvalcord);
	return res;
}
MVoid Max_32f_C1R(const MUInt16* pSrc, MLong lBlockLine, TSize roiSize, MUInt16* pMax)
{
	MLong x;
	MLong y;
	MUInt16 lmax=0;

	for (y=0;y<roiSize.lHeight;y++)
	{
		for (x=0;x<roiSize.lWidth;x++)
		{
			if (*(pSrc+y*lBlockLine+x)>lmax)
			{
				lmax=*(pSrc+y*lBlockLine+x);
			}
		}
	}
	*pMax=lmax;
}

MVoid MaxIndx_32f_C1R(const MUInt16* pSrc, MLong lBlockLine, TSize roiSize, MUInt16* pMax,MLong *pInX,MLong *pInY)
{
	MLong x;
	MLong y;
	MUInt16 lmax=0;
	MLong lx=0;
	MLong ly=0;
	MUInt16 *pTemp = (MUInt16*)pSrc;
	MLong lExt=lBlockLine-roiSize.lWidth;

	for (y=0;y<roiSize.lHeight;y++,pTemp+=lExt)
	{
		for (x=0;x<roiSize.lWidth;x++,pTemp++)
		{
			if (*(pTemp)>lmax)
			{
				lmax=*(pTemp);
				lx=x;
				ly=y;
			}
		}
	}
	*pMax=lmax;
	*pInX=lx;
	*pInY=ly;
}
MRESULT MatchProcess(MHandle hMemMgr,MUInt8 *ap_img,MDouble a_thres,MLong a_width,MLong a_height,TDescriptor *tDescriptor,MLong *pX,MLong *pY)
{
	MLong M=tDescriptor->lHeight;
	MLong N=tDescriptor->lWidth;
	MLong S=tDescriptor->lSample;
	MLong m_elem=M*N;
	MLong m_size=(M+1)*((N+31)/32)*32;
	MLong l_sum;
	MLong l_max=0;
	MLong l_x=-1;
	MLong l_y=-1;
	MLong l_r;
	MLong l_c;
	MRESULT res = LI_ERR_NONE;
	

	MUInt8 * lp_pix_row=MNull ;
	MUInt8 * lp_pix_col=MNull ;
	MUInt8 * lp_pix_tmp=MNull ;
	TSize tsize;
	int i;
	AllocVectMem(hMemMgr,lp_pix_col,m_size,MUInt8);
	AllocVectMem(hMemMgr,lp_pix_row,m_size,MUInt8);
	AllocVectMem(hMemMgr,lp_pix_tmp,m_size,MUInt8);

	memset(lp_pix_col,0,m_size);
	memset(lp_pix_row,0,m_size);
	memset(lp_pix_tmp,0,m_size);

	tsize.lHeight=M;
	tsize.lWidth=N;

	shift_init(ap_img,lp_pix_row,a_width,a_height,tsize);
	shift_copy(lp_pix_row,lp_pix_col,m_size);

	for( l_r=0; l_r<a_height-M; ++l_r )
	{
		for( l_c=0; l_c<a_width-N; ++l_c )
		{
			l_sum=0;
			for (i=0;i<m_elem;i++)
			{
				if ((tDescriptor->pData[i])&(lp_pix_col[i]))
				{
					l_sum++;
				}
			}
			
			
			if (l_sum>=a_thres&&l_sum>l_max)
			{
				l_max=l_sum;
				l_x=l_c*S;
				l_y=l_r*S;
			}
			
			/*
			if (l_sum>l_max)
			{
				l_max=l_sum;
				l_x=l_c*S;
				l_y=l_r*S;
			}
			*/



			shift_right(ap_img,lp_pix_col,lp_pix_tmp,a_width,l_r,l_c,tsize);
		}
		shift_down(ap_img,lp_pix_row,lp_pix_tmp,a_width,l_r,0,tsize);
		shift_copy(lp_pix_row,lp_pix_col,m_elem);
	}
	*pX=l_x;
	*pY=l_y;

EXT:
	FreeVectMem(hMemMgr,lp_pix_col);
	FreeVectMem(hMemMgr,lp_pix_tmp);
	FreeVectMem(hMemMgr,lp_pix_row);
	return res;

}
/*
HI_MPI_IVE_DMA
ap_pix_old:源数据指针
ap_pix_new:数据指针
num：数据长度
功能：数据的copy
*/
MVoid shift_copy(MUInt8 * ap_pix_old, MUInt8 * ap_pix_new ,MLong num)
{
	memcpy(ap_pix_new,ap_pix_old,num);
	//HISI_copy(ap_pix_old,ap_pix_new,num);
}
MVoid shift_right(	MUInt8 * ap_img, MUInt8 * ap_list,MUInt8*ap_tmp,MLong a_width, MLong a_row, MLong a_col,TSize tsize )
{
	MLong N=tsize.lWidth;
	MLong M=tsize.lHeight;
	MLong m_elem=N*M;
	MLong l_off2 = a_col+N+a_width*a_row;
	MLong l_off1 = N-1;
	MLong l_i;

	shift_copy(ap_list+1,ap_tmp,m_elem-1);

	for( l_i=0; l_i<M; ++l_i )
	{
		ap_tmp[l_off1] = ap_img[l_off2];

		l_off2+=a_width;
		l_off1+=N;
	}
	shift_copy(ap_tmp,ap_list,m_elem);
}
MVoid shift_down(MUInt8 * ap_img, MUInt8 * ap_list,MUInt8 *ap_tmp,MLong a_width, MLong a_row, MLong a_col ,TSize tsize)
{
	MLong N=tsize.lWidth;
	MLong M=tsize.lHeight;
	MLong m_elem=N*M;
	MLong l_off = a_col+a_width*(a_row+M);
	MLong l_sta = N*(M-1);
	MLong l_end = l_sta+N;
	MLong l_i;

	//shift_copy(ap_list+N,ap_list,m_elem-N);
	shift_copy(ap_list+N,ap_tmp,m_elem-N);

	for( l_i=l_sta; l_i<l_end; ++l_i )
	{
		ap_tmp[l_i] = ap_img[l_off];

		++l_off;
	}
	shift_copy(ap_tmp,ap_list,m_elem);
}
MVoid shift_init(MUInt8 *ap_img,MUInt8* ap_pix_new,MLong lWidth, MLong lHeight ,TSize tsize)
{
	MLong lx,ly;
	for (ly=0;ly<tsize.lHeight;ly++)
	{
		for (lx=0;lx<tsize.lWidth;lx++)
		{
			*(ap_pix_new+ly*tsize.lWidth+lx)=*(ap_img+ly*lWidth+lx);
		}
	}
	
}
MRESULT ComputeGradients(MHandle hMemMgr,BLOCK *pImage,MLong lSample,MUInt8 **GradientData)
{
	MRESULT res = LI_ERR_NONE;
	MUInt16 m_min_norm=10;
	MLong l_r,l_c;
	MLong S=lSample;

	MLong l_off_x=1;
	MLong l_off_y=1;
	MLong m_bins=8;

	MLong l_mstep=pImage->lBlockLine;
	//MLong l_mstep = lp_sobel_mg->widthStep/sizeof(float);
	//MLong l_istep = ap_img->widthStep/sizeof(float);

	MDouble l_divisor = 12.0/((m_bins-1));

	MDouble l_gmax_gra=0;
	MDouble l_lmax_gra=0;
	MLong l_inx,l_iny;

	MUInt32 *lp_strn_ptr=MNull;
	MUInt32 *lp_strn_tmp=MNull;
	MUInt8 *lp_peak_ptr=MNull;
	MUInt8 *lp_peak_tmp=MNull;
	MInt16 *pImgX=NULL;
	MInt16 *pImgY=NULL;
	MUInt16 *pMag=NULL;
	MUInt16 *pAngle=NULL;
	MInt16 *pxcord=MNull;
	MInt16 *pycord=MNull;
	MUInt16 *pvalcord=MNull;
	MLong ii,jj;

	MLong l_ang1;
	MLong l_bin1;
	MUInt16 l_max=0;
	MLong l_height;
	MLong l_width;
	TSize l_size;
	l_size.lHeight=S;
	l_size.lWidth=S;


	AllocVectMem(hMemMgr,pImgX,(pImage->lBlockLine)*(pImage->lHeight),MInt16);
	AllocVectMem(hMemMgr,pImgY,(pImage->lBlockLine)*(pImage->lHeight),MInt16);
	AllocVectMem(hMemMgr,pMag,(pImage->lBlockLine)*(pImage->lHeight),MUInt16);
	AllocVectMem(hMemMgr,pAngle,(pImage->lBlockLine)*(pImage->lHeight),MUInt16);
	memset(pMag,0,(pImage->lBlockLine)*(pImage->lHeight)*sizeof(MUInt16));
	/*
	HI_MPI_IVE_SOBEL
	pImage->pBlockData为源数据指针，
	pImgX为水平方向输出数据指针
	pImgY为竖直方向输出数据指针，
	功能为获取梯度信息，
	pMag为梯度值大小，pAngle为梯度角度信息。
	梯度值大小与梯度角度信息在HI_MPI_IVE_SOBEL之后再计算
	*/
	HISI_createGradAngle(pImage->pBlockData,pImage->lBlockLine,pImage->lWidth,pImage->lHeight,pImgX,pImgY,pMag,pAngle);

	l_height=(pImage->lHeight-2)/S;
	l_width=(pImage->lWidth-2)/S;

	//WriteMInt16Data("D:\\imggx.txt",pImgX,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMInt16Data("D:\\imggy.txt",pImgY,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMUInt16Data("D:\\imgMag.txt",pMag,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);
	//WriteMUInt16Data("D:\\imgAngle.txt",pAngle,pImage->lBlockLine,pImage->lWidth,pImage->lHeight);


	//AllocVectMem(hMemMgr,lp_strn_tmp,l_height*l_width,MUInt32);
	AllocVectMem(hMemMgr,lp_peak_tmp,l_height*l_width,MUInt8);
	//AllocVectMem(hMemMgr,pxcord,a_grad_num,MInt16);
	//AllocVectMem(hMemMgr,pycord,a_grad_num,MInt16);
	//AllocVectMem(hMemMgr,pvalcord,a_grad_num,MDouble);


	memset(lp_peak_tmp,0,l_width*l_height);
	lp_strn_ptr=lp_strn_tmp;
	lp_peak_ptr=lp_peak_tmp;

	for(  l_r=0; l_r<l_height; ++l_r )
	{
		for( l_c=0; l_c<l_width; ++l_c )
		{
			MaxIndx_32f_C1R(pMag+l_off_y*l_mstep+l_off_x,l_mstep,l_size,&l_max,&l_inx,&l_iny);
			if (l_max<m_min_norm)
			{
				*lp_peak_ptr |= 1 << (m_bins-1);
			}
			else
			{
				l_ang1 = *(pAngle+(l_iny+l_off_y)*l_mstep+l_inx+l_off_x);
				l_bin1 = (MUInt8)((l_ang1>=12?l_ang1-12:l_ang1)/l_divisor);
				//l_bin1 = (MUInt8)(l_ang1/l_divisor);

				*lp_peak_ptr|=1<<l_bin1;
			}
			++lp_peak_ptr;
			l_off_x += S;
		}
		l_off_y += S;
		l_off_x = 1;
	}
	//WriteMUInt8Data("D:\\imgData.txt",lp_peak_tmp,);
	*GradientData=lp_peak_tmp;
	//cannyFile= fopen("D:\\opeateImg.txt", "w");
	

	for(ii=0;ii<l_height;ii++)
	{
		for(jj=0;jj<l_width;jj++)
		{
			//fprintf(cannyFile, "%6d ", *lp_peak_tmp);
			lp_peak_tmp++;
		}
		//fprintf(cannyFile, "\n");
	}
	//fclose(cannyFile);
	//------------------------------------------------------------------------------
	//WriteMUInt8Data("D:\\imgData.txt",lp_peak_tmp,l_width,l_width,l_height);
	//for (x=0;)

EXT:
	if(res==LI_ERR_ALLOC_MEM_FAIL)
	{
		FreeVectMem(hMemMgr,lp_peak_tmp);
	}
	//FreeVectMem(hMemMgr,lp_strn_tmp);
	FreeVectMem(hMemMgr,pImgX);
	FreeVectMem(hMemMgr,pImgY);
	FreeVectMem(hMemMgr,pMag);
	FreeVectMem(hMemMgr,pAngle);
	return res;
	//FreeVectMem(hMemMgr,pxcord);
	//FreeVectMem(hMemMgr,pycord);
	//FreeVectMem(hMemMgr,pvalcord);
}

MRESULT MaskTemplate(MHandle hMemMgr,TDescriptor *pDescriptor,BLOCK *pMask,MRECT rect)
{
	MLong x,y;
	MLong lHeight=rect.bottom-rect.top+1;
	MLong lWidth=rect.right-rect.left+1;
	BLOCK MaskTmp={0};
	BLOCK Maskoperate={0};
	MRESULT res=LI_ERR_NONE;
	MLong lSum=0;

	MLong lOperateWidth=pDescriptor->lWidth;
	MLong lOperateHeight=pDescriptor->lHeight;
	MUInt8 *pMaskData,*pOperateData,*pData;

	MLong lMaskExt,lOperateExt;
	//MLong lImgTmp,lSrcTmp;
	//B_Create(hMemMgr,&MaskTmp,DATA_U8,lWidth,lHeight);
	B_Create(hMemMgr,&Maskoperate,DATA_U8,lOperateWidth,lOperateHeight);
	lMaskExt=pMask->lBlockLine-lWidth;
	//lTmpExt=MaskTmp.lBlockLine-lWidth;
	lOperateExt = Maskoperate.lBlockLine-lOperateWidth;

	pMaskData=(MUInt8*)(pMask->pBlockData)+rect.top*(pMask->lBlockLine)+rect.left;
	//pSrcData = (MUInt8*)(MaskTmp.pBlockData);
	pOperateData = (MUInt8 *)(Maskoperate.pBlockData);
	pData = pDescriptor->pData;

	/*
	for (y=rect.top;y<=rect.bottom;y++,pImageData+=lMaskExt,pSrcData+=lExt)
	{
		for (x=rect.left;x<=rect.right;x++,pImageData++,pSrcData++)
		{
			*pImageData=*pSrcData;
		}
	}
	*/
	Resize(pMaskData,pMask->lBlockLine,lWidth,lHeight,Maskoperate.pBlockData,Maskoperate.lBlockLine,Maskoperate.lWidth,Maskoperate.lHeight,BILINEAR);

	for (y=0;y<Maskoperate.lHeight;y++,pOperateData+=lOperateExt)
	{
		for(x=0;x<Maskoperate.lWidth;x++,pOperateData++,pData++)
		{
			if (*pOperateData==0)
			{
				*pData=0;
				lSum++;
			}
		}
	}
	pDescriptor->lRegion=pDescriptor->lHeight*pDescriptor->lWidth-lSum;

//EXT:
	B_Release(hMemMgr,&Maskoperate);
	return res;
}

MRESULT MatchRigidBody(MHandle hMemMgr,BLOCK *pImage,PTDescriptorClass ptDescriptorClass,MDouble threshold,PTOutParam ptOutParam)
{
	MRESULT res = LI_ERR_NONE;
	BLOCK imageMean={0};
	BLOCK imageEdge={0};
	MLong lX=0,lY=0;
	TDescriptor tDescriptor={0};
	MLong i;
	MUInt8 *pImageData=MNull;
	MLong lSample=5;
	MLong timeStart1,timeEnd1,timeOfMatch1;
	MLong timeStart2,timeEnd2,timeOfMatch2;
	MLong lDescripIndex=0;

	ptOutParam->lX=-1;
	ptOutParam->lY=-1;

	GO(B_Create(hMemMgr,&imageMean,DATA_U8,pImage->lWidth,pImage->lHeight));

	/*
	HI_MPI_IVE_FILTER 
	pImage->pBlockData为源数据指针
	imageMean.pBlockData为输出数据指针
	功能为平滑处理
	IVE_SRC_INFO_S 
	*/
	//IVE_SRC_INFO_S pSrc;
	GO(SmoothBlock(hMemMgr,
		pImage->pBlockData,pImage->lBlockLine,pImage->typeDataA,
		imageMean.pBlockData,imageMean.lBlockLine,imageMean.typeDataA,
		imageMean.lWidth,imageMean.lHeight,4));

	GO(B_Create(hMemMgr,&imageEdge,DATA_U8,pImage->lWidth,pImage->lHeight));
	//PrintBmpEx(imageMean.pBlockData, imageMean.lBlockLine,imageMean.typeDataA & ~0x100,imageMean.lWidth, imageMean.lHeight, 1, "D:\\imageMean.bmp");
	//GO(Edge(hMemMgr,pImage->pBlockData,pImage->lBlockLine,pImage->lWidth,pImage->lHeight,imageEdge.pBlockData,imageEdge.lBlockLine,TYPE_CANNY));
	//PrintBmpEx(imageEdge.pBlockData, imageEdge.lBlockLine,imageEdge.typeDataA & ~0x100,imageEdge.lWidth, imageEdge.lHeight, 1, "D:\\imageEdge.bmp");

	timeStart1=JGetCurrentTime();	
	GO(ComputeGradients(hMemMgr,&imageMean,lSample,&pImageData));
	timeEnd1=JGetCurrentTime();
	timeOfMatch1=timeEnd1-timeStart1;
	timeStart2=JGetCurrentTime();	
	for (i=0;i<ptDescriptorClass->lNumDescriptors;i++)
	{
		tDescriptor=*(ptDescriptorClass->ptDescriptor+i);
		lSample=tDescriptor.lSample;
		
		GO(MatchProcess(hMemMgr,pImageData,(tDescriptor.lRegion*threshold),(imageMean.lWidth-2)/lSample,(imageMean.lHeight-2)/lSample,&tDescriptor,&lX,&lY));
		if (lX!=-1&&lY!=-1)
		{
			//DrawLine(pImage,lX,lY,(tDescriptor.lWidth+1)*tDescriptor.lSample,(tDescriptor.lHeight+1)*tDescriptor.lSample);
			//PrintBmpEx(pImage->pBlockData, pImage->lBlockLine,pImage->typeDataA & ~0x100,pImage->lWidth, pImage->lHeight, 1, "D:\\result2.bmp");
			ptOutParam->lAngle=tDescriptor.lAngle;
			ptOutParam->lX=lX+(tDescriptor.lWidth)*tDescriptor.lSample/2;
			ptOutParam->lY=lY+(tDescriptor.lHeight)*tDescriptor.lSample/2;
			ptOutParam->lWidth=(tDescriptor.lWidth)*tDescriptor.lSample;
			ptOutParam->lHeight=(tDescriptor.lHeight)*tDescriptor.lSample;
			ptOutParam->lIndex=tDescriptor.lDescriptorIndex;
			//Edge(hMemMgr,pImage->pBlockData,pImage->lBlockLine,pImage->lWidth,pImage->lHeight,)
			//k=1;


			break;
		}

	}
	timeEnd2=JGetCurrentTime();
	timeOfMatch2=timeEnd2-timeStart2;
	if (lX==-1||lY==-1)
	{
		res=LI_ERR_NO_FIND;
	}
	//MatchRigidBody(hMemMgr,pImageData,1000,100,100,&tDescriptor);
	
EXT:
	B_Release(hMemMgr,&imageMean);
	B_Release(hMemMgr,&imageEdge);
	FreeVectMem(hMemMgr, pImageData);
	return res;

}

MVoid GetTemp(MHandle hMemMgr,BLOCK *pImage,TOutParam tOutParam)
{
	MRECT rect={0};
	MRESULT res=0;
	BLOCK RectImg={0};
	BLOCK edgeImg={0};
	BLOCK edgeImg2={0};
	BLOCK rotateImg={0};
	rect.bottom=tOutParam.lY+tOutParam.lHeight/2;
	rect.top=tOutParam.lY-tOutParam.lHeight/2;
	rect.left=tOutParam.lX-tOutParam.lWidth/2;
	rect.right=tOutParam.lX+tOutParam.lWidth/2;
	GetRectImage(hMemMgr,pImage,&RectImg,rect);
	PrintBmpEx(RectImg.pBlockData, RectImg.lBlockLine,RectImg.typeDataA & ~0x100,RectImg.lWidth, RectImg.lHeight, 1, "D:\\rect.bmp");
	//GO(B_Create(hMemMgr,&edgeImg,))
	GO(B_Create(hMemMgr,&edgeImg,DATA_U8,RectImg.lWidth,RectImg.lHeight));
	Edge(hMemMgr,RectImg.pBlockData,RectImg.lBlockLine,RectImg.lWidth,RectImg.lHeight,edgeImg.pBlockData,edgeImg.lBlockLine,TYPE_CANNY);
	PrintBmpEx(edgeImg.pBlockData, edgeImg.lBlockLine,edgeImg.typeDataA & ~0x100,edgeImg.lWidth, edgeImg.lHeight, 1, "D:\\edge2.bmp");
	RotateImage(hMemMgr,&RectImg,&rotateImg,360-tOutParam.lIndex*10);
	PrintBmpEx(rotateImg.pBlockData, rotateImg.lBlockLine,rotateImg.typeDataA & ~0x100,rotateImg.lWidth, rotateImg.lHeight, 1, "D:\\rotate2.bmp");
	GO(B_Create(hMemMgr,&edgeImg2,DATA_U8,rotateImg.lWidth,rotateImg.lHeight));
	Edge(hMemMgr,rotateImg.pBlockData,rotateImg.lBlockLine,rotateImg.lWidth,rotateImg.lHeight,edgeImg2.pBlockData,edgeImg2.lBlockLine,TYPE_CANNY);
	PrintBmpEx(edgeImg2.pBlockData, edgeImg2.lBlockLine,edgeImg2.typeDataA & ~0x100,edgeImg2.lWidth, edgeImg2.lHeight, 1, "D:\\edge3.bmp");

	
EXT:
	B_Release(hMemMgr,&RectImg);
	B_Release(hMemMgr,&edgeImg);
	B_Release(hMemMgr,&edgeImg2);
	B_Release(hMemMgr,&rotateImg);






}
MRESULT GetRectImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MRECT rect)
{
	BLOCK pImage={0};
	MLong x,y;
	MLong lHeight=rect.bottom-rect.top+1;
	MLong lWidth=rect.right-rect.left+1;
	MUInt8 *pImageData;
	MUInt8 *pSrcData=(MUInt8*)(pSrc->pBlockData);
	MRESULT res = LI_ERR_NONE;
	//MLong lOperateImageWidth=lSample*(lWidth/lSample+1)+2;
	//MLong lOperateImageHeight=lSample*(lHeight/lSample+1)+2;
	MLong lImgExt,lSrcExt;

	GO(B_Create(hMemMgr,&pImage,DATA_U8,lWidth,lHeight));
	//GO(B_Create(hMemMgr,pDst,DATA_U8,lOperateImageWidth,lOperateImageHeight));

	pSrcData+=rect.top*(pSrc->lBlockLine)+rect.left;//原图的矩形框左上角的数据
	pImageData=(MUInt8*)(pImage.pBlockData);
	lSrcExt=pSrc->lBlockLine-lWidth;//由于需获取矩形框中的图像信息，所以其有效的长度为lWidth
	lImgExt=pImage.lBlockLine-lWidth;
	//lImgTmp=0;
	//lSrcTmp=0;

	for (y=rect.top;y<=rect.bottom;y++,pImageData+=lImgExt,pSrcData+=lSrcExt)
	{
		for (x=rect.left;x<=rect.right;x++,pImageData++,pSrcData++)
		{
			*pImageData=*pSrcData;
		}
	}

	//PrintBmpEx(pImage.pBlockData, pImage.lBlockLine,pImage.typeDataA & ~0x100,pImage.lWidth, pImage.lHeight, 1, "D:\\operate.bmp");
	//Resize(pImage.pBlockData,pImage.lBlockLine,pImage.lWidth,pImage.lHeight,pDst->pBlockData,pDst->lBlockLine,pDst->lWidth,pDst->lHeight,BILINEAR);
	//PrintBmpEx(pDst->pBlockData,pDst->lBlockLine,pDst->typeDataA& ~0x100,pDst->lWidth,pDst->lHeight,1,"D:\\operate2.bmp");
	*pDst=pImage;


EXT:
	//B_Release(hMemMgr,&pImage);
	return res;


}