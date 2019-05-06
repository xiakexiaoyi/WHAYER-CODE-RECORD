#include <iostream>
#include <opencv2/opencv.hpp>
#include "yolo_dll.h"
//#include <boost/thread/thread.hpp>
//#include <boost/bind.hpp>
#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif
#ifndef _WIN64 
#pragma comment(lib,"../darknet/Win32/yolo_cpp_dll.lib")
#else
#pragma comment(lib,"../darknet/x64/yolo_cpp_dll.lib")
#endif
int main()
{
	void *handle;
	char *input = "D:/work/model/test/testimg/2018_0423_sf6_108.jpg";
	/**/
	char *cfgfile = "D:/work/model/test/yolov3-tiny-voc.cfg";
	char *weightfile = "D:/work/model/test/yolov3-tiny-voc_18700.weights";
	/**
	char *cfgfile = "D:/work/model/xnortest/yolov3-tiny_xnor.cfg";
    char *weightfile = "D:/work/model/xnortest/yolov3-tiny_xnor_1700.weights";
	//char *cfgfile = "D:/work/model/1/yolov3-tiny_xnor_voc.cfg";
	//char *weightfile = "D:/work/model/1/yolov3-tiny_xnor_voc_100.weights";
	/**/
	IplImage *src = cvLoadImage(input);//BGR
	YOLOV3_IMAGES img;
	img.lHeight = src->height;
	img.lWidth = src->width;
	img.pixelArray.chunky.lLineBytes = src->widthStep;
	img.pixelArray.chunky.pPixel = src->imageData;
	img.lPixelArrayFormat = 3;
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	int count = 0;
	
	
	init(&handle, cfgfile, weightfile, 0.24,0);
	
	//while(1)
	{ 
		//boost::thread t1(boost::bind(detec, handle, &img, &resultlist));
		//boost::thread t2(boost::bind(detec, handle, &img, &resultlist));
		//t1.join();
		//t2.join();
		detec(handle, &img,&resultlist);
		printf("count = %d\n",count++);
	}
	uninit(handle);
	for(int i=0;i<resultlist.lResultNum;i++)
	{
		int left = resultlist.pResult[i].Target.left;
		int top = resultlist.pResult[i].Target.top;
		int right = resultlist.pResult[i].Target.right;
		int bottom = resultlist.pResult[i].Target.bottom;
		cvRectangle(src, cvPoint(left, top), cvPoint(right,bottom), cvScalar(0, 0, 255));
	}
	//cvSaveImage("D:/work/model/test/testimg/result.jpg", src);
	cvShowImage("Result Show", src);
	cvWaitKey(0);
	cvReleaseImage(&src);
	return 0;
}