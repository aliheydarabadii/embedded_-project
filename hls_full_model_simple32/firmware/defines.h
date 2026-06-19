#ifndef DEFINES_H_
#define DEFINES_H_

#include "ap_fixed.h"
#include "ap_int.h"
#include "nnet_utils/nnet_types.h"
#include <array>
#include <cstddef>
#include <cstdio>
#include <tuple>
#include <tuple>


// hls-fpga-machine-learning insert numbers

// hls-fpga-machine-learning insert layer-precision
typedef ap_fixed<12,4> input_t;
typedef ap_fixed<12,4> average_pooling2d_accum_t;
typedef ap_fixed<12,4> layer2_t;
typedef ap_fixed<16,5> conv2d_accum_t;
typedef ap_fixed<16,5> layer3_t;
typedef ap_fixed<16,5> conv2d_weight_t;
typedef ap_fixed<16,5> conv2d_bias_t;
typedef ap_fixed<14,4> layer4_t;
typedef ap_fixed<18,8> conv2d_relu_table_t;
typedef ap_fixed<16,5> conv2d_1_accum_t;
typedef ap_fixed<16,5> layer5_t;
typedef ap_fixed<16,5> conv2d_1_weight_t;
typedef ap_fixed<16,5> conv2d_1_bias_t;
typedef ap_fixed<12,4> layer6_t;
typedef ap_fixed<18,8> conv2d_1_relu_table_t;
typedef ap_fixed<20,6> average_pooling2d_1_accum_t;
typedef ap_fixed<20,6> layer7_t;
typedef ap_fixed<14,4> dense_accum_t;
typedef ap_fixed<14,4> layer9_t;
typedef ap_fixed<14,4> dense_weight_t;
typedef ap_fixed<14,4> dense_bias_t;
typedef ap_uint<1> layer9_index;
typedef ap_fixed<12,4> result_t;
typedef ap_fixed<18,8> dense_sigmoid_table_t;

// hls-fpga-machine-learning insert emulator-defines


#endif
