#include <stdio.h>

#include "network.h"
#include "detection_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "demo.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif

char *coco_classes[] = {"person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow","elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket","bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple","sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair","couch","potted plant","bed","dining table","toilet","tv","laptop","mouse","remote","keyboard","cell phone","microwave","oven","toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear","hair drier","toothbrush"};

int coco_ids[] = {1,2,3,4,5,6,7,8,9,10,11,13,14,15,16,17,18,19,20,21,22,23,24,25,27,28,31,32,33,34,35,36,37,38,39,40,41,42,43,44,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,67,70,72,73,74,75,76,77,78,79,80,81,82,84,85,86,87,88,89,90};

void train_coco(char *cfgfile, char *weightfile)
{
    //char *train_images = "/home/pjreddie/data/voc/test/train.txt";
    //char *train_images = "/home/pjreddie/data/coco/train.txt";
	char *base;
	float avg_loss;
	network net;
	int imgs,i,side,classes;
	layer l;
	data train, buffer;
	float jitter,loss;
	list *plist;
	char **paths;
	load_args args = {0};
	pthread_t load_thread;
	clock_t time1;
	 char buff[256];
    char *train_images = "data/coco.trainval.txt";
    //char *train_images = "data/bags.train.list";
    char *backup_directory = "/home/pjreddie/backup/";
    srand(time(0));
    base = basecfg(cfgfile);
    printf("%s\n", base);
    avg_loss = -1;
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    imgs = net.batch*net.subdivisions;
    i = *net.seen/imgs;
    


    l = net.layers[net.n - 1];

    side = l.side;
    classes = l.classes;
    jitter = l.jitter;

    plist = get_paths(train_images);
    //int N = plist->size;
    paths = (char **)list_to_array(plist);

    
    args.w = net.w;
    args.h = net.h;
    args.paths = paths;
    args.n = imgs;
    args.m = plist->size;
    args.classes = classes;
    args.jitter = jitter;
    args.num_boxes = side;
    args.d = &buffer;
    args.type = REGION_DATA;

    args.angle = net.angle;
    args.exposure = net.exposure;
    args.saturation = net.saturation;
    args.hue = net.hue;

    load_thread = load_data_in_thread(args);
    
    //while(i*imgs < N*120){
    while(get_current_batch(net) < net.max_batches){
        i += 1;
        time1=clock();
        pthread_join(load_thread, 0);
        train = buffer;
        load_thread = load_data_in_thread(args);

        printf("Loaded: %lf seconds\n", sec(clock()-time1));

        /*
           image im = float_to_image(net.w, net.h, 3, train.X.vals[113]);
           image copy = copy_image(im);
           draw_coco(copy, train.y.vals[113], 7, "truth");
           cvWaitKey(0);
           free_image(copy);
         */

        time1=clock();
        loss = train_network(net, train);
        if (avg_loss < 0) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;

        printf("%d: %f, %f avg, %f rate, %lf seconds, %d images\n", i, loss, avg_loss, get_current_rate(net), sec(clock()-time1), i*imgs);
        if(i%1000==0 || (i < 1000 && i%100 == 0)){
           
            sprintf(buff, "%s/%s_%d.weights", backup_directory, base, i);
            save_weights(net, buff);
        }
        if(i%100==0){
            sprintf(buff, "%s/%s.backup", backup_directory, base);
            save_weights(net, buff);
        }
        free_data(train);
    }
    sprintf(buff, "%s/%s_final.weights", backup_directory, base);
    save_weights(net, buff);
}

void print_cocos(FILE *fp, int image_id, box *boxes, float **probs, int num_boxes, int classes, int w, int h)
{
    int i, j;
	float xmin,ymin,xmax,ymax;
	float bx,by,bw,bh;
    for(i = 0; i < num_boxes; ++i){
        xmin = boxes[i].x - boxes[i].w/2.;
        xmax = boxes[i].x + boxes[i].w/2.;
        ymin = boxes[i].y - boxes[i].h/2.;
        ymax = boxes[i].y + boxes[i].h/2.;

        if (xmin < 0) xmin = 0;
        if (ymin < 0) ymin = 0;
        if (xmax > w) xmax = w;
        if (ymax > h) ymax = h;

        bx = xmin;
        by = ymin;
        bw = xmax - xmin;
        bh = ymax - ymin;

        for(j = 0; j < classes; ++j){
            if (probs[i][j]) fprintf(fp, "{\"image_id\":%d, \"category_id\":%d, \"bbox\":[%f, %f, %f, %f], \"score\":%f},\n", image_id, coco_ids[j], bx, by, bw, bh, probs[i][j]);
        }
    }
}

int get_coco_image_id(char *filename)
{
    char *p = strrchr(filename, '_');
    return atoi(p+1);
}

void validate_coco(char *cfgfile, char *weightfile)
{
	network net;
	char *base;
	list *plist;
	char **paths;
	layer l;
	int classes,side,i,j,m,t,nms,nthreads,w,h,image_id;
	char *path ;
	box *boxes;
	float **probs;
	char buff[1024];
	float thresh,iou_thresh;
	time_t start;
	image *val,*val_resized,*buf_resized,*buf;
	pthread_t *thr;
	float *X ;
	FILE *fp;
	load_args args = {0};
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    fprintf(stderr, "Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    srand(time(0));

    base = "results/";
    plist = get_paths("data/coco_val_5k.list");
    //list *plist = get_paths("/home/pjreddie/data/people-art/test.txt");
    //list *plist = get_paths("/home/pjreddie/data/voc/test/2007_test.txt");
    paths = (char **)list_to_array(plist);

    l = net.layers[net.n-1];
    classes = l.classes;
    side = l.side;
   
    snprintf(buff, 1024, "%s/coco_results.json", base);
    fp = fopen(buff, "w");
    fprintf(fp, "[\n");

    boxes = (box *)calloc(side*side*l.n, sizeof(box));
    probs = (float **)calloc(side*side*l.n, sizeof(float *));
    for(j = 0; j < side*side*l.n; ++j) probs[j] = (float *)calloc(classes, sizeof(float *));

    m = plist->size;
    i=0;

    thresh = .01;
    nms = 1;
    iou_thresh = .5;

    nthreads = 8;
    val = (image *)calloc(nthreads, sizeof(image));
    val_resized = (image *)calloc(nthreads, sizeof(image));
    buf = (image *)calloc(nthreads, sizeof(image));
    buf_resized = (image *)calloc(nthreads, sizeof(image));
    thr = (pthread_t *)calloc(nthreads, sizeof(pthread_t));

    
    args.w = net.w;
    args.h = net.h;
    args.type = IMAGE_DATA;

    for(t = 0; t < nthreads; ++t){
        args.path = paths[i+t];
        args.im = &buf[t];
        args.resized = &buf_resized[t];
        thr[t] = load_data_in_thread(args);
    }
    start = time(0);
    for(i = nthreads; i < m+nthreads; i += nthreads){
        fprintf(stderr, "%d\n", i);
        for(t = 0; t < nthreads && i+t-nthreads < m; ++t){
            pthread_join(thr[t], 0);
            val[t] = buf[t];
            val_resized[t] = buf_resized[t];
        }
        for(t = 0; t < nthreads && i+t < m; ++t){
            args.path = paths[i+t];
            args.im = &buf[t];
            args.resized = &buf_resized[t];
            thr[t] = load_data_in_thread(args);
        }
        for(t = 0; t < nthreads && i+t-nthreads < m; ++t){
            path = paths[i+t-nthreads];
            image_id = get_coco_image_id(path);
            X = val_resized[t].data;
            network_predict(net, X);
            w = val[t].w;
            h = val[t].h;
            get_detection_boxes(l, w, h, thresh, probs, boxes, 0);
            if (nms) do_nms_sort(boxes, probs, side*side*l.n, classes, iou_thresh);
            print_cocos(fp, image_id, boxes, probs, side*side*l.n, classes, w, h);
            free_image(val[t]);
            free_image(val_resized[t]);
        }
    }
    fseek(fp, -2, SEEK_CUR); 
    fprintf(fp, "\n]\n");
    fclose(fp);

    fprintf(stderr, "Total Detection Time: %f Seconds\n", (double)(time(0) - start));
}

void validate_coco_recall(char *cfgfile, char *weightfile)
{
	box *boxes;
	float **probs;
	FILE **fps;
	int i,j, k,m,classes,side,nms,total;
	int correct,proposals,num_labels;
	layer l;
	char *base;
	list *plist;
	char **paths;
	char *path,*id;
	float thresh,iou_thresh,nms_thresh,avg_iou;
	image orig,sized;
    network net = parse_network_cfg(cfgfile);
	box_label *truth;
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    fprintf(stderr, "Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    srand(time(0));

    base = "results/comp4_det_test_";
    plist = get_paths("/home/pjreddie/data/voc/test/2007_test.txt");
    paths = (char **)list_to_array(plist);

    l = net.layers[net.n-1];
    classes = l.classes;
    side = l.side;

   
    fps = (FILE **)calloc(classes, sizeof(FILE *));
    for(j = 0; j < classes; ++j){
        char buff[1024];
        snprintf(buff, 1024, "%s%s.txt", base, coco_classes[j]);
        fps[j] = fopen(buff, "w");
    }
    boxes = (box *)calloc(side*side*l.n, sizeof(box));
    probs =(float **)calloc(side*side*l.n, sizeof(float *));
    for(j = 0; j < side*side*l.n; ++j) probs[j] = (float *)calloc(classes, sizeof(float *));

    m = plist->size;
    i=0;

    thresh = .001;
    nms = 0;
    iou_thresh = .5;
    nms_thresh = .5;

    total = 0;
    correct = 0;
    proposals = 0;
    avg_iou = 0;

    for(i = 0; i < m; ++i){
		char labelpath[4096];
        path = paths[i];
        orig = load_image_color(path, 0, 0);
        sized = resize_image(orig, net.w, net.h);
        id = basecfg(path);
        network_predict(net, sized.data);
        get_detection_boxes(l, 1, 1, thresh, probs, boxes, 1);
        if (nms) do_nms(boxes, probs, side*side*l.n, 1, nms_thresh);

        find_replace(path, "images", "labels", labelpath);
        find_replace(labelpath, "JPEGImages", "labels", labelpath);
        find_replace(labelpath, ".jpg", ".txt", labelpath);
        find_replace(labelpath, ".JPEG", ".txt", labelpath);

        num_labels = 0;
        truth = read_boxes(labelpath, &num_labels);
        for(k = 0; k < side*side*l.n; ++k){
            if(probs[k][0] > thresh){
                ++proposals;
            }
        }
        for (j = 0; j < num_labels; ++j) {

			box t = {truth[j].x, truth[j].y, truth[j].w, truth[j].h};
			float best_iou = 0;
			float iou;
            ++total;          
            
            for(k = 0; k < side*side*l.n; ++k){
                iou = box_iou(boxes[k], t);
                if(probs[k][0] > thresh && iou > best_iou){
                    best_iou = iou;
                }
            }
            avg_iou += best_iou;
            if(best_iou > iou_thresh){
                ++correct;
            }
        }

        fprintf(stderr, "%5d %5d %5d\tRPs/Img: %.2f\tIOU: %.2f%%\tRecall:%.2f%%\n", i, correct, total, (float)proposals/(i+1), avg_iou*100/total, 100.*correct/total);
        free(id);
        free_image(orig);
        free_image(sized);
    }
}

void test_coco(char *cfgfile, char *weightfile, char *filename, float thresh)
{
	detection_layer l;
	float nms;
	clock_t time;
	char buff[256];
	char *input ;
	box *boxes;
	float **probs;
	int j;
	float *X ;
	image im,sized;
    image **alphabet = load_alphabet();
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    l = net.layers[net.n-1];
    set_batch_network(&net, 1);
    srand(2222222);
    nms = .4;
    
    input = buff;
    boxes = (box *)calloc(l.side*l.side*l.n, sizeof(box));
    probs = (float **)calloc(l.side*l.side*l.n, sizeof(float *));
    for(j = 0; j < l.side*l.side*l.n; ++j) probs[j] = (float *)calloc(l.classes, sizeof(float *));
    while(1){
        if(filename){
            strncpy(input, filename, 256);
        } else {
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if(!input) return;
            strtok(input, "\n");
        }
        im = load_image_color(input,0,0);
        sized = resize_image(im, net.w, net.h);
        X = sized.data;
        time=clock();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        get_detection_boxes(l, 1, 1, thresh, probs, boxes, 0);
        if (nms) do_nms_sort(boxes, probs, l.side*l.side*l.n, l.classes, nms);
        draw_detections(im, l.side*l.side*l.n, thresh, boxes, probs, coco_classes, alphabet, 80);
        save_image(im, "prediction");
        show_image(im, "predictions");
        free_image(im);
        free_image(sized);
#ifdef OPENCV
        cvWaitKey(0);
        cvDestroyAllWindows();
#endif
        if (filename) break;
    }
}

void run_coco(int argc, char **argv)
{
	char *cfg,*weights,*filename;
	char *out_filename = find_char_arg(argc, argv, "-out_filename", 0);
    char *prefix = find_char_arg(argc, argv, "-prefix", 0);
    float thresh = find_float_arg(argc, argv, "-thresh", .2);
    int cam_index = find_int_arg(argc, argv, "-c", 0);
    int frame_skip = find_int_arg(argc, argv, "-s", 0);

    if(argc < 4){
        fprintf(stderr, "usage: %s %s [train/test/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    cfg = argv[3];
    weights = (argc > 4) ? argv[4] : 0;
    filename = (argc > 5) ? argv[5]: 0;
    if(0==strcmp(argv[2], "test")) test_coco(cfg, weights, filename, thresh);
    else if(0==strcmp(argv[2], "train")) train_coco(cfg, weights);
    else if(0==strcmp(argv[2], "valid")) validate_coco(cfg, weights);
    else if(0==strcmp(argv[2], "recall")) validate_coco_recall(cfg, weights);
    else if(0==strcmp(argv[2], "demo")) demo(cfg, weights, thresh, cam_index, filename, coco_classes, 80, frame_skip, prefix, out_filename);
}
