#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <Windows.h>

#include "activation_layer.h"
#include "activations.h"
#include "assert.h"
#include "avgpool_layer.h"
#include "batchnorm_layer.h"
#include "blas.h"
#include "connected_layer.h"
#include "convolutional_layer.h"
#include "cost_layer.h"
#include "crnn_layer.h"
#include "crop_layer.h"
#include "detection_layer.h"
#include "dropout_layer.h"
#include "gru_layer.h"
#include "list.h"
#include "local_layer.h"
#include "maxpool_layer.h"
#include "normalization_layer.h"
#include "option_list.h"
#include "parser.h"
#include "region_layer.h"
#include "reorg_layer.h"
#include "rnn_layer.h"
#include "route_layer.h"
#include "shortcut_layer.h"
#include "softmax_layer.h"
#include "utils.h"

//#include <stdint.h>
typedef unsigned long long uint64_t;
#ifndef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

typedef struct{
    char *type;
    list *options;
}section;

list *read_cfg(char *filename);

LAYER_TYPE string_to_layer_type(char * type)
{

    if (strcmp(type, "[shortcut]")==0) return SHORTCUT;
    if (strcmp(type, "[crop]")==0) return CROP;
    if (strcmp(type, "[cost]")==0) return COST;
    if (strcmp(type, "[detection]")==0) return DETECTION;
    if (strcmp(type, "[region]")==0) return REGION;
    if (strcmp(type, "[local]")==0) return LOCAL;
    if (strcmp(type, "[conv]")==0
            || strcmp(type, "[convolutional]")==0) return CONVOLUTIONAL;
    if (strcmp(type, "[activation]")==0) return ACTIVE;
    if (strcmp(type, "[net]")==0
            || strcmp(type, "[network]")==0) return NETWORK;
    if (strcmp(type, "[crnn]")==0) return CRNN;
    if (strcmp(type, "[gru]")==0) return GRU;
    if (strcmp(type, "[rnn]")==0) return RNN;
    if (strcmp(type, "[conn]")==0
            || strcmp(type, "[connected]")==0) return CONNECTED;
    if (strcmp(type, "[max]")==0
            || strcmp(type, "[maxpool]")==0) return MAXPOOL;
    if (strcmp(type, "[reorg]")==0) return REORG;
    if (strcmp(type, "[avg]")==0
            || strcmp(type, "[avgpool]")==0) return AVGPOOL;
    if (strcmp(type, "[dropout]")==0) return DROPOUT;
    if (strcmp(type, "[lrn]")==0
            || strcmp(type, "[normalization]")==0) return NORMALIZATION;
    if (strcmp(type, "[batchnorm]")==0) return BATCHNORM;
    if (strcmp(type, "[soft]")==0
            || strcmp(type, "[softmax]")==0) return SOFTMAX;
    if (strcmp(type, "[route]")==0) return ROUTE;
    return BLANK;
}

void free_section(section *s)
{
	node *n;
    free(s->type);
    n = s->options->front;
    while(n){
		node *next;
        kvp *pair = (kvp *)n->val;
        free(pair->key);
        free(pair);
        next = n->next;
        free(n);
        n = next;
    }
    free(s->options);
    free(s);
}

void parse_data(char *data, float *a, int n)
{
    int i;
	char *curr,*next;
    int done = 0;
    if(!data) return;
    curr = data;
    next = data;
    for(i = 0; i < n && !done; ++i){
        while(*++next !='\0' && *next != ',');
        if(*next == '\0') done = 1;
        *next = '\0';
        sscanf(curr, "%g", &a[i]);
        curr = next+1;
    }
}

typedef struct size_params{
    int batch;
    int inputs;
    int h;
    int w;
    int c;
    int index;
    int time_steps;
    network net;
} size_params;

local_layer parse_local(list *options, size_params params)
{
	local_layer layer;
    int n = option_find_int(options, "filters",1);
    int size = option_find_int(options, "size",1);
    int stride = option_find_int(options, "stride",1);
    int pad = option_find_int(options, "pad",0);
    char *activation_s = option_find_str(options, "activation", "logistic");
    ACTIVATION activation = get_activation(activation_s);

    int batch,h,w,c;
    h = params.h;
    w = params.w;
    c = params.c;
    batch=params.batch;
    if(!(h && w && c)) error("Layer before local layer must output image.");

    layer = make_local_layer(batch,h,w,c,n,size,stride,pad,activation);

    return layer;
}

convolutional_layer parse_convolutional(list *options, size_params params)
{
	char *activation_s;
	ACTIVATION activation;
	int batch,h,w,c;
	int batch_normalize,binary,xnor;
	convolutional_layer layer;
    int n = option_find_int(options, "filters",1);
    int size = option_find_int(options, "size",1);
    int stride = option_find_int(options, "stride",1);
    int pad = option_find_int_quiet(options, "pad",0);
    int padding = option_find_int_quiet(options, "padding",0);
    if(pad) padding = size/2;

    activation_s = option_find_str(options, "activation", "logistic");
    activation = get_activation(activation_s);

    layer.calloc_or_fail=0;//³õÊ¼»¯
    h = params.h;
    w = params.w;
    c = params.c;
    batch=params.batch;
    if(!(h && w && c)) 
	{
		printf("Layer before convolutional layer must output image.\n");
		layer.calloc_or_fail=-3;
		return layer;
		//error("Layer before convolutional layer must output image.");
	}
    batch_normalize = option_find_int_quiet(options, "batch_normalize", 0);
    binary = option_find_int_quiet(options, "binary", 0);
    xnor = option_find_int_quiet(options, "xnor", 0);

    layer = make_convolutional_layer(batch,h,w,c,n,size,stride,padding,activation, batch_normalize, binary, xnor, params.net.adam);
	if(layer.calloc_or_fail < 0)
	{
		printf("Error in make_convolutional_layer.\n");
		return layer;
	}
    layer.flipped = option_find_int_quiet(options, "flipped", 0);
    layer.dot = option_find_float_quiet(options, "dot", 0);
    if(params.net.adam){
        layer.B1 = params.net.B1;
        layer.B2 = params.net.B2;
        layer.eps = params.net.eps;
    }

    return layer;
}

layer parse_crnn(list *options, size_params params)
{
    int output_filters = option_find_int(options, "output_filters",1);
    int hidden_filters = option_find_int(options, "hidden_filters",1);
    char *activation_s = option_find_str(options, "activation", "logistic");
    ACTIVATION activation = get_activation(activation_s);
    int batch_normalize = option_find_int_quiet(options, "batch_normalize", 0);

    layer l = make_crnn_layer(params.batch, params.w, params.h, params.c, hidden_filters, output_filters, params.time_steps, activation, batch_normalize);

    l.shortcut = option_find_int_quiet(options, "shortcut", 0);

    return l;
}

layer parse_rnn(list *options, size_params params)
{
    int output = option_find_int(options, "output",1);
    int hidden = option_find_int(options, "hidden",1);
    char *activation_s = option_find_str(options, "activation", "logistic");
    ACTIVATION activation = get_activation(activation_s);
    int batch_normalize = option_find_int_quiet(options, "batch_normalize", 0);
    int logistic = option_find_int_quiet(options, "logistic", 0);

    layer l = make_rnn_layer(params.batch, params.inputs, hidden, output, params.time_steps, activation, batch_normalize, logistic);

    l.shortcut = option_find_int_quiet(options, "shortcut", 0);

    return l;
}

layer parse_gru(list *options, size_params params)
{
    int output = option_find_int(options, "output",1);
    int batch_normalize = option_find_int_quiet(options, "batch_normalize", 0);

    layer l = make_gru_layer(params.batch, params.inputs, output, params.time_steps, batch_normalize);

    return l;
}

connected_layer parse_connected(list *options, size_params params)
{
    int output = option_find_int(options, "output",1);
    char *activation_s = option_find_str(options, "activation", "logistic");
    ACTIVATION activation = get_activation(activation_s);
    int batch_normalize = option_find_int_quiet(options, "batch_normalize", 0);

    connected_layer layer = make_connected_layer(params.batch, params.inputs, output, activation, batch_normalize);

    return layer;
}

softmax_layer parse_softmax(list *options, size_params params)
{
	char *tree_file;
    int groups = option_find_int_quiet(options, "groups",1);
    softmax_layer layer = make_softmax_layer(params.batch, params.inputs, groups);
    layer.temperature = option_find_float_quiet(options, "temperature", 1);
    tree_file = option_find_str(options, "tree", 0);
    if (tree_file) layer.softmax_tree = read_tree(tree_file);
    return layer;
}

layer parse_region(list *options, size_params params)
{
	char *tree_file;
	char *map_file;
	char *a;
    int coords = option_find_int(options, "coords", 4);
    int classes = option_find_int(options, "classes", 20);
    int num = option_find_int(options, "num", 1);

    layer l = make_region_layer(params.batch, params.w, params.h, num, classes, coords);
    //assert(l.outputs == params.inputs);
    if(l.outputs != params.inputs)
	{
		printf("l.outputs != params.inputs.\n");
		l.calloc_or_fail=-3;
		return l;
	}
	if(l.calloc_or_fail < 0)
	{
		printf("Error in make_region_layer.\n");
		return l;
	}
    l.log = option_find_int_quiet(options, "log", 0);
    l.sqrt = option_find_int_quiet(options, "sqrt", 0);

    l.softmax = option_find_int(options, "softmax", 0);
    l.max_boxes = option_find_int_quiet(options, "max",30);
    l.jitter = option_find_float(options, "jitter", .2);
    l.rescore = option_find_int_quiet(options, "rescore",0);

    l.thresh = option_find_float(options, "thresh", .5);
    l.classfix = option_find_int_quiet(options, "classfix", 0);
    l.absolute = option_find_int_quiet(options, "absolute", 0);
    l.random = option_find_int_quiet(options, "random", 0);

    l.coord_scale = option_find_float(options, "coord_scale", 1);
    l.object_scale = option_find_float(options, "object_scale", 1);
    l.noobject_scale = option_find_float(options, "noobject_scale", 1);
    l.class_scale = option_find_float(options, "class_scale", 1);
    l.bias_match = option_find_int_quiet(options, "bias_match",0);

    tree_file = option_find_str(options, "tree", 0);
    if (tree_file) l.softmax_tree = read_tree(tree_file);
    map_file = option_find_str(options, "map", 0);
    if (map_file) l.map = read_map(map_file);

    a = option_find_str(options, "anchors", 0);
    if(a){
        int len = strlen(a);
        int n = 1;
        int i;
        for(i = 0; i < len; ++i){
            if (a[i] == ',') ++n;
        }
        for(i = 0; i < n; ++i){
            float bias = atof(a);
            l.biases[i] = bias;
            a = strchr(a, ',')+1;
        }
    }
    return l;
}
detection_layer parse_detection(list *options, size_params params)
{
    int coords = option_find_int(options, "coords", 1);
    int classes = option_find_int(options, "classes", 1);
    int rescore = option_find_int(options, "rescore", 0);
    int num = option_find_int(options, "num", 1);
    int side = option_find_int(options, "side", 7);
    detection_layer layer = make_detection_layer(params.batch, params.inputs, num, side, classes, coords, rescore);

    layer.softmax = option_find_int(options, "softmax", 0);
    layer.sqrt = option_find_int(options, "sqrt", 0);

    layer.max_boxes = option_find_int_quiet(options, "max",30);
    layer.coord_scale = option_find_float(options, "coord_scale", 1);
    layer.forced = option_find_int(options, "forced", 0);
    layer.object_scale = option_find_float(options, "object_scale", 1);
    layer.noobject_scale = option_find_float(options, "noobject_scale", 1);
    layer.class_scale = option_find_float(options, "class_scale", 1);
    layer.jitter = option_find_float(options, "jitter", .2);
    layer.random = option_find_int_quiet(options, "random", 0);
    layer.reorg = option_find_int_quiet(options, "reorg", 0);
    return layer;
}

cost_layer parse_cost(list *options, size_params params)
{
    char *type_s = option_find_str(options, "type", "sse");
    COST_TYPE type = get_cost_type(type_s);
    float scale = option_find_float_quiet(options, "scale",1);
    cost_layer layer = make_cost_layer(params.batch, params.inputs, type, scale);
    layer.ratio =  option_find_float_quiet(options, "ratio",0);
    return layer;
}

crop_layer parse_crop(list *options, size_params params)
{
	int noadjust;
	crop_layer l;
    int crop_height = option_find_int(options, "crop_height",1);
    int crop_width = option_find_int(options, "crop_width",1);
    int flip = option_find_int(options, "flip",0);
    float angle = option_find_float(options, "angle",0);
    float saturation = option_find_float(options, "saturation",1);
    float exposure = option_find_float(options, "exposure",1);

    int batch,h,w,c;
    h = params.h;
    w = params.w;
    c = params.c;
    batch=params.batch;
    if(!(h && w && c)) error("Layer before crop layer must output image.");

    noadjust = option_find_int_quiet(options, "noadjust",0);

    l = make_crop_layer(batch,h,w,c,crop_height,crop_width,flip, angle, saturation, exposure);
    l.shift = option_find_float(options, "shift", 0);
    l.noadjust = noadjust;
    return l;
}

layer parse_reorg(list *options, size_params params)
{
	layer layer;
    int stride = option_find_int(options, "stride",1);
    int reverse = option_find_int_quiet(options, "reverse",0);
	int flatten = option_find_int_quiet(options, "flatten", 0);
	int extra = option_find_int_quiet(options, "extra", 0);

    int batch,h,w,c;
    h = params.h;
    w = params.w;
    c = params.c;
    batch=params.batch;
    if(!(h && w && c)) 
	{
		printf("Layer before reorg layer must output image.\n");
		layer.calloc_or_fail=-1;
		return layer;
		//error("Layer before reorg layer must output image.");
	}

    layer = make_reorg_layer(batch, w, h, c, stride, reverse, flatten, extra);
	if(layer.calloc_or_fail < 0)
	{
		printf("Error in make_reorg_layer.\n");
		return layer;
	}
    return layer;
}

maxpool_layer parse_maxpool(list *options, size_params params)
{
	maxpool_layer layer;
    int stride = option_find_int(options, "stride",1);
    int size = option_find_int(options, "size",stride);
    int padding = option_find_int_quiet(options, "padding", (size-1)/2);

    int batch,h,w,c;
    h = params.h;
    w = params.w;
    c = params.c;
    batch=params.batch;
	layer.calloc_or_fail=0;
    if(!(h && w && c)) 
	{
		printf("Layer before maxpool layer must output image.\n");
		layer.calloc_or_fail=-3;
		return layer;
		//error("Layer before maxpool layer must output image.");
	}

    layer = make_maxpool_layer(batch,h,w,c,size,stride,padding);
	if(layer.calloc_or_fail < 0)
	{
		printf("Error in make_maxpool_layer.\n");
		return layer;
	}
    return layer;
}

avgpool_layer parse_avgpool(list *options, size_params params)
{
	avgpool_layer layer;
    int batch,w,h,c;
    w = params.w;
    h = params.h;
    c = params.c;
    batch=params.batch;
    if(!(h && w && c)) error("Layer before avgpool layer must output image.");

    layer = make_avgpool_layer(batch,w,h,c);
    return layer;
}

dropout_layer parse_dropout(list *options, size_params params)
{
    float probability = option_find_float(options, "probability", .5);
    dropout_layer layer = make_dropout_layer(params.batch, params.inputs, probability);
    layer.out_w = params.w;
    layer.out_h = params.h;
    layer.out_c = params.c;
    return layer;
}

layer parse_normalization(list *options, size_params params)
{
    float alpha = option_find_float(options, "alpha", .0001);
    float beta =  option_find_float(options, "beta" , .75);
    float kappa = option_find_float(options, "kappa", 1);
    int size = option_find_int(options, "size", 5);
    layer l = make_normalization_layer(params.batch, params.w, params.h, params.c, size, alpha, beta, kappa);
    return l;
}

layer parse_batchnorm(list *options, size_params params)
{
    layer l = make_batchnorm_layer(params.batch, params.w, params.h, params.c);
    return l;
}

layer parse_shortcut(list *options, size_params params, network net)
{
	int batch;
	layer from,s;
	char *activation_s;
	ACTIVATION activation;
    char *l = option_find(options, "from");   
    int index = atoi(l);
    if(index < 0) index = params.index + index;

    batch = params.batch;
    from = net.layers[index];

    s = make_shortcut_layer(batch, index, params.w, params.h, params.c, from.out_w, from.out_h, from.out_c);

    activation_s = option_find_str(options, "activation", "linear");
    activation = get_activation(activation_s);
    s.activation = activation;
    return s;
}


layer parse_activation(list *options, size_params params)
{
    char *activation_s = option_find_str(options, "activation", "linear");
    ACTIVATION activation = get_activation(activation_s);

    layer l = make_activation_layer(params.batch, params.inputs, activation);

    l.out_h = params.h;
    l.out_w = params.w;
    l.out_c = params.c;
    l.h = params.h;
    l.w = params.w;
    l.c = params.c;

    return l;
}

route_layer parse_route(list *options, size_params params, network net)
{
	route_layer layer;
	convolutional_layer first;
    char *l = option_find(options, "layers");   
    int len = strlen(l);
	int n = 1;
    int i;
	int *layers,*sizes;
	int batch;
    if(!l) 
	{
		printf("Route Layer must specify input layers\n");
		layer.calloc_or_fail=-1;
		return layer;
		//error("Route Layer must specify input layers");
	}
    
    for(i = 0; i < len; ++i){
        if (l[i] == ',') ++n;
    }

    layers = calloc(n, sizeof(int));
	if(layers==NULL)
	{
		printf("Error in calloc.\n");
		layer.calloc_or_fail=-1;
		return layer;
	}
    sizes = calloc(n, sizeof(int));
	if(sizes==NULL)
	{
		printf("Error in calloc.\n");
		layer.calloc_or_fail=-1;
		free(layers);
		return layer;
	}
    for(i = 0; i < n; ++i){
        int index = atoi(l);
        l = strchr(l, ',')+1;
        if(index < 0) index = params.index + index;
        layers[i] = index;
        sizes[i] = net.layers[index].outputs;
    }
    batch = params.batch;

    layer = make_route_layer(batch, n, layers, sizes);
	if(layer.calloc_or_fail < 0)
	{
		printf("Error in parse_route.\n");
		return layer;
	}

    first = net.layers[layers[0]];
    layer.out_w = first.out_w;
    layer.out_h = first.out_h;
    layer.out_c = first.out_c;
    for(i = 1; i < n; ++i){
        int index = layers[i];
        convolutional_layer next = net.layers[index];
        if(next.out_w == first.out_w && next.out_h == first.out_h){
            layer.out_c += next.out_c;
        }else{
            layer.out_h = layer.out_w = layer.out_c = 0;
        }
    }

    return layer;
}

learning_rate_policy get_policy(char *s)
{
    if (strcmp(s, "random")==0) return RANDOM;
    if (strcmp(s, "poly")==0) return POLY;
    if (strcmp(s, "constant")==0) return CONSTANT;
    if (strcmp(s, "step")==0) return STEP;
    if (strcmp(s, "exp")==0) return EXP;
    if (strcmp(s, "sigmoid")==0) return SIG;
    if (strcmp(s, "steps")==0) return STEPS;
    fprintf(stderr, "Couldn't find policy %s, going with constant\n", s);
    return CONSTANT;
}

void parse_net_options(list *options, network *net)
{
	int subdivs;
	char *policy_s;
    net->batch = option_find_int(options, "batch",1);
    net->learning_rate = option_find_float(options, "learning_rate", .001);
    net->momentum = option_find_float(options, "momentum", .9);
    net->decay = option_find_float(options, "decay", .0001);
    subdivs = option_find_int(options, "subdivisions",1);
    net->time_steps = option_find_int_quiet(options, "time_steps",1);
    net->batch /= subdivs;
    net->batch *= net->time_steps;
    net->subdivisions = subdivs;

    net->adam = option_find_int_quiet(options, "adam", 0);
    if(net->adam){
        net->B1 = option_find_float(options, "B1", .9);
        net->B2 = option_find_float(options, "B2", .999);
        net->eps = option_find_float(options, "eps", .000001);
    }

    net->h = option_find_int_quiet(options, "height",0);
    net->w = option_find_int_quiet(options, "width",0);
    net->c = option_find_int_quiet(options, "channels",0);
    net->inputs = option_find_int_quiet(options, "inputs", net->h * net->w * net->c);
    net->max_crop = option_find_int_quiet(options, "max_crop",net->w*2);
    net->min_crop = option_find_int_quiet(options, "min_crop",net->w);

    net->angle = option_find_float_quiet(options, "angle", 0);
    net->aspect = option_find_float_quiet(options, "aspect", 1);
    net->saturation = option_find_float_quiet(options, "saturation", 1);
    net->exposure = option_find_float_quiet(options, "exposure", 1);
    net->hue = option_find_float_quiet(options, "hue", 0);

    if(!net->inputs && !(net->h && net->w && net->c)) 
	{
		printf("No input parameters supplied\n");
		net->errorcode=-3;
		return;
		//error("No input parameters supplied");
	}

    policy_s = option_find_str(options, "policy", "constant");
    net->policy = get_policy(policy_s);
    net->burn_in = option_find_int_quiet(options, "burn_in", 0);
    if(net->policy == STEP){
        net->step = option_find_int(options, "step", 1);
        net->scale = option_find_float(options, "scale", 1);
    } else if (net->policy == STEPS){
        char *l = option_find(options, "steps");   
        char *p = option_find(options, "scales");   
		int len,n,i;
		int *steps;
		float *scales;
        if(!l || !p) 
		{
            printf("STEPS policy must have steps and scales in cfg file\n");
			net->errorcode=-3;
			return;
			//error("STEPS policy must have steps and scales in cfg file");
		}

        len = strlen(l);
        n = 1;
        for(i = 0; i < len; ++i){
            if (l[i] == ',') ++n;
        }
        steps = calloc(n, sizeof(int));
		if(steps==NULL)
		{
			printf("Error in calloc.\n");
			net->errorcode=-1;
			return;
		}
        scales = calloc(n, sizeof(float));
		if(scales==NULL)
		{
			printf("Error in calloc.\n");
			net->errorcode=-1;
			free(steps);
			return;
		}
        for(i = 0; i < n; ++i){
            int step    = atoi(l);
            float scale = atof(p);
            l = strchr(l, ',')+1;
            p = strchr(p, ',')+1;
            steps[i] = step;
            scales[i] = scale;
        }
        net->scales = scales;
        net->steps = steps;
        net->num_steps = n;
    } else if (net->policy == EXP){
        net->gamma = option_find_float(options, "gamma", 1);
    } else if (net->policy == SIG){
        net->gamma = option_find_float(options, "gamma", 1);
        net->step = option_find_int(options, "step", 1);
    } else if (net->policy == POLY || net->policy == RANDOM){
        net->power = option_find_float(options, "power", 1);
    }
    net->max_batches = option_find_int(options, "max_batches", 0);
}

int is_network(section *s)
{
    return (strcmp(s->type, "[net]")==0
            || strcmp(s->type, "[network]")==0);
}
network COM_parse_network_cfg_adapt_GPU(char *filename,int w,int h)
{
	network net={0};
	size_params params;
	section *s;
	list *options;
	size_t workspace_size = 0;
	int count = 0;
	list *sections = read_cfg(filename);
	node *n;
	if(sections==NULL)
	{
		printf("sections is NULL.\n");
		net.errorcode=-1;
        return net;
	}
    if(sections->errorcode != 0)
	{
		printf("Error in read_cfg.\n");
		net.errorcode=sections->errorcode;
		n = sections->front;
		if(n)
		{
			s = (section *)n->val;
			if(s)
			{
				free_section(s);
			}
		}
		free_list(sections);
        return net;
	}
	n = sections->front;
	if (!n) 
	{
		printf("Config file has no sections\n");
		net.errorcode=-3;
		if(n)
		{
			s = (section *)n->val;
			if(s)
			{
				free_section(s);
			}
		}
		free_list(sections);
		return net;
		//error("Config file has no sections");
	}
    net = make_network(sections->size - 1);
	net.gpu_index = gpu_index;
	if(net.errorcode < 0)
	{
		printf("Error in make_network.\n");
		s = (section *)n->val;
		if(s)
		{
			free_section(s);
		}
		free_list(sections);
		return net;
	}

	s = (section *)n->val;
	options = s->options;
	if (!is_network(s)) 
	{
		printf("First section must be [net] or [network]\n");
		net.errorcode=-3;
		free_section(s);
		free_list(sections);
		return net;
		//error("First section must be [net] or [network]");
	}
	parse_net_options(options, &net);
	if(net.errorcode < 0)
	{
		printf("Error in parse_net_options.\n");
		free_section(s);
		free_list(sections);
		return net;
	}

	if (MIN(w, h) == w)
	{
		net.h = (int)(1.0 * 352 / w*h);
		net.h = net.h / 32 * 32;
		net.w = 352;
		if (net.h > 576)
		{
			net.h = 576;
			net.w = (int)(1.0 * 576 / h*w);
			net.w = net.w / 32 * 32;
		}
	}
	else
	{
		net.w = (int)(1.0 * 352 / h*w);
		net.w = net.w / 32 * 32;
		net.h = 352;
		if (net.w > 576)
		{
			net.w = 576;
			net.h = (int)(1.0 * 576 / w*h);
			net.h = net.h / 32 * 32;
		}
	}
	//printf("h=%d w=%d\n", net.h, net.w);
	params.h = net.h;
	params.w = net.w;
	params.c = net.c;
	params.inputs = net.inputs;
	params.batch = net.batch;
	params.time_steps = net.time_steps;
	params.net = net;

	
	n = n->next;
	free_section(s);
	//fprintf(stderr, "layer     filters    size              input                output\n");
	while (n){
		layer l = { 0 };
		LAYER_TYPE lt;
		params.index = count;
		//fprintf(stderr, "%5d ", count);
		s = (section *)n->val;
		options = s->options;
		
	    lt = string_to_layer_type(s->type);
		if (lt == CONVOLUTIONAL){
			l = parse_convolutional(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_convolutional.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
		}
		else if (lt == LOCAL){
			l = parse_local(options, params);
		}
		else if (lt == ACTIVE){
			l = parse_activation(options, params);
		}
		else if (lt == RNN){
			l = parse_rnn(options, params);
		}
		else if (lt == GRU){
			l = parse_gru(options, params);
		}
		else if (lt == CRNN){
			l = parse_crnn(options, params);
		}
		else if (lt == CONNECTED){
			l = parse_connected(options, params);
		}
		else if (lt == CROP){
			l = parse_crop(options, params);
		}
		else if (lt == COST){
			l = parse_cost(options, params);
		}
		else if (lt == REGION){
			l = parse_region(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_region.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
		}
		else if (lt == DETECTION){
			l = parse_detection(options, params);
		}
		else if (lt == SOFTMAX){
			l = parse_softmax(options, params);
			net.hierarchy = l.softmax_tree;
		}
		else if (lt == NORMALIZATION){
			l = parse_normalization(options, params);
		}
		else if (lt == BATCHNORM){
			l = parse_batchnorm(options, params);
		}
		else if (lt == MAXPOOL){
			l = parse_maxpool(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_maxpool.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
		}
		else if (lt == REORG){
			l = parse_reorg(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_reorg.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
		}
		else if (lt == AVGPOOL){
			l = parse_avgpool(options, params);
		}
		else if (lt == ROUTE){
			l = parse_route(options, params, net);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_route.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
		}
		else if (lt == SHORTCUT){
			l = parse_shortcut(options, params, net);
		}
		else if (lt == DROPOUT){
			l = parse_dropout(options, params);
			l.output = net.layers[count - 1].output;
			l.delta = net.layers[count - 1].delta;
#ifdef GPU
			l.output_gpu = net.layers[count - 1].output_gpu;
			l.delta_gpu = net.layers[count - 1].delta_gpu;
#endif
		}
		else{
			fprintf(stderr, "Type not recognized: %s\n", s->type);
		}
		l.dontload = option_find_int_quiet(options, "dontload", 0);
		l.dontloadscales = option_find_int_quiet(options, "dontloadscales", 0);
		option_unused(options);
		net.layers[count] = l;
		if (l.workspace_size > workspace_size) workspace_size = l.workspace_size;
		free_section(s);
		n = n->next;
		++count;
		if (n){
			params.h = l.out_h;
			params.w = l.out_w;
			params.c = l.out_c;
			params.inputs = l.outputs;
		}
	}
	free_list(sections);
	net.outputs = get_network_output_size(net);
	net.output = get_network_output(net);
	if (workspace_size){
		//printf("%ld\n", workspace_size);
#ifdef GPU
		if (gpu_index >= 0){
			net.workspace = cuda_make_array(0, (workspace_size - 1) / sizeof(float) + 1);
		}
		else {
			net.workspace = calloc(1, workspace_size);
		}
#else
		net.workspace = calloc(1, workspace_size);
#endif
	}
	return net;
}
network COM_parse_network_cfg_GPU(char *filename)
{
	network net = {0};
	size_params params;
	section *s;
	list *options;
	size_t workspace_size = 0;
    int count = 0;
    list *sections = read_cfg(filename);
    node *n;
	if(sections==NULL)
	{
		printf("sections is NULL.\n");
		net.errorcode=-1;
        return net;
	}
    if(sections->errorcode != 0)
	{
		printf("Error in read_cfg.\n");
		net.errorcode=sections->errorcode;
		n = sections->front;
		if(n)
		{
			s = (section *)n->val;
			if(s)
			{
				free_section(s);
			}
		}
		free_list(sections);
        return net;
	}
	n = sections->front;
    if(!n) 
	{
		printf("Config file has no sections\n");
		net.errorcode=-3;
		if(n)
		{
			s = (section *)n->val;
			if(s)
			{
				free_section(s);
			}
		}
		free_list(sections);
		return net;
		//error("Config file has no sections");
	}
    net = make_network(sections->size - 1);
    net.gpu_index = gpu_index;
    if(net.errorcode < 0)
	{
		printf("Error in make_network.\n");
		s = (section *)n->val;
		if(s)
		{
			free_section(s);
		}
		free_list(sections);
		return net;
	}

    s = (section *)n->val;
    options = s->options;
    if(!is_network(s)) 
	{
		printf("First section must be [net] or [network]\n");
		net.errorcode=-3;
		free_section(s);
		free_list(sections);
		return net;
		//error("First section must be [net] or [network]");
	}
    parse_net_options(options, &net);
	if(net.errorcode < 0)
	{
		printf("Error in parse_net_options.\n");
		free_section(s);
		free_list(sections);
		return net;
	}

    params.h = net.h;
    params.w = net.w;
    params.c = net.c;
    params.inputs = net.inputs;
    params.batch = net.batch;
    params.time_steps = net.time_steps;
    params.net = net;

    
    n = n->next;
    free_section(s);
    //fprintf(stderr, "layer     filters    size              input                output\n");
    while(n){
		layer l = {0};
		LAYER_TYPE lt;
        params.index = count;
       // fprintf(stderr, "%5d ", count);
        s = (section *)n->val;
        options = s->options;
        
        lt = string_to_layer_type(s->type);
        if(lt == CONVOLUTIONAL){
            l = parse_convolutional(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_convolutional.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
        }else if(lt == LOCAL){
            l = parse_local(options, params);
        }else if(lt == ACTIVE){
            l = parse_activation(options, params);
        }else if(lt == RNN){
            l = parse_rnn(options, params);
        }else if(lt == GRU){
            l = parse_gru(options, params);
        }else if(lt == CRNN){
            l = parse_crnn(options, params);
        }else if(lt == CONNECTED){
            l = parse_connected(options, params);
        }else if(lt == CROP){
            l = parse_crop(options, params);
        }else if(lt == COST){
            l = parse_cost(options, params);
        }else if(lt == REGION){
            l = parse_region(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_region.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
        }else if(lt == DETECTION){
            l = parse_detection(options, params);
        }else if(lt == SOFTMAX){
            l = parse_softmax(options, params);
            net.hierarchy = l.softmax_tree;
        }else if(lt == NORMALIZATION){
            l = parse_normalization(options, params);
        }else if(lt == BATCHNORM){
            l = parse_batchnorm(options, params);
        }else if(lt == MAXPOOL){
            l = parse_maxpool(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_maxpool.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
        }else if(lt == REORG){
            l = parse_reorg(options, params);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_reorg.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
        }else if(lt == AVGPOOL){
            l = parse_avgpool(options, params);
        }else if(lt == ROUTE){
            l = parse_route(options, params, net);
			if(l.calloc_or_fail < 0)
			{
				printf("Error in parse_route.\n");
				net.errorcode=l.calloc_or_fail;
				free_section(s);
				free_list(sections);
				return net;
			}
        }else if(lt == SHORTCUT){
            l = parse_shortcut(options, params, net);
        }else if(lt == DROPOUT){
            l = parse_dropout(options, params);
            l.output = net.layers[count-1].output;
            l.delta = net.layers[count-1].delta;
#ifdef GPU
            l.output_gpu = net.layers[count-1].output_gpu;
            l.delta_gpu = net.layers[count-1].delta_gpu;
#endif
        }else{
            fprintf(stderr, "Type not recognized: %s\n", s->type);
        }
        l.dontload = option_find_int_quiet(options, "dontload", 0);
        l.dontloadscales = option_find_int_quiet(options, "dontloadscales", 0);
        option_unused(options);
        net.layers[count] = l;
        if (l.workspace_size > workspace_size) workspace_size = l.workspace_size;
        free_section(s);
        n = n->next;
        ++count;
        if(n){
            params.h = l.out_h;
            params.w = l.out_w;
            params.c = l.out_c;
            params.inputs = l.outputs;
        }
    }   
    free_list(sections);
    net.outputs = get_network_output_size(net);
    net.output = get_network_output(net);
    if(workspace_size){
        //printf("%ld\n", workspace_size);
#ifdef GPU
        if(gpu_index >= 0){
            net.workspace = cuda_make_array(0, (workspace_size-1)/sizeof(float)+1);
        }else {
            net.workspace = calloc(1, workspace_size);
        }
#else
        net.workspace = calloc(1, workspace_size);
		if(net.workspace==NULL)
		{
			printf("Error in calloc.\n");
			net.errorcode=-1;
			return net;
		}

#endif
    }
    return net;
}

network parse_network_cfg(char *filename)
{
	network net;
	size_params params;
	section *s;
	list *options;
	size_t workspace_size = 0;
    int count = 0;
    list *sections = read_cfg(filename);
    node *n = sections->front;
    if(!n) error("Config file has no sections");
    net = make_network(sections->size - 1);
    net.gpu_index = gpu_index;
    

    s = (section *)n->val;
    options = s->options;
    if(!is_network(s)) error("First section must be [net] or [network]");
    parse_net_options(options, &net);

    params.h = net.h;
    params.w = net.w;
    params.c = net.c;
    params.inputs = net.inputs;
    params.batch = net.batch;
    params.time_steps = net.time_steps;
    params.net = net;

    
    n = n->next;
    free_section(s);
    //fprintf(stderr, "layer     filters    size              input                output\n");
    while(n){
		layer l = {0};
		LAYER_TYPE lt;
        params.index = count;
       // fprintf(stderr, "%5d ", count);
        s = (section *)n->val;
        options = s->options;
        
        lt = string_to_layer_type(s->type);
        if(lt == CONVOLUTIONAL){
            l = parse_convolutional(options, params);
        }else if(lt == LOCAL){
            l = parse_local(options, params);
        }else if(lt == ACTIVE){
            l = parse_activation(options, params);
        }else if(lt == RNN){
            l = parse_rnn(options, params);
        }else if(lt == GRU){
            l = parse_gru(options, params);
        }else if(lt == CRNN){
            l = parse_crnn(options, params);
        }else if(lt == CONNECTED){
            l = parse_connected(options, params);
        }else if(lt == CROP){
            l = parse_crop(options, params);
        }else if(lt == COST){
            l = parse_cost(options, params);
        }else if(lt == REGION){
            l = parse_region(options, params);
        }else if(lt == DETECTION){
            l = parse_detection(options, params);
        }else if(lt == SOFTMAX){
            l = parse_softmax(options, params);
            net.hierarchy = l.softmax_tree;
        }else if(lt == NORMALIZATION){
            l = parse_normalization(options, params);
        }else if(lt == BATCHNORM){
            l = parse_batchnorm(options, params);
        }else if(lt == MAXPOOL){
            l = parse_maxpool(options, params);
        }else if(lt == REORG){
            l = parse_reorg(options, params);
        }else if(lt == AVGPOOL){
            l = parse_avgpool(options, params);
        }else if(lt == ROUTE){
            l = parse_route(options, params, net);
        }else if(lt == SHORTCUT){
            l = parse_shortcut(options, params, net);
        }else if(lt == DROPOUT){
            l = parse_dropout(options, params);
            l.output = net.layers[count-1].output;
            l.delta = net.layers[count-1].delta;
#ifdef GPU
            l.output_gpu = net.layers[count-1].output_gpu;
            l.delta_gpu = net.layers[count-1].delta_gpu;
#endif
        }else{
            fprintf(stderr, "Type not recognized: %s\n", s->type);
        }
        l.dontload = option_find_int_quiet(options, "dontload", 0);
        l.dontloadscales = option_find_int_quiet(options, "dontloadscales", 0);
        option_unused(options);
        net.layers[count] = l;
        if (l.workspace_size > workspace_size) workspace_size = l.workspace_size;
        free_section(s);
        n = n->next;
        ++count;
        if(n){
            params.h = l.out_h;
            params.w = l.out_w;
            params.c = l.out_c;
            params.inputs = l.outputs;
        }
    }   
    free_list(sections);
    net.outputs = get_network_output_size(net);
    net.output = get_network_output(net);
    if(workspace_size){
        //printf("%ld\n", workspace_size);
#ifdef GPU
        if(gpu_index >= 0){
            net.workspace = cuda_make_array(0, (workspace_size-1)/sizeof(float)+1);
        }else {
            net.workspace = calloc(1, workspace_size);
        }
#else
        net.workspace = calloc(1, workspace_size);
#endif
    }
    return net;
}

list *read_cfg(char *filename)
{
	char *line;
    int nu = 0;
	list *sections;
	section *current = 0;
    FILE *file;
	sections = make_list();
	if(sections==NULL)
	{
		return sections;
	}
	file = fopen(filename, "r");
    if(file == 0) 
	{
		printf("fopen cfg return NULL\n");
		sections->errorcode=-100;
		return sections;
		//file_error(filename);
	}
    
    while((line=fgetl(file)) != 0){
        ++ nu;
        strip(line);
        switch(line[0]){
            case '[':
                current = malloc(sizeof(section));
				if(current==NULL)
				{
					printf("Error in malloc.\n");
					sections->errorcode=-1;
					fclose(file);
					return sections;
				}
                list_insert(sections, current);
				if(sections->errorcode==-1)
				{
					free(current);
					fclose(file);
					return sections;
				}
                current->options = make_list();
				if(current->options==NULL)
				{
					sections->errorcode=-1;
					fclose(file);
					return sections;
				}
                current->type = line;
                break;
            case '\0':
            case '#':
            case ';':
                free(line);
                break;
            default:
                if(!read_option(line, current->options)){
                    fprintf(stderr, "Config file error line %d, could parse: %s\n", nu, line);
                    free(line);
                }
                break;
        }
    }
    fclose(file);
    return sections;
}

void save_convolutional_weights_binary(layer l, FILE *fp)
{
#ifdef GPU
    if(gpu_index >= 0){
        pull_convolutional_layer(l);
    }
#endif
	int size,i, j, k;
    binarize_weights(l.weights, l.n, l.c*l.size*l.size, l.binary_weights);
    size = l.c*l.size*l.size;
    fwrite(l.biases, sizeof(float), l.n, fp);
    if (l.batch_normalize){
        fwrite(l.scales, sizeof(float), l.n, fp);
        fwrite(l.rolling_mean, sizeof(float), l.n, fp);
        fwrite(l.rolling_variance, sizeof(float), l.n, fp);
    }
    for(i = 0; i < l.n; ++i){
        float mean = l.binary_weights[i*size];
        if(mean < 0) mean = -mean;
        fwrite(&mean, sizeof(float), 1, fp);
        for(j = 0; j < size/8; ++j){
            int index = i*size + j*8;
            unsigned char c = 0;
            for(k = 0; k < 8; ++k){
                if (j*8 + k >= size) break;
                if (l.binary_weights[index + k] > 0) c = (c | 1<<k);
            }
            fwrite(&c, sizeof(char), 1, fp);
        }
    }
}

void save_convolutional_weights(layer l, FILE *fp)
{
	int num;
    if(l.binary){
        //save_convolutional_weights_binary(l, fp);
        //return;
    }
#ifdef GPU
    if(gpu_index >= 0){
        pull_convolutional_layer(l);
    }
#endif
    num = l.n*l.c*l.size*l.size;
    fwrite(l.biases, sizeof(float), l.n, fp);
    if (l.batch_normalize){
        fwrite(l.scales, sizeof(float), l.n, fp);
        fwrite(l.rolling_mean, sizeof(float), l.n, fp);
        fwrite(l.rolling_variance, sizeof(float), l.n, fp);
    }
    fwrite(l.weights, sizeof(float), num, fp);
    if(l.adam){
        fwrite(l.m, sizeof(float), num, fp);
        fwrite(l.v, sizeof(float), num, fp);
    }
}

void save_batchnorm_weights(layer l, FILE *fp)
{
#ifdef GPU
    if(gpu_index >= 0){
        pull_batchnorm_layer(l);
    }
#endif
    fwrite(l.scales, sizeof(float), l.c, fp);
    fwrite(l.rolling_mean, sizeof(float), l.c, fp);
    fwrite(l.rolling_variance, sizeof(float), l.c, fp);
}

void save_connected_weights(layer l, FILE *fp)
{
#ifdef GPU
    if(gpu_index >= 0){
        pull_connected_layer(l);
    }
#endif
    fwrite(l.biases, sizeof(float), l.outputs, fp);
    fwrite(l.weights, sizeof(float), l.outputs*l.inputs, fp);
    if (l.batch_normalize){
        fwrite(l.scales, sizeof(float), l.outputs, fp);
        fwrite(l.rolling_mean, sizeof(float), l.outputs, fp);
        fwrite(l.rolling_variance, sizeof(float), l.outputs, fp);
    }
}

void save_weights_upto(network net, char *filename, int cutoff)
{
#ifdef GPU
    if(net.gpu_index >= 0){
        cuda_set_device(net.gpu_index);
    }
#endif
	FILE *fp;
	int major = 0;
    int minor = 1;
    int revision = 0;
	int i;
    fprintf(stderr, "Saving weights to %s\n", filename);
    fp = fopen(filename, "wb");
    if(!fp) file_error(filename);

    
    fwrite(&major, sizeof(int), 1, fp);
    fwrite(&minor, sizeof(int), 1, fp);
    fwrite(&revision, sizeof(int), 1, fp);
    fwrite(net.seen, sizeof(int), 1, fp);

    for(i = 0; i < net.n && i < cutoff; ++i){
        layer l = net.layers[i];
        if(l.type == CONVOLUTIONAL){
            save_convolutional_weights(l, fp);
        } if(l.type == CONNECTED){
            save_connected_weights(l, fp);
        } if(l.type == BATCHNORM){
            save_batchnorm_weights(l, fp);
        } if(l.type == RNN){
            save_connected_weights(*(l.input_layer), fp);
            save_connected_weights(*(l.self_layer), fp);
            save_connected_weights(*(l.output_layer), fp);
        } if(l.type == GRU){
            save_connected_weights(*(l.input_z_layer), fp);
            save_connected_weights(*(l.input_r_layer), fp);
            save_connected_weights(*(l.input_h_layer), fp);
            save_connected_weights(*(l.state_z_layer), fp);
            save_connected_weights(*(l.state_r_layer), fp);
            save_connected_weights(*(l.state_h_layer), fp);
        } if(l.type == CRNN){
            save_convolutional_weights(*(l.input_layer), fp);
            save_convolutional_weights(*(l.self_layer), fp);
            save_convolutional_weights(*(l.output_layer), fp);
        } if(l.type == LOCAL){
#ifdef GPU
            if(gpu_index >= 0){
                pull_local_layer(l);
            }
#endif
            int locations = l.out_w*l.out_h;
            int size = l.size*l.size*l.c*l.n*locations;
            fwrite(l.biases, sizeof(float), l.outputs, fp);
            fwrite(l.weights, sizeof(float), size, fp);
        }
    }
    fclose(fp);
}
void save_weights(network net, char *filename)
{
    save_weights_upto(net, filename, net.n);
}

void transpose_matrix(float *a, int rows, int cols)
{
    float *transpose = calloc(rows*cols, sizeof(float));
    int x, y;
    for(x = 0; x < rows; ++x){
        for(y = 0; y < cols; ++y){
            transpose[y*rows + x] = a[x*cols + y];
        }
    }
    memcpy(a, transpose, rows*cols*sizeof(float));
    free(transpose);
}

void load_connected_weights(layer l, FILE *fp, int transpose)
{
    fread(l.biases, sizeof(float), l.outputs, fp);
    fread(l.weights, sizeof(float), l.outputs*l.inputs, fp);
    if(transpose){
        transpose_matrix(l.weights, l.inputs, l.outputs);
    }
    //printf("Biases: %f mean %f variance\n", mean_array(l.biases, l.outputs), variance_array(l.biases, l.outputs));
    //printf("Weights: %f mean %f variance\n", mean_array(l.weights, l.outputs*l.inputs), variance_array(l.weights, l.outputs*l.inputs));
    if (l.batch_normalize && (!l.dontloadscales)){
        fread(l.scales, sizeof(float), l.outputs, fp);
        fread(l.rolling_mean, sizeof(float), l.outputs, fp);
        fread(l.rolling_variance, sizeof(float), l.outputs, fp);
        //printf("Scales: %f mean %f variance\n", mean_array(l.scales, l.outputs), variance_array(l.scales, l.outputs));
        //printf("rolling_mean: %f mean %f variance\n", mean_array(l.rolling_mean, l.outputs), variance_array(l.rolling_mean, l.outputs));
        //printf("rolling_variance: %f mean %f variance\n", mean_array(l.rolling_variance, l.outputs), variance_array(l.rolling_variance, l.outputs));
    }
#ifdef GPU
    if(gpu_index >= 0){
        push_connected_layer(l);
    }
#endif
}

void load_batchnorm_weights(layer l, FILE *fp)
{
    fread(l.scales, sizeof(float), l.c, fp);
    fread(l.rolling_mean, sizeof(float), l.c, fp);
    fread(l.rolling_variance, sizeof(float), l.c, fp);
#ifdef GPU
    if(gpu_index >= 0){
        push_batchnorm_layer(l);
    }
#endif
}

void load_convolutional_weights_binary(layer l, FILE *fp)
{
	int size,i, j, k;
    fread(l.biases, sizeof(float), l.n, fp);
    if (l.batch_normalize && (!l.dontloadscales)){
        fread(l.scales, sizeof(float), l.n, fp);
        fread(l.rolling_mean, sizeof(float), l.n, fp);
        fread(l.rolling_variance, sizeof(float), l.n, fp);
    }
    size = l.c*l.size*l.size;
    for(i = 0; i < l.n; ++i){
        float mean = 0;
        fread(&mean, sizeof(float), 1, fp);
        for(j = 0; j < size/8; ++j){
            int index = i*size + j*8;
            unsigned char c = 0;
            fread(&c, sizeof(char), 1, fp);
            for(k = 0; k < 8; ++k){
                if (j*8 + k >= size) break;
                l.weights[index + k] = (c & 1<<k) ? mean : -mean;
            }
        }
    }
#ifdef GPU
    if(gpu_index >= 0){
        push_convolutional_layer(l);
    }
#endif
}

void load_convolutional_weights(layer l, FILE *fp)
{
	int num;
    if(l.binary){
        //load_convolutional_weights_binary(l, fp);
        //return;
    }
    num = l.n*l.c*l.size*l.size;
    fread(l.biases, sizeof(float), l.n, fp);
    if (l.batch_normalize && (!l.dontloadscales)){
        fread(l.scales, sizeof(float), l.n, fp);
        fread(l.rolling_mean, sizeof(float), l.n, fp);
        fread(l.rolling_variance, sizeof(float), l.n, fp);
        if(0){
            int i;
            for(i = 0; i < l.n; ++i){
                printf("%g, ", l.rolling_mean[i]);
            }
            printf("\n");
            for(i = 0; i < l.n; ++i){
                printf("%g, ", l.rolling_variance[i]);
            }
            printf("\n");
        }
        if(0){
            fill_cpu(l.n, 0, l.rolling_mean, 1);
            fill_cpu(l.n, 0, l.rolling_variance, 1);
        }
    }
    fread(l.weights, sizeof(float), num, fp);
    if(l.adam){
        fread(l.m, sizeof(float), num, fp);
        fread(l.v, sizeof(float), num, fp);
    }
    //if(l.c == 3) scal_cpu(num, 1./256, l.weights, 1);
    if (l.flipped) {
		printf("l.flipped\n");
        transpose_matrix(l.weights, l.c*l.size*l.size, l.n);
    }
    //if (l.binary) binarize_weights(l.weights, l.n, l.c*l.size*l.size, l.weights);
#ifdef GPU
    if(gpu_index >= 0){
        push_convolutional_layer(l);
    }
#endif
}


void load_weights_upto(network *net, char *filename, int cutoff)
{
#ifdef GPU
    if(net->gpu_index >= 0){
        cuda_set_device(net->gpu_index);
    }
#endif
	FILE *fp;
	int major;
    int minor;
    int revision;
	int transpose,i;
    //fprintf(stderr, "Loading weights from %s...", filename);
    fflush(stdout);
    fp = fopen(filename, "rb");
    if(!fp) 
	{
		printf("fopen weight return NULL\n");
		net->errorcode=-101;
		return;
		//file_error(filename);
	}

    
    fread(&major, sizeof(int), 1, fp);
    fread(&minor, sizeof(int), 1, fp);
    fread(&revision, sizeof(int), 1, fp);
	if ((major * 10 + minor) >= 2) {
		fread(net->seen, sizeof(uint64_t), 1, fp);
	}
	else {
		int iseen = 0;
		fread(&iseen, sizeof(int), 1, fp);
		*net->seen = iseen;
	}
    transpose = (major > 1000) || (minor > 1000);

    for(i = 0; i < net->n && i < cutoff; ++i){
        layer l = net->layers[i];
        if (l.dontload) continue;
        if(l.type == CONVOLUTIONAL){
            load_convolutional_weights(l, fp);
        }
        if(l.type == CONNECTED){
            load_connected_weights(l, fp, transpose);
        }
        if(l.type == BATCHNORM){
            load_batchnorm_weights(l, fp);
        }
        if(l.type == CRNN){
            load_convolutional_weights(*(l.input_layer), fp);
            load_convolutional_weights(*(l.self_layer), fp);
            load_convolutional_weights(*(l.output_layer), fp);
        }
        if(l.type == RNN){
            load_connected_weights(*(l.input_layer), fp, transpose);
            load_connected_weights(*(l.self_layer), fp, transpose);
            load_connected_weights(*(l.output_layer), fp, transpose);
        }
        if(l.type == GRU){
            load_connected_weights(*(l.input_z_layer), fp, transpose);
            load_connected_weights(*(l.input_r_layer), fp, transpose);
            load_connected_weights(*(l.input_h_layer), fp, transpose);
            load_connected_weights(*(l.state_z_layer), fp, transpose);
            load_connected_weights(*(l.state_r_layer), fp, transpose);
            load_connected_weights(*(l.state_h_layer), fp, transpose);
        }
        if(l.type == LOCAL){
            int locations = l.out_w*l.out_h;
            int size = l.size*l.size*l.c*l.n*locations;
            fread(l.biases, sizeof(float), l.outputs, fp);
            fread(l.weights, sizeof(float), size, fp);
#ifdef GPU
            if(gpu_index >= 0){
                push_local_layer(l);
            }
#endif
        }
    }
   // fprintf(stderr, "Done!\n");
    fclose(fp);
}
void COM_load_weights_GPU(network *net, char *filename)
{
    load_weights_upto(net, filename, net->n);
}
void load_weights(network *net, char *filename)
{
    load_weights_upto(net, filename, net->n);
}

