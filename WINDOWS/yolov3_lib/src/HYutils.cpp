#define GPU
#include "HY_utils.h"
#include "yolo_dll.h"
#include <iostream>
#include <fstream>
#include <time.h>
#define PRINT 1
#define CELVE 1
using namespace std;
#include <opencv2/opencv.hpp>
#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif
void sort(int n, const float* x, int* indices);
//���ź���
cv::Mat HY_imReszie(cv::Mat pImg, float *im_scale_x, float *im_scale_y)
{
	cv::Mat dst;
	int s_min = min(pImg.cols, pImg.rows);//��Ҫ��ͼƬ������С������ٶ���׼ȷ�ȣ���ֵΪ����ֵ��300
	int s_max = max(pImg.cols, pImg.rows);// 900

	int size_align = 32;
	int im_height = pImg.rows;
	int im_width = pImg.cols;
	int im_size_min = min(im_height, im_width);
	int im_size_max = max(im_height, im_width);
	float im_scale = 1.0f;
	//float im_scale = s_min / (float)im_size_min;
	if (s_max > 3000)
	{
		s_max = 3000;
		if ((int)floorf(im_scale * im_size_max + 0.5f) > s_max) {
			im_scale = s_max / (float)im_size_max;
		}
	}

	if (s_min <= 192)
	{
		s_min = 192;
		if ((int)floorf(im_scale * im_size_min + 0.5f) < s_min) {
			im_scale = s_min / (float)im_size_min;
		}
	}
	//std::cout << "im_scale " << im_scale << std::endl;
	*im_scale_x = floor(im_width * im_scale / size_align) * size_align / im_width;
	*im_scale_y = floor(im_height * im_scale / size_align) * size_align / im_height;
	//std::cout << "im_scale_x " << *im_scale_x << std::endl;
	//std::cout << "im_scale_y " << *im_scale_y << std::endl;
	cv::resize(pImg, dst, cv::Size(0, 0), *im_scale_x, *im_scale_y, 1);


	return dst;
}
int detec_in(void *handle, cv::Mat input, int x1, int y1, int x2, int y2, float score, int &zibaoflag, int &wuhuiflag, int &totalwuhuiflag, int &zibaoflagS, int &wuhuiflagS,  \
	int &totalwuhuiflagS, HYYOLOV3RESULT_PLIST pResultList)
{
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(100 * sizeof(HYYOLOV3_RESULT));
	cv::Mat imageROI;
	int width = x2 - x1;
	int height = y2 - y1;
	imageROI = input(cv::Rect(x1, y1, width, height));

#ifdef RESIZE
	float im_scale_x;
	float im_scale_y;
	cv::Mat scale_img;
	scale_img = HY_imReszie(imageROI, &im_scale_x, &im_scale_y);
#else
	cv::Mat& scale_img= imageROI;
#endif // RESIZE

	YOLOV3_IMAGES img;
	img.lHeight = scale_img.rows;
	img.lWidth = scale_img.cols;
	img.pixelArray.chunky.lLineBytes = scale_img.step;
	img.pixelArray.chunky.pPixel = scale_img.data;
	img.lPixelArrayFormat = 3;

	int w = input.cols;
	int h = input.rows;
	int image_w_batch = 1;
	int image_h_batch = 1;
	image_w_batch = min(int(w / 1000 + 1), 4);
	image_h_batch = min(int(h / 1000 + 1), 4);


	int wstep = w / image_w_batch;
	int hstep = h / image_h_batch;
	int wstart[4] = { 0 };
	int wend[4] = { 0 };
	int hstart[4] = { 0 };
	int hend[4] = { 0 };
	double padding = 0.05;
	for (int i = 0; i < image_w_batch; i++)
	{
		int ws = wstep*i - wstep*padding;
		int we = wstep*(i + 1) + wstep*padding;
		if (ws<0 || we >= w)
		{
			ws = max(int(wstep*i - wstep*padding * 2), 0);
			we = min(int(wstep*(i + 1) + wstep*padding * 2), w - 1);
		}
		wstart[i] = ws;
		wend[i] = we;
	}
	for (int i = 0; i < image_h_batch; i++)
	{
		int hs = hstep*i - hstep*padding;
		int he = hstep*(i + 1) + hstep*padding;
		if (hs<0 || he >= h)
		{
			hs = max(int(hstep*i - hstep*padding * 2), 0);
			he = min(int(hstep*(i + 1) + hstep*padding * 2), h - 1);
		}
		hstart[i] = hs;
		hend[i] = he;
	}

	detec(handle, &img, &resultlist);
	for (int i; i < resultlist.lResultNum; i++)
	{
		if ((resultlist.pResult[i].dVal == 2 || resultlist.pResult[i].dVal == 3) /*&& resultlist.pResult[i].dConfidence < score + 0.06*/)  //��ʵ��
			continue;
		pResultList->pResult[pResultList->lResultNum] = resultlist.pResult[i];
		pResultList->pResult[pResultList->lResultNum].Target.left += x1;
		pResultList->pResult[pResultList->lResultNum].Target.top += y1;
		pResultList->pResult[pResultList->lResultNum].Target.right += x1;
		pResultList->pResult[pResultList->lResultNum].Target.bottom += y1;
		pResultList->pResult[pResultList->lResultNum].flag = (int)resultlist.pResult[i].dVal;   //yolo���תΪpvanet���
		pResultList->pResult[pResultList->lResultNum].dVal = resultlist.pResult[i].dConfidence; //yolo���תΪpvanet���
		pResultList->lResultNum++;
		if (resultlist.pResult[i].dVal == 1 && zibaoflag == 0)
				zibaoflag = 1;
		if (resultlist.pResult[i].dVal == 2 && wuhuiflag == 0)
			wuhuiflag = 1;
		if (resultlist.pResult[i].dVal == 3 && totalwuhuiflag == 0)
			totalwuhuiflag = 1;

		if (resultlist.pResult[i].dVal == 1 && zibaoflagS == 0)
			zibaoflagS = 1;
		if (resultlist.pResult[i].dVal == 2 && wuhuiflagS == 0)
			wuhuiflagS = 1;
		if (resultlist.pResult[i].dVal == 3 && totalwuhuiflagS == 0)
			totalwuhuiflagS = 1;

	}
	free(resultlist.pResult);
	imageROI.release();
#ifdef RESIZE
	scale_img.release();
#endif // RESIZE


	return 0;
}
//��⺯��
 void AllDetection(void *handleout, void *handlein, const char* picpath, char *savePath, char *fname, char *exts)
{
	float nmsthresh = 0.3;
	HYYOLOV3RESULT_LIST  Insulatorlist = { 0 };
	Insulatorlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	FILE* fpcsv;
	char name[256] = "jueyuanzi";
	//int* pick;
	int Num_rois = 0;//������Ŀ

	//------------------------------��һ���־�Ե�Ӽ��-------------------------------//
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		system("pause");
		exit(-1);
	}


	//printf("imread����ʱ��:%d\n", ((star1 - t11) * 1000 / CLOCKS_PER_SEC));
	
	
	//��ͼ���С���е��ڣ�����ΪС��ͼ���С�����32����������С
#ifdef RESIZE
	float im_scale_x;
	float im_scale_y;
	cv::Mat scale_im;
	scale_im = HY_imReszie(input, &im_scale_x, &im_scale_y);
#else
	cv::Mat& scale_im= input;
#endif // RESIZE

	
	//printf("ͼƬresizeʱ��:%d\n", ((t00 - star1) * 1000 / CLOCKS_PER_SEC));
	//printf("Ԥ����ʱ��:%d\n", ((t00 - t0) * 1000 / CLOCKS_PER_SEC));
	

	//���õ�һ���־�Ե�Ӽ��
	
	//std::cout << "im_width " << scale_im.cols << " " << "im_height " << scale_im.rows << std::endl;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec(handleout, &img, &Insulatorlist);

	

	//printf("��һ�����ʱ��:%d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));
	//------------------------------�ڶ����־�Ե�Ӽ��-------------------------------//
	HYYOLOV3RESULT_LIST  Defectlist[100] = { 0 };
	for (int i = 0; i < 100; i++)
	{
		Defectlist[i].pResult = (HYYOLOV3_RESULT*)malloc(100 * sizeof(HYYOLOV3_RESULT));
	}
	int zibaoflag = 0;
	int wuhuiflag = 0;
	int totalwuhuiflag = 0;
	int zibaoflagS[100] = { 0 };
	int wuhuiflagS[100] = { 0 };
	int totalwuhuiflagS[100] = { 0 };

	for (int j = 0; j < Insulatorlist.lResultNum; j++)
	{
		int width = Insulatorlist.pResult[j].Target.right - Insulatorlist.pResult[j].Target.left;
		int height = Insulatorlist.pResult[j].Target.bottom - Insulatorlist.pResult[j].Target.top;
		int x1 = Insulatorlist.pResult[j].Target.left - 0.1*width;
		int y1 = Insulatorlist.pResult[j].Target.top - 0.1*height;
		int x2 = Insulatorlist.pResult[j].Target.right + 0.1*width;
		int y2 = Insulatorlist.pResult[j].Target.bottom + 0.1*height;
		Insulatorlist.pResult[j].flag = (int)Insulatorlist.pResult[j].dVal;  //yolo���תΪpvanet���
		Insulatorlist.pResult[j].dVal = Insulatorlist.pResult[j].dConfidence;  //yolo���תΪpvanet���
		if (x1 < 0)
			x1 = Insulatorlist.pResult[j].Target.left;
		if (y1 < 0)
			y1 = Insulatorlist.pResult[j].Target.top;
		if (x2 >= input.cols)
			x2 = Insulatorlist.pResult[j].Target.right;
		if (y2 >= input.rows)
			y2 = Insulatorlist.pResult[j].Target.bottom;

		

		detec_in(handlein, input,x1,y1,x2,y2,0.76, zibaoflag, wuhuiflag, totalwuhuiflag,  \
			zibaoflagS[j], wuhuiflagS[j], totalwuhuiflagS[j], &Defectlist[j]);

		if (zibaoflagS[j] == 1)
		{
			wuhuiflagS[j] = 0;
			totalwuhuiflagS[j] = 0;
		}
		else if (wuhuiflagS[j] == 1)
		{
			totalwuhuiflagS[j] = 0;
		}
	}

	//printf("�ڶ������ʱ��:%d\n", ((t4 - t3) * 1000 / CLOCKS_PER_SEC));

	//-----------------------д�����------------------------------//
	//#if PRINT
	cv::Mat resultImage;
	resultImage = input.clone();
	//#endif
	fpcsv = fopen(savePath, "at+");
	if (NULL == fpcsv)
	{
		printf("can not open file %s!\n", savePath);
		exit(-1);
	}
	int DefectType = 0;
	int zibaonum = 0;
	int Defectnum = 0;
	int Defectmaxnum = 4;

	//std::cout << "Insulatorlist.lResultNum=" << Insulatorlist.lResultNum << std::endl;
	if (zibaoflag == 1)
	{
		DefectType++;
		for (int j = 0; j < Insulatorlist.lResultNum; j++)
		{
			if (zibaoflagS[j] == 0)continue;
			if (zibaonum == 2)break;
			if (Defectnum == Defectmaxnum)break;
			//	std::cout << "Defectlist[j].lResultNum=" << Defectlist[j].lResultNum << std::endl;
			for (int i = 0; i < Defectlist[j].lResultNum; i++)
			{
				if (zibaonum == 2)break;
				if (Defectnum == Defectmaxnum)break;
				if (Defectlist[j].pResult[i].flag == 1)
				{
					int width = Defectlist[j].pResult[i].Target.right - Defectlist[j].pResult[i].Target.left;
					int height = Defectlist[j].pResult[i].Target.bottom - Defectlist[j].pResult[i].Target.top;
					if (width*height < 1800)continue;
					zibaonum++;
					Defectnum++;

					fprintf(fpcsv, "%s%s,%s,%.12f,%d,%d,%d,%d\n", fname, exts, name, Defectlist[j].pResult[i].dVal, Defectlist[j].pResult[i].Target.left, Defectlist[j].pResult[i].Target.top, Defectlist[j].pResult[i].Target.right, Defectlist[j].pResult[i].Target.bottom);
#if CELVE
					char text[20] = { 0 };
					int x = Defectlist[j].pResult[i].Target.left;
					int y = Defectlist[j].pResult[i].Target.top;

					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(255, 0, 0), 4);
					//sprintf(text, "%s %0.3f", "zibao", dstcls_scores_t[pick[i]]);
					sprintf(text,"%s %0.3f", "zibao", Defectlist[j].pResult[i].dVal);
					cv::putText(resultImage, text, cv::Point(x, y+30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(255, 0, 0),6);
				
#endif
				}
			}
		}
	}
	if (wuhuiflag == 1 && DefectType != 2 && zibaoflag != 1)
	{
		DefectType++;
		for (int j = 0; j < Insulatorlist.lResultNum; j++)
		{
			if (wuhuiflagS[j] == 0)continue;
			if (Defectnum == Defectmaxnum)break;
			for (int i = 0; i < Defectlist[j].lResultNum; i++)
			{
				if (Defectnum == Defectmaxnum)break;
				if (Defectlist[j].pResult[i].flag == 2)
				{
					int width = Defectlist[j].pResult[i].Target.right - Defectlist[j].pResult[i].Target.left;
					int height = Defectlist[j].pResult[i].Target.bottom - Defectlist[j].pResult[i].Target.top;
					if (width*height < 1800)continue;
					Defectnum++;
					fprintf(fpcsv, "%s%s,%s,%.12f,%d,%d,%d,%d\n", fname, exts, name, Defectlist[j].pResult[i].dVal, Defectlist[j].pResult[i].Target.left, Defectlist[j].pResult[i].Target.top, Defectlist[j].pResult[i].Target.right, Defectlist[j].pResult[i].Target.bottom);
#if CELVE
					char text[20] = { 0 };
					int x = Defectlist[j].pResult[i].Target.left;
					int y = Defectlist[j].pResult[i].Target.top;

					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 4);
					sprintf(text, "%s %0.3f", "wuhui", Defectlist[j].pResult[i].dVal);
					
					cv::putText(resultImage, text, cv::Point(x, y+30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
				}
			}
		}
	}
	if (totalwuhuiflag == 1 && DefectType != 2 && zibaoflag != 1)
	{
		DefectType++;
		for (int j = 0; j < Insulatorlist.lResultNum; j++)
		{
			if (totalwuhuiflagS[j] == 0)continue;
			if (Defectnum == Defectmaxnum)break;
			if (zibaonum == 1 && Defectnum == 3)break;
			for (int i = 0; i < Defectlist[j].lResultNum; i++)
			{
				if (Defectnum == Defectmaxnum)break;
				if (zibaonum == 1 && Defectnum == 3)break;
				if (Defectlist[j].pResult[i].flag == 3)
				{
					int width = Defectlist[j].pResult[i].Target.right - Defectlist[j].pResult[i].Target.left;
					int height = Defectlist[j].pResult[i].Target.bottom - Defectlist[j].pResult[i].Target.top;
					if (width*height < 1800)continue;
					Defectnum++;
					fprintf(fpcsv, "%s%s,%s,%.12f,%d,%d,%d,%d\n", fname, exts, name, Defectlist[j].pResult[i].dVal, Defectlist[j].pResult[i].Target.left, Defectlist[j].pResult[i].Target.top, Defectlist[j].pResult[i].Target.right, Defectlist[j].pResult[i].Target.bottom);
#if CELVE
					char text[20] = { 0 };
					int x = Defectlist[j].pResult[i].Target.left;
					int y = Defectlist[j].pResult[i].Target.top;
					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
					sprintf(text, "%s %0.3f", "totalwuhui", Defectlist[j].pResult[i].dVal);
					cv::putText(resultImage, text, cv::Point(x, y+30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255),6);
#endif
					break;//һ����Ե��ֻ��һ��totalwuhui
				}
			}
		}
	}
	fclose(fpcsv);

#if PRINT && !CELVE
	for (int j = 0; j < Insulatorlist.lResultNum; j++)
	{
		for (int i = 0; i < Defectlist[j].lResultNum; i++)
		{
			char text[256] = { 0 };
			if (Defectlist[j].pResult[i].flag == 1)
				sprintf(text, "zibao");
			else if (Defectlist[j].pResult[i].flag == 2)
				sprintf(text, "wuhui");
			else if (Defectlist[j].pResult[i].flag == 3)
				sprintf(text, "totalwuhui");
			else
				continue;
			int x = Defectlist[j].pResult[i].Target.left;
			int y = Defectlist[j].pResult[i].Target.top;
			int width = Defectlist[j].pResult[i].Target.right - Defectlist[j].pResult[i].Target.left;
			int height = Defectlist[j].pResult[i].Target.bottom - Defectlist[j].pResult[i].Target.top;
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(255, 0, 0), 4);//��ɫ
			cv::putText(resultImage, text, cv::Point(x, y), CV_FONT_HERSHEY_TRIPLEX, 4, cv::Scalar(0, 255, 255));
		}
	}
#endif
#if PRINT
	char outputFile[100];
	sprintf_s(outputFile, "../result30001/result_%s.jpg", fname);
	cv::imwrite(outputFile, resultImage);

#endif

	//printf("д���ʱ��:%d\n", ((t6 - t5) * 1000 / CLOCKS_PER_SEC));

	input.release();
#ifdef RESIZE
	scale_im.release();
#endif
	//free(pick), pick = NULL;

	for (int i = 0; i < 100; i++)
	{
		if (Defectlist[i].pResult)
		{
			free(Defectlist[i].pResult);
			Defectlist[i].pResult = NULL;
		}

	}
	if(Insulatorlist.pResult)
	{ 
		free(Insulatorlist.pResult);
		Insulatorlist.pResult = NULL;
	}
	

	//printf("�ͷ���Դʱ��:%d\n", ((t8 - t7) * 1000 / CLOCKS_PER_SEC));
}
//inline void cutsize()
//{
//
//}
void Detection_xiaojinju(void *handle, const char* picpath, char *savePath, char *fname, char *exts)
{
	FILE* fpcsv;
	char name[256] = "xiaochicun";
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(160 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		system("pause");
		exit(-1);
	}
	int w = input.cols;
	int h = input.rows;
	int image_w_batch = 1;
	int image_h_batch = 1;
	image_w_batch = min(int(w / 1000 + 1), 3);
	image_h_batch = min(int(h / 1000 + 1), 3); 

	printf("w=%d h=%d\n",image_w_batch, image_h_batch);
	int wstep = w / image_w_batch;
	int hstep = h / image_h_batch;
	int wstart[4] = { 0 };
	int wend[4] = { 0 };
	int hstart[4] = { 0 };
	int hend[4] = { 0 };
	double padding = 0.05;
	for (int i=0; i < image_w_batch; i++)
	{
		int ws = wstep*i - wstep*padding;
		int we = wstep*(i + 1) + wstep*padding;
		if (ws<0 || we >=w)
		{
			ws = max(int(wstep*i - wstep*padding * 2), 0);
			we = min(int(wstep*(i + 1) + wstep*padding * 2), w-1);
		}
		wstart[i] = ws;
		wend[i] = we;
	}
	for (int i = 0; i < image_h_batch; i++)
	{
		int hs = hstep*i - hstep*padding;
		int he = hstep*(i + 1) + hstep*padding;
		if (hs<0 || he >= h)
		{
			hs = max(int(hstep*i - hstep*padding * 2), 0);
			he = min(int(hstep*(i + 1) + hstep*padding * 2), h-1);
		}
		hstart[i] = hs;
		hend[i] = he;
	}

	for (int j = 0; j < image_h_batch; j++)
	{
		for (int i = 0; i < image_w_batch; i++)
		{
			HYYOLOV3RESULT_LIST  resultlistcut = { 0 };
			resultlistcut.pResult = (HYYOLOV3_RESULT*)malloc(10 * sizeof(HYYOLOV3_RESULT));
			cv::Rect rect(wstart[i], hstart[j], wend[i] - wstart[i], hend[j] - hstart[j]);
			cv::Mat scale_im = input(rect);
			/*char ImgoutputFile[100];
			sprintf_s(ImgoutputFile, "..\\..\\result\\result_%s_%d.jpg", fname, i + j*image_w_batch);
			cv::imwrite(ImgoutputFile, scale_im);
			printf("left=%d ,right=%d ,top=%d ,bot=%d\n",wstart[i], wend[i], hstart[j], hend[j]);*/
			YOLOV3_IMAGES img;
			img.lHeight = scale_im.rows;
			img.lWidth = scale_im.cols;
			img.pixelArray.chunky.lLineBytes = scale_im.step;
			img.pixelArray.chunky.pPixel = scale_im.data;
			img.lPixelArrayFormat = 3;

			detec(handle, &img, &resultlistcut);
			int dropnum = 0;
			for (int k = 0; k < resultlistcut.lResultNum; k++)
			{
				int wcut = resultlistcut.pResult[k].Target.right - resultlistcut.pResult[k].Target.left;
				int hcut = resultlistcut.pResult[k].Target.bottom - resultlistcut.pResult[k].Target.top;
				if (1.0*wcut / hcut > 2.5 || 1.0*hcut / wcut > 2.5)//�����
				{
					dropnum = dropnum + 1;
					continue;
				}
				/*int tupianmianji = img.lWidth*img.lHeight;
				int mianji = (resultlistcut.pResult[k].Target.right - resultlistcut.pResult[k].Target.left)*(resultlistcut.pResult[k].Target.bottom - resultlistcut.pResult[k].Target.top);
				if (1.0*mianji / tupianmianji > 0.4)
				{
					dropnum = dropnum + 1;
					continue;
				}*/
				resultlistcut.pResult[k].flag = i + j*image_w_batch;//����flag
				resultlist.pResult[resultlist.lResultNum+k- dropnum] = resultlistcut.pResult[k];
				/*printf("bot=%d\n", resultlistcut.pResult[k].Target.bottom);
				printf("top=%d\n", resultlistcut.pResult[k].Target.top);
				printf("right=%d\n", resultlistcut.pResult[k].Target.right);
				printf("left=%d\n", resultlistcut.pResult[k].Target.left);
				printf("dConfidence=%f\n", resultlistcut.pResult[k].dConfidence);*/
				//printf("bot=%d\n",resultlist.pResult[resultlist.lResultNum + k].Target.bottom);
				//printf("top=%d\n", resultlist.pResult[resultlist.lResultNum + k].Target.top);
				//printf("right=%d\n", resultlist.pResult[resultlist.lResultNum + k].Target.right);
				//printf("left=%d\n", resultlist.pResult[resultlist.lResultNum + k].Target.left);
				resultlist.pResult[resultlist.lResultNum + k - dropnum].Target.bottom += hstart[j];
				resultlist.pResult[resultlist.lResultNum + k - dropnum].Target.top += hstart[j];
				resultlist.pResult[resultlist.lResultNum + k - dropnum].Target.right += wstart[i];
				resultlist.pResult[resultlist.lResultNum + k - dropnum].Target.left += wstart[i];
			}
			//resultlist.lResultNum += resultlistcut.lResultNum;
			resultlist.lResultNum = resultlist.lResultNum + resultlistcut.lResultNum - dropnum;
			if (resultlistcut.pResult)
			{
				free(resultlistcut.pResult);
				resultlistcut.pResult = NULL;
			}
		}
	}
	float overlap = 0.6;
	for (int i = 0; i < resultlist.lResultNum; i++)   //nms
	{
		/*printf("%d: \n", i);
		printf("bot=%d ",resultlist.pResult[i].Target.bottom);
		printf("top=%d ", resultlist.pResult[i].Target.top);
		printf("right=%d ", resultlist.pResult[i].Target.right);
		printf("left=%d\n", resultlist.pResult[i].Target.left);*/
		if (resultlist.pResult[i].flag < 0)
			continue;
		for (int j = i+1; j < resultlist.lResultNum; j++)
		{
			if (resultlist.pResult[j].flag < 0)
				continue;
			float x1max = max(resultlist.pResult[i].Target.left, resultlist.pResult[j].Target.left);                    // �������������Ͻ�x�������ֵ 
			float x2min = min(resultlist.pResult[i].Target.right, resultlist.pResult[j].Target.right);    // �������������½�x������Сֵ 
			float y1max = max(resultlist.pResult[i].Target.top, resultlist.pResult[j].Target.top);                    // �������������Ͻ�y�������ֵ 
			float y2min = min(resultlist.pResult[i].Target.bottom, resultlist.pResult[j].Target.bottom);   // �������������½�y������Сֵ 
			float overlapWidth = max(0.0f, (x2min - x1max + 1));            // �����������ص��Ŀ�� 
			float overlapHeight = max(0.0f, (y2min - y1max + 1));           // �����������ص��ĸ߶� 
			if (overlapWidth > 0 && overlapHeight > 0)
			{
				
				//float overlapPart = (overlapWidth * overlapHeight) / box_area[indices[j]];    // �����ص��ı���
				int box_area_i = (resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left)*(resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top);
				int box_area_j = (resultlist.pResult[j].Target.right - resultlist.pResult[j].Target.left)*(resultlist.pResult[j].Target.bottom - resultlist.pResult[j].Target.top);
				float inter = overlapWidth * overlapHeight;
				float overlapPart = inter / (box_area_i + box_area_j - inter);
				/*if (i == 1 && j == 4)
				{
					printf("%f %f\n", inter / box_area_i, inter / box_area_j);
				}*/
				if (inter / box_area_i > overlap && inter / box_area_j > overlap)
				{
					if (resultlist.pResult[i].dConfidence > resultlist.pResult[j].dConfidence)
					{
						resultlist.pResult[j].flag = -1;
					}
					else
					{
						resultlist.pResult[i].flag = -1;
					}
				}
				else if (inter / box_area_i > overlap)
				{
					resultlist.pResult[i].flag = -1;
				}
				else if (inter / box_area_j > overlap)
				{
					resultlist.pResult[j].flag = -1;
				}

				//if (overlapPart > overlap)          // �ж��ص������Ƿ񳬹��ص���ֵ 
				//{
				//	if (resultlist.pResult[i].dConfidence > resultlist.pResult[j].dConfidence)
				//	{
				//		resultlist.pResult[j].flag = -1;
				//	}
				//	else
				//	{
				//		resultlist.pResult[i].flag = -1;
				//	}
				//}
			}
		}
	}
	fpcsv = fopen(savePath, "at+");
	if (NULL == fpcsv)
	{
		printf("can not open file %s!\n", savePath);
		exit(-1);
	}
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		if (resultlist.pResult[i].flag < 0)
			continue;

		fprintf(fpcsv, "%s%s,%s,%.12f,%d,%d,%d,%d\n", fname, exts, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
		if(resultlist.pResult[i].dVal==0)
		{
			sprintf(text, "%s %0.3f", "lsqxz", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
			sprintf(text, "%s %0.3f", "lszc", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}

		/******************************************print********************************************/
		
	}
	char outputFile[100];
	sprintf_s(outputFile, "..\\..\\result\\result_%s.jpg", fname);
	cv::imwrite(outputFile, resultImage);

	fclose(fpcsv);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}
void Detection_dajinju(void *handle, const char* picpath,char *savePath, char *fname, char *exts)
{
	FILE* fpcsv;
	char name[256] = "dachicun";
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		system("pause");
		exit(-1);
	}

	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec(handle, &img, &resultlist);

	fpcsv = fopen(savePath, "at+");
	if (NULL == fpcsv)
	{
		printf("can not open file %s!\n", savePath);
		exit(-1);
	}
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		fprintf(fpcsv, "%s%s,%s,%.12f,%d,%d,%d,%d\n", fname, exts, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
		if (resultlist.pResult[i].dVal == 0)
		{
			sprintf(text, "%s %0.3f", "xjqx", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
			sprintf(text, "%s %0.3f", "jyhsh", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}
		else if (resultlist.pResult[i].dVal == 2)
		{
			sprintf(text, "%s %0.3f", "fzcsh", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}
		else if (resultlist.pResult[i].dVal == 3)
		{
			sprintf(text, "%s %0.3f", "xj", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}
		else if (resultlist.pResult[i].dVal == 4)
		{
			sprintf(text, "%s %0.3f", "jyh", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}
		else if (resultlist.pResult[i].dVal == 5)
		{
			sprintf(text, "%s %0.3f", "fzc", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 4);
			//cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
		}

		/******************************************print********************************************/
	}
	char outputFile[100];
	sprintf_s(outputFile, "..\\..\\result\\result_%s.jpg", fname);
	cv::imwrite(outputFile, resultImage);
	
	fclose(fpcsv);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}
//������
/*
*dstcls_boxes_t������λ��
*dstcls_scores_t�����ӵ÷�
*pick:���ӱ��
*CLASS_THRESHs:��ֵ
*img_idx��ͼƬ���
*/
 int draw_rect(cv::Mat image, int* dstcls_cls, float*& dstcls_boxes_t, float*& dstcls_scores_t, int*& pick, float CLASS_THRESHS, int img_idx)
{
	cv::Mat resultImage;
	resultImage = image.clone();
	int flag = 0;
	for (int i = 1; i <= pick[0]; i++)
	{
		//���໭����ͬ�Ŀ���
		int c = dstcls_cls[pick[i]];

		switch (c)
		{
		case 1:

			if (dstcls_scores_t[pick[i]] > CLASS_THRESHS)//�������Ŷ���0.75���ϵĿ���
			{
				flag = 1;
				float x = dstcls_boxes_t[pick[i] * 4];//���ε����Ͻ�x����
				float y = dstcls_boxes_t[pick[i] * 4 + 1];//���ε����Ͻ�y����
				float height = dstcls_boxes_t[pick[i] * 4 + 3] - dstcls_boxes_t[pick[i] * 4 + 1];//���εĳ�
				float width = dstcls_boxes_t[pick[i] * 4 + 2] - dstcls_boxes_t[pick[i] * 4];//���εĿ� 
				cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//��ɫ				
				//cv::putText(resultImage, "Waring_1", cv::Point(x, y), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 0));
				//std::cout << "socre" << i << ": " << dstcls_scores_t[pick[i]] << std::endl;

			}
			break;
			//case 2:
			//	if (dstcls_scores_t[pick[i]] > CLASS_THRESHS)//�������Ŷ���0.75���ϵĿ���
			//	{
			//		float x = dstcls_boxes_t[pick[i] * 4];//���ε����Ͻ�x����
			//		float y = dstcls_boxes_t[pick[i] * 4 + 1];//���ε����Ͻ�y����
			//		float height = dstcls_boxes_t[pick[i] * 4 + 3] - dstcls_boxes_t[pick[i] * 4 + 1];//���εĳ�
			//		float width = dstcls_boxes_t[pick[i] * 4 + 2] - dstcls_boxes_t[pick[i] * 4];//���εĿ� 
			//		cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//��ɫ
			//		//cv::putText(resultImage, "Waring_2", cv::Point(x, y), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 0));
			//		//std::cout << "socre" << i << ": " << dstcls_scores_t[pick[i]] << std::endl;
			//	}
			//	break;

			//case 3:
			//	if (dstcls_scores_t[pick[i]] > CLASS_THRESHS)//�������Ŷ���0.75���ϵĿ���
			//	{
			//		float x = dstcls_boxes_t[pick[i] * 4];//���ε����Ͻ�x����
			//		float y = dstcls_boxes_t[pick[i] * 4 + 1];//���ε����Ͻ�y����
			//		float height = dstcls_boxes_t[pick[i] * 4 + 3] - dstcls_boxes_t[pick[i] * 4 + 1];//���εĳ�
			//		float width = dstcls_boxes_t[pick[i] * 4 + 2] - dstcls_boxes_t[pick[i] * 4];//���εĿ� 
			//		cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(255, 0, 255), 2);//��ɫ
			//		//cv::putText(resultImage, "Others", cv::Point(x, y), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(0, 0, 0));
			//		std::cout << "socre" << i << ": " << dstcls_scores_t[pick[i]] << std::endl;
			//	}

			//	break;
		default:
			break;
		}

	}

	if ((pick[0] == 0) || (flag == 0) || (dstcls_scores_t[pick[1]] < CLASS_THRESHS))//���Ӹ���Ϊ0�����ߵ÷�С����ֵ����������û�п����ļ���
	{
		char outputFile[100];
		sprintf(outputFile, "../result/result_nobox/res_%d.jpg", img_idx);//�ļ���·�� ����
		cv::imwrite(outputFile, resultImage);
	}
	else  //���������п����ļ���
	{
		char outputFile2[100];
		sprintf(outputFile2, "../result/result_box/res_%d.jpg", img_idx);//�ļ���·�� ����
		cv::imwrite(outputFile2, resultImage);
	}
	resultImage.release();
	return 0;

}

// ʵ�ּ����ľ��δ��ڵķǼ���ֵ����
// numBoxes��������Ŀ
// dstcls_scores_t�����ڵ÷�
// dstcls_boxes_t������λ��
// overlap���ص���ֵ����

 void nms(int numBoxes, float overlap, int*& pick, const float* dstcls_boxes_t, const float* dstcls_scores_t)
{
	int i, j, index;
	int numBoxesOut = 0;//���������Ŀ
	float* box_area = (float*)malloc(numBoxes * sizeof(float));   // ���崰���������������ռ� 
	int* indices = (int*)malloc(numBoxes * sizeof(int));          // ���崰������������ռ� 
	int* is_suppressed = (int*)malloc(numBoxes * sizeof(int));    // �����Ƿ����Ʊ��־������ռ� 
	// ��ʼ��indices��is_supperssed��box_area��Ϣ 
	for (i = 0; i < numBoxes; i++)
	{
		indices[i] = i;
		is_suppressed[i] = 0;
		box_area[i] = (float)((dstcls_boxes_t[i * 4 + 2] - dstcls_boxes_t[i * 4] + 1) * (dstcls_boxes_t[i * 4 + 3] - dstcls_boxes_t[i * 4 + 1] + 1));
	}
	// �����봰�ڰ��շ�����ֵ�������������ı�ŷ���indices�� 
	sort(numBoxes, dstcls_scores_t, indices);
	for (i = 0; i < numBoxes; i++)                // ѭ�����д��� 
	{
		if (!is_suppressed[indices[i]])           // �жϴ����Ƿ����� 
		{
			for (j = i + 1; j < numBoxes; j++)    // ѭ����ǰ����֮��Ĵ��� 
			{
				if (!is_suppressed[indices[j]])   // �жϴ����Ƿ����� 
				{
					float x1max = max(dstcls_boxes_t[indices[i] * 4], dstcls_boxes_t[indices[j] * 4]);                    // �������������Ͻ�x�������ֵ 
					float x2min = min(dstcls_boxes_t[indices[i] * 4 + 2], dstcls_boxes_t[indices[j] * 4 + 2]);     // �������������½�x������Сֵ 
					float y1max = max(dstcls_boxes_t[indices[i] * 4 + 1], dstcls_boxes_t[indices[j] * 4 + 1]);                     // �������������Ͻ�y�������ֵ 
					float y2min = min(dstcls_boxes_t[indices[i] * 4 + 3], dstcls_boxes_t[indices[j] * 4 + 3]);     // �������������½�y������Сֵ 
					float overlapWidth = max(0.0f, (x2min - x1max + 1));            // �����������ص��Ŀ�� 
					float overlapHeight = max(0.0f, (y2min - y1max + 1));           // �����������ص��ĸ߶� 
					if (overlapWidth > 0 && overlapHeight > 0)
					{
						//float overlapPart = (overlapWidth * overlapHeight) / box_area[indices[j]];    // �����ص��ı���

						float inter = overlapWidth * overlapHeight;
						float overlapPart = inter / (box_area[indices[i]] + box_area[indices[j]] - inter);
						if (overlapPart > overlap)          // �ж��ص������Ƿ񳬹��ص���ֵ 
						{
							is_suppressed[indices[j]] = 1;           // ������j���Ϊ���� 
						}
					}
				}
			}
		}
	}

	// ��ʼ�����������Ŀ0 
	for (i = 0; i < numBoxes; i++)
	{
		if (!is_suppressed[i]) (numBoxesOut)++;    // ͳ�����������Ŀ 
	}

	pick = (int*)malloc((numBoxesOut + 1) * sizeof(int));         // ����������������ռ�
	index = 1;
	pick[0] = numBoxesOut;//
	for (i = 0; i < numBoxes; i++)                  // �����������봰�� 
	{
		if (!is_suppressed[indices[i]])             // ��δ�������ƵĴ�����Ϣ���浽�����Ϣ�� 
		{
			pick[index] = indices[i];
			index++;
		}
	}

	free(indices);          // �ͷ�indices�ռ� 
	free(box_area);         // �ͷ�box_area�ռ� 
	free(is_suppressed);    // �ͷ�is_suppressed�ռ� 	
}

// ���������������н�������indices�е�����
// n����������
// x����������
// indices����ʼΪ0~n-1��Ŀ 
void sort(int n, const float* x, int* indices)
{
	int i, j;
	for (i = 0; i < n; i++)
		for (j = i + 1; j < n; j++)
		{
			if (x[indices[j]] > x[indices[i]])
			{
				//float x_tmp = x[i];
				int index_tmp = indices[i];
				//x[i] = x[j];
				indices[i] = indices[j];
				//x[j] = x_tmp;
				indices[j] = index_tmp;
			}
		}
}


/************************/
 void del_arg(int argc, char **argv, int index)
{
	int i;
	for (i = index; i < argc - 1; ++i) argv[i] = argv[i + 1];
	argv[i] = 0;
}

 int find_int_arg(int argc, char **argv, char *arg, int def)
{
	int i;
	for (i = 0; i < argc - 1; ++i){
		if (!argv[i]) continue;
		if (0 == strcmp(argv[i], arg)){
			def = atoi(argv[i + 1]);
			del_arg(argc, argv, i);
			del_arg(argc, argv, i);
			break;
		}
	}
	return def;
}

 float find_float_arg(int argc, char **argv, char *arg, float def)
{
	int i;
	for (i = 0; i < argc - 1; ++i){
		if (!argv[i]) continue;
		if (0 == strcmp(argv[i], arg)){
			def = atof(argv[i + 1]);
			del_arg(argc, argv, i);
			del_arg(argc, argv, i);
			break;
		}
	}
	return def;
}

 char *find_char_arg(int argc, char **argv, char *arg, char *def)
{
	int i;
	for (i = 0; i < argc - 1; ++i){
		if (!argv[i]) continue;
		if (0 == strcmp(argv[i], arg)){
			def = argv[i + 1];
			del_arg(argc, argv, i);
			del_arg(argc, argv, i);
			break;
		}
	}
	return def;
}
/************************/