#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "HY_utils.h"
using namespace std;

int main(int argc, char *argv[])
{
	//--------局部变量-----------
	void *yolo_handle = NULL;
	void *yolo_handle2 = NULL;
	int detectM[20] = { 0 };


	float nmsthreshold = 0.3;//nms阈值
	float score = 0.76;//得分
	char savePath[256] = "../../result.csv";//结果输出文件
	char imagelist[256] = "../../list.txt";//图片列表
	char modelType[256] = "M10";//类型名称

	//---绝缘子 模型------------------//
	char *cfgFile_jyz = "../../model/jyz.cfg";
	char *weightFile_jyz = "../../model/jyz.weights";
	//----异物 模型----------------------//
	char *cfgFile_yiwu = "../../model/yw.cfg";
	char *weightFile_yiwu = "../../model/yw.weights";
	//----锈蚀 模型----------------------//
	char *cfgFile_xiushi = "../../model/xiushi.cfg";
	char *weightFile_xiushi = "../../model/xiushi.weights";
	//----油污 模型----------------------//
	char *cfgFile_youwu = "../../model/youwu.cfg";
	char *weightFile_youwu = "../../model/youwu.weights";


	//--变电破损类 模型------------------//

	char *cfgFile_break1 = "../../model/break1.cfg";
	char *weightFile_break1 = "../../model/break1.weights";
	char *cfgFile_break2 = "../../model/break2.cfg";
	char *weightFile_break2 = "../../model/break2.weights";


	//float nmsthreshold = find_float_arg(argc, argv, "--nms", 0);
	//float score = find_float_arg(argc, argv, "--score", 0);
	//char *savePath = find_char_arg(argc, argv, "--savepath", 0);
	//char *imagelist = find_char_arg(argc, argv, "--imagelist", 0);
	//char *modelType = find_char_arg(argc, argv, "--modelTypes", 0);

	for (int i = 1; i < 21; i++)
	{
		char Mchar[100];
		sprintf(Mchar, "M%d", i);
		int Same = strcmp(modelType, Mchar);

		if (Same == 0)
		{
			switch (i)
			{
			case 2: //异物
				yiwuModelDetection(yolo_handle, cfgFile_yiwu, weightFile_yiwu, savePath, score, imagelist);
				break;

			case 3: //锈蚀
				xiushiModelDetection(yolo_handle, cfgFile_xiushi, weightFile_xiushi, savePath, score, imagelist);
				break;
			case 4: //油污
				youwuModelDetection(yolo_handle, cfgFile_youwu, weightFile_youwu, savePath, score, imagelist);
				break;
			case 6: //绝缘子
				jyzModelDetection(yolo_handle, cfgFile_jyz, weightFile_jyz, savePath, score, imagelist);
				break;

			case 10://表计和呼吸器
				breakModelDetection(yolo_handle, yolo_handle2, cfgFile_break1, weightFile_break1, cfgFile_break2, weightFile_break2, savePath, score, imagelist);
				break;

			default:
				printf("没有集成该算法,请检查缺陷类别!\n");
				break;

			}
		}
	}


	

	printf("---process complete---\n");
	system("pause");
	
	return 0;

}
