#include "stdio.h"
#include "stdlib.h"
//#include "detector_GPU.h"
#include "yolodetectGPU.h"
typedef struct
{
	network net;
	float lThresh;
}MRParam;
int init(void **handle, char *cfgfile, char *weightfile, float Thresh, int gpu_index)
{
	int res = 0;
	MRParam *tmpHandle = NULL;
	network net = { 0 };
	tmpHandle = (MRParam*)calloc(1, sizeof(MRParam));
	if (tmpHandle == NULL)
	{
		res = -1;
		goto EXT;
	}
	tmpHandle->lThresh = Thresh;
	tmpHandle->net = net;
#ifdef GPU
	res = HY_cuda_set_device(gpu_index);

	if (res != 0)
	{
		return res;
	}
#endif
	tmpHandle->net = COM_parse_network_cfg_GPU(cfgfile);
	if (tmpHandle->net.errorcode != 0)
	{
		res = tmpHandle->net.errorcode;
		goto EXT;
	}
	if (weightfile) {
		COM_load_weights_GPU(&(tmpHandle->net), weightfile);
		if (tmpHandle->net.errorcode != 0)
		{
			res = tmpHandle->net.errorcode;
			goto EXT;
		}
	}
	COM_set_batch_network_GPU(&(tmpHandle->net), 1);
EXT:
	*handle = (void*)tmpHandle;
	return res;
}
int uninit(void *handle)
{
	MRParam *netParam = (MRParam*)handle;
	if (netParam)
	{
		COM_free_network_GPU(netParam->net);
		free(netParam);
		netParam = NULL;
	}
	return 0;
}
int detec(void *handle, unsigned char* imagdata, int height, int width, int channels, int widstep, int *numDST, float **boxout)
{
	int res = 0;
	MRParam *pSRParam = (MRParam*)handle;
	image out = COM_make_image_GPU(width, height, channels);
	int i, j, k, count = 0;
	if (out.data == NULL)
	{
		res = -1;
		goto EXT;
	}
	for (k = 0; k < channels; ++k) {
		for (i = 0; i <height; ++i) {
			for (j = 0; j < width; ++j) {
				out.data[count++] = imagdata[i*widstep + j*channels + k] / 255.;
			}
		}
	}
	if (0 != COM_YoloDetector_GPU(pSRParam->net, out, pSRParam->lThresh, numDST, boxout))
	{
		res = -1;
		goto EXT;

	}
EXT:
	COM_free_image_GPU(out);
	return res;
}

