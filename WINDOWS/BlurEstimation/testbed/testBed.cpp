// testBed.cpp : main project file.

#include"amcomdef.h"
#include"HY_IMAGEQUALITY.h"

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<iostream>
#include <opencv/cv.h>
#include <opencv/cxcore.h>

#include <opencv/highgui.h>
#include<opencv2/videoio/videoio_c.h>
#include <time.h>
#define WORK_BUFFER 2560*1920*2  
//#define WORK_BUFFER 2561*1921*2     //2560*1920*5
#define STATIC_MEM
#define JLINE_BYTES(Width, BitCt)    (((long)(Width) * (BitCt) + 31) / 32 * 4)
#define  ISVIDEO

#ifdef ISVIDEO
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
	//IQ_IMAGES imgiq_temp={0};
	IQ_IMAGES imgiq1_={0};
	IQ_IMAGES imgiq2_={0};

	HYIQ_TOutParam p1={0};
	CustomParam custopar={0};
	HYIQ_result testresult={0};

	IMQU *pImqu=NULL;
	FILE *fp;
	IplImage *pImage1;//视频 前一帧
	IplImage *pImage2;//视频 当前帧
	//IplImage *pImage2=cvCreateImage(cvSize(2048,1536),IPL_DEPTH_8U,3);

	//CvRect rect;
	CvCapture * capture;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char filePath[MAX_PATH]="..\\..\\testvideo\\";
	char filePath_const[MAX_PATH];
	char fileName[MAX_PATH];

	int nImgnum=0;
	int nVideoNum=0;
	//统计检测结果
	int nNotClearNum=0;
	int nHeavyNotClearNum=0;
	int nNoiseNum=0;
	int nHeavyNoiseNum=0;
	int nTooLightNum=0;
	int nTooDrakNum=0;
	int nCastNum=0;
	int nFrozemNum=0;
	int nLostNum=0;
	int nNormNum=0;
	int nLabelNorm=0;
	//time_t seconds;
	//seconds = time(NULL);
	//printf("自 1970-01-01 起的数 = %ld\n", seconds);

	//输入参数
	custopar.Noiselevel.NoiseLevellowthresh = 4;
	custopar.Noiselevel.Noiselevelhighthresh = 20;
	custopar.ClearLevel.ClearLevellowthresh = 1.4;
	custopar.ClearLevel.ClearLevelhighthresh = 20;
	custopar.Light.darkthresh = -0.5;
	custopar.Light.lightthresh = 0.5;
	custopar.castRatioThresh = 50;

	custopar.ImgFrozen.diff1 = 0.92;
	custopar.ImgFrozen.diff2 = 0.98;
	custopar.ImgFrozen.Hration = 0.7;
	custopar.ImgFrozen.ratio = 0.025;

	custopar.SignalLost.diff = 0.92;
	custopar.SignalLost.blackbackground.grayNum =35;
	custopar.SignalLost.blackbackground.Hration = 0.85;
	custopar.SignalLost.blackbackground.label = 25;
	custopar.SignalLost.nonblackbackground.charcornersnum = 27;
	custopar.SignalLost.nonblackbackground.GradSum = 770.0;
	custopar.SignalLost.nonblackbackground.lostpercent = 0.7;
	custopar.SignalLost.nonblackbackground.meandiff = 70.0;
	custopar.SignalLost.nonblackbackground.priors0 = 0.3;
	custopar.SignalLost.nonblackbackground.priors1 = 0.3;
	custopar.SignalLost.nonblackbackground.sigma0 = 500.0;
	custopar.SignalLost.nonblackbackground.sigma1 = 1500.0;
	//信号丢失参数调整
	/*custopar.SignalLost.nonblackbackground.GradSum = 350.0;
	custopar.SignalLost.nonblackbackground.meandiff = 40.0;
	custopar.SignalLost.nonblackbackground.sigma0 = 600.0;
	custopar.SignalLost.nonblackbackground.priors0 = 0.2;
	custopar.SignalLost.nonblackbackground.lostpercent = 0.55;
	custopar.SignalLost.nonblackbackground.charcornersnum = 10;*/

	nVideoNum=0;
	fp=fopen("..//result.txt","w");
	HYIQ_Init(hMemMgr,&HYIQHandle);

	strcpy(filePath_const,filePath);
	strcat(filePath,"\*.avi");
	hFind = FindFirstFile(filePath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		printf ("FindFirstFile failed (%d)\n", GetLastError());
		return -2;
	}
	memset(fileName,0,MAX_PATH);
	strcpy(fileName,filePath_const);
	strcat(fileName,FindFileData.cFileName);
	fprintf(fp,"\n\n%s\n",fileName);
	do
	{
		nVideoNum++;
		if (nVideoNum>1)
		{
			memset(fileName,0,MAX_PATH);
			strcpy(fileName,filePath_const);
			strcat(fileName,FindFileData.cFileName);
			fprintf(fp,"\n%s\n",fileName);
		} 
		//printf("fileName=%s\n",fileName);
		capture = cvCreateFileCapture (fileName);  //读取视频
		if(capture==NULL) 
		{
			printf("NO capture");    //读取不成功，则标识
			return 1;
		}; 
		pImage2=cvQueryFrame(capture);              //抓取帧
		//cvResize(pImage2_cur,pImage2,0);

		//for 统计结果
		nImgnum=0;
		nNotClearNum=0;
		nHeavyNotClearNum=0;
		nNoiseNum=0;
		nHeavyNoiseNum=0;
		nTooLightNum=0;
		nTooDrakNum=0;
		nCastNum=0;
		nFrozemNum=0;
		nLostNum=0;
		nNormNum=0;

		while(/*nImgnum<100*/1)
		{
			nImgnum++;
			nLabelNorm=0;
			pImage1=cvCloneImage(pImage2);
			pImage2=cvQueryFrame(capture);              //抓取帧
			//cvResize(pImage2_cur,pImage2,0);
			if (!pImage2)//视频是否读取完毕
			{
				if (pImage1 != NULL)
				{
					cvReleaseImage(&pImage1);
					pImage1 = NULL;
				}
				break;
			}

			cvShowImage("img2",pImage2);
			cvWaitKey(1);

			imgiq1.lHeight=pImage1->height;
			imgiq1.lWidth=pImage1->width;
			imgiq1.lPixelArrayFormat=HY_IMAGE_BGR;
			imgiq1.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage1->width,pImage1->depth*pImage1->nChannels);
			imgiq1.pixelArray.chunky.pPixel=pImage1->imageData;

			imgiq2.lHeight=pImage2->height;
			imgiq2.lWidth=pImage2->width;
			imgiq2.lPixelArrayFormat=HY_IMAGE_BGR;
			imgiq2.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage2->width,pImage2->depth*pImage2->nChannels);
			imgiq2.pixelArray.chunky.pPixel=pImage2->imageData;

			p1.CLEAR.CLEARLEVEL=0;
			p1.CLEAR.isDetct=0;
			p1.Bright.BrightLevel1=0;
			p1.Bright.BrightLevel2=0;
			p1.Bright.Brightderiv=0;
			p1.Bright.isDetct=0;
			p1.ImgCast.flag=0;
			p1.ImgCast.isDetct=0;
			p1.NOISE.NOISELEVEL=0;
			p1.NOISE.isDetct=0;
			p1.SignalLost.flag=0;
			p1.SignalLost.isDetct=0;
			p1.ImgFrozen.flag=0;
			p1.ImgFrozen.isDetct=0;

			testresult.clearstatus=0;
			testresult.caststatus=0;
			testresult.lightstatus=0;
			testresult.Noisestatus=0;
			testresult.signalLoststatus=0;
			testresult.imgFrozenstatus=0;

			//HYIQ_CUTIMAGE(&imgiq1_,&imgiq1);  //切图，上下各切掉0.2*height,去除字符块区域，函数里面有了
			//HYIQ_CUTIMAGE(&imgiq2_,&imgiq2);

			//pImqu=(IMQU *)HYIQHandle;
			HYIQ_LOST(HYIQHandle,&imgiq1,&imgiq2,&p1, &custopar);
			p1.SignalLost.isDetct=1;   //判断是否检测过，为后面结果输出函数HYIQ_RESULT提供参数
			//HYIQ_FROZEN(HYIQHandle, &imgiq1, &imgiq2, &p1, &custopar);//50帧测一次？
			//p1.ImgFrozen.isDetct = 1;
				//HYIQ_BRIGHT(HYIQHandle,&imgiq1_,&imgiq2_,&p1);//计算亮度均值和均值差
				HYIQ_BRIGHT(HYIQHandle, &imgiq1, &imgiq2, &p1);
				p1.Bright.isDetct=1;
				if (p1.Bright.Brightderiv<2&&p1.SignalLost.flag==0)
				{
					//HYIQ_NOISE(HYIQHandle,&imgiq1_,&imgiq2_,&p1);
					HYIQ_NOISE(HYIQHandle, &imgiq1, &imgiq2, &p1);
					p1.NOISE.isDetct=1;
					//test
					//if (p1.Bright.BrightLevel1>-0.45&&p1.Bright.BrightLevel2>-0.45)//过暗   过亮呢？
					//{
						//HYIQ_CLEAR(HYIQHandle,&imgiq1_,&imgiq2_,&p1);//过暗不检测
						HYIQ_CLEAR(HYIQHandle, &imgiq1, &imgiq2, &p1);
						p1.CLEAR.isDetct=1;
						//HYIQ_CAST(HYIQHandle,&imgiq1_,&p1);//偏色  过亮或过暗不检测
						HYIQ_CAST(HYIQHandle, &imgiq1, &p1);
						p1.ImgCast.isDetct=1;
					//}
				}

				HYIQ_RESULT(HYIQHandle,&p1,&custopar,&testresult);

				switch(testresult.clearstatus)
				{
				case(IMAGE_CLEAR):
					//fprintf(fp,"Image is clear\n");
					nLabelNorm++;
					break;
				case(IMAGE_NOTCLEAR):
					//fprintf(fp,"Image is not clear\n");
					nNotClearNum++;
					break;
				case(IMAGE_HEVNOTCLEAR):
					//fprintf(fp,"Image is heavy not clear \n");
					nHeavyNotClearNum++;
					break;
				default:
					break;
				}

				switch(testresult.Noisestatus)
				{
				case(IMAGE_UNNOISE):
					//fprintf(fp,"Image is not noise\n");
					nLabelNorm++;
					break;
				case(IMAGE_NOISE):
					//fprintf(fp,"Image is  noised\n");
					nNoiseNum++;
					break;
				case(IMAGE_HEAVENOISE):
					//fprintf(fp,"Image is heavy noise\n");
					nHeavyNoiseNum++;
					break;
				default:
					break;
				}

				switch(testresult.lightstatus)
				{
				case(IMAGE_TOODARK):
					//fprintf(fp,"Image is too dark\n");
					nTooDrakNum++;
					break;
				case(IMAGE_TOOLIGHT):
					//fprintf(fp,"Image is too light\n");
					nTooLightNum++;
					break;
				default:
					nLabelNorm++;
					break;
				}

				switch(testresult.caststatus)
				{
				case(IMAGE_CAST):
					//fprintf(fp,"Image is cast\n");
					nCastNum++;
					break;
				default:
					nLabelNorm++;
					break;
				}

				switch(testresult.signalLoststatus)
				{  
				case (IMAGE_SIGNALLOST):
					//fprintf(fp,"Signal is lost\n");
					nLostNum++;
					break;
				default:
					nLabelNorm++;
					break;
				}
				switch(testresult.imgFrozenstatus)
				{
				case(IMAGE_FROZEN):
					//fprintf(fp,"Image is frozen\n");
					nFrozemNum++;
					break;
				default:
					nLabelNorm++;
					break;
				}

				if (nLabelNorm==6)//6个检测都正常
				{
					nNormNum++;
				}
				//fprintf(fp,"noise level = %10f\n",p1.NOISE.NOISELEVEL);
				//fprintf(fp,"clear level = %10f\n",p1.CLEAR.CLEARLEVEL);
				//fprintf(fp,"luminance saturation = %10f\n",p1.Bright.BrightLevel1);
				//fprintf(fp,"\n\n");
				if (pImage1!=NULL)
				{
					cvReleaseImage(&pImage1);
					pImage1=NULL;
				}
			}
	
		//输出统计结果
		fprintf(fp,"NotClearNum= %d\n",nNotClearNum);
		fprintf(fp,"HeavyNotClearNum= %d\n",nHeavyNotClearNum);
		fprintf(fp,"NoiseNum= %d\n",nNoiseNum);
		fprintf(fp,"HeavyNoiseNum= %d\n",nHeavyNoiseNum);
		fprintf(fp,"TooLightNum= %d\n",nTooLightNum);
		fprintf(fp,"TooDrakNum= %d\n",nTooDrakNum);
		fprintf(fp,"CastNum= %d\n",nCastNum);
		fprintf(fp,"FrozemNum= %d\n",nFrozemNum);
		fprintf(fp,"LostNum= %d\n",nLostNum);
		fprintf(fp,"NormNum= %d\n",nNormNum);
		fprintf(fp,"Imgnum= %d\n",nImgnum);

		cvReleaseCapture(&capture);
		capture=NULL;
	}while (FindNextFile(hFind, &FindFileData));

	fclose (fp); 
	HYIQ_Uninit(HYIQHandle);

	if (pImage1!=NULL)
	{
		cvReleaseImage(&pImage1);
		pImage1=NULL;
	}
	//if (pImage2!=NULL)
	//{
	// cvReleaseImage(&pImage2);
	// pImage2=NULL;
	//}

	cvDestroyAllWindows();
#ifdef STATIC_MEM
	HYIQ_MemMgrDestroy(hMemMgr);
	free(pMem);
#endif

	return 0;
}

#else
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
	IQ_IMAGES imgiq1_={0};
	IQ_IMAGES imgiq2_={0};

	HYIQ_TOutParam p1={0};
	CustomParam custopar={0};
	HYIQ_result testresult={0};
	//IMQU *pImqu=NULL;

	FILE *fp;
	IplImage *pImage1;//视频 前一帧
	IplImage *pImage2;//视频 当前帧

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char filePath[MAX_PATH]="..\\..\\test image\\";
	char filePath_const[MAX_PATH];
	char fileName1[MAX_PATH];
	char fileName2[MAX_PATH];


	int nImgnum;
	//统计检测结果

	int nNotClearNum;
	int nHeavyNotClearNum;
	int nNoiseNum;
	int nHeavyNoiseNum;
	int nTooLightNum;
	int nTooDrakNum;
	int nCastNum;
	int nFrozemNum;
	int nLostNum;
	int nNormNum;
	int nLabelNorm;

	int k;
	int bufSize;

	nImgnum=0;
	nNotClearNum=0;
	nHeavyNotClearNum=0;
	nNoiseNum=0;
	nHeavyNoiseNum=0;
	nTooLightNum=0;
	nTooDrakNum=0;
	nCastNum=0;
	nFrozemNum=0;
	nLostNum=0;
	nNormNum=0;
	fp=fopen("..//result.txt","w");
	HYIQ_Init(hMemMgr,&HYIQHandle);
	//cvNamedWindow("img2",1);
	//信号丢失参数调整
	custopar.SignalLost.nonblackbackground.GradSum = 350.0;
	custopar.SignalLost.nonblackbackground.meandiff = 40.0;
	custopar.SignalLost.nonblackbackground.sigma0 = 600.0;
	custopar.SignalLost.nonblackbackground.priors0 = 0.2;
	custopar.SignalLost.nonblackbackground.lostpercent = 0.55;
	custopar.SignalLost.nonblackbackground.charcornersnum = 10;
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
	fprintf(fp,"%s\n",fileName1);
	while (FindNextFile(hFind, &FindFileData))
	{  	
		nImgnum++;
		nLabelNorm=0;
		//testresult.clearstatus=0;
		//testresult.caststatus=0;
		//testresult.lightstatus=0;
		//testresult.Noisestatus=0;
		//testresult.signalLoststatus=0;
		//testresult.imgFrozenstatus=0;

		//p1.CLEAR.CLEARLEVEL=0;
		//p1.Bright.BrightLevel1=0;
		//p1.Bright.BrightLevel2=0;
		//p1.Bright.Brightderiv=0;
		//p1.ImgCast.flag=0;
		//p1.NOISE.NOISELEVEL=0;
		//p1.SignalLost.flag=0;
		//p1.ImgFrozen.flag=0;

		if (nImgnum==1)
		{
			memset(fileName2,0,MAX_PATH);
			strcpy(fileName2,filePath_const);
			strcat(fileName2,FindFileData.cFileName);
			fprintf(fp,"%s\n",fileName2);
		}
		else
		{
			memset(fileName1,0,MAX_PATH);
			strcpy(fileName1,filePath_const);
			strcat(fileName1,FindFileData.cFileName);
			fprintf(fp,"%s\n",fileName1);
			FindNextFile(hFind, &FindFileData);
			memset(fileName2,0,MAX_PATH);
			strcpy(fileName2,filePath_const);
			strcat(fileName2,FindFileData.cFileName);
			fprintf(fp,"%s\n",fileName2);
		}

		//从文件中读取图像  
		pImage1 = cvLoadImage(fileName1/*"D:\\1\\8_0.jpeg"*/, CV_LOAD_IMAGE_UNCHANGED);  
		pImage2 = cvLoadImage(fileName2/*"D:\\1\\8_1.jpeg"*/, CV_LOAD_IMAGE_UNCHANGED);


		imgiq1.lHeight=pImage1->height;
		imgiq1.lWidth=pImage1->width;
		imgiq1.lPixelArrayFormat=HY_IMAGE_BGR;
		imgiq1.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage1->width,pImage1->depth*pImage1->nChannels);
		imgiq1.pixelArray.chunky.pPixel=pImage1->imageData;

		imgiq2.lHeight=pImage2->height;
		imgiq2.lWidth=pImage2->width;
		imgiq2.lPixelArrayFormat=HY_IMAGE_BGR;
		imgiq2.pixelArray.chunky.lLineBytes=JLINE_BYTES(pImage2->width,pImage2->depth*pImage2->nChannels);
		imgiq2.pixelArray.chunky.pPixel=pImage2->imageData;

		p1.CLEAR.CLEARLEVEL=0;
		p1.CLEAR.isDetct=0;
		p1.Bright.BrightLevel1=0;
		p1.Bright.BrightLevel2=0;
		p1.Bright.Brightderiv=0;
		p1.Bright.isDetct=0;
		p1.ImgCast.flag=0;
		p1.ImgCast.isDetct=0;
		p1.NOISE.NOISELEVEL=0;
		p1.NOISE.isDetct=0;
		p1.SignalLost.flag=0;
		p1.SignalLost.isDetct=0;
		p1.ImgFrozen.flag=0;
		p1.ImgFrozen.isDetct=0;

		testresult.clearstatus=0;
		testresult.caststatus=0;
		testresult.lightstatus=0;
		testresult.Noisestatus=0;
		testresult.signalLoststatus=0;
		testresult.imgFrozenstatus=0;

		//HYIQ_CUTIMAGE(&imgiq1_,&imgiq1);
		//HYIQ_CUTIMAGE(&imgiq2_,&imgiq2);

		//pImqu=(IMQU *)HYIQHandle;
		HYIQ_LOST(HYIQHandle,&imgiq1,&imgiq2,&p1, &custopar);
		p1.SignalLost.isDetct=1;
		HYIQ_FROZEN(HYIQHandle, &imgiq1, &imgiq2, &p1, &custopar);
		p1.ImgFrozen.isDetct = 1;
		HYIQ_BRIGHT(HYIQHandle,&imgiq1,&imgiq2,&p1);//计算亮度均值和均值差
		p1.Bright.isDetct=1;
		if (p1.Bright.Brightderiv<2/*&&p1.SignalLost.flag==0*/)
		{
			HYIQ_NOISE(HYIQHandle,&imgiq1,&imgiq2,&p1);
			p1.NOISE.isDetct=1;
			if (p1.Bright.BrightLevel1>-0.45&&p1.Bright.BrightLevel2>-0.45)//过暗
			{
				HYIQ_CLEAR(HYIQHandle,&imgiq1,&imgiq2,&p1);//过暗不检测
				p1.CLEAR.isDetct=1;
				HYIQ_CAST(HYIQHandle,&imgiq1,&p1);//偏色  过亮或过暗不检测
				p1.ImgCast.isDetct=1;
			}
		}

		HYIQ_RESULT(HYIQHandle,&p1,&custopar,&testresult);
		switch(testresult.clearstatus)
		{
		case(IMAGE_CLEAR):
			fprintf(fp,"Image is clear\n");
			nLabelNorm++;
			break;
		case(IMAGE_NOTCLEAR):
			fprintf(fp,"Image is not clear\n");
			nNotClearNum++;
			break;
		case(IMAGE_HEVNOTCLEAR):
			fprintf(fp,"Image is heavy not clear \n");
			nHeavyNotClearNum++;
			break;
		default:
			break;
		}

		switch(testresult.Noisestatus)
		{
		case(IMAGE_UNNOISE):
			fprintf(fp,"Image is not noise\n");
			nLabelNorm++;
			break;
		case(IMAGE_NOISE):
			fprintf(fp,"Image is  noised\n");
			nNoiseNum++;
			break;
		case(IMAGE_HEAVENOISE):
			fprintf(fp,"Image is heavy noise\n");
			nHeavyNoiseNum++;
			break;
		default:
			break;
		}

		switch(testresult.lightstatus)
		{
		case(IMAGE_TOODARK):
			fprintf(fp,"Image is too dark\n");
			nTooDrakNum++;
			break;
		case(IMAGE_TOOLIGHT):
			fprintf(fp,"Image is too light\n");
			nTooLightNum++;
			break;
		default:
			nLabelNorm++;
			break;
		}

		switch(testresult.caststatus)
		{
		case(IMAGE_CAST):
			fprintf(fp,"Image is cast\n");
			nCastNum++;
			break;
		default:
			nLabelNorm++;
			break;
		}

		switch(testresult.signalLoststatus)
		{  
		case (IMAGE_SIGNALLOST):
			fprintf(fp,"Signal is lost\n");
			nLostNum++;
			break;
		default:
			nLabelNorm++;
			break;
		}
		switch(testresult.imgFrozenstatus)
		{
		case(IMAGE_FROZEN):
			fprintf(fp,"Image is frozen\n");
			nFrozemNum++;
			break;
		default:
			nLabelNorm++;
			break;
		}

		if (nLabelNorm==6)
		{
			nNormNum++;
		}
		fprintf(fp,"noise level = %10f\n",p1.NOISE.NOISELEVEL);
		fprintf(fp,"clear level = %10f\n",p1.CLEAR.CLEARLEVEL);
		fprintf(fp,"luminance saturation = %10f\n",p1.Bright.BrightLevel1);
		//fprintf(fp,"coverDust level = %10f\n",p1.CoverDustLevel);
		fprintf(fp,"\n\n");
	} 

	fprintf(fp,"NotClearNum= %d\n",nNotClearNum);
	fprintf(fp,"HeavyNotClearNum= %d\n",nHeavyNotClearNum);
	fprintf(fp,"NoiseNum= %d\n",nNoiseNum);
	fprintf(fp,"HeavyNoiseNum= %d\n",nHeavyNoiseNum);
	fprintf(fp,"TooLightNum= %d\n",nTooLightNum);
	fprintf(fp,"TooDrakNum= %d\n",nTooDrakNum);
	fprintf(fp,"CastNum= %d\n",nCastNum);
	fprintf(fp,"FrozemNum= %d\n",nFrozemNum);
	fprintf(fp,"LostNum= %d\n",nLostNum);
	//fprintf(fp,"CoverDustNum= %d\n",nCoverDustNum);
	fprintf(fp,"NormNum= %d\n",nNormNum);
	fprintf(fp,"Imgnum= %d\n",nImgnum);
	fclose (fp); 
	HYIQ_Uninit(HYIQHandle);
	//if (pImage1!=NULL)
	//{
	//	cvReleaseImage(&pImage1);
	//	pImage1=NULL;
	//}

	//if (pImage2!=NULL)
	//{
	//	cvReleaseImage(&pImage2);
	//	pImage2=NULL;
	//}

	cvDestroyAllWindows();

#ifdef STATIC_MEM
	HYIQ_MemMgrDestroy(hMemMgr);
	free(pMem);
#endif

	return 0;
}
#endif