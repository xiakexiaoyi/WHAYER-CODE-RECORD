#include <stdio.h>
#include <string.h>
#include "HYL_MatchRigidBody.h"
#include "MatchRigidBody.h"
#include "lidebug.h"
#include "liimgfmttrans.h"
#include "liimage.h"
#include "limath.h"

#define MAX_DESCRITPTOR_NUMBER 1000             //一个类中的最多描述符的数量
#define MAX_CLASS_NUMBER 100                    //描述符集合中最多类的数量

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


MVoid _TransGrayImgToBlock(JOFFSCREEN Image,PBLOCK pBlockImg)
{
	pBlockImg->lBlockLine=Image.pixelArray.chunky.dwImgLine;
	pBlockImg->lHeight=Image.dwHeight;
	pBlockImg->lWidth=Image.dwWidth;
	pBlockImg->typeDataA=DATA_U8;
	pBlockImg->pBlockData=Image.pixelArray.chunky.pPixel;
}

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

typedef struct tag_HYLHandle
{
	MHandle hMemMgr;
	//PMPOINT offset;
	TDescriptorsGroup tDescriptorsGroup;
}MRParam;
/*===========================================================
函数名	：HYL_MemMgrCreate
功能		：获取指定长度的内存空间
输入参数说明：
				pMem：指针			
				lMemSize：长度              
输出参数说明：pMem：指针
返回值说明：无
===========================================================*/
MHandle  HYL_MemMgrCreate (MVoid * pMem, MLong lMemSize)
{
	 return JMemMgrCreate(pMem, lMemSize);
}

MVoid HYL_MemMgrDestroy (MHandle hMemMgr)
{
	JMemMgrDestroy(hMemMgr);
}

MRESULT HYL_MatchInit(MHandle hMemMgr, MHandle *pMRHandle)
{
	MRESULT res = LI_ERR_NONE;

	MRParam *pTLHandle = NULL;
	if (!pMRHandle) return -1;
	AllocVectMem(hMemMgr, pTLHandle,1, MRParam);
	SetVectMem(pTLHandle, 1, 0, MRParam);
	pTLHandle->hMemMgr=hMemMgr;
	pTLHandle->tDescriptorsGroup.lNumClass= 0;
	AllocVectMem(hMemMgr,pTLHandle->tDescriptorsGroup.ptDescriptorClass,MAX_CLASS_NUMBER,TDescriptorClass);
	//AllocVectMem(hMemMgr,pTLHandle->offset,1,MPOINT);
	*pMRHandle = pTLHandle;
EXT:
	if (res<0)
	{
		if (pTLHandle)
		{
			FreeVectMem(hMemMgr, pTLHandle->tDescriptorsGroup.ptDescriptorClass);
			//FreeVectMem(hMemMgr, pTLHandle->offset);
		}
		FreeVectMem(hMemMgr, pTLHandle);
	}
	return res;
}

MRESULT   HYL_MatchUninit(MHandle hMRHandle)
{
	int i,j;
	PTDescriptorClass ptDescriptorClass;
	PTDescriptor ptDescriptor;
	MRParam *pTLHandle = (MRParam*)hMRHandle;
	if (pTLHandle == MNull) return -1;
	MHandle hMemMgr=pTLHandle->hMemMgr;
	PTDescriptorsGroup ptDescriptorGroupTemp = &(pTLHandle->tDescriptorsGroup);
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
	//FreeVectMem(pTLHandle->hMemMgr,pTLHandle->tDescriptorsGroup.ptDescriptorClass);
	//FreeVectMem(pTLHandle->hMemMgr, pTLHandle->offset);
	FreeVectMem(pTLHandle->hMemMgr, pTLHandle);
	return 1;
}

/*===========================================================
函数名	：GetClassIndex
功能		：获取类名称所对应的块的地址
输入参数说明：
				name：类名称
				ptDescriptorsGroup：描述符块               
输出参数说明：
               无
返回值说明：类名称所对应的地址位置
===========================================================*/
MLong GetClassIndex(PTDescriptorsGroup ptDescriptorsGroup,MChar *name)
{
	MLong i;
	PTDescriptorClass ptDescriptorClassTemp;
	if (ptDescriptorsGroup->lNumClass==0)
	{
		return -1;//类为空
	}
	for (i=0;i<ptDescriptorsGroup->lNumClass;i++)
	{
		ptDescriptorClassTemp=ptDescriptorsGroup->ptDescriptorClass+i;
		if (strcmp((char *)ptDescriptorClassTemp->pClassName,name)==0)
		{
			return i;//找到类并返回类的位置
		}
	}
	return -1;//没有找到
}
/*===========================================================
函数名	：AddClasses
功能		：向描述符块中增加类
输入参数说明：
				hMemMgr:操作句柄
				name：类名称
				ptDescriptorsGroup：描述符块               
输出参数说明：
               ptDescriptorsGroup：描述符块
返回值说明：发生错误的类别
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
	strcpy((char *)tDescriptorClass.pClassName,name);
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
函数名	：HYMRB_TrainTemplateFromMask
功能		：根据模板以及Mask图像进行训练，获取描述符
引用全局变量：无
输入参数说明：
               hHYMRDHandle： 操作句柄，需在HYMRB_init中初始化
               pImage： 训练模板图像
			   pMask：  训练Mask图像
			   pClassName：该模板的类的名称
			   lParma:训练的方法。0不对图像进行旋转处理，1对图像进行旋转处理
返回值说明：发生错误的类别
===========================================================*/
MRESULT HYL_TrainTemplateFromMask (MHandle HYLHandle,HYL_PIMAGES pImage,HYL_PIMAGES pMask,MChar *pClassName,MLong lParma)
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
	MRParam* pMgr = (MRParam*)HYLHandle;
	MHandle hMemMgr=MNull;
	PTDescriptorsGroup ptDescriptorsGroupsTemp;

	if (HYLHandle==MNull)
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
	_TransGrayImgToBlock(ImgSrcGray, &BLOCKImage);
	_TransGrayImgToBlock(ImgMaskGray, &BLOCKMask);

//	Logger("HYAMR_TrainTemplateFromMask...\n");

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
	//PrintBmpEx(BLOCKImage.pBlockData, BLOCKImage.lBlockLine, DATA_U8, BLOCKImage.lWidth, BLOCKImage.lHeight, 1, "D:\\blockImg.bmp");
	//PrintBmpEx(BLOCKMask.pBlockData, BLOCKMask.lBlockLine, DATA_U8, BLOCKMask.lWidth, BLOCKMask.lHeight, 1, "D:\\blockMask.bmp");
	lClassIndex=GetClassIndex(ptDescriptorsGroupsTemp,pClassName);
	if (lClassIndex==-1)
	{
		GO(AddClasses(hMemMgr,ptDescriptorsGroupsTemp,pClassName));
		lClassIndex=ptDescriptorsGroupsTemp->lNumClass-1;
	}
	ptDescriptorClass=ptDescriptorsGroupsTemp->ptDescriptorClass+lClassIndex;//获取对应类描述符的指针


	GO(TrainTemplateMethod(hMemMgr,&BLOCKImage,&BLOCKMask,lParma,ptDescriptorClass));//训练模板获得描述符

//	Logger("HYAMR_TrainTemplateFromMask OK...\n");

EXT:
//	ImgRelease(hMemMgr,&ImgSrcBGR);
	ImgRelease(hMemMgr,&ImgSrcGray);
	ImgRelease(hMemMgr,&ImgMaskGray);
//	ImgRelease(hMemMgr,&ImgMaskBGR);
	return res;
}

/*===========================================================
函数名	：UpdateTDescriptor
功能		：更新描述符类中各个描述符的顺序（采用最近使用最靠前的策略）
输入参数说明：
				ptDescriptorClass：描述符类
				index：需要更新的描述符的index
				
				              
输出参数说明：ptDescriptorClass：描述符类              
返回值说明：无
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
MRESULT HYL_GetDashboard(MHandle HYLHandle,HYL_PIMAGES pImage,MChar *pClassName,MDouble threshold,PMPOINT offset)
{
	MRESULT res = LI_ERR_NONE;

	MRParam* pMgr = (MRParam*)HYLHandle;
	BLOCKEXT BextImg = {0};
	JOFFSCREEN ImgSrc = {0}, ImgSrcGray = {0};
	TOutParam outParam = {0};
	MLong lClassIndex;
	PTDescriptorsGroup ptDescriptorsGroup = MNull;
	PTDescriptorClass ptDescriptorClass = MNull;
	ImgSrc = _TransToInteriorImgFmt(pImage);
	GO(ImgCreate(pMgr->hMemMgr, &ImgSrcGray, FORMAT_GRAY, ImgSrc.dwWidth, ImgSrc.dwHeight));
	GO(ImgFmtTrans(&ImgSrc, &ImgSrcGray));
	_TransGrayImgToBlock(ImgSrcGray, (PBLOCK)&BextImg);
	ptDescriptorsGroup = &(pMgr->tDescriptorsGroup);
	if(0==ptDescriptorsGroup->ptDescriptorClass->ptDescriptor->lWidth
		&& 0==ptDescriptorsGroup->ptDescriptorClass->ptDescriptor->lHeight)
	{
		res = LI_ERR_INVALID_POINT;
		goto EXT;
	}
	lClassIndex = GetClassIndex(ptDescriptorsGroup, pClassName);
	if (-1==lClassIndex)
	{
		res = LI_ERR_CLASS_NOT_EXIT;
		goto EXT;
	}
	ptDescriptorClass = ptDescriptorsGroup->ptDescriptorClass + lClassIndex;
	outParam.lClassNumber = lClassIndex;
	GO(MatchRigidBody(pMgr->hMemMgr, (BLOCK*)&BextImg, ptDescriptorClass, threshold, &outParam));
	UpdateTDescriptor(ptDescriptorClass, outParam.lIndex);
	//pMgr->ptTarget.left = outParam.lX - (outParam.lWidth>>1);
	//pMgr->ptTarget.right = outParam.lX + (outParam.lWidth>>1);
	//pMgr->ptTarget.top = outParam.lY - (outParam.lHeight>>1);
	//pMgr->ptTarget.bottom = outParam.lY + (outParam.lHeight>>1);
	offset->x=outParam.lX;
	offset->y=outParam.lY;

	//下面是匹配后结果，匹配展示
	/*BextImg.ext.left = outParam.lX - (outParam.lWidth>>1);
	BextImg.ext.top = outParam.lY - (outParam.lHeight>>1);
	BextImg.ext.right = BextImg.block.lWidth - outParam.lX - (outParam.lWidth>>1);
	BextImg.ext.bottom = BextImg.block.lHeight-outParam.lY - (outParam.lHeight>>1);
	BextImg.block = BE_ValidBlock(&BextImg);
	PrintBmpEx(BextImg.block.pBlockData, BextImg.block.lBlockLine, DATA_U8, BextImg.block.lWidth, BextImg.block.lHeight, 1, "D:\\src1.bmp");*/
EXT:
	ImgRelease(pMgr->hMemMgr,	&ImgSrcGray);
	return res;
}

/*===========================================================
函数名	：HYMRB_SaveTDescriptorsGroup
功能		：将描述符块保存到文件中
输入参数说明：
				path：保存文件的名称
				hHYMRDHandle:操作句柄               
输出参数说明：
               无
返回值说明：发生错误的类别
===========================================================*/
MRESULT HYL_SaveDesMem (MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize)
{
	int i,j,k;
	int lTmpSize;
	unsigned char head[4];
	unsigned char *outPtr;
	MRParam* pMgr;
	MHandle hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp;	
	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	MLong lTmpNum;

	pMgr = (MRParam*) hHYMRDHandle;
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

	lTmpSize = 4;
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

MRESULT HYL_GetDesMem (MHandle hHYMRDHandle, unsigned char *pDesMem)
{
	int res = 0;
	int i,j,k;
	unsigned char head[4];
	unsigned char *inPtr = MNull;
	MLong lNum;

	PTDescriptorClass ptDescirptorClass;
	PTDescriptor ptDescriptor;
	MRParam* pMgr;
	MHandle hMemMgr;
	PTDescriptorsGroup ptDescriptorsGroupTemp;

	pMgr = (MRParam*)hHYMRDHandle;
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
函数名	：HYMRB_SaveTDescriptorsGroup
功能		：将描述符块保存到文件中
输入参数说明：
				path：保存文件的名称
				hHYMRDHandle:操作句柄               
输出参数说明：
               无
返回值说明：发生错误的类别
===========================================================*/
MRESULT HYL_SaveTDescriptorsGroup (MHandle hHYMRDHandle,const char *path)
{
	FILE *fp=fopen(path,"w");

	MLong i;
	MLong j;
	MLong k;
	MRParam* pMgr = (MRParam*) hHYMRDHandle;
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

/*
#define AllocVectMem_S(hMemMgr, pMem, len, TYPE)						\
{																	\
	JASSERT((len) > 0);												\
	printf("S1111\n");												\
	JASSERT((pMem) == MNull);										\
	printf("S22222\n");												\
	JMTMemMgrEnter();												\
	printf("S33333 len=%d\n",len);												\
	(pMem) = (TYPE*)JMemAlloc(hMemMgr, (len)*sizeof(TYPE));			\
	printf("S4444\n");												\
	JMTMemMgrLeave();												\
	printf("S555555\n");												\
	if((pMem) == MNull)												\
	{																\
	printf("S6666666(pMem) == MNull\n");												\
		res = LI_ERR_ALLOC_MEM_FAIL;								\
		goto EXT;													\
	}																\
}
*/
/*===========================================================
函数名	：HYMRB_GetTemplateFromText
功能		：从文件中获取描述符块
输入参数说明：
				hHYMRDHandle:操作句柄
				path：文件名称
				
				              
输出参数说明：ptDescriptorsGroup：描述符块               
返回值说明：发生错误的类别

===========================================================*/
MRESULT HYL_GetTemplateFromText (MHandle hHYMRDHandle,char* path)
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
	MRParam* pMgr = (MRParam*)hHYMRDHandle;
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

