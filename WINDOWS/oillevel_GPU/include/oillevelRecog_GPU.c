#include "oillevelRecog_GPU.h"
#include "detector_GPU.h"
#include <stdio.h>
#include <stdlib.h>
//#ifdef _DEBUG
//#pragma comment(lib, "../Debug/yolo_cpp_dll_no_gpu.lib")
//
//#else
//#pragma comment(lib, "../Release/yolo_cpp_dll_no_gpu.lib")
//
//#endif

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

image HYOIL_ipl_to_image_GPU(OLR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
    image out = COM_make_image_GPU(w, h, c);
	int i, j, k, count=0;
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


int HYOLR_Init_GPU(void *hMemMgr, void **pMRHandle)
{
	int res = 0;
	DRParam *pSRHandle=NULL;
	network net = { 0 };
	pSRHandle=(DRParam *)malloc(1*sizeof(DRParam));
	if (!pSRHandle)
	{
		res= -1;
		goto EXT;
	}
	pSRHandle->net = net;
EXT:
	*pMRHandle=pSRHandle;
	return res;
}
int HYOLR_Uninit_GPU(void *hMRHandle)
{
	DRParam *pSRParam = (DRParam*)hMRHandle;
	COM_free_network_GPU(pSRParam->net);


	if (pSRParam)
		free (pSRParam);
	pSRParam = NULL;
	return 0;
}
int HYOLR_SetParam_GPU(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int gpu_index,int w,int h)
{
	int res=0;
	DRParam *pSRParam = (DRParam*)hMRHandle;
#ifdef GPU
	res = HY_cuda_set_device(gpu_index);
	if (res != 0)
	{
		return res;
	}
#endif
	pSRParam->lThresh = Thresh;
	//pSRParam->net = COM_parse_network_cfg_adapt(cfgfile,w,h);
	pSRParam->net = COM_parse_network_cfg_GPU(cfgfile);
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
	return 0;
	
}


int HYOLR_OilRecog_GPU(void *hMRHandle,OLR_IMAGES *pImg,HYOLR_RESULT_LIST *pResultList)
{
	int res =0;
	int i=0,j=0;
	int total=10;
    float thresh;
	image out;
	BboxOut bbox_out={0};
	
	//sortable_DigitForm *s;
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
		if(bbox_out.Bbox[i] ==NULL)
		{
			res=-1;
			goto EXT;
		}
	}
	//yolo������
	thresh = pSRParam->lThresh;	

	if(!pImg)
	{
		res = -1;
		goto EXT;
	}
    out =   HYOIL_ipl_to_image_GPU(pImg);
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

	if(bbox_out.num == 0)
	{
		printf("δ�ҵ�Ŀ��\n");
		res = -1;
		goto EXT;
	}
	
	for(i = 0; i < bbox_out.num; i++)
	{
		//printf("%f\n",bbox_out.Bbox[i][4]);
		//pResultList->pResult[j].flag=1;
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

	for(i = 0; i < total; ++i)
	{
		if(bbox_out.Bbox[i])
		{
			free(bbox_out.Bbox[i]);
			bbox_out.Bbox[i]=NULL;
		}
	}
	if(bbox_out.Bbox)
	{
		free(bbox_out.Bbox);
		bbox_out.Bbox=NULL;
	}
	

	return res;
}