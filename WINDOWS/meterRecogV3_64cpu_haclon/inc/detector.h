#ifndef DETECOTR_H
#define DETECOTR_H
#include "detector_COM.h"
//#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
//extern "C"
//{
//#endif

	__declspec(dllexport) int COM_YoloDetector(const network net, image pImg, float thresh, int *numDST, float **boxout);
	__declspec(dllexport) network COM_parse_network_cfg(char *filename);
	__declspec(dllexport) network COM_parse_network_cfg_adapt(char *filename, int w, int h);
	__declspec(dllexport) void COM_load_weights(network *net, char *filename);
	__declspec(dllexport) void COM_set_batch_network(network *net, int b);
	__declspec(dllexport) image COM_make_image(int w, int h, int c);
	__declspec(dllexport) void COM_free_image(image m);
	__declspec(dllexport) void COM_free_network(network net);
//#ifdef __cplusplus
//}
//#endif

#endif