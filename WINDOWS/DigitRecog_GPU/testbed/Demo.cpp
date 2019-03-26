#include <opencv2/opencv.hpp>
#include<string.h>
#include "DigitalRecog.h"
//#include "HYL_MatchRigidBody.h"
#include "putText.h"
#include <time.h>
//#ifdef __cplusplus
//extern "C" {
//#endif
//#include "sigar_fileinfo.h"
//#ifdef __cplusplus
//}
//#endif
//
//
//#include "windows.h"  
//#include  "conio.h "  
//  
//#include"WinBase.h"  
//#include "Psapi.h"  
//  
//#pragma  once  
//#pragma  message("Psapi.h --> Linking with Psapi.lib")  
//#pragma  comment(lib,"Psapi.lib")  

#ifdef _DEBUG
#pragma comment(lib, "../Debug/DigitalRecog.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")
#pragma comment(lib, "../Debug/opencv_imgproc244d.lib")
#pragma comment(lib, "../Debug/opencv_ml244d.lib")

#else
#pragma comment(lib, "../Release/DigitalRecog.lib")
//#pragma comment(lib, "../Release/MatchRigidBody.lib")
#pragma comment(lib, "../Release/opencv_core244.lib")
#pragma comment(lib, "../Release/opencv_highgui244.lib")
#pragma comment(lib, "../Release/opencv_imgproc244.lib")
#endif

//#define MR_READ_FILE_PARA "../MatchRigidBody.dat"
int mouseX=-1,mouseY=-1,mouseFlag=0;
int flagEndL = 0,flagEndR = 0;
void onMouse(int Event,int x,int y,int flags,void* param );
int DigitalTrain(HYDR_RESULT_LIST *resultlist);
int DigitalRecog(HYDR_RESULT_LIST resultlist);


char imgname[256]="digit.jpg"; 
 char filename[100]="../../../Web/";
//char resultname[100]="../../../Web/456/";



//double showMemoryInfo()   
//{   
//  double MemorySize;         //单位MB 
//  HANDLE handle=GetCurrentProcess();   
//  
//  PROCESS_MEMORY_COUNTERS pmc;   
//  GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));  
//  MemorySize=pmc.WorkingSetSize/1024; 
//  
//  printf("内存使用: %8lf \n",MemorySize);  //WorkingSetSize The current working set size, in bytes. 
//  
//  return MemorySize; 
//} 
int main()
{

	
	HYDR_RESULT_LIST resultlist = {0};
	strcat(filename,imgname); 
	//strcat(resultname,imgname); 
    resultlist.lResultNum=20;
	resultlist.DRArea = (MDRECT*)malloc(resultlist.lResultNum*sizeof(MDRECT));
	resultlist.pResult = (HYDR_RESULT*)malloc(resultlist.lResultNum*sizeof(HYDR_RESULT)) ;
	DigitalTrain(&resultlist);//训练
	
	
	DigitalRecog(resultlist);//识别
	
		

	

	if (resultlist.DRArea)
		free(resultlist.DRArea);
	if (resultlist.pResult)
		free(resultlist.pResult);

	system("pause");

	return 0;
}

 
int DigitalTrain(HYDR_RESULT_LIST *resultlist)
{
	int res=0;
	int mouseParam[3];
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	CvSize ImgSize; 
	int j,flagL = 0,i=0;
	unsigned char *pData;
	IplImage *pOrgImg=NULL,*maskImage= NULL;
	IplImage *tmpImage = NULL;
	MHandle MHandle=NULL;
	HYL_IMAGES wy_testImage = {0}, wy_maskImage = {0};
	//char *filename="../001.jpg";
	//HYL_MatchInit(NULL, &MHandle);
	CvCapture* capture = cvCaptureFromFile("../../../Web/digit.mp4"); 
	pOrgImg=cvQueryFrame(capture);
	//pOrgImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	if (!pOrgImg)
	{
		printf("Error when loading image.\n"), exit(1); 
	}
	ImgSize.height = pOrgImg->height;
	ImgSize.width = pOrgImg->width;
	tmpImage = cvCreateImage(ImgSize, pOrgImg->depth, pOrgImg->nChannels);
	cvResize(pOrgImg, tmpImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", tmpImage);
	cvNamedWindow("TrainImage2", 0);
	
	
	//Mat m=cvarrToMat(tmpImage,true);
	Mat m=Mat(tmpImage,true);
	putTextZH(m, "操作步骤：\n1、选取单块数字区域。\n", Point(50, 50), Scalar(0, 255, 255), 30, "Arial");
	//tmpImage=&(IplImage)m;
	tmpImage=cvCloneImage(&(IplImage)m);
	//cvShowImage("TrainImage2",tmpImage);
	//cvWaitKey(0);
	cvSaveImage("D:\\testImage.bmp", tmpImage);
	
	

	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	printf("请圈定单块数字区域目标对象：\n");
	while (1)
	{
		//左键进行标记 
		if (mouseParam[2]==CV_EVENT_LBUTTONDOWN)
		{
			startPt.x=mouseParam[0];
			startPt.y=mouseParam[1];
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
			
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			flagL=1;
			cvShowImage("TrainImage2",tmpImage);	
			
			resultlist->DRArea[i].left = startPt.x;
			resultlist->DRArea[i].top = startPt.y; 		
		}
		if (mouseParam[2]==CV_EVENT_MOUSEMOVE && flagL==1)
		{
			endPt.x=mouseParam[0];
			endPt.y=mouseParam[1];
		
			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp",CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0,0,255), 1);
			cvShowImage("TrainImage2",tmpImage);	
			
			resultlist->DRArea[i].right = endPt.x;
			resultlist->DRArea[i].bottom = endPt.y ; 
		}
		
		if(1 == flagEndL && i == 0 )
		{
			//printf("匹配目标选择完成，下面选择识别目标\n");
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
			break;
		}
		if (1 == flagEndL && i != 0 )   //左键标记一个进行重置
		{
			i++;
			resultlist->lAreaNum++;
			flagEndL=0;
			flagL=0;
		}

		if (13 == cvWaitKey(10))  //enter 退出
		{
			break;
		}
		cvShowImage("TrainImage2",tmpImage);
	}
	printf("交互完成\n");
	//cvWaitKey(0);
	cvDestroyWindow("TrainImage2");

	//cvSetImageROI(pOrgImg,cvRect(resultlist->DRArea[1].left,resultlist->DRArea[1].top,resultlist->DRArea[1].right-resultlist->DRArea[1].left,resultlist->DRArea[1].bottom-resultlist->DRArea[1].top));
	//cvSaveImage("../cut.jpg",pOrgImg);
	//cvResetImageROI(pOrgImg);
	//resultlist->origin.x=(resultlist->DRArea[0].left+resultlist->DRArea[0].right)/2;
	//resultlist->origin.y=(resultlist->DRArea[0].top+resultlist->DRArea[0].bottom)/2;

	//maskImage = cvCreateImage(ImgSize, 8, 1);   //mask image
	//pData = (unsigned char *)(maskImage->imageData);
	//for (j=0;j<ImgSize.height;j++)
	//{
	//	for (i=0;i<ImgSize.width;i++)
	//	{
	//		if (j>=resultlist->DRArea[0].top&&j<=resultlist->DRArea[0].bottom&&i>=resultlist->DRArea[0].left&&i<=resultlist->DRArea[0].right)
	//		{
	//			pData[j*maskImage->widthStep+i]=255;
	//		}
	//		else
	//		{
	//			pData[j*maskImage->widthStep+i]=0;
	//		}

	//	}
	//}
	//wy_testImage.lWidth = pOrgImg->width;
	//wy_testImage.lHeight = pOrgImg->height;
	//wy_testImage.pixelArray.chunky.lLineBytes = pOrgImg->widthStep;
	//wy_testImage.pixelArray.chunky.pPixel = pOrgImg->imageData;
	//wy_testImage.lPixelArrayFormat = HYL_IMAGE_BGR;
	////mask
	//wy_maskImage.lWidth = maskImage->width;
	//wy_maskImage.lHeight = maskImage->height;
	//wy_maskImage.pixelArray.chunky.lLineBytes = maskImage->widthStep;
	//wy_maskImage.pixelArray.chunky.pPixel = maskImage->imageData;
	//wy_maskImage.lPixelArrayFormat = HYL_IMAGE_GRAY;
	//if (0 != HYL_TrainTemplateFromMask (MHandle, &wy_testImage, &wy_maskImage, "OK", 0))//训练图片，得到模板
	//{
	//	printf("HYAMR_TrainTemplateFromMask error!\n");
	//	goto EXT;
	//}
	//if (0 != HYL_SaveTDescriptorsGroup(MHandle, MR_READ_FILE_PARA))
	//{
	//	printf("HYAMR_SaveTDescriptorsGroup error!\n");
	//}
EXT:
	//cvReleaseImage(&pOrgImg);
	cvReleaseCapture(&capture);
	cvReleaseImage(&maskImage);
	cvReleaseImage(&tmpImage);
	//HYL_MatchUninit(MHandle);
	return res;

}
int DigitalRecog(HYDR_RESULT_LIST resultlist)
{
	void *hTLHandle=NULL;
	float thresh=0.24;
	DR_IMAGES imgs = {0};
	MHandle MHandle=NULL;
	HYL_IMAGES src = {0};
	Mat m;
	//char *cfgfile="../model/number_led/tiny-yolo-voc.cfg";
	//char *weightfile="../model/number_led/tiny-yolo-voc_final.weights";
	//char *cfgfile="../model/test/tiny-yolo-voc.cfg";
	//char *weightfile="../model/test/tiny-yolo-voc_72000.weights";
	char *cfgfile="../model/newyolo/tiny-yolo-voc.cfg";
	char *weightfile="../model/newyolo/tiny-yolo-voc_final.weights";
	//char *filename="../001.jpg";
   // MPOINT *centre;
	HYDR_RESULT_LIST Tmpresultlist = {0};
    MPOINT offset;
	CvPoint startPt = {0};
	CvPoint endPt = {0};
	IplImage* pImg = 0;
	//pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	//HYL_MatchInit(NULL, &MHandle);
	HYDR_Init(NULL,&hTLHandle);

    HYDR_SetParam(hTLHandle,cfgfile,weightfile,thresh);
	
	
	//resultlist.lMaxResultNum=3;
    Tmpresultlist.pResult = (HYDR_RESULT *)malloc(1*sizeof(HYDR_RESULT));
	//centre = (MPOINT*)malloc(1*sizeof(MPOINT));
	/*if (0 != HYL_GetTemplateFromText(MHandle, MR_READ_FILE_PARA))
	{
		printf("HYAMR_GetTemplateFromText error !\n");
	}*/
	int n=0;
	CvCapture* capture = cvCaptureFromFile("../../../Web/digit.mp4"); 
	while(pImg=cvQueryFrame(capture))
	{
		char resultname[100];
		char resultimgname[100];
		n=n+1;
		if(n%24 > 0)continue;
		//src.lHeight = pImg->height;
		//src.lWidth = pImg->width;
		//src.lPixelArrayFormat = HYL_IMAGE_BGR;
		//src.pixelArray.chunky.pPixel = pImg->imageData;
		//src.pixelArray.chunky.lLineBytes = pImg->widthStep;
		//HYL_GetDashboard(MHandle, &src, "OK", 0.45, centre);//匹配模板,求图像偏移量
		//resultlist.offset.x=centre->x-resultlist.origin.x;
		//resultlist.offset.y=centre->y-resultlist.origin.y;
		resultlist.offset.x=0;
		resultlist.offset.y=0;

		sprintf(resultname, "../../../Web/result/%d_%s", n,imgname);
		sprintf(resultimgname, "../../../Web/result/result_%d_%s", n,imgname);
		int j=0;
		MDRECT rtArea;
		for(int i=0;i<resultlist.lAreaNum;i++)
		{
			/*if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
			return false;
			}
			if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
				return false;
			}

			if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
				return false;
			}
			printf("begin:size=%d,resident=%d,share=%d\n",procmem.size,procmem.resident,procmem.share);*/
			IplImage *RectImg=NULL;
			//IplImage *NumImg=NULL;
			//IplImage *NumImgG=NULL;
			//IplImage *dst=NULL;


			rtArea.bottom=MAX(resultlist.DRArea[i].bottom,resultlist.DRArea[i].top)+resultlist.offset.y;
			rtArea.left = MIN(resultlist.DRArea[i].left,resultlist.DRArea[i].right)+resultlist.offset.x;
			rtArea.top = MIN(resultlist.DRArea[i].bottom,resultlist.DRArea[i].top)+resultlist.offset.y;
			rtArea.right = MAX(resultlist.DRArea[i].left,resultlist.DRArea[i].right)+resultlist.offset.x;

			//rtArea.bottom = resultlist.DRArea[i].bottom+resultlist.offset.y;
			//rtArea.left = resultlist.DRArea[i].left+resultlist.offset.x;
			//rtArea.top = resultlist.DRArea[i].top+resultlist.offset.y;
			//rtArea.right = resultlist.DRArea[i].right+resultlist.offset.x;
			if(rtArea.bottom >= imgs.lHeight)rtArea.bottom=rtArea.bottom-1;
			if(rtArea.right >= imgs.lWidth )rtArea.right=rtArea.right-1;
			if(rtArea.left < 0 )rtArea.left=0;
			if(rtArea.top < 0 )rtArea.top =0;
			RectImg = cvCreateImage(cvSize(rtArea.right-rtArea.left,rtArea.bottom-rtArea.top), pImg->depth, pImg->nChannels);
			cvSetImageROI(pImg,cvRect(rtArea.left,rtArea.top,rtArea.right-rtArea.left,rtArea.bottom-rtArea.top));
			cvCopy(pImg,RectImg);
			cvResetImageROI(pImg);
			cvSaveImage(resultname, RectImg);
			//cvRectangle(pImg, cvPoint(rtArea.left,rtArea.top), cvPoint(rtArea.right,rtArea.bottom), cvScalar(0,255,255));
			//cvShowImage("Rect Show", pImg);
			if (!RectImg||!RectImg->height||!RectImg->width)
			{
				printf("Error when cut image.\n");   
				j=j+1;
				continue;
			}
			imgs.lHeight = RectImg->height;
			imgs.lWidth = RectImg->width;
			imgs.pixelArray.chunky.lLineBytes = RectImg->widthStep;
			imgs.pixelArray.chunky.pPixel = RectImg->imageData;
			Tmpresultlist.lResultNum=0;
			int timestart,timeend;
			double time;
			//timestart=GetTickCount();
			//printf("HYDR_DigitRecog start,w=%d h=%d\n",imgs.lWidth,imgs.lHeight);
			HYDR_DigitRecog(hTLHandle,&imgs,&Tmpresultlist);
			//printf("HYDR_DigitRecog end\n\n");
			//timeend=GetTickCount();
			//time=(double)(timeend-timestart)/1000;
			//printf("时间%f\n\n",time);
			resultlist.pResult[0].dVal=Tmpresultlist.pResult[0].dVal;
			resultlist.pResult[0].Target.left=Tmpresultlist.pResult[0].Target.left+rtArea.left;
			resultlist.pResult[0].Target.right=Tmpresultlist.pResult[0].Target.right+rtArea.left;
			resultlist.pResult[0].Target.top=Tmpresultlist.pResult[0].Target.top+rtArea.top;
			resultlist.pResult[0].Target.bottom=Tmpresultlist.pResult[0].Target.bottom+rtArea.top;
		    cvReleaseImage(&RectImg);

			/*int tmpleft=resultlist.pResult[i-j].Target.left;
			int tmpright=resultlist.pResult[i-j].Target.right;
			int tmptop=resultlist.pResult[i-j].Target.top;
			int tmpbottom=resultlist.pResult[i-j].Target.bottom;
			int tmpw=1.2*(tmpright-tmpleft);
			int tmph=1.2*(tmpbottom-tmptop);


			NumImg = cvCreateImage(cvSize(tmpw,tmph), pImg->depth, pImg->nChannels);
			dst = cvCreateImage(cvSize(tmpw,tmph), pImg->depth, pImg->nChannels);
			NumImgG = cvCreateImage(cvSize(NumImg->width,NumImg->height),NumImg->depth,1);
			cvSetImageROI(pImg,cvRect(tmpleft,tmptop,tmpw,tmph));
			cvCopy(pImg,NumImg);
			cvResetImageROI(pImg);
			cvCvtColor(NumImg,NumImgG,CV_BGR2GRAY);
			cvSmooth(NumImg,dst);
			cvSaveImage("D:\\NumImg.bmp", NumImg);
			cvSaveImage("D:\\dst.bmp", dst);
			cvSaveImage("D:\\NumImgG.bmp", NumImgG);*/




			//i=i-1;
			
			/*if(SIGAR_OK != sigar_proc_mem_get(p_sigar, pid, &procmem)){
				return false;
			}
			if(SIGAR_OK != sigar_proc_cpu_get(p_sigar, pid, &proccpu)){
				return false;
			}

			if(SIGAR_OK != sigar_proc_state_get(p_sigar, pid, &procstate)){
				return false;
			}
			printf("end:size=%d,resident=%d,share=%d\n",procmem.size,procmem.resident,procmem.share);*/
		}

		
		//printf("画框\n");
		////cvDestroyWindow("Result Show");
		for (int i=0; i<resultlist.lAreaNum; i++)
		{
			CvPoint ptStart, ptStop, ptText;
			char text[256]={0};
			CvFont font;
			ptStart.x = resultlist.pResult[i].Target.left;
			ptStart.y = resultlist.pResult[i].Target.top;
			ptStop.x = resultlist.pResult[i].Target.right;
			ptStop.y = resultlist.pResult[i].Target.bottom;
			ptText.x = resultlist.pResult[i].Target.left;
			ptText.y = resultlist.pResult[i].Target.bottom;
			if (ptText.y<pImg->height-10)
				ptText.y += 10;
			cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
			//printf("val=%f\n",resultlist.pResult[i].dVal);
			sprintf(text, "数值=%.0lf", resultlist.pResult[i].dVal);
			//m=cvarrToMat(pImg,true);
			m=Mat(pImg,true);
			putTextZH(m, "操作步骤：\n1、选取单块数字区域。\n", Point(50, 50), Scalar(0, 255, 255), 30, "Arial");
			putTextZH(m, text, ptText, Scalar(0, 255, 255), 40, "Arial");
			pImg=&(IplImage)m;
			//pImg=cvCloneImage(&(IplImage)m);
			//m.release();
			//cvInitFont(&font, CV_FONT_HERSHEY_TRIPLEX, 0.5f, 0.8f);
			//cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,255));
		
		}
		//printf("画框结束\n");
		cvSaveImage(resultimgname, pImg);
		cvShowImage("Result Show", pImg);
		cvWaitKey(10);
	}
	printf("视频结束\n");
	/*cvNamedWindow("Result Show");
	cvShowImage("Result Show", pImg);*/
	cvReleaseCapture(&capture);
	cvReleaseImage(&pImg);
	if(Tmpresultlist.pResult==NULL)
		free(Tmpresultlist.pResult);
	//free(centre);
	//HYL_MatchUninit(MHandle);
	HYDR_Uninit(hTLHandle);

	return 0;
}

void onMouse(int Event,int x,int y,int flags,void* param )
{
	if (Event==CV_EVENT_LBUTTONDOWN)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=1;
	}
	if (Event==CV_EVENT_RBUTTONDOWN)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_RBUTTONDOWN;
	}
	if (Event==CV_EVENT_MOUSEMOVE)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_MOUSEMOVE;
	}
	if (Event==CV_EVENT_LBUTTONUP)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_LBUTTONUP;
		flagEndL=1;
	}
	if (Event==CV_EVENT_RBUTTONUP)
	{
		int *Data=(int*)param;
		Data[0]=x;
		Data[1]=y;
		Data[2]=CV_EVENT_RBUTTONUP;
		flagEndR=1;
	}
}
