#ifndef DETECOTRGPU_H
#define DETECOTRGPU_H
#include "detector_COM.h"

	__declspec(dllexport) int COM_YoloDetector_GPU(const network net, image pImg, float thresh, int *numDST, float **boxout);
	__declspec(dllexport) network COM_parse_network_cfg_GPU(char *filename);
	__declspec(dllexport) network COM_parse_network_cfg_adapt_GPU(char *filename, int w, int h);
	__declspec(dllexport) void COM_load_weights_GPU(network *net, char *filename);
	__declspec(dllexport) void COM_set_batch_network_GPU(network *net, int b);
	__declspec(dllexport) image COM_make_image_GPU(int w, int h, int c);
	__declspec(dllexport) void COM_free_image_GPU(image m);
	__declspec(dllexport) void COM_free_network_GPU(network net);
//#ifdef __cplusplus
//}
//#endif

#endif