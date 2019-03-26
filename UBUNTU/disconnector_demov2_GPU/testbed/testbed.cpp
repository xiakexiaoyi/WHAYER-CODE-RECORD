#include "disconnector_GPU.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string.h>

int main123();
int main123()
{
	int count =0;
	while(1)
	{
            main123();
	    printf("conut=%d\n",count);
	    count++;
	}

	
	return 0;
}
int main()
{
	char *filename="../../photo/1115glkg_fen_125.jpg";
	/******YOLO模型*******/
	char *cfgfile="../../model/yolo-voc.cfg";
	char *weightfile="../../model/yolo-voc_final.weights";
	/*************/
	/******TINY-YOLO模型*******
	char *cfgfile="../../model/1123/tiny-yolo-voc.cfg";
	char *weightfile="../../model/1123/tiny-yolo-voc_55000.weights";
	/*************/
	IplImage* pImg = 0;
	DSR_IMAGES imgs = {0};
	void *hTLHandle=NULL;
	float thresh=0.45;
	HYDSR_RESULT_LIST  resultlist ={0};
	int w=0,h=0;
        int gpu_index=0;
	//IplImage* src = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);    //图像四周加白边
    //pImg = cvCreateImage(cvSize(src->width*1.02, src->height*1.04), src->depth, src->nChannels);
	//cvCopyMakeBorder(src, pImg, cvPoint(src->width*0.01, src->height*0.02), IPL_BORDER_CONSTANT, cvScalarAll(255));

	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR); //原图输入

	w=pImg->width;
	h=pImg->height;
	resultlist.pResult = (HYDSR_RESULT*)malloc(20*sizeof(HYDSR_RESULT));
	HYDSR_Init_GPU(NULL,&hTLHandle);
	HYDSR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh,gpu_index,w,h);

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;

	HYDSR_StateRecog_GPU(hTLHandle,&imgs,&resultlist);
    
	for(int i=0;i < resultlist.lResultNum;i++)
	{
		CvPoint ptStart, ptStop, ptText;
		char text[256]={0};
		char text1[512]={0};
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
		if(resultlist.pResult[i].flag !=1)
		{
			sprintf(text, "%d:state=none", i+1);
		}
		else if(resultlist.pResult[i].dVal==0)
		{
			//continue;
			sprintf(text, "%d:7", i+1);
		}
		else if(resultlist.pResult[i].dVal==1)
		{
			//continue;
			sprintf(text, "%d:n", i+1);
		}
		else if(resultlist.pResult[i].dVal==2)
			sprintf(text, "%d:fen", i+1);
		else if(resultlist.pResult[i].dVal==3)
			sprintf(text, "%d:he", i+1);
		sprintf(text1,"%s %f\n",text,resultlist.pResult[i].dConfidence);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		cvPutText(pImg, text1, ptText, &font,  cvScalar(0,255,0));
	}
	cvShowImage("Result Show", pImg);
	cvWaitKey(0);


	cvReleaseImage(&pImg);
	HYDSR_Uninit_GPU(hTLHandle);
	if (resultlist.pResult)
		free(resultlist.pResult);
        return 0;

}
