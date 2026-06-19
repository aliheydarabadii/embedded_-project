`timescale 1ns / 1ps

module cnn_system_tb;

reg         clk;
reg         reset_n;
reg         start;
wire [9:0]  bram_addr;
wire [11:0] bram_read_data;
wire        cnn_start;
wire [11:0] cnn_input_data;
wire        cnn_input_valid;
wire        cnn_input_read;
wire [11:0] cnn_output_data;
wire        cnn_output_write;
wire        cnn_done;
wire        detection_valid;
wire [11:0] detection_result;

integer i;
integer cycles;
integer input_reads;

input_bram bram (
    .write_clk  (clk),
    .read_clk   (clk),
    .write_en   (1'b0),
    .write_addr (10'd0),
    .write_data (12'd0),
    .read_addr  (bram_addr),
    .read_data  (bram_read_data)
);

cnn_wrapper wrapper (
    .clk              (clk),
    .reset_n          (reset_n),
    .start            (start),
    .bram_addr        (bram_addr),
    .bram_read_data   (bram_read_data),
    .cnn_start        (cnn_start),
    .cnn_input_data   (cnn_input_data),
    .cnn_input_valid  (cnn_input_valid),
    .cnn_input_read   (cnn_input_read),
    .cnn_output_data  (cnn_output_data),
    .cnn_output_write (cnn_output_write),
    .detection_valid  (detection_valid),
    .detection_result (detection_result)
);

cnn_top cnn (
    .clock              (clk),
    .reset              (reset_n),
    .start_port         (cnn_start),
    .input_1_dout       (cnn_input_data),
    .input_1_empty_n    (cnn_input_valid),
    .layer10_out_full_n (1'b1),
    .done_port          (cnn_done),
    .input_1_read       (cnn_input_read),
    .layer10_out_din    (cnn_output_data),
    .layer10_out_write  (cnn_output_write)
);

always #5 clk = ~clk;

always @(posedge clk) begin
    if (reset_n) begin
        cycles = cycles + 1;

        if (cnn_input_read)
            input_reads = input_reads + 1;

        if (detection_valid) begin
            $display("PASS: CNN result 0x%03x after %0d cycles with %0d reads",
                     detection_result, cycles, input_reads);
            $finish;
        end

        if (cycles > 1000000) begin
            $display("FAIL: CNN result timeout after %0d cycles with %0d reads",
                     cycles, input_reads);
            $finish;
        end
    end
end

initial begin
    clk = 1'b0;
    reset_n = 1'b0;
    start = 1'b0;
    cycles = 0;
    input_reads = 0;

    for (i = 0; i < 1024; i = i + 1)
        bram.ram[i] = 12'h000;

    repeat (3) @(negedge clk);
    reset_n = 1'b1;
    @(negedge clk);
    start = 1'b1;
    @(negedge clk);
    start = 1'b0;
end

endmodule
