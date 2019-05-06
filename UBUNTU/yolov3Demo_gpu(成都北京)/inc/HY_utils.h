#ifndef HY_UTILS_H
#define HY_UTILS_H
#include <opencv2/opencv.hpp>
#include "yolo_dll_gpu.h"
typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
} CRECT, *PCRECT;

typedef struct dr_result
{
	int flag;//分类标签
	CRECT    Target;///目标rectangle
	float dVal; //score值
}HYCD_RESULT;
typedef struct
{
	HYCD_RESULT *pResult;
	int  lResultNum;
}HYCD_RESULT_LIST;


void del_arg(int argc, char **argv, int index);
int find_int_arg(int argc, char **argv, char *arg, int def);
float find_float_arg(int argc, char **argv, char *arg, float def);
char *find_char_arg(int argc, char **argv, char *arg, char *def);

cv::Mat HY_imReszie(cv::Mat pImg, float *im_scale_x, float *im_scale_y);

 
//----------------8种算法模块函数-------------------
void Detection_jyz(void *handle, const char* picpath, char *savePath, char *fname, char *exts, int *BreakNum);
void Detection_yiwu(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum);
void Detection_youwu(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum);
void Detection_xiushi(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum);
void Detection_ganta(void *handle, const char* picpath, char *savePath, char *fname, char *exts);
void Detection_break(void *handle1, void *handle2, const char* picpath, char *savePath1, char *savePath2, char *savePath3, char *savePath4, char *fname, char *exts, int BreakNum[]);

//5类算法外部接口函数
int gantaModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList);
int yiwuModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList);
int xiushiModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList);
int youwuModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList);
int jyzModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList);
int breakModelDetection(void *yolo_handle1, void *yolo_handle2, char *cfgFile1, char *weightFile1, char *cfgFile2, char *weightFile2, char *savePath, float score, char* ImgList);

#ifndef WIN32
 static void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
 static void _split_whole_name(const char *whole_name, char *fname, char *ext);
#endif




#endif
