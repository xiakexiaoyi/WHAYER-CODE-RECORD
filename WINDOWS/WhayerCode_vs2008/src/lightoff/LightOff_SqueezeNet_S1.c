#include "flatnn.h"
#include "LightOff_SqueezeNet_S1.h"
#include <math.h>
#include<time.h>

#ifndef CHECK_PTR 
#define CHECK_PTR(ptr) if(!ptr) {res=MERR_NO_MEMORY; goto exit; } 
#endif
#ifndef CHECK_RES 
#define CHECK_RES(res) if(res!=MOK) {goto exit;} 
#endif

int LightOff_SqueezeNet_S1_run_C3(unsigned char* bgr, int iw, int ih, int widthStep,  int* label)
{
	int res = 0;
	int n = 0;
	long start1,end1;
	int LightOff_SqueezeNet_S1_data_ln = 0;
	int LightOff_SqueezeNet_S1_data_lw = 0;
	int LightOff_SqueezeNet_S1_data_lh = 0;
	int LightOff_SqueezeNet_S1_conv1_ln = 0;
	int LightOff_SqueezeNet_S1_conv1_lw = 0;
	int LightOff_SqueezeNet_S1_conv1_lh = 0;
	int LightOff_SqueezeNet_S1_pool1_lw = 0;
	int LightOff_SqueezeNet_S1_pool1_lh = 0;
	int LightOff_SqueezeNet_S1_pool1_ln = 0;
	int LightOff_SqueezeNet_S1_fire2_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh = 0;	
	int LightOff_SqueezeNet_S1_fire2_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire2_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire2_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire2_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire2_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire2_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fp_fire2_concat_ = 0;
	int LightOff_SqueezeNet_S1_fire2_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire2_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire2_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire3_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh = 0;	
	int LightOff_SqueezeNet_S1_fire3_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire3_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire3_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire4_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire4_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire4_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire5_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire5_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire5_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire6_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire6_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire6_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire7_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire7_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire7_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire8_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire8_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire8_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_fire9_expand1x1_ln = 0;
	int LightOff_SqueezeNet_S1_fire9_expand1x1_lw = 0;
	int LightOff_SqueezeNet_S1_fire9_expand1x1_lh = 0;
	int LightOff_SqueezeNet_S1_conv26_lw=0;
	int LightOff_SqueezeNet_S1_conv26_lh=0;
	int LightOff_SqueezeNet_S1_conv26_ln=0;
	int LightOff_SqueezeNet_S1_fire3_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire3_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire3_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire4_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire4_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire4_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire5_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire5_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire5_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire6_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire6_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire6_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire7_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire7_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire7_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire8_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire8_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire8_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fire9_expand3x3_ln = 0;
	int LightOff_SqueezeNet_S1_fire9_expand3x3_lw = 0;
	int LightOff_SqueezeNet_S1_fire9_expand3x3_lh = 0;
	int LightOff_SqueezeNet_S1_fp_fire3_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire4_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire5_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire6_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire7_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire8_concat_ = 0;
	int LightOff_SqueezeNet_S1_fp_fire9_concat_ = 0;
	int LightOff_SqueezeNet_S1_fire3_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire3_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire3_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire4_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire4_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire4_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire5_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire5_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire5_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire6_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire6_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire6_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire7_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire7_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire7_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire8_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire8_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire8_concat_ln = 0;
	int LightOff_SqueezeNet_S1_fire9_concat_lw = 0;
	int LightOff_SqueezeNet_S1_fire9_concat_lh = 0;
	int LightOff_SqueezeNet_S1_fire9_concat_ln = 0;
	int LightOff_SqueezeNet_S1_pool3_lw = 0;
	int LightOff_SqueezeNet_S1_pool3_lh = 0;
	int LightOff_SqueezeNet_S1_pool3_ln = 0;
	int LightOff_SqueezeNet_S1_pool4_lw = 0;
	int LightOff_SqueezeNet_S1_pool4_lh = 0;
	int LightOff_SqueezeNet_S1_pool4_ln = 0;
	int LightOff_SqueezeNet_S1_pool2_lw = 0;
	int LightOff_SqueezeNet_S1_pool2_lh = 0;
	int LightOff_SqueezeNet_S1_pool2_ln = 0;
	int LightOff_SqueezeNet_S1_rpn_cls_prob_lw = 0;
	int LightOff_SqueezeNet_S1_rpn_cls_prob_lh = 0;
	int LightOff_SqueezeNet_S1_rpn_cls_prob_ln = 0;

	float* LightOff_SqueezeNet_S1_b_data_ = 0;
	float* LightOff_SqueezeNet_S1_b_conv1_ = 0;
	float* LightOff_SqueezeNet_S1_b_pool1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_ = 0;	
	float* LightOff_SqueezeNet_S1_b_fire2_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire2_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire2_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_ = 0;	
	float* LightOff_SqueezeNet_S1_b_fire3_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire4_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire5_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire6_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire7_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire8_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire9_expand1x1_ = 0;
	float* LightOff_SqueezeNet_S1_b_conv26_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire3_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire4_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire5_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire6_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire7_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire8_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire9_expand3x3_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire3_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire4_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire5_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire6_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire7_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire8_concat_ = 0;
	float* LightOff_SqueezeNet_S1_b_fire9_concat_ = 0;    
	float* LightOff_SqueezeNet_S1_b_pool2_ = 0;
	float* LightOff_SqueezeNet_S1_b_pool3_ = 0;
	float* LightOff_SqueezeNet_S1_b_pool4_ = 0;
	float* LightOff_SqueezeNet_S1_b_rpn_cls_prob_=0;

	extern float LightOff_SqueezeNet_S1_mean_[];
	extern float LightOff_SqueezeNet_S1_kw_data_[];
	extern float LightOff_SqueezeNet_S1_kb_data_[];
	extern float LightOff_SqueezeNet_S1_kw_fire2_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire2_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire2_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire2_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire2_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire2_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire3_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire3_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire3_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire3_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire3_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire3_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire4_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire4_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire4_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire4_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire4_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire4_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire5_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire5_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire5_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire5_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire5_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire5_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire6_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire6_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire6_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire6_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire6_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire6_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire7_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire7_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire7_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire7_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire7_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire8_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire8_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire8_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire8_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire8_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_fire9_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire9_squeeze1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire9_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kb_fire9_expand1x1_[];
	extern float LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kb_fire9_expand3x3_[];
	extern float LightOff_SqueezeNet_S1_kw_conv26_[];
	extern float LightOff_SqueezeNet_S1_kb_conv26_[];
	
	/*FILE *fire7_kw_expand3x3;
	FILE *fire8_kw_expand3x3;
	FILE *fire9_kw_expand3x3;

    FILE *test;

	

	float *LightOff_SqueezeNet_S1_kw_fire7_expand3x3_ = (float *)malloc(82944*sizeof(float));
	float *LightOff_SqueezeNet_S1_kw_fire8_expand3x3_ = (float *)malloc(147456*sizeof(float));
	float *LightOff_SqueezeNet_S1_kw_fire9_expand3x3_ = (float *)malloc(147456*sizeof(float));
	*/
		
	//fire7_kw_expand3x3=fopen("d://squeeze//fire7_expand3x3_weight.txt","rb");
 //   if (NULL==fire7_kw_expand3x3) 
	//{
 //       printf("Can not open file Data.txt!\n");
 //       return -1;
 //   }	
	//fread(LightOff_SqueezeNet_S1_kw_fire7_expand3x3_,82944,sizeof(float),fire7_kw_expand3x3);
	//printf("f7 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[82943]);
	//fclose(fire7_kw_expand3x3);

	//fire8_kw_expand3x3=fopen("d://squeeze//fire8_expand3x3_weight.txt","rb");
 //   if (NULL==fire8_kw_expand3x3) 
	//{
 //       printf("Can not open file Data.txt!\n");
 //       return -1;
 //   }	
	//fread(LightOff_SqueezeNet_S1_kw_fire8_expand3x3_,147456,sizeof(float),fire8_kw_expand3x3);
	//fclose(fire8_kw_expand3x3);
	//printf("f8 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[147455]);

	//fire9_kw_expand3x3=fopen("d://squeeze//fire9_expand3x3_weight.txt","rb");

 //   if (NULL==fire9_kw_expand3x3) 
	//{
 //       printf("Can not open file Data.txt!\n");
 //       return -1;
 //   }	
	//fread(LightOff_SqueezeNet_S1_kw_fire9_expand3x3_,147456,sizeof(float),fire9_kw_expand3x3);
	//fclose(fire9_kw_expand3x3);
#ifdef PRINTF
	printf("f9 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[147455]);
#endif	
	start1=clock();
	LightOff_SqueezeNet_S1_data_ln = 1;
	LightOff_SqueezeNet_S1_data_lw = iw;// 
	LightOff_SqueezeNet_S1_data_lh = ih;//
	LightOff_SqueezeNet_S1_b_data_ = (float*) malloc(iw * ih *3* sizeof(float));//
	CHECK_PTR(LightOff_SqueezeNet_S1_b_data_);
	res = ly_data_const_mean_C3(bgr, iw, ih, widthStep,LightOff_SqueezeNet_S1_mean_,LightOff_SqueezeNet_S1_b_data_);
	CHECK_RES(res);
	//conv1
	LightOff_SqueezeNet_S1_conv1_lw = (LightOff_SqueezeNet_S1_data_lw + 2*0-7)/2 + 1;
	LightOff_SqueezeNet_S1_conv1_lh = (LightOff_SqueezeNet_S1_data_lh + 2*0-7)/2 + 1;
	LightOff_SqueezeNet_S1_conv1_ln = LightOff_SqueezeNet_S1_data_ln;
	LightOff_SqueezeNet_S1_b_conv1_ = (float*) malloc(LightOff_SqueezeNet_S1_data_ln*96*LightOff_SqueezeNet_S1_conv1_lw*LightOff_SqueezeNet_S1_conv1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_conv1_);
	for(n=0;n<LightOff_SqueezeNet_S1_data_ln;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_data_+n*3*LightOff_SqueezeNet_S1_data_lw*LightOff_SqueezeNet_S1_data_lh,
			LightOff_SqueezeNet_S1_data_lw,LightOff_SqueezeNet_S1_data_lh,3,1,LightOff_SqueezeNet_S1_kw_data_,LightOff_SqueezeNet_S1_kb_data_,
			7,7,0,0,2,2,LightOff_SqueezeNet_S1_b_conv1_+n*96*LightOff_SqueezeNet_S1_conv1_lw*LightOff_SqueezeNet_S1_conv1_lh,96);
		CHECK_RES(res);
	}
	free(LightOff_SqueezeNet_S1_b_data_);
	LightOff_SqueezeNet_S1_b_data_=0;
#ifdef PRINTF
	printf("conv1 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_conv1_[0],LightOff_SqueezeNet_S1_b_conv1_[1],LightOff_SqueezeNet_S1_b_conv1_[2]);
#endif
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_conv1_,LightOff_SqueezeNet_S1_b_conv1_,LightOff_SqueezeNet_S1_conv1_ln*LightOff_SqueezeNet_S1_conv1_lw*LightOff_SqueezeNet_S1_conv1_lh*96);
	CHECK_RES(res);
#ifdef PRINTF
	printf("conv1relu %f,%f,%f\n",LightOff_SqueezeNet_S1_b_conv1_[111*111*96-1],LightOff_SqueezeNet_S1_b_conv1_[111*111*96-2],LightOff_SqueezeNet_S1_b_conv1_[2]);
#endif
	//max_pool
	LightOff_SqueezeNet_S1_pool1_lh = (int)(ceil((float)(LightOff_SqueezeNet_S1_conv1_lh + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool1_lw = (int)(ceil((float)(LightOff_SqueezeNet_S1_conv1_lw + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool1_ln = LightOff_SqueezeNet_S1_conv1_ln;
	LightOff_SqueezeNet_S1_b_pool1_ = (float*) malloc(LightOff_SqueezeNet_S1_pool1_ln*96*LightOff_SqueezeNet_S1_pool1_lw*LightOff_SqueezeNet_S1_pool1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_pool1_);
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_max_float(LightOff_SqueezeNet_S1_b_conv1_+n*96*LightOff_SqueezeNet_S1_conv1_lw*LightOff_SqueezeNet_S1_conv1_lh,LightOff_SqueezeNet_S1_conv1_lw,LightOff_SqueezeNet_S1_conv1_lh,96,LightOff_SqueezeNet_S1_b_pool1_+n*96*LightOff_SqueezeNet_S1_pool1_lw*LightOff_SqueezeNet_S1_pool1_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	free(LightOff_SqueezeNet_S1_b_conv1_);
	LightOff_SqueezeNet_S1_b_conv1_=0;
#ifdef PRINTF
	printf("pool %f,%f,%f\n",LightOff_SqueezeNet_S1_b_pool1_[0],LightOff_SqueezeNet_S1_b_pool1_[1],LightOff_SqueezeNet_S1_b_pool1_[2]);
#endif
	//conv2
	LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw = (LightOff_SqueezeNet_S1_pool1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh = (LightOff_SqueezeNet_S1_pool1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_squeeze1x1_ln = LightOff_SqueezeNet_S1_pool1_ln;
	LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_pool1_ln*16*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_pool1_+n*96*LightOff_SqueezeNet_S1_pool1_lw*LightOff_SqueezeNet_S1_pool1_lh,
			LightOff_SqueezeNet_S1_pool1_lw,LightOff_SqueezeNet_S1_pool1_lh,96,1,LightOff_SqueezeNet_S1_kw_fire2_squeeze1x1_,LightOff_SqueezeNet_S1_kb_fire2_squeeze1x1_,
			1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh,16);
		CHECK_RES(res);
	}
	free(LightOff_SqueezeNet_S1_b_pool1_);
	LightOff_SqueezeNet_S1_b_pool1_=0;
#ifdef PRINTF
	printf("conv2 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_[2]);
#endif	
	//relu
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_,LightOff_SqueezeNet_S1_fire2_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh*16);
	CHECK_RES(res);
	//conv3
#ifdef PRINTF
	printf("fire2_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire2_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire2_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire2_squeeze1x1_[1535]);
#endif
	LightOff_SqueezeNet_S1_fire2_expand1x1_lw = (LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_expand1x1_lh = (LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_expand1x1_ln = LightOff_SqueezeNet_S1_fire2_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_0_ = LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire2_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire2_expand1x1_ln*64*LightOff_SqueezeNet_S1_fire2_expand1x1_lw*LightOff_SqueezeNet_S1_fire2_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire2_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh,LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh,16,1,LightOff_SqueezeNet_S1_kw_fire2_expand1x1_,LightOff_SqueezeNet_S1_kb_fire2_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire2_expand1x1_+n*64*LightOff_SqueezeNet_S1_fire2_expand1x1_lw*LightOff_SqueezeNet_S1_fire2_expand1x1_lh,64);
		CHECK_RES(res);
	}
#ifdef PRINTF	
	printf("conv3 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire2_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire2_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire2_expand1x1_[2]);
#endif	
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire2_expand1x1_,LightOff_SqueezeNet_S1_b_fire2_expand1x1_,LightOff_SqueezeNet_S1_fire2_expand1x1_ln*LightOff_SqueezeNet_S1_fire2_expand1x1_lw*LightOff_SqueezeNet_S1_fire2_expand1x1_lh*64);
	CHECK_RES(res);
#ifdef PRINTF
	printf("fire2_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire2_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire2_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire2_expand1x1_[1023]);
#endif	
	//conv4
	LightOff_SqueezeNet_S1_fire2_expand3x3_lw = (LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_expand3x3_lh = (LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire2_expand3x3_ln = LightOff_SqueezeNet_S1_fire2_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire2_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire2_expand3x3_ln*64*LightOff_SqueezeNet_S1_fire2_expand3x3_lw*LightOff_SqueezeNet_S1_fire2_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire2_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire2_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire2_squeeze1x1_lh,16,1,
			LightOff_SqueezeNet_S1_kw_fire2_expand3x3_,LightOff_SqueezeNet_S1_kb_fire2_expand3x3_,3,3,1,1,1,1,
			LightOff_SqueezeNet_S1_b_fire2_expand3x3_+n*64*LightOff_SqueezeNet_S1_fire2_expand3x3_lw*LightOff_SqueezeNet_S1_fire2_expand3x3_lh,64);
		CHECK_RES(res);
	}
#ifdef PRINTF
	printf("conv4 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire2_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire2_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire2_expand3x3_[2]);
#endif	
	free(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_);
	LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_=0;
#ifdef PRINTF	
	printf("fire2_expand3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire2_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire2_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire2_expand3x3_[9215]);
#endif		
	//LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire2_expand3x3_,LightOff_SqueezeNet_S1_b_fire2_expand3x3_,LightOff_SqueezeNet_S1_fire2_expand3x3_ln*LightOff_SqueezeNet_S1_fire2_expand3x3_lw*LightOff_SqueezeNet_S1_fire2_expand3x3_lh*64);
	CHECK_RES(res);
	
	//concat1
	LightOff_SqueezeNet_S1_fire2_concat_ln = LightOff_SqueezeNet_S1_fire2_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire2_concat_lw = LightOff_SqueezeNet_S1_fire2_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire2_concat_lh = LightOff_SqueezeNet_S1_fire2_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire2_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire2_concat_lw*LightOff_SqueezeNet_S1_fire2_concat_lh*LightOff_SqueezeNet_S1_fire2_concat_ln*128* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire2_concat_);
#ifdef PRINTF	
	printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire2_concat_lw,LightOff_SqueezeNet_S1_fire2_concat_lh);
#endif	
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire2_concat_ = 0;
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire2_expand1x1_+n*64*LightOff_SqueezeNet_S1_fire2_expand1x1_lw*LightOff_SqueezeNet_S1_fire2_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire2_expand1x1_lw,LightOff_SqueezeNet_S1_fire2_expand1x1_lh,64,
			LightOff_SqueezeNet_S1_b_fire2_concat_ + n * LightOff_SqueezeNet_S1_fire2_concat_lw*LightOff_SqueezeNet_S1_fire2_concat_lh*128,
			&LightOff_SqueezeNet_S1_fp_fire2_concat_);
		CHECK_RES(res);
	#ifdef PRINTF
		printf("concat1 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire2_concat_[0],LightOff_SqueezeNet_S1_b_fire2_concat_[1],LightOff_SqueezeNet_S1_b_fire2_concat_[2]);
	#endif
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire2_expand3x3_+n*64*LightOff_SqueezeNet_S1_fire2_expand3x3_lw*LightOff_SqueezeNet_S1_fire2_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire2_expand3x3_lw,LightOff_SqueezeNet_S1_fire2_expand3x3_lh,64,
			LightOff_SqueezeNet_S1_b_fire2_concat_ + n * LightOff_SqueezeNet_S1_fire2_concat_lw*LightOff_SqueezeNet_S1_fire2_concat_lh*128,
			&LightOff_SqueezeNet_S1_fp_fire2_concat_);
		CHECK_RES(res);
	}
#ifdef PRINTF
	printf("concat3 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire2_concat_[0],LightOff_SqueezeNet_S1_b_fire2_concat_[1],LightOff_SqueezeNet_S1_b_fire2_concat_[2]);
	printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire2_expand3x3_lw,LightOff_SqueezeNet_S1_fire2_expand3x3_lh);
#endif	
	free(LightOff_SqueezeNet_S1_b_fire2_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire2_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire2_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire2_expand3x3_=0;
	//fire2 
	//conv5
	LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw = (LightOff_SqueezeNet_S1_fire2_concat_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh = (LightOff_SqueezeNet_S1_fire2_concat_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_squeeze1x1_ln = LightOff_SqueezeNet_S1_fire2_concat_ln;
	LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire2_concat_ln*16*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_);
#ifdef PRINTF
	printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh);
#endif		
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire2_concat_+n*128*LightOff_SqueezeNet_S1_fire2_concat_lw*LightOff_SqueezeNet_S1_fire2_concat_lh,
			LightOff_SqueezeNet_S1_fire2_concat_lw,LightOff_SqueezeNet_S1_fire2_concat_lh,128,1,LightOff_SqueezeNet_S1_kw_fire3_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire3_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh,16);
		CHECK_RES(res);
	}
//	printf("conv5 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_fire2_concat_);
	LightOff_SqueezeNet_S1_b_fire2_concat_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_,LightOff_SqueezeNet_S1_fire3_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh*16);
	CHECK_RES(res);
	//printf("fire3_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire3_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire3_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire3_squeeze1x1_[2047]);
	//conv6
	LightOff_SqueezeNet_S1_fire3_expand1x1_lw = (LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_expand1x1_lh = (LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_expand1x1_ln = LightOff_SqueezeNet_S1_fire3_squeeze1x1_ln;
	
	LightOff_SqueezeNet_S1_b_fire3_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire3_expand1x1_ln*64*LightOff_SqueezeNet_S1_fire3_expand1x1_lw*LightOff_SqueezeNet_S1_fire3_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire3_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh,16,1,LightOff_SqueezeNet_S1_kw_fire3_expand1x1_,LightOff_SqueezeNet_S1_kb_fire3_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire3_expand1x1_+n*64*LightOff_SqueezeNet_S1_fire3_expand1x1_lw*LightOff_SqueezeNet_S1_fire3_expand1x1_lh,64);
		CHECK_RES(res);
	}	
//		printf("conv6 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire3_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire3_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire3_expand1x1_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire3_expand1x1_,LightOff_SqueezeNet_S1_b_fire3_expand1x1_,LightOff_SqueezeNet_S1_fire3_expand1x1_ln*LightOff_SqueezeNet_S1_fire3_expand1x1_lw*LightOff_SqueezeNet_S1_fire3_expand1x1_lh*64);
	CHECK_RES(res);
	//printf("fire3_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire3_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire3_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire3_expand1x1_[1023]);

	//conv7
	LightOff_SqueezeNet_S1_fire3_expand3x3_lw = (LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_expand3x3_lh = (LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire3_expand3x3_ln = LightOff_SqueezeNet_S1_fire3_squeeze1x1_ln;	
	LightOff_SqueezeNet_S1_b_fire3_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire3_expand3x3_ln*64*LightOff_SqueezeNet_S1_fire3_expand3x3_lw*LightOff_SqueezeNet_S1_fire3_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire3_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh,16,1,LightOff_SqueezeNet_S1_kw_fire3_expand3x3_,LightOff_SqueezeNet_S1_kb_fire3_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire3_expand3x3_+n*64*LightOff_SqueezeNet_S1_fire3_expand3x3_lw*LightOff_SqueezeNet_S1_fire3_expand3x3_lh,64);
		CHECK_RES(res);
	}
//	printf("conv7 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire3_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire3_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire3_expand3x3_[2]);
	free(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_);
	LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_=0;	
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire3_expand3x3_,LightOff_SqueezeNet_S1_b_fire3_expand3x3_,LightOff_SqueezeNet_S1_fire3_expand3x3_ln*LightOff_SqueezeNet_S1_fire3_expand3x3_lw*LightOff_SqueezeNet_S1_fire3_expand3x3_lh*64);
	CHECK_RES(res);
	//printf("fire3_expand3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire3_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire3_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire3_expand3x3_[9215]);
	
	//concat2
	LightOff_SqueezeNet_S1_fire3_concat_ln = LightOff_SqueezeNet_S1_fire3_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire3_concat_lw = LightOff_SqueezeNet_S1_fire3_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire3_concat_lh = LightOff_SqueezeNet_S1_fire3_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire3_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire3_concat_lw*LightOff_SqueezeNet_S1_fire3_concat_lh*LightOff_SqueezeNet_S1_fire3_concat_ln*128* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire3_concat_);
	//printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire3_concat_ = 0;
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire3_expand1x1_+n*64*LightOff_SqueezeNet_S1_fire3_expand1x1_lw*LightOff_SqueezeNet_S1_fire3_expand1x1_lh,LightOff_SqueezeNet_S1_fire3_expand1x1_lw,LightOff_SqueezeNet_S1_fire3_expand1x1_lh,64,LightOff_SqueezeNet_S1_b_fire3_concat_ + n * LightOff_SqueezeNet_S1_fire3_concat_lw*LightOff_SqueezeNet_S1_fire3_concat_lh*128,&LightOff_SqueezeNet_S1_fp_fire3_concat_);
		CHECK_RES(res);
	//	printf("concat1 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire3_concat_[0],LightOff_SqueezeNet_S1_b_fire3_concat_[1],LightOff_SqueezeNet_S1_b_fire3_concat_[2]);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire3_expand3x3_+n*64*LightOff_SqueezeNet_S1_fire3_expand3x3_lw*LightOff_SqueezeNet_S1_fire3_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire3_expand3x3_lw,LightOff_SqueezeNet_S1_fire3_expand3x3_lh,64,
			LightOff_SqueezeNet_S1_b_fire3_concat_ + n * LightOff_SqueezeNet_S1_fire3_concat_lw*LightOff_SqueezeNet_S1_fire3_concat_lh*128,
			&LightOff_SqueezeNet_S1_fp_fire3_concat_);
		CHECK_RES(res);
	}
	//printf("concat3 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire3_concat_[0],LightOff_SqueezeNet_S1_b_fire3_concat_[1],LightOff_SqueezeNet_S1_b_fire3_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire3_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire3_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire3_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire3_expand3x3_=0;		
	
	//conv8
	LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw = (LightOff_SqueezeNet_S1_fire3_concat_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh = (LightOff_SqueezeNet_S1_fire3_concat_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln = LightOff_SqueezeNet_S1_fire3_concat_ln;
	LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln*32*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_);
	//printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire3_concat_+n*128*LightOff_SqueezeNet_S1_fire3_concat_lw*LightOff_SqueezeNet_S1_fire3_concat_lh,
			LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh,128,1,LightOff_SqueezeNet_S1_kw_fire4_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire4_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh,32);
		CHECK_RES(res);
	}
	//printf("conv8 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_fire3_concat_);
	LightOff_SqueezeNet_S1_b_fire3_concat_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_,LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh*32);
	CHECK_RES(res);
	//printf("fire4_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire4_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire4_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire4_squeeze1x1_[4095]);
	
	//conv9
	
	LightOff_SqueezeNet_S1_fire4_expand1x1_lw = (LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_expand1x1_lh = (LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_expand1x1_ln = LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire4_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire4_expand1x1_ln*128*LightOff_SqueezeNet_S1_fire4_expand1x1_lw*LightOff_SqueezeNet_S1_fire4_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire4_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh,32,1,
			LightOff_SqueezeNet_S1_kw_fire4_expand1x1_,LightOff_SqueezeNet_S1_kb_fire4_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire4_expand1x1_+n*128*LightOff_SqueezeNet_S1_fire4_expand1x1_lw*LightOff_SqueezeNet_S1_fire4_expand1x1_lh,128);
		CHECK_RES(res);
	}
	//printf("conv9 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire4_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire4_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire4_expand1x1_[2]);
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire4_expand1x1_,LightOff_SqueezeNet_S1_b_fire4_expand1x1_,LightOff_SqueezeNet_S1_fire4_expand1x1_ln*LightOff_SqueezeNet_S1_fire4_expand1x1_lw*LightOff_SqueezeNet_S1_fire4_expand1x1_lh*128);
	CHECK_RES(res);
	//printf("fire4_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire4_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire4_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire4_expand1x1_[4095]);
	//conv10
	LightOff_SqueezeNet_S1_fire4_expand3x3_lw = (LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_expand3x3_lh = (LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire4_expand3x3_ln = LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire4_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire4_expand3x3_ln*128*LightOff_SqueezeNet_S1_fire4_expand3x3_lw*LightOff_SqueezeNet_S1_fire4_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire4_expand3x3_);
	for(n=0;n<LightOff_SqueezeNet_S1_fire4_squeeze1x1_ln;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire4_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire4_squeeze1x1_lh,32,1,LightOff_SqueezeNet_S1_kw_fire4_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire4_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire4_expand3x3_+n*128*LightOff_SqueezeNet_S1_fire4_expand3x3_lw*LightOff_SqueezeNet_S1_fire4_expand3x3_lh,128);
		CHECK_RES(res);
	}	
//	printf("conv10 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire4_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire4_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire4_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire4_expand3x3_,LightOff_SqueezeNet_S1_b_fire4_expand3x3_,LightOff_SqueezeNet_S1_fire4_expand3x3_ln*LightOff_SqueezeNet_S1_fire4_expand3x3_lw*LightOff_SqueezeNet_S1_fire4_expand3x3_lh*128);
	CHECK_RES(res);
	//printf("f4 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire4_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire4_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire4_expand3x3_[36863]);
	
	//concat3
	LightOff_SqueezeNet_S1_fire4_concat_ln = LightOff_SqueezeNet_S1_fire4_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire4_concat_lw = LightOff_SqueezeNet_S1_fire4_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire4_concat_lh = LightOff_SqueezeNet_S1_fire4_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire4_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire4_concat_lw*LightOff_SqueezeNet_S1_fire4_concat_lh*LightOff_SqueezeNet_S1_fire4_concat_ln*256* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire4_concat_);
	//printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire4_concat_ = 0;

		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire4_expand1x1_+n*128*LightOff_SqueezeNet_S1_fire4_expand1x1_lw*LightOff_SqueezeNet_S1_fire4_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire4_expand1x1_lw,LightOff_SqueezeNet_S1_fire4_expand1x1_lh,128,LightOff_SqueezeNet_S1_b_fire4_concat_ + n * LightOff_SqueezeNet_S1_fire4_concat_lw*LightOff_SqueezeNet_S1_fire4_concat_lh*256,&LightOff_SqueezeNet_S1_fp_fire4_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire4_expand3x3_+n*128*LightOff_SqueezeNet_S1_fire4_expand3x3_lw*LightOff_SqueezeNet_S1_fire4_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire4_expand3x3_lw,LightOff_SqueezeNet_S1_fire4_expand3x3_lh,128,
			LightOff_SqueezeNet_S1_b_fire4_concat_ + n * LightOff_SqueezeNet_S1_fire4_concat_lw*LightOff_SqueezeNet_S1_fire4_concat_lh*256,
			&LightOff_SqueezeNet_S1_fp_fire4_concat_);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire4_concat_[0],LightOff_SqueezeNet_S1_b_fire4_concat_[1],LightOff_SqueezeNet_S1_b_fire4_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire4_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire4_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire4_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire4_expand3x3_=0;
		
	LightOff_SqueezeNet_S1_pool2_lh = (int)(ceil((float)(LightOff_SqueezeNet_S1_fire4_concat_lh + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool2_lw = (int)(ceil((float)(LightOff_SqueezeNet_S1_fire4_concat_lw + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool2_ln = LightOff_SqueezeNet_S1_fire4_concat_ln;
	LightOff_SqueezeNet_S1_b_pool2_ = (float*) malloc(LightOff_SqueezeNet_S1_pool2_ln*256*LightOff_SqueezeNet_S1_pool2_lw*LightOff_SqueezeNet_S1_pool2_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_pool2_);
	//printf("pool %d,%d\n ",LightOff_SqueezeNet_S1_pool3_lw,LightOff_SqueezeNet_S1_pool3_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_max_float(LightOff_SqueezeNet_S1_b_fire4_concat_+n*256*LightOff_SqueezeNet_S1_fire4_concat_lw*LightOff_SqueezeNet_S1_fire4_concat_lh,
			LightOff_SqueezeNet_S1_fire4_concat_lw,LightOff_SqueezeNet_S1_fire4_concat_lh,256,LightOff_SqueezeNet_S1_b_pool2_+n*256*LightOff_SqueezeNet_S1_pool2_lw*LightOff_SqueezeNet_S1_pool2_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	//printf("pool %f,%f,%f\n",LightOff_SqueezeNet_S1_b_pool2_[0],LightOff_SqueezeNet_S1_b_pool2_[1],LightOff_SqueezeNet_S1_b_pool2_[2]);
	free(LightOff_SqueezeNet_S1_b_fire4_concat_);
	LightOff_SqueezeNet_S1_b_fire4_concat_=0;
	
	
	//fire5
	//conv11
	LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw = (LightOff_SqueezeNet_S1_pool2_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh = (LightOff_SqueezeNet_S1_pool2_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln = LightOff_SqueezeNet_S1_pool2_ln;
	LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln*32*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_);
	//printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_pool2_+n*256*LightOff_SqueezeNet_S1_pool2_lw*LightOff_SqueezeNet_S1_pool2_lh,
			LightOff_SqueezeNet_S1_pool2_lw,LightOff_SqueezeNet_S1_pool2_lh,256,1,LightOff_SqueezeNet_S1_kw_fire5_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire5_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh,32);
		CHECK_RES(res);
	}
	//printf("conv11 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_pool2_);
	LightOff_SqueezeNet_S1_b_pool2_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_,LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh*32);
	CHECK_RES(res);
	//printf("fire5_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire5_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire5_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire5_squeeze1x1_[8191]);
	//conv12	
	LightOff_SqueezeNet_S1_fire5_expand1x1_lw = (LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_expand1x1_lh = (LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_expand1x1_ln = LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln;
	LightOff_SqueezeNet_S1_b_fire5_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire5_expand1x1_ln*128*LightOff_SqueezeNet_S1_fire5_expand1x1_lw*LightOff_SqueezeNet_S1_fire5_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire5_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh,32,1,
			LightOff_SqueezeNet_S1_kw_fire5_expand1x1_,LightOff_SqueezeNet_S1_kb_fire5_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire5_expand1x1_+n*128*LightOff_SqueezeNet_S1_fire5_expand1x1_lw*LightOff_SqueezeNet_S1_fire5_expand1x1_lh,128);
		CHECK_RES(res);
	}
	//printf("conv12 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire5_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire5_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire5_expand1x1_[2]);
		res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire5_expand1x1_,LightOff_SqueezeNet_S1_b_fire5_expand1x1_,LightOff_SqueezeNet_S1_fire5_expand1x1_ln*LightOff_SqueezeNet_S1_fire5_expand1x1_lw*LightOff_SqueezeNet_S1_fire5_expand1x1_lh*128);
	CHECK_RES(res);
	//printf("fire5_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire5_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire5_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire5_expand1x1_[4095]);
	//conv13
	LightOff_SqueezeNet_S1_fire5_expand3x3_lw = (LightOff_SqueezeNet_S1_fire5_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_expand3x3_lh = (LightOff_SqueezeNet_S1_fire5_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire5_expand3x3_ln = LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire5_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire5_expand3x3_ln*128*LightOff_SqueezeNet_S1_fire5_expand3x3_lw*LightOff_SqueezeNet_S1_fire5_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire5_expand3x3_);
	for(n=0;n<LightOff_SqueezeNet_S1_fire5_squeeze1x1_ln;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*LightOff_SqueezeNet_S1_fire5_expand1x1_lw*LightOff_SqueezeNet_S1_fire5_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire5_expand1x1_lw,LightOff_SqueezeNet_S1_fire5_expand1x1_lh,32,1,LightOff_SqueezeNet_S1_kw_fire5_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire5_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire5_expand3x3_+n*128*LightOff_SqueezeNet_S1_fire5_expand3x3_lw*LightOff_SqueezeNet_S1_fire5_expand3x3_lh,128);
		CHECK_RES(res);
	}	
	//printf("conv13 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire5_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire5_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire5_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire5_expand3x3_,LightOff_SqueezeNet_S1_b_fire5_expand3x3_,LightOff_SqueezeNet_S1_fire5_expand3x3_ln*LightOff_SqueezeNet_S1_fire5_expand3x3_lw*LightOff_SqueezeNet_S1_fire5_expand3x3_lh*128);
	CHECK_RES(res);
//	printf("f5 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire5_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire5_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire5_expand3x3_[36863]);
	
	//concat4
	LightOff_SqueezeNet_S1_fire5_concat_ln = LightOff_SqueezeNet_S1_fire5_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire5_concat_lw = LightOff_SqueezeNet_S1_fire5_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire5_concat_lh = LightOff_SqueezeNet_S1_fire5_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire5_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire5_concat_lw*LightOff_SqueezeNet_S1_fire5_concat_lh*LightOff_SqueezeNet_S1_fire5_concat_ln*256* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire5_concat_);
	//printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire5_concat_ = 0;

		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire5_expand1x1_+n*128*LightOff_SqueezeNet_S1_fire5_expand1x1_lw*LightOff_SqueezeNet_S1_fire5_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire5_expand1x1_lw,LightOff_SqueezeNet_S1_fire5_expand1x1_lh,128,LightOff_SqueezeNet_S1_b_fire5_concat_ + n * LightOff_SqueezeNet_S1_fire5_concat_lw*LightOff_SqueezeNet_S1_fire5_concat_lh*256,&LightOff_SqueezeNet_S1_fp_fire5_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire5_expand3x3_+n*128*LightOff_SqueezeNet_S1_fire5_expand3x3_lw*LightOff_SqueezeNet_S1_fire5_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire5_expand3x3_lw,LightOff_SqueezeNet_S1_fire5_expand3x3_lh,128,
			LightOff_SqueezeNet_S1_b_fire5_concat_ + n * LightOff_SqueezeNet_S1_fire5_concat_lw*LightOff_SqueezeNet_S1_fire5_concat_lh*256,
			&LightOff_SqueezeNet_S1_fp_fire5_concat_);
		CHECK_RES(res);
	}
//	printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire5_concat_[0],LightOff_SqueezeNet_S1_b_fire5_concat_[1],LightOff_SqueezeNet_S1_b_fire5_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire5_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire5_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire5_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire5_expand3x3_=0;
	
	
	//fire6
	//conv14
	LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw = (LightOff_SqueezeNet_S1_fire5_concat_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh = (LightOff_SqueezeNet_S1_fire5_concat_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln = LightOff_SqueezeNet_S1_fire5_concat_ln;
	LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln*48*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_);
	//printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire5_concat_+n*256*LightOff_SqueezeNet_S1_fire5_concat_lw*LightOff_SqueezeNet_S1_fire5_concat_lh,
			LightOff_SqueezeNet_S1_fire5_concat_lw,LightOff_SqueezeNet_S1_fire5_concat_lh,256,1,LightOff_SqueezeNet_S1_kw_fire6_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire6_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh,48);
		CHECK_RES(res);
	}
	//printf("conv14 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_fire5_concat_);
	LightOff_SqueezeNet_S1_b_fire5_concat_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_,LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh*48);
	CHECK_RES(res);
	//printf("fire6_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire6_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire6_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire6_squeeze1x1_[12287]);

	//conv15	
	LightOff_SqueezeNet_S1_fire6_expand1x1_lw = (LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_expand1x1_lh = (LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_expand1x1_ln = LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire6_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire6_expand1x1_ln*192*LightOff_SqueezeNet_S1_fire6_expand1x1_lw*LightOff_SqueezeNet_S1_fire6_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire6_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh,48,1,
			LightOff_SqueezeNet_S1_kw_fire6_expand1x1_,LightOff_SqueezeNet_S1_kb_fire6_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire6_expand1x1_+n*192*LightOff_SqueezeNet_S1_fire6_expand1x1_lw*LightOff_SqueezeNet_S1_fire6_expand1x1_lh,192);
		CHECK_RES(res);
	}
	//printf("conv15 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire6_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire6_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire6_expand1x1_[2]);
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire6_expand1x1_,LightOff_SqueezeNet_S1_b_fire6_expand1x1_,LightOff_SqueezeNet_S1_fire6_expand1x1_ln*LightOff_SqueezeNet_S1_fire6_expand1x1_lw*LightOff_SqueezeNet_S1_fire6_expand1x1_lh*192);
	CHECK_RES(res);
	//printf("fire6_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire6_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire6_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire6_expand1x1_[9215]);
	//conv16
	LightOff_SqueezeNet_S1_fire6_expand3x3_lw = (LightOff_SqueezeNet_S1_fire6_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_expand3x3_lh = (LightOff_SqueezeNet_S1_fire6_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire6_expand3x3_ln = LightOff_SqueezeNet_S1_fire6_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire6_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire6_expand3x3_ln*192*LightOff_SqueezeNet_S1_fire6_expand3x3_lw*LightOff_SqueezeNet_S1_fire6_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire6_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire6_expand1x1_lw*LightOff_SqueezeNet_S1_fire6_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire6_expand1x1_lw,LightOff_SqueezeNet_S1_fire6_expand1x1_lh,48,1,LightOff_SqueezeNet_S1_kw_fire6_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire6_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire6_expand3x3_+n*192*LightOff_SqueezeNet_S1_fire6_expand3x3_lw*LightOff_SqueezeNet_S1_fire6_expand3x3_lh,192);
		CHECK_RES(res);
	}	
//	printf("conv16 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire6_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire6_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire6_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire6_expand3x3_,LightOff_SqueezeNet_S1_b_fire6_expand3x3_,LightOff_SqueezeNet_S1_fire6_expand3x3_ln*LightOff_SqueezeNet_S1_fire6_expand3x3_lw*LightOff_SqueezeNet_S1_fire6_expand3x3_lh*192);
	CHECK_RES(res);
	//printf("f6 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire6_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire6_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire6_expand3x3_[82943]);
	
	//concat5
	LightOff_SqueezeNet_S1_fire6_concat_ln = LightOff_SqueezeNet_S1_fire6_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire6_concat_lw = LightOff_SqueezeNet_S1_fire6_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire6_concat_lh = LightOff_SqueezeNet_S1_fire6_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire6_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire6_concat_lw*LightOff_SqueezeNet_S1_fire6_concat_lh*LightOff_SqueezeNet_S1_fire6_concat_ln*384* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire6_concat_);
	//printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire6_concat_ = 0;

		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire6_expand1x1_+n*192*LightOff_SqueezeNet_S1_fire6_expand1x1_lw*LightOff_SqueezeNet_S1_fire6_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire6_expand1x1_lw,LightOff_SqueezeNet_S1_fire6_expand1x1_lh,192,LightOff_SqueezeNet_S1_b_fire6_concat_ + n * LightOff_SqueezeNet_S1_fire6_concat_lw*LightOff_SqueezeNet_S1_fire6_concat_lh*384,&LightOff_SqueezeNet_S1_fp_fire6_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire6_expand3x3_+n*192*LightOff_SqueezeNet_S1_fire6_expand3x3_lw*LightOff_SqueezeNet_S1_fire6_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire6_expand3x3_lw,LightOff_SqueezeNet_S1_fire6_expand3x3_lh,192,
			LightOff_SqueezeNet_S1_b_fire6_concat_ + n * LightOff_SqueezeNet_S1_fire6_concat_lw*LightOff_SqueezeNet_S1_fire6_concat_lh*384,
			&LightOff_SqueezeNet_S1_fp_fire6_concat_);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire6_concat_[0],LightOff_SqueezeNet_S1_b_fire6_concat_[1],LightOff_SqueezeNet_S1_b_fire6_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire6_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire6_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire6_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire6_expand3x3_=0;
	
	//fire7
	//conv17
	LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw = (LightOff_SqueezeNet_S1_fire6_concat_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh = (LightOff_SqueezeNet_S1_fire6_concat_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln = LightOff_SqueezeNet_S1_fire6_concat_ln;
	LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln*48*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_);
	//printf("%d,%d\n ",LightOff_SqueezeNet_S1_fire3_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire6_concat_+n*384*LightOff_SqueezeNet_S1_fire6_concat_lw*LightOff_SqueezeNet_S1_fire6_concat_lh,
			LightOff_SqueezeNet_S1_fire6_concat_lw,LightOff_SqueezeNet_S1_fire6_concat_lh,384,1,LightOff_SqueezeNet_S1_kw_fire7_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire7_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh,48);
		CHECK_RES(res);
	}
#ifdef PRINTF
	printf("conv17 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_[2]);
#endif
	free(LightOff_SqueezeNet_S1_b_fire6_concat_);
	LightOff_SqueezeNet_S1_b_fire6_concat_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_,LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh*48);
	CHECK_RES(res);
//printf("fire7_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire7_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire7_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire7_squeeze1x1_[18431]);
	//conv18	
	LightOff_SqueezeNet_S1_fire7_expand1x1_lw = (LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_expand1x1_lh = (LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_expand1x1_ln = LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire7_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire7_expand1x1_ln*192*LightOff_SqueezeNet_S1_fire7_expand1x1_lw*LightOff_SqueezeNet_S1_fire7_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire7_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh,48,1,
			LightOff_SqueezeNet_S1_kw_fire7_expand1x1_,LightOff_SqueezeNet_S1_kb_fire7_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire7_expand1x1_+n*1921*LightOff_SqueezeNet_S1_fire7_expand1x1_lw*LightOff_SqueezeNet_S1_fire7_expand1x1_lh,192);
		CHECK_RES(res);
	}
//	printf("conv18 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire7_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire7_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire7_expand1x1_[2]);
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire7_expand1x1_,LightOff_SqueezeNet_S1_b_fire7_expand1x1_,LightOff_SqueezeNet_S1_fire7_expand1x1_ln*LightOff_SqueezeNet_S1_fire7_expand1x1_lw*LightOff_SqueezeNet_S1_fire7_expand1x1_lh*192);
	CHECK_RES(res);
//	printf("fire7_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire7_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire7_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire7_expand1x1_[9215]);
	//conv19
	LightOff_SqueezeNet_S1_fire7_expand3x3_lw = (LightOff_SqueezeNet_S1_fire7_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_expand3x3_lh = (LightOff_SqueezeNet_S1_fire7_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire7_expand3x3_ln = LightOff_SqueezeNet_S1_fire7_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire7_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire7_expand3x3_ln*192*LightOff_SqueezeNet_S1_fire7_expand3x3_lw*LightOff_SqueezeNet_S1_fire7_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire7_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*LightOff_SqueezeNet_S1_fire7_expand1x1_lw*LightOff_SqueezeNet_S1_fire7_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire7_expand1x1_lw,LightOff_SqueezeNet_S1_fire7_expand1x1_lh,48,1,LightOff_SqueezeNet_S1_kw_fire7_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire7_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire7_expand3x3_+n*192*LightOff_SqueezeNet_S1_fire7_expand3x3_lw*LightOff_SqueezeNet_S1_fire7_expand3x3_lh,192);
		CHECK_RES(res);
	}	
	//printf("conv19 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire7_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire7_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire7_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire7_expand3x3_,LightOff_SqueezeNet_S1_b_fire7_expand3x3_,LightOff_SqueezeNet_S1_fire7_expand3x3_ln*LightOff_SqueezeNet_S1_fire7_expand3x3_lw*LightOff_SqueezeNet_S1_fire7_expand3x3_lh*192);
	CHECK_RES(res);
//	printf("f7 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire7_expand3x3_[82943]);
	//concat6
	LightOff_SqueezeNet_S1_fire7_concat_ln = LightOff_SqueezeNet_S1_fire7_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire7_concat_lw = LightOff_SqueezeNet_S1_fire7_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire7_concat_lh = LightOff_SqueezeNet_S1_fire7_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire7_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire7_concat_lw*LightOff_SqueezeNet_S1_fire7_concat_lh*LightOff_SqueezeNet_S1_fire7_concat_ln*384* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire7_concat_);
//	printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire7_concat_ = 0;

		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire7_expand1x1_+n*192*LightOff_SqueezeNet_S1_fire7_expand1x1_lw*LightOff_SqueezeNet_S1_fire7_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire7_expand1x1_lw,LightOff_SqueezeNet_S1_fire7_expand1x1_lh,192,LightOff_SqueezeNet_S1_b_fire7_concat_ + n * LightOff_SqueezeNet_S1_fire7_concat_lw*LightOff_SqueezeNet_S1_fire7_concat_lh*384,&LightOff_SqueezeNet_S1_fp_fire7_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire7_expand3x3_+n*192*LightOff_SqueezeNet_S1_fire7_expand3x3_lw*LightOff_SqueezeNet_S1_fire7_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire7_expand3x3_lw,LightOff_SqueezeNet_S1_fire7_expand3x3_lh,192,
			LightOff_SqueezeNet_S1_b_fire7_concat_ + n * LightOff_SqueezeNet_S1_fire7_concat_lw*LightOff_SqueezeNet_S1_fire7_concat_lh*384,
			&LightOff_SqueezeNet_S1_fp_fire7_concat_);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire7_concat_[0],LightOff_SqueezeNet_S1_b_fire7_concat_[1],LightOff_SqueezeNet_S1_b_fire7_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire7_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire7_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire7_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire7_expand3x3_=0;
	
	//fire8
	//conv20
	LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw = (LightOff_SqueezeNet_S1_fire7_concat_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh = (LightOff_SqueezeNet_S1_fire7_concat_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln = LightOff_SqueezeNet_S1_fire7_concat_ln;
	LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln*64*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire7_concat_+n*384*LightOff_SqueezeNet_S1_fire7_concat_lw*LightOff_SqueezeNet_S1_fire7_concat_lh,
			LightOff_SqueezeNet_S1_fire7_concat_lw,LightOff_SqueezeNet_S1_fire7_concat_lh,384,1,LightOff_SqueezeNet_S1_kw_fire8_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire8_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh,64);
		CHECK_RES(res);
	}
	//printf("conv20 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_fire7_concat_);
	LightOff_SqueezeNet_S1_b_fire7_concat_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_,LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh*64);
	CHECK_RES(res);
//printf("fire8_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire8_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire8_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire8_squeeze1x1_[24575]);
	//conv21	
	LightOff_SqueezeNet_S1_fire8_expand1x1_lw = (LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_expand1x1_lh = (LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_expand1x1_ln = LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln;
	LightOff_SqueezeNet_S1_b_fire8_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire8_expand1x1_ln*256*LightOff_SqueezeNet_S1_fire8_expand1x1_lw*LightOff_SqueezeNet_S1_fire8_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire8_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh,64,1,
			LightOff_SqueezeNet_S1_kw_fire8_expand1x1_,LightOff_SqueezeNet_S1_kb_fire8_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire8_expand1x1_+n*256*LightOff_SqueezeNet_S1_fire8_expand1x1_lw*LightOff_SqueezeNet_S1_fire8_expand1x1_lh,256);
		CHECK_RES(res);
	}
//	printf("conv21 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire8_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire8_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire8_expand1x1_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire8_expand1x1_,LightOff_SqueezeNet_S1_b_fire8_expand1x1_,LightOff_SqueezeNet_S1_fire8_expand1x1_ln*LightOff_SqueezeNet_S1_fire8_expand1x1_lw*LightOff_SqueezeNet_S1_fire8_expand1x1_lh*256);
	CHECK_RES(res);
	//printf("fire8_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire8_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire8_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire8_expand1x1_[16383]);
	//conv22
	LightOff_SqueezeNet_S1_fire8_expand3x3_lw = (LightOff_SqueezeNet_S1_fire8_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_expand3x3_lh = (LightOff_SqueezeNet_S1_fire8_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire8_expand3x3_ln = LightOff_SqueezeNet_S1_fire8_squeeze1x1_ln;	
	LightOff_SqueezeNet_S1_b_fire8_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire8_expand3x3_ln*256*LightOff_SqueezeNet_S1_fire8_expand3x3_lw*LightOff_SqueezeNet_S1_fire8_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire8_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire8_expand1x1_lw*LightOff_SqueezeNet_S1_fire8_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire8_expand1x1_lw,LightOff_SqueezeNet_S1_fire8_expand1x1_lh,64,1,LightOff_SqueezeNet_S1_kw_fire8_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire8_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire8_expand3x3_+n*256*LightOff_SqueezeNet_S1_fire8_expand3x3_lw*LightOff_SqueezeNet_S1_fire8_expand3x3_lh,256);
		CHECK_RES(res);
	}	
//	printf("conv22 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire8_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire8_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire8_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire8_expand3x3_,LightOff_SqueezeNet_S1_b_fire8_expand3x3_,LightOff_SqueezeNet_S1_fire8_expand3x3_ln*LightOff_SqueezeNet_S1_fire8_expand3x3_lw*LightOff_SqueezeNet_S1_fire8_expand3x3_lh*256);
	CHECK_RES(res);
	//printf("f8 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire8_expand3x3_[147455]);

	//concat7
	LightOff_SqueezeNet_S1_fire8_concat_ln = LightOff_SqueezeNet_S1_fire8_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire8_concat_lw = LightOff_SqueezeNet_S1_fire8_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire8_concat_lh = LightOff_SqueezeNet_S1_fire8_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire8_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire8_concat_lw*LightOff_SqueezeNet_S1_fire8_concat_lh*LightOff_SqueezeNet_S1_fire8_concat_ln*512* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire8_concat_);
	
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire8_concat_ = 0;
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire8_expand1x1_+n*256*LightOff_SqueezeNet_S1_fire8_expand1x1_lw*LightOff_SqueezeNet_S1_fire8_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire8_expand1x1_lw,LightOff_SqueezeNet_S1_fire8_expand1x1_lh,256,LightOff_SqueezeNet_S1_b_fire8_concat_ + n * LightOff_SqueezeNet_S1_fire8_concat_lw*LightOff_SqueezeNet_S1_fire8_concat_lh*512,&LightOff_SqueezeNet_S1_fp_fire8_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire8_expand3x3_+n*256*LightOff_SqueezeNet_S1_fire8_expand3x3_lw*LightOff_SqueezeNet_S1_fire8_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire8_expand3x3_lw,LightOff_SqueezeNet_S1_fire8_expand3x3_lh,256,
			LightOff_SqueezeNet_S1_b_fire8_concat_ + n * LightOff_SqueezeNet_S1_fire8_concat_lw*LightOff_SqueezeNet_S1_fire8_concat_lh*512,
			&LightOff_SqueezeNet_S1_fp_fire8_concat_);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire8_concat_[0],LightOff_SqueezeNet_S1_b_fire8_concat_[1],LightOff_SqueezeNet_S1_b_fire8_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire8_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire8_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire8_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire8_expand3x3_=0;

	LightOff_SqueezeNet_S1_pool3_lh = (int)(ceil((float)(LightOff_SqueezeNet_S1_fire8_concat_lh + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool3_lw = (int)(ceil((float)(LightOff_SqueezeNet_S1_fire8_concat_lw + 2*0-3)/2)) + 1;
	LightOff_SqueezeNet_S1_pool3_ln = LightOff_SqueezeNet_S1_fire8_concat_ln;
	LightOff_SqueezeNet_S1_b_pool3_ = (float*) malloc(LightOff_SqueezeNet_S1_pool3_ln*512*LightOff_SqueezeNet_S1_pool3_lw*LightOff_SqueezeNet_S1_pool3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_pool3_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_max_float(LightOff_SqueezeNet_S1_b_fire8_concat_+n*512*LightOff_SqueezeNet_S1_fire8_concat_lw*LightOff_SqueezeNet_S1_fire8_concat_lh,
			LightOff_SqueezeNet_S1_fire8_concat_lw,LightOff_SqueezeNet_S1_fire8_concat_lh,512,LightOff_SqueezeNet_S1_b_pool3_+n*512*LightOff_SqueezeNet_S1_pool3_lw*LightOff_SqueezeNet_S1_pool3_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire8_concat_[0],LightOff_SqueezeNet_S1_b_fire8_concat_[1],LightOff_SqueezeNet_S1_b_fire8_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire8_concat_);
	LightOff_SqueezeNet_S1_b_fire8_concat_=0;

	//fire9
	//conv23
	LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw = (LightOff_SqueezeNet_S1_pool3_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh = (LightOff_SqueezeNet_S1_pool3_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln = LightOff_SqueezeNet_S1_pool3_ln;
	LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln*64*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_pool3_+n*512*LightOff_SqueezeNet_S1_pool3_lw*LightOff_SqueezeNet_S1_pool3_lh,
			LightOff_SqueezeNet_S1_pool3_lw,LightOff_SqueezeNet_S1_pool3_lh,512,1,LightOff_SqueezeNet_S1_kw_fire9_squeeze1x1_,
			LightOff_SqueezeNet_S1_kb_fire9_squeeze1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh,64);
		CHECK_RES(res);
	}
	//printf("conv23 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_[0],LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_[1],LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_[2]);
	free(LightOff_SqueezeNet_S1_b_pool3_);
	LightOff_SqueezeNet_S1_b_pool3_=0;
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_,LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_,LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh*64);
	CHECK_RES(res);
	//printf("fire9_squeeze1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire9_squeeze1x1_[0],LightOff_SqueezeNet_S1_kw_fire9_squeeze1x1_[1],LightOff_SqueezeNet_S1_kw_fire9_squeeze1x1_[32765]);
	//conv24	
	LightOff_SqueezeNet_S1_fire9_expand1x1_lw = (LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_expand1x1_lh = (LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh + 2*0-1)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_expand1x1_ln = LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln;	
	LightOff_SqueezeNet_S1_b_fire9_expand1x1_ = (float*) malloc(LightOff_SqueezeNet_S1_fire9_expand1x1_ln*256*LightOff_SqueezeNet_S1_fire9_expand1x1_lw*LightOff_SqueezeNet_S1_fire9_expand1x1_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire9_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh,64,1,
			LightOff_SqueezeNet_S1_kw_fire9_expand1x1_,LightOff_SqueezeNet_S1_kb_fire9_expand1x1_,1,1,0,0,1,1,LightOff_SqueezeNet_S1_b_fire9_expand1x1_+n*256*LightOff_SqueezeNet_S1_fire9_expand1x1_lw*LightOff_SqueezeNet_S1_fire9_expand1x1_lh,256);
		CHECK_RES(res);
	}
	//printf("conv24 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire9_expand1x1_[0],LightOff_SqueezeNet_S1_b_fire9_expand1x1_[1],LightOff_SqueezeNet_S1_b_fire9_expand1x1_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire9_expand1x1_,LightOff_SqueezeNet_S1_b_fire9_expand1x1_,LightOff_SqueezeNet_S1_fire9_expand1x1_ln*LightOff_SqueezeNet_S1_fire9_expand1x1_lw*LightOff_SqueezeNet_S1_fire9_expand1x1_lh*256);
	CHECK_RES(res);
	//printf("fire9_expand1x1 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire9_expand1x1_[0],LightOff_SqueezeNet_S1_kw_fire9_expand1x1_[1],LightOff_SqueezeNet_S1_kw_fire9_expand1x1_[16383]);
	//conv25
	LightOff_SqueezeNet_S1_fire9_expand3x3_lw = (LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_expand3x3_lh = (LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh + 2*1-3)/1 + 1;
	LightOff_SqueezeNet_S1_fire9_expand3x3_ln = LightOff_SqueezeNet_S1_fire9_squeeze1x1_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_fire9_expand3x3_ = (float*) malloc(LightOff_SqueezeNet_S1_fire9_expand3x3_ln*256*LightOff_SqueezeNet_S1_fire9_expand3x3_lw*LightOff_SqueezeNet_S1_fire9_expand3x3_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire9_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw*LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh,
			LightOff_SqueezeNet_S1_fire9_squeeze1x1_lw,LightOff_SqueezeNet_S1_fire9_squeeze1x1_lh,64,1,LightOff_SqueezeNet_S1_kw_fire9_expand3x3_,
			LightOff_SqueezeNet_S1_kb_fire9_expand3x3_,3,3,1,1,1,1,LightOff_SqueezeNet_S1_b_fire9_expand3x3_+n*256*LightOff_SqueezeNet_S1_fire9_expand3x3_lw*LightOff_SqueezeNet_S1_fire9_expand3x3_lh,256);
		CHECK_RES(res);
	}
	//printf("conv25 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire9_expand3x3_[0],LightOff_SqueezeNet_S1_b_fire9_expand3x3_[1],LightOff_SqueezeNet_S1_b_fire9_expand3x3_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_fire9_expand3x3_,LightOff_SqueezeNet_S1_b_fire9_expand3x3_,LightOff_SqueezeNet_S1_fire9_expand3x3_ln*LightOff_SqueezeNet_S1_fire9_expand3x3_lw*LightOff_SqueezeNet_S1_fire9_expand3x3_lh*256);
	CHECK_RES(res);
	//printf("f9 3x3 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[0],LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[1],LightOff_SqueezeNet_S1_kw_fire9_expand3x3_[147455]);
	//concat8
	LightOff_SqueezeNet_S1_fire9_concat_ln = LightOff_SqueezeNet_S1_fire9_expand1x1_ln;
	LightOff_SqueezeNet_S1_fire9_concat_lw = LightOff_SqueezeNet_S1_fire9_expand1x1_lw;
	LightOff_SqueezeNet_S1_fire9_concat_lh = LightOff_SqueezeNet_S1_fire9_expand1x1_lh;
	LightOff_SqueezeNet_S1_b_fire9_concat_ = (float*) malloc(LightOff_SqueezeNet_S1_fire9_concat_lw*LightOff_SqueezeNet_S1_fire9_concat_lh*LightOff_SqueezeNet_S1_fire9_concat_ln*512* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_fire9_concat_);
	//printf("concat2 %d,%d\n ",LightOff_SqueezeNet_S1_fire3_concat_lw,LightOff_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		LightOff_SqueezeNet_S1_fp_fire9_concat_ = 0;
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire9_expand1x1_+n*256*LightOff_SqueezeNet_S1_fire9_expand1x1_lw*LightOff_SqueezeNet_S1_fire9_expand1x1_lh,
			LightOff_SqueezeNet_S1_fire9_expand1x1_lw,LightOff_SqueezeNet_S1_fire9_expand1x1_lh,256,LightOff_SqueezeNet_S1_b_fire9_concat_ + n * LightOff_SqueezeNet_S1_fire9_concat_lw*LightOff_SqueezeNet_S1_fire9_concat_lh*512,&LightOff_SqueezeNet_S1_fp_fire9_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(LightOff_SqueezeNet_S1_b_fire9_expand3x3_+n*256*LightOff_SqueezeNet_S1_fire9_expand3x3_lw*LightOff_SqueezeNet_S1_fire9_expand3x3_lh,
			LightOff_SqueezeNet_S1_fire9_expand3x3_lw,LightOff_SqueezeNet_S1_fire9_expand3x3_lh,256,
			LightOff_SqueezeNet_S1_b_fire9_concat_ + n * LightOff_SqueezeNet_S1_fire9_concat_lw*LightOff_SqueezeNet_S1_fire9_concat_lh*512,
			&LightOff_SqueezeNet_S1_fp_fire9_concat_);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",LightOff_SqueezeNet_S1_b_fire9_concat_[0],LightOff_SqueezeNet_S1_b_fire9_concat_[1],LightOff_SqueezeNet_S1_b_fire9_concat_[2]);
	free(LightOff_SqueezeNet_S1_b_fire9_expand1x1_);
	LightOff_SqueezeNet_S1_b_fire9_expand1x1_=0;
	free(LightOff_SqueezeNet_S1_b_fire9_expand3x3_);
	LightOff_SqueezeNet_S1_b_fire9_expand3x3_=0;

	//conv26
	LightOff_SqueezeNet_S1_conv26_lw = (LightOff_SqueezeNet_S1_fire9_concat_lw -1+2*1)/1 + 1;
	LightOff_SqueezeNet_S1_conv26_lh = (LightOff_SqueezeNet_S1_fire9_concat_lh -1+2*1)/1 + 1;
	LightOff_SqueezeNet_S1_conv26_ln = LightOff_SqueezeNet_S1_fire9_concat_ln;
	//LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_;
	LightOff_SqueezeNet_S1_b_conv26_ = (float*) malloc(LightOff_SqueezeNet_S1_conv26_ln*2*LightOff_SqueezeNet_S1_conv26_lw*LightOff_SqueezeNet_S1_conv26_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_conv26_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(LightOff_SqueezeNet_S1_b_fire9_concat_+n*512*LightOff_SqueezeNet_S1_fire9_concat_lw*LightOff_SqueezeNet_S1_fire9_concat_lh,
			LightOff_SqueezeNet_S1_fire9_concat_lw,LightOff_SqueezeNet_S1_fire9_concat_lh,512,1,
			LightOff_SqueezeNet_S1_kw_conv26_,LightOff_SqueezeNet_S1_kb_conv26_,1,1,1,1,1,1,LightOff_SqueezeNet_S1_b_conv26_+n*2*LightOff_SqueezeNet_S1_conv26_lw*LightOff_SqueezeNet_S1_conv26_lh,2);
		CHECK_RES(res);
	}
	//printf("conv26 %f,%f,%f\n",LightOff_SqueezeNet_S1_b_conv26_[0],LightOff_SqueezeNet_S1_b_conv26_[1],LightOff_SqueezeNet_S1_b_conv26_[2]);
	res = ly_relu_float(LightOff_SqueezeNet_S1_b_conv26_,LightOff_SqueezeNet_S1_b_conv26_,LightOff_SqueezeNet_S1_conv26_ln*LightOff_SqueezeNet_S1_conv26_lw*LightOff_SqueezeNet_S1_conv26_lh*2);
	CHECK_RES(res);	
	//printf("conv10 %f,%f,%f\n",LightOff_SqueezeNet_S1_kw_conv26_[0],LightOff_SqueezeNet_S1_kw_conv26_[1],LightOff_SqueezeNet_S1_kw_conv26_[1023]);
	LightOff_SqueezeNet_S1_pool4_lh = (int)(ceil((float)(LightOff_SqueezeNet_S1_conv26_lh + 2*0-15)/1)) + 1;
	LightOff_SqueezeNet_S1_pool4_lw = (int)(ceil((float)(LightOff_SqueezeNet_S1_conv26_lw + 2*0-15)/1)) + 1;
	LightOff_SqueezeNet_S1_pool4_ln = LightOff_SqueezeNet_S1_conv26_ln;
	LightOff_SqueezeNet_S1_b_pool4_ = (float*) malloc(LightOff_SqueezeNet_S1_pool4_ln*2*LightOff_SqueezeNet_S1_pool4_lw*LightOff_SqueezeNet_S1_pool4_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_pool4_);
	//printf("pool %d,%d\n ",LightOff_SqueezeNet_S1_pool3_lw,LightOff_SqueezeNet_S1_pool3_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_ave_float(LightOff_SqueezeNet_S1_b_conv26_+n*2*LightOff_SqueezeNet_S1_conv26_lw*LightOff_SqueezeNet_S1_conv26_lh,
			LightOff_SqueezeNet_S1_conv26_lw,LightOff_SqueezeNet_S1_conv26_lh,2,LightOff_SqueezeNet_S1_b_pool4_+n*3*LightOff_SqueezeNet_S1_pool4_lw*LightOff_SqueezeNet_S1_pool4_lh,15,15,1,1,0,0);
		CHECK_RES(res);
	}
#ifdef PRINTF
	printf("pool %f,%f\n",LightOff_SqueezeNet_S1_b_pool4_[0],LightOff_SqueezeNet_S1_b_pool4_[1]);
#endif
	free(LightOff_SqueezeNet_S1_b_conv26_);
	LightOff_SqueezeNet_S1_b_conv26_=0;	
	LightOff_SqueezeNet_S1_rpn_cls_prob_lw = LightOff_SqueezeNet_S1_pool4_lw;
	LightOff_SqueezeNet_S1_rpn_cls_prob_lh = LightOff_SqueezeNet_S1_pool4_lh;
	LightOff_SqueezeNet_S1_rpn_cls_prob_ln = LightOff_SqueezeNet_S1_pool4_ln;	
	LightOff_SqueezeNet_S1_b_rpn_cls_prob_ = (float*) malloc(LightOff_SqueezeNet_S1_rpn_cls_prob_ln*2*LightOff_SqueezeNet_S1_rpn_cls_prob_lw*LightOff_SqueezeNet_S1_rpn_cls_prob_lh* sizeof(float));
	CHECK_PTR(LightOff_SqueezeNet_S1_b_rpn_cls_prob_);
#ifdef PRINTF
	printf("softmax %d,%d\n ",LightOff_SqueezeNet_S1_rpn_cls_prob_lw,LightOff_SqueezeNet_S1_rpn_cls_prob_lh);
	printf("%d,%d,%d\n",LightOff_SqueezeNet_S1_rpn_cls_prob_ln,LightOff_SqueezeNet_S1_rpn_cls_prob_lw,LightOff_SqueezeNet_S1_rpn_cls_prob_lh);
#endif
	res = ly_softmax(LightOff_SqueezeNet_S1_b_pool4_,1*1,2,1,LightOff_SqueezeNet_S1_b_rpn_cls_prob_);
	CHECK_RES(res);
	free(LightOff_SqueezeNet_S1_b_pool4_);
	LightOff_SqueezeNet_S1_b_pool4_=0;		
#ifdef PRINTF
	printf("%f,%f,%f\n",LightOff_SqueezeNet_S1_b_rpn_cls_prob_[0],LightOff_SqueezeNet_S1_b_rpn_cls_prob_[1],LightOff_SqueezeNet_S1_b_rpn_cls_prob_[2]);
#endif
	//printf("%f,%f\n",LightOff_SqueezeNet_S1_b_rpn_cls_prob_[0],LightOff_SqueezeNet_S1_b_rpn_cls_prob_[1]);
	if(LightOff_SqueezeNet_S1_b_rpn_cls_prob_[0]>LightOff_SqueezeNet_S1_b_rpn_cls_prob_[1]  /*&& LightOff_SqueezeNet_S1_b_rpn_cls_prob_[0]>LightOff_SqueezeNet_S1_b_rpn_cls_prob_[2]*/)
	{
		*label=0;
	}
	else /*(LightOff_SqueezeNet_S1_b_rpn_cls_prob_[1]>LightOff_SqueezeNet_S1_b_rpn_cls_prob_[0]  &&LightOff_SqueezeNet_S1_b_rpn_cls_prob_[1]>LightOff_SqueezeNet_S1_b_rpn_cls_prob_[2])*/
	{
		*label=1;
	}
	//else *label=2;
	
exit:
	if(LightOff_SqueezeNet_S1_b_data_) {free(LightOff_SqueezeNet_S1_b_data_);}
	if(LightOff_SqueezeNet_S1_b_conv1_) {free(LightOff_SqueezeNet_S1_b_conv1_);}
	if(LightOff_SqueezeNet_S1_b_conv26_) {free(LightOff_SqueezeNet_S1_b_conv26_);}
	if(LightOff_SqueezeNet_S1_b_pool1_) {free(LightOff_SqueezeNet_S1_b_pool1_);}
	if(LightOff_SqueezeNet_S1_b_pool2_) {free(LightOff_SqueezeNet_S1_b_pool2_);}
	if(LightOff_SqueezeNet_S1_b_pool3_) {free(LightOff_SqueezeNet_S1_b_pool3_);}
	if(LightOff_SqueezeNet_S1_b_pool4_) {free(LightOff_SqueezeNet_S1_b_pool4_);}
	if(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire2_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire2_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire2_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire2_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire2_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire2_concat_) {free(LightOff_SqueezeNet_S1_b_fire2_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire3_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire3_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire3_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire3_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire3_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire3_concat_) {free(LightOff_SqueezeNet_S1_b_fire3_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire4_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire4_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire4_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire4_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire4_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire4_concat_) {free(LightOff_SqueezeNet_S1_b_fire4_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire5_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire5_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire5_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire5_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire5_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire5_concat_) {free(LightOff_SqueezeNet_S1_b_fire5_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire6_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire6_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire6_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire6_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire6_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire6_concat_) {free(LightOff_SqueezeNet_S1_b_fire6_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire7_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire7_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire7_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire7_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire7_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire7_concat_) {free(LightOff_SqueezeNet_S1_b_fire7_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire8_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire8_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire8_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire8_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire8_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire8_concat_) {free(LightOff_SqueezeNet_S1_b_fire8_concat_);}
	if(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_) {free(LightOff_SqueezeNet_S1_b_fire9_squeeze1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire9_expand1x1_) {free(LightOff_SqueezeNet_S1_b_fire9_expand1x1_);}
	if(LightOff_SqueezeNet_S1_b_fire9_expand3x3_) {free(LightOff_SqueezeNet_S1_b_fire9_expand3x3_);}
	if(LightOff_SqueezeNet_S1_b_fire9_concat_) {free(LightOff_SqueezeNet_S1_b_fire9_concat_);}
	end1=clock();
#ifdef PRINTF
	printf("squeeze 运行时间：%d\n",end1-start1);
#endif
	return res;
}


