#include <iostream>

#include "myproject.h"
#include "parameters.h"


void myproject(
    input_t input_1[32*32*1],
    result_t layer10_out[1]
) {

    // hls-fpga-machine-learning insert IO
    //#pragma HLS ARRAY_RESHAPE variable=input_1 complete dim=0
    //#pragma HLS ARRAY_PARTITION variable=layer10_out complete dim=0
    #pragma HLS interface mode=fifo port=input_1
    #pragma HLS interface mode=fifo port=layer10_out
    //#pragma HLS DATAFLOW

    // hls-fpga-machine-learning insert load weights
#ifndef __BAMBU__
    static bool loaded_weights = false;
    if (!loaded_weights) {
        nnet::load_weights_from_txt<conv2d_weight_t, 9>(w3, "w3.txt");
        nnet::load_weights_from_txt<conv2d_bias_t, 1>(b3, "b3.txt");
        nnet::load_weights_from_txt<conv2d_1_weight_t, 18>(w5, "w5.txt");
        nnet::load_weights_from_txt<conv2d_1_bias_t, 2>(b5, "b5.txt");
        nnet::load_weights_from_txt<dense_weight_t, 2>(w9, "w9.txt");
        nnet::load_weights_from_txt<dense_bias_t, 1>(b9, "b9.txt");
        loaded_weights = true;    }
#endif
    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    // hls-fpga-machine-learning insert layers

    layer2_t layer2_out[16*16*1];
    //#pragma HLS ARRAY_PARTITION variable=layer2_out complete dim=0

    layer3_t layer3_out[8*8*1];
    //#pragma HLS ARRAY_PARTITION variable=layer3_out complete dim=0

    layer4_t layer4_out[8*8*1];
    //#pragma HLS ARRAY_PARTITION variable=layer4_out complete dim=0

    layer5_t layer5_out[4*4*2];
    //#pragma HLS ARRAY_PARTITION variable=layer5_out complete dim=0

    layer6_t layer6_out[4*4*2];
    //#pragma HLS ARRAY_PARTITION variable=layer6_out complete dim=0

    layer7_t layer7_out[1*1*2];
    //#pragma HLS ARRAY_PARTITION variable=layer7_out complete dim=0

    auto& layer8_out = layer7_out;
    layer9_t layer9_out[1];
    //#pragma HLS ARRAY_PARTITION variable=layer9_out complete dim=0

    nnet::pooling2d_cl<input_t, layer2_t, config2>(input_1, layer2_out); // average_pooling2d

    nnet::conv_2d_cl<layer2_t, layer3_t, config3>(layer2_out, layer3_out, w3, b3); // conv2d

    nnet::relu<layer3_t, layer4_t, relu_config4>(layer3_out, layer4_out); // conv2d_relu

    nnet::conv_2d_cl<layer4_t, layer5_t, config5>(layer4_out, layer5_out, w5, b5); // conv2d_1

    nnet::relu<layer5_t, layer6_t, relu_config6>(layer5_out, layer6_out); // conv2d_1_relu

    nnet::pooling2d_cl<layer6_t, layer7_t, config7>(layer6_out, layer7_out); // average_pooling2d_1

    nnet::dense<layer7_t, layer9_t, config9>(layer8_out, layer9_out, w9, b9); // dense

    nnet::sigmoid<layer9_t, result_t, sigmoid_config10>(layer9_out, layer10_out); // dense_sigmoid

}

