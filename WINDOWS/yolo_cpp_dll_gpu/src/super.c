#include "network.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif

void train_super(char *cfgfile, char *weightfile)
{
    char *train_images = "/data/imagenet/imagenet1k.train.list";
    char *backup_directory = "/home/pjreddie/backup/";
	char *base;
	float avg_loss = -1;
	network net;
	int imgs;
	int i;
	data train, buffer;
	list *plist;
	char **paths;
	load_args args = {0};
	pthread_t load_thread;
	clock_t time1;
	char buff[256];

    srand(time(0));
    base = basecfg(cfgfile);
    printf("%s\n", base);

   
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    imgs = net.batch*net.subdivisions;
    i = *net.seen/imgs;
    


    plist = get_paths(train_images);
    //int N = plist->size;
    paths = (char **)list_to_array(plist);

    
    args.w = net.w;
    args.h = net.h;
    args.scale = 4;
    args.paths = paths;
    args.n = imgs;
    args.m = plist->size;
    args.d = &buffer;
    args.type = SUPER_DATA;

    load_thread = load_data_in_thread(args);
   
    //while(i*imgs < N*120){
    while(get_current_batch(net) < net.max_batches){
		float loss;
        i += 1;
        time1=clock();
        pthread_join(load_thread, 0);
        train = buffer;
        load_thread = load_data_in_thread(args);

        printf("Loaded: %lf seconds\n", sec(clock()-time1));

        time1=clock();
        loss = train_network(net, train);
        if (avg_loss < 0) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;

        printf("%d: %f, %f avg, %f rate, %lf seconds, %d images\n", i, loss, avg_loss, get_current_rate(net), sec(clock()-time1), i*imgs);
        if(i%1000==0){
            char buff[256];
            sprintf(buff, "%s/%s_%d.weights", backup_directory, base, i);
            save_weights(net, buff);
        }
        if(i%100==0){
            char buff[256];
            sprintf(buff, "%s/%s.backup", backup_directory, base);
            save_weights(net, buff);
        }
        free_data(train);
    }
    
    sprintf(buff, "%s/%s_final.weights", backup_directory, base);
    save_weights(net, buff);
}

void test_super(char *cfgfile, char *weightfile, char *filename)
{
	clock_t time;
	char buff[256];
    char *input = buff;
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(2222222);

    
    
    while(1){
		image im;
		float *X;
		image out;
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
        resize_network(&net, im.w, im.h);
        printf("%d %d\n", im.w, im.h);

        X = im.data;
        time=clock();
        network_predict(net, X);
        out = get_network_image(net);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        save_image(out, "out");

        free_image(im);
        if (filename) break;
    }
}


void run_super(int argc, char **argv)
{
	char *cfg;
    char *weights;
    char *filename;
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [train/test/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    cfg = argv[3];
    weights = (argc > 4) ? argv[4] : 0;
    filename = (argc > 5) ? argv[5] : 0;
    if(0==strcmp(argv[2], "train")) train_super(cfg, weights);
    else if(0==strcmp(argv[2], "test")) test_super(cfg, weights, filename);
    /*
    else if(0==strcmp(argv[2], "valid")) validate_super(cfg, weights);
    */
}