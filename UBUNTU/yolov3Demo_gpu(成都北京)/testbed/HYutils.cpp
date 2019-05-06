#include "HY_utils.h"
#include <iostream>
#include <fstream>
#include <time.h>
#define PRINT 1
#define CELVE 1
using namespace std;


std::vector<string> ListPreprocess(char* imagelist, int* lines, char* savePath);


//缩放函数
 cv::Mat HY_imReszie(cv::Mat pImg, float *im_scale_x, float *im_scale_y)
{
	cv::Mat dst;
	int s_min = min(pImg.cols, pImg.rows);
	int s_max = max(pImg.cols, pImg.rows);

	int size_align = 32;
	int im_height = pImg.rows;
	int im_width = pImg.cols;
	int im_size_min = min(im_height, im_width);
	int im_size_max = max(im_height, im_width);
	float im_scale = 1.0f;
	//float im_scale = s_min / (float)im_size_min;
	if (s_max > 2500)
	{
		s_max = 2500;
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


//-----------------------------------------------------------//
//@ 功能： break检测函数模块 YOLO框架
void Detection_break(void *handle1, void *handle2, const char *picpath, char *savePath1, char *savePath2, char *savePath3, char *savePath4, char *fname, char *exts,int *BreakNum)
{
	char name[256] = "break";
	FILE *fpcsv1,*fpcsv2,*fpcsv3,*fpcsv4;
	fpcsv1 = fopen(savePath1, "at+");
	fpcsv2 = fopen(savePath2, "at+");
	fpcsv3 = fopen(savePath3, "at+");
	fpcsv4 = fopen(savePath4, "at+");
	if ((NULL == fpcsv1) || (NULL == fpcsv2) || (NULL == fpcsv3) ||(NULL == fpcsv4))
	{
		printf("can not open file %s!\n", savePath1);
		exit(-1);
	}

	float score2 = 0.36;
	//第一步检测结果
	HYYOLOV3RESULT_LIST  resultlist1 = { 0 };
	resultlist1.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	
	//第二步检测结果
	HYCD_RESULT_LIST  Defectlist[100] = { 0 };
	for (int i = 0; i < 100; i++)
	{
		Defectlist[i].pResult = (HYCD_RESULT*)malloc(100 * sizeof(HYCD_RESULT));
	}

	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}
	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle1, &img, &resultlist1);//第一个模型检测
        float RoIscale = 0.3;
//	std::cout << "第一步检测数目： " << resultlist1.lResultNum << std::endl;
	if (resultlist1.lResultNum>0)
	{

		for (int i = 0; i < resultlist1.lResultNum;i++)
		{

			HYYOLOV3RESULT_LIST  resultlist2 = { 0 };
			resultlist2.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));

			
			cv::Mat imageROI;
			int width = resultlist1.pResult[i].Target.right - resultlist1.pResult[i].Target.left;
			int height = resultlist1.pResult[i].Target.bottom - resultlist1.pResult[i].Target.top;


                        width += width * RoIscale;
                        height += height * RoIscale;
                        width = min(width, input.cols);
                        height = min(height, input.rows);

//                        std::cout << "l " << resultlist1.pResult[i].Target.left << std::endl;;
//                        std::cout << "r " << resultlist1.pResult[i].Target.right << std::endl;
//                        std::cout << "t " << resultlist1.pResult[i].Target.top << std::endl;
//                        std::cout << "b " << resultlist1.pResult[i].Target.bottom << std::endl;



//			int x = resultlist1.pResult[i].Target.left;
//			int y = resultlist1.pResult[i].Target.top;
//			int x = resultlist1.pResult[i].Target.left - 0.15*width;
//			int y = resultlist1.pResult[i].Target.top - 0.15*height;
                        int x = (resultlist1.pResult[i].Target.left + resultlist1.pResult[i].Target.right)/2;
			int y = (resultlist1.pResult[i].Target.top + resultlist1.pResult[i].Target.bottom)/2;
                        
                        int left,right,top,bottom;
                        left = x-width/2+1 ;
                        top = y- height/2+1 ;
                        right = x+width/2-1;
                        bottom = y+height/2-1; 

//                        std::cout << "x " << x << std::endl;
//                        std::cout << "y " << y << std::endl;


                        if (left < 0)
                            left = 1;
                        if (top < 0)
                            top = 1;
                        if (right > input.cols)
                            right = input.cols-1;
                        if (bottom > input.rows)
                            bottom = input.rows-1;
//			imageROI = input(cv::Rect(x, y, width, height)); //
			imageROI = input(cv::Rect(left, top, right-left, bottom-top));

//                        std::cout << "x " << x << std::endl;;
//                        std::cout << "y " << y << std::endl;
//                        std::cout << "width " << width << std::endl;
//                        std::cout << "height " << height << std::endl;


			
			
			//char outputFile11[100];
			//sprintf_s(outputFile11, "./result_%d.jpg", x);
			//cv::imwrite(outputFile11, imageROI);
		
			YOLOV3_IMAGES img2;
			img2.lHeight = imageROI.rows;
			img2.lWidth = imageROI.cols;
			img2.pixelArray.chunky.lLineBytes = imageROI.step;
			img2.pixelArray.chunky.pPixel = imageROI.data;
			img2.lPixelArrayFormat = 3;

	                char outputFile1[512];
	                sprintf(outputFile1, "../../result_part1/res_%s%d%dkind_%d.jpg",fname,x,y,resultlist1.pResult[i].dVal);
	                cv::imwrite(outputFile1, imageROI);


			//输入 第一个模型的结果图
			detec_gpu(handle2, &img2, &resultlist2);// 第二个模型检测

			for (int j = 0; j < resultlist2.lResultNum; j++)
			{
				if (resultlist2.pResult[j].dConfidence > score2)
				{
					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.left = resultlist2.pResult[j].Target.left;
					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.top = resultlist2.pResult[j].Target.top;

					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.right = resultlist2.pResult[j].Target.right;
					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.bottom = resultlist2.pResult[j].Target.bottom;

					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.left += left;
					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.top += top;

					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.right += left;
					Defectlist[i].pResult[Defectlist[i].lResultNum].Target.bottom += top;


					Defectlist[i].pResult[Defectlist[i].lResultNum].dVal = resultlist2.pResult[j].dConfidence;//socre 值
					Defectlist[i].pResult[Defectlist[i].lResultNum].flag = resultlist2.pResult[j].dVal; //标签值
					Defectlist[i].lResultNum++;

				}

			}

			free(resultlist2.pResult);
			imageROI.release();
		}
		
	}

	
	cv::Mat resultImage;
	resultImage = input.clone();

	for (int s = 0; s < resultlist1.lResultNum;s++)
	{
		for (int i = 0; i < Defectlist[s].lResultNum; i++)
		{
                        int flag_mohu=0;
                        int flag_break=0;
                        int flag_si=0;
                        int flag_oil=0;

			//表模糊0 表损坏1 表正常2 呼吸器硅胶变色3 呼吸器油封正常4 呼吸器油封破损5

			//根据标签判断输出策略
			char text[20] = { 0 };
			int ResLabel = Defectlist[s].pResult[i].flag;//获取标签值
			if (ResLabel==4) //如果表计正常或者油封正常 不输出
				continue;

			if ((ResLabel == 0 || ResLabel == 1 || ResLabel == 2) &&(resultlist1.pResult[s].dVal == 0))
			{
				// @功能：只输出表计破损或者模糊，输出第一步框，输出第二步得分和标签
                                
				//表计模糊
				if ((ResLabel == 0 && flag_mohu == 0) || (ResLabel == 2))
				{

					//只取出第二步中 所有标签 合并
					float MaxScore_1 = 0.0;//表计模糊最高得分
					int Res1Index_1 = 0; //表计模糊第一步索引号
					int Res2Index_1 = 0;//表计模糊第二步索引号

					for (int k = 0; k < Defectlist[s].lResultNum; k++)
					{
						if (Defectlist[s].pResult[k].flag == 0)
						{
							float BiaoScore = Defectlist[s].pResult[k].dVal;
							if (BiaoScore > MaxScore_1)
							{
								MaxScore_1 = BiaoScore;
								Res1Index_1 = s;
								Res2Index_1 = k;
							}
						}

					}
                                        if (MaxScore_1 > 0)
                                        {


                                            MaxScore_1 = (MaxScore_1 + resultlist1.pResult[s].dConfidence) / 2.0;
					    int x = resultlist1.pResult[Res1Index_1].Target.left;
					    int y = resultlist1.pResult[Res1Index_1].Target.top;
					    int x2 = resultlist1.pResult[Res1Index_1].Target.right;
					    int y2 = resultlist1.pResult[Res1Index_1].Target.bottom;

					    int width = x2 - x;
					    int height = y2 - y;
			
					    char* name = "bj_bjps";

					    (*BreakNum)++;
					    fprintf(fpcsv1, "%d,../../TESTA/%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, exts, name, MaxScore_1, x, y, x2, y2);
#if PRINT
					    sprintf(text, "%s %0.3f", name, MaxScore_1);
					    cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
					    cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif

                                        }
					//获取第一步框子
//					int x = resultlist1.pResult[Res1Index_1].Target.left;
//					int y = resultlist1.pResult[Res1Index_1].Target.top;
//					int x2 = resultlist1.pResult[Res1Index_1].Target.right;
//					int y2 = resultlist1.pResult[Res1Index_1].Target.bottom;
//
//					int width = x2 - x;
//					int height = y2 - y;
//			
//					char* name = "bj_bjps";
//
//					(*BreakNum)++;
//					fprintf(fpcsv1, "%d,C:\\TESTA\\%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, exts, name, MaxScore_1, x, y, x2, y2);
//#if PRINT
//					sprintf(text, "%s %0.3f", name, MaxScore_1);
//					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
//					cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
//#endif

				}
		
				//表计破损
//				if ((ResLabel == 1) && (flag_break == 0))
                                if ((ResLabel == 1 && flag_break == 0) || (ResLabel == 2))
				{
					// 取出第二步结果中 表计破损标签中得分最高的标签及索引
					//只取出第二步中 所有标签 合并
					float MaxScore_2 = 0.0;//表计破损最高得分
					int Res1Index_2 = 0; //表计破损第一步索引号
					int Res2Index_2 = 0;//表计破损第二步索引号

					for (int k = 0; k < Defectlist[s].lResultNum; k++)
					{
						if (Defectlist[s].pResult[k].flag == 1)
						{
							float BiaoScore = Defectlist[s].pResult[k].dVal;
							if (BiaoScore > MaxScore_2)
							{
								MaxScore_2 = BiaoScore;
								Res1Index_2 = s;
								Res2Index_2 = k;
							}
						}

					}
                                        if (MaxScore_2 > 0)
                                        {
                                            MaxScore_2 = (MaxScore_2 + resultlist1.pResult[s].dConfidence) / 2.0;
					    int x = resultlist1.pResult[Res1Index_2].Target.left;
					    int y = resultlist1.pResult[Res1Index_2].Target.top;
					    int x2 = resultlist1.pResult[Res1Index_2].Target.right;
					    int y2 = resultlist1.pResult[Res1Index_2].Target.bottom;

					    int width = x2 - x;
					    int height = y2 - y;
					    char* name = "bj_bpmh";

					    (*BreakNum)++;

					    fprintf(fpcsv2, "%d,../../TESTA/%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, exts, name, MaxScore_2, x, y, x2, y2);
#if PRINT
					    sprintf(text, "%s %0.3f", name, MaxScore_2);
					    cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
					    cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
                                        }
					//获取第一步框子
//					int x = resultlist1.pResult[Res1Index_2].Target.left;
//					int y = resultlist1.pResult[Res1Index_2].Target.top;
//					int x2 = resultlist1.pResult[Res1Index_2].Target.right;
//					int y2 = resultlist1.pResult[Res1Index_2].Target.bottom;
//
//					int width = x2 - x;
//					int height = y2 - y;
//					char* name = "bj_bpmh";
//                                        const char* name2 = "C:/TESTA/";

//					(*BreakNum)++;

//					fprintf(fpcsv2, "%d,C:\\TESTA\\%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, exts, name, MaxScore_2, x, y, x2, y2);
//#if PRINT
//					sprintf(text, "%s %0.3f", name, MaxScore_2);
//					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
//					cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
//#endif

				}

				break;
			}
			//硅胶变色或者 油封破损
			if ((ResLabel == 3 || ResLabel == 5) && (resultlist1.pResult[s].dVal == 1))
			{

				// @功能：只输出硅胶变色和油封破损，输出第一步框
				//硅胶变色
				if ((ResLabel == 3) && (flag_si == 0))
				{


					// 取出第二步结果中 硅胶变色标签中得分最高的标签及索引
					//只取出第二步中 所有标签 合并
					float MaxScore_1 = 0.0;//硅胶变色最高得分
                                        float MaxRight_1 = 0.0;
                                        float MaxBottom_1 = 0.0;
					int Res1Index_1 = 0; //硅胶变色第一步索引号
					int Res2Index_1 = 0;//硅胶变色第二步索引号

					for (int k = 0; k < Defectlist[s].lResultNum; k++)
					{
						if (Defectlist[s].pResult[k].flag == 3)
						{
							float BiaoScore = Defectlist[s].pResult[k].dVal;
                                                        float MaxRight = Defectlist[s].pResult[k].Target.right;
                                                        float MaxBottom = Defectlist[s].pResult[k].Target.bottom;

//                                                       Res1Index_1 = s;
//							 Res2Index_1 = k;
							if (BiaoScore > MaxScore_1)
							{
								MaxScore_1 = BiaoScore;
								Res1Index_1 = s;
								Res2Index_1 = k;
							}
                                                        if (MaxRight > MaxRight_1)
							{
								MaxRight_1 = MaxRight;
								//Res1Index_1 = s;
								//Res2Index_1 = k;
							}
                                                        if (MaxBottom > MaxBottom_1)
							{
								MaxBottom_1 = MaxBottom;
								//Res1Index_1 = s;
								//Res2Index_1 = k;
							}
						}

					}
					
//					//获取第一步框子,框子高减少10%
//					int x = resultlist1.pResult[Res1Index_1].Target.left;
//					int y = resultlist1.pResult[Res1Index_1].Target.top;
//					int x2 = resultlist1.pResult[Res1Index_1].Target.right;
//					int y2 = resultlist1.pResult[Res1Index_1].Target.bottom;
                                        int x,y,x2,y2,width_tmp,height_tmp; 
                                        x = min(resultlist1.pResult[Res1Index_1].Target.left,Defectlist[Res1Index_1].pResult[Res2Index_1].Target.left);
                                        y = min(resultlist1.pResult[Res1Index_1].Target.top,Defectlist[Res1Index_1].pResult[Res2Index_1].Target.top);

                                        x2 = min(resultlist1.pResult[Res1Index_1].Target.right,int(MaxRight_1));
                                        y2 = min(resultlist1.pResult[Res1Index_1].Target.bottom,int(MaxBottom_1));
                    

//					int x2_tmp =x+0.85*( resultlist1.pResult[Res1Index_1].Target.right - resultlist1.pResult[Res1Index_1].Target.left);
//					int y2_tmp =y+0.85*( resultlist1.pResult[Res1Index_1].Target.bottom - resultlist1.pResult[Res1Index_1].Target.top);
//                                        x2 = max(Defectlist[Res1Index_1].pResult[Res2Index_1].Target.right,x2_tmp);
//                                        y2 = max(Defectlist[Res1Index_1].pResult[Res2Index_1].Target.top,y2_tmp);


                                        width_tmp = resultlist1.pResult[Res1Index_1].Target.right-resultlist1.pResult[Res1Index_1].Target.left;
                                        height_tmp =resultlist1.pResult[Res1Index_1].Target.bottom-resultlist1.pResult[Res1Index_1].Target.top;
                                        if((x2 - x)*(y2-y)<(0.5*width_tmp*height_tmp))
                                        {
                                           printf("warning=========\n");
                                           x2 = (x2+resultlist1.pResult[Res1Index_1].Target.right)/2;
                                           y2 = (y2+resultlist1.pResult[Res1Index_1].Target.bottom)/2 ;
                                        }
					int width = x2 - x;
					int height = y2 - y;
                                            
//                                        cout << "x "<< x << endl;
//                                        cout << "y "<< y << endl;
//                                        cout << "x2_tmp "<< x2_tmp << endl;
//                                        cout << "y2_tmp "<< y2_tmp << endl;
//                                        cout << "x2 "<< x2 << endl;
//                                        cout << "y2 "<< y2 << endl;
//					height = height*0.9;
					char* name = "hxq_gjbs";

					(*BreakNum)++;
					fprintf(fpcsv3, "%d,../../TESTA/%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum,fname, exts, name, MaxScore_1, x, y, x2, y2);
#if PRINT
					sprintf(text, "%s %0.3f", name, MaxScore_1);
					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
					cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
                                
                                        
				}

				//油封破损
				if ((ResLabel == 5) && (flag_oil == 0))
				{
					// 取出第二步结果中 油封破损标签中得分最高的标签及索引
					//只取出第二步中 所有标签 合并
					float MaxScore_2 = 0.0;//油封破损最高得分
					int Res1Index_2 = 0; //油封破损第一步索引号
					int Res2Index_2 = 0;//油封破损第二步索引号

					for (int k = 0; k < Defectlist[s].lResultNum; k++)
					{
						if (Defectlist[s].pResult[k].flag == 5)
						{
							float BiaoScore = Defectlist[s].pResult[k].dVal;
							if (BiaoScore > MaxScore_2)
							{
								MaxScore_2 = BiaoScore;
								Res1Index_2 = s;
								Res2Index_2 = k;
							}
						}

					}

					//获取第一步框子
					int x = resultlist1.pResult[Res1Index_2].Target.left;
					int y = resultlist1.pResult[Res1Index_2].Target.top;
					int x2 = resultlist1.pResult[Res1Index_2].Target.right;
					int y2 = resultlist1.pResult[Res1Index_2].Target.bottom;

					int width = x2 - x;
					int height = y2 - y;
					char* name = "hxq_yfps";
					(*BreakNum)++;

					fprintf(fpcsv4, "%d,../../TESTA/%s%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, exts, name, MaxScore_2, x, y, x2, y2);
#if PRINT
					sprintf(text, "%s %0.3f", name, MaxScore_2);
					cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
					cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif

				}

				continue;
			}
		}
	}

#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv1),fclose(fpcsv2),fclose(fpcsv3),fclose(fpcsv4);
	input.release();
	free(resultlist1.pResult);

	if (Defectlist->pResult)
	{
		free(Defectlist->pResult);
		Defectlist->pResult = NULL;
	}

}

//-----------------------------------------------------------//
//@ 功能： 基础类型测函数模块 YOLO框架
void Detection_jyz(void *handle, const char* picpath, char *savePath, char *fname, char *exts, int *BreakNum)
{
	FILE* fpcsv;
	char name[256] = "jyz_pl";
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}
	//fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle, &img, &resultlist);

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
		//fprintf(fpcsv, "%s%s,%s,%.4f,%d,%d,%d,%d\n", fname, exts, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;

		//面积判定
//		int min_wh = min(width, height);
//		int mianji = width*height;
//		int tupianmianji = img.lHeight*img.lWidth;
//		if (((mianji*1.0 / tupianmianji) > 0.5) || (((mianji*1.0 / tupianmianji) < 0.1)))
//			continue;


		if (resultlist.pResult[i].dVal == 0)
		{
                        (*BreakNum)++;
			fprintf(fpcsv, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "jyz_pl", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
                        (*BreakNum)++;
			fprintf(fpcsv, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "jyz_pl", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
		}
/*
#if 0:
		else if (resultlist.pResult[i].dVal == 2)
		{
			//fprintf(fpcsv, "%s%s,%s,%.4f,%d,%d,%d,%d\n", fname, exts, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "tjzc", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
		}
#endif
*/
		/******************************************print********************************************/
	}
#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}
//-----------------------------------------------------------//
//@ 功能： 杆塔类型测函数模块 YOLO框架
void Detection_ganta(void *handle, const char* picpath, char *savePath, char *fname, char *exts)
{
	FILE* fpcsv;
	char name[256] = "ganta";
	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}

	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle, &img, &resultlist);

	fpcsv = fopen(savePath, "at+");
	if (NULL == fpcsv)
	{
		printf("can not open file %s!\n", savePath);
		exit(-1);
	}

	//fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		int min_wh = min(width, height);
		int mianji = width*height;
		int tupianmianji = img.lHeight*img.lWidth;
		if (((mianji*1.0 / tupianmianji) > 0.4) || ((width*1.0 / img.lWidth)>0.3) || ((height*1.0 / img.lHeight)>0.3) || (min_wh<100))
			continue;

		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
		if (resultlist.pResult[i].dVal == 0)
		{
			fprintf(fpcsv, "%s,%s,%.4f,%d,%d,%d,%d\n", fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "nest", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
		}
		


		/******************************************print********************************************/
	}
#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}
//-----------------------------------------------------------//
//@ 功能： 异物类型测函数模块 YOLO框架
void Detection_yiwu(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum)
{

	char name[256] = "yw_gkxfw";
	char name2[256] = "yw_nc";
	FILE *fpcsv1,*fpcsv2;
	fpcsv1 = fopen(savePath1, "at+");
	fpcsv2 = fopen(savePath2, "at+");
	if ((NULL == fpcsv1)|| (NULL == fpcsv2))
	{
		printf("can not open file %s!\n", savePath1);
		exit(-1);
	}


	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}

	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle, &img, &resultlist);



	//fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);



		if (resultlist.pResult[i].dVal == 0)
		{

                        (*BreakNum)++;
			fprintf(fpcsv1, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "yw_gkxfw", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
                        (*BreakNum)++;
			fprintf(fpcsv2, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name2, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "yw_nc", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
		}



		/******************************************print********************************************/
	}
#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv1),fclose(fpcsv2);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}


//-----------------------------------------------------------//
//@ 功能： 锈蚀类型测函数模块 YOLO框架
void Detection_xiushi(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum)
{

	char name[256] = "yw_gkxfw";
	char name2[256] = "yw_nc";
	FILE *fpcsv1, *fpcsv2;
	fpcsv1 = fopen(savePath1, "at+");
	fpcsv2 = fopen(savePath2, "at+");
	if ((NULL == fpcsv1) || (NULL == fpcsv2))
	{
		printf("can not open file %s!\n", savePath1);
		exit(-1);
	}


	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}

	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle, &img, &resultlist);



	//fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);



		if (resultlist.pResult[i].dVal == 0)
		{

			(*BreakNum)++;
			fprintf(fpcsv1, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "yw_gkxfw", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
			(*BreakNum)++;
			fprintf(fpcsv2, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name2, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "yw_nc", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
		}



		/******************************************print********************************************/
	}
#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv1), fclose(fpcsv2);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}


//-----------------------------------------------------------//
//@ 功能： 油污类型测函数模块 YOLO框架
void Detection_youwu(void *handle, const char* picpath, char *savePath1, char *savePath2, char *fname, char *exts, int *BreakNum)
{

	char name[256] = "yw_gkxfw";
	char name2[256] = "yw_nc";
	FILE *fpcsv1, *fpcsv2;
	fpcsv1 = fopen(savePath1, "at+");
	fpcsv2 = fopen(savePath2, "at+");
	if ((NULL == fpcsv1) || (NULL == fpcsv2))
	{
		printf("can not open file %s!\n", savePath1);
		exit(-1);
	}


	HYYOLOV3RESULT_LIST  resultlist = { 0 };
	resultlist.pResult = (HYYOLOV3_RESULT*)malloc(20 * sizeof(HYYOLOV3_RESULT));
	cv::Mat input = cv::imread(picpath);
	if (input.empty())
	{
		std::cout << " load  image error  !!!" << std::endl;
		//system("pause");
		exit(-1);
	}

	cv::Mat& scale_im = input;

	YOLOV3_IMAGES img;
	img.lHeight = scale_im.rows;
	img.lWidth = scale_im.cols;
	img.pixelArray.chunky.lLineBytes = scale_im.step;
	img.pixelArray.chunky.pPixel = scale_im.data;
	img.lPixelArrayFormat = 3;

	detec_gpu(handle, &img, &resultlist);



	//fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	cv::Mat resultImage;
	resultImage = input.clone();
	for (int i = 0; i < resultlist.lResultNum; i++)
	{
		/******************************************print********************************************/
		char text[20] = { 0 };
		int width = resultlist.pResult[i].Target.right - resultlist.pResult[i].Target.left;
		int height = resultlist.pResult[i].Target.bottom - resultlist.pResult[i].Target.top;
		int x = resultlist.pResult[i].Target.left;
		int y = resultlist.pResult[i].Target.top;
		//cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);



		if (resultlist.pResult[i].dVal == 0)
		{

			(*BreakNum)++;
			fprintf(fpcsv1, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "sly_bjbmyw", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);//R
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 0, 255), 6);
#endif
		}
		else if (resultlist.pResult[i].dVal == 1)
		{
			(*BreakNum)++;
			fprintf(fpcsv2, "%d,../../TESTA/%s,%s,%.4f,%d,%d,%d,%d\n", *BreakNum, fname, name2, resultlist.pResult[i].dConfidence, resultlist.pResult[i].Target.left, resultlist.pResult[i].Target.top, resultlist.pResult[i].Target.right, resultlist.pResult[i].Target.bottom);
#if PRINT
			sprintf(text, "%s %0.3f", "sly_dmyw", resultlist.pResult[i].dConfidence);
			cv::rectangle(resultImage, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 255), 4);
			cv::putText(resultImage, text, cv::Point(x, y + 30), CV_FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0, 255, 0), 6);
#endif
		}



		/******************************************print********************************************/
	}
#if PRINT
	char outputFile[512];
	sprintf(outputFile, "../../result/res_%s", fname);
	cv::imwrite(outputFile, resultImage);
#endif
	fclose(fpcsv1), fclose(fpcsv2);
	input.release();
	if (resultlist.pResult)
	{
		free(resultlist.pResult);
		resultlist.pResult = NULL;
	}

}


//=========================6种算法模块外部接口函数===========================//
//基础类检测函数
int jyzModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList)
{       
	int BreakNum = 0;
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[1024];//照片名称
	char ext[1024];//后缀名

	float thresh = 0.25;  //0.62
	int gpu_index = 0;
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//cout << "lines = " << lines << endl;

	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle, cfgFile, weightFile, thresh, gpu_index);

	if (yolo_handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);
	FILE* fpcsv;
	fpcsv = fopen(savePath, "w");
	fprintf(fpcsv, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fclose(fpcsv);


	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		//detectM[i] ?=1 to detect algorithm
		imgnum++;

		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos+1));
		char fname[512];
		strcpy(fname,s.c_str());

		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_jyz(yolo_handle, strArray[i].c_str(), savePath, fname, ext, &BreakNum); //【3】：调用检测函数

		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle);

}

//变电破损类检测函数
int breakModelDetection(void *yolo_handle1, void *yolo_handle2, char *cfgFile1, char *weightFile1, char *cfgFile2, char *weightFile2, char *savePath, float score, char* ImgList)
{
	int BreakNum = 0;//总的缺陷数
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[1024];//照片名称
	char ext[1024];//后缀名

	float thresh = 0.30;  //0.62
	int gpu_index = 0;
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle1, cfgFile1, weightFile1, thresh, gpu_index);//第一个模型初始化
	init_gpu(&yolo_handle2, cfgFile2, weightFile2, thresh, gpu_index);//第二个模型初始化

	if (yolo_handle1 == 0 || yolo_handle2 == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;

        char* savePath1 = "../../schy_bj_bjps_result.csv";
        char* savePath2 = "../../schy_bj_bpmh_result.csv";
        char* savePath3 = "../../schy_hxq_gjbs_result.csv";
        char* savePath4 = "../../schy_hxq_yfps_result.csv";
    
	FILE *fpcsv1,*fpcsv2,*fpcsv3,*fpcsv4;
	fpcsv1 = fopen(savePath1, "w");
	fprintf(fpcsv1, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv2 = fopen(savePath2, "w");
	fprintf(fpcsv2, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv3 = fopen(savePath3, "w");
	fprintf(fpcsv3, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv4 = fopen(savePath4, "w");
	fprintf(fpcsv4, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fclose(fpcsv1),fclose(fpcsv2),fclose(fpcsv3),fclose(fpcsv4);

	for (int i = 0; i < lines; i++)
	{
		imgnum++;

//		_splitpath(strArray[i].c_str(), NULL, NULL, fname, ext);


		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos+1));
		char fname[512];
		strcpy(fname,s.c_str());


		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_break(yolo_handle1, yolo_handle2,strArray[i].c_str(), savePath1, savePath2, savePath3, savePath4, fname, ext,&BreakNum); //【3】：调用检测函数
		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle1);
	uninit_gpu(yolo_handle2);
	return 0;
}
//附属设施类检测函数(跟小金具类似)

//杆塔类检测函数
int gantaModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList)
{
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[512];//照片名称
	char ext[512];//后缀名

	float thresh = 0.65;  //0.62
	int gpu_index = 0;
	//char savePath[1024] = "D:/result0830.csv";//argv[3]
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//cout << "lines = " << lines << endl;

	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle, cfgFile, weightFile, thresh, gpu_index);

	if (yolo_handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);
	FILE* fpcsv;
	fpcsv = fopen(savePath, "w");
	fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	fclose(fpcsv);


	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		//detectM[i] ?=1 to detect algorithm
		imgnum++;

		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos+1));
		char fname[512];
		strcpy(fname,s.c_str());

		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_ganta(yolo_handle, strArray[i].c_str(), savePath, fname, ext); //【3】：调用检测函数
		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));

	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle);

}

//异物类检测函数
int yiwuModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList)
{
    int BreakNum = 0;
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[512];//照片名称
	char ext[512];//后缀名

	float thresh = 0.24;  //0.62
	int gpu_index = 0;
	//char savePath[1024] = "D:/result0830.csv";//argv[3]
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//cout << "lines = " << lines << endl;

	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle, cfgFile, weightFile, thresh, gpu_index);

	if (yolo_handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);

        char* savePath1 = "../../result_yw_gkxfw.csv";
        char* savePath2 = "../../result_yw_nc.csv";


	FILE *fpcsv1,*fpcsv2;
	fpcsv1 = fopen(savePath1, "w");
	fprintf(fpcsv1, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv2 = fopen(savePath2, "w");
	fprintf(fpcsv2, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fclose(fpcsv1),fclose(fpcsv2);



	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		imgnum++;

		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos+1));
		char fname[512];
		strcpy(fname,s.c_str());

		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_yiwu(yolo_handle, strArray[i].c_str(), savePath1, savePath2, fname, ext, &BreakNum); //【3】：调用检测函数
		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));
	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle);

}

//锈蚀类检测函数
int xiushiModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList)
{
	int BreakNum = 0;
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[512];//照片名称
	char ext[512];//后缀名

	float thresh = 0.24;  //0.62
	int gpu_index = 0;
	//char savePath[1024] = "D:/result0830.csv";//argv[3]
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//cout << "lines = " << lines << endl;

	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle, cfgFile, weightFile, thresh, gpu_index);

	if (yolo_handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);

	char* savePath1 = "../../result_yw_gkxfw.csv";
	char* savePath2 = "../../result_yw_nc.csv";


	FILE *fpcsv1, *fpcsv2;
	fpcsv1 = fopen(savePath1, "w");
	fprintf(fpcsv1, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv2 = fopen(savePath2, "w");
	fprintf(fpcsv2, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fclose(fpcsv1), fclose(fpcsv2);



	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		imgnum++;

		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos + 1));
		char fname[512];
		strcpy(fname, s.c_str());

		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_xiushi(yolo_handle, strArray[i].c_str(), savePath1, savePath2, fname, ext, &BreakNum); //【3】：调用检测函数
		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));
	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle);

}

//油污类检测函数
int youwuModelDetection(void *yolo_handle, char *cfgFile, char *weightFile, char *savePath, float score, char* ImgList)
{
	int BreakNum = 0;
	int lines = 0;//行总数
	int imgnum = 0;//照片索引号
	char fname[512];//照片名称
	char ext[512];//后缀名

	float thresh = 0.24;  //0.62
	int gpu_index = 0;
	//char savePath[1024] = "D:/result0830.csv";//argv[3]
	std::vector<string> strArray;//图片路径

	strArray = ListPreprocess(ImgList, &lines, savePath);

	//cout << "lines = " << lines << endl;
	//cout<<"image name"<<endl;
	//cout<<strArray<<endl;
	
	//---------初始化网络模型----------------------
	init_gpu(&yolo_handle, cfgFile, weightFile, thresh, gpu_index);

	if (yolo_handle == 0)
	{
		std::cout << "model loading failed ... " << std::endl;
		return -1;
	}
	printf("Loading Finished\n");


	//---------图片列表检测----------------------
	//time_t t1 = 0, t2 = 0;
	//printf("LOOP= %ld\n", LOOP);

	char* savePath1 = "../../result_yw_gkxfw.csv";
	char* savePath2 = "../../result_yw_nc.csv";


	FILE *fpcsv1, *fpcsv2;
	fpcsv1 = fopen(savePath1, "w");
	fprintf(fpcsv1, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fpcsv2 = fopen(savePath2, "w");
	fprintf(fpcsv2, "ID,PATH,TYPE,SCORE,XMIN,YMIN,XMAX,YMAX\n");
	fclose(fpcsv1), fclose(fpcsv2);



	imgnum = 0;
	for (int i = 0; i < lines; i++)
	{
		imgnum++;

		std::string path;
		path = strArray[i];

		int pos = path.find_last_of('/');
		string s(path.substr(pos + 1));
		char fname[512];
		strcpy(fname, s.c_str());

		printf("%d/%d %s\n", imgnum, lines, fname);//显示处理进度
		//t1 = clock();
		Detection_youwu(yolo_handle, strArray[i].c_str(), savePath1, savePath2, fname, ext, &BreakNum); //【3】：调用检测函数
		//t2 = clock();
		//printf("all time(ms): %d\n", ((t2 - t1) * 1000 / CLOCKS_PER_SEC));
	}



	//---------释放网络及资源----------------------

	std::vector<string>().swap(strArray);
	uninit_gpu(yolo_handle);

}


//---------------------------------------------------------//

 //---------------------------------------------------------//
 //@功能： 文件list 预处理函数
 //返回：文件数组
 std::vector<string> ListPreprocess(char* imagelist, int* lines,char* savePath)
 {
	 //---------文件List 预处理----------------------
	 //int lines = 0;//行总数
	
	 //FILE* fpcsv;
	 //fpcsv = fopen(savePath, "w");
	 //fprintf(fpcsv, "filename,name,score,xmin,ymin,xmax,ymax\n");
	 //fclose(fpcsv);

	 std::vector<string> strArray;//照片路径集合
	 string str;
	 strArray.clear();


	 ifstream  readFile(imagelist);
	 if (!readFile)
	 {
		 std::cout << "can not open list file!" << std::endl;
		 
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
			 ++(*lines);
			 strArray.push_back(str);
		 }
	 }

	 readFile.close();

	 return strArray;
 }
 //---------------------------------------------------------//



 //---------------------------------------------------------//
 //@ 功能： 得到 cmd指令 字符
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
 //---------------------------------------------------------//
