#ifndef DETECOTR_H
#define DETECOTR_H
#ifdef GPU
#include "cuda.h"
#endif
#include "stddef.h"

//#ifdef __cplusplus //指明函数的编译方式，以得到没有任何修饰的函数名
//extern "C"
//{
//#endif
	typedef struct {
		int h;
		int w;
		int c;
		float *data;
	} image;

	typedef struct {
		int *leaf;
		int n;
		int *parent;
		int *group;
		char **name;

		int groups;
		int *group_size;
		int *group_offset;
	} tree;

	struct network_state;

	struct layer;
	typedef struct layer layer;

	typedef enum {
		CONVOLUTIONAL,
		DECONVOLUTIONAL,
		CONNECTED,
		MAXPOOL,
		SOFTMAX,
		DETECTION,
		DROPOUT,
		CROP,
		ROUTE,
		COST,
		NORMALIZATION,
		AVGPOOL,
		LOCAL,
		SHORTCUT,
		ACTIVE,
		RNN,
		GRU,
		CRNN,
		BATCHNORM,
		NETWORK,
		XNOR,
		_REGION,
		REORG,
		BLANK
	} LAYER_TYPE;

	typedef enum {
		LOGISTIC, RELU, RELIE, LINEAR, RAMP, TANH, PLSE, LEAKY, ELU, LOGGY, STAIR, HARDTAN, LHTAN
	}ACTIVATION;

	typedef enum {
		SSE, MASKED, SMOOTH
	} COST_TYPE;

	struct layer {
		LAYER_TYPE type;
		ACTIVATION activation;
		COST_TYPE cost_type;
		void(*forward)   (struct layer, struct network_state);
		void(*backward)  (struct layer, struct network_state);
		void(*update)    (struct layer, int, float, float, float);
		void(*forward_gpu)   (struct layer, struct network_state);
		void(*backward_gpu)  (struct layer, struct network_state);
		void(*update_gpu)    (struct layer, int, float, float, float);
		int calloc_or_fail;
		int batch_normalize;
		int shortcut;
		int batch;
		int forced;
		int flipped;
		int inputs;
		int outputs;
		int extra;
		int truths;
		int h, w, c;
		int out_h, out_w, out_c;
		int n;
		int max_boxes;
		int groups;
		int size;
		int side;
		int stride;
		int reverse;
		int flatten;
		int pad;
		int sqrt;
		int flip;
		int index;
		int binary;
		int xnor;
		int steps;
		int hidden;
		float dot;
		float angle;
		float jitter;
		float saturation;
		float exposure;
		float shift;
		float ratio;
		int softmax;
		int classes;
		int coords;
		int background;
		int rescore;
		int objectness;
		int does_cost;
		int joint;
		int noadjust;
		int reorg;
		int log;

		int adam;
		float B1;
		float B2;
		float eps;
		float *m_gpu;
		float *v_gpu;
		int t;
		float *m;
		float *v;

		tree *softmax_tree;
		int  *map;

		float alpha;
		float beta;
		float kappa;

		float coord_scale;
		float object_scale;
		float noobject_scale;
		float class_scale;
		int bias_match;
		int random;
		float thresh;
		int classfix;
		int absolute;

		int dontload;
		int dontloadscales;

		float temperature;
		float probability;
		float scale;

		int *indexes;
		float *rand;
		float *cost;
		char  *cweights;
		float *state;
		float *prev_state;
		float *forgot_state;
		float *forgot_delta;
		float *state_delta;

		float *concat;
		float *concat_delta;

		float *binary_weights;

		float *biases;
		float *bias_updates;

		float *scales;
		float *scale_updates;

		float *weights;
		float *weight_updates;

		float *col_image;
		int   * input_layers;
		int   * input_sizes;
		float * delta;
		float * output;
		float * squared;
		float * norms;

		float * spatial_mean;
		float * mean;
		float * variance;

		float * mean_delta;
		float * variance_delta;

		float * rolling_mean;
		float * rolling_variance;

		float * x;
		float * x_norm;

		struct layer *input_layer;
		struct layer *self_layer;
		struct layer *output_layer;

		struct layer *input_gate_layer;
		struct layer *state_gate_layer;
		struct layer *input_save_layer;
		struct layer *state_save_layer;
		struct layer *input_state_layer;
		struct layer *state_state_layer;

		struct layer *input_z_layer;
		struct layer *state_z_layer;

		struct layer *input_r_layer;
		struct layer *state_r_layer;

		struct layer *input_h_layer;
		struct layer *state_h_layer;

		float *z_cpu;
		float *r_cpu;
		float *h_cpu;

		float *binary_input;

		size_t workspace_size;

//GPU变量
#ifdef GPU

		float *z_gpu;
		float *r_gpu;
		float *h_gpu;

		int *indexes_gpu;
		float * prev_state_gpu;
		float * forgot_state_gpu;
		float * forgot_delta_gpu;
		float * state_gpu;
		float * state_delta_gpu;
		float * gate_gpu;
		float * gate_delta_gpu;
		float * save_gpu;
		float * save_delta_gpu;
		float * concat_gpu;
		float * concat_delta_gpu;

		float *binary_input_gpu;
		float *binary_weights_gpu;

		float * mean_gpu;
		float * variance_gpu;

		float * rolling_mean_gpu;
		float * rolling_variance_gpu;

		float * variance_delta_gpu;
		float * mean_delta_gpu;

		float * col_image_gpu;

		float * x_gpu;
		float * x_norm_gpu;
		float * weights_gpu;
		float * weight_updates_gpu;

		float * biases_gpu;
		float * bias_updates_gpu;

		float * scales_gpu;
		float * scale_updates_gpu;

		float * output_gpu;
		float * delta_gpu;
		float * rand_gpu;
		float * squared_gpu;
		float * norms_gpu;
#ifdef CUDNN
		cudnnTensorDescriptor_t srcTensorDesc, dstTensorDesc;
		cudnnTensorDescriptor_t dsrcTensorDesc, ddstTensorDesc;
		cudnnFilterDescriptor_t weightDesc;
		cudnnFilterDescriptor_t dweightDesc;
		cudnnConvolutionDescriptor_t convDesc;
		cudnnConvolutionFwdAlgo_t fw_algo;
		cudnnConvolutionBwdDataAlgo_t bd_algo;
		cudnnConvolutionBwdFilterAlgo_t bf_algo;
#endif
#endif 
	};

	typedef enum {
		CONSTANT, STEP, EXP, POLY, STEPS, SIG, RANDOM
	} learning_rate_policy;

	typedef struct network {
		int errorcode;  //-1 calloc;-2 fopen
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
		float angle;
		float aspect;
		float exposure;
		float saturation;
		float hue;

		int gpu_index;
		tree *hierarchy;

#ifdef GPU
		float **input_gpu;
		float **truth_gpu;
#endif
	} network;

	typedef struct network_state {
		float *truth;
		float *input;
		float *delta;
		float *workspace;
		int train;
		int index;
		network net;
	} network_state;

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