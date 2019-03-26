#include "oillevelRecog.h"
#include "HYL_OilLevel.h"
#include <opencv2/opencv.hpp>


#pragma comment(lib, "../x64/Release/OilLevelLoc.lib")
#pragma comment(lib, "../x64/Release/oillevel.lib")
#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif

int main()
{
	int res=0;
	char *filename="../img/justForTestPic333.jpg";
	//char *cfgfile="../model/oillevel/yolo-voc.cfg";
	//char *weightfile="../model/oillevel/yolo-voc_36000.weights";
	char *cfgfile = "../model/oillevel/tiny-yolo-voc.cfg";
	char *weightfile = "../model/oillevel/tiny-yolo-voc_final.weights";
	char *cfgfile_squareness="../model/sq/tiny-yolo-voc.cfg";
	char *weightfile_squareness="../model/sq/tiny-yolo-voc_final.weights";
	IplImage* pImg = 0;
	OLR_IMAGES imgs = {0};
	void *hTLHandle=NULL;
	float thresh=0.40;
	HYOLR_RESULT_LIST  resultlist ={0};
	int w=0,h=0;
	pImg = cvLoadImage(filename,CV_LOAD_IMAGE_COLOR);
	if(!pImg)
		return -1;
	
	w=pImg->width;
	h=pImg->height;
	resultlist.pResult = (HYOLR_RESULT*)malloc(20*sizeof(HYOLR_RESULT));
	//HYOLR_Init(NULL,&hTLHandle);//初始化
	if(0!=HYOLR_Init(NULL,&hTLHandle))
	{
		printf("HYOLR_Init error.\n");
		if (resultlist.pResult)
			free(resultlist.pResult);
		return -1;
	}
	//HYOLR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h);//载入yolo
	if(0!=HYOLR_SetParam(hTLHandle,cfgfile,weightfile,thresh,w,h))
	{
		printf("HYOLR_SetParam error.\n");
		if (resultlist.pResult)
			free(resultlist.pResult);
		HYOLR_Uninit(hTLHandle);
		return -1;
	}

	imgs.lHeight = pImg->height;
	imgs.lWidth = pImg->width;
	imgs.pixelArray.chunky.lLineBytes = pImg->widthStep;
	imgs.pixelArray.chunky.pPixel = pImg->imageData;
	//HYOLR_OilRecog(hTLHandle,&imgs,&resultlist);//识别油位窗口
	if(0!=HYOLR_OilRecog(hTLHandle,&imgs,&resultlist))	//寻找油位窗口
	{
		printf("未找到目标\n");
		printf("HYOLR_OilRecog error.\n");
		if (resultlist.pResult)
			free(resultlist.pResult);
		HYOLR_Uninit(hTLHandle);
		return -1;
	}
	/*if( resultlist.lResultNum==0)
	{
		printf("未找到目标\n");
		exit(-100);
	}*/
	for(int i=0;i < resultlist.lResultNum;i++)//显示结果
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
	for (int i = 0; i < resultlist.lResultNum; i++)	//每个油位窗口中确定油位
	{
		CvPoint startPt = { 0 };
		CvPoint endPt = { 0 };
		startPt.x = resultlist.pResult[i].Target.left;
		startPt.y = resultlist.pResult[i].Target.top;
		endPt.x = resultlist.pResult[i].Target.right;
		endPt.y = resultlist.pResult[i].Target.bottom;
		pImg = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
		cvSetImageROI(pImg, cvRect(startPt.x, startPt.y, endPt.x - startPt.x, endPt.y - startPt.y));
		cvSaveImage("../cut.jpg", pImg);
		cvResetImageROI(pImg);
		//两种油位分别讨论
		if (resultlist.pResult[i].dVal == 0)//oillevel
		{
			IplImage* src = cvLoadImage("../cut.jpg");
			void *hOHandle = NULL;
			int imgw = src->width;
			int imgh = src->height;
			OLR_IMAGES imgs_squareness = { 0 };
			HYOLR_RESULT_LIST  resultlist_squareness = { 0 };
			resultlist_squareness.pResult = (HYOLR_RESULT*)malloc(20 * sizeof(HYOLR_RESULT));
			//HYOLR_Init(NULL,&hOHandle);
			if (0 != HYOLR_Init(NULL, &hOHandle))
			{
				printf("HYOLR_Init error.\n");
				if (resultlist_squareness.pResult)
					free(resultlist_squareness.pResult);
				res = -1;
				goto EXT;
			}
			//HYOLR_SetParam(hOHandle,cfgfile_squareness,weightfile_squareness,thresh,imgw,imgh);
			if (0 != HYOLR_SetParam(hOHandle, cfgfile_squareness, weightfile_squareness, thresh, imgw, imgh))
			{
				printf("HYOLR_SetParam error.\n");
				if (resultlist_squareness.pResult)
					free(resultlist_squareness.pResult);
				HYOLR_Uninit(hOHandle);
				res = -1;
				goto EXT;
			}
			imgs_squareness.lHeight = src->height;
			imgs_squareness.lWidth = src->width;
			imgs_squareness.pixelArray.chunky.lLineBytes = src->widthStep;
			imgs_squareness.pixelArray.chunky.pPixel = src->imageData;
			//HYOLR_OilRecog(hOHandle,&imgs_squareness,&resultlist_squareness);//寻找油位
			if (HYOLR_OilRecog(hOHandle, &imgs_squareness, &resultlist_squareness) != 0)
			{
				printf("没有油位\n");
				if (resultlist_squareness.pResult)
					free(resultlist_squareness.pResult);
				HYOLR_Uninit(hOHandle);
				res = -1;
				goto EXT;
			}
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

			cvShowImage("Result Show", src);
			cvSaveImage("../cutoil.jpg", src);
			double percent = 0;
			if (resultlist_squareness.pResult[i].dVal == 0)
				percent = 100.0 - 100.0*resultlist_squareness.pResult[i].Target.top / imgh;
			else if (resultlist_squareness.pResult[i].dVal == 1)
				percent = 10.0 - 10.0*(resultlist_squareness.pResult[i].Target.top + resultlist_squareness.pResult[i].Target.bottom) / (imgh * 2);
			printf("percent=%f\n", percent);
			cvWaitKey(0);
			cvReleaseImage(&src);
			HYOLR_Uninit(hOHandle);
			if (resultlist_squareness.pResult)
				free(resultlist_squareness.pResult);


		}
		else if (resultlist.pResult[i].dVal == 1)//whiteblock
		{
			IplImage* src = cvLoadImage("../cut.jpg", CV_LOAD_IMAGE_GRAYSCALE);
			int oil[1] = { 0 };
			OL_IMAGES imgs1 = { 0 };
			imgs1.lHeight = src->height;
			imgs1.lWidth = src->width;
			imgs1.pixelArray.chunky.lLineBytes = src->widthStep;
			imgs1.pixelArray.chunky.pPixel = src->imageData;
			HY_whiteBlock(&imgs1, oil);//寻找白块中黑线
			//printf("%d\n",oil[0]);
			if (oil[0] == 0)
			{
				printf("未找到油位\n");
			}
			IplImage* dst = cvLoadImage("../cut.jpg");
			cvCircle(dst, cvPoint(dst->width / 2, oil[0]), 1, CV_RGB(255, 0, 0), -1, 8, 0);
			double percent = 100.0 - 100.0*oil[0] / src->height;
			printf("percent=%f\n", percent);
			cvShowImage("OIL Result Show", dst);
			cvSaveImage("../result.jpg", dst);
			//HY_OilLevel(&imgs1,oil,max,min);
			//printf("oil=%d\n",oil[0]);
			cvWaitKey(0);
			cvReleaseImage(&src);
			cvReleaseImage(&dst);
		}
	}

EXT:
	cvReleaseImage(&pImg);
	HYOLR_Uninit(hTLHandle);
	if (resultlist.pResult)
		free(resultlist.pResult);
}