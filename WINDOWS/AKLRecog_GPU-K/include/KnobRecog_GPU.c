#include "KnobRecog_GPU.h"
#include "detector_GPU.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"



#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

typedef struct 
{
	network net;
	float lThresh;
}DRParam;

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
	int first;
	int next;
	Point CPoint;
    float *KnobForm;
} sortable_KnobForm;

image HYKNOB_ipl_to_image_GPU(KR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
	int i, j, k, count=0;
    image out = COM_make_image_GPU(w, h, c);
	if(out.data==NULL)
	{
		return out;
	}

    for(k= 0; k < c; ++k){
        for(i = 0; i < h; ++i){
            for(j = 0; j < w; ++j){
                out.data[count++] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return out;
}


int HYKR_Init_GPU(void *hMemMgr, void **pMRHandle)
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
int HYKR_Uninit_GPU(void *hMRHandle)
{
	DRParam *pDRParam = (DRParam*)hMRHandle;
	//一定要注意释放模型参数

	COM_free_network_GPU(pDRParam->net);
	

	if (pDRParam)
		free (pDRParam);
	pDRParam = NULL;
	return 0;
}

int HYKR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h)
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
	pDRParam->net = COM_parse_network_cfg_adapt_GPU(cfgfile,w,h);
	//pDRParam->net = parse_network_cfg_new(cfgfile);
	if(pDRParam->net.errorcode !=0)
	{
		res=pDRParam->net.errorcode;
		return res;
	}
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

int knobcomp_GPU(const void *pa, const void *pb)
{
	float diff;
    sortable_KnobForm a = *(sortable_KnobForm *)pa;
    sortable_KnobForm b = *(sortable_KnobForm *)pb;
	if(a.CPoint.y - b.CPoint.y>(a.KnobForm[3] - a.KnobForm[2])/2)
		diff=1;
	else if(b.CPoint.y - a.CPoint.y>(a.KnobForm[3] - a.KnobForm[2])/2)
		diff=-1;
	else if(a.CPoint.x - b.CPoint.x>0)
		diff=1;
	else if(a.CPoint.x - b.CPoint.x<0)
		diff=-1;
   // float diff = a.CPoint.x - b.CPoint.x ;
    if(diff < 0) return -1;//a
    else if(diff > 0) return 1;//b
    return 0;
}

int knobboxcomp_GPU(const void *pa, const void *pb)
{
    float* a = *(float* *)pa;
    float* b = *(float* *)pb;
    float diff = a[4] - b[4] ;
    if(diff < 0) return 1;
    else if(diff > 0) return -1;
    return 0;
}

void KNOBGetOverLappingRate_GPU(MKRECT rt1,MKRECT rt2,long *minRate, long *maxRate)
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
		if (maxRate) *maxRate=lSizeOverlap*100/lSizeRt2;
		//if (minRate) *minRate = MIN(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
		//if (maxRate) *maxRate = MAX(lSizeOverlap*100/lSizeRt1, lSizeOverlap*100/lSizeRt2);
	}
	else
	{
		if (minRate) *minRate = 0;
		if (maxRate) *maxRate = 0;
	}
	return;
}

int HYKR_KnobRecog_GPU(void *hMRHandle,KR_IMAGES *pImg , HYKR_RESULT_LIST *pResultList)
{
	int res =0;
	int i=0,j=0;
	int total=100;
	double max=0.0000;
	float thresh = 0.0f;
	int group=1;
	int tmp,num=0;
	int Digitnum[10]={0};
	int normalHeight = 0;
	int lResultNum=0;
	MKRECT targetarea;
	image out ;
	//int maxindex=-1;
	int MinDist,MinDistX;
	int previousX[100];
	BboxOut bbox={0};
	sortable_KnobForm *s;
    DRParam *pDRParam = (DRParam*)hMRHandle;
	total=pResultList->lMaxResultNum;
	bbox.Bbox = (float **)calloc(total, sizeof(float *));  //free
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
			res=-1;
			for(j = 0; j < i; ++j) 
			{
				free(bbox.Bbox[j]);
			}
			free(bbox.Bbox);
			return res;
		}
	}

	if(!pImg)
	{
		res = -1;
		goto EXT;
	}
	thresh = pDRParam->lThresh;	
	out = HYKNOB_ipl_to_image_GPU(pImg);
	if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}
	bbox.num = 0;
	//printf("----------------------把手启动分析-------------------\n");
	//YoloDetector(pDRParam->net, out, thresh,0.5,&bbox.num,bbox.Bbox);
	if(0!= COM_YoloDetector_GPU(pDRParam->net, out, thresh,&bbox.num,bbox.Bbox))
	{
		res=-1;
		COM_free_image_GPU(out);
		goto EXT;
	}
	/*printf("偏移x=%d y=%d\n",pResultList->offset.x,pResultList->offset.y);
	printf("bbox.num=%d\n",bbox.num);
	printf("pResultList->lRealResultNum=%d\n",pResultList->lRealResultNum);
	for(i = 0; i < bbox.num; ++i){ 
		printf("%d:%f  left=%f right=%f top=%f bot=%f\n",i,bbox.Bbox[i][5],bbox.Bbox[i][0],bbox.Bbox[i][1],bbox.Bbox[i][2],bbox.Bbox[i][3]);
	}*/
	qsort(bbox.Bbox, bbox.num, sizeof(float*), knobboxcomp_GPU);
	COM_free_image_GPU(out);
	
	s = (sortable_KnobForm*)calloc(bbox.num, sizeof(sortable_KnobForm));
	if(s==NULL)
	{
		res=-1;
		goto EXT;
	}

	if(bbox.num < 1){		//
		res = -1;
		goto EXT;
	}

	normalHeight = bbox.Bbox[0][3]-bbox.Bbox[0][2];
	j=0;
	for(i = 0; i < bbox.num; ++i){ 
		//
		float a;
		s[i].KnobForm =bbox.Bbox[i]; 
		s[i].CPoint.x = (s[i].KnobForm[0]+s[i].KnobForm[1])/2 ;
		s[i].CPoint.y = (s[i].KnobForm[2]+s[i].KnobForm[3])/2 ;
	    a=fabs(1-(1.0*normalHeight)/(s[i].KnobForm[3]-s[i].KnobForm[2]));
		//if(fabs(a)>0.3)
		{
			//printf("-1\n");
			//s[i].flag = -1;
		}
		//else
		{
			s[i].flag = 0;//选取实际数目个目标
			j=j+1;
			if(j> pResultList->lRealResultNum)s[i].flag=-1;
		}
	}
	
	qsort(s, bbox.num, sizeof(sortable_KnobForm), knobcomp_GPU);  //从左到右排序,从上到下排序

	//for(i = 0; i < bbox.num; ++i){   //Demo
	//	pResultList->pResult[i].flag=1;
	//	pResultList->pResult[i].dVal=s[i].KnobForm[5];
	//	pResultList->pResult[i].Target.left=s[i].KnobForm[0];
	//	pResultList->pResult[i].Target.right=s[i].KnobForm[1];
	//	pResultList->pResult[i].Target.top=s[i].KnobForm[2];
	//	pResultList->pResult[i].Target.bottom=s[i].KnobForm[3];
	//}
//////////////////////////////////////////////////////////// //work
	for(j = 0; j < pResultList->lRealResultNum; ++j)
	{
		long lMaxRate=0;
		int previous=-1;
		targetarea.left=pResultList->LRArea[j].left;//+pResultList->offset.x;
		targetarea.right=pResultList->LRArea[j].right;//+pResultList->offset.x;
		targetarea.top=pResultList->LRArea[j].top;//+pResultList->offset.y;
		targetarea.bottom=pResultList->LRArea[j].bottom;//+pResultList->offset.y;
		pResultList->pResult[j].flag=0;
		for(i = 0; i < bbox.num; ++i){
			MKRECT tmpArea;
			long tmpRate;
			long tmpRate1;
			long tmpRate2;
			tmpArea.left=s[i].KnobForm[0];
			tmpArea.right=s[i].KnobForm[1];
			tmpArea.top=s[i].KnobForm[2];
			tmpArea.bottom=s[i].KnobForm[3];
			KNOBGetOverLappingRate_GPU(targetarea,tmpArea,NULL,&tmpRate1);
			KNOBGetOverLappingRate_GPU(tmpArea,targetarea,NULL,&tmpRate2);
			tmpRate=MAX(tmpRate1,tmpRate2);
			//printf("rate=%d\n",tmpRate);
			if(tmpRate > 20 && tmpRate > lMaxRate)
			{
				pResultList->pResult[j].flag=1;
				pResultList->pResult[j].dVal=s[i].KnobForm[5];
				pResultList->pResult[j].Target.left=s[i].KnobForm[0];
				pResultList->pResult[j].Target.right=s[i].KnobForm[1];
				pResultList->pResult[j].Target.top=s[i].KnobForm[2];
				pResultList->pResult[j].Target.bottom=s[i].KnobForm[3];
				lMaxRate=tmpRate;
			}
		}
		lResultNum+=1;

	}
//////////////////////////////////////////////////////
	pResultList->lResultNum=pResultList->lRealResultNum;

	//pResultList->lResultNum=bbox.num;
	

EXT:
	for(i = 0; i < total; ++i) 
	{
		free(bbox.Bbox[i]);
	}
	free(bbox.Bbox);
	if(s)
	free(s);
	

	return res;
}


