#include <iostream>
#ifdef __cplusplus
extern "C" {
#endif
#include "yolo_v3_dll.h"
#ifdef __cplusplus
}
#endif
#include "yolo_dll.h"


//#ifndef WIN32
//#pragma comment(lib,"../darknet/x64/darknet.lib")
//#else
//#pragma comment(lib,"../darknet/Win32/darknet_no_gpu.lib")
//#endif



typedef struct {
	network net;
	float thresh;
}Param;
typedef struct {
	int num;
	float **Bbox;
}BboxOut;

int init(void **handle,char *cfgfile,char *weightfile,float thresh,int gpu_index)
{
	int res = 0;
	Param *tmpHandle = NULL;
	tmpHandle = (Param*)calloc(1,sizeof(Param));
	if (tmpHandle == NULL)
	{
		res = -1;
		goto EXT;
	}
#ifdef GPU
	res = HY_cuda_set_device(gpu_index);
	if (res != 0)
	{
		return res;
	}
#endif
	tmpHandle->thresh = thresh;
#ifdef GPU
	tmpHandle->net = parse_network_cfg_custom(cfgfile, 1); // set batch=1
#else
	tmpHandle->net = parse_network_cfg_custom_no_gpu(cfgfile, 1); // set batch=1
#endif
	if (tmpHandle->net.errorcode != 0)
	{
		res = tmpHandle->net.errorcode;
		goto EXT;
	}
	if (weightfile) {
#ifdef GPU
		load_weights(&tmpHandle->net, weightfile);
#else
		load_weights_no_gpu(&tmpHandle->net, weightfile);
#endif
		if (tmpHandle->net.errorcode != 0)
		{
			res = tmpHandle->net.errorcode;
			goto EXT;
		}
	}
#ifdef GPU
	fuse_conv_batchnorm(tmpHandle->net);
	calculate_binary_weights(tmpHandle->net);  //xnorģ��
#else
	fuse_conv_batchnorm_no_gpu(tmpHandle->net);
	calculate_binary_weights_no_gpu(tmpHandle->net);  //xnorģ��
#endif
EXT:
	*handle = (void*)tmpHandle;
	return res;
}
int uninit(void *handle)
{
	Param *netParam = (Param*)handle;
	if (netParam)
	{
#ifdef GPU
		free_network(netParam->net);
#else
		free_network_no_gpu(netParam->net);
#endif
		free(netParam);
		netParam = NULL;
	}		
	return 0;
}

int detec(void *handle, YOLOV3_PIMAGES img, HYYOLOV3RESULT_PLIST pResultList)
{
	double time;
	int res = 0;
	Param *netParam = (Param*)handle;
	BboxOut bbox_out = { 0 };
#ifdef GPU
	image out = _ipl_to_image((unsigned char *)img->pixelArray.chunky.pPixel, img->lHeight, img->lWidth, img->lPixelArrayFormat, img->pixelArray.chunky.lLineBytes);  //cost time
#else
	image out = _ipl_to_image_no_gpu((unsigned char *)img->pixelArray.chunky.pPixel, img->lHeight, img->lWidth, img->lPixelArrayFormat, img->pixelArray.chunky.lLineBytes);
#endif

	
	if (out.data == NULL)
	{
		res = -1;
		return res;
	}
	if (out.c > 1)
#ifdef GPU
		_rgbgr_image(out);  //cost time
#else
		_rgbgr_image_no_gpu(out);
#endif
	int total = 100;
	bbox_out.Bbox = (float**)calloc(total, sizeof(float*));
	for (int i = 0; i < 100; i++)bbox_out.Bbox[i] = (float*)calloc(6, sizeof(float));

#ifdef GPU
	yolo_detector_v3(netParam->net, out, netParam->thresh, &bbox_out.num, bbox_out.Bbox);
	_free_image(out);
#else
	yolo_detector_v3_no_gpu(netParam->net, out, netParam->thresh, &bbox_out.num, bbox_out.Bbox);
	_free_image_no_gpu(out);
#endif
	int j = 0;
	for (int i = 0; i < bbox_out.num; i++)
	{
		pResultList->pResult[j].Target.left = bbox_out.Bbox[i][0];
		pResultList->pResult[j].Target.right = bbox_out.Bbox[i][1];
		pResultList->pResult[j].Target.top = bbox_out.Bbox[i][2];
		pResultList->pResult[j].Target.bottom = bbox_out.Bbox[i][3];
		pResultList->pResult[j].dConfidence = bbox_out.Bbox[i][4];
		pResultList->pResult[j].dVal = bbox_out.Bbox[i][5];
		j = j + 1;
	}
	pResultList->lResultNum = j;
EXT:
	
	if (bbox_out.Bbox)
	{
		for (int i = 0; i < total; ++i)
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

int init_no_gpu(void **handle, char *cfgfile, char *weightfile, float thresh)
{
	return init(handle, cfgfile, weightfile, thresh,0);
}
int uninit_no_gpu(void *handle)
{
	return uninit(handle);
}
int detec_no_gpu(void *handle, YOLOV3_PIMAGES img, HYYOLOV3RESULT_PLIST pResultList)
{
	return detec(handle,img, pResultList);
}