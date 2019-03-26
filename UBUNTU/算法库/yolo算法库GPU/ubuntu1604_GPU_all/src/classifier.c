#include "network.h"
#include "utils.h"
#include "parser.h"
#include "option_list.h"
#include "blas.h"
#include "assert.h"
#include "classifier.h"
#include "cuda.h"
#ifdef WIN32
#include <time.h>
#include <winsock.h>
#include "gettimeofday.h"
#else
#include <sys/time.h>
#endif

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/core/version.hpp"
#ifndef CV_VERSION_EPOCH
#include "opencv2/videoio/videoio_c.h"
#endif
image get_image_from_stream(CvCapture *cap);
#endif

float *get_regression_values(char **labels, int n)
{
    float *v =(float*)calloc(n, sizeof(float));
    int i;
    for(i = 0; i < n; ++i){
        char *p = strchr(labels[i], ' ');
        *p = 0;
        v[i] = atof(p+1);
    }
    return v;
}

void train_classifier(char *datacfg, char *cfgfile, char *weightfile, int *gpus, int ngpus, int clear)
{
    int i,seed,imgs,classes,N,epoch;
	network net;
    float avg_loss = -1;
	char *backup_directory;
	char *label_list;
	char *train_list;
    char *base = basecfg(cfgfile);
	list *options;
	char **labels;
	list *plist;
	clock_t time1;
	char **paths;
	char buff[256];
	network *nets;
	load_args args={0};
	data train;
    data buffer;
	pthread_t load_thread;
	float loss;
    printf("%s\n", base);
    printf("%d\n", ngpus);
    nets = (network *)calloc(ngpus, sizeof(network));
	
    srand(time(0));
    seed = rand();
    for(i = 0; i < ngpus; ++i){
        srand(seed);
#ifdef GPU
        cuda_set_device(gpus[i]);
#endif
        nets[i] = parse_network_cfg(cfgfile);
        if(weightfile){
            load_weights(&nets[i], weightfile);
        }
        if(clear) *nets[i].seen = 0;
        nets[i].learning_rate *= ngpus;
    }
    srand(time(0));
    net = nets[0];

    imgs = net.batch * net.subdivisions * ngpus;

    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    options = read_data_cfg(datacfg);

    backup_directory = option_find_str(options, "backup", "/backup/");
    label_list = option_find_str(options, "labels", "data/labels.list");
    train_list = option_find_str(options, "train", "data/train.list");
    classes = option_find_int(options, "classes", 2);

    labels = get_labels(label_list);
    plist = get_paths(train_list);
    paths = (char **)list_to_array(plist);
    printf("%d\n", plist->size);
    N = plist->size;
   
    args.w = net.w;
    args.h = net.h;
    args.threads = 32;
    args.hierarchy = net.hierarchy;

    args.min = net.min_crop;
    args.max = net.max_crop;
    args.angle = net.angle;
    args.aspect = net.aspect;
    args.exposure = net.exposure;
    args.saturation = net.saturation;
    args.hue = net.hue;
    args.size = net.w;

    args.paths = paths;
    args.classes = classes;
    args.n = imgs;
    args.m = N;
    args.labels = labels;
    args.type = CLASSIFICATION_DATA;
  
    args.d = &buffer;
    load_thread = load_data(args);

    epoch = (*net.seen)/N;
    while(get_current_batch(net) < net.max_batches || net.max_batches == 0){
        time1=clock();

        pthread_join(load_thread, 0);
        train = buffer;
        load_thread = load_data(args);

        printf("Loaded: %lf seconds\n", sec(clock()-time1));
        time1=clock();

        loss = 0;
#ifdef GPU
        if(ngpus == 1){
            loss = train_network(net, train);
        } else {
            loss = train_networks(nets, ngpus, train, 4);
        }
#else
        loss = train_network(net, train);
#endif
        if(avg_loss == -1) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;
        printf("%d, %.3f: %f, %f avg, %f rate, %lf seconds, %d images\n", get_current_batch(net), (float)(*net.seen)/N, loss, avg_loss, get_current_rate(net), sec(clock()-time1), *net.seen);
        free_data(train);
        if(*net.seen/N > epoch){
            epoch = *net.seen/N;
            
            sprintf(buff, "%s/%s_%d.weights",backup_directory,base, epoch);
            save_weights(net, buff);
        }
        if(get_current_batch(net)%100 == 0){
           
            sprintf(buff, "%s/%s.backup",backup_directory,base);
            save_weights(net, buff);
        }
    }
    
    sprintf(buff, "%s/%s.weights", backup_directory, base);
    save_weights(net, buff);

    free_network(net);
    free_ptrs((void**)labels, classes);
    free_ptrs((void**)paths, plist->size);
    free_list(plist);
    free(base);
}


/*
   void train_classifier(char *datacfg, char *cfgfile, char *weightfile, int clear)
   {
   srand(time(0));
   float avg_loss = -1;
   char *base = basecfg(cfgfile);
   printf("%s\n", base);
   network net = parse_network_cfg(cfgfile);
   if(weightfile){
   load_weights(&net, weightfile);
   }
   if(clear) *net.seen = 0;

   int imgs = net.batch * net.subdivisions;

   printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
   list *options = read_data_cfg(datacfg);

   char *backup_directory = option_find_str(options, "backup", "/backup/");
   char *label_list = option_find_str(options, "labels", "data/labels.list");
   char *train_list = option_find_str(options, "train", "data/train.list");
   int classes = option_find_int(options, "classes", 2);

   char **labels = get_labels(label_list);
   list *plist = get_paths(train_list);
   char **paths = (char **)list_to_array(plist);
   printf("%d\n", plist->size);
   int N = plist->size;
   clock_t time;

   load_args args = {0};
   args.w = net.w;
   args.h = net.h;
   args.threads = 8;

   args.min = net.min_crop;
   args.max = net.max_crop;
   args.angle = net.angle;
   args.aspect = net.aspect;
   args.exposure = net.exposure;
   args.saturation = net.saturation;
   args.hue = net.hue;
   args.size = net.w;
   args.hierarchy = net.hierarchy;

   args.paths = paths;
   args.classes = classes;
   args.n = imgs;
   args.m = N;
   args.labels = labels;
   args.type = CLASSIFICATION_DATA;

   data train;
   data buffer;
   pthread_t load_thread;
   args.d = &buffer;
   load_thread = load_data(args);

   int epoch = (*net.seen)/N;
   while(get_current_batch(net) < net.max_batches || net.max_batches == 0){
   time=clock();

   pthread_join(load_thread, 0);
   train = buffer;
   load_thread = load_data(args);

   printf("Loaded: %lf seconds\n", sec(clock()-time));
   time=clock();

#ifdef OPENCV
if(0){
int u;
for(u = 0; u < imgs; ++u){
    image im = float_to_image(net.w, net.h, 3, train.X.vals[u]);
    show_image(im, "loaded");
    cvWaitKey(0);
}
}
#endif

float loss = train_network(net, train);
free_data(train);

if(avg_loss == -1) avg_loss = loss;
avg_loss = avg_loss*.9 + loss*.1;
printf("%d, %.3f: %f, %f avg, %f rate, %lf seconds, %d images\n", get_current_batch(net), (float)(*net.seen)/N, loss, avg_loss, get_current_rate(net), sec(clock()-time), *net.seen);
if(*net.seen/N > epoch){
    epoch = *net.seen/N;
    char buff[256];
    sprintf(buff, "%s/%s_%d.weights",backup_directory,base, epoch);
    save_weights(net, buff);
}
if(get_current_batch(net)%100 == 0){
    char buff[256];
    sprintf(buff, "%s/%s.backup",backup_directory,base);
    save_weights(net, buff);
}
}
char buff[256];
sprintf(buff, "%s/%s.weights", backup_directory, base);
save_weights(net, buff);

free_network(net);
free_ptrs((void**)labels, classes);
free_ptrs((void**)paths, plist->size);
free_list(plist);
free(base);
}
*/

void validate_classifier_crop(char *datacfg, char *filename, char *weightfile)
{
    int i = 0,classes,topk,m;
	list *options;
	char *label_list;
	char *valid_list;
	char **labels;
	list *plist;
	char **paths;
	clock_t time1;
	float avg_acc;
	float avg_topk;
	int splits,num;
	data val, buffer;
	pthread_t load_thread;
	float *acc;
	char **part;
	load_args args = {0};
    network net = parse_network_cfg(filename);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "labels", "data/labels.list");
    valid_list = option_find_str(options, "valid", "data/train.list");
    classes = option_find_int(options, "classes", 2);
    topk = option_find_int(options, "top", 1);

    labels = get_labels(label_list);
    plist = get_paths(valid_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    
    avg_acc = 0;
    avg_topk = 0;
    splits = m/1000;
    num = (i+1)*m/splits - i*m/splits;

    

    
    args.w = net.w;
    args.h = net.h;

    args.paths = paths;
    args.classes = classes;
    args.n = num;
    args.m = 0;
    args.labels = labels;
    args.d = &buffer;
    args.type = OLD_CLASSIFICATION_DATA;

    load_thread = load_data_in_thread(args);
    for(i = 1; i <= splits; ++i){
        time1=clock();

        pthread_join(load_thread, 0);
        val = buffer;

        num = (i+1)*m/splits - i*m/splits;
        part = paths+(i*m/splits);
        if(i != splits){
            args.paths = part;
            load_thread = load_data_in_thread(args);
        }
        printf("Loaded: %d images in %lf seconds\n", val.X.rows, sec(clock()-time1));

        time1=clock();
        acc = network_accuracies(net, val, topk);
        avg_acc += acc[0];
        avg_topk += acc[1];
        printf("%d: top 1: %f, top %d: %f, %lf seconds, %d images\n", i, avg_acc/i, topk, avg_topk/i, sec(clock()-time1), val.X.rows);
        free_data(val);
    }
}

void validate_classifier_10(char *datacfg, char *filename, char *weightfile)
{
    int i, j,classes,topk,m,w,h,shift;
	char *label_list;
	char *valid_list;
	char **labels ;
	list *plist;
	list *options;
	char **paths;
	float *pred;
	float *p;
	float avg_acc,avg_topk;
	image im;
	image images[10];
	int *indexes;
    network net = parse_network_cfg(filename);
    set_batch_network(&net, 1);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "labels", "data/labels.list");
    valid_list = option_find_str(options, "valid", "data/train.list");
    classes = option_find_int(options, "classes", 2);
    topk = option_find_int(options, "top", 1);

    labels = get_labels(label_list);
    plist = get_paths(valid_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    avg_acc = 0;
    avg_topk = 0;
    indexes = (int *)calloc(topk, sizeof(int));

    for(i = 0; i < m; ++i){
        int class = -1;
        char *path = paths[i];
        for(j = 0; j < classes; ++j){
            if(strstr(path, labels[j])){
                class = j;
                break;
            }
        }
        w = net.w;
        h = net.h;
        shift = 32;
        im = load_image_color(paths[i], w+shift, h+shift);
        
        images[0] = crop_image(im, -shift, -shift, w, h);
        images[1] = crop_image(im, shift, -shift, w, h);
        images[2] = crop_image(im, 0, 0, w, h);
        images[3] = crop_image(im, -shift, shift, w, h);
        images[4] = crop_image(im, shift, shift, w, h);
        flip_image(im);
        images[5] = crop_image(im, -shift, -shift, w, h);
        images[6] = crop_image(im, shift, -shift, w, h);
        images[7] = crop_image(im, 0, 0, w, h);
        images[8] = crop_image(im, -shift, shift, w, h);
        images[9] = crop_image(im, shift, shift, w, h);
        pred = (float*)calloc(classes, sizeof(float));
        for(j = 0; j < 10; ++j){
            p = network_predict(net, images[j].data);
            if(net.hierarchy) hierarchy_predictions(p, net.outputs, net.hierarchy, 1);
            axpy_cpu(classes, 1, p, 1, pred, 1);
            free_image(images[j]);
        }
        free_image(im);
        top_k(pred, classes, topk, indexes);
        free(pred);
        if(indexes[0] == class) avg_acc += 1;
        for(j = 0; j < topk; ++j){
            if(indexes[j] == class) avg_topk += 1;
        }

        printf("%d: top 1: %f, top %d: %f\n", i, avg_acc/(i+1), topk, avg_topk/(i+1));
    }
}

void validate_classifier_full(char *datacfg, char *filename, char *weightfile)
{
    int i, j,classes,topk,m,size;
	char *label_list;
	char *valid_list;
	char **labels;
	list *options;
	char **paths;
	list *plist;
	float avg_acc,avg_topk;
	int *indexes;
	float *pred;
	image im;
	image resized;
    network net = parse_network_cfg(filename);
    set_batch_network(&net, 1);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "labels", "data/labels.list");
    valid_list = option_find_str(options, "valid", "data/train.list");
    classes = option_find_int(options, "classes", 2);
    topk = option_find_int(options, "top", 1);

    labels = get_labels(label_list);
    plist = get_paths(valid_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    avg_acc = 0;
    avg_topk = 0;
    indexes = (int *)calloc(topk, sizeof(int));

    size = net.w;
    for(i = 0; i < m; ++i){
        int class = -1;
        char *path = paths[i];
        for(j = 0; j < classes; ++j){
            if(strstr(path, labels[j])){
                class = j;
                break;
            }
        }
        im = load_image_color(paths[i], 0, 0);
        resized = resize_min(im, size);
        resize_network(&net, resized.w, resized.h);
        //show_image(im, "orig");
        //show_image(crop, "cropped");
        //cvWaitKey(0);
        pred = network_predict(net, resized.data);
        if(net.hierarchy) hierarchy_predictions(pred, net.outputs, net.hierarchy, 1);

        free_image(im);
        free_image(resized);
        top_k(pred, classes, topk, indexes);

        if(indexes[0] == class) avg_acc += 1;
        for(j = 0; j < topk; ++j){
            if(indexes[j] == class) avg_topk += 1;
        }

        printf("%d: top 1: %f, top %d: %f\n", i, avg_acc/(i+1), topk, avg_topk/(i+1));
    }
}


void validate_classifier_single(char *datacfg, char *filename, char *weightfile)
{
	int i, j,classes,topk,m,size;
	char *label_list;
	char *leaf_list;
	char **labels;
	char *valid_list;
	list *options;
	char **paths;
	list *plist;

	float avg_acc,avg_topk;
	int *indexes;
	float *pred;
	image im;
	image resized,crop;
    
    network net = parse_network_cfg(filename);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "labels", "data/labels.list");
    leaf_list = option_find_str(options, "leaves", 0);
    if(leaf_list) change_leaves(net.hierarchy, leaf_list);
    valid_list = option_find_str(options, "valid", "data/train.list");
    classes = option_find_int(options, "classes", 2);
    topk = option_find_int(options, "top", 1);

    labels = get_labels(label_list);
    plist = get_paths(valid_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    avg_acc = 0;
    avg_topk = 0;
    indexes = (int*)calloc(topk, sizeof(int));

    for(i = 0; i < m; ++i){
        int class = -1;
        char *path = paths[i];
        for(j = 0; j < classes; ++j){
            if(strstr(path, labels[j])){
                class = j;
                break;
            }
        }
        im = load_image_color(paths[i], 0, 0);
        resized = resize_min(im, net.w);
        crop = crop_image(resized, (resized.w - net.w)/2, (resized.h - net.h)/2, net.w, net.h);
        //show_image(im, "orig");
        //show_image(crop, "cropped");
        //cvWaitKey(0);
        pred = network_predict(net, crop.data);
        if(net.hierarchy) hierarchy_predictions(pred, net.outputs, net.hierarchy, 1);

        if(resized.data != im.data) free_image(resized);
        free_image(im);
        free_image(crop);
        top_k(pred, classes, topk, indexes);

        if(indexes[0] == class) avg_acc += 1;
        for(j = 0; j < topk; ++j){
            if(indexes[j] == class) avg_topk += 1;
        }

        printf("%d: top 1: %f, top %d: %f\n", i, avg_acc/(i+1), topk, avg_topk/(i+1));
    }
}

void validate_classifier_multi(char *datacfg, char *filename, char *weightfile)
{
	int scales[] = {224, 288, 320, 352, 384};
    int i, j,classes,topk,m,size,nscales;
	char *label_list;
	char *leaf_list;
	char **labels;
	char *valid_list;
	list *options;
	char **paths;
	list *plist;

	float avg_acc,avg_topk;
	int *indexes;
	float *pred;
	float *p;
	image im,r;
	image resized;
	network net;

    net = parse_network_cfg(filename);
    set_batch_network(&net, 1);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "labels", "data/labels.list");
    valid_list = option_find_str(options, "valid", "data/train.list");
    classes = option_find_int(options, "classes", 2);
    topk = option_find_int(options, "top", 1);

    labels = get_labels(label_list);
    plist = get_paths(valid_list);
    
    nscales = sizeof(scales)/sizeof(scales[0]);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    avg_acc = 0;
    avg_topk = 0;
    indexes = (int*)calloc(topk, sizeof(int));

    for(i = 0; i < m; ++i){
        int class = -1;
        char *path = paths[i];
        for(j = 0; j < classes; ++j){
            if(strstr(path, labels[j])){
                class = j;
                break;
            }
        }
        pred = (float*)calloc(classes, sizeof(float));
        im = load_image_color(paths[i], 0, 0);
        for(j = 0; j < nscales; ++j){
            r = resize_min(im, scales[j]);
            resize_network(&net, r.w, r.h);
            p = network_predict(net, r.data);
            if(net.hierarchy) hierarchy_predictions(p, net.outputs, net.hierarchy, 1);
            axpy_cpu(classes, 1, p, 1, pred, 1);
            flip_image(r);
            p = network_predict(net, r.data);
            axpy_cpu(classes, 1, p, 1, pred, 1);
            if(r.data != im.data) free_image(r);
        }
        free_image(im);
        top_k(pred, classes, topk, indexes);
        free(pred);
        if(indexes[0] == class) avg_acc += 1;
        for(j = 0; j < topk; ++j){
            if(indexes[j] == class) avg_topk += 1;
        }

        printf("%d: top 1: %f, top %d: %f\n", i, avg_acc/(i+1), topk, avg_topk/(i+1));
    }
}

void try_classifier(char *datacfg, char *cfgfile, char *weightfile, char *filename, int layer_num)
{
	int i, j,classes,topk,m,size,top;
	char *name_list;
	char *leaf_list;
	char **labels;
	char *valid_list;
	list *options;
	
	list *plist;
	char **names;
	float avg_acc,avg_topk;
	int *indexes;
	float *predictions;
	float *X;
	image im;
	image orig,r;
	clock_t time;
	char buff[256];
    char *input ;
	layer l;
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(2222222);

    options = read_data_cfg(datacfg);

    name_list = option_find_str(options, "names", 0);
    if(!name_list) name_list = option_find_str(options, "labels", "data/labels.list");
    top = option_find_int(options, "top", 1);

    i = 0;
    names = get_labels(name_list);
    
    indexes =(int*)calloc(top, sizeof(int));
    input = buff;
    while(1){
		float mean[] = {0.48263312050943, 0.45230225481413, 0.40099074308742};
        float std[] = {0.22590347483426, 0.22120921437787, 0.22103996251583};
        float var[3];
        if(filename){
            strncpy(input, filename, 256);
        }else{
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if(!input) return;
            strtok(input, "\n");
        }
        orig = load_image_color(input, 0, 0);
        r = resize_min(orig, 256);
        im = crop_image(r, (r.w - 224 - 1)/2 + 1, (r.h - 224 - 1)/2 + 1, 224, 224);
       
        var[0] = std[0]*std[0];
        var[1] = std[1]*std[1];
        var[2] = std[2]*std[2];

        normalize_cpu(im.data, mean, var, 1, 3, im.w*im.h);
	
        X = im.data;
        time=clock();
        predictions = network_predict(net, X);

        l = net.layers[layer_num];
        for(i = 0; i < l.c; ++i){
            if(l.rolling_mean) printf("%f %f %f\n", l.rolling_mean[i], l.rolling_variance[i], l.scales[i]);
        }
#ifdef GPU
        cuda_pull_array(l.output_gpu, l.output, l.outputs);
#endif
        for(i = 0; i < l.outputs; ++i){
            printf("%f\n", l.output[i]);
        }
        /*

           printf("\n\nWeights\n");
           for(i = 0; i < l.n*l.size*l.size*l.c; ++i){
           printf("%f\n", l.filters[i]);
           }

           printf("\n\nBiases\n");
           for(i = 0; i < l.n; ++i){
           printf("%f\n", l.biases[i]);
           }
         */

        top_predictions(net, top, indexes);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        for(i = 0; i < top; ++i){
            int index = indexes[i];
            printf("%s: %f\n", names[index], predictions[index]);
        }
        free_image(im);
        if (filename) break;
    }
}

void predict_classifier(char *datacfg, char *cfgfile, char *weightfile, char *filename, int top)
{
	int i, j,classes,topk,m,size;
	char *name_list;
	char *leaf_list;
	char **labels;
	char *valid_list;
	list *options;
	
	list *plist;
	char **names;
	float avg_acc,avg_topk;
	int *indexes;
	float *predictions;
	float *X;
	image im;
	image orig,r;
	clock_t time;
	char buff[256];
    char *input ;

    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(2222222);

    options = read_data_cfg(datacfg);

    name_list = option_find_str(options, "names", 0);
    if(!name_list) name_list = option_find_str(options, "labels", "data/labels.list");
    if(top == 0) top = option_find_int(options, "top", 1);

    i = 0;
    names = get_labels(name_list);

    indexes = (int*)calloc(top, sizeof(int)); 
    input = buff;
    size = net.w;
    while(1){
        if(filename){
            strncpy(input, filename, 256);
        }else{
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if(!input) return;
            strtok(input, "\n");
        }
        im = load_image_color(input, 0, 0);
	    r = letterbox_image(im, net.w, net.h);
        //r = resize_min(im, size);
        //resize_network(&net, r.w, r.h);
        printf("%d %d\n", r.w, r.h);

        X = r.data;
        time=clock();
        predictions = network_predict(net, X);
        if(net.hierarchy) hierarchy_predictions(predictions, net.outputs, net.hierarchy, 0);
        top_k(predictions, net.outputs, top, indexes);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        for(i = 0; i < top; ++i){
            int index = indexes[i];
            if(net.hierarchy) printf("%d, %s: %f, parent: %s \n",index, names[index], predictions[index], (net.hierarchy->parent[index] >= 0) ? names[net.hierarchy->parent[index]] : "Root");
            else printf("%s: %f\n",names[index], predictions[index]);
        }
        if(r.data != im.data) free_image(r);
        free_image(im);
        if (filename) break;
    }
}


void label_classifier(char *datacfg, char *filename, char *weightfile)
{
	int i, j,classes,topk,m,size,ind;
	char *label_list;
	char *test_list;
	char **labels;
	char *valid_list;
	list *options;
	
	list *plist;
	char **paths;
	float avg_acc,avg_topk;
	int *indexes;
	float *pred;
	float *X;
	image im;
	image resized,crop;
	clock_t time1;
	char buff[256];
    char *input ;

    network net = parse_network_cfg(filename);
    set_batch_network(&net, 1);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    label_list = option_find_str(options, "names", "data/labels.list");
    test_list = option_find_str(options, "test", "data/train.list");
    classes = option_find_int(options, "classes", 2);

    labels = get_labels(label_list);
    plist = get_paths(test_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    for(i = 0; i < m; ++i){
        im = load_image_color(paths[i], 0, 0);
        resized = resize_min(im, net.w);
        crop = crop_image(resized, (resized.w - net.w)/2, (resized.h - net.h)/2, net.w, net.h);
        pred = network_predict(net, crop.data);

        if(resized.data != im.data) free_image(resized);
        free_image(im);
        free_image(crop);
        ind = max_index(pred, classes);

        printf("%s\n", labels[ind]);
    }
}


void test_classifier(char *datacfg, char *cfgfile, char *weightfile, int target_layer)
{
	int i, j,classes,topk,m,size,ind;
	char *label_list;
	char *test_list;
	char **labels;
	char *valid_list;
	list *options;
	
	list *plist;
	char **paths;
	float avg_acc,avg_topk;
	int *indexes;
	matrix pred;
	image im;
	image resized,crop;
	clock_t time1;
	char buff[256];
    char *input ;
	load_args args = {0};
    data val, buffer;
	pthread_t load_thread;
    int curr = 0;
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    options = read_data_cfg(datacfg);

    test_list = option_find_str(options, "test", "data/test.list");
    classes = option_find_int(options, "classes", 2);

    plist = get_paths(test_list);

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    


    
    args.w = net.w;
    args.h = net.h;
    args.paths = paths;
    args.classes = classes;
    args.n = net.batch;
    args.m = 0;
    args.labels = 0;
    args.d = &buffer;
    args.type = OLD_CLASSIFICATION_DATA;

    load_thread = load_data_in_thread(args);
    for(curr = net.batch; curr < m; curr += net.batch){
        time1=clock();

        pthread_join(load_thread, 0);
        val = buffer;

        if(curr < m){
            args.paths = paths + curr;
            if (curr + net.batch > m) args.n = m - curr;
            load_thread = load_data_in_thread(args);
        }
        fprintf(stderr, "Loaded: %d images in %lf seconds\n", val.X.rows, sec(clock()-time1));

        time1=clock();
        pred = network_predict_data(net, val);

       
        if (target_layer >= 0){
            //layer l = net.layers[target_layer];
        }

        for(i = 0; i < pred.rows; ++i){
            printf("%s", paths[curr-net.batch+i]);
            for(j = 0; j < pred.cols; ++j){
                printf("\t%g", pred.vals[i][j]);
            }
            printf("\n");
        }

        free_matrix(pred);

        fprintf(stderr, "%lf seconds, %d images, %d total\n", sec(clock()-time1), val.X.rows, curr);
        free_data(val);
    }
}


void threat_classifier(char *datacfg, char *cfgfile, char *weightfile, int cam_index, const char *filename)
{
#ifdef OPENCV
    float threat = 0;
	float curr_threat,ratio,r,g;
    float roll = .2,fps,curr;
	int x1,y1,x2,y2,border,count,h,w,i,j,index;
	network net;
	list *options;
	CvCapture *cap ;
	int top;
	char *name_list;
	char **names;
	int *indexes;
	float *predictions;
	char buff[256];
	image in,in_s,out;
    printf("Classifier Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    options = read_data_cfg(datacfg);

    srand(2222222);


    if(filename){
        cap = cvCaptureFromFile(filename);
    }else{
        cap = cvCaptureFromCAM(cam_index);
    }

    top = option_find_int(options, "top", 1);

    name_list = option_find_str(options, "names", 0);
    names = get_labels(name_list);

    indexes = (int*)calloc(top, sizeof(int));

    if(!cap) error("Couldn't connect to webcam.\n");
    //cvNamedWindow("Threat", CV_WINDOW_NORMAL); 
    //cvResizeWindow("Threat", 512, 512);
    fps = 0;
   

    count = 0;

    while(1){
		struct timeval tval_before, tval_after, tval_result;
        ++count;
        gettimeofday(&tval_before, NULL);

        in = get_image_from_stream(cap);
        if(!in.data) break;
        in_s = resize_image(in, net.w, net.h);

        out = in;
        x1 = out.w / 20;
        y1 = out.h / 20;
        x2 = 2*x1;
        y2 = out.h - out.h/20;

        border = .01*out.h;
        h = y2 - y1 - 2*border;
        w = x2 - x1 - 2*border;

        predictions = network_predict(net, in_s.data);
        curr_threat = 0;
        if(1){
            curr_threat = predictions[0] * 0 + 
                predictions[1] * .6 + 
                predictions[2];
        } else {
            curr_threat = predictions[218] +
                predictions[539] + 
                predictions[540] + 
                predictions[368] + 
                predictions[369] + 
                predictions[370];
        }
        threat = roll * curr_threat + (1-roll) * threat;

        draw_box_width(out, x2 + border, y1 + .02*h, x2 + .5 * w, y1 + .02*h + border, border, 0,0,0);
        if(threat > .97) {
            draw_box_width(out,  x2 + .5 * w + border,
                    y1 + .02*h - 2*border, 
                    x2 + .5 * w + 6*border, 
                    y1 + .02*h + 3*border, 3*border, 1,0,0);
        }
        draw_box_width(out,  x2 + .5 * w + border,
                y1 + .02*h - 2*border, 
                x2 + .5 * w + 6*border, 
                y1 + .02*h + 3*border, .5*border, 0,0,0);
        draw_box_width(out, x2 + border, y1 + .42*h, x2 + .5 * w, y1 + .42*h + border, border, 0,0,0);
        if(threat > .57) {
            draw_box_width(out,  x2 + .5 * w + border,
                    y1 + .42*h - 2*border, 
                    x2 + .5 * w + 6*border, 
                    y1 + .42*h + 3*border, 3*border, 1,1,0);
        }
        draw_box_width(out,  x2 + .5 * w + border,
                y1 + .42*h - 2*border, 
                x2 + .5 * w + 6*border, 
                y1 + .42*h + 3*border, .5*border, 0,0,0);

        draw_box_width(out, x1, y1, x2, y2, border, 0,0,0);
        for(i = 0; i < threat * h ; ++i){
            ratio = (float) i / h;
            r = (ratio < .5) ? (2*(ratio)) : 1;
            g = (ratio < .5) ? 1 : 1 - 2*(ratio - .5);
            draw_box_width(out, x1 + border, y2 - border - i, x2 - border, y2 - border - i, 1, r, g, 0);
        }
        top_predictions(net, top, indexes);
        
        sprintf(buff, "/home/pjreddie/tmp/threat_%06d", count);
        //save_image(out, buff);

        printf("\033[2J");
        printf("\033[1;1H");
        printf("\nFPS:%.0f\n",fps);

        for(i = 0; i < top; ++i){
            index = indexes[i];
            printf("%.1f%%: %s\n", predictions[index]*100, names[index]);
        }

        if(1){
            show_image(out, "Threat");
            cvWaitKey(10);
        }
        free_image(in_s);
        free_image(in);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        curr = 1000000.f/((long int)tval_result.tv_usec);
        fps = .9*fps + .1*curr;
    }
#endif
}


void gun_classifier(char *datacfg, char *cfgfile, char *weightfile, int cam_index, const char *filename)
{
	network net;
	list *options;
	
	int top,i,threat;
	char *name_list;
	char **names;
	int *indexes;
	float fps;
	image in ;
	image in_s;
	float curr;
  
#ifdef OPENCV
	CvCapture * cap;
    int bad_cats[] = {218, 539, 540, 1213, 1501, 1742, 1911, 2415, 4348, 19223, 368, 369, 370, 1133, 1200, 1306, 2122, 2301, 2537, 2823, 3179, 3596, 3639, 4489, 5107, 5140, 5289, 6240, 6631, 6762, 7048, 7171, 7969, 7984, 7989, 8824, 8927, 9915, 10270, 10448, 13401, 15205, 18358, 18894, 18895, 19249, 19697};

    printf("Classifier Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    options = read_data_cfg(datacfg);

    srand(2222222);
   

    if(filename){
        cap = cvCaptureFromFile(filename);
    }else{
        cap = cvCaptureFromCAM(cam_index);
    }

    top = option_find_int(options, "top", 1);

    name_list = option_find_str(options, "names", 0);
    names = get_labels(name_list);

    indexes = (int*)calloc(top, sizeof(int));

    if(!cap) error("Couldn't connect to webcam.\n");
    cvNamedWindow("Threat Detection", CV_WINDOW_NORMAL); 
    cvResizeWindow("Threat Detection", 512, 512);
    fps = 0;
 

    while(1){
        struct timeval tval_before, tval_after, tval_result;
		float *predictions ;
        gettimeofday(&tval_before, NULL);

        in = get_image_from_stream(cap);
        in_s = resize_image(in, net.w, net.h);
        show_image(in, "Threat Detection");

        predictions = network_predict(net, in_s.data);
        top_predictions(net, top, indexes);

        printf("\033[2J");
        printf("\033[1;1H");

        threat = 0;
        for(i = 0; i < sizeof(bad_cats)/sizeof(bad_cats[0]); ++i){
            int index = bad_cats[i];
            if(predictions[index] > .01){
                printf("Threat Detected!\n");
                threat = 1;
                break;
            }
        }
        if(!threat) printf("Scanning...\n");
        for(i = 0; i < sizeof(bad_cats)/sizeof(bad_cats[0]); ++i){
            int index = bad_cats[i];
            if(predictions[index] > .01){
                printf("%s\n", names[index]);
            }
        }

        free_image(in_s);
        free_image(in);

        cvWaitKey(10);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        curr = 1000000.f/((long int)tval_result.tv_usec);
        fps = .9*fps + .1*curr;
    }
#endif
}

void demo_classifier(char *datacfg, char *cfgfile, char *weightfile, int cam_index, const char *filename)
{
	network net;
	list *options;
	
	int top,i;
	char *name_list;
	char **names;
	int *indexes;
	float fps;
	image in ;
	image in_s;
	float curr;
#ifdef OPENCV
	CvCapture * cap;
    printf("Classifier Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    options = read_data_cfg(datacfg);

    srand(2222222);
    ;

    if(filename){
        cap = cvCaptureFromFile(filename);
    }else{
        cap = cvCaptureFromCAM(cam_index);
    }

    top = option_find_int(options, "top", 1);

    name_list = option_find_str(options, "names", 0);
    names = get_labels(name_list);

    indexes = (int*)calloc(top, sizeof(int));

    if(!cap) error("Couldn't connect to webcam.\n");
    cvNamedWindow("Classifier", CV_WINDOW_NORMAL); 
    cvResizeWindow("Classifier", 512, 512);
    fps = 0;
   

    while(1){
        struct timeval tval_before, tval_after, tval_result;
		float *predictions ;
        gettimeofday(&tval_before, NULL);

        in = get_image_from_stream(cap);
        in_s = resize_image(in, net.w, net.h);
        show_image(in, "Classifier");

        predictions = network_predict(net, in_s.data);
        if(net.hierarchy) hierarchy_predictions(predictions, net.outputs, net.hierarchy, 1);
        top_predictions(net, top, indexes);

        printf("\033[2J");
        printf("\033[1;1H");
        printf("\nFPS:%.0f\n",fps);

        for(i = 0; i < top; ++i){
            int index = indexes[i];
            printf("%.1f%%: %s\n", predictions[index]*100, names[index]);
        }

        free_image(in_s);
        free_image(in);

        cvWaitKey(10);

        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        curr = 1000000.f/((long int)tval_result.tv_usec);
        fps = .9*fps + .1*curr;
    }
#endif
}


void run_classifier(int argc, char **argv)
{
	char *gpu_list;
	int *gpus;
	int gpu,ngpus,layer;
	int cam_index,top,clear;
	int i,len;
	char *data,*cfg,*weights,*filename,*layer_s;
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [train/test/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    gpu_list = find_char_arg(argc, argv, "-gpus", 0);
    gpus = 0;
    gpu = 0;
    ngpus = 0;
    if(gpu_list){
        printf("%s\n", gpu_list);
        len = strlen(gpu_list);
        ngpus = 1;
        for(i = 0; i < len; ++i){
            if (gpu_list[i] == ',') ++ngpus;
        }
        gpus = (int*)calloc(ngpus, sizeof(int));
        for(i = 0; i < ngpus; ++i){
            gpus[i] = atoi(gpu_list);
            gpu_list = strchr(gpu_list, ',')+1;
        }
    } else {
        gpu = gpu_index;
        gpus = &gpu;
        ngpus = 1;
    }

    cam_index = find_int_arg(argc, argv, "-c", 0);
    top = find_int_arg(argc, argv, "-t", 0);
    clear = find_arg(argc, argv, "-clear");
    data = argv[3];
    cfg = argv[4];
    weights = (argc > 5) ? argv[5] : 0;
    filename = (argc > 6) ? argv[6]: 0;
    layer_s = (argc > 7) ? argv[7]: 0;
    layer = layer_s ? atoi(layer_s) : -1;
    if(0==strcmp(argv[2], "predict")) predict_classifier(data, cfg, weights, filename, top);
    else if(0==strcmp(argv[2], "try")) try_classifier(data, cfg, weights, filename, atoi(layer_s));
    else if(0==strcmp(argv[2], "train")) train_classifier(data, cfg, weights, gpus, ngpus, clear);
    else if(0==strcmp(argv[2], "demo")) demo_classifier(data, cfg, weights, cam_index, filename);
    else if(0==strcmp(argv[2], "gun")) gun_classifier(data, cfg, weights, cam_index, filename);
    else if(0==strcmp(argv[2], "threat")) threat_classifier(data, cfg, weights, cam_index, filename);
    else if(0==strcmp(argv[2], "test")) test_classifier(data, cfg, weights, layer);
    else if(0==strcmp(argv[2], "label")) label_classifier(data, cfg, weights);
    else if(0==strcmp(argv[2], "valid")) validate_classifier_single(data, cfg, weights);
    else if(0==strcmp(argv[2], "validmulti")) validate_classifier_multi(data, cfg, weights);
    else if(0==strcmp(argv[2], "valid10")) validate_classifier_10(data, cfg, weights);
    else if(0==strcmp(argv[2], "validcrop")) validate_classifier_crop(data, cfg, weights);
    else if(0==strcmp(argv[2], "validfull")) validate_classifier_full(data, cfg, weights);
}


