#include "network.h"
#include "utils.h"
#include "parser.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif

void train_writing(char *cfgfile, char *weightfile)
{
	float avg_loss = -1;
	char *base = basecfg(cfgfile);
	network net = parse_network_cfg(cfgfile);
	int imgs;
	list *plist;
	char **paths;
	clock_t time1;
	int N;
	image out;
	data train, buffer;
	load_args args = {0};
	pthread_t load_thread;
	int epoch;
    char *backup_directory = "/home/pjreddie/backup/";
    srand(time(0));
    
    
    printf("%s\n", base);
    
    if(weightfile){
        load_weights(&net, weightfile);
    }
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    imgs = net.batch*net.subdivisions;
    plist = get_paths("figures.list");
    paths = (char **)list_to_array(plist);
    
    N = plist->size;
    printf("N: %d\n", N);
    out = get_network_image(net);

    

    
    args.w = net.w;
    args.h = net.h;
    args.out_w = out.w;
    args.out_h = out.h;
    args.paths = paths;
    args.n = imgs;
    args.m = N;
    args.d = &buffer;
    args.type = WRITING_DATA;

    load_thread = load_data_in_thread(args);
    epoch = (*net.seen)/N;
    while(get_current_batch(net) < net.max_batches || net.max_batches == 0){
		float loss;
        time1=clock();
        pthread_join(load_thread, 0);
        train = buffer;
        load_thread = load_data_in_thread(args);
        printf("Loaded %lf seconds\n",sec(clock()-time1));

        time1=clock();
        loss = train_network(net, train);

        /*
           image pred = float_to_image(64, 64, 1, out);
           print_image(pred);
         */

        /*
           image im = float_to_image(256, 256, 3, train.X.vals[0]);
           image lab = float_to_image(64, 64, 1, train.y.vals[0]);
           image pred = float_to_image(64, 64, 1, out);
           show_image(im, "image");
           show_image(lab, "label");
           print_image(lab);
           show_image(pred, "pred");
           cvWaitKey(0);
         */

        if(avg_loss == -1) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;
        printf("%d, %.3f: %f, %f avg, %f rate, %lf seconds, %d images\n", get_current_batch(net), (float)(*net.seen)/N, loss, avg_loss, get_current_rate(net), sec(clock()-time1), *net.seen);
        free_data(train);
        if(get_current_batch(net)%100 == 0){
            char buff[256];
            sprintf(buff, "%s/%s_batch_%d.weights", backup_directory, base, get_current_batch(net));
            save_weights(net, buff);
        }
        if(*net.seen/N > epoch){
			char buff[256];
            epoch = *net.seen/N;
            
            sprintf(buff, "%s/%s_%d.weights",backup_directory,base, epoch);
            save_weights(net, buff);
        }
    }
}

void test_writing(char *cfgfile, char *weightfile, char *filename)
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
		image pred;
		image upsampled;
		image thresh;
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
        printf("%d %d %d\n", im.h, im.w, im.c);
        X = im.data;
        time=clock();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, sec(clock()-time));
        pred = get_network_image(net);

        upsampled = resize_image(pred, im.w, im.h);
        thresh = threshold_image(upsampled, .5);
        pred = thresh;

        show_image(pred, "prediction");
        show_image(im, "orig");
#ifdef OPENCV
        cvWaitKey(0);
        cvDestroyAllWindows();
#endif

        free_image(upsampled);
        free_image(thresh);
        free_image(im);
        if (filename) break;
    }
}

void run_writing(int argc, char **argv)
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
    if(0==strcmp(argv[2], "train")) train_writing(cfg, weights);
    else if(0==strcmp(argv[2], "test")) test_writing(cfg, weights, filename);
}

