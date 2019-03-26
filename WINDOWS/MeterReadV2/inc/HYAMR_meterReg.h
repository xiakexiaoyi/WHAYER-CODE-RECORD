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



/*描述符单元*/
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

/*类描述符*/
typedef struct 
{
	MLong  lNumDescriptors;
	MLong lClassIndex;	
	MChar pClassName[128];
	HYAMR_PTDescriptor ptDescriptor;
}HYAMR_TDescriptorClass,*HYAMR_PTDescriptorClass;

/*类描述符的集合*/
typedef struct 
{
	MLong lNumClass;
	HYAMR_PTDescriptorClass ptDescriptorClass;	
}HYAMR_TDescriptorsGroup,*HYAMR_PTDescriptorsGroup;


#define HYAMR_MAX_PT_LIST	30
typedef struct
{
	//直线的三个参数，倾角，截距，是否垂直，有向线段的方向
	MDouble CoeffK;//out
	MDouble CoeffB;//out
	MBool bVert;//out
	MDouble dtAngle;//out
	//xsd
	MPOINT ptStart;
	MPOINT ptEnd;

	//刻度点的序列
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
	MPOINT ptPos;	//大刻度点位置
	MDouble ptVal;	//大刻度点值
	MDouble ptDist;	//大刻度点和直线末端距离
	MLong ptIndex;	//大刻度点索引值
}HYAMR_PT_INFO;

typedef struct
{
	//直线参数
	MDouble dCoeffK;
	MDouble dCoeffB;
	MBool bVert;
	MDouble dAngle;

	MPOINT ptStart;
	MPOINT ptEnd;

	//大刻度点
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

// 交互参数结构体
typedef struct
{
	// 刻度点标记、刻度值及个数
	MPOINT ptPosList[HYAMR_MAX_PT_LIST];
	MDouble dPtValList[HYAMR_MAX_PT_LIST];
	MLong lPtNum;

	MPOINT circleCoord;   //圆心坐标
	//大刻度起始点
	//MPOINT ptStart;

	MLong Dist ;   //圆环内圈半径减去外圈半径

	//指针线条数、颜色，及与背景之间灰度值强弱
	MLong lLineNum;
	HY_LINE_TYPE lLineColor[2];
	MBool bWhiteBackground;	// true背景强(white)于指针(black) haar计算时 sum(背景)-sum(指针)/2

	//指针线宽度
	MLong lLineWidth;

	//Mask类型
	MLong lMaskWidth;
	MLong lMaskHeight;
	MLong lMaskStride;
	MByte *pMaskData;
	MLong lMaskDataType;

	//指针线强度是否可区分
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
函数名	：HYAMR_MemMgrCreate
功能		：获取指定长度的操作句柄
输入参数说明：
				pMem：指针			
				lMemSize：长度              
输出参数说明：pMem：指针
返回值说明：无
===========================================================*/
METERREG_API MHandle HYAMR_MemMgrCreate(MVoid * pMem, MLong lMemSize);

/*===========================================================
函数名	：HYAMR_MemMgrDestroy
功能		：释放内存空间
输入参数说明：
				hMemMgr：HYMRB_MemMgrCreate中获得的指针				            
返回值说明：无
===========================================================*/
METERREG_API MVoid HYAMR_MemMgrDestroy(MHandle hMemMgr);

/*===========================================================
函数名	：HYAMR_Init
功能		：初始化，初始化操作句柄
输入参数说明：
				hMemMgr:操作句柄
				              
输出参数说明：
               pMRBreg：操作句柄
返回值说明：错误代码
===========================================================*/
METERREG_API MRESULT HYAMR_Init(MHandle hMemMgr,MHandle *pMRBreg);

/*===========================================================
函数名	：HYAMR_Uninit
功能		：函数结束，释放内存
输入参数说明：
				hHYMRDHandle:操作句柄
				              
输出参数说明：               
返回值说明：错误代码
===========================================================*/
METERREG_API MRESULT HYAMR_Uninit(MHandle hHYMRDHandle);

/*===========================================================
函数名	：HYAMR_TrainTemplateFromMask
功能		：根据模板以及Mask图像进行训练，获取描述符
引用全局变量：无
输入参数说明：
               hHYMRDHandle： 操作句柄，需在HYMRB_init中初始化
               pImage： 训练模板图像
			   pMask：  训练Mask图像
			   pClassName：该模板的类的名称
			   lParma:训练的方法。0不对图像进行旋转处理，1对图像进行旋转处理
返回值说明：错误代码
===========================================================*/
METERREG_API MRESULT HYAMR_TrainTemplateFromMask(MHandle hHYMRDHandle,HYLPAMR_IMAGES pImage,HYLPAMR_IMAGES pMask,MChar *pClassName,MLong lParma);

/*===========================================================
函数名	：HYAMR_ReadNumber
功能		：电流表读数
引用全局变量：无
输入参数说明：
               hHYMRDHandle： 操作句柄，需在HYMRB_init中初始化
               pImage： 待处理图像			  
			   pClassName：该模板的类的名称
			   threshold:匹配阈值
			   pointResultIn:大刻度点位置
			   numInPoint：大刻度点个数
			   numData:大刻度值
			   result:读数结果
返回值说明：错误代码
===========================================================*/
METERREG_API MRESULT HYAMR_GetMeterResult(MHandle hHYMRDHandle, HYAMR_READ_PARA readPara, MDouble *pResult);

/*===========================================================
函数名	：HYAMR_GetPoint
功能		：获取表盘中大刻度点位置
引用全局变量：无
输入参数说明：
               hHYMRDHandle： 操作句柄，需在HYMRB_init中初始化
               pImage： 训练图像
			   pClassName：该模板的类的名称
			   threshold:匹配阈值
			   pointResultIn:大刻度点位置
			   pNumInPoint：大刻度点个数
返回值说明：错误代码
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
函数名	：HYAMR_SetParam
功能		：设置表盘中大刻度点位置
引用全局变量：无
输入参数说明：
               hHYMRDHandle： 操作句柄，需在HYMRB_init中初始化
             		   pPatternIn：输入的点的列表，如果pPatternIn==NULL，说明不设置刻度点位置
             		   如果pPatternIn!=NULL，输入的是刻度的位置列表
返回值说明：错误代码
===========================================================*/
METERREG_API  MRESULT HYAMR_SetParam(MHandle hHYMRDHandle, HYLPAMR_IMAGES pMaskImage, HYAMR_INTERACTIVE_PARA *pInPara);
//METERREG_API  MRESULT HYAMR_SetParam(MHandle hHYMRDHandle,	HYAMR_PATTERN_IN *pPatternIn, MLong *lx, MLong *ly, MLong *lr);		

/*===========================================================
函数名	：HYAMR_SaveTDescriptorsGroup
功能		：将描述符块保存到文件中
输入参数说明：
				path：保存文件的名称
				hHYMRDHandle:操作句柄               
输出参数说明：
               无
返回值说明：错误代码
===========================================================*/
METERREG_API MLong HYAMR_SaveTDescriptorsGroup(MHandle hHYMRDHandle,const char *path);
METERREG_API MLong HYAMR_SaveDesMem(MHandle hHYMRDHandle, unsigned char *pDesMem, MLong lSize, MLong *lDstSize);

/*===========================================================
函数名	：HYAMR_GetTemplateFromText
功能		：从文件中获取描述符块
输入参数说明：
				hHYMRDHandle:操作句柄
				path：文件名称
				
				              
输出参数说明：ptDescriptorsGroup：描述符块               
返回值说明：错误代码
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

// 读数结果取中值 防止连续读数出现跳变
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