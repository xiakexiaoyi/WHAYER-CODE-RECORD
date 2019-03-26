#include <opencv2/opencv.hpp>
/***********************/
//#include "HY_Network.h"
#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
extern "C"
{
#endif

#include "detector.h"

#ifdef __cplusplus
}
#endif
/***********************/
using namespace std;
using namespace cv;

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} MARECT, *PMARECT;

typedef struct{
	int flag;
	int next;
	Point CPoint;
	float *DigitForm;
} sortable_DigitForm;

int mouseX = -1, mouseY = -1, mouseFlag = 0;
int flagEndL = 0, flagEndR = 0;
void onMouse(int Event, int x, int y, int flags, void* param);
int DigitalTrain(char *single_picname, MARECT *DRArea, int *lAreaNum);
//int DigitalRecog(char *single_picname, MARECT *DRArea, int lAreaNum, YoloNet_detect yolo, float thresh);
/***********************/
int DigitalRecog(char *single_picname, MARECT *DRArea, int lAreaNum, void *hMRHandle, float thresh);
typedef struct
{
	network net;
	float lThresh;
}DRParam;
int HYDSR_Init(void *hMemMgr, void **pMRHandle)
{
	int res = 0;
	DRParam *pSRHandle = NULL;

	pSRHandle = (DRParam *)malloc(1 * sizeof(DRParam));
	if (pSRHandle == NULL)
	{
		res = -1;
		goto EXT;
	}

EXT:
	*pMRHandle = pSRHandle;
	return res;
}
int HYDSR_SetParam(void *hMRHandle, char *cfgfile, char *weightfile, float Thresh, int w, int h)
{
	int res = 0;
	DRParam *pSRParam = (DRParam*)hMRHandle;

	pSRParam->lThresh = Thresh;
	pSRParam->net = COM_parse_network_cfg(cfgfile);   //cfg中的width和height尺寸
	if (pSRParam->net.errorcode != 0)
	{
		res = pSRParam->net.errorcode;
		return res;
	}
	if (weightfile) {
		COM_load_weights(&(pSRParam->net), weightfile);
		if (pSRParam->net.errorcode != 0)
		{
			res = pSRParam->net.errorcode;
			return res;
		}
	}
	COM_set_batch_network(&(pSRParam->net), 1);
	return res;

}
int HYDSR_Uninit(void *hMRHandle)
{
	DRParam *pSRParam = (DRParam*)hMRHandle;
	COM_free_network(pSRParam->net);

	if (pSRParam)
		free(pSRParam);
	pSRParam = NULL;
	return 0;
}
/***********************/
int main()
{
	/***********************/
	//YoloNet_detect yolo;
	void *hTLHandle = NULL;
	/***********************/
	char single_picname[256];
	char *cfgfile = "../model/digit/number.cfg";
	float thresh = 0.24;
	char *weightfile = "../model/digit/number.weights";
	/***********************/
	//yolo.initNetData(cfgfile, weightfile);
	if (0 != HYDSR_Init(NULL, &hTLHandle))
	{
		printf("HYDSR_Init error.\n");
		return -1;
	}
	int w = 0;
	int h = 0;
	if (0 != HYDSR_SetParam(hTLHandle, cfgfile, weightfile, thresh, w, h))
	{
		printf("HYDSR_SetParam error.\n");
		HYDSR_Uninit(hTLHandle);
		return -1;
	}
	/***********************/
	while (1)
	{
		MARECT DRArea[100] = { 0 };
		int Areanum = 0;
		
		printf("请输入测试单张照片路径：（例如输入:E:\\nest_photos\\1.jpg)(注：路径中不能包含空格)(回车确定)\n");
		scanf("%s", &single_picname);
		if (atoi(single_picname) == 1)break;
		DigitalTrain(single_picname, DRArea, &Areanum);
		/***********************/
		//DigitalRecog(single_picname, DRArea, Areanum, yolo, thresh);
		DigitalRecog(single_picname, DRArea, Areanum, hTLHandle, thresh);
		/***********************/
	}
	
	/***********************/
	//yolo.ReleaseNetwork();
	HYDSR_Uninit(hTLHandle);
	/***********************/
}

int DigitalTrain(char *single_picname, MARECT *DRArea, int *lAreaNum)
{
	CvSize ImgSize;
	CvPoint startPt = { 0 };
	CvPoint endPt = { 0 };
	int mouseParam[3];
	int flagL = 0,i=0;
	IplImage *src = cvLoadImage(single_picname);
	if (!src)
	{
		printf("Error when loading image.\n"), exit(1);
	}
	ImgSize.height = src->height;
	ImgSize.width = src->width;
	IplImage *tmpImage = cvCreateImage(ImgSize, src->depth, src->nChannels);
	cvResize(src, tmpImage, CV_INTER_LINEAR);
	cvSaveImage("D:\\testImage.bmp", tmpImage);
	cvNamedWindow("TrainImage2", 1);
	cvSetMouseCallback("TrainImage2", onMouse, (void*)mouseParam);
	printf("请圈定匹配目标对象：\n");
	while (1)
	{
		//左键进行标记 
		if (mouseParam[2] == CV_EVENT_LBUTTONDOWN)
		{
			startPt.x = mouseParam[0];
			startPt.y = mouseParam[1];
			endPt.x = mouseParam[0];
			endPt.y = mouseParam[1];

			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp", CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0, 0, 255), 1);
			flagL = 1;
			cvShowImage("TrainImage2", tmpImage);

			DRArea[i].left = startPt.x;
			DRArea[i].top = startPt.y;
		}
		if (mouseParam[2] == CV_EVENT_MOUSEMOVE && flagL == 1)
		{
			endPt.x = mouseParam[0];
			endPt.y = mouseParam[1];

			cvReleaseImage(&tmpImage);
			tmpImage = cvLoadImage("D:\\testImage.bmp", CV_LOAD_IMAGE_COLOR);
			cvRectangle(tmpImage, startPt, endPt, CV_RGB(0, 0, 255), 1);
			cvShowImage("TrainImage2", tmpImage);

			DRArea[i].right = endPt.x;
			DRArea[i].bottom = endPt.y;
		}
		if (1 == flagEndL)   //左键标记一个进行重置
		{
			i++;
			(*lAreaNum)++;
			flagEndL = 0;
			flagL = 0;
			printf("已画%d个框\n",i);
		}

		if (13 == cvWaitKey(10))  //enter 退出
		{
			break;
		}
		cvShowImage("TrainImage2", tmpImage);
	}
	printf("交互完成\n");
	//cvWaitKey(0);
	cvDestroyWindow("TrainImage2");
	cvReleaseImage(&src);
	cvReleaseImage(&tmpImage);
}
int comp(const void *pa, const void *pb)
{
	sortable_DigitForm a = *(sortable_DigitForm *)pa;
	sortable_DigitForm b = *(sortable_DigitForm *)pb;
	float diff = a.CPoint.x - b.CPoint.x;
	if (diff < 0) return -1;
	else if (diff > 0) return 1;
	return 0;
}

int boxcomp(const void *pa, const void *pb)
{
	float* a = *(float* *)pa;
	float* b = *(float* *)pb;
	float diff = a[4] - b[4];
	if (diff < 0) return 1;
	else if (diff > 0) return -1;
	return 0;
}
void GetOverLappingRate(MARECT rt1, MARECT rt2, long *minRate, long *maxRate)
{
	long lSizeRt1, lSizeRt2;
	long lMaxYDist = MAX(rt2.bottom, rt1.bottom) - MIN(rt2.top, rt1.top);
	long lMaxXDist = MAX(rt2.right, rt1.right) - MIN(rt2.left, rt1.left);
	if (!minRate && !maxRate) return;

	lSizeRt1 = (rt1.bottom - rt1.top)*(rt1.right - rt1.left);
	lSizeRt2 = (rt2.bottom - rt2.top)*(rt2.right - rt2.left);

	if (lSizeRt1 <= 0 || lSizeRt2 <= 0)
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 100;
	}

	///如果两个矩形有交集，则距离为0
	if (lMaxXDist <= (rt2.right - rt2.left + rt1.right - rt1.left)
		&& lMaxYDist <= (rt2.bottom - rt2.top + rt1.bottom - rt1.top))
	{

		long lSizeOverlap = (MIN(rt2.right, rt1.right) - MAX(rt2.left, rt1.left))
			*(MIN(rt2.bottom, rt1.bottom) - MAX(rt2.top, rt1.top));
		if (minRate) *minRate = MIN(lSizeOverlap * 100 / lSizeRt1, lSizeOverlap * 100 / lSizeRt2);
		if (maxRate) *maxRate = MAX(lSizeOverlap * 100 / lSizeRt1, lSizeOverlap * 100 / lSizeRt2);
	}
	else
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 0;
	}
	return;
}
int DigitalRecog(char *single_picname, MARECT *DRArea, int lAreaNum, void *hMRHandle, float thresh)
{
	IplImage *src = cvLoadImage(single_picname);
	if (!src)
	{
		printf("Error when loading image.\n"), exit(1);
	}
	IplImage *dst = cvLoadImage(single_picname);
	if (!dst)
	{
		printf("Error when loading image.\n"), exit(1);
	}
	for (int i = 0; i < lAreaNum; i++)
	{
		MARECT rtArea;
		rtArea.left = MIN(DRArea[i].left, DRArea[i].right);
		rtArea.right = MAX(DRArea[i].left, DRArea[i].right);
		rtArea.top = MIN(DRArea[i].top, DRArea[i].bottom);
		rtArea.bottom = MAX(DRArea[i].top, DRArea[i].bottom);
		if (rtArea.bottom >= src->height)rtArea.bottom = src->height - 1;
		if (rtArea.right >= src->width)rtArea.right = src->width - 1;
		if (rtArea.left < 0)rtArea.left = 0;
		if (rtArea.top < 0)rtArea.top = 0;
		IplImage *RectImg = cvCreateImage(cvSize(rtArea.right - rtArea.left, rtArea.bottom - rtArea.top), src->depth, src->nChannels);
		cvSetImageROI(src, cvRect(rtArea.left, rtArea.top, rtArea.right - rtArea.left, rtArea.bottom - rtArea.top));
		cvCopy(src, RectImg);
		cvResetImageROI(src);
		if (!RectImg )
		{
			printf("Error when cut image.\n");
			i = i + 1;
			cvReleaseImage(&RectImg);
			continue;
		}
		int bnum = 0;

		int boxnum = 100;//候选框数最大值
		float **bboxout;
		bboxout = (float **)calloc(boxnum, sizeof(float *));
		for (int i = 0; i < boxnum; i++)
			bboxout[i] = (float*)calloc(6, sizeof(float));
		/*********************检测数字*************************/
		/***********************/
		//yolo.object_detection((unsigned char *)RectImg->imageData, RectImg->height, RectImg->width, RectImg->nChannels, (int)RectImg->widthStep, thresh, &bnum, bboxout);//检测
		DRParam *pSRParam = (DRParam*)hMRHandle;
		int width = RectImg->width;
		int height = RectImg->height;
		int channels = RectImg->nChannels;
		int widstep = RectImg->widthStep;
		unsigned char* imagdata = (unsigned char *)RectImg->imageData;
		image out = COM_make_image(width, height, channels);
		int p, j, k, count = 0;
		if (out.data == NULL)
		{
			return -1;
		}
		for (k = 0; k < channels; ++k) {
			for (p = 0; p <height; ++p) {
				for (j = 0; j < width; ++j) {
					out.data[count++] = imagdata[p*widstep + j*channels + k] / 255.;
				}
			}
		}
		COM_YoloDetector(pSRParam->net, out, thresh, &bnum, bboxout);
		COM_free_image(out);
		/***********************/
		qsort(bboxout, bnum, sizeof(float*), boxcomp);
		bnum = (bnum>6) ? 6 : bnum;
		if (bnum < 2){
			printf("error:digit num=%d\n", bnum);
			for (int i = 0; i < boxnum; ++i)
			{
				if (bboxout[i])
				{
					free(bboxout[i]);
					bboxout[i] = NULL;
				}
			}
			if (bboxout)
			{
				free(bboxout);
				bboxout = NULL;
			}
			cvReleaseImage(&RectImg);
			continue;
		}
		sortable_DigitForm *s = (sortable_DigitForm*)calloc(bnum, sizeof(sortable_DigitForm));
		MARECT rtArea1, rtArea2;
		for (int i = 0; i < bnum; ++i){
			//排除异常高度,重叠区域
			float a;
			long lMaxRate;
			int step = src->widthStep / sizeof(unsigned char);
			int channels = 3;
			int x, y;
			int whitepixelnum = 0, redpixelnum = 0, pixelnum = 0;
			if (s[i].flag < 0)continue;
			s[i].DigitForm = bboxout[i];
			s[i].CPoint.x = (s[i].DigitForm[0] + s[i].DigitForm[1]) / 2;
			s[i].CPoint.y = (s[i].DigitForm[2] + s[i].DigitForm[3]) / 2;
			rtArea1.left = s[i].DigitForm[0];
			rtArea1.right = s[i].DigitForm[1];
			rtArea1.top = s[i].DigitForm[2];
			rtArea1.bottom = s[i].DigitForm[3];
			for (int j = 0; j<bnum; j++){
				if (i == j)continue;
				if (s[j].flag < 0)continue;
				s[j].DigitForm = bboxout[j];
				rtArea2.left = s[j].DigitForm[0];
				rtArea2.right = s[j].DigitForm[1];
				rtArea2.top = s[j].DigitForm[2];
				rtArea2.bottom = s[j].DigitForm[3];
				lMaxRate = 0;
				GetOverLappingRate(rtArea1, rtArea2, NULL, &lMaxRate);
				//if(lMaxRate > 0)printf("lMaxRate=%d\n",lMaxRate);
				if (lMaxRate > 25)
				{
					if (bboxout[j][4] < bboxout[i][4])
						s[j].flag = -2;
					else
						s[i].flag = -2;
				}
			}
		}
		qsort(s, bnum, sizeof(sortable_DigitForm), comp);
		MARECT resultArea = { 0 };
		int value=0;
		int flag = 0;
		for (int i = 0; i < bnum; i++)
		{
			if (s[i].flag<0)continue;
			if (flag == 0)
			{
				resultArea.left = s[i].DigitForm[0];
				resultArea.right = s[i].DigitForm[1];
				resultArea.top = s[i].DigitForm[2];
				resultArea.bottom = s[i].DigitForm[3];
				value = s[i].DigitForm[5];
				flag = 1;
			}
			else
			{
				resultArea.left = MIN(resultArea.left,s[i].DigitForm[0]);
				resultArea.right = MAX(resultArea.right,s[i].DigitForm[1]);
				resultArea.top = MIN(resultArea.top,s[i].DigitForm[2]);
				resultArea.bottom = MAX(resultArea.bottom,s[i].DigitForm[3]);
				value = value*10+s[i].DigitForm[5];
			}
		}
		/***************************************************/
		CvPoint ptStart, ptStop, ptText;//画框，输出结果图
		char text[100] = { 0 };
		CvFont font;
		ptStart.x = resultArea.left + rtArea.left;
		ptStart.y = resultArea.top + rtArea.top;
		ptStop.x = resultArea.right + rtArea.left;
		ptStop.y = resultArea.bottom + rtArea.top;
		ptText.x = ptStart.x;
		ptText.y = ptStop.y;
		if (ptText.y < src->height - 25)
			ptText.y += 25;
		cvRectangle(dst, ptStart, ptStop, cvScalar(0, 0, 255), 2);
		sprintf(text, "val=%d", value);
		printf("val=%d\n", value);
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8f, 1.0f);
		cvPutText(dst, text, ptText, &font, cvScalar(0, 255, 0));

		for (int i = 0; i < boxnum; ++i)
		{
			if (bboxout[i])
			{
				free(bboxout[i]);
				bboxout[i] = NULL;
			}
		}
		if (bboxout)
		{
			free(bboxout);
			bboxout = NULL;
		}
		if (s)
		{
			free(s);
			s = NULL;
		}
		cvReleaseImage(&RectImg);
	}
	char *result_img = "..\\result\\result.jpg";
	cvSaveImage(result_img, dst);

	cvReleaseImage(&src);
	cvReleaseImage(&dst);
}

void onMouse(int Event, int x, int y, int flags, void* param)
{
	if (Event == CV_EVENT_LBUTTONDOWN)
	{
		int *Data = (int*)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = 1;
	}
	if (Event == CV_EVENT_RBUTTONDOWN)
	{
		int *Data = (int*)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_RBUTTONDOWN;
	}
	if (Event == CV_EVENT_MOUSEMOVE)
	{
		int *Data = (int*)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_MOUSEMOVE;
	}
	if (Event == CV_EVENT_LBUTTONUP)
	{
		int *Data = (int*)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_LBUTTONUP;
		flagEndL = 1;
	}
	if (Event == CV_EVENT_RBUTTONUP)
	{
		int *Data = (int*)param;
		Data[0] = x;
		Data[1] = y;
		Data[2] = CV_EVENT_RBUTTONUP;
		flagEndR = 1;
	}
}
