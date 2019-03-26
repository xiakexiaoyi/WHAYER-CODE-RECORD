#include "network.h"
#include "utils.h"
#include "parser.h"

char *dice_labels[] = {"face1","face2","face3","face4","face5","face6"};

void train_dice(char *cfgfile, char *weightfile)
{
	float avg_loss = -1;
	char *base = basecfg(cfgfile);
    char *backup_directory = "/home/pjreddie/backup/";
	network net;
	int imgs = 1024;
	int i;
	char **labels;
	list *plist;
	char **paths;
	clock_t time1;
    srand(time(0));
    
    
    printf("%s\n", base);
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    
    i = *net.seen/imgs;
    labels = dice_labels;
    plist = get_paths("data/dice/dice.train.list");
    paths = (char **)list_to_array(plist);
    printf("%d\n", plist->size);
    
    while(1){
		data train;
		float loss;
        ++i;
        time1=clock();
        train = load_data_old(paths, imgs, plist->size, labels, 6, net.w, net.h);
        printf("Loaded: %lf seconds\n", sec(clock()-time1));

        time1=clock();
        loss = train_network(net, train);
        if(avg_loss == -1) avg_loss = loss;
        avg_loss = avg_loss*.9 + loss*.1;
        printf("%d: %f, %f avg, %lf seconds, %d images\n", i, loss, avg_loss, sec(clock()-time1), *net.seen);
        free_data(train);
        if((i % 100) == 0) net.learning_rate *= .1;
        if(i%100==0){
            char buff[256];
            sprintf(buff, "%s/%s_%d.weights",backup_directory,base, i);
            save_weights(net, buff);
        }
    }
}

void validate_dice(char *filename, char *weightfile)
{
	char **labels;
	list *plist;
	char **paths;
	int m;
	data val;
	float *acc;
    network net = parse_network_cfg(filename);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    srand(time(0));

    labels = dice_labels;
    plist = get_paths("data/dice/dice.val.list");

    paths = (char **)list_to_array(plist);
    m = plist->size;
    free_list(plist);

    val = load_data_old(paths, m, 0, labels, 6, net.w, net.h);
    acc = network_accuracies(net, val, 2);
    printf("Validation Accuracy: %f, %d images\n", acc[0], m);
    free_data(val);
}

void test_dice(char *cfgfile, char *weightfile, char *filename)
{
	int i = 0;
	char **names;
	char buff[256];
    char *input = buff;
    int indexes[6];
    network net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    srand(2222222);
    
    names = dice_labels;
    
    while(1){
		image im;
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
        im = load_image_color(input, net.w, net.h);
        X = im.data;
        predictions = network_predict(net, X);
        top_predictions(net, 6, indexes);
        for(i = 0; i < 6; ++i){
            int index = indexes[i];
            printf("%s: %f\n", names[index], predictions[index]);
        }
        free_image(im);
        if (filename) break;
    }
}

void run_dice(int argc, char **argv)
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
    filename = (argc > 5) ? argv[5]: 0;
    if(0==strcmp(argv[2], "test")) test_dice(cfg, weights, filename);
    else if(0==strcmp(argv[2], "train")) train_dice(cfg, weights);
    else if(0==strcmp(argv[2], "valid")) validate_dice(cfg, weights);
}

