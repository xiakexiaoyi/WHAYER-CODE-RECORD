#include <stdio.h>
#include "oillevelRecog_GPU.h"
#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#pragma comment(lib, "../Debug/oillevel.lib")
#pragma comment(lib, "../Debug/opencv_core244d.lib")
#pragma comment(lib, "../Debug/opencv_highgui244d.lib")

#else
#pragma comment(lib, "../Release/OilLevelLoc.lib")
#pragma comment(lib, "../Release/oillevel.lib")
#pragma comment(lib, "../Release/opencv_core244.lib")
#pragma comment(lib, "../Release/opencv_highgui244.lib")
#pragma comment(lib, "../Release/opencv_imgproc244.lib")


#endif

int main()
{
	int res=0;
	char *filename="../../photo/151641-391688_1-0.jpg";
	char *cfgfile="../../model/oillevel/tiny-yolo-voc.cfg";
	char *weightfile="../../model/oillevel/tiny-yolo-voc_21000.weights";
	char *cfgfile_squareness="../../model/sq/tiny-yolo-voc.cfg";
	char *weightfile_squareness="../../model/sq/tiny-yolo-voc_12000.weights";
	IplImage* pImg = 0;
	OLR_IMAGES imgs = {0};
	void *hTLHandle=NULL;
	void *hOHandle = NULL;
	float thresh=0.40;
	HYOLR_RESULT_LIST  resultlist ={0};
	int w=0,h=0;
	int gpu_index=0;
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	if(!pImg)
		return -1;
	
	w=pImg->width;
	h=pImg->height;
	resultlist.pResult = (HYOLR_RESULT*)malloc(20*sizeof(HYOLR_RESULT));
	HYOLR_Init_GPU(NULL,&hTLHandle);//��ʼ��
	HYOLR_SetParam_GPU(hTLHandle,cfgfile,weightfile,thresh,gpu_index,w,h);//载入yolo
	//HYOLR_Init(NULL,&hOHandle);
	if (0 != HYOLR_Init_GPU(NULL, &hOHandle))
	{
		printf("HYOLR_Init error.\n");
		res = -1;
		goto EXT;
	}
	//HYOLR_SetParam(hOHandle,cfgfile_squareness,weightfile_squareness,thresh,imgw,imgh);
	if (0 != HYOLR_SetParam_GPU(hOHandle, cfgfile_squareness, weightfile_squareness, thresh, gpu_index, w, h))
	{
		printf("HYOLR_SetParam error.\n");
		res = -1;
		goto EXT;
	}


	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	//HYOLR_OilRecog(hTLHandle,&imgs,&resultlist);//ʶ����λ����
	if(HYOLR_OilRecog_GPU(hTLHandle,&imgs,&resultlist)<0)	//Ѱ����λ����
	{
		printf("δ�ҵ�Ŀ��\n");
	}
	/*if( resultlist.lResultNum==0)
	{
		printf("δ�ҵ�Ŀ��\n");
		exit(-100);
	}*/
	for(int i=0;i < resultlist.lResultNum;i++)//��ʾ���
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
		if(resultlist.pResult[i].dVal==0)
			sprintf(text, "%d:oil", i+1);
		else if(resultlist.pResult[i].dVal==1)
			sprintf(text, "%d:white", i+1);
		else
			sprintf(text, "%d:state=none", i+1);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
		//cvPutText(pImg, text, ptText, &font,  cvScalar(0,255,0));
	}
	cvShowImage("Result Show", pImg);
	cvSaveImage("../Loc.jpg", pImg);
	cvWaitKey(10);
	for(int i=0;i < resultlist.lResultNum;i++)	//ÿ����λ������ȷ����λ
	{
		CvPoint startPt = { 0 };
		CvPoint endPt = { 0 };
		startPt.x = resultlist.pResult[i].Target.left;
		startPt.y = resultlist.pResult[i].Target.top;
		endPt.x = resultlist.pResult[i].Target.right;
		endPt.y = resultlist.pResult[i].Target.bottom;
		pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
		cvSetImageROI(pImg,cvRect(startPt.x, startPt.y, endPt.x- startPt.x, endPt.y- startPt.y));
		cvSaveImage("../cut.jpg", pImg);
		cvResetImageROI(pImg);
		//油位合并在一起
		if (resultlist.pResult[i].dVal == 0 || resultlist.pResult[i].dVal == 1)//改动
		{
			IplImage* src = cvLoadImage("../cut.jpg");
			double percent = 0;
			int imgw = src->width;
			int imgh = src->height;
			OLR_IMAGES imgs_squareness = { 0 };
			HYOLR_RESULT_LIST  resultlist_squareness = { 0 };
			resultlist_squareness.pResult = (HYOLR_RESULT*)malloc(20 * sizeof(HYOLR_RESULT));
			
			imgs_squareness.lHeight = src->height;
			imgs_squareness.lWidth = src->width;
			imgs_squareness.pixelArray.chunky.lLineBytes = src->widthStep;
			imgs_squareness.pixelArray.chunky.pPixel = src->imageData;
			
			//改动
			int flag=HYOLR_OilRecog_GPU(hOHandle, &imgs_squareness, &resultlist_squareness);
			if(flag==-99)//判断返回值，为-99时为 未找到目标
			{
				printf("没有油位\n");
				percent=100;
				printf("percent=%f\n", percent);
				cvReleaseImage(&src);
				if (resultlist_squareness.pResult)
					free(resultlist_squareness.pResult);
				continue;
			}
			if(flag !=0)
			{
				printf("HYOLR_OilRecog_GPU error!\n");
				cvReleaseImage(&src);
				if (resultlist_squareness.pResult)
					free(resultlist_squareness.pResult);
				if (resultlist.pResult)
					free(resultlist.pResult);
				cvReleaseImage(&pImg);
				res = -1;
				goto EXT;
			}
			//
			/*HYOLR_RESULT_LIST  resultlist_squarenesstmp = { 0 };
			resultlist_squarenesstmp.pResult = (HYOLR_RESULT*)malloc(20 * sizeof(HYOLR_RESULT));
			resultlist_squarenesstmp.lResultNum = 0;
			for (int j = 0; j < resultlist_squareness.lResultNum; j++)
			{
				if (resultlist_squarenesstmp.pResult[j].dVal == 3)
					continue;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].dVal = resultlist_squareness.pResult[j].dVal;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].dConfidence = resultlist_squareness.pResult[j].dConfidence;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].flag = resultlist_squareness.pResult[j].flag;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].Target.bottom = resultlist_squareness.pResult[j].Target.bottom;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].Target.left = resultlist_squareness.pResult[j].Target.left;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum].Target.right = resultlist_squareness.pResult[j].Target.right;
				resultlist_squarenesstmp.pResult[resultlist_squarenesstmp.lResultNum++].Target.top = resultlist_squareness.pResult[j].Target.top;
			}*/
			
			for (int i = 0; i < resultlist_squareness.lResultNum; i++)
			{
				CvPoint ptStart, ptStop, ptText;
				char text[256] = { 0 };
				CvFont font;
				ptStart.x = resultlist_squareness.pResult[i].Target.left;
				ptStart.y = resultlist_squareness.pResult[i].Target.top;
				ptStop.x = resultlist_squareness.pResult[i].Target.right;
				ptStop.y = resultlist_squareness.pResult[i].Target.bottom;

				ptText.x = resultlist_squareness.pResult[i].Target.left;
				ptText.y = resultlist_squareness.pResult[i].Target.bottom;
				if (ptText.y < src->height - 10)
					ptText.y += 10;
				cvRectangle(src, ptStart, ptStop, cvScalar(0, 0, 255));
				if (resultlist_squareness.pResult[i].dVal == 0)
					sprintf(text, "%d:squareness", i + 1);
				else if (resultlist_squareness.pResult[i].dVal == 1)
					sprintf(text, "%d:redsquareness", i + 1);
				else
					sprintf(text, "%d:state=none", i + 1);
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
				//cvPutText(src, text, ptText, &font,  cvScalar(0,255,0));
			}

			cvShowImage("OIL Result Show", src);
			cvSaveImage("../cutoil.jpg", src);
			
			int flagoil = 0;
			for (int j = 0; j < resultlist_squareness.lResultNum; j++)
			{
				if (resultlist_squareness.pResult[j].dVal == 0)
				{
					percent = 100.0 - 100.0*resultlist_squareness.pResult[j].Target.top / imgh;
					flagoil = 1;
				}
				else if (resultlist_squareness.pResult[j].dVal == 1)
				{
					percent = 100.0 - 100.0*(resultlist_squareness.pResult[j].Target.top + resultlist_squareness.pResult[j].Target.bottom) / (imgh * 2);
					flagoil = 1;
				}
					
				else if (resultlist_squareness.pResult[j].dVal == 2)
				{
					percent = 100.0 - 100.0*(resultlist_squareness.pResult[j].Target.top + resultlist_squareness.pResult[j].Target.bottom) / (imgh * 2);
					flagoil = 1;
				}			
			}
			if (flagoil == 0)
				percent = 100;

			/*if (resultlist_squareness.pResult[i].dVal == 0)
				percent = 100.0 - 100.0*resultlist_squareness.pResult[i].Target.top / imgh;
			else if (resultlist_squareness.pResult[i].dVal == 1)
				percent = 100.0 - 100.0*(resultlist_squareness.pResult[i].Target.top + resultlist_squareness.pResult[i].Target.bottom) / (imgh * 2);
			else if (resultlist_squareness.pResult[i].dVal == 2)
				percent = 100.0 - 100.0*(resultlist_squareness.pResult[i].Target.top + resultlist_squareness.pResult[i].Target.bottom) / (imgh * 2);*/
		
			
			printf("percent=%f\n", percent);
			cvWaitKey(0);
			cvReleaseImage(&src);
			if (resultlist_squareness.pResult)
				free(resultlist_squareness.pResult);
		}
		
	}

EXT:
	cvReleaseImage(&pImg);
	HYOLR_Uninit_GPU(hTLHandle);
	HYOLR_Uninit_GPU(hOHandle);
	if (resultlist.pResult)
		free(resultlist.pResult);
}
