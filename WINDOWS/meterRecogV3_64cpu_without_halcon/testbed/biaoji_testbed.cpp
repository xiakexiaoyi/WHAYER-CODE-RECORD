#include<iostream>
#include<opencv2/opencv.hpp>
#include "HY_MeterRecogV3.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <windows.h>
#include <cmath>
#define WINDOW_NAME "鼠标事件窗口"
#define EVENT_WINDOW "滚动条窗口"

#ifndef CV_VERSION_EPOCH
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)"" CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)"" CVAUX_STR(CV_VERSION_MAJOR)"" CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif

#pragma comment(lib, "../x64/Release/meterRecogV3.lib")

void onChangeTrackBarOut(int pos, void* usrdata);
void onChangeTrackBarIn(int pos, void* userdata);
void onChangeTrackBarRow(int pos, void* usrdata);
void onChangeTrackBarCol(int pos, void* userdata);
void onChangeTrackBarStart(int pos, void* userdata);
void onChangeTrackBarEnd(int pos, void* userdata);
void onChangeTrackBarFlag(int pos, void* userdata);
int mouseX = -1, mouseY = -1, mouseFlag = 0;
int flagEnd = 0;
void onMouse(int Event, int x, int y, int flags, void* param);

#define HYAMR_MAX_PT_LIST	30

typedef struct
{
	cv::Mat src;
	int RadiusOut;
	int RadiusIn;
	int Row;
	int Col;
	int StartAngle;
	int EndAngle;
	int flag;
}MyData;

int train(const char *input, MV3Para *pOutPattern);
int detect(const char *input, MV3Para *pOutPattern);
boost::mutex mutex;
int main()//多线程
{
	
	int count = 0;
	MV3Para para = { 0 };
	//char single_picname[100] = "../001-1.png";//单张图片文件路径 test
	//char single_picname[100] = "D:/20181217/123/250154639.jpg";//单张图片文件路径 
	char single_picname[100] = "D:/work/涌凰/Image/biaoji/001266P0T0Z0_1_1_32768/172934-579627_1-0.jpg";//单张图片文件路径
	const char *input = single_picname;
	train(input, &para);
	while (1)
	{
		boost::thread t1(boost::bind(detect, input, &para));
		//boost::thread t2(boost::bind(detect, input, &para));
		t1.join();
		//t2.join();
		printf("\n");
	}
	return 0;
}
int main123()
{
	while (1)
	{ 
		int count =0;
		MV3Para para = { 0 };
		//const char *input = "D:/图片/tmp.bmp";
		char single_picname[100]="../001-1.png";//单张图片文件路径
		printf("请输入单张照片的绝对路径,输入1退出\n");
		//scanf("%s", &single_picname);
		if (atoi(single_picname) == 1)
			break;
		const char *input = single_picname;
		train(input, &para);
		//while(1)
		{
			detect(input, &para);
			printf("count=%d\n",++count);
		}
	}
	//detect(input, &para);
	return 0;
}
int train(const char *input, MV3Para *pOutPattern)
{
	//IplImage *tmp=cvLoadImage(input);
	//cv::Mat src=cv::cvarrToMat(tmp);
	
	cv::Mat src = cv::imread(input);

	int mouseParam[3];
	/**************************************************************/
	int count = MIN(src.cols, src.rows) / 2;
	int RadiusOut = 0, RadiusIn = 0;
	cv::namedWindow(EVENT_WINDOW,0);
	cv::imshow(EVENT_WINDOW, src);
	MyData usrdata;
	usrdata.src = src;
	usrdata.RadiusOut = RadiusOut;
	usrdata.RadiusIn = RadiusIn;
	usrdata.Row = src.rows / 2;
	usrdata.Col = src.cols / 2;

	usrdata.RadiusOut = 0;
	usrdata.RadiusIn = 0;
	usrdata.Row = 0;
	usrdata.Col = 0;
	usrdata.StartAngle = 0;
	usrdata.EndAngle = 0;
	usrdata.flag = 0;  //flag=0:顺时针  flag=1:逆时针  目的是为了可视化反向填充
	/*usrdata.RadiusOut = 130;
	usrdata.RadiusIn = 70;
	usrdata.Row = 407;
	usrdata.Col = 451;
	usrdata.StartAngle = 220;
	usrdata.EndAngle = 320;*/

	usrdata.RadiusOut = 253;   //微信图片_201902271951412
	usrdata.RadiusIn = 167;
	usrdata.Row = 702;
	usrdata.Col = 323;
	usrdata.StartAngle = 227;
	usrdata.EndAngle = 322;
	usrdata.flag = 1;

	usrdata.RadiusOut = 253;   //微信图片_201902271951416
	usrdata.RadiusIn = 162;
	usrdata.Row = 646;
	usrdata.Col = 355;
	usrdata.StartAngle = 223;
	usrdata.EndAngle = 319;
	usrdata.flag = 1;

	usrdata.RadiusOut = 253;   //微信图片_201902271951415
	usrdata.RadiusIn = 162;
	usrdata.Row = 1053;
	usrdata.Col = 781;
	usrdata.StartAngle = 226;
	usrdata.EndAngle = 310;
	usrdata.flag = 1;

	usrdata.RadiusOut = 446;   //微信图片_20190227195141
	usrdata.RadiusIn = 217;
	usrdata.Row = 1364;
	usrdata.Col = 752;
	usrdata.StartAngle = 225;
	usrdata.EndAngle = 325;
	usrdata.flag = 1;

	usrdata.RadiusOut = 215;   //微信图片_20190227195141
	usrdata.RadiusIn = 172;
	usrdata.Row = 339;
	usrdata.Col = 474;
	usrdata.StartAngle = 150;
	usrdata.EndAngle = 42;
	usrdata.flag = 0;

	usrdata.RadiusOut = 83;   
	usrdata.RadiusIn = 56;
	usrdata.Row = 616;
	usrdata.Col = 859;
	usrdata.StartAngle = 220;
	usrdata.EndAngle = 320;
	usrdata.flag = 1;

	/*usrdata.RadiusOut = 131;     //1918735693.jpg
	usrdata.RadiusIn = 68;
	usrdata.Row = 353;
	usrdata.Col = 205;
	usrdata.StartAngle = 214;
	usrdata.EndAngle = 319;
	usrdata.flag = 1;*/

	/*usrdata.RadiusOut = 97;     //951872446.jpg
	usrdata.RadiusIn = 62;
	usrdata.Row = 360;
	usrdata.Col = 218;
	usrdata.StartAngle = 229;
	usrdata.EndAngle = 315;
	usrdata.flag = 1;*/

	/*usrdata.RadiusOut = 69;     //934261086.jpg
	usrdata.RadiusIn = 38;  //38,43 is error,48,49 is right
	usrdata.Row = 305;
	usrdata.Col = 218;
	usrdata.StartAngle = 217;
	usrdata.EndAngle = 318;
	usrdata.flag = 1;*/
	while (1)
	//while (0)   //test
	{
		cv::createTrackbar("RadiusOut", EVENT_WINDOW, &usrdata.RadiusOut, count, onChangeTrackBarOut, &usrdata);
		cv::createTrackbar("RadiusIn", EVENT_WINDOW, &usrdata.RadiusIn, count, onChangeTrackBarIn, &usrdata);
		cv::createTrackbar("Row", EVENT_WINDOW, &usrdata.Row, src.rows, onChangeTrackBarRow, &usrdata);
		cv::createTrackbar("Col", EVENT_WINDOW, &usrdata.Col, src.cols, onChangeTrackBarCol, &usrdata);
		cv::createTrackbar("StartAngle", EVENT_WINDOW, &usrdata.StartAngle, 360, onChangeTrackBarStart, &usrdata);
		cv::createTrackbar("EndAngle", EVENT_WINDOW, &usrdata.EndAngle, 360, onChangeTrackBarEnd, &usrdata);
		cv::createTrackbar("Flag", EVENT_WINDOW, &usrdata.flag, 1, onChangeTrackBarFlag, &usrdata);
		if (cv::waitKey(10) == 13)
			break;
	}
	cv::destroyWindow(EVENT_WINDOW);

	//return 0;
	pOutPattern->Col = usrdata.Col;
	pOutPattern->Row = usrdata.Row;
	pOutPattern->RadiusOut = usrdata.RadiusOut;
	pOutPattern->RadiusIn = usrdata.RadiusIn;
	//避雷器
	pOutPattern->AngleStart = 360-usrdata.StartAngle;
	pOutPattern->AngleEnd = 360-usrdata.EndAngle;

	//sf6
	//pOutPattern->AngleStart = 360 - usrdata.StartAngle;
	//pOutPattern->AngleEnd = -1* usrdata.EndAngle;

	//角度范围  -360 ~ 360
	int wrect = CV_PI * 2 * pOutPattern->RadiusOut;//pOutPattern->RadiusOut外径
	int wrectactual = CV_PI * 2 * pOutPattern->RadiusOut*(abs(pOutPattern->AngleStart- pOutPattern->AngleEnd))/360;//pOutPattern->AngleStart起始角度  pOutPattern->AngleEnd结尾角度
	int hrect = pOutPattern->RadiusOut - pOutPattern->RadiusIn;//pOutPattern->RadiusIn内径
	cv::Mat rect = cv::Mat::zeros(cv::Size(wrectactual, hrect), CV_8UC3);
	for (int k = 0; k < rect.channels(); k++)
	{
		for (int j = 0; j < hrect; j++)
		{
			for (int i = 0; i < wrectactual; i++)
			//for (int i = 0; i < wrect; i++)
			{
				double theta = CV_PI*2.0 / wrect*(i + 1);//逆时针
				double rho = pOutPattern->RadiusOut - j - 1;
				if(pOutPattern->AngleStart > pOutPattern->AngleEnd)//顺时针
					theta = -1 * theta;//调整为顺时针
				theta = theta + 1.0*pOutPattern->AngleStart / 360 * CV_PI*2.0;//调整开始角度为pOutPattern->AngleStart
				//极坐标转换 x = rcos（θ），y = rsin（θ），图片的笛卡尔坐标系y轴是逆序的
				int position_x = pOutPattern->Col + rho*std::cos(theta) + 0.5;
				int position_y = pOutPattern->Row - rho * (float)std::sin(theta) + 0.5;
				//if (position_x == 2041 && position_y == 2685)//刻度点定位
				//	printf("x=%d y=%d\n",i,j);
				rect.ptr<cv::Vec3b>(j)[i][k] = src.ptr<cv::Vec3b>(position_y)[position_x][k];	
			}
		}
	}

	cv::imwrite("D:/rect.jpg", rect);
	cv::Mat resizeimg;
	cv::resize(rect, resizeimg,cv::Size(416,64));
	cv::imwrite("D:/resizeimg.jpg", resizeimg);

	/*cv::Mat dst_1;
	cv::equalizeHist(resizeimg, dst_1);
	cv::imwrite("D:/equalizeHist.jpg", dst_1);*/


	cv::Mat imageLog(resizeimg.size(), CV_32FC3);
	for (int i = 0; i < imageLog.rows; i++)
	{
		for (int j = 0; j < imageLog.cols; j++)
		{
			imageLog.at<cv::Vec3f>(i, j)[0] = log(1 + resizeimg.at<cv::Vec3b>(i, j)[0]);
			imageLog.at<cv::Vec3f>(i, j)[1] = log(1 + resizeimg.at<cv::Vec3b>(i, j)[1]);
			imageLog.at<cv::Vec3f>(i, j)[2] = log(1 + resizeimg.at<cv::Vec3b>(i, j)[2]);
		}
	}
	cv::normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
	cv::convertScaleAbs(imageLog, imageLog);
	cv::imwrite("D:/imageLog.jpg", imageLog);
	
	cv::Mat imageGamma(resizeimg.size(), CV_32FC3);
	for (int i = 0; i < imageGamma.rows; i++)
	{
		for (int j = 0; j < imageGamma.cols; j++)
		{
			
			imageGamma.at<cv::Vec3f>(i, j)[0]=pow(resizeimg.at<cv::Vec3b>(i, j)[0], 3);
			imageGamma.at<cv::Vec3f>(i, j)[1] = pow(resizeimg.at<cv::Vec3b>(i, j)[1], 3);
			imageGamma.at<cv::Vec3f>(i, j)[2] = pow(resizeimg.at<cv::Vec3b>(i, j)[2], 3);
			
			//imageGamma.at<cv::Vec3f>(i, j)[0] = (resizeimg.at<cv::Vec3b>(i, j)[0])*(resizeimg.at<cv::Vec3b>(i, j)[0])*(resizeimg.at<cv::Vec3b>(i, j)[0]);
			//imageGamma.at<cv::Vec3f>(i, j)[1] = (resizeimg.at<cv::Vec3b>(i, j)[1])*(resizeimg.at<cv::Vec3b>(i, j)[1])*(resizeimg.at<cv::Vec3b>(i, j)[1]);
			//imageGamma.at<cv::Vec3f>(i, j)[2] = (resizeimg.at<cv::Vec3b>(i, j)[2])*(resizeimg.at<cv::Vec3b>(i, j)[2])*(resizeimg.at<cv::Vec3b>(i, j)[2]);
		}
	}
	cv::normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
	cv::convertScaleAbs(imageGamma, imageGamma);
	cv::imwrite("D:/imageGamma.jpg", imageGamma);
	/////
	/*pOutPattern->Col = 451;     //test
	pOutPattern->Row =407;
	pOutPattern->RadiusOut = 130;
	pOutPattern->RadiusIn = 70;
	pOutPattern->AngleStart = 140;
	pOutPattern->AngleEnd = 40;	*/		
	/**************************************************************/
	cv::namedWindow(WINDOW_NAME,0);
	printf("大刻度点从小往大标记: 右键添加，左键删除\n");
	
	mouseParam[0] = -1;
	mouseParam[1] = -1;
	mouseParam[2] = -1;
	cvSetMouseCallback(WINDOW_NAME, onMouse, (void*)mouseParam);
	int lPtNumTmp = 0;
	MV3POINT ptListTmp[50];
	IplImage *testImage = NULL;
	IplImage imgTmp = src;
	testImage = cvCloneImage(&imgTmp);
	cvCircle(testImage, cvPoint(usrdata.Col, usrdata.Row), usrdata.RadiusOut, CV_RGB(255, 0, 0),2);
	cvCircle(testImage, cvPoint(usrdata.Col, usrdata.Row), usrdata.RadiusIn, CV_RGB(255, 0, 0), -1);
	if (usrdata.flag == 0)
		cvEllipse(testImage,cvPoint(usrdata.Col, usrdata.Row),cvSize(usrdata.RadiusOut, usrdata.RadiusOut),0,usrdata.StartAngle,usrdata.EndAngle,CV_RGB(255, 0, 0), -1);
		//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, pos, CV_RGB(255, 0, 0), -1);
	else
	{
		if (usrdata.StartAngle<usrdata.EndAngle)
			cvEllipse(testImage,cvPoint(usrdata.Col, usrdata.Row),cvSize(usrdata.RadiusOut, usrdata.RadiusOut),0,usrdata.StartAngle,usrdata.EndAngle-360,CV_RGB(255, 0, 0), -1);
			//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, pos - 360, CV_RGB(255, 0, 0), -1);
		else
			cvEllipse(testImage,cvPoint(usrdata.Col, usrdata.Row),cvSize(usrdata.RadiusOut, usrdata.RadiusOut),0,usrdata.StartAngle-360,usrdata.EndAngle,CV_RGB(255, 0, 0), -1);
			//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, pos, CV_RGB(255, 0, 0), -1);
	}
	while (1)
	//while (0)  //test
	{
		if (CV_EVENT_RBUTTONDOWN == mouseParam[2])	// add red circle
		{
			if (mouseParam[0] >= 0 && mouseParam[0]<src.cols
				&& mouseParam[1] >= 0 && mouseParam[1]<src.rows)
			{
				ptListTmp[lPtNumTmp].x = mouseParam[0];
				ptListTmp[lPtNumTmp].y = mouseParam[1];
				lPtNumTmp++;
				cvCircle(testImage, cvPoint(mouseParam[0], mouseParam[1]), 1, CV_RGB(255, 0, 0), -1, 8, 0);
			}
			printf("ptListTmp[%d].x=%d ptListTmp[%d].y=%d\n", lPtNumTmp, mouseParam[0], lPtNumTmp, mouseParam[1]);
			printf("lPtNum=%d\n", lPtNumTmp);
			mouseParam[2] = -1;
		}
		else if (CV_EVENT_LBUTTONDOWN == mouseParam[2])		// delete white circle
		{
			for (int i = 0; i<lPtNumTmp; i++)
			{
				if (abs(mouseParam[0] - ptListTmp[i].x)<5 && abs(mouseParam[1] - ptListTmp[i].y)<5)
				{
					cvCircle(testImage, cvPoint(ptListTmp[i].x, ptListTmp[i].y), 1, CV_RGB(255, 255, 255), -1, 8, 0);
					for (int j = i; j<lPtNumTmp - 1; j++)
						ptListTmp[j] = ptListTmp[j + 1];
					lPtNumTmp--;
					break;
				}
			}
			mouseParam[2] = -1;
		}
		cvShowImage(WINDOW_NAME, testImage);
		if (13 == cvWaitKey(10))	// Enter
			break;
	}
	cv::destroyWindow(WINDOW_NAME);
	/*lPtNumTmp = 11;
	ptListTmp[0].x = 2041;
	ptListTmp[0].y = 2685;
	ptListTmp[1].x = 1837;
	ptListTmp[1].y = 2389;
	ptListTmp[2].x = 1809;
	ptListTmp[2].y = 2033;
	ptListTmp[3].x = 1953;
	ptListTmp[3].y = 1697;
	ptListTmp[4].x = 2221;
	ptListTmp[4].y = 1469;
	ptListTmp[5].x = 2577;
	ptListTmp[5].y = 1381;
	ptListTmp[6].x = 2933;
	ptListTmp[6].y = 1473;
	ptListTmp[7].x = 3201;
	ptListTmp[7].y = 1713;
	ptListTmp[8].x = 3333;
	ptListTmp[8].y = 2049;
	ptListTmp[9].x = 3305;
	ptListTmp[9].y = 2405;
	ptListTmp[10].x = 3125;
	ptListTmp[10].y = 2721;*/
	/*lPtNumTmp = 4;   //test
	ptListTmp[0].x =369 ;
	ptListTmp[0].y = 315;
	ptListTmp[1].x =427 ;
	ptListTmp[1].y = 285;
	ptListTmp[2].x =488 ;
	ptListTmp[2].y =290 ;
	ptListTmp[3].x = 543;
	ptListTmp[3].y =320 ;*/
	printf("lPtNum=%d\n", lPtNumTmp);
	pOutPattern->lPtNum = lPtNumTmp;
	//fprintf(fp, "%d ", pOutPattern->lPtNum);
	for (int i = 0; i<lPtNumTmp; i++)
	{
		pOutPattern->ptPosList[i] = ptListTmp[i];
		//fprintf(fp, "%ld ", pOutPattern->ptPosList[i].x);
		//fprintf(fp, "%ld ", pOutPattern->ptPosList[i].y);
	}
	//long y1 = ptListTmp[0].y - usrdata.Row;
	//long x1 = ptListTmp[0].x - usrdata.Col;
	//long y2 = ptListTmp[lPtNumTmp-1].y - usrdata.Row;
	//long x2 = ptListTmp[lPtNumTmp-1].x - usrdata.Col;
	//pOutPattern->AngleStart = atan2l(y1, x1);
	//pOutPattern->AngleEnd = atan2l(y2, x2);
	//printf("%f %f\n", atan2l(y1, x1) * 180 / CV_PI, atan2l(y2, x2) * 180 / CV_PI);
	long lTmpMin;
	long step;
	/*step = 1;   //test
	lTmpMin=0;*/
	printf("请输入步长\n");
	scanf("%d", &step);
	printf("请输入最小值\n");
	scanf("%d", &lTmpMin);
	for (int i = 0; i<lPtNumTmp; i++)
	{
		pOutPattern->dPtValList[i] = (double)lTmpMin;
		printf("pOutPattern->dPtValList[%d] = %d\n", i, lTmpMin);
		//fprintf(fp, "%f ", pOutPattern->dPtValList[i]);
		lTmpMin = lTmpMin + step;
	}

	cvReleaseImage(&testImage);
	return 0;
}
#define GrayStep(x)			(( (x) + 3 ) & (~3) )
int detect(const char *input, MV3Para *pOutPattern)
{
	void *handle = NULL;
	char *cfgfile = "D:/work/model/zhizhen/tiny-yolo-voc.cfg";
	char *weightfile = "D:/work/model/zhizhen/tiny-yolo-voc_final.weights";
	if (0 != yoloinit(&handle, cfgfile, weightfile, 0.24))//初始化
	{
		yolouninit(handle);
		return -1;
	}
	//while(1)
	{ 
		IplImage *src = cvLoadImage(input);
		HYMRV3_IMAGES image = { 0 };
		HYMR_POINTERRESULT MeterResult[1]={0};								   
		image.lWidth = src->width;
		image.lHeight = src->height;
		image.pixelArray.chunky.lLineBytes = src->widthStep;
		image.pixelArray.chunky.pPixel = src->imageData;
		image.lPixelArrayFormat = HYAMR_IMAGE_BGR;
		//pOutPattern->cfgfile=cfgfile;
		//pOutPattern->weightfile=weightfile;
		printf("h=%d w=%d\n",image.lHeight, image.lWidth);
		if(0!=MeterReadRecogV3(handle,&image, *pOutPattern,MeterResult))//检测
		{
			printf("MeterReadRecogV3 failed!\n");
			yolouninit(handle);
			cvReleaseImage(&src);
			return -1;
		}
		printf("h=%d w=%d\n", image.lHeight, image.lWidth);
		//MeterReadRecogV3(&image, *pOutPattern,MeterResult);
		//printf("MeterValue=%f\n",MeterResult[0].MeterValue);
		mutex.lock();
		std::cout <<"id:"<< boost::this_thread::get_id() <<",MeterValue="<< std::setiosflags(std::ios::fixed) << std::setprecision(2)<< MeterResult[0].MeterValue<< std::endl;
		mutex.unlock();
		cvReleaseImage(&src);
	}
	yolouninit(handle);//释放初始化内存
	return 0;

}

void onChangeTrackBarOut(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), pos, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(pos, pos), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(pos, pos), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(pos, pos), 0, input.StartAngle, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(pos, pos), 0, input.StartAngle - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarIn(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), pos, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarRow(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, pos), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, pos), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(input.Col, pos), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(input.Col, pos), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, pos), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, pos), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarCol(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(pos, input.Row), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(pos, input.Row), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(pos, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(pos, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(pos, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(pos, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarStart(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, pos, input.EndAngle, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, pos, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, pos, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, pos - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarEnd(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	//cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, pos, CV_RGB(255, 0, 0), -1);
	if (input.flag == 0)
		cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, pos, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, pos - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, pos, CV_RGB(255, 0, 0), -1);
	}
	cv::imshow(EVENT_WINDOW, tempImage);
}
void onChangeTrackBarFlag(int pos, void* usrdata)
{
	MyData input = *(MyData*)(usrdata);
	cv::Mat tempImage;
	input.src.copyTo(tempImage);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusOut, CV_RGB(255, 0, 0), 2);
	cv::circle(tempImage, cv::Point(input.Col, input.Row), input.RadiusIn, CV_RGB(255, 0, 0), -1);
	if (pos==0)
		cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle, CV_RGB(255, 0, 0), -1);
	else
	{
		if (input.StartAngle<input.EndAngle)
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle, input.EndAngle - 360, CV_RGB(255, 0, 0), -1);
		else
			cv::ellipse(tempImage, cv::Point(input.Col, input.Row), cv::Size(input.RadiusOut, input.RadiusOut), 0, input.StartAngle - 360, input.EndAngle, CV_RGB(255, 0, 0), -1);
	}
	//cv::imwrite("../123.png",tempImage);
	cv::imshow(EVENT_WINDOW, tempImage);
	//cv::imwrite("../123.png",tempImage);
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
		flagEnd = 1;
	}
}

