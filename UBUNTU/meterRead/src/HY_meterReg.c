#include <stdio.h>
#include <string.h>

#include"litimer.h"
#include"liimage.h"
#include"liimgfmttrans.h"
#include "liedge.h"
#include "lidebug.h"
#include "bbgeometry.h"
#include "math.h"
#include"lineSegment_ex.h"
#include"MatchRigidBody.h"
#include"liimage.h"
#include "limath.h"
#include "Location.h"

#include "lisort.h"
#include "licomdef.h"

#include "HYAMR_meterReg.h"
#include "HaarResponse.h"

#define WORK_BUFFER 512*384*100
#define MAX_DESCRITPTOR_NUMBER 1000             //һ�����е����������������
#define MAX_CLASS_NUMBER 100                    //����������������������



#define MAX_RESULT_LEN 3

typedef struct  
{
	MHandle	hMemMgr;
	TDescriptorsGroup tDescriptorsGroup;

	MLong lNumPts;				//restriction 1
	//MLong lMaxArcAngle;			//restriction 2

	MBool bExistCircle;				//use to indicate whether circle exist?
	CIRCLE_INFO AmmeterCircle;	//if exist, store the parameters

	//xsd 2015.12.17
	CIRCLE outerCirclePara;
	CIRCLE innerCirclePara;
	MLong lLineYPos;

	//xsd
	MRECT ptTarget;
	MRECT ptTrainTarget;
	MBool bRefresh;
}AMMETERINFO,*PAMMETERINFO;	




static MVoid TransGrayImgToBlock(JOFFSCREEN Image,PBLOCK pBlockImg);
static JOFFSCREEN _TransToInteriorImgFmt(HYLPAMR_IMAGES pImg);
static MLong GetClassIndex(PTDescriptorsGroup ptDescriptorsGroup,MChar *name);
static MRESULT AddClasses(MHandle hMemMgr,PTDescriptorsGroup ptDescriptorsGroup,MChar *name);
static MVoid UpdateTDescriptor(PTDescriptorClass ptDescriptorClass,MLong index);
static MVoid UpdateTDescriptorNumber(PTDescriptorClass ptDescriptorClass,MLong index);
static MVoid ReadParaInfoCpy(PARAM_INFO Param, HYAMR_READ_PARA *pDst);

static MVoid WriteMem(unsigned char *pInPut, unsigned char *pOutPut, int lSize);

//============================================================
#define MAX_LOOP_NUM 1
MLong lLoopNum = 0;
//============================================================


//===============================================================
// �ڴ濽������pInPut��������ȡlSize���ֽ���Ϣ��pOutPut��
MVoid WriteMem(unsigned char *pInPut, unsigned char *pOutPut, int lSize)
{
	int i;
	unsigned char *pInData = NULL;
	unsigned char *pOutData = NULL;

	pInData = pInPut;
	pOutData = pOutPut;
	for (i=0; i<lSize; i++)
	{
		*pOutData = *pInData;
		pOutData++;
		pInData++;
	}
}
/*===========================================================
������	��_TransToInteriorImgFmt
����		�����ⲿͼƬ��ʽתΪ�ڲ�ͼƬ��ʽ
����ȫ�ֱ�������
�������˵����pImg �ⲿͼƬ��ʽ��ָ��
����ֵ˵���������ڲ�ͼƬ
===========================================================*/
JOFFSCREEN _TransToInteriorImgFmt(HYLPAMR_IMAGES pImg)
{
	JOFFSCREEN img = *(JOFFSCREEN*)pImg;
	switch(img.fmtImg)
	{

	case HYAMR_IMAGE_GRAY:
		img.fmtImg = FORMAT_GRAY;
		break;
	case HYAMR_IMAGE_YUYV:
		img.fmtImg = FORMAT_YUYV;
		break;
	case HYAMR_IMAGE_RGB:
		img.fmtImg = FORMAT_RGB;
		break;
	case HYAMR_IMAGE_BGR:
		img.fmtImg = FORMAT_BGR;
		break;

#ifdef TRIM_RGB

#endif
	default:
		JASSERT(MFalse);
		break;
	}
	JASSERT(IF_DATA_BYTES(img.fmtImg) == 1);
	return img;
}
/*===========================================================
������	��TransGrayImgToBlock
����		�����Ҷ�ͼ��תΪBLOCK��ʽ��ͼ��
����ȫ�ֱ�����DATA_U8ͼƬ��ʽ
�������˵����Image �Ҷ�ͼ��
�������˵����pBLockImg �Ҷ�BLOCK
����ֵ˵������
===========================================================*/
MVoid TransGrayImgToBlock(JOFFSCREEN Image,PBLOCK pBlockImg)
{
	pBlockImg->lBlockLine=Image.pixelArray.chunky.dwImgLine;
	pBlockImg->lHeight=Image.dwHeight;
	pBlockImg->lWidth=Image.dwWidth;
	pBlockImg->typeDataA=DATA_U8;
	pBlockImg->pBlockData=Image.pixelArray.chunky.pPixel;
}

/*===========================================================
������	��HYMRB_TrainTemplateFromMask
����		������ģ���Լ�Maskͼ�����ѵ������ȡ������
����ȫ�ֱ�������
�������˵����
hHYMRDHandle�� �������������HYMRB_init�г�ʼ��
pImage�� ѵ��ģ��ͼ��
pMask��  ѵ��Maskͼ��
pClassName����ģ����������
lParma:ѵ���ķ�����0����ͼ�������ת����1��ͼ�������ת����
����ֵ˵����������������
===========================================================*/
MRESULT HYAMR_TrainTemplateFromMask (MHandle hHYMRDHandle,HYLPAMR_IMAGES pImage,HYLPAMR_IMAGES pMask,MChar *pClassName,MLong lParma)
{
	MRESULT res=LI_ERR_NONE;
	JOFFSCREEN ImgSrc  = _TransToInteriorImgFmt(pImage);
	JOFFSCREEN ImgMask = _TransToInteriorImgFmt(pMask);
	//	JOFFSCREEN ImgSrcBGR = {0};
	//	JOFFSCREEN ImgMaskBGR ={0};
	JOFFSCREEN ImgSrcGray ={0};
	JOFFSCREEN ImgMaskGray ={0};
	BLOCK BLOCKImage={0};
	BLOCK BLOCKMask={0};

	PTDescriptorClass ptDescriptorClass;
	MLong lClassIndex;

	//	MLong flag=0;
	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	MHandle hMemMgr=MNull;
	PTDescriptorsGroup ptDescriptorsGroupsTemp;

	if (hHYMRDHandle==MNull)
	{
		res=LI_ERR_NOT_INIT;
		goto EXT;
	}
	hMemMgr=pMgr->hMemMgr;
	ptDescriptorsGroupsTemp = &(pMgr->tDescriptorsGroup);

	//	GO(ImgCreate(hMemMgr,&ImgSrcBGR,FORMAT_BGR,ImgSrc.dwWidth,ImgSrc.dwHeight));
	GO(ImgCreate(hMemMgr,&ImgSrcGray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
	//	GO(ImgCreate(hMemMgr,&ImgMaskBGR,FORMAT_BGR,ImgMask.dwWidth,ImgMask.dwHeight));
	GO(ImgCreate(hMemMgr,&ImgMaskGray,FORMAT_GRAY,ImgMask.dwWidth,ImgMask.dwHeight));

	GO(ImgFmtTrans(&ImgSrc, &ImgSrcGray));
	GO(ImgFmtTrans(&ImgMask, &ImgMaskGray));
	TransGrayImgToBlock(ImgSrcGray, &BLOCKImage);
	TransGrayImgToBlock(ImgMaskGray, &BLOCKMask);

	Logger("HYAMR_TrainTemplateFromMask...\n");

	/*	if ((ImgSrc.fmtImg==FORMAT_GRAY))
	{
	flag=1;
	TransGrayImgToBlock(ImgSrc,&BLOCKImage);		
	}

	if (ImgSrc.fmtImg==FORMAT_BGR)
	{
	flag=1;
	ImgFmtTrans(&ImgSrc,&ImgSrcGray);
	TransGrayImgToBlock(ImgSrcGray,&BLOCKImage);
	}

	if (ImgSrc.fmtImg==FORMAT_RGB)
	{
	flag=1;
	ImgFmtTrans(&ImgSrc,&ImgSrcBGR);
	ImgFmtTrans(&ImgSrcBGR,&ImgSrcGray);
	TransGrayImgToBlock(ImgSrcGray,&BLOCKImage);
	}
	if (ImgSrc.fmtImg==FORMAT_YUYV)
	{
	flag=1;
	ImgFmtTrans(&ImgSrc,&ImgSrcBGR);
	ImgFmtTrans(&ImgSrcBGR,&ImgSrcGray);
	TransGrayImgToBlock(ImgSrcGray,&BLOCKImage);
	}
	if (flag==0)
	{
	res=LI_ERR_IMAGE_FORMAT;
	goto EXT;
	}
	flag=0;
	if ((ImgMask.fmtImg==FORMAT_GRAY))
	{
	flag=1;
	TransGrayImgToBlock(ImgMask,&BLOCKMask);		
	}

	if (ImgMask.fmtImg==FORMAT_BGR)
	{
	flag=1;
	ImgFmtTrans(&ImgMask,&ImgMaskGray);
	TransGrayImgToBlock(ImgMaskGray,&BLOCKMask);
	}

	if (ImgMask.fmtImg==FORMAT_RGB)
	{
	flag=1;
	ImgFmtTrans(&ImgMask,&ImgMaskBGR);
	ImgFmtTrans(&ImgMaskBGR,&ImgMaskGray);
	TransGrayImgToBlock(ImgMaskGray,&BLOCKMask);
	}
	if (ImgMask.fmtImg==FORMAT_YUYV)
	{
	flag=1;
	ImgFmtTrans(&ImgMask,&ImgMaskBGR);
	ImgFmtTrans(&ImgMaskBGR,&ImgMaskGray);
	TransGrayImgToBlock(ImgMaskGray,&BLOCKMask);
	}

	if (flag==0)
	{
	res=LI_ERR_IMAGE_FORMAT;
	goto EXT;
	}		*/

	if (ImgMask.dwHeight!=ImgSrc.dwHeight||ImgMask.dwWidth!=ImgSrc.dwWidth)
	{
		res=LI_ERR_IMAGE_SIZE_UNMATCH;
		goto EXT;
	}
	PrintBmpEx(BLOCKImage.pBlockData, BLOCKImage.lBlockLine, DATA_U8, BLOCKImage.lWidth, BLOCKImage.lHeight, 1, ".\\log\\blockImg.bmp");
	PrintBmpEx(BLOCKMask.pBlockData, BLOCKMask.lBlockLine, DATA_U8, BLOCKMask.lWidth, BLOCKMask.lHeight, 1, ".\\log\\blockMask.bmp");
	lClassIndex=GetClassIndex(ptDescriptorsGroupsTemp,pClassName);
	if (lClassIndex==-1)
	{
		GO(AddClasses(hMemMgr,ptDescriptorsGroupsTemp,pClassName));
		lClassIndex=ptDescriptorsGroupsTemp->lNumClass-1;
	}
	ptDescriptorClass=ptDescriptorsGroupsTemp->ptDescriptorClass+lClassIndex;//��ȡ��Ӧ����������ָ��


	GO(TrainTemplateMethod(hMemMgr,&BLOCKImage,&BLOCKMask,lParma,ptDescriptorClass));//ѵ��ģ����������

	Logger("HYAMR_TrainTemplateFromMask OK...\n");

EXT:
	//	ImgRelease(hMemMgr,&ImgSrcBGR);
	ImgRelease(hMemMgr,&ImgSrcGray);
	ImgRelease(hMemMgr,&ImgMaskGray);
	//	ImgRelease(hMemMgr,&ImgMaskBGR);
	return res;
}


// ͨ������ƥ���㷨����ȡ���̶�����ͼ���е�λ����Ϣ������ŵ��ڴ�����
MRESULT HYAMR_GetObjRect(MHandle hHYMRDHandle,HYLPAMR_IMAGES pImage,MChar *pClassName, MDouble threshold)
{
	MRESULT res=LI_ERR_NONE;
	BLOCKEXT BextImg = {0};
	JOFFSCREEN ImgSrc  = {0};
	MLong lClassIndex;
	//MLong i;
	PTDescriptorClass ptDescriptorClass=MNull;
	JOFFSCREEN ImgSrcGray ={0};
	TOutParam outParam={0};

	MLong x=0,y=0,r=0;
	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	PTDescriptorsGroup ptDescriptorsGroupsTemp=MNull;
	PARAM_INFO Param={0};
	if (hHYMRDHandle==MNull || pImage == MNull || pClassName == MNull)
	{
		res=LI_ERR_NOT_INIT;
		goto EXT;
	}

	ImgSrc = _TransToInteriorImgFmt(pImage);

	// ��ȡGrayͨ��
	GO(ImgCreate(pMgr->hMemMgr,&ImgSrcGray,FORMAT_GRAY,ImgSrc.dwWidth,ImgSrc.dwHeight));
	GO(ImgFmtTrans(&ImgSrc, &ImgSrcGray)); 
	TransGrayImgToBlock(ImgSrcGray, (PBLOCK)&BextImg); 

	ptDescriptorsGroupsTemp=&(pMgr->tDescriptorsGroup);
	if(ptDescriptorsGroupsTemp->ptDescriptorClass->ptDescriptor->lHeight==0||ptDescriptorsGroupsTemp->ptDescriptorClass->ptDescriptor->lWidth==0)
	{
		res=LI_ERR_INVALID_POINT;
		goto EXT;
	}
	// ��������ѡȡ���ʵ�������
	lClassIndex=GetClassIndex(ptDescriptorsGroupsTemp,pClassName);
	if (lClassIndex==-1)
	{
		res=LI_ERR_CLASS_NOT_EXIT;
		goto EXT;
	}

	ptDescriptorClass=ptDescriptorsGroupsTemp->ptDescriptorClass+lClassIndex;//��ȡ��Ӧ����������ָ��
	outParam.lClassNumber=lClassIndex;

	//����ƥ��Ŀ�꣬���Ŀ��λ���Լ�Ŀ��ĳ���
	GO(MatchRigidBody(pMgr->hMemMgr,(BLOCK*)&BextImg,ptDescriptorClass,threshold,&outParam));
	JPrintf("match ok!\n");
	//����������������
	UpdateTDescriptorNumber(ptDescriptorClass,outParam.lIndex);

	// set target info
	pMgr->ptTarget.left = outParam.lX - outParam.lWidth/2;
	pMgr->ptTarget.top = outParam.lY - outParam.lHeight/2;
	pMgr->ptTarget.right = pMgr->ptTarget.left + outParam.lWidth;
	pMgr->ptTarget.bottom= pMgr->ptTarget.top + outParam.lHeight;
	pMgr->bRefresh = MFalse;
	// train target
	pMgr->ptTrainTarget.left = pMgr->ptTarget.left;
	pMgr->ptTrainTarget.right= pMgr->ptTarget.right;
	pMgr->ptTrainTarget.top= pMgr->ptTarget.top;
	pMgr->ptTrainTarget.bottom = pMgr->ptTarget.bottom;

EXT:	
	ImgRelease(pMgr->hMemMgr,	&ImgSrcGray);
	return res;

}

// ʶ�����ָ����
// 1. ����ƥ�䣬Ѱ�Ҳ���ͼ���б���λ����Ϣ������ʶ��׶ε�maskͼ���룬��ȡ������ͼ��
// 2. �ж�ָ�����������������1������ʱ��Ĭ��Ϊ���±����ں�ɫָ�룬�����Զ�ָ���д����ƣ���������ͼ���RGB�ռ�ת����YUV�ռ�
// 3. ����ָ����������ѭ��ִ�й���4��5������ʶ��ÿһ��ָ����
// 4. ���ָ���߶���1������YUV�ռ�������ɫ��ֵķ�����ͻ����ʶ��ָ���ߣ�
//     ���ָ���߽�һ����Ϊ��ɫ���ŵ������������YUV�ռ�����ɫ��֣�
//     ���ָ���߽�һ���ҷǺ�ɫ��������ִ���
// 5. ִ��ָ��ʶ����GetLineInfo������ָ���߲������̿������ ReadParaInfoCpy
MRESULT HYAMR_GetLineParam(MHandle hHYMRDHandle, HYLPAMR_IMAGES pSrcImage, HYAMR_INTERACTIVE_PARA *pPara, MChar *pClass,
						   MDouble dThreshold, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, HYAMR_READ_PARA *pLinePara)
{
	MRESULT res = LI_ERR_NONE;

	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	BLOCKEXT srcBextImg = {0}, maskBextImg = {0};
	BLOCK srcBlock = {0}, maskBlock = {0},srcBlocktmp = {0};
	JOFFSCREEN srcImg = {0}, graySrcImg = {0};
	MLong lObjWidth, lObjHeight, lMaskObjWidth, lMaskObjHeight, lXDiff, lYDiff;
	MLong YellowSum1=0, YellowSum2=0;
	MLong YellowNum1=0, YellowNum2=0;

	TOutParam outParam = {0};
	MLong lClassIndex;
	PTDescriptorsGroup ptDescriptorsGroup = MNull;
	PTDescriptorClass ptDescriptorClass = MNull;

	PARAM_INFO inParam = {0};
	MLong i,j;
	HYAMR_READ_PARA *pTmpLine;

	// red diff image
	BLOCK diffBlockImage = {0};
	//MLong histArray[256] = {0};

	MLong isYUV = 1;
	JOFFSCREEN ImgSrcYUV420 = {0};
	JOFFSCREEN yuvTmp = {0};
	MByte *srcY, *srcV, *srcU;

	if (MNull==hHYMRDHandle || MNull==pSrcImage || MNull==pLinePara)
	{
		res = LI_ERR_NOT_INIT;
		return res;
	}

	// rgb to gray  blockext
	srcImg = _TransToInteriorImgFmt(pSrcImage);
	GO(ImgCreate(pMgr->hMemMgr, &graySrcImg, FORMAT_GRAY, srcImg.dwWidth, srcImg.dwHeight));
	GO(ImgFmtTrans(&srcImg, &graySrcImg));
	TransGrayImgToBlock(graySrcImg, (PBLOCK)&srcBextImg);

	maskBextImg.block.lBlockLine = pPara->lMaskStride;
	maskBextImg.block.lHeight = pPara->lMaskHeight;
	maskBextImg.block.lWidth = pPara->lMaskWidth;
	maskBextImg.block.pBlockData = pPara->pMaskData;
	maskBextImg.block.typeDataA = pPara->lMaskDataType;

	// match object
	ptDescriptorsGroup = &(pMgr->tDescriptorsGroup);
	ptDescriptorClass = ptDescriptorsGroup->ptDescriptorClass;
	if(0 == ptDescriptorClass->ptDescriptor->lWidth && 0 == ptDescriptorClass->ptDescriptor->lHeight)
	{
		res = LI_ERR_INVALID_POINT;
		goto EXT;
	}
	lClassIndex = GetClassIndex(ptDescriptorsGroup, pClass);
	if (-1==lClassIndex)
	{
		res = LI_ERR_CLASS_NOT_EXIT;
		goto EXT;
	}
	ptDescriptorClass = ptDescriptorsGroup->ptDescriptorClass + lClassIndex;
	outParam.lClassNumber = lClassIndex;
	GO(MatchRigidBody(pMgr->hMemMgr, (BLOCK*)&srcBextImg, ptDescriptorClass, dThreshold, &outParam));
	UpdateTDescriptor(ptDescriptorClass, outParam.lIndex);
	pMgr->ptTarget.left = outParam.lX - (outParam.lWidth>>1);
	pMgr->ptTarget.right = outParam.lX + (outParam.lWidth>>1);
	pMgr->ptTarget.top = outParam.lY - (outParam.lHeight>>1);
	pMgr->ptTarget.bottom = outParam.lY + (outParam.lHeight>>1);
	// refresh target info
	GO(HYAMR_SaveTargetInfoToMem (hHYMRDHandle, pCurTargetMem, pTrainTargetMem, 0));

	srcBextImg.ext.left = pMgr->ptTarget.left;
	srcBextImg.ext.top = pMgr->ptTarget.top;
	srcBextImg.ext.right =  srcBextImg.block.lWidth - pMgr->ptTarget.right;
	srcBextImg.ext.bottom = srcBextImg.block.lHeight - pMgr->ptTarget.bottom;
	//maskBextImg.ext = srcBextImg.ext;
	maskBextImg.ext.left = pMgr->ptTrainTarget.left;
	maskBextImg.ext.right = maskBextImg.block.lWidth - pMgr->ptTrainTarget.right;
	maskBextImg.ext.top = pMgr->ptTrainTarget.top;
	maskBextImg.ext.bottom = maskBextImg.block.lHeight - pMgr->ptTrainTarget.bottom;
	lObjWidth = srcBextImg.block.lWidth - (srcBextImg.ext.left + srcBextImg.ext.right);
	lObjHeight = srcBextImg.block.lHeight - (srcBextImg.ext.top + srcBextImg.ext.bottom);
	lMaskObjWidth = maskBextImg.block.lWidth - (maskBextImg.ext.left + maskBextImg.ext.right);
	lMaskObjHeight = maskBextImg.block.lHeight - (maskBextImg.ext.top + maskBextImg.ext.bottom);
	lXDiff = lMaskObjWidth - lObjWidth;
	lYDiff = lMaskObjHeight - lObjHeight;
	if ((srcBextImg.ext.left - lXDiff) >= 0)
	{
		srcBextImg.ext.left -= lXDiff;
	}
	else
	{
		srcBextImg.ext.right += lXDiff;
	}
	if ((srcBextImg.ext.top - lYDiff) >= 0)
	{
		srcBextImg.ext.top -= lYDiff;
	}
	else
	{
		srcBextImg.ext.bottom += lYDiff;
	}

	srcBlock = BE_ValidBlock(&srcBextImg);
	maskBlock = BE_ValidBlock(&maskBextImg);

	GO(B_Create(pMgr->hMemMgr, &srcBlocktmp, DATA_U8, srcBlock.lWidth, srcBlock.lHeight)); 
	B_Cpy(&srcBlocktmp, &srcBlock);//���Ʊ���ԭʼͼƬӰ��

	inParam.bWhiteBackground = pPara->bWhiteBackground;
	inParam.lHarrWidth = pPara->lLineWidth;
	inParam.lPicBlurLevel = pPara->picLevel;
	inParam.lDoubleRedLine =0;
	inParam.pCircleInfo = &(pMgr->AmmeterCircle);
	inParam.lCircleDist = pPara->Dist ;

	if (pPara->lLineNum >= 2)
	{
		// ��ɫ�ռ�ת�� RGB��>YUV
		GO(B_Create(pMgr->hMemMgr, &diffBlockImage, DATA_U8, srcBlock.lWidth, srcBlock.lHeight));
		GO(ImgCreate(pMgr->hMemMgr, &ImgSrcYUV420, FORMAT_YUV420_VUVU, srcImg.dwWidth, srcImg.dwHeight));
		GO(ImgFmtTrans(&srcImg, &ImgSrcYUV420));
		yuvTmp = ImgSrcYUV420;
		ImgChunky2Plannar(&yuvTmp);

		srcY = (MByte*)ImgSrcYUV420.pixelArray.chunky.pPixel + ImgSrcYUV420.pixelArray.chunky.dwImgLine * pMgr->ptTarget.top + pMgr->ptTarget.left;
		srcV = (MByte*)yuvTmp.pixelArray.planar.pPixel[1] + yuvTmp.pixelArray.planar.dwImgLine[1] * (pMgr->ptTarget.top>>1) + ((pMgr->ptTarget.left>>1)<<1);
		srcU = srcV + 1;
	}

	for (i=0; i<pPara->lLineNum; i++)
	{
		pTmpLine = pLinePara + i;

		if(pPara->lLineNum >= 2 )
		{
			/// �����ߵ���ɫ��һ��
			if(pPara->lLineColor[0]!=pPara->lLineColor[1])     
			{
				//ָ����Ϊ����,��Ϊ�׵�,�����ɫָ����,��ɫ���ϵ�ֵС���ܱߣ��ܱ߸�Ϊ128
				//���������������ָ��Ϊһ��һ�� ��Ϊ��
				if(pPara->bWhiteBackground)
				{
					if(HY_LINE_RED == pPara->lLineColor[i])
					{	
						MLong histArray[256] = {0};
						// ��ɫ��� �����ɫָ����
						getRedDiffImage2_yuv(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
						B_Cpy(&srcBlock, &diffBlockImage);
					}
					else                                         
					{
						MLong histArray[256] = {0};
						// ��ɫ��� �����ɫָ����
						getBlackDiffImage2_yuv(srcY, srcV, srcU, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
						B_Cpy(&srcBlock, &diffBlockImage);
					}
				}
				else
				{
					//ָ����Ϊ����,Ϊ�ڵ�,�����ɫָ����,��ɫ���ϵ�ֵҪ�����ܱߣ��ܱ߸�Ϊ0 ����������ǲ�ͬ��
					//���������������ָ��Ϊһ��һ�� ��Ϊ��
					if (HY_LINE_RED == pPara->lLineColor[i])
					{
						MLong histArray[256] = {0};
						// ��ɫ��� �����ɫָ����
						getRedDiffImage2_yuvblack(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
						B_Cpy(&srcBlock, &diffBlockImage);
					}
					else
					{
						MLong histArray[256] = {0};
						// ��ɫ��� �����ɫָ����
						getWhiteDiffImage2_yuv(srcY, srcV, srcU, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
						B_Cpy(&srcBlock, &diffBlockImage);
						//B_Cpy(&srcBlock, &srcBlocktmp); //���Ƹ�ԭͼ���к���ָ����
					}
				}
			}
			///������ɫһ��������ô���� ��������
			else if(pPara->lLineColor[0]==pPara->lLineColor[1])
			{
				if(pPara->bWhiteBackground)
				{
					if(HY_LINE_RED == pPara->lLineColor[i])
					{	
						if(i==0)
						{
							MLong histArray[256] = {0};
							// ��ɫ��� �����ɫָ����
							getRedDiffImage2_yuv(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
							B_Cpy(&srcBlock, &diffBlockImage);
						}
						else
						{
							MLong histArray[256] = {0};
							inParam.lDoubleRedLine =1;
							getRedDiffImage2_yuv(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
							B_Cpy(&srcBlock, &diffBlockImage);
						}
					}
					else                                         
					{
						MLong histArray[256] = {0};
						// ��ɫ��� ����ڵ׺�ɫָ����
						getRedDiffImage2_yuvblack(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
						B_Cpy(&srcBlock, &diffBlockImage);
					}
				}
			}
		}

		else if (HY_LINE_RED == pPara->lLineColor[i])		// one line case
		{
			GO(B_Create(pMgr->hMemMgr, &diffBlockImage, DATA_U8, srcBlock.lWidth, srcBlock.lHeight));
			GO(ImgCreate(pMgr->hMemMgr, &ImgSrcYUV420, FORMAT_YUV420_VUVU, srcImg.dwWidth, srcImg.dwHeight));
			GO(ImgFmtTrans(&srcImg, &ImgSrcYUV420));
			yuvTmp = ImgSrcYUV420;
			ImgChunky2Plannar(&yuvTmp);

			srcY = (MByte*)ImgSrcYUV420.pixelArray.chunky.pPixel + ImgSrcYUV420.pixelArray.chunky.dwImgLine * pMgr->ptTarget.top + pMgr->ptTarget.left;
			srcV = (MByte*)yuvTmp.pixelArray.planar.pPixel[1] + yuvTmp.pixelArray.planar.dwImgLine[1] * (pMgr->ptTarget.top>>1) + ((pMgr->ptTarget.left>>1)<<1);
			srcU = srcV + 1;

			//PrintBmpEx(srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, DATA_U8, ImgSrcYUV420.dwWidth,ImgSrcYUV420.dwHeight, 1, ".\\log\\src_yuv.bmp");
			if(pPara->bWhiteBackground)
			{
				MLong histArray[256] = {0};
				getRedDiffImage2_yuv(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
			}
			else
			{
				MLong histArray[256] = {0};
				getRedDiffImage2_yuvblack(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
			}
			B_Cpy(&srcBlock, &diffBlockImage);
		}

		else if (HY_LINE_YELLOW == pPara->lLineColor[i])		// one line case
		{
			MByte *pBlockData =(MByte*)srcBlock.pBlockData ; 
			for (j=0;j<srcBlock.lHeight;j++)
			{		
				for(i =0; i<srcBlock.lWidth;i++)
				{
					if(pBlockData[j*srcBlock.lBlockLine+i] >120)
					{
						if(j<srcBlock.lHeight/2)
						{
							YellowSum1 +=pBlockData[j*srcBlock.lBlockLine+i] ;
							YellowNum1++;
						}
						else
						{
							YellowSum2 +=pBlockData[j*srcBlock.lBlockLine+i] ;
							YellowNum2++;
						}										
				    }
			     }
			}
			if(YellowNum1>YellowNum2)  //��ɫָ���������� ��֮������
			{
				for (j=0;j<srcBlock.lHeight;j++)
				{		
					for(i =0; i<srcBlock.lWidth;i++)
					{
						if(j>=srcBlock.lHeight/8 && j<srcBlock.lHeight/4)
							pBlockData[j*srcBlock.lBlockLine+i] =255;
						else
							pBlockData[j*srcBlock.lBlockLine+i]=0;
					}
				}
			}
			else
			{
				for (j=0;j<srcBlock.lHeight;j++)
				{		
					for(i =0; i<srcBlock.lWidth;i++)
					{
						if(j>=(srcBlock.lHeight*3/4) && j<(srcBlock.lHeight*7/8))
							pBlockData[j*srcBlock.lBlockLine+i] =255;
						else
							pBlockData[j*srcBlock.lBlockLine+i]=0;
					}
				}
			}
		}
			
		if (LI_ERR_NONE != GetLineInfo(pMgr->hMemMgr, &srcBlock, &maskBlock, &inParam))
		{
			pTmpLine->dCoeffK = 0;
			pTmpLine->dCoeffB = 0;
			pTmpLine->bVert = MFalse;
			res = LI_ERR_UNKNOWN;
			goto EXT;
		}
		else
		{
			ReadParaInfoCpy(inParam, pTmpLine); // ָ������Ϣ����
		}
	}

EXT:
	ImgRelease(pMgr->hMemMgr, &graySrcImg);
	B_Release(pMgr->hMemMgr, &diffBlockImage);
	B_Release(pMgr->hMemMgr, &srcBlocktmp);
	ImgRelease(pMgr->hMemMgr, &ImgSrcYUV420);
	return res;
}

// ����ָ���߲�����Ϣ
MVoid ReadParaInfoCpy(PARAM_INFO Param, HYAMR_READ_PARA *pDst)
{
	MLong i;

	pDst->bVert = Param.pCircleInfo->lineInfo.lineParam.bVertical;
	pDst->dCoeffK = Param.pCircleInfo->lineInfo.lineParam.Coefk;
	if(pDst->bVert)
		pDst->dCoeffB = Param.pCircleInfo->lineInfo.lineParam.Coefb;    // -lLeft
	else
		pDst->dCoeffB = Param.pCircleInfo->lineInfo.lineParam.Coefb;	// - k*left

	pDst->ptStart.x = Param.pCircleInfo->lineInfo.lineParam.ptStart.x;		// +left
	pDst->ptStart.y = Param.pCircleInfo->lineInfo.lineParam.ptStart.y;		// +top
	pDst->ptEnd.x = Param.pCircleInfo->lineInfo.lineParam.ptEnd.x;		// +left
	pDst->ptEnd.y = Param.pCircleInfo->lineInfo.lineParam.ptEnd.y;		// +top

	for (i=0; i<Param.pCircleInfo->lPtNum; i++)
	{
		pDst->ptInfo[i].ptPos.x = Param.pCircleInfo->pPtinfo[i].ptPoint.x;
		pDst->ptInfo[i].ptPos.y = Param.pCircleInfo->pPtinfo[i].ptPoint.y;
	}
	pDst->lPtNum = Param.pCircleInfo->lPtNum;
}

// �����Ǳ����
// 1. �����̶ȵ���Ϣ��ָ���߲�����Ϣ
// 2. ���ݴ�̶ȵ����������ָ��ʽ���غ�һ���ƣ�
//     �����ָ��ʽ���أ�����GetSwitchStatus���㿪��״̬��
//     �����һ���ƣ�����CalcMeterResult������̵�ǰ����
MRESULT HYAMR_GetMeterResult(MHandle hHYMRDHandle, HYAMR_READ_PARA readPara, MDouble *pResult)
{
	MRESULT res = LI_ERR_NONE;

	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	MPOINT *pPtPos;
	MDouble *pPtVal;
	LINEPARAM lineParam;
	MLong i;

	AllocVectMem(pMgr->hMemMgr, pPtPos, readPara.lPtNum, MPOINT);
	AllocVectMem(pMgr->hMemMgr, pPtVal, readPara.lPtNum, MDouble);

	for (i=0; i<readPara.lPtNum; i++)
	{
		*(pPtPos + i) = readPara.ptInfo[i].ptPos;
		*(pPtVal + i) = readPara.ptInfo[i].ptVal;
	}

	lineParam.Coefk = readPara.dCoeffK;
	lineParam.Coefb = readPara.dCoeffB;
	lineParam.bVertical = readPara.bVert;
	lineParam.ptStart = readPara.ptStart;
	lineParam.ptEnd = readPara.ptEnd;

	if (SWITCH_METER_PTNUM == readPara.lPtNum)
	{
		*pResult = GetSwitchStatus(pPtPos, pPtVal, &lineParam, readPara.lPtNum);
	}
	else
	{
		*pResult = CalcMeterResult(pPtPos, pPtVal, &lineParam, readPara.lPtNum);
	}

EXT:
	FreeVectMem(pMgr->hMemMgr, pPtPos);
	FreeVectMem(pMgr->hMemMgr, pPtVal);
	return res;
}

/*===========================================================
������	��HYMRB_SaveTDescriptorsGroup
����		������������Ϣ���浽�ڴ���
�������˵����
hHYMRDHandle:  ������� 
pDesMem:   �ڴ��ַ
lSize:   Ԥ�����ڴ�ռ��С
lDstSize:   ʵ�ʴ����������Ϣռ���ڴ�ռ��С
�������˵����
��
����ֵ˵����������������
===========================================================*/
MLong HYAMR_SaveDesMem (MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize)
{
	int i,j;
	int lTmpSize;
	unsigned char head[4];
	unsigned char *outPtr;
	PAMMETERINFO pMgr;
	MHandle hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp;	
	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	MLong lTmpNum;

	pMgr = (PAMMETERINFO) hHYMRDHandle;
	hMemMgr=pMgr->hMemMgr;
	ptDescriptorsGroupTemp=&(pMgr->tDescriptorsGroup);

	if (ptDescriptorsGroupTemp==MNull)
	{
		return -1;
	}
	if (ptDescriptorsGroupTemp->lNumClass==0)
	{
		return -1;
	}
	if (MNull==pDesMem)
	{
		return -1;
	}

	lTmpSize = 4;  // lTmpSize������ʱ�ۻ������ڴ�ʹ�����
	for (i=0; i<ptDescriptorsGroupTemp->lNumClass; i++)
	{
		ptDescirptorClass = ptDescriptorsGroupTemp->ptDescriptorClass+i;
		lTmpSize += 24;			// 4+4+16
		for (j=0;j<ptDescirptorClass->lNumDescriptors; j++)
		{
			ptDescriptor=ptDescirptorClass->ptDescriptor+j;
			lTmpSize += 24;		// 4*6
			lTmpNum = ptDescriptor->lWidth*ptDescriptor->lHeight;
			lTmpSize += lTmpNum;
		}
	}
	if (lTmpSize > lSize)
	{
		Err_Print("HYAMR_SaveDesMem  inMemSize is not big enough!\n");
		return -1;
	}
	*lDstSize = lTmpSize;

	outPtr = pDesMem;
	head[0] = (ptDescriptorsGroupTemp->lNumClass & 0xff000000)>>24;
	head[1] = (ptDescriptorsGroupTemp->lNumClass & 0x00ff0000)>>16;
	head[2] = (ptDescriptorsGroupTemp->lNumClass & 0x0000ff00)>>8;
	head[3] = (ptDescriptorsGroupTemp->lNumClass & 0x000000ff);
	WriteMem(head, outPtr, 4);
	outPtr += 4;

	for (i=0;i<ptDescriptorsGroupTemp->lNumClass; i++)
	{
		ptDescirptorClass = ptDescriptorsGroupTemp->ptDescriptorClass+i;
		head[0] = (ptDescirptorClass->lClassIndex & 0xff000000)>>24;
		head[1] = (ptDescirptorClass->lClassIndex & 0x00ff0000)>>16;
		head[2] = (ptDescirptorClass->lClassIndex & 0x0000ff00)>>8;
		head[3] = (ptDescirptorClass->lClassIndex & 0x000000ff);
		WriteMem(head, outPtr, 4);
		outPtr += 4;

		head[0] = (ptDescirptorClass->lNumDescriptors & 0xff000000)>>24;
		head[1] = (ptDescirptorClass->lNumDescriptors & 0x00ff0000)>>16;
		head[2] = (ptDescirptorClass->lNumDescriptors & 0x0000ff00)>>8;
		head[3] = (ptDescirptorClass->lNumDescriptors & 0x000000ff);
		WriteMem(head, outPtr, 4);
		outPtr += 4;

		WriteMem(ptDescirptorClass->pClassName, outPtr, 16);
		outPtr += 16;

		for (j=0; j<ptDescirptorClass->lNumDescriptors; j++)
		{
			ptDescriptor=ptDescirptorClass->ptDescriptor+j;

			head[0] = (ptDescriptor->lDescriptorIndex & 0xff000000)>>24;
			head[1] = (ptDescriptor->lDescriptorIndex & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lDescriptorIndex & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lDescriptorIndex & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			head[0] = (ptDescriptor->lSample & 0xff000000)>>24;
			head[1] = (ptDescriptor->lSample & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lSample & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lSample & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			head[0] = (ptDescriptor->lWidth & 0xff000000)>>24;
			head[1] = (ptDescriptor->lWidth & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lWidth & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lWidth & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			head[0] = (ptDescriptor->lHeight & 0xff000000)>>24;
			head[1] = (ptDescriptor->lHeight & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lHeight & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lHeight & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			head[0] = (ptDescriptor->lRegion & 0xff000000)>>24;
			head[1] = (ptDescriptor->lRegion & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lRegion & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lRegion & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			head[0] = (ptDescriptor->lAngle & 0xff000000)>>24;
			head[1] = (ptDescriptor->lAngle & 0x00ff0000)>>16;
			head[2] = (ptDescriptor->lAngle & 0x0000ff00)>>8;
			head[3] = (ptDescriptor->lAngle & 0x000000ff);
			WriteMem(head, outPtr, 4);
			outPtr += 4;

			lTmpNum = ptDescriptor->lWidth*ptDescriptor->lHeight;
			WriteMem(ptDescriptor->pData, outPtr, lTmpNum);
			outPtr += lTmpNum;
		}
	}

	return 0;
}

// ������ƥ��ѵ����õ���������Ϣ���ļ�
MLong HYAMR_SaveTDescriptorsGroup (MHandle hHYMRDHandle,const char *path)
{
	FILE *fp=fopen(path,"w");

	MLong i;
	MLong j;
	MLong k;
	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;
	MHandle hMemMgr=pMgr->hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp=&(pMgr->tDescriptorsGroup);	
	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	if (fp==MNull)
	{
		return -1;
	}
	if (ptDescriptorsGroupTemp==MNull)
	{
		return -1;
	}
	if (ptDescriptorsGroupTemp->lNumClass==0)
	{
		return -1;
	}
	//if (ptDescriptorsGroup)
	fprintf(fp,"%d\n",ptDescriptorsGroupTemp->lNumClass);
	for (i=0;i<ptDescriptorsGroupTemp->lNumClass;i++)
	{
		ptDescirptorClass=ptDescriptorsGroupTemp->ptDescriptorClass+i;
		fprintf(fp,"%d ",ptDescirptorClass->lClassIndex);
		fprintf(fp,"%d\n",ptDescirptorClass->lNumDescriptors);
		fprintf(fp,"%s\n",ptDescirptorClass->pClassName);
		for (j=0;j<ptDescirptorClass->lNumDescriptors;j++)
		{
			ptDescriptor=ptDescirptorClass->ptDescriptor+j;
			fprintf(fp,"%d\n",ptDescriptor->lDescriptorIndex);
			fprintf(fp,"%d %d %d %d %d\n",ptDescriptor->lSample,ptDescriptor->lWidth,ptDescriptor->lHeight,ptDescriptor->lRegion,ptDescriptor->lAngle);
			for (k=0;k<ptDescriptor->lWidth*ptDescriptor->lHeight;k++)
			{
				if (k%(ptDescriptor->lWidth)==0)
				{
					fprintf(fp,"\n");
				}
				fprintf(fp,"%4d ",*(ptDescriptor->pData+k));
			}
			fprintf(fp,"\n");

		}

	}
	fclose(fp);
	return 0;
}

/*===========================================================
������	��HYAMR_SaveTargetInfo
����		�������̶���λ����Ϣ(rect)���浽�ļ���
�������˵����
hHYMRDHandle:�������  
path������·��(��ǰ֡ͼ��λ����Ϣ���)
trainPath: ����·����ѵ��֡ͼ��λ����Ϣ��ţ�
isTrain:  ����Ƿ�Ϊѵ���׶δ�ű���λ����Ϣ   1 ѵ���׶�  0 ʶ��׶�
�������˵����
��
����ֵ˵����������������
===========================================================*/
MRESULT HYAMR_SaveTargetInfo (MHandle hHYMRDHandle,const char *path, const char *trainPath, MBool isTrain)
{
	MRESULT res=LI_ERR_NONE;

	int left, right, top, bottom;

	FILE *fp = MNull;
	FILE *trainFp = MNull;
	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;
	//	printf("[%s-%s-%d]\n", __FILE__,__FUNCTION__);
	fp = fopen(path, "wb");;
	if(MNull == fp)
	{
		res = LI_ERR_NO_FIND;
		return res;
	}

	left = right = top = bottom = 0;
	left = pMgr->ptTarget.left;
	right = pMgr->ptTarget.right;
	top = pMgr->ptTarget.top;
	bottom = pMgr->ptTarget.bottom;
	//	printf("left=%d, right=%d, top=%d, bottom=%d\n");
	if(0==left && 0==right && 0==top && 0==bottom)
	{
		Err_Print("target not exist!\n");
		fclose(fp);
		res = LI_ERR_NO_FIND;
		return res;
	}

	fprintf(fp, "%d %d %d %d", left, right, top, bottom);
	fclose(fp);
	if(1==isTrain)	// save train target info
	{
		JPrintf("[%s,%s,%d]\n", __FILE__, __FUNCTION__, __LINE__);
		trainFp = fopen(trainPath, "wb");
		if(MNull == trainFp)
		{
			res = LI_ERR_NO_FIND;
			return res;
		}

		left = right = top = bottom = 0;

		left = pMgr->ptTrainTarget.left;
		right = pMgr->ptTrainTarget.right;
		top = pMgr->ptTrainTarget.top;
		bottom = pMgr->ptTrainTarget.bottom;
		if(0==left && 0==right && 0==top && 0==bottom)
		{
			Err_Print("target not exist!\n");
			fclose(trainFp);
			res = LI_ERR_NO_FIND;
			return res;
		}

		fprintf(trainFp, "%d %d %d %d", left, right, top, bottom);
		fclose(trainFp);
	}

	return res;
}

//=========================================================
// ������λ����Ϣ��ŵ��ڴ���
// hHYMRDHandle  �������
// pCurTargetMem �ڴ��ַ ��ŵ�ǰ֡ͼ���б���λ����Ϣ
// pTrainTarget  �ڴ��ַ ���ѵ���׶�ͼ���б���λ����Ϣ
// isTrain ����Ƿ�Ϊѵ���׶�    0 ʶ��׶�   1 ѵ���׶�
MRESULT HYAMR_SaveTargetInfoToMem (MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, MBool isTrain)
{
	MRESULT res = LI_ERR_NONE;

	unsigned char curRect[16];
	unsigned char trainRect[16];
	int left, right, top, bottom;
	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;
	unsigned char *pTmpData = MNull;

	if (MNull == pMgr)
	{
		printf("HYAMR_SaveTargetInfoToMem: pMgr is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pCurTargetMem)
	{
		printf("HYAMR_SaveTargetInfoToMem: pCurTargetMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pTrainTargetMem)
	{
		printf("HYAMR_SaveTargetInfoToMem: pTrainTaregtMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	// cur rect
	left = right = top = bottom = 0;
	left = pMgr->ptTarget.left;
	right = pMgr->ptTarget.right;
	top = pMgr->ptTarget.top;
	bottom = pMgr->ptTarget.bottom;
	//printf("left=%d  right=%d  top=%d  bottom=%d\n", left, right, top, bottom);
	if(0==left && 0==right && 0==top && 0==bottom)
	{
		printf("cur target not exist!\n");
		res = LI_ERR_NO_FIND;
		return res;
	}
	curRect[0] = (left & 0xff000000)>>24;
	curRect[1] = (left & 0x00ff0000)>>16;
	curRect[2] = (left & 0x0000ff00)>>8;
	curRect[3] = (left & 0x000000ff);

	curRect[4] = (right & 0xff000000)>>24;
	curRect[5] = (right & 0x00ff0000)>>16;
	curRect[6] = (right & 0x0000ff00)>>8;
	curRect[7] = (right & 0x000000ff);

	curRect[8] = (top & 0xff000000)>>24;
	curRect[9] = (top & 0x00ff0000)>>16;
	curRect[10] = (top & 0x0000ff00)>>8;
	curRect[11] = (top & 0x000000ff);

	curRect[12] = (bottom & 0xff000000)>>24;
	curRect[13] = (bottom & 0x00ff0000)>>16;
	curRect[14] = (bottom & 0x0000ff00)>>8;
	curRect[15] = (bottom & 0x000000ff);

	pTmpData = pCurTargetMem;
	WriteMem(curRect, pTmpData, 16);

	// train rect
	if (isTrain)
	{
		left = right = top = bottom = 0;
		left = pMgr->ptTrainTarget.left;
		right = pMgr->ptTrainTarget.right;
		top = pMgr->ptTrainTarget.top;
		bottom = pMgr->ptTrainTarget.bottom;
		//printf("left=%d  right=%d  top=%d  bottom=%d\n", left, right, top, bottom);
		if(0==left && 0==right && 0==top && 0==bottom)
		{
			printf("train target not exist!\n");
			res = LI_ERR_NO_FIND;
			return res;
		}
		trainRect[0] = (left & 0xff000000)>>24;
		trainRect[1] = (left & 0x00ff0000)>>16;
		trainRect[2] = (left & 0x0000ff00)>>8;
		trainRect[3] = (left & 0x000000ff);

		trainRect[4] = (right & 0xff000000)>>24;
		trainRect[5] = (right & 0x00ff0000)>>16;
		trainRect[6] = (right & 0x0000ff00)>>8;
		trainRect[7] = (right & 0x000000ff);

		trainRect[8] = (top & 0xff000000)>>24;
		trainRect[9] = (top & 0x00ff0000)>>16;
		trainRect[10] = (top & 0x0000ff00)>>8;
		trainRect[11] = (top & 0x000000ff);

		trainRect[12] = (bottom & 0xff000000)>>24;
		trainRect[13] = (bottom & 0x00ff0000)>>16;
		trainRect[14] = (bottom & 0x0000ff00)>>8;
		trainRect[15] = (bottom & 0x000000ff);

		pTmpData = pTrainTargetMem;
		WriteMem(trainRect, pTmpData, 16);
	}
	return res;
}


//tmp
MRESULT HYAMR_SaveTargetInfoToMemTmp (MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, MBool isTrain,int *rect )
{
	MRESULT res = LI_ERR_NONE;

	unsigned char curRect[16];
	unsigned char trainRect[16];
	int left, right, top, bottom;
	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;
	unsigned char *pTmpData = MNull;

	if (MNull == pMgr)
	{
		printf("HYAMR_SaveTargetInfoToMem: pMgr is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pCurTargetMem)
	{
		printf("HYAMR_SaveTargetInfoToMem: pCurTargetMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pTrainTargetMem)
	{
		printf("HYAMR_SaveTargetInfoToMem: pTrainTaregtMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	// cur rect
	left = right = top = bottom = 0;
	left = rect[0];
	right = rect[2];
	top = rect[1];
	bottom = rect[3];
	pMgr->ptTarget.left = pMgr->ptTrainTarget.left = left;
	pMgr->ptTarget.right = pMgr->ptTrainTarget.right = right;
	pMgr->ptTarget.top =  pMgr->ptTrainTarget.top = top;
	pMgr->ptTarget.bottom = pMgr->ptTrainTarget.bottom = bottom ;
	//printf("left=%d  right=%d  top=%d  bottom=%d\n", left, right, top, bottom);
	if(0==left && 0==right && 0==top && 0==bottom)
	{
		printf("cur target not exist!\n");
		res = LI_ERR_NO_FIND;
		return res;
	}
	curRect[0] = (left & 0xff000000)>>24;
	curRect[1] = (left & 0x00ff0000)>>16;
	curRect[2] = (left & 0x0000ff00)>>8;
	curRect[3] = (left & 0x000000ff);

	curRect[4] = (right & 0xff000000)>>24;
	curRect[5] = (right & 0x00ff0000)>>16;
	curRect[6] = (right & 0x0000ff00)>>8;
	curRect[7] = (right & 0x000000ff);

	curRect[8] = (top & 0xff000000)>>24;
	curRect[9] = (top & 0x00ff0000)>>16;
	curRect[10] = (top & 0x0000ff00)>>8;
	curRect[11] = (top & 0x000000ff);

	curRect[12] = (bottom & 0xff000000)>>24;
	curRect[13] = (bottom & 0x00ff0000)>>16;
	curRect[14] = (bottom & 0x0000ff00)>>8;
	curRect[15] = (bottom & 0x000000ff);

	pTmpData = pCurTargetMem;
	WriteMem(curRect, pTmpData, 16);

	// train rect
	if (isTrain)
	{
		left = right = top = bottom = 0;
		left = pMgr->ptTrainTarget.left;
		right = pMgr->ptTrainTarget.right;
		top = pMgr->ptTrainTarget.top;
		bottom = pMgr->ptTrainTarget.bottom;
		//printf("left=%d  right=%d  top=%d  bottom=%d\n", left, right, top, bottom);
		if(0==left && 0==right && 0==top && 0==bottom)
		{
			printf("train target not exist!\n");
			res = LI_ERR_NO_FIND;
			return res;
		}
		trainRect[0] = (left & 0xff000000)>>24;
		trainRect[1] = (left & 0x00ff0000)>>16;
		trainRect[2] = (left & 0x0000ff00)>>8;
		trainRect[3] = (left & 0x000000ff);

		trainRect[4] = (right & 0xff000000)>>24;
		trainRect[5] = (right & 0x00ff0000)>>16;
		trainRect[6] = (right & 0x0000ff00)>>8;
		trainRect[7] = (right & 0x000000ff);

		trainRect[8] = (top & 0xff000000)>>24;
		trainRect[9] = (top & 0x00ff0000)>>16;
		trainRect[10] = (top & 0x0000ff00)>>8;
		trainRect[11] = (top & 0x000000ff);

		trainRect[12] = (bottom & 0xff000000)>>24;
		trainRect[13] = (bottom & 0x00ff0000)>>16;
		trainRect[14] = (bottom & 0x0000ff00)>>8;
		trainRect[15] = (bottom & 0x000000ff);

		pTmpData = pTrainTargetMem;
		WriteMem(trainRect, pTmpData, 16);
	}
	return res;
}

/*=========================================================
������	��HYAMR_GetTargetInfo
����		�����ļ��л�ȡ����λ����Ϣ
�������˵����
hHYMRDHandle:�������
path���ļ�·��
trainPath: �ļ�·��
lImgWidth: ͼ����
lImgHeight: ͼ��߶�
�������˵������           
����ֵ˵����������������
===========================================================*/
MRESULT HYAMR_GetTargetInfo (MHandle hHYMRDHandle,char* path, char *trainPath, MLong lImgWidth, MLong lImgHeight)
{
	MRESULT res = LI_ERR_NONE;

	int left=0, right=0, top=0, bottom=0;

	FILE *fp = MNull;
	FILE *trainFp = MNull;
	MLong width, height;

	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;
	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	// get current target info from file "path"
	fp = fopen(path, "rb");
	if(MNull==fp)

	{
		res = LI_ERR_NO_FIND;
		return res;
	}
	//	printf("[%s,%s,%d]\n", __FILE__,__FUNCTION__,__LINE__);
	fscanf(fp, "%d %d %d %d", (int *)(&left), (int *)(&right),  (int *)(&top),  (int *)(&bottom));
	//	printf("left=%d, right=%d, top=%d, bottom=%d\n", left, right, top, bottom);
	width = right - left + 1;
	height= bottom-top+1;
	//	JPrintf("[%s,%s,%d]~~~fp width=%d,height=%d\n", __FILE__, __FUNCTION__, __LINE__, width, height);
	if (width <=1 || height <=1 ||width >lImgWidth ||height>lImgHeight)	
	{
		Err_Print("target not exist!\n");
		res = LI_ERR_NO_FIND;
		if (!fp)fclose(fp);
		return res;
	}


	pMgr->ptTarget.left = left;
	pMgr->ptTarget.right = right;
	pMgr->ptTarget.top = top;
	pMgr->ptTarget.bottom = bottom;

	if (!fp)
		fclose(fp);
	// get train target info from file "trainPath"
	trainFp = fopen(trainPath, "rb");
	if(MNull == trainFp)
	{
		res = LI_ERR_NO_FIND;
		return res;
	}

	left = right = top = bottom = 0;
	fscanf(trainFp, "%d %d %d %d", (int *)(&left), (int *)(&right), (int *)(&top), (int *)(&bottom));
	width = right - left + 1;
	height= bottom-top+1;
	//	JPrintf("[%s,%s,%d]~~~trainFp width=%d,height=%d\n", __FILE__, __FUNCTION__, __LINE__, width, height);
	if (width <=1 || height <=1 ||width >lImgWidth ||height>lImgHeight)	
	{
		Err_Print("target not exist!\n");
		res = LI_ERR_NO_FIND;
		if (!trainFp)
			fclose(trainFp);
		return res;
	}

	pMgr->ptTrainTarget.left = left;
	pMgr->ptTrainTarget.right = right;
	pMgr->ptTrainTarget.bottom = bottom;
	pMgr->ptTrainTarget.top = top;
	if(!trainFp)
		fclose(trainFp);

	return res;
}

//===========================================================
// ���ڴ��ȡ����λ����Ϣ
// hHYMRDHandle  �������
// pCurTargetMem  �ڴ��ַ ��ŵ�ǰ֡ͼ���б���λ����Ϣ
// pTrainTargetMem  �ڴ��ַ ���ѵ���׶�ͼ���б���λ����Ϣ
// lImgWidth    ͼ����
// lImgHeight   ͼ��߶�
MRESULT HYAMR_GetTaregetInfoFromMem (MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTaregtMem,
									 MLong lImgWidth, MLong lImgHeight)
{
	MRESULT res = LI_ERR_NONE;

	unsigned char curRect[16];
	unsigned char trainRect[16];
	MLong lWidth, lHeight;
	unsigned char *pTmpData;
	PAMMETERINFO pMgr = (PAMMETERINFO) hHYMRDHandle;

	if (MNull == pMgr)
	{
		printf("HYAMR_SaveTargetInfoToMem: pMgr is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pCurTargetMem)
	{
		printf("HYAMR_GetTaregetInfoFromMem: pCurTargetMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}
	if (MNull==pTrainTaregtMem)
	{
		printf("HYAMR_GetTaregetInfoFromMem: pTrainTaregtMem is null!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	pTmpData = pCurTargetMem;
	WriteMem(pTmpData, curRect, 16);
	pMgr->ptTarget.left = (curRect[0]<<24) | (curRect[1]<<16) | (curRect[2]<<8) | curRect[3];
	pMgr->ptTarget.right = (curRect[4]<<24) | (curRect[5]<<16) | (curRect[6]<<8) | curRect[7];
	pMgr->ptTarget.top = (curRect[8]<<24) | (curRect[9]<<16) | (curRect[10]<<8) | curRect[11];
	pMgr->ptTarget.bottom = (curRect[12]<<24) | (curRect[13]<<16) | (curRect[14]<<8) | curRect[15];
	//printf("left=%d  right=%d  top=%d  bottom=%d\n", pMgr->ptTarget.left, pMgr->ptTarget.right, pMgr->ptTarget.top, pMgr->ptTarget.bottom);
	lWidth = pMgr->ptTarget.right - pMgr->ptTarget.left + 1;
	lHeight = pMgr->ptTarget.bottom - pMgr->ptTarget.top + 1;
	if (lWidth<=1 || lWidth>lImgWidth || lHeight<=1 || lHeight>lImgHeight)
	{
		printf("cur target error!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	pTmpData = pTrainTaregtMem;
	WriteMem(pTmpData, trainRect, 16);
	pMgr->ptTrainTarget.left = (trainRect[0]<<24) | (trainRect[1]<<16) | (trainRect[2]<<8) | trainRect[3];
	pMgr->ptTrainTarget.right = (trainRect[4]<<24) | (trainRect[5]<<16) | (trainRect[6]<<8) | trainRect[7];
	pMgr->ptTrainTarget.top = (trainRect[8]<<24) | (trainRect[9]<<16) | (trainRect[10]<<8) | trainRect[11];
	pMgr->ptTrainTarget.bottom = (trainRect[12]<<24) | (trainRect[13]<<16) | (trainRect[14]<<8) | trainRect[15];
	//printf("left=%d  right=%d  top=%d  bottom=%d\n", pMgr->ptTrainTarget.left, pMgr->ptTrainTarget.right, pMgr->ptTrainTarget.top, pMgr->ptTrainTarget.bottom);
	lWidth = pMgr->ptTrainTarget.right - pMgr->ptTrainTarget.left + 1;
	lHeight = pMgr->ptTrainTarget.bottom - pMgr->ptTrainTarget.top + 1;
	if (lWidth<=1 || lWidth>lImgWidth || lHeight<=1 || lHeight>lImgHeight)
	{
		printf("train target error!\n");
		res = LI_ERR_UNKNOWN;
		return res;
	}

	return res;
}


/*===========================================================
������	��GetClassIndex
����		����ȡ����������Ӧ�Ŀ�ĵ�ַ
�������˵����
name��������
ptDescriptorsGroup����������               
�������˵����
��
����ֵ˵��������������Ӧ�ĵ�ַλ��
===========================================================*/
MLong GetClassIndex(PTDescriptorsGroup ptDescriptorsGroup,MChar *name)
{
	MLong i;
	PTDescriptorClass ptDescriptorClassTemp;
	if (ptDescriptorsGroup->lNumClass==0)
	{
		return -1;//��Ϊ��
	}
	for (i=0;i<ptDescriptorsGroup->lNumClass;i++)
	{
		ptDescriptorClassTemp=ptDescriptorsGroup->ptDescriptorClass+i;
		if (strcmp(ptDescriptorClassTemp->pClassName,name)==0)
		{
			return i;//�ҵ��ಢ�������λ��
		}
	}
	return -1;//û���ҵ�
}
/*===========================================================
������	��AddClasses
����		��������������������
�������˵����
hMemMgr:�������
name��������
ptDescriptorsGroup����������               
�������˵����
ptDescriptorsGroup����������
����ֵ˵����������������
===========================================================*/
MRESULT AddClasses(MHandle hMemMgr,PTDescriptorsGroup ptDescriptorsGroup,MChar *name)
{
	MRESULT res=LI_ERR_NONE;
	TDescriptorClass tDescriptorClass={0};
	ptDescriptorsGroup->lNumClass++;
	if (ptDescriptorsGroup->lNumClass>MAX_DESCRITPTOR_NUMBER)
	{
		res=LI_ERR_CLASS_SIZE_TOO_BIG;
		goto EXT;
	}
	//ptDescriptorsGroup->ptDescriptorClass+ptDescriptorsGroup->lNumClass-1;
	tDescriptorClass.lClassIndex=ptDescriptorsGroup->lNumClass-1;
	tDescriptorClass.lNumDescriptors=0;
	strcpy(tDescriptorClass.pClassName,name);
	AllocVectMem(hMemMgr,tDescriptorClass.ptDescriptor,MAX_DESCRITPTOR_NUMBER,TDescriptor);
	*(ptDescriptorsGroup->ptDescriptorClass+ptDescriptorsGroup->lNumClass-1)=tDescriptorClass;
EXT:
	if (res==LI_ERR_CLASS_SIZE_TOO_BIG)
	{
		ptDescriptorsGroup->lNumClass=MAX_DESCRITPTOR_NUMBER;
	}
	if (res==LI_ERR_ALLOC_MEM_FAIL)
	{
		(ptDescriptorsGroup->lNumClass)--;
	}
	return res;	
}
/*===========================================================
������	��HYMRB_Init
����		����ʼ������ʼ�������������������
�������˵����
hMemMgr:�������
ptDescriptorsGroup����������

�������˵����
pMRBreg���������
����ֵ˵����������������
===========================================================*/
MRESULT HYAMR_Init (MHandle hMemMgr,MHandle *pMRBreg)
{
	MRESULT res=LI_ERR_NONE;
	PAMMETERINFO pMgr = MNull;

	AllocVectMem(hMemMgr, pMgr, 1, AMMETERINFO);
	SetVectMem(pMgr, 1, 0, AMMETERINFO);
	pMgr->hMemMgr = hMemMgr;
	pMgr->tDescriptorsGroup.lNumClass = 0;
	pMgr->bRefresh = MTrue;
	pMgr->ptTarget.bottom = pMgr->ptTarget.top = pMgr->ptTarget.left = pMgr->ptTarget.right = 0;
	pMgr->ptTrainTarget.bottom = pMgr->ptTrainTarget.top = pMgr->ptTrainTarget.left = pMgr->ptTrainTarget.right = 0;

	AllocVectMem(hMemMgr,pMgr->tDescriptorsGroup.ptDescriptorClass,MAX_CLASS_NUMBER,TDescriptorClass);

	pMgr->bExistCircle = MFalse;
	pMgr->lNumPts = 30;				//���11����
	//pMgr->lMaxArcAngle = 180;		//�������Ļ���165

	GO(CreateCircleInfo(hMemMgr, &pMgr->AmmeterCircle, pMgr->lNumPts*10));

	*pMRBreg=(MHandle)pMgr;
EXT:
	if (res<0)
	{
		if (pMgr)
		{
			FreeVectMem(hMemMgr, pMgr->tDescriptorsGroup.ptDescriptorClass);
			FreeCicleInfo(hMemMgr, &(pMgr->AmmeterCircle));
		}
		FreeVectMem(hMemMgr, pMgr);
	}
	return res;	
}


MRESULT HYAMR_SetParam(MHandle hHYMRDHandle, HYLPAMR_IMAGES pMaskImage, HYAMR_INTERACTIVE_PARA *pInPara)
{
	MRESULT res=LI_ERR_NONE;
	MLong xc,yc,r,i;
	MLong lLeft, lTop;
	JOFFSCREEN tmpImage;
	BLOCK tmpBlock = {0};
	PAMMETERINFO pMgr=(PAMMETERINFO)hHYMRDHandle;
	MLong lRate;
	MLong tmpR=0;

	if (pMgr==MNull)
	{
		pMgr->bExistCircle = MFalse;
		return LI_ERR_NOT_INIT;
	}

	if (pInPara->lPtNum < 2)
	{
		pMgr->bExistCircle = MFalse;
		return LI_ERR_NOT_INIT;
	}

	lLeft = pMgr->ptTarget.left;
	lTop = pMgr->ptTarget.top;
	if(pInPara->lPtNum>2)
	{
		xc = pInPara->circleCoord.x-lLeft;
		yc = pInPara->circleCoord.y-lTop;
	}
	else
	{
		xc = -10000;
		yc = -10000;
	}
	for (i=0; i<pInPara->lPtNum; i++)
	{
		pInPara->ptPosList[i].x -= lLeft;
		pInPara->ptPosList[i].y -= lTop;
	}

	//if (SWITCH_METER_PTNUM == pInPara->lPtNum)
	//{   // ����ָ��ʽ���أ�����ֻ��������ǵ�  ����ȡ���е�ٶ�Ϊ��Բ�ġ���
	//	// �뾶��ȡ���������Ա���ϵ����lRate����ʵ�������ȡ��Ŀ����Ϊ�˱�֤ʶ����׼ȷ
	//	lRate = 3;
	//	xc = (pInPara->ptPosList[0].x + pInPara->ptPosList[1].x)>>1;
	//	yc = (pInPara->ptPosList[0].y + pInPara->ptPosList[1].y)>>1;
	//	r = vDistance3L(pInPara->ptPosList[0], pInPara->ptPosList[1]) * lRate;
	//}
	//else
	//{
	//	res = vCircleFitting(pInPara->ptPosList, pInPara->lPtNum, &xc, &yc, &r);
	//	if (LI_ERR_NONE != res)
	//	{
	//		return res = LI_ERR_INVALID_PARAM;
	//	}
	//}

	if(pInPara->lPtNum>2)
	{
		r =0 ;
		for (i=0; i<pInPara->lPtNum; i++)
		{
			tmpR =sqrt((MDouble)(pInPara->ptPosList[i].x-xc)*(pInPara->ptPosList[i].x-xc)+
				(pInPara->ptPosList[i].y-yc)*(pInPara->ptPosList[i].y-yc));
			r +=tmpR;
		}
		r /=pInPara->lPtNum;
	}
	else
	{
		r =0;	
		tmpR =sqrt((MDouble)(pInPara->ptPosList[0].x-pInPara->ptPosList[1].x)*(pInPara->ptPosList[0].x-pInPara->ptPosList[1].x)+
			(pInPara->ptPosList[0].y-pInPara->ptPosList[1].y)*(pInPara->ptPosList[0].y-pInPara->ptPosList[1].y));
		r +=tmpR;
		r /=pInPara->lPtNum;
	}
	pMgr->AmmeterCircle.circleParam.xcoord =xc ;
	pMgr->AmmeterCircle.circleParam.ycoord =yc;
	pMgr->AmmeterCircle.circleParam.lRadius = r;
	pMgr->AmmeterCircle.lConfidence = 2000;
	pMgr->AmmeterCircle.lLineNum = 0;
	pMgr->AmmeterCircle.lPtNum = pInPara->lPtNum;
	for (i=0;i<pMgr->AmmeterCircle.lPtNum; i++)
	{
		pMgr->AmmeterCircle.pPtinfo[i].ptPoint.x = pInPara->ptPosList[i].x;
		pMgr->AmmeterCircle.pPtinfo[i].ptPoint.y = pInPara->ptPosList[i].y;
		if(pInPara->ptPosList[i].x==xc && pInPara->ptPosList[i].y==yc)
		{
			res = LI_ERR_UNKNOWN;
			break;
		}
		pMgr->AmmeterCircle.pPtinfo[i].dPtAngle = vComputeAngle(pInPara->ptPosList[i].x-xc, pInPara->ptPosList[i].y-yc);
	}
	pMgr->bExistCircle = MTrue;

	// ����ʶ��׶ε�maskģ��ͼ
	tmpImage = _TransToInteriorImgFmt(pMaskImage);
	TransGrayImgToBlock(tmpImage, &tmpBlock);
	pInPara->lMaskWidth = tmpBlock.lWidth;
	pInPara->lMaskHeight = tmpBlock.lHeight;
	pInPara->lMaskStride = tmpBlock.lBlockLine;
	pInPara->lMaskDataType = tmpBlock.typeDataA;
	pInPara->pMaskData = tmpBlock.pBlockData;

	return res;
}
/*===========================================================
������	��HYMRB_Uninit
����		�������������ͷ��ڴ�
�������˵����
hHYMRDHandle:�������

�������˵����               
����ֵ˵����������������
===========================================================*/
MRESULT HYAMR_Uninit (MHandle hHYMRDHandle)
{
	MRESULT res=LI_ERR_NONE;
	PTDescriptorClass ptDescriptorClass;
	PTDescriptor ptDescriptor;
	MLong i,j;
	PAMMETERINFO pMgr=(PAMMETERINFO)hHYMRDHandle;
	MHandle hMemMgr=pMgr->hMemMgr;
	// TDescriptorsGroup tDescriptorGroupTemp=(pMgr->tDescriptorsGroup);
	PTDescriptorsGroup ptDescriptorGroupTemp = &(pMgr->tDescriptorsGroup);
	for (i=0;i<ptDescriptorGroupTemp->lNumClass;i++)
	{
		ptDescriptorClass=ptDescriptorGroupTemp->ptDescriptorClass+i;

		ptDescriptor=ptDescriptorClass->ptDescriptor;
		for (j=0;j<ptDescriptorClass->lNumDescriptors;j++,ptDescriptor++)
		{
			FreeVectMem(hMemMgr,ptDescriptor->pData);
		}
		FreeVectMem(hMemMgr,ptDescriptorClass->ptDescriptor);
	}
	FreeVectMem(hMemMgr,ptDescriptorGroupTemp->ptDescriptorClass);
	FreeCicleInfo(hMemMgr, &(pMgr->AmmeterCircle));
	FreeVectMem(hMemMgr,pMgr);
	return res;
}

/*===========================================================
������	��HYMRB_GetTemplateFromText
����		�����ļ��л�ȡ��������
�������˵����
hHYMRDHandle:�������
path���ļ�����


�������˵����ptDescriptorsGroup����������               
����ֵ˵����������������

===========================================================*/
MRESULT HYAMR_GetTemplateFromText (MHandle hHYMRDHandle,char* path)
{
	FILE *fp=fopen(path,"r");

	MLong i;
	MLong j;
	MLong k;

	MRESULT res=LI_ERR_NONE;
	MLong lNumClass=0;
	MLong lClassIndex=0;
	MLong lDescripotrIndex=0;
	MLong lNumDescriptors=0;
	MLong lData=0;

	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	//PTMatchRigidMGR pMgr=(PTMatchRigidMGR)hHYMRDHandle;
	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	MHandle hMemMgr=pMgr->hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp=&(pMgr->tDescriptorsGroup);
	if (fp==MNull)
	{
		return -1;
	}
	fscanf(fp,"%d",&lNumClass);
	ptDescriptorsGroupTemp->lNumClass=lNumClass;
	for (i=0;i<lNumClass;i++)
	{
		fscanf(fp,"%d %d",&lClassIndex,&lNumDescriptors);
		ptDescirptorClass=ptDescriptorsGroupTemp->ptDescriptorClass+i;
		ptDescirptorClass->lClassIndex=lClassIndex;
		ptDescirptorClass->lNumDescriptors=lNumDescriptors;
		fscanf(fp,"%s",ptDescirptorClass->pClassName);
		ptDescirptorClass->ptDescriptor=MNull;
		AllocVectMem(hMemMgr,ptDescirptorClass->ptDescriptor,MAX_DESCRITPTOR_NUMBER,TDescriptor);
		for (j=0;j<lNumDescriptors;j++)
		{
			//tDescriptor={0};
			ptDescriptor=ptDescirptorClass->ptDescriptor+j;
			ptDescriptor->pData=MNull;
			//fscanf(fp,"%d",&(tDescriptor.lDescriptorIndex));
			//fscanf(fp,"%d %d %d %d %d",&(tDescriptor.lSample),&(tDescriptor.lWidth),&(tDescriptor.lHeight),&(tDescriptor.lRegion),&(tDescriptor.lAngle));
			ptDescriptor->lNumber=0;
			fscanf(fp,"%d",&(ptDescriptor->lDescriptorIndex));
			fscanf(fp,"%d %d %d %d %d",&(ptDescriptor->lSample),&(ptDescriptor->lWidth),&(ptDescriptor->lHeight),&(ptDescriptor->lRegion),&(ptDescriptor->lAngle));
			AllocVectMem(hMemMgr,ptDescriptor->pData,(ptDescriptor->lWidth)*(ptDescriptor->lHeight),MUInt8);
			for (k=0;k<(ptDescriptor->lWidth)*(ptDescriptor->lHeight);k++)
			{
				fscanf(fp,"%d",&lData);
				//*(ptDescriptor->pData++)=lData;
				//				*(ptDescriptor->pData+k)=lData;
				*(ptDescriptor->pData+k) = TRIM_UINT8(lData);
			}
			//*ptDescriptor=tDescriptor;
		}
	}
EXT:
	fclose(fp);
	return res;

}

// ���ڴ��ж�ȡ����ƥ����������Ϣ
MLong HYAMR_GetDesMem (MHandle hHYMRDHandle, unsigned char *pDesMem)
{
	int res = 0;
	int i,j;
	unsigned char head[4];
	unsigned char *inPtr = MNull;
	MLong lNum;

	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	PAMMETERINFO pMgr;
	MHandle hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp;

	pMgr = (PAMMETERINFO)hHYMRDHandle;
	hMemMgr=pMgr->hMemMgr;
	ptDescriptorsGroupTemp=&(pMgr->tDescriptorsGroup);

	if (MNull==pDesMem)
	{
		Err_Print("HYAMR_GetDesMem  inDesMem=NULL!\n");
		res = -1;
		goto EXT;
	}
	inPtr = pDesMem;
	WriteMem(inPtr, head, 4);
	inPtr += 4;
	ptDescriptorsGroupTemp->lNumClass = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];
	for (i=0; i<ptDescriptorsGroupTemp->lNumClass; i++)
	{
		ptDescirptorClass=ptDescriptorsGroupTemp->ptDescriptorClass+i;
		WriteMem(inPtr, head, 4);
		inPtr += 4;
		ptDescirptorClass->lClassIndex = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

		WriteMem(inPtr, head, 4);
		inPtr += 4;
		ptDescirptorClass->lNumDescriptors = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

		WriteMem(inPtr, ptDescirptorClass->pClassName, 16);
		inPtr += 16;

		ptDescirptorClass->ptDescriptor=MNull;
		AllocVectMem(hMemMgr,ptDescirptorClass->ptDescriptor,MAX_DESCRITPTOR_NUMBER,TDescriptor);
		for (j=0; j<ptDescirptorClass->lNumDescriptors; j++)
		{
			ptDescriptor=ptDescirptorClass->ptDescriptor+j;
			ptDescriptor->pData=MNull;

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lDescriptorIndex = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lSample = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lWidth = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lHeight = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lRegion = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			WriteMem(inPtr, head, 4);
			inPtr += 4;
			ptDescriptor->lAngle = (head[0]<<24) | (head[1]<<16) | (head[2]<<8) | head[3];

			lNum = (ptDescriptor->lWidth)*(ptDescriptor->lHeight);
			AllocVectMem(hMemMgr,ptDescriptor->pData, lNum, MUInt8);
			WriteMem(inPtr, ptDescriptor->pData, lNum);
			inPtr += lNum;
		}
	}

EXT:
	return res;
}

/*===========================================================
������	��UpdateTDescriptor
����		���������������и�����������˳�򣨲������ʹ���ǰ�Ĳ��ԣ�
�������˵����
ptDescriptorClass����������
index����Ҫ���µ���������index


�������˵����ptDescriptorClass����������              
����ֵ˵������
===========================================================*/
MVoid UpdateTDescriptor(PTDescriptorClass ptDescriptorClass,MLong index)
{
	MLong i;
	MLong j;
	TDescriptor tDescriptor={0};
	for (i=0;i<ptDescriptorClass->lNumDescriptors;i++)
	{
		if ((ptDescriptorClass->ptDescriptor+i)->lDescriptorIndex==index)
		{
			break;
		}
	}
	tDescriptor=*(ptDescriptorClass->ptDescriptor+i);
	for (j=i;j>=1;j--)
	{
		*(ptDescriptorClass->ptDescriptor+j)=*(ptDescriptorClass->ptDescriptor+j-1);
	}
	*(ptDescriptorClass->ptDescriptor)=tDescriptor;
}



/*===========================================================
������	��HYMRB_MemMgrCreate
����		����ȡָ�����ȵ��ڴ�ռ�
�������˵����
pMem��ָ��			
lMemSize������              
�������˵����pMem��ָ��
����ֵ˵������
===========================================================*/
MHandle  HYAMR_MemMgrCreate (MVoid * pMem, MLong lMemSize)
{
	return JMemMgrCreate(pMem, lMemSize);
}

MVoid HYAMR_MemMgrDestroy (MHandle hMemMgr)
{
	JMemMgrDestroy(hMemMgr);
}

/*===========================================================
������	��UpdateTDescriptorNumber
����		���������������и�����������˳�򣨲���ʹ������ǰ�Ĳ��ԣ�
�������˵����
ptDescriptorClass����������
index����Ҫ���µ���������index


�������˵����ptDescriptorClass����������              
����ֵ˵������
===========================================================*/
MVoid UpdateTDescriptorNumber(PTDescriptorClass ptDescriptorClass,MLong index)
{
	MLong i;
	MLong j;
	TDescriptor tDescriptor={0};

	for (i=0;i<ptDescriptorClass->lNumDescriptors;i++)
	{
		if ((ptDescriptorClass->ptDescriptor+i)->lDescriptorIndex==index)
		{
			(ptDescriptorClass->ptDescriptor+i)->lNumber++;

			break;
		}
	}
	//tDescriptor=*(ptDescriptorClass->ptDescriptor+i);
	//ptDescriptor=ptDescriptorClass->ptDescriptor+i;
	for (j=i;j>0;j--)
	{
		if ((ptDescriptorClass->ptDescriptor+j-1)->lNumber<(ptDescriptorClass->ptDescriptor+j)->lNumber)
		{
			tDescriptor=*(ptDescriptorClass->ptDescriptor+j-1);
			*(ptDescriptorClass->ptDescriptor+j-1)=*(ptDescriptorClass->ptDescriptor+j);
			*(ptDescriptorClass->ptDescriptor+j)=tDescriptor;
		}
		else
		{
			break;
		}
	}
	//*(ptDescriptorClass->ptDescriptor)=tDescriptor;

}


MRESULT HYAMR_GetDescriptors (MHandle hHYMRDHandle,HYAMR_TDescriptorsGroup **pptDescriptorGroup)
{
	MRESULT res=LI_ERR_NONE;
	MHandle hMemMgr=MNull;
	//PTMatchRigidMGR pMgr=MNull;
	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	if (hHYMRDHandle==MNull)
	{
		res=LI_ERR_NOT_INIT;
		goto EXT;
	}
	else
	{
		pMgr=(PAMMETERINFO)hHYMRDHandle;
		*pptDescriptorGroup=(HYAMR_TDescriptorsGroup*)(&(pMgr->tDescriptorsGroup));
	}
EXT:
	return res;
}

MRESULT HYAMR_SetDescriptors (MHandle hHYMRDHandle,HYAMR_TDescriptorsGroup *ptDescriptorGroup)
{
	MRESULT res=LI_ERR_NONE;
	MHandle hMemMgr=MNull;
	PAMMETERINFO pMgr=MNull;
	PTDescriptorClass ptDescriptorClass,ptDescriptorClassTemp;
	PTDescriptor ptDescriptor,ptDescriptorTemp;
	PTDescriptorsGroup ptDescriptorsGroupTemp=MNull;
	PTDescriptorsGroup ptDescriptorsGroupSource=(PTDescriptorsGroup)ptDescriptorGroup;
	MLong i=0,j=0;
	MLong lNumClass=0;

	if (hHYMRDHandle==MNull)
	{
		res=LI_ERR_NOT_INIT;
		goto EXT;
	}
	else
	{
		pMgr=(PAMMETERINFO)hHYMRDHandle;
		ptDescriptorsGroupTemp=&(pMgr->tDescriptorsGroup);
		lNumClass=ptDescriptorsGroupSource->lNumClass;
		ptDescriptorsGroupTemp->lNumClass=ptDescriptorsGroupSource->lNumClass;
		ptDescriptorClassTemp=ptDescriptorsGroupTemp->ptDescriptorClass;
		ptDescriptorClass=ptDescriptorsGroupSource->ptDescriptorClass;
		//ptDescriptorsGroupTemp->ptDescriptorClass=

		for (i=0;i<lNumClass;i++)
		{
			ptDescriptorClassTemp->lNumDescriptors=ptDescriptorClass->lNumDescriptors;
			ptDescriptorClassTemp->lClassIndex=ptDescriptorClass->lClassIndex;
			JMemCpy(ptDescriptorClassTemp->pClassName,ptDescriptorClass->pClassName,128);
			ptDescriptorClassTemp->ptDescriptor=MNull;
			AllocVectMem(hMemMgr,ptDescriptorClassTemp->ptDescriptor,MAX_DESCRITPTOR_NUMBER,TDescriptor);
			for (j=0;j<ptDescriptorClassTemp->lNumDescriptors;j++)
			{
				ptDescriptor=ptDescriptorClass->ptDescriptor+j;
				ptDescriptorTemp=ptDescriptorClassTemp->ptDescriptor+j;

				ptDescriptorTemp->lAngle=ptDescriptor->lAngle;
				ptDescriptorTemp->lDescriptorIndex=ptDescriptor->lDescriptorIndex;
				ptDescriptorTemp->lHeight=ptDescriptor->lHeight;
				ptDescriptorTemp->lWidth=ptDescriptor->lWidth;
				ptDescriptorTemp->lRegion=ptDescriptor->lRegion;
				ptDescriptorTemp->lSample=ptDescriptor->lSample;
				ptDescriptorTemp->lNumber=ptDescriptor->lNumber;
				ptDescriptorTemp->pData=MNull;
				AllocVectMem(hMemMgr,ptDescriptorTemp->pData,(ptDescriptorTemp->lWidth)*(ptDescriptorTemp->lHeight),MUInt8);
				JMemCpy(ptDescriptorTemp->pData,ptDescriptor->pData,(ptDescriptorTemp->lWidth)*(ptDescriptorTemp->lHeight));
			}
			ptDescriptorClassTemp++;
			ptDescriptorClass++;
		}


	}
EXT:
	return res;
}

// ��ֵ���㣬��Ҫ��Ϊ�˷�ֹ����֡ͼ���ʶ���������쳣����
// pBuffer  ��Ŷ���������ڴ��ַ
// lBufferLen  �Ѵ�ŵĶ����������
MDouble HYAMR_FindMidian (MDouble *pBuffer, MLong lBufferLen)
{
	MDouble dTmpVal;
	MLong i, j;
	MLong lIndex;
	MDouble dMedVal;

	if(MNull==pBuffer || 0==lBufferLen)
	{
		Err_Print("HYAMR_FindMidian init error!\n");
		return -1;
	}

	for (i=0; i<lBufferLen-1;i++)
	{
		for(j=i+1; j<lBufferLen; j++)
		{
			if (*(pBuffer+i)>*(pBuffer+j))
			{
				dTmpVal = *(pBuffer+i);
				*(pBuffer+i) = *(pBuffer+j);
				*(pBuffer+j) = dTmpVal;
			}
		}
	}

	lIndex = lBufferLen>>1;
	if (lBufferLen%2 == 0)
	{
		dMedVal = (*(pBuffer+lIndex) + *(pBuffer+lIndex-1))/2;
		return dMedVal;
	}
	else
		return (*(pBuffer+lIndex));
}

//����ָ���߿�ȣ���������ʶ��׶�haarģ��ߴ���
// 1 �ж�ָ������ɫ�����Ϊ��ɫ������ɫ��֣�����RGBֱ��ת�Ҷ�ͼ
// 2 ���ָ����Ϊ��ɫ�����ù�ʽ128-*pData�����Ե�����򣬵���canny�㷨�����Ե
// 3 ���ݽ����׶Σ���ָ�����ϵı�ǵ㣬��8��������Ѱ�ұ�Ե�㣬������Լ���ָ���߿��
// 4 ���ۼӵ��߿��ȡ��ֵ��Ϊ���յ�ָ���߿��
MRESULT HYAMR_CalcHaarWidth(MHandle hHYMRDHandle, HYLPAMR_IMAGES pSrcImg, HY_LINE_TYPE *pLineColor, MPOINT *pPtList, MLong lPtNum, MLong *lHaarWidth)
{
	MRESULT res = LI_ERR_NONE;

	PAMMETERINFO pMgr = (PAMMETERINFO)hHYMRDHandle;
	MLong lSearchThreshold;
	MLong lWidth, lHeight, lStride;
	MLong lExt;
	MByte *pData, *pEdgeData;
	//MLong lLineStartWidth, lLineEndWidth;
	BLOCK grayBlockImg = {0}, edgeImg = {0};
	JOFFSCREEN ImgSrc = {0}, ImgSrcGray = {0};
	MLong i, j;
	MLong lTmpWidthArray[4];
	MLong lDirectionLength[8];
	MPOINT ptDirection[8];
	MPOINT tmpPt;

	MLong dist[3];

	// red diff image
	BLOCK diffBlockImage = {0};
	MLong histArray[256] = {0};

	MLong isYUV = 1;
	JOFFSCREEN ImgSrcYUV420 = {0};
	JOFFSCREEN yuvTmp = {0};
	MByte *srcY, *srcV, *srcU;
	MLong lObjWidth, lObjHeight;

	if (MNull==pPtList || 0>=lPtNum || MNull==lHaarWidth)
	{
		res = LI_ERR_NOT_INIT;
		goto EXT;
	}

	ImgSrc = _TransToInteriorImgFmt(pSrcImg);
	GO(ImgCreate(pMgr->hMemMgr, &ImgSrcGray, FORMAT_GRAY, ImgSrc.dwWidth, ImgSrc.dwHeight));
	GO(ImgFmtTrans(&ImgSrc, &ImgSrcGray));

	if (HY_LINE_RED == pLineColor[0])
	{
		lObjWidth = pMgr->ptTarget.right - pMgr->ptTarget.left + 1;
		lObjHeight = pMgr->ptTarget.bottom - pMgr->ptTarget.top + 1;
		GO(B_Create(pMgr->hMemMgr, &diffBlockImage, DATA_U8, lObjWidth, lObjHeight));
		GO(ImgCreate(pMgr->hMemMgr, &ImgSrcYUV420, FORMAT_YUV420_VUVU, ImgSrc.dwWidth, ImgSrc.dwHeight));
		GO(ImgFmtTrans(&ImgSrc, &ImgSrcYUV420));
		yuvTmp = ImgSrcYUV420;
		ImgChunky2Plannar(&yuvTmp);

		srcY = (MByte*)ImgSrcYUV420.pixelArray.chunky.pPixel + ImgSrcYUV420.pixelArray.chunky.dwImgLine * pMgr->ptTarget.top + pMgr->ptTarget.left;
		srcV = (MByte*)yuvTmp.pixelArray.planar.pPixel[1] + yuvTmp.pixelArray.planar.dwImgLine[1] * (pMgr->ptTarget.top>>1) + ((pMgr->ptTarget.left>>1)<<1);
		srcU = srcV + 1;

		getRedDiffImage2_yuv(srcY, srcV, ImgSrcYUV420.pixelArray.chunky.dwImgLine, &diffBlockImage, histArray);
		//B_Cpy(&grayBlockImg, &diffBlockImage);
		grayBlockImg.lWidth = diffBlockImage.lWidth;
		grayBlockImg.lHeight = diffBlockImage.lHeight;
		grayBlockImg.lBlockLine = diffBlockImage.lBlockLine;
		grayBlockImg.pBlockData = diffBlockImage.pBlockData;
		grayBlockImg.typeDataA = diffBlockImage.typeDataA;
		PrintBmpEx(grayBlockImg.pBlockData, grayBlockImg.lBlockLine, DATA_U8, grayBlockImg.lWidth, grayBlockImg.lHeight, 1, ".\\log\\yuv2gray.bmp");
	}
	else
	{
		TransGrayImgToBlock(ImgSrcGray, &grayBlockImg);
	}

	pData = grayBlockImg.pBlockData;
	lWidth = grayBlockImg.lWidth;
	lHeight = grayBlockImg.lHeight;
	lStride = grayBlockImg.lBlockLine;
	lExt = lStride - lWidth;
	lSearchThreshold = MAX(5, (MIN(lWidth, lHeight))>>5);

	GO(B_Create(pMgr->hMemMgr, &edgeImg, DATA_U8, lWidth, lHeight));
	if (HY_LINE_RED == pLineColor[0])
	{
		pEdgeData = (MByte *)edgeImg.pBlockData;
		for (j=0; j<lHeight; j++, pData+=lExt, pEdgeData+=lExt)
		{
			for (i=0; i<lWidth; i++, pData++, pEdgeData++)
			{
				*pEdgeData = 128 - *pData;  // ������ͼʱ�򣬽���������ֵ��Ϊ128����ɫָ����������ֵ����128
			}
		}
		PrintBmpEx(edgeImg.pBlockData, edgeImg.lBlockLine, DATA_U8, edgeImg.lWidth, edgeImg.lHeight, 1, ".\\log\\edge_aa.bmp");
		for (i=0; i<lPtNum; i++)
		{
			(pPtList + i)->x -= pMgr->ptTarget.left;
			(pPtList + i)->y -= pMgr->ptTarget.top;
		}
	}
	else
	{
		GO(Edge(pMgr->hMemMgr, (MByte*)grayBlockImg.pBlockData, lStride, lWidth, lHeight,
			(MByte*)edgeImg.pBlockData, edgeImg.lBlockLine, TYPE_CANNY));
		PrintBmpEx(edgeImg.pBlockData, edgeImg.lBlockLine, DATA_U8, edgeImg.lWidth, edgeImg.lHeight, 1, ".\\log\\edge.bmp");
	}

	*lHaarWidth = 0;
	for (i=0; i<lPtNum; i++)
	{
		tmpPt = *(pPtList + i);
		searchPt(&edgeImg, tmpPt, 0, lSearchThreshold, &ptDirection[0], &lDirectionLength[0]);  // 8��������
		searchPt(&edgeImg, tmpPt, 1, lSearchThreshold, &ptDirection[1], &lDirectionLength[1]);
		searchPt(&edgeImg, tmpPt, 2, lSearchThreshold, &ptDirection[2], &lDirectionLength[2]);
		searchPt(&edgeImg, tmpPt, 3, lSearchThreshold, &ptDirection[3], &lDirectionLength[3]);
		searchPt(&edgeImg, tmpPt, 4, lSearchThreshold, &ptDirection[4], &lDirectionLength[4]);
		searchPt(&edgeImg, tmpPt, 5, lSearchThreshold, &ptDirection[5], &lDirectionLength[5]);
		searchPt(&edgeImg, tmpPt, 6, lSearchThreshold, &ptDirection[6], &lDirectionLength[6]);
		searchPt(&edgeImg, tmpPt, 7, lSearchThreshold, &ptDirection[7], &lDirectionLength[7]);
		lTmpWidthArray[0] = lDirectionLength[0] + lDirectionLength[4];
		lTmpWidthArray[1] = lDirectionLength[1] + lDirectionLength[5];
		lTmpWidthArray[2] = lDirectionLength[2] + lDirectionLength[6];
		lTmpWidthArray[3] = lDirectionLength[3] + lDirectionLength[7];

		dist[i] = MAX(4, MIN(lTmpWidthArray[0], MIN(lTmpWidthArray[1], MIN(lTmpWidthArray[2], lTmpWidthArray[3]))));
		// ����������� ����Ҫ�Է��صı�Ե��ptDirection�����ж� �Ƿ�Ϊָ���߱�Ե  ����δ���ж�
		*lHaarWidth += dist[i];
	}
	*lHaarWidth /=lPtNum;
	/*if(dist[1]-4>=dist[0] || dist[2]-8>=dist[0])
	{
	*lHaarWidth =4;
	}
	else
	{
	*lHaarWidth /= lPtNum;
	}*/
	//*lHaarWidth = 2;

EXT:
	ImgRelease(pMgr->hMemMgr, &ImgSrcGray);
	if (HY_LINE_RED == pLineColor[0])
	{
		B_Release(pMgr->hMemMgr, &diffBlockImage);
		ImgRelease(pMgr->hMemMgr, &ImgSrcYUV420);
	}
	B_Release(pMgr->hMemMgr, &edgeImg);
	return res;
}