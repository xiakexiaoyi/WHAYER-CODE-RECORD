#include "network.h"
#include "utils.h"
#include "parser.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif

void train_tag(char *cfgfile, char *weightfile, int clear)
{
	float avg_loss = -1;
	char *base;
	char *backup_directory;
	network net;
	int imgs = 1024;
	list *plist;
	char **paths;
	int N;
	clock_t time1;
	pthread_t load_thread;
    data train;
    data buffer;
    load_args args = {0};
	char buff[256];
	int epoch;

    srand(time(0));
    
    base = basecfg(cfgfile);
    backup_directory = "/home/pjreddie/backup/";
    printf("%s\n", base);
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    if(clear) *net.seen = 0;
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    
    plist = get_paths("/home/pjreddie/tag/train.list");
    paths = (char **)list_to_array(plist);
    printf("%d\n", plist->size);
    N = plist->size;
    
    
    args.w = net.w;
    args.h = net.h;

    args.min = net.w;
    args.max = net.max_crop;
    args.size = net.w;

    args.paths = paths;
    args.classes = net.outputs;
    args.n = imgs;
    args.m = N;
    args.d = &buffer;
    args.type = TAG_DATA;

    args.angle = net.angle;
    args.exposure = net.exposure;
    args.saturation = net.saturation;
    args.hue = net.hue;

    fprintf(stderr, "%d classes\n", net.outputs);

    load_thread = load_data_in_thread(args);
    epoch = (*net.seen)/N;
    while(get_current_batch(net) < net.max_batches || net.max_batches == 0){
		float loss;
        time1=clock();
        pthread_join(load_thread, 0);
        train = buffer;

        load_thread = load_data_in_thread(args);
        printf("Loaded: %lf seconds\n", sec(clock()-time1));
        time1=clock();
        loss = train_network(net, train);
        if(avg_loss == -1) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;
        printf("%d, %.3f: %f, %f avg, %f rate, %lf seconds, %d images\n", get_current_batch(net), (float)(*net.seen)/N, loss, avg_loss, get_current_rate(net), sec(clock()-time1), *net.seen);
        free_data(train);
        if(*net.seen/N > epoch){
			char buff[256];
            epoch = *net.seen/N;
            
            sprintf(buff, "%s/%s_%d.weights",backup_directory,base, epoch);
            save_weights(net, buff);
        }
        if(get_current_batch(net)%100 == 0){
            char buff[256];
            sprintf(buff, "%s/%s.backup",backup_directory,base);
            save_weights(net, buff);
        }
    }
    
    sprintf(buff, "%s/%s.weights", backup_directory, base);
    save_weights(net, buff);

    pthread_join(load_thread, 0);
    free_data(buffer);
    free_network(net);
    free_ptrs((void**)paths, plist->size);
    free_list(plist);
    free(base);
}

void test_tag(char *cfgfile, char *weightfile, char *filename)
{
	int i = 0;
	char **names;
	clock_t time;
    int indexes[10];
    char buff[256];
    char *input = buff;
	int size;

    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(2222222);
    
    names = get_labels("data/tags.txt");
    
    size = net.w;
    while(1){
		image im ;
		image r;
		float *X;
		float *predictions;
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
        r = resize_min(im, size);
        resize_network(&net, r.w, r.h);
        printf("%d %d\n", r.w, r.h);

        X = r.data;
        time=clock();
        predictions = network_predict(net, X);
        top_predictions(net, 10, indexes);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        for(i = 0; i < 10; ++i){
            int index = indexes[i];
            printf("%.1f%%: %s\n", predictions[index]*100, names[index]);
        }
        if(r.data != im.data) free_image(r);
        free_image(im);
        if (filename) break;
    }
}


void run_tag(int argc, char **argv)
{
	char *cfg;
    char *weights;
    char *filename;
	int clear;
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [train/test/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    clear = find_arg(argc, argv, "-clear");
    cfg = argv[3];
    weights = (argc > 4) ? argv[4] : 0;
    filename = (argc > 5) ? argv[5] : 0;
    if(0==strcmp(argv[2], "train")) train_tag(cfg, weights, clear);
    else if(0==strcmp(argv[2], "test")) test_tag(cfg, weights, filename);
}

