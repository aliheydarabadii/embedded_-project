#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "ap_fixed.h"
#include "ap_int.h"

#include "nnet_utils/nnet_code_gen.h"
#include "nnet_utils/nnet_helpers.h"
// hls-fpga-machine-learning insert includes
#include "nnet_utils/nnet_activation.h"
#include "nnet_utils/nnet_activation_stream.h"
#include "nnet_utils/nnet_conv2d.h"
#include "nnet_utils/nnet_conv2d_stream.h"
#include "nnet_utils/nnet_dense.h"
#include "nnet_utils/nnet_dense_compressed.h"
#include "nnet_utils/nnet_dense_stream.h"
#include "nnet_utils/nnet_pooling.h"
#include "nnet_utils/nnet_pooling_stream.h"

// hls-fpga-machine-learning insert weights
#include "weights/w3.h"
#include "weights/b3.h"
#include "weights/w5.h"
#include "weights/b5.h"
#include "weights/w9.h"
#include "weights/b9.h"


// hls-fpga-machine-learning insert layer-config
// average_pooling2d
struct config2 : nnet::pooling2d_config {
    static const unsigned in_height = 32;
    static const unsigned in_width = 32;
    static const unsigned n_filt = 1;
    static const unsigned stride_height = 2;
    static const unsigned stride_width = 2;
    static const unsigned pool_height = 2;
    static const unsigned pool_width = 2;

    static const unsigned filt_height = pool_height;
    static const unsigned filt_width = pool_width;
    static const unsigned n_chan = n_filt;

    static const unsigned out_height = 16;
    static const unsigned out_width = 16;
    static const unsigned pad_top = 0;
    static const unsigned pad_bottom = 0;
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const bool count_pad = false;
    static const nnet::Pool_Op pool_op = nnet::Average;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned reuse_factor = 1;
    typedef average_pooling2d_accum_t accum_t;
};

// conv2d
struct config3_mult : nnet::dense_config {
    static const unsigned n_in = 9;
    static const unsigned n_out = 1;
    static const unsigned reuse_factor = 1;
    static const unsigned strategy = nnet::resource;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    typedef conv2d_accum_t accum_t;
    typedef conv2d_bias_t bias_t;
    typedef conv2d_weight_t weight_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseResource_rf_leq_nin<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

struct config3 : nnet::conv2d_config {
    static const unsigned pad_top = 0;
    static const unsigned pad_bottom = 1;
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 1;
    static const unsigned in_height = 16;
    static const unsigned in_width = 16;
    static const unsigned n_chan = 1;
    static const unsigned filt_height = 3;
    static const unsigned filt_width = 3;
    static const unsigned kernel_size = filt_height * filt_width;
    static const unsigned n_filt = 1;
    static const unsigned stride_height = 2;
    static const unsigned stride_width = 2;
    static const unsigned out_height = 8;
    static const unsigned out_width = 8;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit =
        DIV_ROUNDUP(kernel_size * n_chan * n_filt, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    static const unsigned strategy = nnet::resource;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned min_height = 16;
    static const unsigned min_width = 16;
    static const ap_uint<filt_height * filt_width> pixels[min_height * min_width];
    static const unsigned n_partitions = 64;
    static const unsigned n_pixels = out_height * out_width / n_partitions;
    template<class data_T, class CONFIG_T>
    using fill_buffer = nnet::fill_buffer_3<data_T, CONFIG_T>;
    typedef conv2d_accum_t accum_t;
    typedef conv2d_bias_t bias_t;
    typedef conv2d_weight_t weight_t;
    typedef config3_mult mult_config;
    template<unsigned K, unsigned S, unsigned W>
    using scale_index_height = nnet::scale_index_regular<K, S, W>;
    template<unsigned K, unsigned S, unsigned W>
    using scale_index_width = nnet::scale_index_regular<K, S, W>;
};
const ap_uint<config3::filt_height * config3::filt_width> config3::pixels[] = {ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0),ap_uint<config3::filt_height * config3::filt_width>(0)};

// conv2d_relu
struct relu_config4 : nnet::activ_config {
    static const unsigned n_in = 64;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_parallel;
    static const unsigned reuse_factor = 1;
    typedef conv2d_relu_table_t table_t;
};

// conv2d_1
struct config5_mult : nnet::dense_config {
    static const unsigned n_in = 9;
    static const unsigned n_out = 2;
    static const unsigned reuse_factor = 1;
    static const unsigned strategy = nnet::resource;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    typedef conv2d_1_accum_t accum_t;
    typedef conv2d_1_bias_t bias_t;
    typedef conv2d_1_weight_t weight_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseResource_rf_leq_nin<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

struct config5 : nnet::conv2d_config {
    static const unsigned pad_top = 0;
    static const unsigned pad_bottom = 1;
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 1;
    static const unsigned in_height = 8;
    static const unsigned in_width = 8;
    static const unsigned n_chan = 1;
    static const unsigned filt_height = 3;
    static const unsigned filt_width = 3;
    static const unsigned kernel_size = filt_height * filt_width;
    static const unsigned n_filt = 2;
    static const unsigned stride_height = 2;
    static const unsigned stride_width = 2;
    static const unsigned out_height = 4;
    static const unsigned out_width = 4;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit =
        DIV_ROUNDUP(kernel_size * n_chan * n_filt, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    static const unsigned strategy = nnet::resource;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned min_height = 8;
    static const unsigned min_width = 8;
    static const ap_uint<filt_height * filt_width> pixels[min_height * min_width];
    static const unsigned n_partitions = 16;
    static const unsigned n_pixels = out_height * out_width / n_partitions;
    template<class data_T, class CONFIG_T>
    using fill_buffer = nnet::fill_buffer_5<data_T, CONFIG_T>;
    typedef conv2d_1_accum_t accum_t;
    typedef conv2d_1_bias_t bias_t;
    typedef conv2d_1_weight_t weight_t;
    typedef config5_mult mult_config;
    template<unsigned K, unsigned S, unsigned W>
    using scale_index_height = nnet::scale_index_regular<K, S, W>;
    template<unsigned K, unsigned S, unsigned W>
    using scale_index_width = nnet::scale_index_regular<K, S, W>;
};
const ap_uint<config5::filt_height * config5::filt_width> config5::pixels[] = {ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0),ap_uint<config5::filt_height * config5::filt_width>(0)};

// conv2d_1_relu
struct relu_config6 : nnet::activ_config {
    static const unsigned n_in = 32;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_parallel;
    static const unsigned reuse_factor = 1;
    typedef conv2d_1_relu_table_t table_t;
};

// average_pooling2d_1
struct config7 : nnet::pooling2d_config {
    static const unsigned in_height = 4;
    static const unsigned in_width = 4;
    static const unsigned n_filt = 2;
    static const unsigned stride_height = 4;
    static const unsigned stride_width = 4;
    static const unsigned pool_height = 4;
    static const unsigned pool_width = 4;

    static const unsigned filt_height = pool_height;
    static const unsigned filt_width = pool_width;
    static const unsigned n_chan = n_filt;

    static const unsigned out_height = 1;
    static const unsigned out_width = 1;
    static const unsigned pad_top = 0;
    static const unsigned pad_bottom = 0;
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const bool count_pad = false;
    static const nnet::Pool_Op pool_op = nnet::Average;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned reuse_factor = 1;
    typedef average_pooling2d_1_accum_t accum_t;
};

// dense
struct config9 : nnet::dense_config {
    static const unsigned n_in = 2;
    static const unsigned n_out = 1;
    static const unsigned io_type = nnet::io_parallel;
    static const unsigned strategy = nnet::resource;
    static const unsigned reuse_factor = 1;
    static const unsigned n_zeros = 0;
    static const unsigned n_nonzeros = 2;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    typedef dense_accum_t accum_t;
    typedef dense_bias_t bias_t;
    typedef dense_weight_t weight_t;
    typedef layer9_index index_t;
    template<class data_T, class res_T, class CONFIG_T>
    using kernel = nnet::DenseResource_rf_leq_nin<data_T, res_T, CONFIG_T>;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

// dense_sigmoid
struct sigmoid_config10 : nnet::activ_config {
    static const unsigned n_in = 1;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_parallel;
    static const unsigned reuse_factor = 1;
    typedef dense_sigmoid_table_t table_t;
};



#endif
