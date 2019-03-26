#ifndef _HYAMR_METER_REG_H
#define _HYAMR_METER_REG_H
#include"amcomdef.h"

//#define ENABLE_DLL

#ifdef ENABLE_DLL
#define METERREG_API	__declspec(dllexport)
#else
#define	METERREG_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	/* Defines for image color format*/
#define HYAMR_IMAGE_GRAY				0x00		//chalk original 0x0A 	
#define HYAMR_IMAGE_YUYV               0x02        //y,u,y,v,y,u,y,v......
#define HYAMR_IMAGE_RGB                0x04        //r,g,b,r,g,b.....
#define HYAMR_IMAGE_BGR                0x06        //b,g,r,b,g,r.....

#define MR_READ_FILE_TARGET "C:\\Target.dat"
#define MR_TRAIN_TARGET "C:\\trainTarget.dat"

#define SWITCH_METER_PTNUM 2

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
} HYAMR_IMAGES, *HYLPAMR_IMAGES;



/*��������Ԫ*/
typedef struct  
{
	MLong lWidth;
	MLong lHeight;
	MLong lRegion;
	MLong lSample;
	MLong lAngle;
	MLong lDescriptorIndex;
	MLong lNumber;
	MUInt8 *pData;	
}HYAMR_TDescriptor,*HYAMR_PTDescriptor;

/*��������*/
typedef struct 
{
	MLong  lNumDescriptors;
	MLong lClassIndex;	
	MChar pClassName[128];
	HYAMR_PTDescriptor ptDescriptor;
}HYAMR_TDescriptorClass,*HYAMR_PTDescriptorClass;

/*���������ļ���*/
typedef struct 
{
	MLong lNumClass;
	HYAMR_PTDescriptorClass ptDescriptorClass;	
}HYAMR_TDescriptorsGroup,*HYAMR_PTDescriptorsGroup;


#define HYAMR_MAX_PT_LIST	30
typedef struct
{
	//ֱ�ߵ�������������ǣ��ؾ࣬�Ƿ�ֱ�������߶εķ���
	MDouble CoeffK;//out
	MDouble CoeffB;//out
	MBool bVert;//out
	MDouble dtAngle;//out
	//xsd
	MPOINT ptStart;
	MPOINT ptEnd;

	//�̶ȵ������
	MPOINT ptList[HYAMR_MAX_PT_LIST];//in/out
	MDouble dValList[HYAMR_MAX_PT_LIST];
	MLong lptNum;				//in/out
}HYAMR_PATTERN_OUT;

typedef struct
{
	MPOINT ptList[HYAMR_MAX_PT_LIST];
	MLong lptNum;
}HYAMR_PATTERN_IN;

//xsd
typedef struct
{
	MPOINT ptPos;	//��̶ȵ�λ��
	MDouble ptVal;	//��̶ȵ�ֵ
	MDouble ptDist;	//��̶ȵ��ֱ��ĩ�˾���
	MLong ptIndex;	//��̶ȵ�����ֵ
}HYAMR_PT_INFO;

typedef struct
{
	//ֱ�߲���
	MDouble dCoeffK;
	MDouble dCoeffB;
	MBool bVert;
	MDouble dAngle;

	MPOINT ptStart;
	MPOINT ptEnd;

	//��̶ȵ�
	HYAMR_PT_INFO ptInfo[HYAMR_MAX_PT_LIST];
	MLong lPtNum;
}HYAMR_READ_PARA;

typedef struct
{
	MPOINT ptPos[20];
	MDouble angleVal[20];
	MLong index;
}HYAMR_PT_POS;

typedef enum
{
	HY_LINE_BLACK = 0,
	HY_LINE_WHITE,
	HY_LINE_RED,
	HY_LINE_YELLOW
}HY_LINE_TYPE;

typedef enum
{
	HY_RECT = 0,
	HY_RING,
	HY_RING_LINE,
	HY_UNKNOWN_MASKTYPE
}HY_MASK_TYPE;

typedef enum
{
	HY_LOW_BLUR = 0,
	HY_HIGH_BLUR,
	HY_OTHER_BLUR
}HY_BLUR_LEVEL;

// ���������ṹ��
typedef struct
{
	// �̶ȵ��ǡ��̶�ֵ������
	MPOINT ptPosList[HYAMR_MAX_PT_LIST];
	MDouble dPtValList[HYAMR_MAX_PT_LIST];
	MLong lPtNum;

	MPOINT circleCoord;   //Բ������
	//��̶���ʼ��
	//MPOINT ptStart;

	MLong Dist ;   //Բ����Ȧ�뾶��ȥ��Ȧ�뾶

	//ָ������������ɫ�����뱳��֮��Ҷ�ֵǿ��
	MLong lLineNum;
	HY_LINE_TYPE lLineColor[2];
	MBool bWhiteBackground;	// true����ǿ(white)��ָ��(black) haar����ʱ sum(����)-sum(ָ��)/2

	//ָ���߿��
	MLong lLineWidth;

	//Mask����
	MLong lMaskWidth;
	MLong lMaskHeight;
	MLong lMaskStride;
	MByte *pMaskData;
	MLong lMaskDataType;

	//ָ����ǿ���Ƿ������
	HY_BLUR_LEVEL  picLevel;

}HYAMR_INTERACTIVE_PARA;


/* Defines for error*/
#define HYAMR_ERR_NONE					       0
#define HYAMR_ERR_UNKNOWN				      -1
#define HYAMR_ERR_IMAGE_FORMAT			      -101
#define HYAMR_ERR_IMAGE_SIZE_UNMATCH	          -102
#define HYAMR_ERR_ALLOC_MEM_FAIL		          -201
#define HYAMR_ERR_NO_FIND			          -1201
#define HYAMR_CLASS_NOT_EXIT                   -1301
#define HYAMR_ERR_CLASS_SIZE_TOO_BIG           -1401
#define HYAMR_ERR_INDEX_NO_FIND                -1501
#define HYAMR_ERR_SEEDS_NUMBER                 -1601
#define HYAMR_ERR_INVALID_POINT                -1701
#define HYAMR_ERR_INVALID_NUMBERDATA           -1801
//===========================================================

//===========================================================
//#define HYAMR_MemMgrCreate PX(HYAMR_MemMgrCreate)
//===========================================================

/*===========================================================
������	��HYAMR_MemMgrCreate
����		����ȡָ�����ȵĲ������
�������˵����
				pMem��ָ��			
				lMemSize������              
�������˵����pMem��ָ��
����ֵ˵������
===========================================================*/
METERREG_API MHandle HYAMR_MemMgrCreate(MVoid * pMem, MLong lMemSize);

/*===========================================================
������	��HYAMR_MemMgrDestroy
����		���ͷ��ڴ�ռ�
�������˵����
				hMemMgr��HYMRB_MemMgrCreate�л�õ�ָ��				            
����ֵ˵������
===========================================================*/
METERREG_API MVoid HYAMR_MemMgrDestroy(MHandle hMemMgr);

/*===========================================================
������	��HYAMR_Init
����		����ʼ������ʼ���������
�������˵����
				hMemMgr:�������
				              
�������˵����
               pMRBreg���������
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_Init(MHandle hMemMgr,MHandle *pMRBreg);

/*===========================================================
������	��HYAMR_Uninit
����		�������������ͷ��ڴ�
�������˵����
				hHYMRDHandle:�������
				              
�������˵����               
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_Uninit(MHandle hHYMRDHandle);

/*===========================================================
������	��HYAMR_TrainTemplateFromMask
����		������ģ���Լ�Maskͼ�����ѵ������ȡ������
����ȫ�ֱ�������
�������˵����
               hHYMRDHandle�� �������������HYMRB_init�г�ʼ��
               pImage�� ѵ��ģ��ͼ��
			   pMask��  ѵ��Maskͼ��
			   pClassName����ģ����������
			   lParma:ѵ���ķ�����0����ͼ�������ת����1��ͼ�������ת����
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_TrainTemplateFromMask(MHandle hHYMRDHandle,HYLPAMR_IMAGES pImage,HYLPAMR_IMAGES pMask,MChar *pClassName,MLong lParma);

/*===========================================================
������	��HYAMR_ReadNumber
����		�����������
����ȫ�ֱ�������
�������˵����
               hHYMRDHandle�� �������������HYMRB_init�г�ʼ��
               pImage�� ������ͼ��			  
			   pClassName����ģ����������
			   threshold:ƥ����ֵ
			   pointResultIn:��̶ȵ�λ��
			   numInPoint����̶ȵ����
			   numData:��̶�ֵ
			   result:�������
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_GetMeterResult(MHandle hHYMRDHandle, HYAMR_READ_PARA readPara, MDouble *pResult);

/*===========================================================
������	��HYAMR_GetPoint
����		����ȡ�����д�̶ȵ�λ��
����ȫ�ֱ�������
�������˵����
               hHYMRDHandle�� �������������HYMRB_init�г�ʼ��
               pImage�� ѵ��ͼ��
			   pClassName����ģ����������
			   threshold:ƥ����ֵ
			   pointResultIn:��̶ȵ�λ��
			   pNumInPoint����̶ȵ����
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_GetObjRect(MHandle hHYMRDHandle,HYLPAMR_IMAGES pImage,MChar *pClassName, MDouble threshold);

METERREG_API MRESULT HYAMR_GetPointerLine(MHandle hHYMRDHandle, HYLPAMR_IMAGES pImage, MChar *pClassName, MDouble threshold, 
											HYAMR_READ_PARA *readPara, MLong lMeterType);

METERREG_API MRESULT HYAMR_GetLineInfo(MHandle hHYMRDHandle, HYLPAMR_IMAGES pImage, MChar *pClassName, MDouble threshold,
										HYAMR_READ_PARA *pLinePara, MLong lMeterType);
METERREG_API MRESULT HYAMR_GetLineInfo_mem(MHandle hHYMRDHandle, HYLPAMR_IMAGES pImage, MChar *pClassName, MDouble threshold,
									  HYAMR_READ_PARA *pLinePara, MLong lMeterType, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem);
//METERREG_API MRESULT HYAMR_GetSwitchLine(MHandle hHYMRDHandle, HYLPAMR_IMAGES pImage, MChar *pClassName, MDouble threshold, HYAMR_READ_PARA *pLinePara);
METERREG_API MRESULT HYAMR_GetLineParam(MHandle hHYMRDHandle, HYLPAMR_IMAGES pSrcImage, HYAMR_INTERACTIVE_PARA *pPara,
										MChar *pClass, MDouble dThreshold, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, HYAMR_READ_PARA *pLinePara);
/*===========================================================
������	��HYAMR_SetParam
����		�����ñ����д�̶ȵ�λ��
����ȫ�ֱ�������
�������˵����
               hHYMRDHandle�� �������������HYMRB_init�г�ʼ��
             		   pPatternIn������ĵ���б����pPatternIn==NULL��˵�������ÿ̶ȵ�λ��
             		   ���pPatternIn!=NULL��������ǿ̶ȵ�λ���б�
����ֵ˵�����������
===========================================================*/
METERREG_API  MRESULT HYAMR_SetParam(MHandle hHYMRDHandle, HYLPAMR_IMAGES pMaskImage, HYAMR_INTERACTIVE_PARA *pInPara);
//METERREG_API  MRESULT HYAMR_SetParam(MHandle hHYMRDHandle,	HYAMR_PATTERN_IN *pPatternIn, MLong *lx, MLong *ly, MLong *lr);		

/*===========================================================
������	��HYAMR_SaveTDescriptorsGroup
����		�����������鱣�浽�ļ���
�������˵����
				path�������ļ�������
				hHYMRDHandle:�������               
�������˵����
               ��
����ֵ˵�����������
===========================================================*/
METERREG_API MLong HYAMR_SaveTDescriptorsGroup(MHandle hHYMRDHandle,const char *path);
METERREG_API MLong HYAMR_SaveDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize);

/*===========================================================
������	��HYAMR_GetTemplateFromText
����		�����ļ��л�ȡ��������
�������˵����
				hHYMRDHandle:�������
				path���ļ�����
				
				              
�������˵����ptDescriptorsGroup����������               
����ֵ˵�����������
===========================================================*/
METERREG_API MRESULT HYAMR_GetTemplateFromText(MHandle hHYMRDHandle,char* path);
METERREG_API MLong HYAMR_GetDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem);

/******************************target info***************************************/
METERREG_API MRESULT HYAMR_SaveTargetInfo(MHandle hHYMRDHandle,const char *path, const char *trainPath, MBool isTrain);
METERREG_API MRESULT HYAMR_GetTargetInfo(MHandle hHYMRDHandle,char* path, char *trainPath, MLong lImgWidth, MLong lImgHeight);

METERREG_API MRESULT HYAMR_SaveTargetInfoToMem(MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, MBool isTrain);
METERREG_API MRESULT HYAMR_GetTaregetInfoFromMem(MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTaregtMem, MLong lImgWidth, MLong lImgHeight);
/********************************************************************************/

METERREG_API MRESULT HYAMR_GetDescriptors(MHandle hHYMRDHandle,HYAMR_TDescriptorsGroup **pptDescriptorGroup);

METERREG_API MRESULT HYAMR_SetDescriptors(MHandle hHYMRDHandle,HYAMR_TDescriptorsGroup *ptDescriptorGroup);

// �������ȡ��ֵ ��ֹ����������������
METERREG_API MDouble HYAMR_FindMidian (MDouble *pBuffer, MLong lBufferLen);

METERREG_API MRESULT HYAMR_CalcHaarWidth(MHandle hHYMRDHandle, HYLPAMR_IMAGES pSrcImg, HY_LINE_TYPE *pLineColor, MPOINT *pPtList, MLong lPtNum, MLong *lHaarWidth);

METERREG_API MRESULT HYAMR_SaveTargetInfoToMemTmp (MHandle hHYMRDHandle, unsigned char *pCurTargetMem, unsigned char *pTrainTargetMem, MBool isTrain,int *rect );

/************************************************************************/
/* The function used to get version information of face tracking library.*/ 
/************************************************************************/
typedef struct
{
	MLong lCodebase;			// Codebase version number 
	MLong lMajor;				// major version number 
	MLong lMinor;				// minor version number
	MLong lBuild;				// Build version number, increasable only
	const MChar *Version;		// version in string form
	const MChar *BuildDate;	// latest build Date
	const MChar *CopyRight;	// copyright 
} HYAMR_Version;
METERREG_API const HYAMR_Version * HYAMeterReg_GetVersion();


#ifdef __cplusplus
}
#endif
#endif