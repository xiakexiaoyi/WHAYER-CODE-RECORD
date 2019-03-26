#include <stdio.h>
#include <stdlib.h>
#include "HYL_disconnector.h"
#include "licommon\lidebug.h"
#include "image\liimage.h"
#include "licommon\lierrdef.h"
extern "C"
{
#include "disconnector\disconnector_SqueezeNet_S1.h"
}
#include "resample.h"

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



MRESULT HYL_disconnectorReg(MHandle hMRHandle, HYL_IMAGES *pImage,HYED_RESULT_LIST *pResultList)
{
	MRESULT res = 0;
	MDWord i;
	JOFFSCREEN img={0};
	JOFFSCREEN ResizeImg={0};
	MRECT originArea;
	MRECT rtArea;
	int label;
	//convert ED_IMAGES to inner image format
	img = _TransToInteriorImgFmt(pImage);
	//TLParam* pTLParam = (TLParam*)hMRHandle;
	GO(ImgCreate(NULL, &ResizeImg, FORMAT_BGR, 227, 227));
	for(i=0;i<pResultList->lAreaNum;i++)
	{
		JOFFSCREEN imgT = img;
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
		{
			res = -1;
			continue;
		}

		ImgOffset(&imgT,rtArea.left,rtArea.top);
        imgT.dwWidth=rtArea.right-rtArea.left;
		imgT.dwHeight=rtArea.bottom-rtArea.top;

		//PrintBmpEx(imgT.pixelArray.chunky.pPixel,imgT.pixelArray.chunky.dwImgLine,DATA_U8,imgT.dwWidth, imgT.dwHeight, 3, "..\\imgT.bmp");
		
		
		ResampleImage(NULL,imgT.dwWidth,imgT.dwHeight,imgT.pixelArray.chunky.dwImgLine,ResizeImg.dwWidth,ResizeImg.dwHeight,ResizeImg.pixelArray.chunky.dwImgLine,(unsigned char*)imgT.pixelArray.chunky.pPixel,(unsigned char*)ResizeImg.pixelArray.chunky.pPixel,0);
		
		//PrintBmpEx(ResizeImg.pixelArray.chunky.pPixel,ResizeImg.pixelArray.chunky.dwImgLine,DATA_U8,ResizeImg.dwWidth, ResizeImg.dwHeight, 3, "..\\ResizeImg.bmp");
		label=-1;
		//LightOff_SqueezeNet_S1_run_C3((unsigned char*)imgT.pixelArray.chunky.pPixel,imgT.dwWidth,ResizeImg.dwHeight,imgT.pixelArray.chunky.dwImgLine,&label);
		//LightOff_SqueezeNet_S1_run_C3((unsigned char*)ResizeImg.pixelArray.chunky.pPixel,ResizeImg.dwWidth,ResizeImg.dwHeight,ResizeImg.pixelArray.chunky.dwImgLine,&label);
		disconnector_SqueezeNet_S1_run_C3((unsigned char*)ResizeImg.pixelArray.chunky.pPixel,ResizeImg.dwWidth,ResizeImg.dwHeight,ResizeImg.pixelArray.chunky.dwImgLine,&label);
		//printf("label=%d\n",label);
		pResultList->pResult[i].result=label;
		/*if(label==0)
			pResultList->pResult[i].result=2;
		else if(label==1)
			pResultList->pResult[i].result=1;*/
		//lightcompare(&img,&rtArea,&pResultList->pResult[i].result);
	}

EXT:
	ImgRelease(NULL,&ResizeImg);
	return res;
}
