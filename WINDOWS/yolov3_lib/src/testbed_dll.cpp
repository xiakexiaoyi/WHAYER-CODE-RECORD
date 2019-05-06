#define GPU
//#define RESIZE
#include <windows.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "yolo_dll.h"
#include "HY_utils.h"
#include "testbed_dll.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
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
using namespace std;
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tmpres /= 10;  /*convert into microseconds*/
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
double what_time_is_it_now()
{
	struct timeval time;
	if (gettimeofday(&time, NULL)) {
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
int detect(char *input,char *cfgfile, char *weightfile)
{
	void *handle;
	IplImage *src = cvLoadImage(input);//BGR
	if (src == NULL)
		printf("load image failed\n");
	YOLOV3_IMAGES img;
	
	img.lHeight = src->height;
	img.lWidth = src->width;
	img.pixelArray.chunky.lLineBytes = src->widthStep;
	img.pixelArray.chunky.pPixel = src->imageData;
	img.lPixelArrayFormat = 3;
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	static int count= 0;

#ifdef GPU	
	init(&handle, cfgfile, weightfile, 0.24, 0);
#else
	init_no_gpu(&handle, cfgfile, weightfile, 0.24);
#endif

	{
#ifdef GPU	
		detec(handle, &img, &resultlist);
#else
		detec_no_gpu(handle, &img, &resultlist);
#endif
		printf("count_in = %d\n", count++);
	}
#ifdef GPU	
	uninit(handle);
#else
	uninit_no_gpu(handle);
#endif
	for (int i = 0; i<resultlist.lResultNum; i++)
	{
		int left = resultlist.pResult[i].Target.left;
		int top = resultlist.pResult[i].Target.top;
		int right = resultlist.pResult[i].Target.right;
		int bottom = resultlist.pResult[i].Target.bottom;
		cvRectangle(src, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255));
	}
	//cvSaveImage("D:/work/model/test/testimg/result.jpg", src);
	if (cfgfile == "D:/work/model/test/新建文件夹/yolov3-tiny-voc.cfg")
		cvSaveImage("D:/work/model/test/testimg/result1.jpg", src);
		//printf("1\n");
	else
		cvSaveImage("D:/work/model/test/testimg/result2.jpg", src);
		//printf("2\n");
	cvShowImage("Result Show", src);
	cvWaitKey(10);
	cvReleaseImage(&src);
	return 0;
}

int main_boost()//boost
{
	char *input = "D:/work/model/test/testimg/2018_0423_sf6_108.jpg";
	char *cfgfile = "D:/work/model/test/新建文件夹/yolov3-tiny-voc.cfg";
	char *cfgfile1 = "D:/work/model/test/yolov3-tiny-voc.cfg";
	char *weightfile = "D:/work/model/test/yolov3-tiny-voc_18700.weights";
	int count = 0;
	while (1)
	{
		boost::thread t1(boost::bind(detect, input, cfgfile, weightfile));
		boost::thread t2(boost::bind(detect, input, cfgfile1, weightfile));
		t1.join();
		t2.join();
		printf("count=%d\n", ++count);
	}
	return 0;
}
int main_multest()//Multiple yolov3 test
{
	void *handleout;
	void *handlein;
	char *cfgfile;
	char *weightfile;
	float threshout = 0.24;  //0.62
	float threshin = 0.24;  //0.76
	int gpu_index_out = 0;
	int gpu_index_in = 0;
	
	
	cfgfile = "D:/work/model/test/yolov3-tiny-voc.cfg";
	weightfile = "D:/work/model/test/yolov3-tiny-voc_18700.weights";
	init(&handleout, cfgfile, weightfile, threshout, gpu_index_out);
	cfgfile = "D:/work/model/test/yolov3-tiny-voc.cfg";
	weightfile = "D:/work/model/test/yolov3-tiny-voc_18700.weights";
	init(&handlein, cfgfile, weightfile, threshin, gpu_index_in);
	YOLOV3_IMAGES img;
	IplImage *src = cvLoadImage("D:/work/model/test/testimg/2018_0424_bileiqi_00062.jpg");
	
	img.lHeight = src->height;
	img.lWidth = src->width;
	img.pixelArray.chunky.lLineBytes = src->widthStep;
	img.pixelArray.chunky.pPixel = src->imageData;
	img.lPixelArrayFormat = 3;

	int count = 0;
	while(1)
	{ 
		printf("count=%d\n",count++);
		IplImage *srctemp1 = cvCloneImage(src);
		IplImage *srctemp2 = cvCloneImage(src);
		HYYOLOV3RESULT_LIST  resultlist1 = { 0 };
		resultlist1.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
		HYYOLOV3RESULT_LIST  resultlist2 = { 0 };
		resultlist2.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
		detec(handleout, &img, &resultlist1);
		detec(handlein, &img, &resultlist2);
		HYYOLOV3RESULT_LIST  &resultlist = resultlist1;
		for (int i = 0; i<resultlist.lResultNum; i++)
		{
			CvPoint ptText;
			char text[256] = { 0 };
			CvFont font;
			int left = resultlist.pResult[i].Target.left;
			int top = resultlist.pResult[i].Target.top;
			int right = resultlist.pResult[i].Target.right;
			int bottom = resultlist.pResult[i].Target.bottom;
			ptText.x = resultlist.pResult[i].Target.left;
			ptText.y = resultlist.pResult[i].Target.bottom;

			if (ptText.y<srctemp1->height - 20)
				ptText.y += 20;
			cvRectangle(srctemp1, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255), 10);
			sprintf(text, "state=%d", (int)resultlist.pResult[i].dVal);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 3.0f, 3.0f, 0, 10);
			cvPutText(srctemp1, text, ptText, &font, cvScalar(0, 255, 0));
		}
		cvSaveImage("..\\..\\result\\resultlist1.jpg", srctemp1);
		{
			HYYOLOV3RESULT_LIST  &resultlist = resultlist2;
			for (int i = 0; i<resultlist.lResultNum; i++)
			{
				CvPoint ptText;
				char text[256] = { 0 };
				CvFont font;
				int left = resultlist.pResult[i].Target.left;
				int top = resultlist.pResult[i].Target.top;
				int right = resultlist.pResult[i].Target.right;
				int bottom = resultlist.pResult[i].Target.bottom;
				ptText.x = resultlist.pResult[i].Target.left;
				ptText.y = resultlist.pResult[i].Target.bottom;

				if (ptText.y<srctemp2->height - 20)
					ptText.y += 20;
				cvRectangle(srctemp2, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255), 10);
				sprintf(text, "state=%d", (int)resultlist.pResult[i].dVal);
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 3.0f, 3.0f, 0, 10);
				cvPutText(srctemp2, text, ptText, &font, cvScalar(0, 255, 0));
			}

			cvSaveImage("..\\..\\result\\resultlist2.jpg", srctemp2);
		}
		cvReleaseImage(&srctemp1);
		cvReleaseImage(&srctemp2);
		free(resultlist1.pResult);
		free(resultlist2.pResult);
	}
	uninit(handleout);
	uninit(handlein);

}
int main_xiaojinju(int argc, char *argv[])
{
	time_t t11 = 0, t12 = 0;

	float nmsthreshold = 0.3;//argv[4]
	float score = 0.76;//argv[5]
	char savePath[256] = "D:\\result0820.csv";//argv[3]
	char imagelist[256] = "D:/list.txt";//argv[1]  "F:\Work_Whayer\Algorithm\HYInsu12\testwu.txt"
	char modelType[256] = "M3";//argv[2]

	int detectM[10] = { 0 };

	for (int i = 1; i < 10; i++)
	{
		char Mchar[100];
		sprintf(Mchar, "M%d", i);
		char *p = strstr(modelType, Mchar);
		if (NULL != p)
		{
			detectM[i] = 1;
		}
	}

	//---------文件List 预处理----------------------
	char fname[_MAX_FNAME];//照片名称
	char ext[_MAX_EXT];//后缀名
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号

	FILE* fpcsv;
	fpcsv = fopen(savePath, "w");
	fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	fclose(fpcsv);

	std::vector<string> strArray;//照片路径集合
	string str;
	strArray.clear();


	ifstream  readFile(imagelist);
	if (!readFile)
	{
		std::cout << "can not open list file!" << std::endl;
		return -1;
	}
	else
	{
		while (!readFile.eof())
		{
			getline(readFile, str);
			if (str == "" || (str[0] == '/'))
			{
				continue;
			}
			++lines;
			strArray.push_back(str);
		}
	}

	readFile.close();

	void *handle;
	char *cfgfile;
	char *weightfile;
	float thresh = 0.24;  //0.62
	int gpu_index = 0;
	//---------初始化网络模型----------------------
	t11 = clock();
	cfgfile = "D:/work/model/xiaojinju/yolov3-voc.cfg";
	weightfile = "D:/work/model/xiaojinju/yolov3-voc_final.weights";
	init(&handle, cfgfile, weightfile, thresh, gpu_index);

	if (handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");
	t12 = clock();
	//printf("load model time(ms): %d\n", ((t12 - t11) * 1000 / CLOCKS_PER_SEC));

	//---------图片列表检测----------------------

	//long long LOOP = 0;
	//while (1)
	//{
	time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);
	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		//detectM[i] ?=1 to detect algorithm
		imgnum++;

		_splitpath(strArray[i].c_str(), NULL, NULL, fname, ext);

		printf("%d/%d %s%s\n", imgnum, lines, fname, ext);//显示处理进度
		t1 = clock();
		Detection_xiaojinju(handle, strArray[i].c_str(), savePath, fname, ext); //【3】：调用检测函数
		t2 = clock();
		if (detectM[5] == 1)
		{
			//
		}
		
		printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}
	//	LOOP++;
	//}


	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);


	printf("---process complete---\n");
	system("pause");
	return 0;
}
int main_dajinju(int argc, char *argv[])
{
	time_t t11 = 0, t12 = 0;

	float nmsthreshold = 0.3;//argv[4]
	float score = 0.76;//argv[5]
	char savePath[256] = "D:\\result0820.csv";//argv[3]
	char imagelist[256] = "D:/listceshi.txt";//argv[1]  "F:\Work_Whayer\Algorithm\HYInsu12\testwu.txt"
	char modelType[256] = "M3";//argv[2]

	int detectM[10] = { 0 };

	for (int i = 1; i < 10; i++)
	{
		char Mchar[100];
		sprintf(Mchar, "M%d", i);
		char *p = strstr(modelType, Mchar);
		if (NULL != p)
		{
			detectM[i] = 1;
		}
	}

	//---------文件List 预处理----------------------
	char fname[_MAX_FNAME];//照片名称
	char ext[_MAX_EXT];//后缀名
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号

	FILE* fpcsv;
	fpcsv = fopen(savePath, "w");
	fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	fclose(fpcsv);

	std::vector<string> strArray;//照片路径集合
	string str;
	strArray.clear();


	ifstream  readFile(imagelist);
	if (!readFile)
	{
		std::cout << "can not open list file!" << std::endl;
		return -1;
	}
	else
	{
		while (!readFile.eof())
		{
			getline(readFile, str);
			if (str == "" || (str[0] == '/'))
			{
				continue;
			}
			++lines;
			strArray.push_back(str);
		}
	}

	readFile.close();

	void *handle;
	char *cfgfile;
	char *weightfile;
	float thresh = 0.24;  //0.62
	int gpu_index = 0;
	//---------初始化网络模型----------------------
	t11 = clock();
	cfgfile = "D:/work/model/dajinju/yolov3-voc.cfg";
	weightfile = "D:/work/model/dajinju/yolov3-voc_20000.weights";
	init(&handle, cfgfile, weightfile, thresh, gpu_index);

	if (handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");
	t12 = clock();
	//printf("load model time(ms): %d\n", ((t12 - t11) * 1000 / CLOCKS_PER_SEC));

	//---------图片列表检测----------------------

	//long long LOOP = 0;
	//while (1)
	//{
	time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);
	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		//detectM[i] ?=1 to detect algorithm
		imgnum++;

		_splitpath(strArray[i].c_str(), NULL, NULL, fname, ext);

		printf("%d/%d %s%s\n", imgnum, lines, fname, ext);//显示处理进度
		t1 = clock();
		Detection_dajinju(handle, strArray[i].c_str(), savePath, fname, ext); //【3】：调用检测函数
		if (detectM[5] == 1)
		{
			//
		}
		t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}
	//	LOOP++;
	//}


	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);


	printf("---process complete---\n");
	system("pause");
	return 0;
}
int main_standard(int argc, char *argv[])//jueyuanzi
{

	//---------局部变量----------------------
	time_t t11 = 0, t12 = 0;

	float nmsthreshold = 0.3;//argv[4]
	float score = 0.76;//argv[5]
	char savePath[256] = "D:\\result0820.csv";//argv[3]
	char imagelist[256] = "D:/listceshi.txt";//argv[1]  "F:\Work_Whayer\Algorithm\HYInsu12\testwu.txt"
	char modelType[256] = "M3";//argv[2]

	int detectM[10] = { 0 };
	char* modelpath1 = "./jueyuanzi1.bin"; //模型1路径
	char* modelpath2 = "./jueyuanzi_xiao.bin"; //模型2路径


											   //float nmsthreshold = find_float_arg(argc, argv, "--nms", 0);
											   //float score = find_float_arg(argc, argv, "--score", 0);
											   //char *savePath = find_char_arg(argc, argv, "--savepath", 0);
											   //char *imagelist = find_char_arg(argc, argv, "--imagelist", 0);
											   //char *modelType = find_char_arg(argc, argv, "--modelTypes", 0);


	for (int i = 1; i < 10; i++)
	{
		char Mchar[100];
		sprintf(Mchar, "M%d", i);
		char *p = strstr(modelType, Mchar);
		if (NULL != p)
		{
			detectM[i] = 1;
		}
	}

	//---------文件List 预处理----------------------
	char fname[_MAX_FNAME];//照片名称
	char ext[_MAX_EXT];//后缀名
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号

	FILE* fpcsv;
	fpcsv = fopen(savePath, "w");
	fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	fclose(fpcsv);

	std::vector<string> strArray;//照片路径集合
	string str;
	strArray.clear();


	ifstream  readFile(imagelist);
	if (!readFile)
	{
		std::cout << "can not open list file!" << std::endl;
		return -1;
	}
	else
	{
		while (!readFile.eof())
		{
			getline(readFile, str);
			if (str == "" || (str[0] == '/'))
			{
				continue;
			}
			++lines;
			strArray.push_back(str);
		}
	}

	readFile.close();

	//---------创建检测对象----------------------
	

	void *handleout;
	void *handlein;
	char *cfgfile;
	char *weightfile;
	float threshout = 0.24;  //0.62
	float threshin = 0.24;  //0.76
	int gpu_index_out = 0;
	int gpu_index_in = 0;
								//---------初始化网络模型----------------------
	t11 = clock();
	cfgfile = "D:/work/model/yolov3-voc.cfg";
	weightfile = "D:/work/model/yolov3-voc_final.weights";
	init(&handleout, cfgfile, weightfile, threshout, gpu_index_out);
	cfgfile = "D:/work/model/yolov3-voc.cfg";
	weightfile = "D:/work/model/yolov3-voc_final.weights";
	init(&handlein, cfgfile, weightfile, threshin, gpu_index_in);

	if ((handleout == 0) || (handlein == 0))
	{
		if (handleout == 0 && handlein == 0)
		{
			std::cout << "model_1 and model_2 loading failed ... " << std::endl;
			return -1;
		}
		else if (handleout == 0)
		{
			std::cout << "model_1 loading failed ... " << std::endl;
			return -1;
		}
		else
		{
			std::cout << "model_2 loading failed ... " << std::endl;
			return -1;
		}

	}

	printf("Loading Finished\n");
	t12 = clock();
	printf("load model time(ms): %d\n", ((t12 - t11) * 1000 / CLOCKS_PER_SEC));

	//---------图片列表检测----------------------

	long long LOOP = 0;
	//while (1)
	//{
	time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);
	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		//detectM[i] ?=1 to detect algorithm
		imgnum++;

		_splitpath(strArray[i].c_str(), NULL, NULL, fname, ext);

		printf("%d/%d %s%s\n", imgnum, lines, fname, ext);//显示处理进度
		t1 = clock();
		AllDetection(handleout, handlein, strArray[i].c_str(), savePath, fname, ext); //【3】：调用检测函数
		if (detectM[5] == 1)
		{
			//
		}
		t2 = clock();
		printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}
	//	LOOP++;
	//}


	//---------释放网络及资源----------------------
	
	std::vector<string>().swap(strArray);

	
	printf("---process complete---\n");
	system("pause");
	return 0;

}
int main_onesteptest()
{
	
	double time;
	double totaltime = 0.0;
	void *handle;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char filePath[MAX_PATH] = "..\\..\\test image\\";
	char fileresultPath[MAX_PATH] = "..\\..\\result\\";
	//char filePath[MAX_PATH] = "..\\testimage\\";
	//char fileresultPath[MAX_PATH] = "..\\result\\";
	char filePath_const[MAX_PATH];
	char fileName1[MAX_PATH];
	char fileName2[MAX_PATH];
	char *cfgfile = "D:/work/model/yaopian/yolov3-tiny-voc.cfg";
	char *weightfile = "D:/work/model/yaopian/yolov3-tiny-voc_12000.weights";
	//char *cfgfile = "../model/yolov3-voc.cfg";
	//char *weightfile = "../model/yolov3-voc_final.weights";
	strcpy(filePath_const, filePath);
	strcat(filePath, "\*.bmp");
	hFind = FindFirstFile(filePath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return -2;
	}
	int count = 0;
	int imagenum = 0;
	init(&handle, cfgfile, weightfile, 0.24, 0);
	do{
		memset(fileName1, 0, MAX_PATH);
		strcpy(fileName1, filePath_const);
		strcat(fileName1, FindFileData.cFileName);
		memset(fileName2, 0, MAX_PATH);
		strcpy(fileName2, fileresultPath);
		strcat(fileName2, FindFileData.cFileName);
		printf("%d:fileName=%s\n", count++,fileName1);
		imagenum++;
		IplImage *src = cvLoadImage(fileName1);//BGR
		if (src == 0)
		{
			imagenum--;
			char shrinked_filename[1024];
			if (strlen(fileName1) >= 1024) sprintf(shrinked_filename, "name is too long");
			else sprintf(shrinked_filename, "%s", fileName1);
			fprintf(stderr, "Cannot load image \"%s\"\n", shrinked_filename);
			FILE* fw = fopen("..\\..\\bad.list", "a");
			//FILE* fw = fopen("..\\bad.list", "a");
			fwrite(shrinked_filename, sizeof(char), strlen(shrinked_filename), fw);
			char *new_line = "\n";
			fwrite(new_line, sizeof(char), strlen(new_line), fw);
			fclose(fw);

			cvReleaseImage(&src);
			continue;
		}
#ifdef RESIZE
		IplImage *srcResize;
		int minlength = cv::min(src->width,src->height);
		int w_resize = 0;
		int h_resize = 0;
		if(minlength > 720)
		{
			if (minlength == src->width)
			{ 
				w_resize = 720;
				h_resize = w_resize*src->height / src->width;
			}
			else
			{
				h_resize = 720;
				w_resize = h_resize*src->width / src->height;
			}
			srcResize = cvCreateImage(cvSize(w_resize, h_resize), src->depth, src->nChannels);
			cvResize(src, srcResize);
		}
		else
		{
			w_resize = src->width;
			h_resize = src->height;
			srcResize = cvCreateImage(cvSize(w_resize, h_resize), src->depth, src->nChannels);
			cvCopy(src, srcResize);
		}
#endif		
		YOLOV3_IMAGES img;
#ifndef RESIZE
		img.lHeight = src->height;
		img.lWidth = src->width;
		img.pixelArray.chunky.lLineBytes = src->widthStep;
		img.pixelArray.chunky.pPixel = src->imageData;
		img.lPixelArrayFormat = 3;
#else
		img.lHeight = srcResize->height;
		img.lWidth = srcResize->width;
		img.pixelArray.chunky.lLineBytes = srcResize->widthStep;
		img.pixelArray.chunky.pPixel = srcResize->imageData;
		img.lPixelArrayFormat = 3;
#endif	
		HYYOLOV3RESULT_LIST  resultlist = { 0 };
		resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
		time = what_time_is_it_now();
		detec(handle, &img, &resultlist);
		printf(" detec Predicted in %f seconds.\n", (what_time_is_it_now() - time));
		totaltime += (what_time_is_it_now() - time);
		for (int i = 0; i<resultlist.lResultNum; i++)
		{
			CvPoint ptText;
			char text[256] = { 0 };
			CvFont font;
			int left = resultlist.pResult[i].Target.left;
			int top = resultlist.pResult[i].Target.top;
			int right = resultlist.pResult[i].Target.right;
			int bottom = resultlist.pResult[i].Target.bottom;
			ptText.x = resultlist.pResult[i].Target.left;
			ptText.y = resultlist.pResult[i].Target.bottom;
			
#ifdef RESIZE
			if (ptText.y<srcResize->height - 10)
				ptText.y += 10;
			cvRectangle(srcResize, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255));
			sprintf(text, "state=%d", (int)resultlist.pResult[i].dVal);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.2f, 0.5f);
			cvPutText(srcResize, text, ptText, &font, cvScalar(0, 255, 0));
#else
			if (ptText.y<src->height - 20)
				ptText.y += 20;
			cvRectangle(src, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255),10);
			sprintf(text, "state=%d", (int)resultlist.pResult[i].dVal);
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 3.0f, 3.0f,0,10);
			cvPutText(src, text, ptText, &font, cvScalar(0, 255, 0));
#endif	
		}
#ifdef RESIZE
		cvSaveImage(fileName2, srcResize);
#else
		cvSaveImage(fileName2, src);
#endif	
		free(resultlist.pResult);
		cvReleaseImage(&src);
#ifdef RESIZE
		cvReleaseImage(&srcResize);
#endif	
	} while (FindNextFile(hFind, &FindFileData));
	printf("--- average time is %f seconds.---\n", 1.0*totaltime/ imagenum);
	uninit(handle);
	FindClose(hFind);
	system("pause");
	return 0;
}

int main_origin()
{
	void *handle;
	char *input = "D:/work/model/test/testimg/2018_0423_sf6_108.jpg";
	/**/
	char *cfgfile = "D:/work/model/test/新建文件夹/yolov3-tiny-voc.cfg";
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
	
#ifdef GPU	
	init(&handle, cfgfile, weightfile, 0.24,0);
#else
	init_no_gpu(&handle, cfgfile, weightfile, 0.24);
#endif
	
	{ 
#ifdef GPU	
		detec(handle, &img,&resultlist);
#else
		detec_no_gpu(handle, &img, &resultlist);
#endif
		printf("count = %d\n",count++);
	}
#ifdef GPU	
	uninit(handle);
#else
	uninit_no_gpu(handle);
#endif
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
using namespace cv;
Mat resize_image(Mat im, int w, int h);
Mat letterbox_image(Mat im, int w, int h);

bool check(int m)
{
	int onestart = 0;
	int oneend = 0;
	while ((m >> 1) > 0) {
		if ((m & 1 == 1) && (m >> 1 & 1 == 1))
		{
			if (onestart == 0)
				onestart = 1;

		}
		else if (onestart == 1 && oneend == 0)
		{
			oneend = 1;
		}
		else if (oneend == 1 && (m >> 1 & 1 == 1))
		{
			return false;
		}
		m = m >> 1;
	}
	return true;
}
int distance(int x, int n)
{
	int yushu = x % 10;
	int shang = x / 10;
	int shiweishucishu = shang;
	if (shang > 4)
		shiweishucishu += 10;
	int geweishucishu = 0;
	if (yushu > 4)
		geweishucishu = 1;
	int juli = (x - geweishucishu - shiweishucishu)*n;
	return juli;
}
int weixin()
{
	vector<int> a = { 0,1,2,3,4,5,6,7,8,9 };
	vector<int> b(8, 1);
	int j = 0;
	srand((unsigned)time(NULL));
	for (int i = 0; i < 8; i++) {
		j = rand() % (10 - i);
		if (a[j] == 0 && i == 0)
		{
			i--;
			continue;
		}
		b[i] = a[j];
		a.erase(a.begin() + j);
	}
	int num = 0;
	for (int i = 0; i < b.size(); i++)
	{
		num = num * 10 + b[i];
		
	}
	return num;
}
int main()
{
	vector<int> a = { 0,1,2,3,4,5,6,7,8,9 };
	vector<int> b(8, 1);
	int j = 0;
	srand((unsigned)time(NULL));
	for (int i = 0; i < 8; i++) {
		j = rand() % (10 - i);
		if (a[j] == 0 && i==0)
		{
			i--;
			continue;
		}
		b[i] = a[j];
		a.erase(a.begin() + j);
	}
	for (int i=0; i < b.size(); i++)
	{
		cout << b[i];
	}
	printf("weixin()=%d\n", weixin());
	int m = 99;
	int x = 6;
	int n = 10;
	int yushu = x % 10;
	int shang = x / 10;
	printf("yushu=%d\n", yushu);
	printf("shang=%d\n", shang);
	int shiweishucishu = shang;
	if (shang > 4)
		shiweishucishu += 10;
	int geweishucishu = 0;
	if (yushu > 4)
		geweishucishu = 1;
	int juli = (x - geweishucishu - shiweishucishu)*n;
	printf("juli=%d\n", juli);
	int flag=check(m);
	if (flag == TRUE)
	{
		printf("no\n");
	}
	else
	{
		printf("2\n");
	}
	
	//main_dajinju(argc,argv);
	//main_xiaojinju(argc, argv);
	//main_onesteptest();

	/*Mat src=imread("D:/6.jpg");
	Mat dst=resize_image(src,416,416);
	imwrite("D:/6_resize.jpg",dst);
	Mat dst1= letterbox_image(src, 416, 416);
	imwrite("D:/6_letterbox.jpg", dst1);*/
	return 0;
}
//cols w;rows h
void embed_image_cv(Mat source, Mat dest, int dx, int dy)
{
	int x, y, k;
	for (k = 0; k < source.channels(); ++k) {
		for (y = 0; y < source.rows; ++y) {
			for (x = 0; x < source.cols; ++x) {
				float val = source.ptr<Vec3b>(y)[x][k];
				dest.ptr<Vec3b>(dy + y)[dx + x][k] = val;
				//float val = get_pixel(source, x, y, k);
				//set_pixel(dest, dx + x, dy + y, k, val);
			}
		}
	}
}
Mat letterbox_image(Mat im, int w, int h)
{
	int new_w = im.cols;
	int new_h = im.rows;
	if (((float)w / im.cols) < ((float)h / im.rows)) {
		new_w = w;
		new_h = (im.rows * w) / im.cols;
	}
	else {
		new_h = h;
		new_w = (im.cols * h) / im.rows;
	}
	//image resized = resize_image(im, new_w, new_h);
	//image boxed = make_image(w, h, im.c);
	Mat resized= resize_image(im, new_w, new_h);
	Mat boxed(h, w, CV_8UC3, Scalar(128, 128, 128));

	//fill_image(boxed, .5);

	//int i;
	//for(i = 0; i < boxed.w*boxed.h*boxed.c; ++i) boxed.data[i] = 0;
	embed_image_cv(resized, boxed, (w - new_w) / 2, (h - new_h) / 2);
	
	return boxed;
}
Mat resize_image(Mat im, int w, int h)
{
	//image resized = make_image(w, h, im.c);
	//image part = make_image(w, im.h, im.c);
	//Mat resized(w, h, CV_8UC3);
	//Mat part(w, im.rows, CV_8UC3);
	
	Mat resized(h, w, CV_8UC3);
	Mat part(im.rows, w, CV_8UC3);
	int r, c, k;
	float w_scale = (float)(im.cols - 1) / (w - 1);
	float h_scale = (float)(im.rows - 1) / (h - 1);
	for (k = 0; k < im.channels(); ++k) {
		for (r = 0; r < im.rows; ++r) {
			for (c = 0; c < w; ++c) {
				float val = 0;
				if (c == w - 1 || im.cols == 1) {
					val = im.ptr<Vec3b>(r)[im.cols - 1][k];
					//val = im.ptr<Vec3b>(im.cols - 1)[r][k];
					//val = get_pixel(im, im.w - 1, r, k);
				}
				else {
					float sx = c*w_scale;
					int ix = (int)sx;
					float dx = sx - ix;

					val = (1 - dx) * im.ptr<Vec3b>(r)[ix][k] + dx * im.ptr<Vec3b>(r)[ix + 1][k];
					//val= (1 - dx) * im.ptr<Vec3b>(ix)[r][k] + dx * im.ptr<Vec3b>(ix+1)[r][k];
					//val = (1 - dx) * get_pixel(im, ix, r, k) + dx * get_pixel(im, ix + 1, r, k);
				}
				part.ptr<Vec3b>(r)[c][k] = (unsigned char)val;
				//part.ptr<Vec3b>(c)[r][k] = val;
				//set_pixel(part, c, r, k, val);
			}
		}
	}
	for (k = 0; k < im.channels(); ++k) {
		for (r = 0; r < h; ++r) {
			float sy = r*h_scale;
			int iy = (int)sy;
			float dy = sy - iy;
			for (c = 0; c < w; ++c) {
				float val = (1 - dy)*part.ptr<Vec3b>(iy)[c][k];
				resized.ptr<Vec3b>(r)[c][k] = val;
				//float val = (1 - dy)*part.ptr<Vec3b>(c)[iy][k];
				//resized.ptr<Vec3b>(c)[r][k] = val;
				//float val = (1 - dy) * get_pixel(part, c, iy, k);
				//set_pixel(resized, c, r, k, val);
			}
			if (r == h - 1 || im.rows == 1) continue;
			
			for (c = 0; c < w; ++c) {
				float val = dy*part.ptr<Vec3b>(iy + 1)[c][k];
				resized.ptr<Vec3b>(r)[c][k] = resized.ptr<Vec3b>(r)[c][k]+(unsigned char)val;
				//float val = (1 - dy)*part.ptr<Vec3b>(c)[iy+1][k];
				//resized.ptr<Vec3b>(c)[r][k] = val;
				//float val = dy * get_pixel(part, c, iy + 1, k);
				//add_pixel(resized, c, r, k, val);
			}
		}
	}
	//free_image(part);
	return resized;
}