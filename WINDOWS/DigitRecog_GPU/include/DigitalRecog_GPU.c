#include <stdio.h>
#include <stdlib.h>
#include "DigitalRecog_GPU.h"
//#include "opencv/cv.h"
//#include "opencv/cxcore.h"

#include "detector_GPU.h"
#include "math.h"
/*#ifdef _DEBUG
#pragma comment(lib, "../Debug/yolo.lib")

#else
#pragma comment(lib, "../Release/yolo.lib")*/    //64 or 32
//#pragma comment(lib, "../Release/yolo_cpp_dll_no_gpu.lib")
//#pragma comment(lib, "../Release/opencv_core244.lib")
//#pragma comment(lib, "../Release/opencv_highgui244.lib")
//#pragma comment(lib, "../Release/opencv_imgproc244.lib")
//#endif


#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef ABS
#  define ABS(x) (((x)+((x)>>31))^((x)>>31))
#endif
#ifndef CEIL_4
#define CEIL_4(x)			(( (x) + 3 ) & (~3) )
#endif
typedef struct 
{
	 network net;
	float lThresh;
} DRParam;

typedef struct 
{
	int x;
	int y;
}Point;

typedef struct{
    int num;
	float **Bbox;
} BboxOut;


typedef struct{
	int flag;
	int next;
	Point CPoint;
    float *DigitForm;
} sortable_DigitForm;

image HYDIG_ipl_to_image_GPU(DR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
	int i, j, k, count=0;
    image out = COM_make_image_GPU(w, h, c);  //out.data = calloc(h*w*c, sizeof(float));
	if(out.data==NULL)
		return out;

    for(k= 0; k < c; ++k){
        for(i = 0; i < h; ++i){
            for(j = 0; j < w; ++j){
                out.data[count++] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return out;
}


int HYDR_Init_GPU(void *hMemMgr, void **pMRHandle)
{
	int res = 0;

	DRParam *pDRHandle=NULL;
	network net = { 0 };
	pDRHandle=(DRParam *)malloc(1*sizeof(DRParam));
	if (!pDRHandle)
	{
		res= -1;
		goto EXT;
	}
	pDRHandle->net = net;
EXT:
	*pMRHandle = pDRHandle;
	return res;
}
int HYDR_Uninit_GPU(void *hMRHandle)
{
	DRParam *pDRParam = (DRParam*)hMRHandle;
	//一定要注意释放模型参数

	COM_free_network_GPU(pDRParam->net);
	

	if (pDRParam)
		free (pDRParam);
	pDRParam = NULL;
	return 0;
}

int HYDR_SetParam_GPU(void *hMRHandle,const char *cfgfile,const char *weightfile,float Thresh,int gpu_index,int w,int h)
{
	int res=0;
	DRParam *pDRParam = (DRParam*)hMRHandle;
#ifdef GPU
	res = HY_cuda_set_device(gpu_index);
	if (res != 0)
	{
		return res;
	}
#endif
	pDRParam->lThresh = Thresh;

	//pDRParam->net = parse_network_cfg_adapt(cfgfile, w, h);
	
	pDRParam->net = COM_parse_network_cfg_GPU(cfgfile);
    if(pDRParam->net.errorcode !=0)
	{
		res=pDRParam->net.errorcode;
		return res;
	}
	//pDRParam->net = parse_network_cfg(cfgfile);
	if(weightfile){
		COM_load_weights_GPU(&(pDRParam->net), weightfile);
		 if(pDRParam->net.errorcode !=0)
		{
			res=pDRParam->net.errorcode;
			return res;
		}
    }
	COM_set_batch_network_GPU(&(pDRParam->net), 1);
EXT:

	return res;
}


int TPointDis_GPU(Point a,Point b)
{
	return sqrt(pow((a.x-b.x),2.0)+pow((a.y-b.y),2.0));
}
int comp_GPU(const void *pa, const void *pb)
{
    sortable_DigitForm a = *(sortable_DigitForm *)pa;
    sortable_DigitForm b = *(sortable_DigitForm *)pb;
    float diff = a.CPoint.x - b.CPoint.x ;
    if(diff < 0) return -1;
    else if(diff > 0) return 1;
    return 0;
}
int boxcomp_GPU(const void *pa, const void *pb)
{
    float* a = *(float* *)pa;
    float* b = *(float* *)pb;
    float diff = a[4] - b[4] ;
    if(diff < 0) return 1;
    else if(diff > 0) return -1;
    return 0;
}
void GetOverLappingRate_GPU(DRRECT rt1,DRRECT rt2,long *minRate, long *maxRate)
{
	long lSizeRt1, lSizeRt2;
	long lMaxYDist = MAX(rt2.bottom,rt1.bottom)-MIN(rt2.top,rt1.top);
	long lMaxXDist = MAX(rt2.right,rt1.right)-MIN(rt2.left,rt1.left);
	if (!minRate && !maxRate) return;

	lSizeRt1 = (rt1.bottom-rt1.top)*(rt1.right-rt1.left);
	lSizeRt2 = (rt2.bottom-rt2.top)*(rt2.right-rt2.left);

	if (lSizeRt1<=0 || lSizeRt2 <=0) 
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 100;
	}

	///如果两个矩形有交集，则距离为0
	if (lMaxXDist<= (rt2.right-rt2.left+rt1.right-rt1.left)
	&& lMaxYDist <= (rt2.bottom-rt2.top+rt1.bottom-rt1.top))
	{

		long lSizeOverlap = (MIN(rt2.right, rt1.right)-MAX(rt2.left, rt1.left))
						*(MIN(rt2.bottom,rt1.bottom)-MAX(rt2.top, rt1.top));
		if (minRate) *minRate = MIN(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
		if (maxRate) *maxRate = MAX(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
	}
	else
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 0;
	}
	return;
}

int HYDR_DigitRecog_GPU(void *hMRHandle,DR_IMAGES *pImg , HYDR_RESULT_LIST *pResultList)
{
	int res =0;
	int i=0,j=0;
	int x,y;
	int total=10;
	double max=0.0000;
	float thresh = 0.0f;
	int group=1;
	int tmp,num=0;
	int Digitnum[10]={0};
	int normalHeight = 0;
	DRRECT rtArea1,rtArea2;
	image out ;
	//int maxindex=-1;
	int MinDist,MinDistX;
	int beta=0;
	int rednum=0;
	int  maxgrad;
	double* R;
	double* G;
	double* B;
	MHandle hMemMgr;
	BboxOut bbox={0};
	sortable_DigitForm *s;
    DRParam *pDRParam = (DRParam*)hMRHandle;

	
	
	bbox.Bbox = (float **)calloc(total, sizeof(float *));  //1m
	if(bbox.Bbox==NULL)
	{
		res=-1;
		return res;
	}
	for(i = 0; i < total; ++i)
	{
		bbox.Bbox[i] = (float*)calloc(6, sizeof(float));
		if(bbox.Bbox[i]==NULL)
		{
			for(j = 0; j < i; ++j)
			{
				free(bbox.Bbox[j]);
			}
			free(bbox.Bbox);
			res=-1;
			return res;
		}
	}

	thresh = pDRParam->lThresh;	
	out = HYDIG_ipl_to_image_GPU(pImg);//2m
	if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}
	bbox.num = 0;
	//printf("out.w=%d out.h=%d\n",out.w,out.h);

	//YoloDetector(pDRParam->net, out, thresh,&bbox.num,bbox.Bbox);

	if(0 != COM_YoloDetector_GPU(pDRParam->net, out, thresh,&bbox.num,bbox.Bbox))
	{
		//printf("error:0 > YoloDetector\n");
		res=-2;
		COM_free_image_GPU(out);
		goto EXT;
	}
	//printf("0:bbox.num=%d\n",bbox.num);
	qsort(bbox.Bbox, bbox.num, sizeof(float*), boxcomp_GPU);
	bbox.num=(bbox.num>6)?6:bbox.num;
	/*
	if(bbox.num>6)
	{
		bbox.Bbox[5][4];
	}*/
	//
	COM_free_image_GPU(out);//2f
	s = (sortable_DigitForm*)calloc(bbox.num, sizeof(sortable_DigitForm));//3m
	if(s==NULL)
	{
		res=-1;
		goto EXT;
	}
	/*B = (double*)calloc(bbox.num, sizeof(double));
	G = (double*)calloc(bbox.num, sizeof(double));
	R = (double*)calloc(bbox.num, sizeof(double));*/
	
	if(bbox.num < 2){	
		//printf("error:bbox.num=%d\n",bbox.num);
		res = -1;
		goto EXT;
	}

	normalHeight = bbox.Bbox[0][3]-bbox.Bbox[0][2];
	
	//printf("bbox.num=%d\n",bbox.num);
	for(i = 0; i < bbox.num; ++i){ 
		//排除异常高度,重叠区域
		float a;
		long lMaxRate;
		int step=pImg->pixelArray.chunky.lLineBytes/sizeof(unsigned char);
		int channels=3;
		int x,y;
		int whitepixelnum=0,redpixelnum=0,pixelnum=0;
		unsigned char* data=(unsigned char *)pImg->pixelArray.chunky.pPixel;
		if(s[i].flag < 0)continue;
		s[i].DigitForm =bbox.Bbox[i]; 
		s[i].CPoint.x = (s[i].DigitForm[0]+s[i].DigitForm[1])/2 ;
		s[i].CPoint.y = (s[i].DigitForm[2]+s[i].DigitForm[3])/2 ;
		rtArea1.left=s[i].DigitForm[0];
		rtArea1.right=s[i].DigitForm[1];
		rtArea1.top=s[i].DigitForm[2];
		rtArea1.bottom=s[i].DigitForm[3];
		for(j=0;j<bbox.num;j++){
			if(i==j)continue;
			if(s[j].flag < 0)continue;
			s[j].DigitForm =bbox.Bbox[j];
			rtArea2.left=s[j].DigitForm[0];
			rtArea2.right=s[j].DigitForm[1];
			rtArea2.top=s[j].DigitForm[2];
			rtArea2.bottom=s[j].DigitForm[3];
			lMaxRate=0;
			GetOverLappingRate_GPU(rtArea1,rtArea2,NULL,&lMaxRate);
			//if(lMaxRate > 0)printf("lMaxRate=%d\n",lMaxRate);
			if(lMaxRate > 25)
			{
				if(bbox.Bbox[j][4] < bbox.Bbox[i][4])
					s[j].flag = -2;
				else
					s[i].flag = -2;
			}
		}
		
		/***********
		for(y=rtArea1.top;y<rtArea1.bottom-1;y++)
		{
			for(x=rtArea1.left;x<rtArea1.right-1;x++)
			{
				B[i]+=data[y*step+x*channels+0];
				G[i]+=data[y*step+x*channels+1];
				R[i]+=data[y*step+x*channels+2];	
				pixelnum+=1;
			}
		}
		if(pixelnum > 0)
		{
			B[i]=B[i]/pixelnum;
			G[i]=G[i]/pixelnum;
			R[i]=R[i]/pixelnum;
		}

		if(s[i].DigitForm[5]==1)
		{
			B[i]=B[i]*1.2;
			G[i]=G[i]*1.2;
			R[i]=R[i]*1.2;
		}
/***********/
		/*if(s[i].flag >= 0)
		{
			a=fabs(1-(1.0*normalHeight)/(s[i].DigitForm[3]-s[i].DigitForm[2]));
			if(fabs(a)>0.3)
			{
				printf("a=%f,高度异常\n",a);
				s[i].flag = -3;
			}
			else
			{
				s[i].flag = 0;
				
			}
		}*/
	}
	//for(i = 0; i < bbox.num; ++i){
	//	printf("s[%d].DigitForm[5]=%.2f %f\n",i,s[i].DigitForm[5],s[i].DigitForm[4]);
	//	//if(s[i].DigitForm[5]==1)
	//	{
	//		printf("w=%f  h=%f\n",s[i].DigitForm[1]-s[i].DigitForm[0],s[i].DigitForm[3]-s[i].DigitForm[2]);
	//	}
	//}
	//printf("排除异常颜色\n");
	/***********
	for(i = 0; i < bbox.num; ++i){ 
		if(s[i].flag < 0)continue;
		for(j=0;j<bbox.num;j++){
			long tmp=0,dist=0;
			int color_dist;
			if(i>=j)continue;
			if(s[j].flag < 0)continue;
			tmp=R[i]-R[j];
			dist=tmp*tmp;
			tmp=G[i]-G[j];
			dist+=tmp*tmp;
			tmp=B[i]-B[j];
			dist+=tmp*tmp;
			tmp=(dist*128)/((R[j]+G[j]+B[j])/3+16);
			//printf("i=%d,j=%d,dist=%d,tmp=%d\n",i,j,dist,tmp);
			color_dist=1024*10*7/8;
			//printf("color_dist=%d\n",color_dist);
			if(tmp>color_dist)
			{
				//printf("%f:颜色异常\n",s[j].DigitForm[5]);
				s[j].flag=-1;
			}
		}
	}
	/***********/
	//printf("完成排除异常颜色\n");
	
	//printf("排除异常梯度\n");
	/***********
	maxgrad=0;
	for(i = 0; i < bbox.num; ++i)
	{ 
		MByte* pSrcData;
		MLong lSrcLine;
		MDWord dwSum, dwMaxGrad;
		
		MLong lGradLine;
		int lWidth;
		int lHeight;
		int left;
		int right;
		int top;
		int bottom;
		IplImage *tmp=NULL;
		IplImage *roi=NULL;
		CvScalar mean;
		CvScalar std_dev;
		short* Gx=MNull, *Gy=MNull;
		if(s[i].flag<0)
			continue;
		roi=cvCreateImage(cvSize(pImg->lWidth,pImg->lHeight), IPL_DEPTH_8U, 3);
		roi->imageData=pImg->pixelArray.chunky.pPixel;
		pResultList->pResult[i+1].dVal=s[i].DigitForm[5];
		pResultList->pResult[i+1].Target.left=s[i].DigitForm[0];
		pResultList->pResult[i+1].Target.right=s[i].DigitForm[1];
		pResultList->pResult[i+1].Target.top=s[i].DigitForm[2];
		pResultList->pResult[i+1].Target.bottom=s[i].DigitForm[3];
		//printf("%d:right=%f left=%f bottom=%f top=%f\n",i,s[i].DigitForm[1],s[i].DigitForm[0],s[i].DigitForm[3],s[i].DigitForm[2]);
	    left=s[i].DigitForm[0];
		right=s[i].DigitForm[1];
		top=s[i].DigitForm[2];
		bottom=s[i].DigitForm[3];
		
		cvSetImageROI(roi,cvRect(left,top,right-left,bottom-top));
		tmp = cvCreateImage(cvSize(roi->roi->width,roi->roi->height), roi->depth,1);
		lWidth=right-left;
		lHeight=bottom-top;
		cvCvtColor(roi,tmp,CV_BGR2GRAY);
		//printf("w=%d h=%d\n",roi->width,roi->height);
		//printf("w=%d h=%d\n",roi->roi->width,roi->roi->height);
		//printf("w=%d h=%d\n",tmp->width,tmp->height);
		//cvCopy(roi,tmp,0);
		cvResetImageROI(roi);
		lGradLine = JMemLength(right-left);
		pSrcData=tmp->imageData;
		lSrcLine=tmp->widthStep;
		Gx = (short*)malloc((lGradLine*lHeight)*sizeof(short));
		Gy = (short*)malloc((lGradLine*lHeight)*sizeof(short));
		dwMaxGrad=0;
		Gradient_C(pSrcData, lSrcLine, Gx, Gy, lGradLine, MNull, 0,
		lWidth, lHeight, &dwSum, &dwMaxGrad, MNull);
		//printf("%f  %d\n",s[i].DigitForm[5],dwMaxGrad);
		//cvAvgSdv(tmp,&mean,&std_dev,0);
		
		//printf("%f\n",std_dev.val[0]);
		if(i==0){
			maxgrad=dwMaxGrad;
			continue;
		}
		else if(dwMaxGrad < 0.5*maxgrad)
		{
			//printf("%d %f\n",dwMaxGrad,0.2*maxgrad);
			s[i].flag=-1;
			//printf("dwMaxGrad1111111111111111111111\n");
		}
		//if(s[i].DigitForm[4] >= 0.6)
		//{
		//	minstd=MIN(minstd,std_dev.val[0]);
		//}
		if(Gx)
		free(Gx);
		if(Gy)
		free(Gy);
		cvReleaseImage(&tmp);
		cvReleaseImage(&roi);
	}
	/***********/
	//printf("完成排除异常梯度\n");
	//printf("bbox.num=%d\n",bbox.num);
	qsort(s, bbox.num, sizeof(sortable_DigitForm), comp_GPU);  //从左到右排序
	/*for(i = 0; i < bbox.num; ++i){
		printf("num=%f,percent=%f\n",bbox.Bbox[i][5],bbox.Bbox[i][4]);
	}*/


	for(i = 0; i < bbox.num; ++i){
		if(s[i].flag<0)
			continue;
		if(s[i].flag==0)
		{
			s[i].flag=group;
			group=group+1;
		}
		MinDist=0x7FFFFFFF,MinDistX=-1;
		for(j = 0; j < bbox.num; ++j)
		{  
			if(i==j)continue;
			if(s[j].flag!=0)continue;
			if(TPointDis_GPU(s[i].CPoint,s[j].CPoint) < MinDist){
				MinDist=TPointDis_GPU(s[i].CPoint,s[j].CPoint);
				MinDistX=j;
			}
		}
		if(MinDist < 2*normalHeight){
			s[i].next=MinDistX;
			s[MinDistX].flag=s[i].flag;
		}
	}
	/*for(i = 0; i < bbox.num; ++i)
	{
		printf("%f %f\n",s[i].DigitForm[5],s[i].DigitForm[4]);
	}*/
	/*for(i = 0; i < bbox.num; ++i)
	{
		if(s[i].DigitForm[4] < 0.6)
		{
			int left;
			int right;
			int top;
			int bottom;
			IplImage *tmp=NULL;
			IplImage *roi=NULL;
			CvScalar mean;
			CvScalar std_dev;
			roi=cvCreateImage(cvSize(pImg->lWidth,pImg->lHeight), IPL_DEPTH_8U, 3);
			roi->imageData=pImg->pixelArray.chunky.pPixel;
			pResultList->pResult[i+1].dVal=s[i].DigitForm[5];
			pResultList->pResult[i+1].Target.left=s[i].DigitForm[0];
			pResultList->pResult[i+1].Target.right=s[i].DigitForm[1];
			pResultList->pResult[i+1].Target.top=s[i].DigitForm[2];
			pResultList->pResult[i+1].Target.bottom=s[i].DigitForm[3];
			left=s[i].DigitForm[0];
			right=s[i].DigitForm[1];
			top=s[i].DigitForm[2];
			bottom=s[i].DigitForm[3];
			tmp = cvCreateImage(cvSize(right-left,bottom-top), roi->depth, roi->nChannels);
			cvSetImageROI(roi,cvRect(left,top,right-left,bottom-top));
			cvCopy(roi,tmp,0);
			cvResetImageROI(roi);
			cvAvgSdv(tmp,&mean,&std_dev,0);
			if(std_dev.val[0] < 0.5*minstd)
			{
				s[j].flag==-1;
				printf("1111111111111111111111\n");
			}
			cvReleaseImage(&tmp);
			cvReleaseImage(&roi);
		}
	}*/

	for(i = 0;i<group-1;++i){
		tmp=0;
		for(j = 0; j < bbox.num; ++j){
			if(s[j].flag<0)continue;
			if(s[j].flag!=i+1)continue;
			tmp=tmp+1;
			/*if(s[j].DigitForm[5] > 9)
			{
				pResultList->pResult[pResultList->lResultNum].dVal=s[j].DigitForm[5]-10;
				beta+=1;
			}
			else
				pResultList->pResult[pResultList->lResultNum].dVal=s[j].DigitForm[5];*/
			pResultList->pResult[pResultList->lResultNum].dVal=s[j].DigitForm[5];
			pResultList->pResult[pResultList->lResultNum].Target.left=s[j].DigitForm[0];
			pResultList->pResult[pResultList->lResultNum].Target.right=s[j].DigitForm[1];
			pResultList->pResult[pResultList->lResultNum].Target.top=s[j].DigitForm[2];
			pResultList->pResult[pResultList->lResultNum].Target.bottom=s[j].DigitForm[3];
			while(s[j].next > 0){
				/*if(beta >= 1)
				{
					if(s[s[j].next].DigitForm[5] > 9)
						printf("error digitalrecog\n");
					else
					{
						pResultList->pResult[pResultList->lResultNum].dVal=pResultList->pResult[pResultList->lResultNum].dVal*10+s[s[j].next].DigitForm[5];
						beta+=1;
					}
				}
				else
				{
					if(s[s[j].next].DigitForm[5] > 9)
					{
						beta+=1;
						pResultList->pResult[pResultList->lResultNum].dVal=pResultList->pResult[pResultList->lResultNum].dVal*10+s[s[j].next].DigitForm[5]-10;
					}
					else
						pResultList->pResult[pResultList->lResultNum].dVal=pResultList->pResult[pResultList->lResultNum].dVal*10+s[s[j].next].DigitForm[5];
				}*/
				if(s[s[j].next].flag<0)
				{
					j=s[j].next;
					continue;
				}
				pResultList->pResult[pResultList->lResultNum].dVal=pResultList->pResult[pResultList->lResultNum].dVal*10+s[s[j].next].DigitForm[5];
				pResultList->pResult[pResultList->lResultNum].Target.left = MIN(pResultList->pResult[pResultList->lResultNum].Target.left,s[s[j].next].DigitForm[0]);
				pResultList->pResult[pResultList->lResultNum].Target.right = MAX(pResultList->pResult[pResultList->lResultNum].Target.right,s[s[j].next].DigitForm[1]);
				pResultList->pResult[pResultList->lResultNum].Target.top = MIN(pResultList->pResult[pResultList->lResultNum].Target.top,s[s[j].next].DigitForm[2]);
				pResultList->pResult[pResultList->lResultNum].Target.bottom = MAX(pResultList->pResult[pResultList->lResultNum].Target.bottom,s[s[j].next].DigitForm[3]);
				tmp=tmp+1;
				j=s[j].next;
			}
			  //printf("%f  %d\n",pResultList->pResult[pResultList->lResultNum].dVal,beta);
			/*if(beta != 0)
				pResultList->pResult[pResultList->lResultNum].dVal=1.0*pResultList->pResult[pResultList->lResultNum].dVal/pow(10,(beta-1));
			printf("val=%f",pResultList->pResult[pResultList->lResultNum].dVal);*/
			if(tmp >=3)
				pResultList->lResultNum+=1;
			break;
			
		}
	}
	//printf("%f\n",pResultList->pResult[0].dVal);
EXT:
	for(i = 0; i < total; ++i) //1f
	{
	
		if(bbox.Bbox[i])
		{
			free(bbox.Bbox[i]);
			bbox.Bbox[i]=NULL;
			
		}
		/*else 
		{
			printf("free fail\n");
			break;
		}*/
	
	}
	if(bbox.Bbox)
	{
		free(bbox.Bbox);
		bbox.Bbox=NULL;
	}
	if(s)
	{
		free(s);//3f
		s=NULL;
	}
	//if(B)
	//{
	//	free(B);//3f
	//	B=NULL;
	//}
	//if(G)
	//{
	//	free(G);//3f
	//	G=NULL;
	//}
	//if(R)
	//{
	//	free(R);//3f
	//	R=NULL;
	//}

	

	return res;
}


