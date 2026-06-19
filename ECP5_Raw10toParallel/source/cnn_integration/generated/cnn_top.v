`default_nettype none

module cnn_top (
    input  wire        clock,
    input  wire        reset,
    input  wire        start_port,
    input  wire [11:0] input_1_dout,
    input  wire        input_1_empty_n,
    input  wire        layer10_out_full_n,
    output wire        done_port,
    output wire        input_1_read,
    output wire [11:0] layer10_out_din,
    output wire        layer10_out_write
);

    p_Z9myprojectP8ac_fixedILi12ELi4ELb1EL9ac_q_mode0EL9ac_o_mode0EES3_s u_cnn (
        .clock              (clock),
        .reset              (reset),
        .start_port         (start_port),
        .input_1_dout       (input_1_dout),
        .input_1_empty_n    (input_1_empty_n),
        .layer10_out_full_n (layer10_out_full_n),
        .done_port          (done_port),
        .input_1_read       (input_1_read),
        .layer10_out_din    (layer10_out_din),
        .layer10_out_write  (layer10_out_write)
    );

endmodule

`default_nettype wire
