#include "SwitchRecog.h"
#include "detector.h"
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
typedef struct{
    int num;
	float **Bbox;
} BboxOut;

typedef struct 
{
	network net;
	float lThresh;
}SRParam;
typedef struct 
{
	int x;
	int y;
}Point;
typedef struct{
	int flag;
	int next;
	Point CPoint;
    float *bbox;
} DBbox;

image HYS_ipl_to_image(SR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
	int i, j, k, count=0;
    image out = COM_make_image(w, h, c);
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


int HYSR_Init(void *hMemMgr, void **pMRHandle)
{
	int res = 0;
	SRParam *pSRHandle=NULL;
	network net = { 0 };
	pSRHandle=(SRParam *)malloc(1*sizeof(SRParam));
	if(pSRHandle==NULL)
	{
		res= -1;
		goto EXT;
	}
	pSRHandle->net = net;
EXT:
	*pMRHandle = pSRHandle;
	return res;
}
int HYSR_Uninit(void *hMRHandle)
{
	SRParam *pSRParam = (SRParam*)hMRHandle;
	COM_free_network(pSRParam->net);


	if (pSRParam)
		free (pSRParam);
	pSRParam = NULL;
	return 0;
}
int HYSR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh)
{
	int res=0;
	SRParam *pSRParam = (SRParam*)hMRHandle;
#ifdef GPU
	res = HY_cuda_set_device(gpu_index);
	if (res != 0)
	{
		return res;
	}
#endif
	pSRParam->lThresh = Thresh;
	pSRParam->net = COM_parse_network_cfg(cfgfile);
	if(pSRParam->net.errorcode !=0)
	{
		res=pSRParam->net.errorcode;
		return res;
	}
	if(weightfile){
		COM_load_weights(&(pSRParam->net), weightfile);
		if(pSRParam->net.errorcode !=0)
		{
			res=pSRParam->net.errorcode;
			return res;
		}
    }
	COM_set_batch_network(&(pSRParam->net), 1);
EXT:

	return res;
	
}
int TPointDis(Point a,Point b)
{
	return sqrt(pow((a.x-b.x),2.0)+pow((a.y-b.y),2.0));
}


int HYSR_SwitchRecog(void *hMRHandle,SR_IMAGES *pImg,HYSR_RESULT *result)
{
	int res =0;
	int i=0,j=0;
	int total=10;
	double max=0.0000;
	int group=1;
	int Maxindex=0;
	int MinDist,MinDistX;
	float MaxProb;
    float thresh;
	image out;
	BboxOut bbox_out={0};
	
	//sortable_DigitForm *s;
	SRParam *pSRParam = (SRParam*)hMRHandle;
	bbox_out.Bbox = (float**)calloc(total, sizeof(float *));  //free
	if(bbox_out.Bbox==NULL)
	{
		res=-1;
		return res;
	}
	for(i = 0; i < total; ++i) 
	{
		bbox_out.Bbox[i] = (float*)calloc(6, sizeof(float));
		if(bbox_out.Bbox[i]==NULL)
		{
			for(j = 0; j < i; ++j)
			{
				free(bbox_out.Bbox[j]);
			}
			free(bbox_out.Bbox);
			res=-1;
			return res;
		}
	}
	//yolo检测输出
	thresh = pSRParam->lThresh;	

	if(!pImg)
	{
		res = -1;
		goto EXT;
	}
    out =   HYS_ipl_to_image(pImg);
	if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}

	if(0!= COM_YoloDetector(pSRParam->net,out,thresh,&bbox_out.num,bbox_out.Bbox))
	{
		res=-1;
		COM_free_image(out);
		goto EXT;
	}

	COM_free_image(out);

	if(bbox_out.num == 0)
	{
		printf("未找到开关\n");
		res = -1;
		goto EXT;
	}
	

	MaxProb = bbox_out.Bbox[0][4];
	for(i = 1;i<bbox_out.num; i++)
	{
		if(bbox_out.Bbox[i][4]>MaxProb)
		{
			MaxProb= bbox_out.Bbox[i][4];
			Maxindex =i;
		}
	}

	
	result->rtTarget.left = bbox_out.Bbox[Maxindex][0];
	result->rtTarget.top = bbox_out.Bbox[Maxindex][2];
	result->rtTarget.right = bbox_out.Bbox[Maxindex][1];
	result->rtTarget.bottom = bbox_out.Bbox[Maxindex][3];
	result->dlabel = bbox_out.Bbox[Maxindex][5];
	result->dConfidence = MaxProb;
EXT:

	for(i = 0; i < total; ++i)
	{
		free(bbox_out.Bbox[i]);
	}
	free(bbox_out.Bbox);

	return res;
}


