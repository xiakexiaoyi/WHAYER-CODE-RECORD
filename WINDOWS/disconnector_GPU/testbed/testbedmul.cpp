#include "disconnector.h"
#include <opencv2/opencv.hpp>
#include "putText.h"
#include <io.h>
#include <windows.h>
#include <stdio.h>


#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>

#ifdef _DEBUG
#pragma comment(lib, "../Debug/stateRecog.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")

#else
#pragma comment(lib, "../Release/disconnector.lib")
#pragma comment(lib, "../Release/opencv_core244.lib")
#pragma comment(lib, "../Release/opencv_highgui244.lib")
#pragma comment(lib, "../Release/opencv_imgproc244.lib")

using namespace cv;
using namespace std;

#endif
void detc(int threadNum)
{
	float thresh=0.24;
	void *hTLHandle=NULL;
	IplImage* pImg = 0;
	//IplImage* pImg1 = 0;
	//IplImage* pImg2 = 0;
	DSR_IMAGES imgs = {0};
	HYDSR_RESULT_LIST  resultlist ={0};
	/*DSR_IMAGES imgs1 = {0};
	HYDSR_RESULT_LIST  resultlist1 ={0};
	DSR_IMAGES imgs2 = {0};
	HYDSR_RESULT_LIST  resultlist2 ={0};*/
	char *cfgfile="../model/1123/tiny-yolo-voc.cfg";
	char *weightfile="../model/1123/tiny-yolo-voc_70000.weights";

	//char *cfgfile="../model/yolo-voc.cfg";
	//char *weightfile="../model/yolo-voc_final.weights";
	

	pImg = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
	//pImg1 = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
	//pImg2 = cvLoadImage("../110KV1632.jpg",CV_LOAD_IMAGE_COLOR); //原图输入
	int w=0,h=0;
    int flag=0;
	w=pImg->width;
	h=pImg->height;
	printf("ThreadNum: %d_befor  HYDSR_Init \n", threadNum);
	HYDSR_Init(NULL,&hTLHandle);
	printf("ThreadNum: %d_after HYDSR_Init\n", threadNum);
	printf("ThreadNum: %d_befor HYDSR_SetParam  \n", threadNum);
	flag=HYDSR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);
	printf("ThreadNum: %d_after HYDSR_SetParam\n", threadNum);
    if(flag != 0)
	{
		printf("Release_%d\n",threadNum);
		cvReleaseImage(&pImg);
		HYDSR_Uninit(hTLHandle);
		return;
	}

	resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	/*resultlist1.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	imgs1.lHeight = pImg1->height;
	imgs1.lWidth = pImg1->width;
	imgs1.pixelArray.chunky.lLineBytes = pImg1->widthStep;
	imgs1.pixelArray.chunky.pPixel = pImg1->imageData;
	resultlist2.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	imgs2.lHeight = pImg2->height;
	imgs2.lWidth = pImg2->width;
	imgs2.pixelArray.chunky.lLineBytes = pImg2->widthStep;
	imgs2.pixelArray.chunky.pPixel = pImg2->imageData;*/

	printf("ThreadNum: %d_befor  HYDSR_StateRecog \n", threadNum);
	HYDSR_StateRecog(hTLHandle,&imgs,&resultlist);
	printf("ThreadNum: %d_after  HYDSR_StateRecog \n", threadNum);

	//boost::thread t1(boost::bind(HYDSR_StateRecog, hTLHandle, &imgs1, &resultlist1));
    //boost::thread t2(boost::bind(HYDSR_StateRecog, hTLHandle, &imgs2, &resultlist2));
	//t1.join();
    //t2.join();
	/*while (1)
    {
       cvWaitKey(10);
    }*/
   /*  if(t1.joinable())
     {
      t1.join();
     }
    

    if(t2.joinable())
     {
      t2.join();
     }*/
	printf("lResultNum%d=%d\n",threadNum,resultlist.lResultNum);
	//printf("lResultNum1=%d\n",resultlist1.lResultNum);
	//printf("lResultNum2=%d\n",resultlist2.lResultNum);
	for(int i=0;i<resultlist.lResultNum;i++)
	 {
		 printf("dConfidence%d=%f\n",threadNum,resultlist.pResult[i].dConfidence);
	 }
     /*for(int i=0;i<resultlist1.lResultNum;i++)
	 {
		 printf("dConfidence1=%f\n",resultlist1.pResult[i].dConfidence);
	 }
	 for(int i=0;i<resultlist2.lResultNum;i++)
	 {
		 printf("dConfidence2=%f\n",resultlist2.pResult[i].dConfidence);
	 }*/
	cvReleaseImage(&pImg);
	//cvReleaseImage(&pImg1);
	//cvReleaseImage(&pImg2);
	if (resultlist.pResult)
		free(resultlist.pResult);
	/*if (resultlist1.pResult)
		free(resultlist1.pResult);
	if (resultlist2.pResult)
		free(resultlist2.pResult);*/
	HYDSR_Uninit(hTLHandle);

}
int main111()
{
	//detc(1);
	while(1)
	{
		boost::thread t1(boost::bind(detc, 1));
		//boost::thread t2(boost::bind(detc, 2));
		//boost::thread t3(boost::bind(detc, 3));
		//boost::thread t4(boost::bind(detc, 4));
		//boost::thread t5(boost::bind(detc, 5));
		//boost::thread t6(boost::bind(detc, 6));
		//boost::thread t7(boost::bind(detc, 7));
		//boost::thread t8(boost::bind(detc, 8));
		t1.join();
		//t2.join();
		//t3.join();
		//t4.join();
		//t5.join();
		//t6.join();
		//t7.join();
		//t8.join();
	}

	return 0;
}