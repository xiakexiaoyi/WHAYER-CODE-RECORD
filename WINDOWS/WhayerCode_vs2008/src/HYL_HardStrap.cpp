#include <stdio.h>
#include <stdlib.h>
#include "HYL_HardStrap.h"
#include <opencv2\opencv.hpp>
#include "licommon\lidebug.h"
#include "imgfmt_trans\liimgfmttrans.h"
#include "image\liimage.h"
#include "licommon\lierrdef.h"
#include "ExceptionDetection\GaborSVM.h"
#include "licommon\limem.h"

#define pi 3.1415926

static JOFFSCREEN _TransToInteriorImgFmt(const HYL_IMAGES* pImg)
{
    JOFFSCREEN img = *(JOFFSCREEN*)pImg;
    switch(img.fmtImg)
    {
    case HYL_IMAGE_GRAY:
        img.fmtImg = FORMAT_GRAY;
        break;
    case HYL_IMAGE_YUYV:
        img.fmtImg = FORMAT_YUYV;
        break;
    case HYL_IMAGE_RGB:
        img.fmtImg = FORMAT_RGB;
        break;
    case HYL_IMAGE_BGR:
        img.fmtImg = FORMAT_BGR;
        break;
    case HYL_IMAGE_YUV420:
        img.fmtImg = FORMAT_YUV420;
		break;
	case HYL_IMAGE_HUE:
		img.fmtImg = FORMAT_GRAY;
		break;
	case HYL_IMAGE_YUV:
		img.fmtImg = FORMAT_YUV;
		break;
    default:
        JASSERT(MFalse);
        break;
    }
    JASSERT(IF_DATA_BYTES(img.fmtImg) == 1);
    return img;
}

typedef struct tag_TLHandle
{
	MHandle hMemMgr;
	Gsvminfo *gaborsvm;
}TLParam;
int CreateGabor1(CvGabor *gabors[][8])
{
	double Sigma = pi; 
    double F= sqrt(2.0); 
    double n=-3.0 ; 
	for (int j=0;j<3;j++) 
   { 
       for (int i=0;i<8;i++) 
       { 
           gabors[j][i] = new CvGabor((pi*i)/8,n,Sigma,F); 
       } 
       n=n+1; 
   }
	return 0;
}
int ReleaseGabor1(CvGabor *gabors[][8])
{
	///É¾³ý·ÖÅäµÄgaborÄÚ´æ
     for ( int j=0;j<3;j++)
       {
           for ( int s=0;s<8;s++)
           {
               delete(gabors[j][s]);
           }   
       }
	 return 0;
}
MRESULT HYL_HardStrapInit(MHandle hMemMgr, MHandle *pMRHandle)
{
	MRESULT res = LI_ERR_NONE;
	TLParam *pTLHandle = NULL;

	if (!pMRHandle) return -1;
	AllocVectMem(hMemMgr, pTLHandle,1, TLParam);
	SetVectMem(pTLHandle, 1, 0, TLParam);
	pTLHandle->hMemMgr=hMemMgr;
	AllocVectMem(hMemMgr, pTLHandle->gaborsvm,1, Gsvminfo);
    pTLHandle->gaborsvm->gaborsize=10;
	pTLHandle->gaborsvm->imgpatch= cvCreateImage(cvSize(100,100), 8, 1);
	pTLHandle->gaborsvm->imggabora= cvCreateImage(cvSize(pTLHandle->gaborsvm->gaborsize,pTLHandle->gaborsvm->gaborsize), 8, 1);
	pTLHandle->gaborsvm->imggaborb= cvCreateImage(cvSize(pTLHandle->gaborsvm->gaborsize,pTLHandle->gaborsvm->gaborsize), 8, 1);
	CreateGabor1(pTLHandle->gaborsvm->gabors);	
	*pMRHandle = pTLHandle;
	JPrintf("HYED_Init()\n");
EXT:
	if (res<0)
	{
		FreeVectMem(hMemMgr, pTLHandle);
	}
	return res;
}

MRESULT   HYL_HardStrapUninit(MHandle hMRHandle)
{
	TLParam *pTLHandle = (TLParam*)hMRHandle;
	if (pTLHandle == MNull) return -1;
	svm_free_and_destroy_model(&pTLHandle->gaborsvm->model);
	cvReleaseImage(&pTLHandle->gaborsvm->imgpatch);
	cvReleaseImage(&pTLHandle->gaborsvm->imggabora);
	cvReleaseImage(&pTLHandle->gaborsvm->imggaborb);
	ReleaseGabor1(pTLHandle->gaborsvm->gabors);
	FreeVectMem(pTLHandle->hMemMgr, pTLHandle->gaborsvm);
	FreeVectMem(pTLHandle->hMemMgr, pTLHandle);
	return 1;
}

//test parameters 
MRESULT HYL_HardStrapSetParam(MHandle hMRHandle,char *modelfile,char *Filters0file,char *Filters1file)
{
	MRESULT res = 1;
	TLParam *pTLHandle = (TLParam*)hMRHandle;
	pTLHandle->gaborsvm->model=svm_load_model(modelfile);
	
EXT:
	return res;
}


MRESULT HYL_HardStrapExceptionDetection(MHandle hMRHandle, HYL_IMAGES *pImage,HYED_RESULT_LIST *pResultList)
{
	MDWord i, j, k;
	MLong lCurTime;
	MLong lMeanVal=0;
	MRESULT res = 0;
	JOFFSCREEN img={0},img1={0};
	JOFFSCREEN WarpImg={0};
	JOFFSCREEN ResizeImg={0};
	JOFFSCREEN imgCR ={0};
	JOFFSCREEN imgYuv={0},WarpImgYuv ={0};
	JOFFSCREEN imgGray ={0},WarpImgGray={0};
	
	//UCImage img2Sift;
	//IntImage vx, vy;

	MLong cellSize;
	JMASK ChangeMask = {0};
	//IntImage mask;
	//DImage Mag;
	int maxval, offset;

	//UCImage InnerImg1, InnerImg2, warpImg;
	//FImage fInnerImg1, fWarpImg;
	//UInt32Image segment1, segmentWarp;
	
	TLParam* pTLParam = (TLParam*)hMRHandle;
	Gsvminfo *gaborsvm = pTLParam->gaborsvm;
	///input parameter check
	if (!pTLParam ||!pImage){res = LI_ERR_INVALID_PARAM; goto EXT;}
//	JASSERT(pImage->lHeight<=pImage2->lHeight && pImage->lWidth<=pImage2->lWidth);
	
	//convert ED_IMAGES to inner image format
	img = _TransToInteriorImgFmt(pImage);
	//img1 = _TransToInteriorImgFmt(pImage);
	//GO(ImgCreate(pTLParam->hMemMgr, &WarpImg, FORMAT_BGR, img1.dwWidth, img1.dwHeight));
	//WarpImg = img1;
	
	/*GO(ImgCreate(pTLParam->hMemMgr, &imgYuv, FORMAT_YUYV, img.dwWidth, img.dwHeight));
	GO(ImgCreate(pTLParam->hMemMgr, &WarpImgYuv, FORMAT_YUYV, img.dwWidth, img.dwHeight));
	GO(ImgFmtTrans(&img, &imgYuv));
	GO(ImgFmtTrans(&WarpImg, &WarpImgYuv));*/
	GO(ImgCreate(pTLParam->hMemMgr, &imgGray, FORMAT_GRAY, img.dwWidth, img.dwHeight));
	//GO(ImgCreate(pTLParam->hMemMgr, &WarpImgGray, FORMAT_GRAY, img1.dwWidth, img1.dwHeight));
	GO(ImgFmtTrans(&img, &imgGray));
	//GO(ImgFmtTrans(&WarpImg, &WarpImgGray));
	MRECT originArea;
	MRECT rtArea;
	for(int i=1;i<pResultList->lAreaNum;i++)
	{
		originArea.bottom = pResultList->DtArea[i].bottom;
		originArea.left = pResultList->DtArea[i].left;
		originArea.top = pResultList->DtArea[i].top;
		originArea.right = pResultList->DtArea[i].right;
		rtArea.bottom = pResultList->DtArea[i].bottom+pResultList->offset.y;
		rtArea.left = pResultList->DtArea[i].left+pResultList->offset.x;
		rtArea.top = pResultList->DtArea[i].top+pResultList->offset.y;
		rtArea.right = pResultList->DtArea[i].right+pResultList->offset.x;
		if(rtArea.bottom == img.dwHeight)rtArea.bottom=rtArea.bottom-1;
		if(rtArea.right == img.dwWidth)rtArea.right=rtArea.right-1;
		if(rtArea.bottom >= img.dwHeight || rtArea.left < 0 || rtArea.top < 0 || rtArea.right >= img.dwWidth)
		{
			res = -2;
			continue;
		}
		if((originArea.right-originArea.left)<=0 || (originArea.bottom-originArea.top)<=0 )
			continue;
		GaborSVM(pTLParam->hMemMgr,gaborsvm,imgGray,&rtArea,&pResultList->pResult[i].result);
		//PCANET(pTLParam->hMemMgr,pPCANETSVM->svm,pPCANETSVM->Filters0,pPCANETSVM->Filters1,pPCANETSVM->pcaNet,imgGray,&rtArea,&pResultList->pResult[i].result);
		//if(pResultList->DtArea[i].Type == 1)
		//{
		//	pResultList->pResult[i].Type = 1;
		//	/*GO(ImgCreate(pTLParam->hMemMgr, &ResizeImg, FORMAT_BGR, rtArea.right-rtArea.left, rtArea.bottom-rtArea.top));
		//	Resize(WarpImg.pixelArray.chunky.pPixel,WarpImg.pixelArray.chunky.dwImgLine,WarpImg.dwWidth,WarpImg.dwHeight,
		//		ResizeImg.pixelArray.chunky.pPixel,ResizeImg.pixelArray.chunky.dwImgLine,ResizeImg.dwWidth,ResizeImg.dwHeight,POINTSAMPLE);*/
		//	JOFFSCREEN imgT = img;
		//	JOFFSCREEN WarpImgT = WarpImg/*ResizeImg*/;
		//	ImgOffset(&imgT,pResultList->DtArea[i].left,pResultList->DtArea[i].top);
		//	PrintBmpEx(imgT.pixelArray.chunky.pPixel,imgT.pixelArray.chunky.dwImgLine,DATA_U8,pResultList->DtArea[i].right
		//		-pResultList->DtArea[i].left, pResultList->DtArea[i].bottom-pResultList->DtArea[i].top, 3, "..\\ColorChar1.bmp");
		//	PrintBmpEx(WarpImgT.pixelArray.chunky.pPixel,WarpImgT.pixelArray.chunky.dwImgLine,DATA_U8, 
		//		WarpImgT.dwWidth, WarpImgT.dwHeight, 3, "..\\ColorChar2.bmp");
		//	lightcompare2(&img,&WarpImg,&originArea,&rtArea,&pResultList->pResult[i].result);
		//}
		//else if(pResultList->DtArea[i].Type == 2)
		//{
		//	pResultList->pResult[i].Type = 2;
		//	PCANET(pTLParam->hMemMgr,pPCANETSVM->svm,pPCANETSVM->Filters0,pPCANETSVM->Filters1,pPCANETSVM->pcaNet,imgGray,WarpImgGray,&originArea,&rtArea,&pResultList->pResult[i].result);
		//	
		//}
	}
	/*ImgRelease(pTLParam->hMemMgr,&imgYuv);
	ImgRelease(pTLParam->hMemMgr,&WarpImgYuv);*/
	ImgRelease(pTLParam->hMemMgr,&imgGray);
	//ImgRelease(pTLParam->hMemMgr,&WarpImgGray);
	
EXT:
	//ImgRelease(pTLParam->hMemMgr,&imgYuv);
	
	//MaskRelease(MNull, &ChangeMask);
	return res;
}
