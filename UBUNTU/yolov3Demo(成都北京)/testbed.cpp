#define GPU 
#include "yolo_dll.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string.h>


int main()
{
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	HYYOLOV3RESULT_LIST  resultlistCPU = { 0 };
	YOLOV3_IMAGES img;
	void *handle = NULL;
	void *handleCPU = NULL;
	char *cfgFile = "../../model/yolov3-voc.cfg";
	char *weightFile = "../../model/yolov3-voc_75000.weights";
        cv::Mat input=cv::imread("../../photo/1.jpg");

	img.lHeight = input.rows;
	img.lWidth = input.cols;
	img.pixelArray.chunky.lLineBytes = input.step;
	img.pixelArray.chunky.pPixel = input.data;
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
        resultlistCPU.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
#ifdef GPU
printf("-------------------GPU--------------------\n");
        init(&handle, cfgFile, weightFile, 0.1,0);
	detec(handle, &img, &resultlist);
	uninit(handle);

printf("-------------------CPU--------------------\n");
	init_no_gpu(&handleCPU, cfgFile, weightFile, 0.1);
	detec_no_gpu(handleCPU, &img, &resultlistCPU);
	uninit_no_gpu(handleCPU);
#endif




        return 0;

}
