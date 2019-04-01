#ifndef DETECOTRGPU_H
#define DETECOTRGPU_H
#include "detector_COM.h"
//#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
//extern "C"
//{
//#endif

	 int COM_YoloDetector_GPU(const network net, image pImg, float thresh, int *numDST, float **boxout);
	network COM_parse_network_cfg_GPU(char *filename);
	 network COM_parse_network_cfg_adapt_GPU(char *filename, int w, int h);
	 void COM_load_weights_GPU(network *net, char *filename);
	 void COM_set_batch_network_GPU(network *net, int b);
	image COM_make_image_GPU(int w, int h, int c);
	 void COM_free_image_GPU(image m);
	void COM_free_network_GPU(network net);
//#ifdef __cplusplus
//}
//#endif

#endif
