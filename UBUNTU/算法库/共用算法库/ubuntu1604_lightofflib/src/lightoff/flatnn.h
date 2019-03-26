#ifndef __FLATNN_H__
#define __FLATNN_H__

#define MOK			        0
#define MERR_BASIC_BASE		0X0001
#define MERR_NO_MEMORY		(MERR_BASIC_BASE+3)

#define CHECK_RES(res) if(res!=0) {goto exit;}
#define CHECK_PTR(ptr) if((ptr) == 0) {         \
    res = MERR_NO_MEMORY;                       \
    goto exit;                                  \
  }

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define USE_EIGEN

#ifdef __cplusplus
extern "C" {
#endif

  int ly_data_const_mean_C3(
      const unsigned char* inputs, int iw, int ih, int widStep,
      const float meanC3[3],
      float* outputs);

  int ly_conv_float(
      const float* inputs, int iw, int ih, int ichannels, int groups,
      const float* weights, const float* bias, int ww, int wh, int pad_w, int pad_h, int stepw, int steph,
      float* outputs, int ochannels);

  int ly_relu_float(const float* inputs, float* outputs, int num);

  int ly_softmax(const float* inputs, int lwh, int ichannels, int ln, float* output);

  int ly_inner_product(const float* inputs, int num, int inputnum, float* outputs, int outputnum,
                       const float* weights, const float* bias);

  int ly_pooling_max_float(const float* inputs, int iw, int ih, int ichannels,
                           float* outputs, int kw, int kh, int stepw, int steph, int pad_w, int pad_h);
  int ly_pooling_ave_float(const float* inputs, int iw, int ih, int ichannels,  float* outputs, int kw, int kh, int stepw, int steph, int pad_w, int pad_h);

  int ly_roipooling_maxf(const float* inputs, int width_, int height_, int channels_,
                         const float* rois, int num_rois, int pooled_width_, int pooled_height_, float spatial_scale_,
                         float* outputs, int noutput);

  int ly_rpn(const float* inputs_cls, int iw, int ih, int ichannels, int imagew, int imageh,
             const float* input_bbox_pred, float* rois, int feat_stride_,
             int pre_nms_topN_, int post_nms_topN_, float nms_thresh_, int min_size_,
             float* anchors, int num_anchors, int* num_keep);

  int ly_concat_channels_float(float* inputs, int iw, int ih,
                               int ichannels, float* outputs, int* ptrout);

  int ly_lrn_cross_float(const float* inputs, int iw, int ih, int ichannels,int ln,
                         float* outputs, int size, float alpha, float beta, float k);

  int ly_deconv_float(
			const float* inputs, int iw, int ih, int ichannels, int groups,
			const float* weights, const float* bias, int ww, int wh, int pad_w, int pad_h, int stepw, int steph,
			float* outputs, int ochannels);
#ifdef __cplusplus
}
#endif

#endif
