// testBed.cpp : main project file.

#include"amcomdef.h"
#include"HY_IMAGEQUALITY.h"

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include <opencv/highgui.h>
#include <time.h>

#define WORK_BUFFER 2560*1920*5
#define STATIC_MEM
#define JLINE_BYTES(Width, BitCt)    (((long)(Width) * (BitCt) + 31) / 32 * 4)

int main(int argc, char* argv[])
{
	MHandle HYIQHandle=MNull;
#ifdef STATIC_MEM
	MVoid *pMem=malloc(WORK_BUFFER);
	MHandle hMemMgr = HYIQ_MemMgrCreate(pMem,WORK_BUFFER);
#else
	MHandle hMemMgr = MNull;
#endif

	IQ_IMAGES imgiq1={0};
	IQ_IMAGES imgiq2={0};

	HYIQ_TOutParam p1={0};

	IplImage *pImage1;//视频 前一帧
	IplImage *pImage2;//视频 当前帧

	int nImgnum=0;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char filePath[MAX_PATH]="D:\\20151125眉山\\100\\";
	char filePath_const[MAX_PATH];
	char fileName1[MAX_PATH];
	char fileName2[MAX_PATH];

	HYIQ_Init(hMemMgr,&HYIQHandle);

	strcpy(filePath_const,filePath);
	strcat(filePath,"\*.jpg");
	hFind = FindFirstFile(filePath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		printf ("FindFirstFile failed (%d)\n", GetLastError());
		return -2;
	}

	memset(fileName1,0,MAX_PATH);
	strcpy(fileName1,filePath_const);
	strcat(fileName1,FindFileData.cFileName);
	while (FindNextFile(hFind, &FindFileData))
	{  	
		nImgnum++;

		if (nImgnum==1)
		{
			memset(fileName2,0,MAX_PATH);
			strcpy(fileName2,filePath_const);
			strcat(fileName2,FindFileData.cFileName);
		}
		else
		{
			memset(fileName1,0,MAX_PATH);
			strcpy(fileName1,filePath_const);
			strcat(fileName1,FindFileData.cFileName);
			FindNextFile(hFind, &FindFileData);
			memset(fileName2,0,MAX_PATH);
			strcpy(fileName2,filePath_const);
			strcat(fileName2,FindFileData.cFileName);
		}

		//从文件中读取图像  
		pImage1 = cvLoadImage(fileName1/*"D:\\1\\8_0.jpeg"*/, CV_LOAD_IMAGE_UNCHANGED);  
		pImage2 = cvLoadImage(fileName2/*"D:\\1\\8_1.jpeg"*/, CV_LOAD_IMAGE_UNCHANGED);
		//cvShowImage("img",pImage1);
		//cvWaitKey(0);

		imgiq1.lHeight=pImage1->height;
		imgiq1.lWidth=pImage1->width;
		imgiq1.lPixelArrayFormat=HY_IMAGE_BGR;
		imgiq1.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage1->width,pImage1->depth*pImage1->nChannels);
		imgiq1.pixelArray.chunky.pPixel=pImage1->imageData;

		imgiq2.lHeight=pImage2->height;
		imgiq2.lWidth=pImage2->width;
		imgiq2.lPixelArrayFormat=HY_IMAGE_BGR;
		imgiq2.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage2->width,pImage2->depth*pImage2->nChannels);
		imgiq2.pixelArray.chunky.pPixel=pImage2->imageData;//

		p1.CLEAR.CLEARLEVEL=0;
		p1.CLEAR.isDetct=0;
		p1.Bright.BrightLevel1=0;
		p1.Bright.BrightLevel2=0;
		p1.Bright.Brightderiv=0;
		p1.Bright.isDetct=0;
		p1.ImgCast.flag=0;
		p1.ImgCast.CastValue=0.0;
		p1.ImgCast.isDetct=0;
		p1.NOISE.NOISELEVEL=0;
		p1.NOISE.isDetct=0;
		p1.SignalLost.flag=0;
		p1.SignalLost.isDetct=0;
		p1.ImgFrozen.flag=0;
		p1.ImgFrozen.isDetct=0;
		p1.Sigma.sigma1=0;
		p1.Sigma.sigma2=0;


		HYIQ_LOST(HYIQHandle,&imgiq1,&imgiq2,&p1);
		p1.SignalLost.isDetct=1;
		HYIQ_FROZEN(HYIQHandle,&imgiq1,&imgiq2,&p1);
		p1.ImgFrozen.isDetct=1;

		HYIQ_BRIGHT(HYIQHandle,&imgiq1,&imgiq2,&p1);//计算亮度均值和均值差
		p1.Bright.isDetct=1;
		if (p1.Bright.Brightderiv<2/*&&p1.SignalLost.flag==0*/)
		{
			HYIQ_NOISE(HYIQHandle,&imgiq1,&imgiq2,&p1);
			p1.NOISE.isDetct=1;
			if (p1.Bright.BrightLevel1>27.5&&p1.Bright.BrightLevel2>27.5)//过暗
			{
				HYIQ_CLEAR(HYIQHandle,&imgiq1,&imgiq2,&p1);//过暗不检测
				p1.CLEAR.isDetct=1;
				HYIQ_CAST(HYIQHandle,&imgiq1,&p1);//偏色  过亮或过暗不检测
				p1.ImgCast.isDetct=1;
			}
		}

		if(p1.SignalLost.flag&&p1.SignalLost.isDetct)
		{
			printf("image signal is lost!\n");
		}
		if(p1.ImgFrozen.flag&&p1.ImgFrozen.isDetct)
		{
			printf("image is frozen!\n");
		}
		if(p1.Bright.isDetct)
		{
			if(p1.Bright.BrightLevel1<25)
			{
				printf("image is too dark!\n");
			}
			else if(p1.Bright.BrightLevel1>75)
			{
				printf("image is too light!\n");
			}
		}
		if(p1.NOISE.isDetct)
		{
			if(p1.NOISE.NOISELEVEL>3&&p1.NOISE.NOISELEVEL<20)
			{
				printf("image is noised!\n");
			}
			else if(p1.NOISE.NOISELEVEL>20)
			{
				printf("image is heavy noise!\n");
			}
		}
		if(p1.CLEAR.isDetct)
		{
			if(p1.CLEAR.CLEARLEVEL>1.4&&p1.CLEAR.CLEARLEVEL<20)
			{
				printf("image is blur!\n");
			}
			else if(p1.CLEAR.CLEARLEVEL>20)
			{
				printf("image is heavy blur!\n");
			}
		}
		if(p1.ImgCast.isDetct)
		{
			if(p1.ImgCast.CastValue>50)
			{
				printf("image is cast!\n");
			}
		}                
	} 

	HYIQ_Uninit(HYIQHandle);

#ifdef STATIC_MEM
	HYIQ_MemMgrDestroy(hMemMgr);
	free(pMem);
#endif

	return 0;
}