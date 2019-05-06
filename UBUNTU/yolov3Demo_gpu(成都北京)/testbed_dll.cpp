#define GPU 
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "yolo_dll.h"
//-----------------------
//#include <io.h>
//#include <vector>
//#include <AtlBase.h>
//#include <tchar.h>

//-----------------------


using namespace std;

//static void GetAllFileInfo(LPCTSTR path, vector<LPCTSTR> &filesPathVector);
static void SingleDetection(void* handle, cv::Mat input, const char* outFile, int imgIndex, bool Save, bool Show);

int main()
{
	//-----�ֲ�����-----------------
	//vector<LPCTSTR> filesPathVector;//�ļ�����ÿ����Ƭ·������
	void *handle = NULL;

	bool model = 0;// ���ͼƬ�ļ��л��Ǽ����Ƶ 1���ļ��� 0����Ƶ

	char* outFile = "../../result/";//����洢��·��

	char *videoPath="../../Image/fire1.avi";//��Ƶ·��
	char *multPicPath="F:/Work_Whayer/HYyolo_v3/yolov3_lib/yolov3_lib/1030/1";//ͼƬ�ļ���·�� 

	char *cfgFile = "../../model/yolov3-voc.cfg";
	char *weightFile = "../../model/yolov3-voc_75000.weights";

	//-----��ʼ��-----------------
	init(&handle, cfgFile, weightFile, 0.1,0);//�����cfg�ļ�·����ģ���ļ�·������ֵ

	
	if (model)
	{
		//��ȡ�ļ���
		/*cv::Mat input;

		GetAllFileInfo(_T(multPicPath), filesPathVector);
		int count = filesPathVector.size();

		for (int n = 0; n < count; n++)
		{
			//char *imgFile = "D:/01Fire_Somke_Sets/Test/422.jpg";
			char imgFile[100];

			sprintf_s(imgFile, "%s", filesPathVector[n]);//Photo path

			input = cv::imread(imgFile);
			SingleDetection(handle, input, outFile, n, 1, 0);//
		}

		input.release();*/

	}
	else
	{
		//��ȡ��Ƶ

		cv::VideoCapture capture(videoPath);
	
		if (!capture.isOpened())
		{
			std::cout << "video fail to open!" << std::endl;
			return -1;
		}
		cv::Mat frame;
		int idx = 0;
		//read	 			
		while (1)
		{

			if (!capture.read(frame))
				break;
			SingleDetection(handle, frame, outFile, idx, 1, 1);
			idx++;
		}

		capture.release();
	}
	
	uninit(handle);

	//system("pause");
	return 0;
}

/*
input:�����ͼƬMat
outFile:���������ļ���·��
imgIndex��ͼƬ������
Save���Ƿ񱣴��� 1:���� 0:������
Show���Ƿ���ʾ��� 1:��ʾ 0:����ʾ
*/
//���ż�⺯��
static void SingleDetection(void* handle, cv::Mat input,const char* outFile, int imgIndex, bool Save, bool Show)
{
	//--------�ֲ�����----------------
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;
	char outputFile[100];


	if (input.empty())
	{

		printf("error:load photo error!\n");
		return;
	}

	YOLOV3_IMAGES img;
	img.lHeight = input.rows;
	img.lWidth = input.cols;
	img.pixelArray.chunky.lLineBytes = input.step;
	img.pixelArray.chunky.pPixel = input.data;

	img.lPixelArrayFormat = 3;
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));

	detec(handle, &img, &resultlist);



	for (int i = 0; i < resultlist.lResultNum; i++)
	{


		int c = (int)resultlist.pResult[i].dVal;

		switch (c)
		{
		case 0:
			left = resultlist.pResult[i].Target.left;
			top = resultlist.pResult[i].Target.top;
			right = resultlist.pResult[i].Target.right;
			bottom = resultlist.pResult[i].Target.bottom;
			

			cv::rectangle(input, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 255, 255));
			cv::putText(input, "Fire", cv::Point(left, top + 20), CV_FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(0, 255, 255));

			break;
		case 1:

			left = resultlist.pResult[i].Target.left;
			top = resultlist.pResult[i].Target.top;
			right = resultlist.pResult[i].Target.right;
			bottom = resultlist.pResult[i].Target.bottom;

			cv::rectangle(input, cvPoint(left, top), cvPoint(right, bottom), cvScalar(0, 0, 255));
			cv::putText(input, "Fireothers", cv::Point(left, top + 20), CV_FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(0, 0, 255));


			break;
		case 2:
			left = resultlist.pResult[i].Target.left;
			top = resultlist.pResult[i].Target.top;
			right = resultlist.pResult[i].Target.right;
			bottom = resultlist.pResult[i].Target.bottom;

			cv::rectangle(input, cvPoint(left, top), cvPoint(right, bottom), cvScalar(255, 0, 0));
			cv::putText(input, "Smoke", cv::Point(left, top + 20), CV_FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(255, 0, 0));
			break;
		case 3:
			left = resultlist.pResult[i].Target.left;
			top = resultlist.pResult[i].Target.top;
			right = resultlist.pResult[i].Target.right;
			bottom = resultlist.pResult[i].Target.bottom;

			cv::rectangle(input, cvPoint(left, top), cvPoint(right, bottom), cvScalar(255, 0, 255));
			cv::putText(input, "Smokeothers", cv::Point(left, top + 20), CV_FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(255, 0, 255));
			break;
		default:
			break;
		}

	}
	if (Save)
	{
		sprintf(outputFile, "%s/res_%d.jpg", outFile, imgIndex);//�ļ���·�� ����
		cv::imwrite(outputFile, input);
	}
	if (Show)
	{
		cv::imshow("Result Show", input);
		cv::waitKey(5);
	}

	free(resultlist.pResult);

}

/*
//�õ������ļ�����Ϣ
static void GetAllFileInfo(LPCTSTR path, vector<LPCTSTR> &filesPathVector)
{
	//�ҵ���һ���ļ�
	_tfinddata64_t c_file;
	intptr_t hFile;
	TCHAR root[MAX_PATH];
	_tcscpy(root, path);
	_tcscat(root, _T("\\*.*"));//���Ҵ���׺���ļ�����
	hFile = _tfindfirst64(root, &c_file);
	if (hFile == -1)
		return;

	//search all files recursively.
	do
	{
		if (_tcslen(c_file.name) == 1 && c_file.name[0] == _T('.')
			|| _tcslen(c_file.name) == 2 && c_file.name[0] == _T('.') && c_file.name[1] == _T('.'))
			continue;
		TCHAR *fullPath = new TCHAR[MAX_PATH];
		_tcscpy(fullPath, path);
		_tcscat(fullPath, _T("\\"));
		_tcscat(fullPath, c_file.name);
		if (c_file.attrib&_A_SUBDIR)
		{
			GetAllFileInfo(fullPath, filesPathVector);
		}
		else
		{
			//���ļ�ȫ·��������vector��
			filesPathVector.push_back(fullPath);
		}
	} while (_tfindnext64(hFile, &c_file) == 0);
	//�ر����� handle
	_findclose(hFile);
}*/
