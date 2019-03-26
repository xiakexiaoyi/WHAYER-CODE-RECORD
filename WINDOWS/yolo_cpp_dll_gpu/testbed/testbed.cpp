#include "meterDec.h"
#include <io.h>
#include <iostream>
#include <opencv2/opencv.hpp>

typedef void*					MHandle;


int main()//单独对表盘检测测试
{
	MHandle hTLHandle = NULL;
	struct _finddata_t fileinfo;
	//保存文件句柄 
	//long fHandle;
	intptr_t fHandle;
	char to_search[9000];
	char single_img[9000];//单张图片文件路径
	char result_img[9000];//单张图片文件路径
	//char *cfgfile = "../model/tiny-yolo-voc.cfg";
	//char *weightfile = "../model/tiny-yolo-voc_final.weights";
	char *cfgfile = "../zhizhenmodel/tiny-yolo-voc.cfg";
	char *weightfile = "../zhizhenmodel/tiny-yolo-voc_final.weights";
	float thresh = 0.24;

	MR_IMAGES imgs = { 0 };
	char single_picname[9000];//单张图片文件路径
	int w = 0, h = 0;
	if (0 != HYMR_Init(NULL, &hTLHandle))
	{

		printf("HYDSR_Init error.\n");
		return -1;
	}
	if (0 != HYMR_SetParam(hTLHandle, cfgfile, weightfile, thresh, w, h))
	{
		printf("HYDSR_SetParam error.\n");
		return -1;
	}


	printf("请输入测试单张照片路径：（例如输入:E:\\nest_photos\\1.jpg)(注：路径中不能包含空格)(回车确定)\n");


	scanf("%s", &single_picname);

	printf("分析开始\n");
	char result_path[9000] = "..\\result\\";//单张图片文件路径


	sprintf(to_search, "%s\\*.bmp", single_picname);

	if ((fHandle = _findfirst(to_search, &fileinfo)) == -1L)
	{
		int fileOrNot = 1;
		int len = strlen(fileinfo.name);
		fileinfo.name[len - 4] = '\0';
		//sprintf(result_img, "%s\\%s_result.jpg", result_path, fileinfo.name);
		char *result_img = "..\\result\\result.jpg";

	}
	else{
		do{
			HYMR_RESULT_LIST  resultlist = { 0 };
			int fileOrNot = 2;
			int len = strlen(fileinfo.name);
			sprintf(single_img, "%s\\%s", single_picname, fileinfo.name);
			fileinfo.name[len - 4] = '\0';
			//sprintf(single_img, "%s\\%s.jpg", single_picname, fileinfo.name);

			//Detec_disconnect(single_img, result_img, fileinfo.name, yolo, thresh, fileOrNot);
			IplImage *testImage = cvLoadImage(single_img);
			IplImage *testImage416 = cvCreateImage(cvSize(416, 416), testImage->depth, testImage->nChannels);
			cvCopyMakeBorder(testImage, testImage416, cvPoint(0, 0), IPL_BORDER_CONSTANT,cvScalarAll(255));
			
			imgs.lWidth = testImage416->width;
			imgs.lHeight = testImage416->height;
			imgs.pixelArray.chunky.lLineBytes = testImage416->widthStep;
			imgs.pixelArray.chunky.pPixel = testImage416->imageData;
			//imgs.lPixelArrayFormat = HYAMR_IMAGE_BGR;
			resultlist.pResult = (HYMR_RESULT*)malloc(20 * sizeof(HYMR_RESULT));
			if (0 != HYMR_meterRecog(hTLHandle, &imgs, &resultlist))	//
			{
				printf("未找到目标\n");
				printf("HYOLR_OilRecog error.\n");
			}
			for (int i = 0; i<resultlist.lResultNum; i++)
			{
				cvRectangle(testImage416, cvPoint(resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top), cvPoint(resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom), cvScalar(0, 0, 255), 4);
			}
			if (resultlist.lResultNum == 0)
			{
				sprintf(result_img, "%s\\result\\%s_resultnone.jpg", single_picname, fileinfo.name);
			}
			else
			{
				sprintf(result_img, "%s\\result\\%s_result_%f.jpg", single_picname, fileinfo.name, resultlist.pResult[0].dConfidence);
			}
			printf("%s\n", result_img);
			cvSaveImage(result_img, testImage416);
			free(resultlist.pResult);
			cvReleaseImage(&testImage);
			cvReleaseImage(&testImage416);
		} while (_findnext(fHandle, &fileinfo) == 0);
	}


}