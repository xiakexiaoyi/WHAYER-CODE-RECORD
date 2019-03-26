#include <stdio.h>
#include <stdlib.h>
#include "detector.h"
#include "meterDec.h"

typedef struct{
    int num;
	float **Bbox;
} BboxOut;

typedef struct 
{
	network net;
	float lThresh;
}MParam;

image HYMD_ipl_to_image(MR_IMAGES *pImg)
{
	unsigned char *data = (unsigned char *)pImg->pixelArray.chunky.pPixel;
	int h = pImg->lHeight;
	int w = pImg->lWidth;
	int c = 3;
	int step = pImg->pixelArray.chunky.lLineBytes;
    image out = COM_make_image(w, h, c);
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
int HYMR_Init(void *hMemMgr, void **pMRHandle)
{
	int res = 0;
	MParam *pSRHandle=NULL;
	
	//if (!pMRHandle) return -1;
	pSRHandle=(MParam *)malloc(1*sizeof(MParam));
	if (!pSRHandle)
	{
		res= -1;
		goto EXT;
	}
	
EXT:
	*pMRHandle = pSRHandle;
	return res;
}
int HYMR_Uninit(void *hMRHandle)
{
	MParam *pSRParam = (MParam*)hMRHandle;
	COM_free_network(pSRParam->net);


	if (pSRParam)
		free (pSRParam);
	pSRParam = NULL;
	return 0;
}
int HYMR_SetParam(void *hMRHandle,char *cfgfile, char *weightfile,float Thresh,int w,int h)
{
	int res=0;
	MParam *pSRParam = (MParam*)hMRHandle;

	pSRParam->lThresh = Thresh;
	//pSRParam->net = COM_parse_network_cfg_adapt(cfgfile,w,h);
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
	return 0;
	
}
int HYMR_meterRecog(void *hMRHandle,MR_IMAGES *pImg,HYMR_RESULT_LIST *pResultList)
{
	int res =0;
	int i=0,j=0;
	int total=10;
    float thresh;
	image out;
	BboxOut bbox_out={0};
	
	//sortable_DigitForm *s;
	MParam *pSRParam = (MParam*)hMRHandle;
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
	//yolo检测输出
	thresh = pSRParam->lThresh;	

	if(!pImg)
	{
		res = -1;
		goto EXT;
	}
    out =   HYMD_ipl_to_image(pImg);
	if(out.data==NULL)
	{
		res=-1;
		goto EXT;
	}

	//COM_YoloDetector(pSRParam->net,out,thresh,&bbox_out.num,bbox_out.Bbox);
	if(0!=COM_YoloDetector(pSRParam->net,out,thresh,&bbox_out.num,bbox_out.Bbox))
	{
		res=-1;
		COM_free_image(out);
		goto EXT;
	}

	COM_free_image(out);
	printf("bbox_out.num=%d\n",bbox_out.num);
	if(bbox_out.num == 0)
	{
		printf("未找到目标\n");
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
