`timescale 1ns / 1ps

module cnn_wrapper_tb;

reg         clk;
reg         reset_n;
reg         start;
reg         cnn_output_write;
reg  [11:0] cnn_output_data;

wire [9:0]  bram_addr;
wire [11:0] bram_read_data;
wire        cnn_start;
wire [11:0] cnn_input_data;
wire        cnn_input_valid;
wire        cnn_input_read;
wire        detection_valid;
wire [11:0] detection_result;

integer i;
integer consumed;

assign cnn_input_read = cnn_input_valid;

input_bram bram (
    .write_clk  (clk),
    .read_clk   (clk),
    .write_en   (1'b0),
    .write_addr (10'd0),
    .write_data (12'd0),
    .read_addr  (bram_addr),
    .read_data  (bram_read_data)
);

cnn_wrapper dut (
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

always #5 clk = ~clk;

always @(posedge clk) begin
    if (cnn_input_read) begin
        if (cnn_input_data !== consumed[11:0]) begin
            $display("FAIL: sample %0d had value %0d", consumed,
                     cnn_input_data);
            $finish;
        end
        consumed = consumed + 1;
    end
end

initial begin
    clk = 1'b0;
    reset_n = 1'b0;
    start = 1'b0;
    cnn_output_write = 1'b0;
    cnn_output_data = 12'h000;
    consumed = 0;

    for (i = 0; i < 1024; i = i + 1)
        bram.ram[i] = i[11:0];

    repeat (3) @(negedge clk);
    reset_n = 1'b1;
    @(negedge clk);
    start = 1'b1;
    @(negedge clk);
    start = 1'b0;

    wait (consumed == 1024);
    repeat (2) @(negedge clk);
    cnn_output_data = 12'h080;
    cnn_output_write = 1'b1;
    @(negedge clk);
    cnn_output_write = 1'b0;

    if (detection_valid !== 1'b1 || detection_result !== 12'h080) begin
        $display("FAIL: CNN output result was not latched");
        $finish;
    end

    @(negedge clk);
    if (detection_valid !== 1'b0) begin
        $display("FAIL: detection_valid was not a one-cycle pulse");
        $finish;
    end

    $display("PASS: CNN wrapper streams 1024 ordered samples and latches output");
    $finish;
end

endmodule
