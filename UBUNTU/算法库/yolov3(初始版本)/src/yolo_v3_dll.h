#ifndef YOLO_V3_DLL_H
#define YOLO_V3_DLL_H
#include "stddef.h"
//#ifdef GPU
//#ifdef CUDNN
	//#include "cuda.h"
//#endif
//#endif

typedef struct{
	int w;
	int h;
	int c;
	float *data;
} image;
typedef struct layer layer;
typedef struct _tree tree;

typedef enum {
	CONSTANT, STEP, EXP, POLY, STEPS, SIG, RANDOM
} learning_rate_policy;

typedef struct network {
	int errorcode;
	float *workspace;
	int n;
	int batch;
	int *seen;
	float epoch;
	int subdivisions;
	float momentum;
	float decay;
	layer *layers;
	int outputs;
	float *output;
	learning_rate_policy policy;

	float learning_rate;
	float gamma;
	float scale;
	float power;
	int time_steps;
	int step;
	int max_batches;
	float *scales;
	int   *steps;
	int num_steps;
	int burn_in;

	int adam;
	float B1;
	float B2;
	float eps;

	int inputs;
	int h, w, c;
	int max_crop;
	int min_crop;
	int flip; // horizontal flip 50% probability augmentaiont for classifier training (default = 1)
	float angle;
	float aspect;
	float exposure;
	float saturation;
	float hue;
	int small_object;

	int gpu_index;
	tree *hierarchy;

#ifdef GPU
	float *input_state_gpu;

	float **input_gpu;
	float **truth_gpu;
	float **input16_gpu;
	float **output16_gpu;
	size_t *max_input16_size;
	size_t *max_output16_size;
	int wait_stream;
#endif
} network;

#define USE_DEBUG
#ifdef USE_DEBUG
#define DEBUG_PRINT() printf("file: \"%s\" line %d\nfunc: %s\n", __FILE__, __LINE__, __FUNCTION__);
#else
#define DEBUG_PRINT()
#endif

//#define YOLOV3DLL_API __declspec(dllexport) 
#if defined(_MSC_VER)
#define YOLOV3DLL_API __declspec(dllexport)
#else
#define YOLOV3DLL_API 
#endif

#ifdef GPU
YOLOV3DLL_API network parse_network_cfg_custom(char *filename, int batch);
YOLOV3DLL_API void load_weights(network *net, char *filename);
YOLOV3DLL_API void fuse_conv_batchnorm(network net);
YOLOV3DLL_API int yolo_detector_v3(const network net, const image pImg, float thresh, int *numDST, float **boxout);
YOLOV3DLL_API void free_network(network net);
YOLOV3DLL_API void calculate_binary_weights(network net);
YOLOV3DLL_API int HY_cuda_set_device(int n);
YOLOV3DLL_API image _ipl_to_image(unsigned char *data, int h, int w, int c, int step);
YOLOV3DLL_API void _rgbgr_image(image im);
YOLOV3DLL_API void _free_image(image m);
YOLOV3DLL_API image _make_image(int w, int h, int c);
#else
YOLOV3DLL_API network parse_network_cfg_custom_no_gpu(char *filename, int batch);
YOLOV3DLL_API void load_weights_no_gpu(network *net, char *filename);
YOLOV3DLL_API void fuse_conv_batchnorm_no_gpu(network net);
YOLOV3DLL_API int yolo_detector_v3_no_gpu(const network net, const image pImg, float thresh, int *numDST, float **boxout);
YOLOV3DLL_API void calculate_binary_weights_no_gpu(network net);
YOLOV3DLL_API void free_network_no_gpu(network net);
YOLOV3DLL_API image _ipl_to_image_no_gpu(unsigned char *data, int h, int w, int c, int step);
YOLOV3DLL_API void _rgbgr_image_no_gpu(image im);
YOLOV3DLL_API void _free_image_no_gpu(image m);
YOLOV3DLL_API image _make_image_no_gpu(int w, int h, int c);
#endif


#endif
