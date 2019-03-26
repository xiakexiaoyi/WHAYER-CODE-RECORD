#include "flatnn.h"
#include "ProtectionLink_SqueezeNet_S1.h"
#include <math.h>

#ifndef CHECK_PTR 
#define CHECK_PTR(ptr) if(!ptr) {res=MERR_NO_MEMORY; goto exit; } 
#endif
#ifndef CHECK_RES 
#define CHECK_RES(res) if(res!=MOK) {goto exit;} 
#endif

MRESULT ProtectionLink_SqueezeNet_S1_run_C3(unsigned char* bgr, int iw, int ih, int widthStep,  int* label)
{
	MRESULT res = 0;
	int n = 0;
	int ProtectionLink_SqueezeNet_S1_data_ln = 0;
	int ProtectionLink_SqueezeNet_S1_data_lw = 0;
	int ProtectionLink_SqueezeNet_S1_data_lh = 0;
	int ProtectionLink_SqueezeNet_S1_conv1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_conv1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_conv1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_pool1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_pool1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_pool1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_0_lw = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_0_lh = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_0_ln = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_lw = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_lh = 0;
	//int ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire2_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire2_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh = 0;

	int ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh = 0;	
	int ProtectionLink_SqueezeNet_S1_fire3_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand1x1_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh = 0;
	int ProtectionLink_SqueezeNet_S1_conv26_lw=0;
	int ProtectionLink_SqueezeNet_S1_conv26_lh=0;
	int ProtectionLink_SqueezeNet_S1_conv26_ln=0;
	int ProtectionLink_SqueezeNet_S1_fire3_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand3x3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire3_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire4_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire5_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire6_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire7_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire8_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fp_fire9_concat_ = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire3_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire4_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire5_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire6_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire7_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire8_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_concat_lw = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_concat_lh = 0;
	int ProtectionLink_SqueezeNet_S1_fire9_concat_ln = 0;
	int ProtectionLink_SqueezeNet_S1_pool3_lw = 0;
	int ProtectionLink_SqueezeNet_S1_pool3_lh = 0;
	int ProtectionLink_SqueezeNet_S1_pool3_ln = 0;
	int ProtectionLink_SqueezeNet_S1_pool4_lw = 0;
	int ProtectionLink_SqueezeNet_S1_pool4_lh = 0;
	int ProtectionLink_SqueezeNet_S1_pool4_ln = 0;
	int ProtectionLink_SqueezeNet_S1_pool2_lw = 0;
	int ProtectionLink_SqueezeNet_S1_pool2_lh = 0;
	int ProtectionLink_SqueezeNet_S1_pool2_ln = 0;
	int ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lw = 0;
	int ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lh = 0;
	int ProtectionLink_SqueezeNet_S1_rpn_cls_prob_ln = 0;

	float* ProtectionLink_SqueezeNet_S1_b_data_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_conv1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_pool1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_ = 0;	
	float* ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire2_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_ = 0;	
	float* ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_conv26_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire3_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire4_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire5_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire6_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire7_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire8_concat_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_fire9_concat_ = 0;    
	float* ProtectionLink_SqueezeNet_S1_b_pool2_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_pool3_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_pool4_ = 0;
	float* ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_=0;

	extern float ProtectionLink_SqueezeNet_S1_mean_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_data_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_data_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire2_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire2_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire2_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire2_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire2_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire2_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire3_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire3_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire3_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire3_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire3_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire3_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire4_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire4_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire4_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire4_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire4_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire4_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire5_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire5_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire5_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire5_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire5_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire5_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire6_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire6_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire6_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire6_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire6_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire6_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire7_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire7_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire7_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire7_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire7_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire8_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire8_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire8_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire8_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire8_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire9_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire9_squeeze1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire9_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire9_expand1x1_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_fire9_expand3x3_[];
	extern float ProtectionLink_SqueezeNet_S1_kw_conv26_[];
	extern float ProtectionLink_SqueezeNet_S1_kb_conv26_[];
//	FILE *fire7_kw_expand3x3;
//	FILE *fire8_kw_expand3x3;
//	FILE *fire9_kw_expand3x3;
//
//    FILE *test;
//
//	float *test_test = (float *)malloc(4718592*sizeof(float));
//
//	float *ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_ = (float *)malloc(82944*sizeof(float));
//	float *ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_ = (float *)malloc(147456*sizeof(float));
//	float *ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_ = (float *)malloc(147456*sizeof(float));
//	/*printf("f7 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[82943]);*/
//	
//	test=fopen("d://squeeze//etruck_PVANET_LITE_A_netdata1_binary.txt","rb");
//    if (NULL==test) 
//	{
//        printf("Can not open file test Data.txt!\n");
//        return -1;
//    }	
//	fread(test_test,4718592,sizeof(float),test);
//
//	printf("f7 3x3 %f,%f,%f\n",test_test[0],test_test[1],test_test[2]);
//
//
//
//
//	
//	
//	
//	fire7_kw_expand3x3=fopen("d://squeeze//fire7_expand3x3_weight.txt","rb");
//    if (NULL==fire7_kw_expand3x3) 
//	{
//        printf("Can not open file Data.txt!\n");
//        return -1;
//    }	
//	fread(ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_,82944,sizeof(float),fire7_kw_expand3x3);
//	/*printf("f7 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[82943]);*/
//	fclose(fire7_kw_expand3x3);
//
//	fire8_kw_expand3x3=fopen("d://squeeze//fire8_expand3x3_weight.txt","rb");
//    if (NULL==fire8_kw_expand3x3) 
//	{
//        printf("Can not open file Data.txt!\n");
//        return -1;
//    }	
//	fread(ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_,147456,sizeof(float),fire8_kw_expand3x3);
//	fclose(fire8_kw_expand3x3);
//	/*printf("f8 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[147455]);
//*/
//	fire9_kw_expand3x3=fopen("d://squeeze//fire9_expand3x3_weight.txt","rb");
//
//    if (NULL==fire9_kw_expand3x3) 
//	{
//        printf("Can not open file Data.txt!\n");
//        return -1;
//    }	
//	fread(ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_,147456,sizeof(float),fire9_kw_expand3x3);
//	fclose(fire9_kw_expand3x3);
	/*printf("f9 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[147455]);*/
	ProtectionLink_SqueezeNet_S1_data_ln = 1;
	ProtectionLink_SqueezeNet_S1_data_lw = iw;// 
	ProtectionLink_SqueezeNet_S1_data_lh = ih;//
	ProtectionLink_SqueezeNet_S1_b_data_ = (float*) malloc(iw * ih *3* sizeof(float));//
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_data_);
	res = ly_data_const_mean_C3(bgr, iw, ih, widthStep,ProtectionLink_SqueezeNet_S1_mean_,ProtectionLink_SqueezeNet_S1_b_data_);
	CHECK_RES(res);
	//conv1
	ProtectionLink_SqueezeNet_S1_conv1_lw = (ProtectionLink_SqueezeNet_S1_data_lw + 2*0-7)/2 + 1;
	ProtectionLink_SqueezeNet_S1_conv1_lh = (ProtectionLink_SqueezeNet_S1_data_lh + 2*0-7)/2 + 1;
	ProtectionLink_SqueezeNet_S1_conv1_ln = ProtectionLink_SqueezeNet_S1_data_ln;
	ProtectionLink_SqueezeNet_S1_b_conv1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_data_ln*96*ProtectionLink_SqueezeNet_S1_conv1_lw*ProtectionLink_SqueezeNet_S1_conv1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_conv1_);
	for(n=0;n<ProtectionLink_SqueezeNet_S1_data_ln;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_data_+n*3*ProtectionLink_SqueezeNet_S1_data_lw*ProtectionLink_SqueezeNet_S1_data_lh,
			ProtectionLink_SqueezeNet_S1_data_lw,ProtectionLink_SqueezeNet_S1_data_lh,3,1,ProtectionLink_SqueezeNet_S1_kw_data_,ProtectionLink_SqueezeNet_S1_kb_data_,
			7,7,0,0,2,2,ProtectionLink_SqueezeNet_S1_b_conv1_+n*96*ProtectionLink_SqueezeNet_S1_conv1_lw*ProtectionLink_SqueezeNet_S1_conv1_lh,96);
		CHECK_RES(res);
	}
	free(ProtectionLink_SqueezeNet_S1_b_data_);
	ProtectionLink_SqueezeNet_S1_b_data_=0;
	printf("conv1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_conv1_[0],ProtectionLink_SqueezeNet_S1_b_conv1_[1],ProtectionLink_SqueezeNet_S1_b_conv1_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_conv1_,ProtectionLink_SqueezeNet_S1_b_conv1_,ProtectionLink_SqueezeNet_S1_conv1_ln*ProtectionLink_SqueezeNet_S1_conv1_lw*ProtectionLink_SqueezeNet_S1_conv1_lh*96);
	CHECK_RES(res);
	//printf("conv1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_conv1_[0],ProtectionLink_SqueezeNet_S1_b_conv1_[1],ProtectionLink_SqueezeNet_S1_b_conv1_[2]);
	//max_pool
	ProtectionLink_SqueezeNet_S1_pool1_lh = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_conv1_lh + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool1_lw = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_conv1_lw + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool1_ln = ProtectionLink_SqueezeNet_S1_conv1_ln;
	ProtectionLink_SqueezeNet_S1_b_pool1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_pool1_ln*96*ProtectionLink_SqueezeNet_S1_pool1_lw*ProtectionLink_SqueezeNet_S1_pool1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_pool1_);
	for(n=0;n<ProtectionLink_SqueezeNet_S1_conv1_ln;n++) 
	{
		res = ly_pooling_max_float(ProtectionLink_SqueezeNet_S1_b_conv1_+n*96*ProtectionLink_SqueezeNet_S1_conv1_lw*ProtectionLink_SqueezeNet_S1_conv1_lh,ProtectionLink_SqueezeNet_S1_conv1_lw,ProtectionLink_SqueezeNet_S1_conv1_lh,96,ProtectionLink_SqueezeNet_S1_b_pool1_+n*96*ProtectionLink_SqueezeNet_S1_pool1_lw*ProtectionLink_SqueezeNet_S1_pool1_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	free(ProtectionLink_SqueezeNet_S1_b_conv1_);
	ProtectionLink_SqueezeNet_S1_b_conv1_=0;
	//printf("pool %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_pool1_[0],ProtectionLink_SqueezeNet_S1_b_pool1_[1],ProtectionLink_SqueezeNet_S1_b_pool1_[2]);
	//conv2
	ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_pool1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_pool1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_pool1_ln;
	ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_pool1_ln*16*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_pool1_+n*96*ProtectionLink_SqueezeNet_S1_pool1_lw*ProtectionLink_SqueezeNet_S1_pool1_lh,
			ProtectionLink_SqueezeNet_S1_pool1_lw,ProtectionLink_SqueezeNet_S1_pool1_lh,96,1,ProtectionLink_SqueezeNet_S1_kw_fire2_squeeze1x1_,ProtectionLink_SqueezeNet_S1_kb_fire2_squeeze1x1_,
			1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh,16);
		CHECK_RES(res);
	}
	free(ProtectionLink_SqueezeNet_S1_b_pool1_);
	ProtectionLink_SqueezeNet_S1_b_pool1_=0;
    printf("conv2 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_[2]);
	//relu
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh*16);
	CHECK_RES(res);
	//conv3
	/*printf("fire2_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire2_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire2_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire2_squeeze1x1_[1535]);*/

	ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_0_ = ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire2_expand1x1_ln*64*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh,ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh,16,1,ProtectionLink_SqueezeNet_S1_kw_fire2_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire2_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh,64);
		CHECK_RES(res);
	}
	 printf("conv3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_,ProtectionLink_SqueezeNet_S1_fire2_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh*64);
	CHECK_RES(res);
	///printf("fire2_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire2_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire2_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire2_expand1x1_[1023]);
	//conv4
	ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire2_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire2_expand3x3_ln*64*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire2_squeeze1x1_lh,16,1,
			ProtectionLink_SqueezeNet_S1_kw_fire2_expand3x3_,ProtectionLink_SqueezeNet_S1_kb_fire2_expand3x3_,3,3,1,1,1,1,
			ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_+n*64*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh,64);
		CHECK_RES(res);
	}
	printf("conv4 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_=0;
	//printf("fire2_expand3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire2_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire2_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire2_expand3x3_[9215]);
	//ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_fire2_relu_squeeze1x1_0_split_1_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_,ProtectionLink_SqueezeNet_S1_fire2_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh*64);
	CHECK_RES(res);
	
	//concat1
	ProtectionLink_SqueezeNet_S1_fire2_concat_ln = ProtectionLink_SqueezeNet_S1_fire2_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire2_concat_lw = ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire2_concat_lh = ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire2_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire2_concat_lw*ProtectionLink_SqueezeNet_S1_fire2_concat_lh*ProtectionLink_SqueezeNet_S1_fire2_concat_ln*128* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire2_concat_);
	printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire2_concat_lw,ProtectionLink_SqueezeNet_S1_fire2_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire2_concat_ = 0;
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire2_expand1x1_lh,64,
			ProtectionLink_SqueezeNet_S1_b_fire2_concat_ + n * ProtectionLink_SqueezeNet_S1_fire2_concat_lw*ProtectionLink_SqueezeNet_S1_fire2_concat_lh*128,
			&ProtectionLink_SqueezeNet_S1_fp_fire2_concat_);
		CHECK_RES(res);
		printf("concat1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire2_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire2_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire2_concat_[2]);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_+n*64*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh,64,
			ProtectionLink_SqueezeNet_S1_b_fire2_concat_ + n * ProtectionLink_SqueezeNet_S1_fire2_concat_lw*ProtectionLink_SqueezeNet_S1_fire2_concat_lh*128,
			&ProtectionLink_SqueezeNet_S1_fp_fire2_concat_);
		CHECK_RES(res);
	}
	printf("concat3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire2_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire2_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire2_concat_[2]);
	printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire2_expand3x3_lh);
	free(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_=0;
	//fire2 
	//conv5
	ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_fire2_concat_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_fire2_concat_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_fire2_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire2_concat_ln*16*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_);
	//printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire2_concat_+n*128*ProtectionLink_SqueezeNet_S1_fire2_concat_lw*ProtectionLink_SqueezeNet_S1_fire2_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire2_concat_lw,ProtectionLink_SqueezeNet_S1_fire2_concat_lh,128,1,ProtectionLink_SqueezeNet_S1_kw_fire3_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire3_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh,16);
		CHECK_RES(res);
	}
	printf("conv5 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire2_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire2_concat_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh*16);
	CHECK_RES(res);
	//printf("fire3_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire3_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire3_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire3_squeeze1x1_[2047]);
	//conv6
	ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_ln;
	
	ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire3_expand1x1_ln*64*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh,16,1,ProtectionLink_SqueezeNet_S1_kw_fire3_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire3_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh,64);
		CHECK_RES(res);
	}	
		printf("conv6 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_,ProtectionLink_SqueezeNet_S1_fire3_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh*64);
	CHECK_RES(res);
	//printf("fire3_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire3_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire3_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire3_expand1x1_[1023]);

	//conv7
	ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire3_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_ln;	
	ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire3_expand3x3_ln*64*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_+n*16*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh,16,1,ProtectionLink_SqueezeNet_S1_kw_fire3_expand3x3_,ProtectionLink_SqueezeNet_S1_kb_fire3_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_+n*64*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh,64);
		CHECK_RES(res);
	}
	printf("conv7 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_=0;	
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_,ProtectionLink_SqueezeNet_S1_fire3_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh*64);
	CHECK_RES(res);
	//printf("fire3_expand3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire3_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire3_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire3_expand3x3_[9215]);
	
	//concat2
	ProtectionLink_SqueezeNet_S1_fire3_concat_ln = ProtectionLink_SqueezeNet_S1_fire3_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire3_concat_lw = ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire3_concat_lh = ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire3_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire3_concat_lw*ProtectionLink_SqueezeNet_S1_fire3_concat_lh*ProtectionLink_SqueezeNet_S1_fire3_concat_ln*128* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire3_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire3_concat_ = 0;
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh,ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_expand1x1_lh,64,ProtectionLink_SqueezeNet_S1_b_fire3_concat_ + n * ProtectionLink_SqueezeNet_S1_fire3_concat_lw*ProtectionLink_SqueezeNet_S1_fire3_concat_lh*128,&ProtectionLink_SqueezeNet_S1_fp_fire3_concat_);
		CHECK_RES(res);
		printf("concat1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire3_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire3_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire3_concat_[2]);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_+n*64*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire3_expand3x3_lh,64,
			ProtectionLink_SqueezeNet_S1_b_fire3_concat_ + n * ProtectionLink_SqueezeNet_S1_fire3_concat_lw*ProtectionLink_SqueezeNet_S1_fire3_concat_lh*128,
			&ProtectionLink_SqueezeNet_S1_fp_fire3_concat_);
		CHECK_RES(res);
	}
	printf("concat3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire3_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire3_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire3_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_=0;		
	
	//conv8
	ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_fire3_concat_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_fire3_concat_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_fire3_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln*32*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_);
	//printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire3_concat_+n*128*ProtectionLink_SqueezeNet_S1_fire3_concat_lw*ProtectionLink_SqueezeNet_S1_fire3_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh,128,1,ProtectionLink_SqueezeNet_S1_kw_fire4_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire4_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh,32);
		CHECK_RES(res);
	}
	printf("conv8 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire3_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire3_concat_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh*32);
	CHECK_RES(res);
	printf("fire4_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire4_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire4_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire4_squeeze1x1_[4095]);
	
	//conv9
	
	ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire4_expand1x1_ln*128*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh,32,1,
			ProtectionLink_SqueezeNet_S1_kw_fire4_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire4_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_+n*128*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh,128);
		CHECK_RES(res);
	}
	printf("conv9 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_[2]);
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_,ProtectionLink_SqueezeNet_S1_fire4_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh*128);
	CHECK_RES(res);
	//printf("fire4_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire4_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire4_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire4_expand1x1_[4095]);
	//conv10
	ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire4_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire4_expand3x3_ln*128*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_);
	for(n=0;n<ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_ln;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire4_squeeze1x1_lh,32,1,ProtectionLink_SqueezeNet_S1_kw_fire4_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire4_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_+n*128*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh,128);
		CHECK_RES(res);
	}	
	printf("conv10 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_,ProtectionLink_SqueezeNet_S1_fire4_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh*128);
	CHECK_RES(res);
	//printf("f4 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire4_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire4_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire4_expand3x3_[36863]);
	
	//concat3
	ProtectionLink_SqueezeNet_S1_fire4_concat_ln = ProtectionLink_SqueezeNet_S1_fire4_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire4_concat_lw = ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire4_concat_lh = ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire4_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire4_concat_lw*ProtectionLink_SqueezeNet_S1_fire4_concat_lh*ProtectionLink_SqueezeNet_S1_fire4_concat_ln*256* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire4_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire4_concat_ = 0;

		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_+n*128*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire4_expand1x1_lh,128,ProtectionLink_SqueezeNet_S1_b_fire4_concat_ + n * ProtectionLink_SqueezeNet_S1_fire4_concat_lw*ProtectionLink_SqueezeNet_S1_fire4_concat_lh*256,&ProtectionLink_SqueezeNet_S1_fp_fire4_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_+n*128*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire4_expand3x3_lh,128,
			ProtectionLink_SqueezeNet_S1_b_fire4_concat_ + n * ProtectionLink_SqueezeNet_S1_fire4_concat_lw*ProtectionLink_SqueezeNet_S1_fire4_concat_lh*256,
			&ProtectionLink_SqueezeNet_S1_fp_fire4_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire4_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire4_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire4_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_=0;
		
	ProtectionLink_SqueezeNet_S1_pool2_lh = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_fire4_concat_lh + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool2_lw = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_fire4_concat_lw + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool2_ln = ProtectionLink_SqueezeNet_S1_fire4_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_pool2_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_pool2_ln*256*ProtectionLink_SqueezeNet_S1_pool2_lw*ProtectionLink_SqueezeNet_S1_pool2_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_pool2_);
	//printf("pool %d,%d\n ",ProtectionLink_SqueezeNet_S1_pool3_lw,ProtectionLink_SqueezeNet_S1_pool3_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_max_float(ProtectionLink_SqueezeNet_S1_b_fire4_concat_+n*256*ProtectionLink_SqueezeNet_S1_fire4_concat_lw*ProtectionLink_SqueezeNet_S1_fire4_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire4_concat_lw,ProtectionLink_SqueezeNet_S1_fire4_concat_lh,256,ProtectionLink_SqueezeNet_S1_b_pool2_+n*256*ProtectionLink_SqueezeNet_S1_pool2_lw*ProtectionLink_SqueezeNet_S1_pool2_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	printf("pool %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_pool2_[0],ProtectionLink_SqueezeNet_S1_b_pool2_[1],ProtectionLink_SqueezeNet_S1_b_pool2_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire4_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire4_concat_=0;
	
	
	//fire5
	//conv11
	ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_pool2_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_pool2_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_pool2_ln;
	ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln*32*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_);
	//printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_pool2_+n*256*ProtectionLink_SqueezeNet_S1_pool2_lw*ProtectionLink_SqueezeNet_S1_pool2_lh,
			ProtectionLink_SqueezeNet_S1_pool2_lw,ProtectionLink_SqueezeNet_S1_pool2_lh,256,1,ProtectionLink_SqueezeNet_S1_kw_fire5_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire5_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh,32);
		CHECK_RES(res);
	}
	printf("conv11 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_pool2_);
	ProtectionLink_SqueezeNet_S1_b_pool2_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh*32);
	CHECK_RES(res);
	//printf("fire5_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire5_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire5_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire5_squeeze1x1_[8191]);
	//conv12	
	ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln;
	ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire5_expand1x1_ln*128*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh,32,1,
			ProtectionLink_SqueezeNet_S1_kw_fire5_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire5_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_+n*128*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh,128);
		CHECK_RES(res);
	}
	printf("conv12 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_[2]);
		res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_,ProtectionLink_SqueezeNet_S1_fire5_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh*128);
	CHECK_RES(res);
	//printf("fire5_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire5_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire5_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire5_expand1x1_[4095]);
	//conv13
	ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire5_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire5_expand3x3_ln*128*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_);
	for(n=0;n<ProtectionLink_SqueezeNet_S1_fire5_squeeze1x1_ln;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_+n*32*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh,32,1,ProtectionLink_SqueezeNet_S1_kw_fire5_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire5_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_+n*128*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh,128);
		CHECK_RES(res);
	}	
	printf("conv13 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_,ProtectionLink_SqueezeNet_S1_fire5_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh*128);
	CHECK_RES(res);
	//printf("f5 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire5_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire5_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire5_expand3x3_[36863]);
	
	//concat4
	ProtectionLink_SqueezeNet_S1_fire5_concat_ln = ProtectionLink_SqueezeNet_S1_fire5_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire5_concat_lw = ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire5_concat_lh = ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire5_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire5_concat_lw*ProtectionLink_SqueezeNet_S1_fire5_concat_lh*ProtectionLink_SqueezeNet_S1_fire5_concat_ln*256* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire5_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire5_concat_ = 0;

		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_+n*128*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire5_expand1x1_lh,128,ProtectionLink_SqueezeNet_S1_b_fire5_concat_ + n * ProtectionLink_SqueezeNet_S1_fire5_concat_lw*ProtectionLink_SqueezeNet_S1_fire5_concat_lh*256,&ProtectionLink_SqueezeNet_S1_fp_fire5_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_+n*128*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire5_expand3x3_lh,128,
			ProtectionLink_SqueezeNet_S1_b_fire5_concat_ + n * ProtectionLink_SqueezeNet_S1_fire5_concat_lw*ProtectionLink_SqueezeNet_S1_fire5_concat_lh*256,
			&ProtectionLink_SqueezeNet_S1_fp_fire5_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire5_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire5_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire5_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_=0;
	
	
	//fire6
	//conv14
	ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_fire5_concat_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_fire5_concat_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_fire5_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln*48*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_);
	//printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire5_concat_+n*256*ProtectionLink_SqueezeNet_S1_fire5_concat_lw*ProtectionLink_SqueezeNet_S1_fire5_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire5_concat_lw,ProtectionLink_SqueezeNet_S1_fire5_concat_lh,256,1,ProtectionLink_SqueezeNet_S1_kw_fire6_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire6_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh,48);
		CHECK_RES(res);
	}
	printf("conv14 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire5_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire5_concat_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh*48);
	CHECK_RES(res);
	//printf("fire6_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire6_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire6_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire6_squeeze1x1_[12287]);

	//conv15	
	ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire6_expand1x1_ln*192*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh,48,1,
			ProtectionLink_SqueezeNet_S1_kw_fire6_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire6_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_+n*192*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh,192);
		CHECK_RES(res);
	}
	printf("conv15 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_[2]);
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_,ProtectionLink_SqueezeNet_S1_fire6_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh*192);
	CHECK_RES(res);
	//printf("fire6_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire6_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire6_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire6_expand1x1_[9215]);
	//conv16
	ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire6_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire6_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire6_expand3x3_ln*192*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh,48,1,ProtectionLink_SqueezeNet_S1_kw_fire6_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire6_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_+n*192*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh,192);
		CHECK_RES(res);
	}	
	printf("conv16 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_,ProtectionLink_SqueezeNet_S1_fire6_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh*192);
	CHECK_RES(res);
	//printf("f6 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire6_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire6_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire6_expand3x3_[82943]);
	
	//concat5
	ProtectionLink_SqueezeNet_S1_fire6_concat_ln = ProtectionLink_SqueezeNet_S1_fire6_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire6_concat_lw = ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire6_concat_lh = ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire6_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire6_concat_lw*ProtectionLink_SqueezeNet_S1_fire6_concat_lh*ProtectionLink_SqueezeNet_S1_fire6_concat_ln*384* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire6_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire6_concat_ = 0;

		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_+n*192*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire6_expand1x1_lh,192,ProtectionLink_SqueezeNet_S1_b_fire6_concat_ + n * ProtectionLink_SqueezeNet_S1_fire6_concat_lw*ProtectionLink_SqueezeNet_S1_fire6_concat_lh*384,&ProtectionLink_SqueezeNet_S1_fp_fire6_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_+n*192*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire6_expand3x3_lh,192,
			ProtectionLink_SqueezeNet_S1_b_fire6_concat_ + n * ProtectionLink_SqueezeNet_S1_fire6_concat_lw*ProtectionLink_SqueezeNet_S1_fire6_concat_lh*384,
			&ProtectionLink_SqueezeNet_S1_fp_fire6_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire6_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire6_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire6_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_=0;
	
	//fire7
	//conv17
	ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_fire6_concat_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_fire6_concat_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_fire6_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln*48*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_);
	//printf("%d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire3_squeeze1x1_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire6_concat_+n*384*ProtectionLink_SqueezeNet_S1_fire6_concat_lw*ProtectionLink_SqueezeNet_S1_fire6_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire6_concat_lw,ProtectionLink_SqueezeNet_S1_fire6_concat_lh,384,1,ProtectionLink_SqueezeNet_S1_kw_fire7_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire7_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh,48);
		CHECK_RES(res);
	}
	printf("conv17 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire6_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire6_concat_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh*48);
	CHECK_RES(res);
//printf("fire7_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire7_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire7_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire7_squeeze1x1_[18431]);
	//conv18	
	ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire7_expand1x1_ln*192*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh,48,1,
			ProtectionLink_SqueezeNet_S1_kw_fire7_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire7_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_+n*1921*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh,192);
		CHECK_RES(res);
	}
	printf("conv18 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_[2]);
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_,ProtectionLink_SqueezeNet_S1_fire7_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh*192);
	CHECK_RES(res);
	//printf("fire7_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire7_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire7_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire7_expand1x1_[9215]);
	//conv19
	ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire7_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire7_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire7_expand3x3_ln*192*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_+n*48*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh,48,1,ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire7_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_+n*192*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh,192);
		CHECK_RES(res);
	}	
	printf("conv19 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_,ProtectionLink_SqueezeNet_S1_fire7_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh*192);
	CHECK_RES(res);
	//printf("f7 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire7_expand3x3_[82943]);
	//concat6
	ProtectionLink_SqueezeNet_S1_fire7_concat_ln = ProtectionLink_SqueezeNet_S1_fire7_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire7_concat_lw = ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire7_concat_lh = ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire7_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire7_concat_lw*ProtectionLink_SqueezeNet_S1_fire7_concat_lh*ProtectionLink_SqueezeNet_S1_fire7_concat_ln*384* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire7_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire7_concat_ = 0;

		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_+n*192*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire7_expand1x1_lh,192,ProtectionLink_SqueezeNet_S1_b_fire7_concat_ + n * ProtectionLink_SqueezeNet_S1_fire7_concat_lw*ProtectionLink_SqueezeNet_S1_fire7_concat_lh*384,&ProtectionLink_SqueezeNet_S1_fp_fire7_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_+n*192*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire7_expand3x3_lh,192,
			ProtectionLink_SqueezeNet_S1_b_fire7_concat_ + n * ProtectionLink_SqueezeNet_S1_fire7_concat_lw*ProtectionLink_SqueezeNet_S1_fire7_concat_lh*384,
			&ProtectionLink_SqueezeNet_S1_fp_fire7_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire7_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire7_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire7_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_=0;
	
	//fire8
	//conv20
	ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_fire7_concat_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_fire7_concat_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_fire7_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln*64*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire7_concat_+n*384*ProtectionLink_SqueezeNet_S1_fire7_concat_lw*ProtectionLink_SqueezeNet_S1_fire7_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire7_concat_lw,ProtectionLink_SqueezeNet_S1_fire7_concat_lh,384,1,ProtectionLink_SqueezeNet_S1_kw_fire8_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire8_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh,64);
		CHECK_RES(res);
	}
	printf("conv20 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire7_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire7_concat_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh*64);
	CHECK_RES(res);
//printf("fire8_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire8_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire8_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire8_squeeze1x1_[24575]);
	//conv21	
	ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln;
	ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire8_expand1x1_ln*256*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh,64,1,
			ProtectionLink_SqueezeNet_S1_kw_fire8_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire8_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_+n*256*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh,256);
		CHECK_RES(res);
	}
	printf("conv21 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_,ProtectionLink_SqueezeNet_S1_fire8_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh*256);
	CHECK_RES(res);
	//printf("fire8_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire8_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire8_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire8_expand1x1_[16383]);
	//conv22
	ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire8_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire8_squeeze1x1_ln;	
	ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire8_expand3x3_ln*256*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh,64,1,ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire8_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_+n*256*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh,256);
		CHECK_RES(res);
	}	
	printf("conv22 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_,ProtectionLink_SqueezeNet_S1_fire8_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh*256);
	CHECK_RES(res);
	printf("f8 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire8_expand3x3_[147455]);

	//concat7
	ProtectionLink_SqueezeNet_S1_fire8_concat_ln = ProtectionLink_SqueezeNet_S1_fire8_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire8_concat_lw = ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire8_concat_lh = ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire8_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire8_concat_lw*ProtectionLink_SqueezeNet_S1_fire8_concat_lh*ProtectionLink_SqueezeNet_S1_fire8_concat_ln*512* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire8_concat_);
	
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire8_concat_ = 0;
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_+n*256*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire8_expand1x1_lh,256,ProtectionLink_SqueezeNet_S1_b_fire8_concat_ + n * ProtectionLink_SqueezeNet_S1_fire8_concat_lw*ProtectionLink_SqueezeNet_S1_fire8_concat_lh*512,&ProtectionLink_SqueezeNet_S1_fp_fire8_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_+n*256*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire8_expand3x3_lh,256,
			ProtectionLink_SqueezeNet_S1_b_fire8_concat_ + n * ProtectionLink_SqueezeNet_S1_fire8_concat_lw*ProtectionLink_SqueezeNet_S1_fire8_concat_lh*512,
			&ProtectionLink_SqueezeNet_S1_fp_fire8_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire8_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire8_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire8_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_=0;

	ProtectionLink_SqueezeNet_S1_pool3_lh = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_fire8_concat_lh + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool3_lw = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_fire8_concat_lw + 2*0-3)/2)) + 1;
	ProtectionLink_SqueezeNet_S1_pool3_ln = ProtectionLink_SqueezeNet_S1_fire8_concat_ln;
	ProtectionLink_SqueezeNet_S1_b_pool3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_pool3_ln*512*ProtectionLink_SqueezeNet_S1_pool3_lw*ProtectionLink_SqueezeNet_S1_pool3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_pool3_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_max_float(ProtectionLink_SqueezeNet_S1_b_fire8_concat_+n*512*ProtectionLink_SqueezeNet_S1_fire8_concat_lw*ProtectionLink_SqueezeNet_S1_fire8_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire8_concat_lw,ProtectionLink_SqueezeNet_S1_fire8_concat_lh,512,ProtectionLink_SqueezeNet_S1_b_pool3_+n*512*ProtectionLink_SqueezeNet_S1_pool3_lw*ProtectionLink_SqueezeNet_S1_pool3_lh,3,3,2,2,0,0);
		CHECK_RES(res);
	}
	//printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire8_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire8_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire8_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire8_concat_);
	ProtectionLink_SqueezeNet_S1_b_fire8_concat_=0;

	//fire9
	//conv23
	ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw = (ProtectionLink_SqueezeNet_S1_pool3_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh = (ProtectionLink_SqueezeNet_S1_pool3_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln = ProtectionLink_SqueezeNet_S1_pool3_ln;
	ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln*64*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_);
	
	for(n=0;n<1;n++) 
	{
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_pool3_+n*512*ProtectionLink_SqueezeNet_S1_pool3_lw*ProtectionLink_SqueezeNet_S1_pool3_lh,
			ProtectionLink_SqueezeNet_S1_pool3_lw,ProtectionLink_SqueezeNet_S1_pool3_lh,512,1,ProtectionLink_SqueezeNet_S1_kw_fire9_squeeze1x1_,
			ProtectionLink_SqueezeNet_S1_kb_fire9_squeeze1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh,64);
		CHECK_RES(res);
	}
	printf("conv23 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_pool3_);
	ProtectionLink_SqueezeNet_S1_b_pool3_=0;
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_,ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_,ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh*64);
	CHECK_RES(res);
	//printf("fire9_squeeze1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire9_squeeze1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire9_squeeze1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire9_squeeze1x1_[32765]);
	//conv24	
	ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw = (ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh = (ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh + 2*0-1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_expand1x1_ln = ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln;	
	ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire9_expand1x1_ln*256*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh,64,1,
			ProtectionLink_SqueezeNet_S1_kw_fire9_expand1x1_,ProtectionLink_SqueezeNet_S1_kb_fire9_expand1x1_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_+n*256*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh,256);
		CHECK_RES(res);
	}
	printf("conv24 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_[0],ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_[1],ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_,ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_,ProtectionLink_SqueezeNet_S1_fire9_expand1x1_ln*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh*256);
	CHECK_RES(res);
	//printf("fire9_expand1x1 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire9_expand1x1_[0],ProtectionLink_SqueezeNet_S1_kw_fire9_expand1x1_[1],ProtectionLink_SqueezeNet_S1_kw_fire9_expand1x1_[16383]);
	//conv25
	ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw = (ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh = (ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh + 2*1-3)/1 + 1;
	ProtectionLink_SqueezeNet_S1_fire9_expand3x3_ln = ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_1_ = ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire9_expand3x3_ln*256*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_+n*64*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lw,ProtectionLink_SqueezeNet_S1_fire9_squeeze1x1_lh,64,1,ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_,
			ProtectionLink_SqueezeNet_S1_kb_fire9_expand3x3_,3,3,1,1,1,1,ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_+n*256*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh,256);
		CHECK_RES(res);
	}
	printf("conv25 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_[0],ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_[1],ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_,ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_,ProtectionLink_SqueezeNet_S1_fire9_expand3x3_ln*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh*256);
	CHECK_RES(res);
	//printf("f9 3x3 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[0],ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[1],ProtectionLink_SqueezeNet_S1_kw_fire9_expand3x3_[147455]);
	//concat8
	ProtectionLink_SqueezeNet_S1_fire9_concat_ln = ProtectionLink_SqueezeNet_S1_fire9_expand1x1_ln;
	ProtectionLink_SqueezeNet_S1_fire9_concat_lw = ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw;
	ProtectionLink_SqueezeNet_S1_fire9_concat_lh = ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh;
	ProtectionLink_SqueezeNet_S1_b_fire9_concat_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_fire9_concat_lw*ProtectionLink_SqueezeNet_S1_fire9_concat_lh*ProtectionLink_SqueezeNet_S1_fire9_concat_ln*512* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_fire9_concat_);
	//printf("concat2 %d,%d\n ",ProtectionLink_SqueezeNet_S1_fire3_concat_lw,ProtectionLink_SqueezeNet_S1_fire3_concat_lh);
	for(n=0;n<1;n++) {
		ProtectionLink_SqueezeNet_S1_fp_fire9_concat_ = 0;
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_+n*256*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw*ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh,
			ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lw,ProtectionLink_SqueezeNet_S1_fire9_expand1x1_lh,256,ProtectionLink_SqueezeNet_S1_b_fire9_concat_ + n * ProtectionLink_SqueezeNet_S1_fire9_concat_lw*ProtectionLink_SqueezeNet_S1_fire9_concat_lh*512,&ProtectionLink_SqueezeNet_S1_fp_fire9_concat_);
		CHECK_RES(res);
		res = ly_concat_channels_float(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_+n*256*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw*ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh,
			ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lw,ProtectionLink_SqueezeNet_S1_fire9_expand3x3_lh,256,
			ProtectionLink_SqueezeNet_S1_b_fire9_concat_ + n * ProtectionLink_SqueezeNet_S1_fire9_concat_lw*ProtectionLink_SqueezeNet_S1_fire9_concat_lh*512,
			&ProtectionLink_SqueezeNet_S1_fp_fire9_concat_);
		CHECK_RES(res);
	}
	printf("concat %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_fire9_concat_[0],ProtectionLink_SqueezeNet_S1_b_fire9_concat_[1],ProtectionLink_SqueezeNet_S1_b_fire9_concat_[2]);
	free(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_);
	ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_=0;
	free(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_);
	ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_=0;

	//conv26
	ProtectionLink_SqueezeNet_S1_conv26_lw = (ProtectionLink_SqueezeNet_S1_fire9_concat_lw -1+2*1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_conv26_lh = (ProtectionLink_SqueezeNet_S1_fire9_concat_lh -1+2*1)/1 + 1;
	ProtectionLink_SqueezeNet_S1_conv26_ln = ProtectionLink_SqueezeNet_S1_fire9_concat_ln;
	//ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_fire3_relu_squeeze1x1_0_split_0_ = ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_;
	ProtectionLink_SqueezeNet_S1_b_conv26_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_conv26_ln*2*ProtectionLink_SqueezeNet_S1_conv26_lw*ProtectionLink_SqueezeNet_S1_conv26_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_conv26_);
	for(n=0;n<1;n++) {
		res = ly_conv_float(ProtectionLink_SqueezeNet_S1_b_fire9_concat_+n*512*ProtectionLink_SqueezeNet_S1_fire9_concat_lw*ProtectionLink_SqueezeNet_S1_fire9_concat_lh,
			ProtectionLink_SqueezeNet_S1_fire9_concat_lw,ProtectionLink_SqueezeNet_S1_fire9_concat_lh,512,1,
			ProtectionLink_SqueezeNet_S1_kw_conv26_,ProtectionLink_SqueezeNet_S1_kb_conv26_,1,1,0,0,1,1,ProtectionLink_SqueezeNet_S1_b_conv26_+n*2*ProtectionLink_SqueezeNet_S1_conv26_lw*ProtectionLink_SqueezeNet_S1_conv26_lh,2);
		CHECK_RES(res);
	}
	printf("conv26 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_b_conv26_[0],ProtectionLink_SqueezeNet_S1_b_conv26_[1],ProtectionLink_SqueezeNet_S1_b_conv26_[2]);
	res = ly_relu_float(ProtectionLink_SqueezeNet_S1_b_conv26_,ProtectionLink_SqueezeNet_S1_b_conv26_,ProtectionLink_SqueezeNet_S1_conv26_ln*ProtectionLink_SqueezeNet_S1_conv26_lw*ProtectionLink_SqueezeNet_S1_conv26_lh*2);
	CHECK_RES(res);	
	printf("conv10 %f,%f,%f\n",ProtectionLink_SqueezeNet_S1_kw_conv26_[0],ProtectionLink_SqueezeNet_S1_kw_conv26_[1],ProtectionLink_SqueezeNet_S1_kw_conv26_[1023]);
	ProtectionLink_SqueezeNet_S1_pool4_lh = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_conv26_lh + 2*0-15)/1)) + 1;
	ProtectionLink_SqueezeNet_S1_pool4_lw = (int)(ceil((float)(ProtectionLink_SqueezeNet_S1_conv26_lw + 2*0-15)/1)) + 1;
	ProtectionLink_SqueezeNet_S1_pool4_ln = ProtectionLink_SqueezeNet_S1_conv26_ln;
	ProtectionLink_SqueezeNet_S1_b_pool4_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_pool4_ln*2*ProtectionLink_SqueezeNet_S1_pool4_lw*ProtectionLink_SqueezeNet_S1_pool4_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_pool4_);
	//printf("pool %d,%d\n ",ProtectionLink_SqueezeNet_S1_pool3_lw,ProtectionLink_SqueezeNet_S1_pool3_lh);
	for(n=0;n<1;n++) 
	{
		res = ly_pooling_ave_float(ProtectionLink_SqueezeNet_S1_b_conv26_+n*2*ProtectionLink_SqueezeNet_S1_conv26_lw*ProtectionLink_SqueezeNet_S1_conv26_lh,
			ProtectionLink_SqueezeNet_S1_conv26_lw,ProtectionLink_SqueezeNet_S1_conv26_lh,2,ProtectionLink_SqueezeNet_S1_b_pool4_+n*2*ProtectionLink_SqueezeNet_S1_pool4_lw*ProtectionLink_SqueezeNet_S1_pool4_lh,15,15,1,1,0,0);
		CHECK_RES(res);
	}
	printf("pool %f,%f\n",ProtectionLink_SqueezeNet_S1_b_pool4_[0],ProtectionLink_SqueezeNet_S1_b_pool4_[1]);
	free(ProtectionLink_SqueezeNet_S1_b_conv26_);
	ProtectionLink_SqueezeNet_S1_b_conv26_=0;	
	ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lw = ProtectionLink_SqueezeNet_S1_pool4_lw;
	ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lh = ProtectionLink_SqueezeNet_S1_pool4_lh;
	ProtectionLink_SqueezeNet_S1_rpn_cls_prob_ln = ProtectionLink_SqueezeNet_S1_pool4_ln;	
	ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_ = (float*) malloc(ProtectionLink_SqueezeNet_S1_rpn_cls_prob_ln*2*ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lw*ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lh* sizeof(float));
	CHECK_PTR(ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_);
	///printf("softmax %d,%d\n ",ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lw,ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lh);
	//printf("%d,%d,%d\n",ProtectionLink_SqueezeNet_S1_rpn_cls_prob_ln,ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lw,ProtectionLink_SqueezeNet_S1_rpn_cls_prob_lh);
	res = ly_softmax(ProtectionLink_SqueezeNet_S1_b_pool4_,1*1,2,1,ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_);
	CHECK_RES(res);
	free(ProtectionLink_SqueezeNet_S1_b_pool4_);
	ProtectionLink_SqueezeNet_S1_b_pool4_=0;		

	printf("%f,%f\n",ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_[0],ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_[1]);
	if(ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_[0]>ProtectionLink_SqueezeNet_S1_b_rpn_cls_prob_[1])
	{
		*label=0;
	}
	else *label=1;
exit:
	if(ProtectionLink_SqueezeNet_S1_b_data_) {free(ProtectionLink_SqueezeNet_S1_b_data_);}
	if(ProtectionLink_SqueezeNet_S1_b_conv1_) {free(ProtectionLink_SqueezeNet_S1_b_conv1_);}
	if(ProtectionLink_SqueezeNet_S1_b_conv26_) {free(ProtectionLink_SqueezeNet_S1_b_conv26_);}
	if(ProtectionLink_SqueezeNet_S1_b_pool1_) {free(ProtectionLink_SqueezeNet_S1_b_pool1_);}
	if(ProtectionLink_SqueezeNet_S1_b_pool2_) {free(ProtectionLink_SqueezeNet_S1_b_pool2_);}
	if(ProtectionLink_SqueezeNet_S1_b_pool3_) {free(ProtectionLink_SqueezeNet_S1_b_pool3_);}
	if(ProtectionLink_SqueezeNet_S1_b_pool4_) {free(ProtectionLink_SqueezeNet_S1_b_pool4_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire2_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire2_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire2_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire2_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire2_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire3_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire3_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire3_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire3_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire3_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire4_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire4_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire4_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire4_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire4_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire5_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire5_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire5_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire5_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire5_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire6_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire6_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire6_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire6_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire6_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire7_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire7_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire7_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire7_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire7_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire8_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire8_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire8_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire8_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire8_concat_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire9_squeeze1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_) {free(ProtectionLink_SqueezeNet_S1_b_fire9_expand1x1_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_) {free(ProtectionLink_SqueezeNet_S1_b_fire9_expand3x3_);}
	if(ProtectionLink_SqueezeNet_S1_b_fire9_concat_) {free(ProtectionLink_SqueezeNet_S1_b_fire9_concat_);}
	
	return res;
}


