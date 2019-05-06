#include "yolo_v3_dll.h"
#include "image.h"
#include "utils.h"
#include "network.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "demo.h"
#include "option_list.h"


//void test_detector123(char *cfgfile, char *weightfile)
//{
//	float thresh = 0.24;
//	float hier_thresh = 0.5;
//	char *filename = "123.jpg";
//	network net = parse_network_cfg_custom(cfgfile, 1); // set batch=1
//	if (weightfile) {
//		load_weights(&net, weightfile);
//	}
//	//set_batch_network(&net, 1);
//	fuse_conv_batchnorm(net);
//	calculate_binary_weights(net);
//
//	srand(2222222);
//	double time;
//	char buff[256];
//	char *input = buff;
//	int j;
//	float nms = .45;    // 0.4F
//	while (1) {
//		
//		image im = load_image(input, 0, 0, net.c);
//		int letterbox = 0;
//		image sized = resize_image(im, net.w, net.h);
//		//image sized = letterbox_image(im, net.w, net.h); letterbox = 1;
//		layer l = net.layers[net.n - 1];
//
//		//box *boxes = calloc(l.w*l.h*l.n, sizeof(box));
//		//float **probs = calloc(l.w*l.h*l.n, sizeof(float *));
//		//for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = calloc(l.classes, sizeof(float *));
//
//		float *X = sized.data;
//
//		//time= what_time_is_it_now();
//		double time = get_time_point();
//		network_predict(net, X);
//		//network_predict_image(&net, im); letterbox = 1;
//		printf("%s: Predicted in %lf milli-seconds.\n", input, ((double)get_time_point() - time) / 1000);
//		//printf("%s: Predicted in %f seconds.\n", input, (what_time_is_it_now()-time));
//
//		int nboxes = 0;
//		detection *dets = get_network_boxes(&net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes, letterbox);
//		if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
//		//draw_detections_v3(im, dets, nboxes, thresh, names, alphabet, l.classes, ext_output);
//		save_image(im, "predictions");
//		
//
//		// pseudo labeling concept - fast.ai
//		
//
//		free_detections(dets, nboxes);
//		free_image(im);
//		free_image(sized);
//		//free(boxes);
//		//free_ptrs((void **)probs, l.w*l.h*l.n);
//#ifdef OPENCV
//		if (!dont_show) {
//			cvWaitKey(0);
//			cvDestroyAllWindows();
//		}
//#endif
//		if (filename) break;
//	}
//
//	// free memory
//	//free_ptrs(names, net.layers[net.n - 1].classes);
//	//free_list_contents_kvp(options);
//	//free_list(options);
//
//	int i;
//	const int nsize = 8;
//	/*for (j = 0; j < nsize; ++j) {
//		for (i = 32; i < 127; ++i) {
//			free_image(alphabet[j][i]);
//		}
//		free(alphabet[j]);
//	}
//	free(alphabet);*/
//
//	free_network(net);
//}
//int main123()
//{
//	//char *cfgfile = "D:/work/model/xnortest/yolov3-tiny_xnor.cfg";
//	//char *weightfile = "D:/work/model/xnortest/yolov3-tiny_xnor_1700.weights";
//	char *cfgfile = "D:/work/model/1/yolov3-tiny_xnor_voc.cfg";
//	char *weightfile = "D:/work/model/1/yolov3-tiny_xnor_voc_100.weights";
//	test_detector123(cfgfile,weightfile);
//}
int yolo_detector_v3(const network net, const image pImg, float thresh, int *numDST, float **boxout)
{
	int res = 0;
	srand(2222222);
	double time;
	int j;
	float nms = .45;    // 0.4F

	int letterbox = 0;
	float hier_thresh = 0.5;
	image sized = resize_image(pImg, net.w, net.h);
	layer l = net.layers[net.n - 1];

	float *X = sized.data;
	time = what_time_is_it_now();
	network_predict(net, X);
	//printf(" Predicted in %f seconds.\n", (what_time_is_it_now() - time));
	int nboxes = 0;
	detection *dets = get_network_boxes(&net, pImg.w, pImg.h, thresh, hier_thresh, 0, 1, &nboxes, letterbox);
	if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
	box_detections_v3(pImg, dets, nboxes, thresh, numDST, boxout);
	free_detections(dets, nboxes);
	free_image(sized);

	return res;
}
int yolo_detector_v3_no_gpu(const network net, const image pImg, float thresh, int *numDST, float **boxout)
{
	return yolo_detector_v3(net, pImg, thresh, numDST, boxout);
}
image _make_empty_image(int w, int h, int c)
{
	image out;
	out.data = 0;
	out.h = h;
	out.w = w;
	out.c = c;
	return out;
}

image _make_image(int w, int h, int c)
{
	image out = _make_empty_image(w, h, c);
	out.data = (float*)calloc(h*w*c, sizeof(float));
	return out;
}
image _ipl_to_image(unsigned char *data, int h, int w, int c, int step)
{

	image out = _make_image(w, h, c);
	int i, j, k, count = 0;
	if (out.data == NULL)
		return out;
	for (k = 0; k < c; ++k) {
		for (i = 0; i < h; ++i) {
			for (j = 0; j < w; ++j) {
				out.data[count++] = data[i*step + j*c + k] / 255.;
			}
		}
	}
	return out;
}
void _rgbgr_image(image im)
{
	int i;
	for (i = 0; i < im.w*im.h; ++i) {
		float swap = im.data[i];
		im.data[i] = im.data[i + im.w*im.h * 2];
		im.data[i + im.w*im.h * 2] = swap;
	}
}
void _free_image(image m)
{
	if (m.data) {
		free(m.data);
	}
}

image _make_image_no_gpu(int w, int h, int c)
{
	return _make_image(w,h,c);
}
image _ipl_to_image_no_gpu(unsigned char *data, int h, int w, int c, int step)
{
	return _ipl_to_image(data,h,w,c,step);
}
void _rgbgr_image_no_gpu(image im)
{
	_rgbgr_image(im);
}
void _free_image_no_gpu(image m)
{
	_free_image(m);
}

