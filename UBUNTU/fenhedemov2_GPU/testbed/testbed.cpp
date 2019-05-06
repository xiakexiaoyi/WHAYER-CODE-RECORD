#include <stdio.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include "SwitchRecog_GPU.h"


int main()
{
	

	HYSR_RESULT  result ={0};
	void *hTLHandle=NULL;
	float thresh=0.1;
	char *cfgfile="../model/平面/tiny-yolo-voc.cfg";
	char *weightfile="../model/平面/tiny-yolo-voc_final.weights";
	//char *cfgfile="../model/tiny-yolo-voc_IO.cfg";
	//char *weightfile="../model/tiny-yolo-voc_final_IO.weights";
	
	//char *filename="../5011����B��.jpg";
	int gpu_index=0;

	char *filename="../1.jpg";
	IplImage* pImg = 0;
	SR_IMAGES imgs = {0};

	
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
    
	HYSR_Init_GPU(NULL,&hTLHandle);
	HYSR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh,gpu_index);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	HYSR_SwitchRecog_GPU(hTLHandle,&imgs,&result);


	
	CvPoint ptStart, ptStop, ptText;
	char text[256]={0};
	CvFont font;
	ptStart.x = result.rtTarget.left;
	ptStart.y = result.rtTarget.top;
	ptStop.x = result.rtTarget.right;
	ptStop.y = result.rtTarget.bottom;
	ptText.x = result.rtTarget.left;
	ptText.y = result.rtTarget.bottom;
printf("%d %d %d %d\n",ptStart.x,ptStart.y,ptStop.x,ptStop.y);
	if (ptText.y<pImg->height-10)
		ptText.y += 10;
	if(result.dlabel==1)
	{
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		sprintf(text, "val=he confi=%.2f", result.dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	}
	else
	{
		cvRectangle(pImg, ptStart, ptStop, cvScalar(0,0,255));
		sprintf(text, "val=fen confi=%.2f", result.dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	}
	printf("%s\n",text);
	cvSaveImage("result.jpg", pImg);
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);


	HYSR_Uninit_GPU(hTLHandle);

	return 0;
}


