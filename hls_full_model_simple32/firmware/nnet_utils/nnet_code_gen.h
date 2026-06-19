#ifndef NNET_INSTR_GEN_H_
#define NNET_INSTR_GEN_H_

#include "nnet_conv1d_latency.h"
#include "nnet_helpers.h"

#include "hls_stream.h"
#include "nnet_common.h"
#include "nnet_function_stubs.h"
#include "nnet_mult.h"

namespace nnet {

template <class data_T, class res_T, typename CONFIG_T> class PointwiseConv1D {
  public:
    static void pointwise_conv(data_T data[CONFIG_T::in_width * CONFIG_T::n_chan],
                               res_T res[CONFIG_T::out_width * CONFIG_T::n_filt],
                               typename CONFIG_T::weight_t weights[CONFIG_T::n_chan * CONFIG_T::n_filt],
                               typename CONFIG_T::bias_t biases[CONFIG_T::n_filt]) {
        // To be implemented in subclasses
    }
};

// hls4ml insert code
template<class data_T, typename CONFIG_T>
class fill_buffer_3 : public nnet::FillConv2DBuffer<data_T, CONFIG_T> {
    public:
    static void fill_buffer(
        data_T data[CONFIG_T::in_height * CONFIG_T::in_width * CONFIG_T::n_chan],
        data_T buffer[CONFIG_T::n_pixels][CONFIG_T::filt_height * CONFIG_T::filt_width * CONFIG_T::n_chan],
        const unsigned partition
    ) {
        if (partition ==   0) {
            buffer[0][0] =    data[0]; buffer[0][1] =    data[1]; buffer[0][2] =    data[2]; buffer[0][3] =   data[16]; buffer[0][4] =   data[17]; buffer[0][5] =   data[18]; buffer[0][6] =   data[32]; buffer[0][7] =   data[33]; buffer[0][8] =   data[34];

        }
        if (partition ==   1) {
            buffer[0][0] =    data[2]; buffer[0][1] =    data[3]; buffer[0][2] =    data[4]; buffer[0][3] =   data[18]; buffer[0][4] =   data[19]; buffer[0][5] =   data[20]; buffer[0][6] =   data[34]; buffer[0][7] =   data[35]; buffer[0][8] =   data[36];

        }
        if (partition ==   2) {
            buffer[0][0] =    data[4]; buffer[0][1] =    data[5]; buffer[0][2] =    data[6]; buffer[0][3] =   data[20]; buffer[0][4] =   data[21]; buffer[0][5] =   data[22]; buffer[0][6] =   data[36]; buffer[0][7] =   data[37]; buffer[0][8] =   data[38];

        }
        if (partition ==   3) {
            buffer[0][0] =    data[6]; buffer[0][1] =    data[7]; buffer[0][2] =    data[8]; buffer[0][3] =   data[22]; buffer[0][4] =   data[23]; buffer[0][5] =   data[24]; buffer[0][6] =   data[38]; buffer[0][7] =   data[39]; buffer[0][8] =   data[40];

        }
        if (partition ==   4) {
            buffer[0][0] =    data[8]; buffer[0][1] =    data[9]; buffer[0][2] =   data[10]; buffer[0][3] =   data[24]; buffer[0][4] =   data[25]; buffer[0][5] =   data[26]; buffer[0][6] =   data[40]; buffer[0][7] =   data[41]; buffer[0][8] =   data[42];

        }
        if (partition ==   5) {
            buffer[0][0] =   data[10]; buffer[0][1] =   data[11]; buffer[0][2] =   data[12]; buffer[0][3] =   data[26]; buffer[0][4] =   data[27]; buffer[0][5] =   data[28]; buffer[0][6] =   data[42]; buffer[0][7] =   data[43]; buffer[0][8] =   data[44];

        }
        if (partition ==   6) {
            buffer[0][0] =   data[12]; buffer[0][1] =   data[13]; buffer[0][2] =   data[14]; buffer[0][3] =   data[28]; buffer[0][4] =   data[29]; buffer[0][5] =   data[30]; buffer[0][6] =   data[44]; buffer[0][7] =   data[45]; buffer[0][8] =   data[46];

        }
        if (partition ==   7) {
            buffer[0][0] =   data[14]; buffer[0][1] =   data[15]; buffer[0][2] =          0; buffer[0][3] =   data[30]; buffer[0][4] =   data[31]; buffer[0][5] =          0; buffer[0][6] =   data[46]; buffer[0][7] =   data[47]; buffer[0][8] =          0;

        }
        if (partition ==   8) {
            buffer[0][0] =   data[32]; buffer[0][1] =   data[33]; buffer[0][2] =   data[34]; buffer[0][3] =   data[48]; buffer[0][4] =   data[49]; buffer[0][5] =   data[50]; buffer[0][6] =   data[64]; buffer[0][7] =   data[65]; buffer[0][8] =   data[66];

        }
        if (partition ==   9) {
            buffer[0][0] =   data[34]; buffer[0][1] =   data[35]; buffer[0][2] =   data[36]; buffer[0][3] =   data[50]; buffer[0][4] =   data[51]; buffer[0][5] =   data[52]; buffer[0][6] =   data[66]; buffer[0][7] =   data[67]; buffer[0][8] =   data[68];

        }
        if (partition ==  10) {
            buffer[0][0] =   data[36]; buffer[0][1] =   data[37]; buffer[0][2] =   data[38]; buffer[0][3] =   data[52]; buffer[0][4] =   data[53]; buffer[0][5] =   data[54]; buffer[0][6] =   data[68]; buffer[0][7] =   data[69]; buffer[0][8] =   data[70];

        }
        if (partition ==  11) {
            buffer[0][0] =   data[38]; buffer[0][1] =   data[39]; buffer[0][2] =   data[40]; buffer[0][3] =   data[54]; buffer[0][4] =   data[55]; buffer[0][5] =   data[56]; buffer[0][6] =   data[70]; buffer[0][7] =   data[71]; buffer[0][8] =   data[72];

        }
        if (partition ==  12) {
            buffer[0][0] =   data[40]; buffer[0][1] =   data[41]; buffer[0][2] =   data[42]; buffer[0][3] =   data[56]; buffer[0][4] =   data[57]; buffer[0][5] =   data[58]; buffer[0][6] =   data[72]; buffer[0][7] =   data[73]; buffer[0][8] =   data[74];

        }
        if (partition ==  13) {
            buffer[0][0] =   data[42]; buffer[0][1] =   data[43]; buffer[0][2] =   data[44]; buffer[0][3] =   data[58]; buffer[0][4] =   data[59]; buffer[0][5] =   data[60]; buffer[0][6] =   data[74]; buffer[0][7] =   data[75]; buffer[0][8] =   data[76];

        }
        if (partition ==  14) {
            buffer[0][0] =   data[44]; buffer[0][1] =   data[45]; buffer[0][2] =   data[46]; buffer[0][3] =   data[60]; buffer[0][4] =   data[61]; buffer[0][5] =   data[62]; buffer[0][6] =   data[76]; buffer[0][7] =   data[77]; buffer[0][8] =   data[78];

        }
        if (partition ==  15) {
            buffer[0][0] =   data[46]; buffer[0][1] =   data[47]; buffer[0][2] =          0; buffer[0][3] =   data[62]; buffer[0][4] =   data[63]; buffer[0][5] =          0; buffer[0][6] =   data[78]; buffer[0][7] =   data[79]; buffer[0][8] =          0;

        }
        if (partition ==  16) {
            buffer[0][0] =   data[64]; buffer[0][1] =   data[65]; buffer[0][2] =   data[66]; buffer[0][3] =   data[80]; buffer[0][4] =   data[81]; buffer[0][5] =   data[82]; buffer[0][6] =   data[96]; buffer[0][7] =   data[97]; buffer[0][8] =   data[98];

        }
        if (partition ==  17) {
            buffer[0][0] =   data[66]; buffer[0][1] =   data[67]; buffer[0][2] =   data[68]; buffer[0][3] =   data[82]; buffer[0][4] =   data[83]; buffer[0][5] =   data[84]; buffer[0][6] =   data[98]; buffer[0][7] =   data[99]; buffer[0][8] =  data[100];

        }
        if (partition ==  18) {
            buffer[0][0] =   data[68]; buffer[0][1] =   data[69]; buffer[0][2] =   data[70]; buffer[0][3] =   data[84]; buffer[0][4] =   data[85]; buffer[0][5] =   data[86]; buffer[0][6] =  data[100]; buffer[0][7] =  data[101]; buffer[0][8] =  data[102];

        }
        if (partition ==  19) {
            buffer[0][0] =   data[70]; buffer[0][1] =   data[71]; buffer[0][2] =   data[72]; buffer[0][3] =   data[86]; buffer[0][4] =   data[87]; buffer[0][5] =   data[88]; buffer[0][6] =  data[102]; buffer[0][7] =  data[103]; buffer[0][8] =  data[104];

        }
        if (partition ==  20) {
            buffer[0][0] =   data[72]; buffer[0][1] =   data[73]; buffer[0][2] =   data[74]; buffer[0][3] =   data[88]; buffer[0][4] =   data[89]; buffer[0][5] =   data[90]; buffer[0][6] =  data[104]; buffer[0][7] =  data[105]; buffer[0][8] =  data[106];

        }
        if (partition ==  21) {
            buffer[0][0] =   data[74]; buffer[0][1] =   data[75]; buffer[0][2] =   data[76]; buffer[0][3] =   data[90]; buffer[0][4] =   data[91]; buffer[0][5] =   data[92]; buffer[0][6] =  data[106]; buffer[0][7] =  data[107]; buffer[0][8] =  data[108];

        }
        if (partition ==  22) {
            buffer[0][0] =   data[76]; buffer[0][1] =   data[77]; buffer[0][2] =   data[78]; buffer[0][3] =   data[92]; buffer[0][4] =   data[93]; buffer[0][5] =   data[94]; buffer[0][6] =  data[108]; buffer[0][7] =  data[109]; buffer[0][8] =  data[110];

        }
        if (partition ==  23) {
            buffer[0][0] =   data[78]; buffer[0][1] =   data[79]; buffer[0][2] =          0; buffer[0][3] =   data[94]; buffer[0][4] =   data[95]; buffer[0][5] =          0; buffer[0][6] =  data[110]; buffer[0][7] =  data[111]; buffer[0][8] =          0;

        }
        if (partition ==  24) {
            buffer[0][0] =   data[96]; buffer[0][1] =   data[97]; buffer[0][2] =   data[98]; buffer[0][3] =  data[112]; buffer[0][4] =  data[113]; buffer[0][5] =  data[114]; buffer[0][6] =  data[128]; buffer[0][7] =  data[129]; buffer[0][8] =  data[130];

        }
        if (partition ==  25) {
            buffer[0][0] =   data[98]; buffer[0][1] =   data[99]; buffer[0][2] =  data[100]; buffer[0][3] =  data[114]; buffer[0][4] =  data[115]; buffer[0][5] =  data[116]; buffer[0][6] =  data[130]; buffer[0][7] =  data[131]; buffer[0][8] =  data[132];

        }
        if (partition ==  26) {
            buffer[0][0] =  data[100]; buffer[0][1] =  data[101]; buffer[0][2] =  data[102]; buffer[0][3] =  data[116]; buffer[0][4] =  data[117]; buffer[0][5] =  data[118]; buffer[0][6] =  data[132]; buffer[0][7] =  data[133]; buffer[0][8] =  data[134];

        }
        if (partition ==  27) {
            buffer[0][0] =  data[102]; buffer[0][1] =  data[103]; buffer[0][2] =  data[104]; buffer[0][3] =  data[118]; buffer[0][4] =  data[119]; buffer[0][5] =  data[120]; buffer[0][6] =  data[134]; buffer[0][7] =  data[135]; buffer[0][8] =  data[136];

        }
        if (partition ==  28) {
            buffer[0][0] =  data[104]; buffer[0][1] =  data[105]; buffer[0][2] =  data[106]; buffer[0][3] =  data[120]; buffer[0][4] =  data[121]; buffer[0][5] =  data[122]; buffer[0][6] =  data[136]; buffer[0][7] =  data[137]; buffer[0][8] =  data[138];

        }
        if (partition ==  29) {
            buffer[0][0] =  data[106]; buffer[0][1] =  data[107]; buffer[0][2] =  data[108]; buffer[0][3] =  data[122]; buffer[0][4] =  data[123]; buffer[0][5] =  data[124]; buffer[0][6] =  data[138]; buffer[0][7] =  data[139]; buffer[0][8] =  data[140];

        }
        if (partition ==  30) {
            buffer[0][0] =  data[108]; buffer[0][1] =  data[109]; buffer[0][2] =  data[110]; buffer[0][3] =  data[124]; buffer[0][4] =  data[125]; buffer[0][5] =  data[126]; buffer[0][6] =  data[140]; buffer[0][7] =  data[141]; buffer[0][8] =  data[142];

        }
        if (partition ==  31) {
            buffer[0][0] =  data[110]; buffer[0][1] =  data[111]; buffer[0][2] =          0; buffer[0][3] =  data[126]; buffer[0][4] =  data[127]; buffer[0][5] =          0; buffer[0][6] =  data[142]; buffer[0][7] =  data[143]; buffer[0][8] =          0;

        }
        if (partition ==  32) {
            buffer[0][0] =  data[128]; buffer[0][1] =  data[129]; buffer[0][2] =  data[130]; buffer[0][3] =  data[144]; buffer[0][4] =  data[145]; buffer[0][5] =  data[146]; buffer[0][6] =  data[160]; buffer[0][7] =  data[161]; buffer[0][8] =  data[162];

        }
        if (partition ==  33) {
            buffer[0][0] =  data[130]; buffer[0][1] =  data[131]; buffer[0][2] =  data[132]; buffer[0][3] =  data[146]; buffer[0][4] =  data[147]; buffer[0][5] =  data[148]; buffer[0][6] =  data[162]; buffer[0][7] =  data[163]; buffer[0][8] =  data[164];

        }
        if (partition ==  34) {
            buffer[0][0] =  data[132]; buffer[0][1] =  data[133]; buffer[0][2] =  data[134]; buffer[0][3] =  data[148]; buffer[0][4] =  data[149]; buffer[0][5] =  data[150]; buffer[0][6] =  data[164]; buffer[0][7] =  data[165]; buffer[0][8] =  data[166];

        }
        if (partition ==  35) {
            buffer[0][0] =  data[134]; buffer[0][1] =  data[135]; buffer[0][2] =  data[136]; buffer[0][3] =  data[150]; buffer[0][4] =  data[151]; buffer[0][5] =  data[152]; buffer[0][6] =  data[166]; buffer[0][7] =  data[167]; buffer[0][8] =  data[168];

        }
        if (partition ==  36) {
            buffer[0][0] =  data[136]; buffer[0][1] =  data[137]; buffer[0][2] =  data[138]; buffer[0][3] =  data[152]; buffer[0][4] =  data[153]; buffer[0][5] =  data[154]; buffer[0][6] =  data[168]; buffer[0][7] =  data[169]; buffer[0][8] =  data[170];

        }
        if (partition ==  37) {
            buffer[0][0] =  data[138]; buffer[0][1] =  data[139]; buffer[0][2] =  data[140]; buffer[0][3] =  data[154]; buffer[0][4] =  data[155]; buffer[0][5] =  data[156]; buffer[0][6] =  data[170]; buffer[0][7] =  data[171]; buffer[0][8] =  data[172];

        }
        if (partition ==  38) {
            buffer[0][0] =  data[140]; buffer[0][1] =  data[141]; buffer[0][2] =  data[142]; buffer[0][3] =  data[156]; buffer[0][4] =  data[157]; buffer[0][5] =  data[158]; buffer[0][6] =  data[172]; buffer[0][7] =  data[173]; buffer[0][8] =  data[174];

        }
        if (partition ==  39) {
            buffer[0][0] =  data[142]; buffer[0][1] =  data[143]; buffer[0][2] =          0; buffer[0][3] =  data[158]; buffer[0][4] =  data[159]; buffer[0][5] =          0; buffer[0][6] =  data[174]; buffer[0][7] =  data[175]; buffer[0][8] =          0;

        }
        if (partition ==  40) {
            buffer[0][0] =  data[160]; buffer[0][1] =  data[161]; buffer[0][2] =  data[162]; buffer[0][3] =  data[176]; buffer[0][4] =  data[177]; buffer[0][5] =  data[178]; buffer[0][6] =  data[192]; buffer[0][7] =  data[193]; buffer[0][8] =  data[194];

        }
        if (partition ==  41) {
            buffer[0][0] =  data[162]; buffer[0][1] =  data[163]; buffer[0][2] =  data[164]; buffer[0][3] =  data[178]; buffer[0][4] =  data[179]; buffer[0][5] =  data[180]; buffer[0][6] =  data[194]; buffer[0][7] =  data[195]; buffer[0][8] =  data[196];

        }
        if (partition ==  42) {
            buffer[0][0] =  data[164]; buffer[0][1] =  data[165]; buffer[0][2] =  data[166]; buffer[0][3] =  data[180]; buffer[0][4] =  data[181]; buffer[0][5] =  data[182]; buffer[0][6] =  data[196]; buffer[0][7] =  data[197]; buffer[0][8] =  data[198];

        }
        if (partition ==  43) {
            buffer[0][0] =  data[166]; buffer[0][1] =  data[167]; buffer[0][2] =  data[168]; buffer[0][3] =  data[182]; buffer[0][4] =  data[183]; buffer[0][5] =  data[184]; buffer[0][6] =  data[198]; buffer[0][7] =  data[199]; buffer[0][8] =  data[200];

        }
        if (partition ==  44) {
            buffer[0][0] =  data[168]; buffer[0][1] =  data[169]; buffer[0][2] =  data[170]; buffer[0][3] =  data[184]; buffer[0][4] =  data[185]; buffer[0][5] =  data[186]; buffer[0][6] =  data[200]; buffer[0][7] =  data[201]; buffer[0][8] =  data[202];

        }
        if (partition ==  45) {
            buffer[0][0] =  data[170]; buffer[0][1] =  data[171]; buffer[0][2] =  data[172]; buffer[0][3] =  data[186]; buffer[0][4] =  data[187]; buffer[0][5] =  data[188]; buffer[0][6] =  data[202]; buffer[0][7] =  data[203]; buffer[0][8] =  data[204];

        }
        if (partition ==  46) {
            buffer[0][0] =  data[172]; buffer[0][1] =  data[173]; buffer[0][2] =  data[174]; buffer[0][3] =  data[188]; buffer[0][4] =  data[189]; buffer[0][5] =  data[190]; buffer[0][6] =  data[204]; buffer[0][7] =  data[205]; buffer[0][8] =  data[206];

        }
        if (partition ==  47) {
            buffer[0][0] =  data[174]; buffer[0][1] =  data[175]; buffer[0][2] =          0; buffer[0][3] =  data[190]; buffer[0][4] =  data[191]; buffer[0][5] =          0; buffer[0][6] =  data[206]; buffer[0][7] =  data[207]; buffer[0][8] =          0;

        }
        if (partition ==  48) {
            buffer[0][0] =  data[192]; buffer[0][1] =  data[193]; buffer[0][2] =  data[194]; buffer[0][3] =  data[208]; buffer[0][4] =  data[209]; buffer[0][5] =  data[210]; buffer[0][6] =  data[224]; buffer[0][7] =  data[225]; buffer[0][8] =  data[226];

        }
        if (partition ==  49) {
            buffer[0][0] =  data[194]; buffer[0][1] =  data[195]; buffer[0][2] =  data[196]; buffer[0][3] =  data[210]; buffer[0][4] =  data[211]; buffer[0][5] =  data[212]; buffer[0][6] =  data[226]; buffer[0][7] =  data[227]; buffer[0][8] =  data[228];

        }
        if (partition ==  50) {
            buffer[0][0] =  data[196]; buffer[0][1] =  data[197]; buffer[0][2] =  data[198]; buffer[0][3] =  data[212]; buffer[0][4] =  data[213]; buffer[0][5] =  data[214]; buffer[0][6] =  data[228]; buffer[0][7] =  data[229]; buffer[0][8] =  data[230];

        }
        if (partition ==  51) {
            buffer[0][0] =  data[198]; buffer[0][1] =  data[199]; buffer[0][2] =  data[200]; buffer[0][3] =  data[214]; buffer[0][4] =  data[215]; buffer[0][5] =  data[216]; buffer[0][6] =  data[230]; buffer[0][7] =  data[231]; buffer[0][8] =  data[232];

        }
        if (partition ==  52) {
            buffer[0][0] =  data[200]; buffer[0][1] =  data[201]; buffer[0][2] =  data[202]; buffer[0][3] =  data[216]; buffer[0][4] =  data[217]; buffer[0][5] =  data[218]; buffer[0][6] =  data[232]; buffer[0][7] =  data[233]; buffer[0][8] =  data[234];

        }
        if (partition ==  53) {
            buffer[0][0] =  data[202]; buffer[0][1] =  data[203]; buffer[0][2] =  data[204]; buffer[0][3] =  data[218]; buffer[0][4] =  data[219]; buffer[0][5] =  data[220]; buffer[0][6] =  data[234]; buffer[0][7] =  data[235]; buffer[0][8] =  data[236];

        }
        if (partition ==  54) {
            buffer[0][0] =  data[204]; buffer[0][1] =  data[205]; buffer[0][2] =  data[206]; buffer[0][3] =  data[220]; buffer[0][4] =  data[221]; buffer[0][5] =  data[222]; buffer[0][6] =  data[236]; buffer[0][7] =  data[237]; buffer[0][8] =  data[238];

        }
        if (partition ==  55) {
            buffer[0][0] =  data[206]; buffer[0][1] =  data[207]; buffer[0][2] =          0; buffer[0][3] =  data[222]; buffer[0][4] =  data[223]; buffer[0][5] =          0; buffer[0][6] =  data[238]; buffer[0][7] =  data[239]; buffer[0][8] =          0;

        }
        if (partition ==  56) {
            buffer[0][0] =  data[224]; buffer[0][1] =  data[225]; buffer[0][2] =  data[226]; buffer[0][3] =  data[240]; buffer[0][4] =  data[241]; buffer[0][5] =  data[242]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  57) {
            buffer[0][0] =  data[226]; buffer[0][1] =  data[227]; buffer[0][2] =  data[228]; buffer[0][3] =  data[242]; buffer[0][4] =  data[243]; buffer[0][5] =  data[244]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  58) {
            buffer[0][0] =  data[228]; buffer[0][1] =  data[229]; buffer[0][2] =  data[230]; buffer[0][3] =  data[244]; buffer[0][4] =  data[245]; buffer[0][5] =  data[246]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  59) {
            buffer[0][0] =  data[230]; buffer[0][1] =  data[231]; buffer[0][2] =  data[232]; buffer[0][3] =  data[246]; buffer[0][4] =  data[247]; buffer[0][5] =  data[248]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  60) {
            buffer[0][0] =  data[232]; buffer[0][1] =  data[233]; buffer[0][2] =  data[234]; buffer[0][3] =  data[248]; buffer[0][4] =  data[249]; buffer[0][5] =  data[250]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  61) {
            buffer[0][0] =  data[234]; buffer[0][1] =  data[235]; buffer[0][2] =  data[236]; buffer[0][3] =  data[250]; buffer[0][4] =  data[251]; buffer[0][5] =  data[252]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  62) {
            buffer[0][0] =  data[236]; buffer[0][1] =  data[237]; buffer[0][2] =  data[238]; buffer[0][3] =  data[252]; buffer[0][4] =  data[253]; buffer[0][5] =  data[254]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  63) {
            buffer[0][0] =  data[238]; buffer[0][1] =  data[239]; buffer[0][2] =          0; buffer[0][3] =  data[254]; buffer[0][4] =  data[255]; buffer[0][5] =          0; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
    }
};
template<class data_T, typename CONFIG_T>
class fill_buffer_5 : public nnet::FillConv2DBuffer<data_T, CONFIG_T> {
    public:
    static void fill_buffer(
        data_T data[CONFIG_T::in_height * CONFIG_T::in_width * CONFIG_T::n_chan],
        data_T buffer[CONFIG_T::n_pixels][CONFIG_T::filt_height * CONFIG_T::filt_width * CONFIG_T::n_chan],
        const unsigned partition
    ) {
        if (partition ==   0) {
            buffer[0][0] =    data[0]; buffer[0][1] =    data[1]; buffer[0][2] =    data[2]; buffer[0][3] =    data[8]; buffer[0][4] =    data[9]; buffer[0][5] =   data[10]; buffer[0][6] =   data[16]; buffer[0][7] =   data[17]; buffer[0][8] =   data[18];

        }
        if (partition ==   1) {
            buffer[0][0] =    data[2]; buffer[0][1] =    data[3]; buffer[0][2] =    data[4]; buffer[0][3] =   data[10]; buffer[0][4] =   data[11]; buffer[0][5] =   data[12]; buffer[0][6] =   data[18]; buffer[0][7] =   data[19]; buffer[0][8] =   data[20];

        }
        if (partition ==   2) {
            buffer[0][0] =    data[4]; buffer[0][1] =    data[5]; buffer[0][2] =    data[6]; buffer[0][3] =   data[12]; buffer[0][4] =   data[13]; buffer[0][5] =   data[14]; buffer[0][6] =   data[20]; buffer[0][7] =   data[21]; buffer[0][8] =   data[22];

        }
        if (partition ==   3) {
            buffer[0][0] =    data[6]; buffer[0][1] =    data[7]; buffer[0][2] =          0; buffer[0][3] =   data[14]; buffer[0][4] =   data[15]; buffer[0][5] =          0; buffer[0][6] =   data[22]; buffer[0][7] =   data[23]; buffer[0][8] =          0;

        }
        if (partition ==   4) {
            buffer[0][0] =   data[16]; buffer[0][1] =   data[17]; buffer[0][2] =   data[18]; buffer[0][3] =   data[24]; buffer[0][4] =   data[25]; buffer[0][5] =   data[26]; buffer[0][6] =   data[32]; buffer[0][7] =   data[33]; buffer[0][8] =   data[34];

        }
        if (partition ==   5) {
            buffer[0][0] =   data[18]; buffer[0][1] =   data[19]; buffer[0][2] =   data[20]; buffer[0][3] =   data[26]; buffer[0][4] =   data[27]; buffer[0][5] =   data[28]; buffer[0][6] =   data[34]; buffer[0][7] =   data[35]; buffer[0][8] =   data[36];

        }
        if (partition ==   6) {
            buffer[0][0] =   data[20]; buffer[0][1] =   data[21]; buffer[0][2] =   data[22]; buffer[0][3] =   data[28]; buffer[0][4] =   data[29]; buffer[0][5] =   data[30]; buffer[0][6] =   data[36]; buffer[0][7] =   data[37]; buffer[0][8] =   data[38];

        }
        if (partition ==   7) {
            buffer[0][0] =   data[22]; buffer[0][1] =   data[23]; buffer[0][2] =          0; buffer[0][3] =   data[30]; buffer[0][4] =   data[31]; buffer[0][5] =          0; buffer[0][6] =   data[38]; buffer[0][7] =   data[39]; buffer[0][8] =          0;

        }
        if (partition ==   8) {
            buffer[0][0] =   data[32]; buffer[0][1] =   data[33]; buffer[0][2] =   data[34]; buffer[0][3] =   data[40]; buffer[0][4] =   data[41]; buffer[0][5] =   data[42]; buffer[0][6] =   data[48]; buffer[0][7] =   data[49]; buffer[0][8] =   data[50];

        }
        if (partition ==   9) {
            buffer[0][0] =   data[34]; buffer[0][1] =   data[35]; buffer[0][2] =   data[36]; buffer[0][3] =   data[42]; buffer[0][4] =   data[43]; buffer[0][5] =   data[44]; buffer[0][6] =   data[50]; buffer[0][7] =   data[51]; buffer[0][8] =   data[52];

        }
        if (partition ==  10) {
            buffer[0][0] =   data[36]; buffer[0][1] =   data[37]; buffer[0][2] =   data[38]; buffer[0][3] =   data[44]; buffer[0][4] =   data[45]; buffer[0][5] =   data[46]; buffer[0][6] =   data[52]; buffer[0][7] =   data[53]; buffer[0][8] =   data[54];

        }
        if (partition ==  11) {
            buffer[0][0] =   data[38]; buffer[0][1] =   data[39]; buffer[0][2] =          0; buffer[0][3] =   data[46]; buffer[0][4] =   data[47]; buffer[0][5] =          0; buffer[0][6] =   data[54]; buffer[0][7] =   data[55]; buffer[0][8] =          0;

        }
        if (partition ==  12) {
            buffer[0][0] =   data[48]; buffer[0][1] =   data[49]; buffer[0][2] =   data[50]; buffer[0][3] =   data[56]; buffer[0][4] =   data[57]; buffer[0][5] =   data[58]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  13) {
            buffer[0][0] =   data[50]; buffer[0][1] =   data[51]; buffer[0][2] =   data[52]; buffer[0][3] =   data[58]; buffer[0][4] =   data[59]; buffer[0][5] =   data[60]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  14) {
            buffer[0][0] =   data[52]; buffer[0][1] =   data[53]; buffer[0][2] =   data[54]; buffer[0][3] =   data[60]; buffer[0][4] =   data[61]; buffer[0][5] =   data[62]; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
        if (partition ==  15) {
            buffer[0][0] =   data[54]; buffer[0][1] =   data[55]; buffer[0][2] =          0; buffer[0][3] =   data[62]; buffer[0][4] =   data[63]; buffer[0][5] =          0; buffer[0][6] =          0; buffer[0][7] =          0; buffer[0][8] =          0;

        }
    }
};

} // namespace nnet

#endif
