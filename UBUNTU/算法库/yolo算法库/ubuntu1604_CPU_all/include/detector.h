#ifndef DETECOTR_H
#define DETECOTR_H
#include "detector_COM.h"
//#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
//extern "C"
//{
//#endif

	int COM_YoloDetector(const network net, image pImg, float thresh, int *numDST, float **boxout);
	network COM_parse_network_cfg(char *filename);
	network COM_parse_network_cfg_adapt(char *filename, int w, int h);
	void COM_load_weights(network *net, char *filename);
	void COM_set_batch_network(network *net, int b);
	image COM_make_image(int w, int h, int c);
	void COM_free_image(image m);
	void COM_free_network(network net);
//#ifdef __cplusplus
//}
//#endif

#endif
