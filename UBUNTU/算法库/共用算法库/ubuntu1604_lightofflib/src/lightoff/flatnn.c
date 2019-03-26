#include "flatnn.h"
#include <math.h>
#include <float.h>

#ifndef CHECK_PTR
#define CHECK_PTR(ptr) if(!ptr) { res = MERR_NO_MEMORY; goto exit; }
#endif

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

#ifdef USE_EIGEN
void convmul(const float* col_buffer, const float* weights, float* output, int M, int N, int K);
#endif

double round(double val)
{
    return (val> 0.0) ? floor(val+ 0.5) : ceil(val- 0.5);
}

struct ff2ss
{
  float* bbx;
  float score;
  float area;
};

static int c_f2ss_compare(const void* aa,const void* bb)
{
  struct ff2ss* a = (struct ff2ss*) aa;
  struct ff2ss* b = (struct ff2ss*) bb;

  if(a->score>b->score)
    return -1;
  else
    return 1;
}

static int
bbox_transform_inv(
    const float* bbox,
    const float* deltas,
    float* pred_boxes,
    float* score,
    int nboxes,
    float w, float h,
    float minsize)
{
  int nkeep = 0;
  int i;
  for (i = 0; i < nboxes; ++i) {
    float widths = bbox[2] - bbox[0] + 1.0f;
    float heights = bbox[3] - bbox[1] + 1.0f;
    float ctr_x = bbox[0] + 0.5f * widths;
    float ctr_y = bbox[1] + 0.5f * heights;
    float pred_ctr_x = deltas[0] * widths + ctr_x;
    float pred_ctr_y = deltas[1] * heights + ctr_y;
    float pred_w = expf(deltas[2]) * widths;
    float pred_h = expf(deltas[3]) * heights;

    pred_boxes[0] = pred_ctr_x - 0.5f * pred_w;
    pred_boxes[1] = pred_ctr_y - 0.5f * pred_h;
    pred_boxes[2] = pred_ctr_x + 0.5f * pred_w;
    pred_boxes[3] = pred_ctr_y + 0.5f * pred_h;

    pred_boxes[0] = MAX(0.f, pred_boxes[0]);
    pred_boxes[0] = MIN(pred_boxes[0], w-1);
    pred_boxes[1] = MAX(0.f, pred_boxes[1]);
    pred_boxes[1] = MIN(pred_boxes[1], h-1);
    pred_boxes[2] = MAX(0.f, pred_boxes[2]);
    pred_boxes[2] = MIN(pred_boxes[2], w-1);
    pred_boxes[3] = MAX(0.f, pred_boxes[3]);
    pred_boxes[3] = MIN(pred_boxes[3], h-1);

    pred_w = pred_boxes[2] - pred_boxes[0] + 1.0f;
    pred_h = pred_boxes[3] - pred_boxes[1] + 1.0f;

    if(pred_w>=minsize && pred_h>=minsize) {
      score[nkeep] = score[i];
      pred_boxes += 4;
      nkeep++;
    }

    bbox += 4;
    deltas += 4;
  }

  return nkeep;
}

static int
c_nms(struct ff2ss* fs, int nfs, float overlap, int* keepbuf, int npreserv) {
  int nkeep = 0;
  int i, j;
  for (i = 0; i < nfs; ++i) {
    float* bbox;
    float ix1;
    float iy1;
    float ix2;
    float iy2;

    if(fs[i].score <= 0)
      continue;

    bbox = fs[i].bbx;
    ix1 = bbox[0];
    iy1 = bbox[1];
    ix2 = bbox[2];
    iy2 = bbox[3];

    keepbuf[nkeep++] = i;
    if(nkeep>=npreserv)
      break;

    for (j = i+1; j < nfs; ++j) {
      float* bbox2;
      float xx1;
      float yy1;
      float xx2;
      float yy2;

      float w;
      float h;
      float inter;
      float ovr;

      if(fs[j].score <= 0)
        continue;

      bbox2 = fs[j].bbx;
      xx1 = MAX(ix1, bbox2[0]);
      yy1 = MAX(iy1, bbox2[1]);
      xx2 = MIN(ix2, bbox2[2]);
      yy2 = MIN(iy2, bbox2[3]);

      w = MAX(0.0f, xx2 - xx1 + 1);
      h = MAX(0.0f, yy2 - yy1 + 1);
      inter = w*h;
      ovr = inter / (fs[i].area + fs[j].area - inter);
      if(ovr >= overlap)
        fs[j].score = -1;
    }
  }
  return nkeep;
}

static void im2col_cpu(const float* data_im, const int channels,
                       const int height, const int width, const int kernel_h,
                       const int kernel_w, const int pad_h, const int pad_w,
                       const int stride_h, const int stride_w, float* data_col)
{
  int height_col = (height + 2 * pad_h - kernel_h) / stride_h + 1;
  int width_col = (width + 2 * pad_w - kernel_w) / stride_w + 1;
  int channels_col = channels * kernel_h * kernel_w;
  int h, w, c;

  for (c = 0; c < channels_col; ++c) {
    int w_offset = c % kernel_w;
    int h_offset = (c / kernel_w) % kernel_h;
    int c_im = c / kernel_h / kernel_w;
    for (h = 0; h < height_col; ++h) {
      for (w = 0; w < width_col; ++w) {
        int h_pad = h * stride_h - pad_h + h_offset;
        int w_pad = w * stride_w - pad_w + w_offset;
        if (h_pad >= 0 && h_pad < height && w_pad >= 0 && w_pad < width)
          data_col[(c * height_col + h) * width_col + w] =
              data_im[(c_im * height + h_pad) * width + w_pad];
        else
          data_col[(c * height_col + h) * width_col + w] = 0;
      }
    }
  }
}
static void col2im_cpu(const float* data_col, const int channels,
    const int height, const int width, const int patch_h, const int patch_w,
    const int pad_h, const int pad_w,
    const int stride_h, const int stride_w,
	float* data_im)
{
	int height_col = (height + 2 * pad_h - patch_h) / stride_h + 1;
	int width_col = (width + 2 * pad_w - patch_w) / stride_w + 1;
	int channels_col = channels * patch_h * patch_w;
	int c, h, w;
	for (c = 0; c < channels_col; ++c) {
		int w_offset = c % patch_w;
		int h_offset = (c / patch_w) % patch_h;
		int c_im = c / patch_h / patch_w;
		for (h = 0; h < height_col; ++h) {
			for (w = 0; w < width_col; ++w) {
				int h_pad = h * stride_h - pad_h + h_offset;
				int w_pad = w * stride_w - pad_w + w_offset;
				if (h_pad >= 0 && h_pad < height && w_pad >= 0 && w_pad < width)
					data_im[(c_im * height + h_pad) * width + w_pad] +=
							data_col[(c * height_col + h) * width_col + w];
			}
		}
	}
}

int ly_data_const_mean_C3(
    const unsigned char* inputs, int iw, int ih, int widStep,
    const float meanC3[3],
    float* outputs)
{
  int i, j;

  for(j=0;j<ih;j++) {
    const unsigned char*ptrBgr = inputs+j*widStep;

    float* otrOutputCB = outputs + j*iw;
    float* otrOutputCG = otrOutputCB + iw*ih;
    float* otrOutputCR = otrOutputCG + iw*ih;

    for(i=0;i<iw;i++) {
      otrOutputCB[i] = (float) ptrBgr[0] - meanC3[0];
      otrOutputCG[i] = (float) ptrBgr[1] - meanC3[1];
      otrOutputCR[i] = (float) ptrBgr[2] - meanC3[2];
      ptrBgr += 3;
    }
  }

  return MOK;
}

int ly_inner_product(const float* inputs, int num, int inputnum, float* outputs, int outputnum,
                     const float* weights, const float* bias)
{
  int res = MOK;
  float* all1 = 0;

  int N = outputnum;
  int K = inputnum;
  int i, k;

  convmul(weights,inputs , outputs , num, N, K);
  if (bias) {
    for (i = 0; i < num; i++) {
      for (k = 0; k < N; k++)
        outputs[i * N + k] += bias[k];
    }
  }

  return res;
}

int ly_conv_float(
    const float* inputs, int iw, int ih, int ichannels, int groups,
    const float* weights, const float* bias, int ww, int wh, int pad_w, int pad_h, int stepw, int steph,
    float* outputs, int ochannels)
{
  int res = MOK;
  int j, k;

  float* col_buffer = 0;
  int M, N, K;
  int i=0, g;

  int oh = (ih + 2 * pad_h - wh) / steph + 1;
  int ow = (iw + 2 * pad_w - ww) / stepw + 1;

  int weight_offset=0, col_offset=0, top_offset=0;

  M = ochannels/groups;
  N = ow * oh;
  K = ww * wh * ichannels/groups;

  weight_offset = M* K;
  col_offset = K*N;
  top_offset = M*N;

  col_buffer = (float*) calloc(1, ichannels * ww * wh * ow * oh * sizeof(float));
  CHECK_PTR(col_buffer);

  im2col_cpu(inputs, ichannels, ih, iw, wh, ww, pad_h, pad_w, steph, stepw,
             col_buffer);

  for (g = 0; g < groups; ++g) {
    convmul(col_buffer + col_offset * g, weights + weight_offset * g, outputs + top_offset*g , M, N, K);
  }

  if(bias) {
	  for(j=0;j<ochannels;j++)
	  {
		for(k=0;k<N;k++)
		{
		  outputs[j*N+k] += bias[j];
		}
	  }
  }

exit:
  if( col_buffer ) free(col_buffer);
  return res;
}

int ly_relu_float(const float* inputs, float* outputs, int n)
{
  int res = MOK;
  int i = 0;

  for(i=0;i<n;i++)
    outputs[i] = MAX(inputs[i], 0);

  return res;
}

int ly_softmax(const float* inputs, int lwh, int ichannels, int ln, float* output)
{
  int res = MOK;
  int i = 0, j = 0, k = 0;
  int dim = lwh * ichannels;
  float* scale_data = 0;
  float* top_data;

  memcpy(output, inputs, sizeof(float)*ichannels*lwh*ln);
  scale_data = (float*) calloc((size_t) lwh, sizeof(float));
  CHECK_PTR(scale_data);

  for (i = 0; i < ln; ++i) {
    top_data = output + i * dim;

    // initialize scale_data to the first plane
    memcpy(scale_data, inputs+i*dim, lwh * sizeof(float));

    for (j = 0; j < ichannels; j++) {
      for (k = 0; k < lwh; k++) {
        scale_data[k] = MAX(scale_data[k], inputs[i * dim + j * lwh + k]);
      }
    }

    // subtraction
    for (j = 0; j < ichannels; ++j) {
      for (k = 0; k < lwh; ++k) {
        top_data[j*lwh+k] = (float) exp(top_data[j*lwh+k]-scale_data[k]);
      }
    }

    // sum after exp
    memset(scale_data, 0, sizeof(float)*lwh);
    for (j = 0; j < ichannels; j++) {
      for (k = 0; k < lwh; k++) {
        scale_data[k] += top_data[j*lwh+k];
      }
    }

    // division
    for (j = 0; j < ichannels; j++) {
      for (k = 0; k < lwh; ++k) {
        top_data[j*lwh+k] /= scale_data[k];
      }
    }
  }

exit:
  if(scale_data)
    free(scale_data);

  return res;
}

int ly_pooling_max_float(const float* inputs, int iw, int ih, int ichannels, float* outputs, int kw, int kh, int stepw, int steph, int pad_w, int pad_h)
{
  int res = MOK;
  int ow=0, oh=0;
  int c, ph, pw, h, w;

  oh = (int)(ceil((float)(ih + 2 * pad_h - kh) / steph)) + 1;
  ow = (int)(ceil((float)(iw + 2 * pad_w - kw) / stepw)) + 1;

  for (c = 0; c < ichannels; ++c)
  {
    const float* ptrInputs = inputs + iw*ih*c;
    float* ptrOutputs = outputs + ow*oh*c;

    for (ph = 0; ph < oh; ++ph)
    {
      for (pw = 0; pw < ow; ++pw)
      {
        int hstart = ph * steph - pad_h;
        int wstart = pw * stepw - pad_w;
        int hend = MIN(hstart + kh, ih);
        int wend = MIN(wstart + kw, iw);
        int pool_index = ph * ow + pw;

        hstart = MAX(hstart, 0);
        wstart = MAX(wstart, 0);

        ptrOutputs[pool_index] = -FLT_MAX;

        for (h = hstart; h < hend; ++h)
        {
          for (w = wstart; w < wend; ++w)
          {
            int index = h * iw + w;
            if (ptrInputs[index] > ptrOutputs[pool_index])
            {
              ptrOutputs[pool_index] = ptrInputs[index];
            }
          }
        }
      }
    }
  }
  return res;
}

int ly_pooling_ave_float(const float* inputs, int iw, int ih, int ichannels,  float* outputs, int kw, int kh, int stepw, int steph, int pad_w, int pad_h)
{

	  int res = MOK;
	  int ow=0, oh=0;
	  int c, ph, pw, h, w;
	  int sum,num;
	 
	  oh = (int)(ceil((float)(ih + 2 * pad_h - kh) / steph)) + 1;
	  ow = (int)(ceil((float)(iw + 2 * pad_w - kw) / stepw)) + 1;

	  for (c = 0; c < ichannels; ++c)
	  {
			const float* ptrInputs = inputs + iw*ih*c;
			float* ptrOutputs = outputs + ow*oh*c;

			for (ph = 0; ph < oh; ++ph)
			{
				for (pw = 0; pw < ow; ++pw)
			  {
				int hstart = ph * steph - pad_h;
				int wstart = pw * stepw - pad_w;
				int hend = MIN(hstart + kh, ih);
				int wend = MIN(wstart + kw, iw);
				int pool_index = ph * ow + pw;

				hstart = MAX(hstart, 0);
				wstart = MAX(wstart, 0);

				ptrOutputs[pool_index] = -FLT_MAX;
				 sum=0;
				 num = 0;
				for (h = hstart; h < hend; ++h)
				{
				  for (w = wstart; w < wend; ++w)
				  {
					int index = h * iw + w;
					sum+=ptrInputs[index];					
					num++;
					
				  }
				}
				ptrOutputs[pool_index] = sum/(float)num;
			  }
		 }
	  }
	  return res;
}



int ly_roipooling_maxf(const float* inputs, int width_, int height_, int channels_,
                       const float* rois, int num_rois, int pooled_width_, int pooled_height_, float spatial_scale_,
                       float* outputs, int noutput)
{
  int i = 0, n=0;

  for(i=0;i<noutput;i++)
    outputs[i] = -FLT_MAX;

  for (n = 0; n < num_rois; ++n) {
    int roi_start_w = (int) round(rois[0] * spatial_scale_);
    int roi_start_h = (int) round(rois[1] * spatial_scale_);
    int roi_end_w = (int) round(rois[2] * spatial_scale_);
    int roi_end_h = (int) round(rois[3] * spatial_scale_);

    int roi_height = MAX(roi_end_h - roi_start_h + 1, 1);
    int roi_width = MAX(roi_end_w - roi_start_w + 1, 1);

    const float bin_size_h = (float)(roi_height) / pooled_height_;
    const float bin_size_w = (float)(roi_width) / pooled_width_;

    const float *batch_data = inputs;

    int w, h, c, ph, pw;
    for (c = 0; c < channels_; ++c) {
      for (ph = 0; ph < pooled_height_; ++ph) {
        for (pw = 0; pw < pooled_width_; ++pw) {
          // Compute pooling region for this output unit:
          //  start (included) = floor(ph * roi_height / pooled_height_)
          //  end (excluded) = ceil((ph + 1) * roi_height / pooled_height_)
          int hstart = (int)(floor(ph * bin_size_h));
          int wstart = (int)(floor(pw * bin_size_w));
          int hend = (int)(ceil((ph + 1) * bin_size_h));
          int wend = (int)(ceil((pw + 1) * bin_size_w));
		  int is_empty;
		  int pool_index;

          hstart = MIN(MAX(hstart + roi_start_h, 0), height_);
          hend = MIN(MAX(hend + roi_start_h, 0), height_);
          wstart = MIN(MAX(wstart + roi_start_w, 0), width_);
          wend = MIN(MAX(wend + roi_start_w, 0), width_);

          is_empty = (hend <= hstart) || (wend <= wstart);
          pool_index = ph * pooled_width_ + pw;
          if (is_empty) {
            outputs[pool_index] = 0;
          }

          for (h = hstart; h < hend; ++h) {
            for (w = wstart; w < wend; ++w) {
              const int index = h * width_ + w;
              if (batch_data[index] > outputs[pool_index]) {
                outputs[pool_index] = batch_data[index];
              }
            }
          }
        }
      }
      // Increment all data pointers by one channel
      batch_data += width_ * height_;
      outputs += pooled_width_ * pooled_height_;
    }
    // Increment ROI data pointer
    rois += 4;
  }

  return MOK;
}

int ly_rpn(const float* inputs_cls, int iw, int ih, int ichannels, int imagew, int imageh,
           const float* bbox_deltas, float* rois, int feat_stride_,
           int pre_nms_topN_, int post_nms_topN_, float nms_thresh_, int min_size_,
           float* anchors_, int num_anchors_, int* num_keep)
{
  int res = 0;
  int width = iw;
  int height = ih;
  int i, j, c, k, K;
  float* shifts=0, *ptrs=0;
  float* anchors=0, *fptrs=0;
  float* bbox_deltas_r=0, *scores_r=0;
  float* proposals=0;
  struct ff2ss* f2buf = 0;
  int nkeep;
  int* kp=0; int topN;
  const float* scores = inputs_cls + width*height*num_anchors_;

  shifts = (float*) calloc(1, 4*width*height*sizeof(float));
  CHECK_PTR(shifts);

  ptrs = shifts;
  for (j = 0; j < height; ++j) {
    for (i = 0; i < width; ++i) {
      ptrs[0] = (float)(i*feat_stride_);
      ptrs[2] = ptrs[0];
      ptrs[1] = (float)(j*feat_stride_);
      ptrs[3] = ptrs[1];
      ptrs += 4;
    }
  }

  K = width*height;
  anchors = (float*)calloc(1, 4*K*num_anchors_*sizeof(float));
  CHECK_PTR(anchors);

  fptrs = anchors;
  ptrs = &shifts[0];
  for(j=0;j<K;j++) {
    float* anc = &anchors_[0];
    for (i = 0; i < num_anchors_; ++i) {
      fptrs[0] = anc[0] + ptrs[0];
      fptrs[1] = anc[1] + ptrs[1];
      fptrs[2] = anc[2] + ptrs[2];
      fptrs[3] = anc[3] + ptrs[3];
      fptrs += 4;
      anc += 4;
    }
    ptrs += 4;
  }

  // reshape bbox_deltas
  bbox_deltas_r = (float*) calloc(1, 4*num_anchors_*width*height*sizeof(float));
  CHECK_PTR(bbox_deltas_r);

  for(c=0;c<num_anchors_;c++) {
    for(j=0;j<height;j++) {
      for (i=0;i<width;i++) {
        bbox_deltas_r[(j*width+i)*num_anchors_*4+4*c+0] = bbox_deltas[(4*c+0)*width*height + j*width + i];
        bbox_deltas_r[(j*width+i)*num_anchors_*4+4*c+1] = bbox_deltas[(4*c+1)*width*height + j*width + i];
        bbox_deltas_r[(j*width+i)*num_anchors_*4+4*c+2] = bbox_deltas[(4*c+2)*width*height + j*width + i];
        bbox_deltas_r[(j*width+i)*num_anchors_*4+4*c+3] = bbox_deltas[(4*c+3)*width*height + j*width + i];
      }
    }
  }

  // reshape score
  // scores are (1, A, H, W) format
  // transpose to (1, H, W, A)
  scores_r = (float*) calloc(1, num_anchors_*width*height*sizeof(float));
  CHECK_PTR(scores_r);

  for(c=0;c<num_anchors_;c++) {
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        scores_r[j*width*num_anchors_+i*num_anchors_+c] = scores[c*width*height+j*width+i];
      }
    }
  }

  proposals = (float*) calloc(1, 4*num_anchors_*width*height*sizeof(float));
  CHECK_PTR(proposals);

  nkeep = bbox_transform_inv(anchors, bbox_deltas_r, proposals, scores_r,
                             K*num_anchors_, (float)imagew, (float)imageh, (float)min_size_);

  f2buf = (struct ff2ss*) calloc(1, nkeep*sizeof(struct ff2ss));
  CHECK_PTR(f2buf);

  for (i = 0; i < nkeep; ++i) {
    float * bbx = proposals+4*i;
    f2buf[i].score = scores_r[i];
    f2buf[i].bbx = bbx;
    f2buf[i].area = (bbx[2] - bbx[0] + 1) * (bbx[3] - bbx[1] + 1);
  }
  qsort(f2buf, (size_t)nkeep, sizeof(struct ff2ss), c_f2ss_compare);

  nkeep = MIN(nkeep, pre_nms_topN_);
  kp = (int*) calloc(1, sizeof(int)*post_nms_topN_);
  CHECK_PTR(kp);

  topN = c_nms(f2buf, nkeep, nms_thresh_, kp, post_nms_topN_);

  for (k = 0; k < topN; ++k)
  {
    int ti = kp[k];
    rois[k*4+0] = f2buf[ti].bbx[0];
    rois[k*4+1] = f2buf[ti].bbx[1];
    rois[k*4+2] = f2buf[ti].bbx[2];
    rois[k*4+3] = f2buf[ti].bbx[3];
  }

  *num_keep = topN;
exit:
  if(shifts)
    free(shifts);
  if(anchors)
    free(anchors);
  if(bbox_deltas_r)
    free(bbox_deltas_r);
  if(scores_r)
    free(scores_r);
  if(proposals)
    free(proposals);
  if(f2buf)
    free(f2buf);
  if(kp)
    free(kp);
  return res;
}

int ly_concat_channels_float(float* inputs, int iw, int ih,
                             int ichannels, float* outputs, int* ptrout)
{
  int res = MOK;
  int num = iw*ih*ichannels;

  float* ptrOut = outputs + *ptrout;
  memcpy(ptrOut, inputs, sizeof(float)*num);

  (*ptrout) += num;
  return res;
}


int ly_lrn_cross_float(const float* inputs, int iw, int ih, int ichannels,int ln,
                       float* outputs, int size, float alpha, float beta, float k)
{
  int res = MOK;
  int i = 0, c = 0;
  int count = 0;
  int count_ps = 0;
  float* scale_data = 0;
  float* padded_square = 0;
  float* ptrSqu = 0;
  float alpha_over_size = alpha / size;

  count = ichannels * iw * ih;
  scale_data = (float*) calloc(1, count * sizeof(float));
  CHECK_PTR(scale_data);

  for (i = 0; i < count; ++i) {
    scale_data[i] = k;
  }

  count_ps = (ichannels+size-1)*iw*ih;
  padded_square = (float*) calloc(1, count_ps * sizeof(float));
  CHECK_PTR(padded_square);
  memset(padded_square, 0, sizeof(float)*count_ps);

  ptrSqu = padded_square + (size-1)/2*iw*ih;
  for(i=0;i<count;i++)
    ptrSqu[i] = inputs[i]*inputs[i];

  // Create the first channel scale
  for (c = 0; c < size; ++c)
  {
    int N = iw*ih;
    ptrSqu = padded_square + c*iw*ih;

    for(i=0;i<N;i++) {
      scale_data[i] = alpha_over_size * ptrSqu[i] + scale_data[i];
    }
  }

  for (c = 1; c < ichannels; ++c)
  {
    int N = iw*ih;
    float* ptrScaleData = scale_data + c*iw*ih;

    // copy previous scale
    memcpy(ptrScaleData, ptrScaleData-iw*ih, sizeof(float) * N);

    // add new plane
    ptrSqu = padded_square + ( c + size - 1)*iw*ih;
    for(i=0;i<N;i++) {
      ptrScaleData[i] = alpha_over_size * ptrSqu[i] + ptrScaleData[i];
    }
    // subtract last plane
    ptrSqu = padded_square + ( c - 1)*iw*ih;
    for(i=0;i<N;i++) {
      ptrScaleData[i] = -alpha_over_size * ptrSqu[i] + ptrScaleData[i];
    }
  }

  // In the end, compute output
  for(i=0;i<count;i++)
  {
    outputs[i] = (float)pow(scale_data[i], -beta);
    outputs[i] = outputs[i] * inputs[i];
  }

exit:
  if(scale_data) free(scale_data);
  if(padded_square) free(padded_square);
  return res;
}

int ly_deconv_float(
		const float* inputs, int iw, int ih, int ichannels, int groups,
		const float* weights, const float* bias, int ww, int wh, int pad_w, int pad_h, int stepw, int steph,
		float* outputs, int ochannels)
{
	int res = MOK;
	int ow=0, oh=0;
	int conv_in_height_=0, conv_in_width_=0, conv_out_spatial_dim_=0;
	int kernel_dim_=0, weight_offset_=0, col_offset_=0, output_offset_=0;
	int size_colbuf=0;
	int conv_out_channels_=0, conv_in_channels_=0;
	int g=0, j=0, k=0;
	int M=0, N=0, K=0;

	float* col_buffer = 0;
	float* all1 = 0;

	oh = steph * (ih - 1) + wh - 2 * pad_h;
	ow = stepw * (iw - 1) + ww - 2 * pad_w;

	conv_in_height_ = oh;
	conv_in_width_ = ow;
	conv_out_spatial_dim_  = ih * iw;

	conv_out_channels_ = ichannels;
	conv_in_channels_ = ochannels;

	kernel_dim_ = conv_in_channels_ * wh * ww;
	weight_offset_ = conv_out_channels_ * kernel_dim_ / groups / groups;
	col_offset_ = kernel_dim_ * conv_out_spatial_dim_ / groups;
	output_offset_ = conv_out_channels_ * conv_out_spatial_dim_ / groups;

	size_colbuf = kernel_dim_ * ih * iw;
	col_buffer = (float*) calloc(1, size_colbuf * sizeof(float));
	CHECK_PTR(col_buffer);
	memset(col_buffer, 0, size_colbuf * sizeof(float));

	M = kernel_dim_ / groups;
	N = conv_out_spatial_dim_;
	K = conv_out_channels_ / groups;

	for (g = 0; g < groups; ++g) {
		convmul(inputs + output_offset_ * g, weights + weight_offset_ * g, col_buffer + col_offset_*g , M, N, K);
	}

	memset(outputs, 0, N*ochannels*sizeof(float));
	col2im_cpu(col_buffer, conv_in_channels_, conv_in_height_, conv_in_width_,
        wh, ww, pad_h, pad_w, steph, stepw, outputs);

	if(bias) {
		N = ow * oh;
		for(j=0;j<ochannels;j++)
		{
			for(k=0;k<N;k++)
			{
				outputs[j*N+k] += bias[j];
			}
		}
	}

exit:
	if( col_buffer ) free(col_buffer);
	if( all1 ) free(all1);
	return res;
}

