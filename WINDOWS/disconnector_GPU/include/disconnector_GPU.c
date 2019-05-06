#include "stdio.h"
#include "stdlib.h"
#include "disconnector_GPU.h"
#include "detector_GPU.h"

typedef struct{
    int num;
	float **Bbox;
} BboxOut;

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

image HY_ipl_to_image_GPU(DSR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
	int i, j, k, count=0;
    image out = COM_make_image_GPU(w, h, c);
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


int HYDSR_Init_GPU(void *hMemMgr, void **pMRHandle)
{
	int res = 0;
	DRParam *pSRHandle=NULL;
	network net = { 0 };
	pSRHandle=(DRParam *)malloc(1*sizeof(DRParam));
	if(pSRHandle==NULL)
	{
		res= -1;
		goto EXT;
	}
	pSRHandle->net= net;
EXT:
	*pMRHandle = pSRHandle;
	return res;
}
int HYDSR_Uninit_GPU(void *hMRHandle)
{
	DRParam *pSRParam = (DRParam*)hMRHandle;
	
	COM_free_network_GPU(pSRParam->net);

	if (pSRParam)
		free (pSRParam);
	pSRParam = NULL;
	return 0;
}
int HYDSR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h)
{
	int res=0;
	DRParam *pSRParam = (DRParam*)hMRHandle;
#ifdef GPU
	res=HY_cuda_set_device(gpu_index);
	if (res != 0)
	{
		return res;
	}
#endif
	
	pSRParam->lThresh = Thresh;
	//pSRParam->net = parse_network_cfg_adapt(cfgfile,w,h);   //自适应尺寸
	pSRParam->net = COM_parse_network_cfg_GPU(cfgfile);   //cfg中的width和height尺寸
	if(pSRParam->net.errorcode !=0)
	{
		res=pSRParam->net.errorcode;
		return res;
	}
	if(weightfile){
        COM_load_weights_GPU(&(pSRParam->net), weightfile);
		if(pSRParam->net.errorcode !=0)
		{
			res=pSRParam->net.errorcode;
			return res;
		}
    }
    COM_set_batch_network_GPU(&(pSRParam->net), 1);
	
	return res;
	
}


int HYDSR_StateRecog_GPU(void *hMRHandle,DSR_IMAGES *pImg,HYDSR_RESULT_LIST *pResultList)
{
	int res =0;
	int i=0,j=0;
	int total=1000;
    float thresh;
	image out;
	BboxOut bbox_out={0};
	
	
	DRParam *pSRParam = (DRParam*)hMRHandle;
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
    out =   HY_ipl_to_image_GPU(pImg);
	if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}
	//COM_YoloDetector(pSRParam->net,out,thresh,&bbox_out.num,bbox_out.Bbox);
	if(0!=COM_YoloDetector_GPU(pSRParam->net,out,thresh,&bbox_out.num,bbox_out.Bbox))
	{
		res=-1;
		COM_free_image_GPU(out);
		goto EXT;
	}
	COM_free_image_GPU(out);
	printf("bbox_out.num=%d\n",bbox_out.num);
	if(bbox_out.num == 0)
	{
		//printf("未找到目标\n");
		res = -1;
		goto EXT;
	}
	
	for(i = 0; i < bbox_out.num; i++)
	{
		pResultList->pResult[j].flag=1;
		pResultList->pResult[j].dVal=bbox_out.Bbox[i][5];
		pResultList->pResult[j].Target.left=bbox_out.Bbox[i][0];
		pResultList->pResult[j].Target.right=bbox_out.Bbox[i][1];
		pResultList->pResult[j].Target.top=bbox_out.Bbox[i][2];
		pResultList->pResult[j].Target.bottom=bbox_out.Bbox[i][3];
		pResultList->pResult[j].dConfidence=bbox_out.Bbox[i][4];
		j=j+1;
	}
	pResultList->lResultNum=j;
EXT:
	if (bbox_out.Bbox)
	{
		for (i = 0; i < total; ++i)
		{
			if (bbox_out.Bbox[i])
			{
				free(bbox_out.Bbox[i]);
				bbox_out.Bbox[i] = NULL;
			}
		}
		free(bbox_out.Bbox);
		bbox_out.Bbox = NULL;
	}
	

	return res;
}