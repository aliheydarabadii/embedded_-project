`timescale 1ns / 1ps

module frame_grabber_tb;

reg         clk;
reg         reset_n;
reg  [11:0] red_in;
reg  [11:0] green_in;
reg  [11:0] blue_in;
reg         vsync_in;
reg         de_in;

wire        bram_we;
wire [9:0]  bram_addr;
wire [11:0] bram_wdata;
wire        frame_done;

integer writes;
integer row;
reg     saw_frame_done;
reg [11:0] captured [0:3];

frame_grabber #(
    .SRC_WIDTH  (4),
    .SRC_HEIGHT (4),
    .DST_WIDTH  (2),
    .DST_HEIGHT (2),
    .H_STRIDE   (2),
    .V_STRIDE   (2)
) dut (
    .clk        (clk),
    .reset_n    (reset_n),
    .red_in     (red_in),
    .green_in   (green_in),
    .blue_in    (blue_in),
    .vsync_in   (vsync_in),
    .de_in      (de_in),
    .bram_we    (bram_we),
    .bram_addr  (bram_addr),
    .bram_wdata (bram_wdata),
    .frame_done (frame_done)
);

always #5 clk = ~clk;

always @(posedge clk) begin
    if (bram_we) begin
        if (bram_addr !== writes[9:0]) begin
            $display("FAIL: write address %0d, expected %0d", bram_addr, writes);
            $finish;
        end

        captured[writes] = bram_wdata;
        writes = writes + 1;
    end

    if (frame_done)
        saw_frame_done = 1'b1;
end

task send_line;
    input integer line;
    integer col;
    reg [11:0] value;
    begin
        for (col = 0; col < 4; col = col + 1) begin
            value = (line * 4 + col) << 4;
            red_in = value;
            green_in = value;
            blue_in = value;
            de_in = 1'b1;
            @(posedge clk);
            #1;
        end

        de_in = 1'b0;
        @(posedge clk);
        #1;
    end
endtask

initial begin
    clk = 1'b0;
    reset_n = 1'b0;
    red_in = 12'd0;
    green_in = 12'd0;
    blue_in = 12'd0;
    vsync_in = 1'b0;
    de_in = 1'b0;
    writes = 0;
    saw_frame_done = 1'b0;

    repeat (2) @(posedge clk);
    reset_n = 1'b1;
    @(posedge clk);
    #1;

    vsync_in = 1'b1;
    @(posedge clk);
    #1;
    vsync_in = 1'b0;

    for (row = 0; row < 4; row = row + 1)
        send_line(row);

    repeat (6) @(posedge clk);
    #1;

    if (writes != 4) begin
        $display("FAIL: captured %0d samples, expected 4", writes);
        $finish;
    end

    if (captured[0] !== 12'd0 ||
        captured[1] !== 12'd2 ||
        captured[2] !== 12'd8 ||
        captured[3] !== 12'd10) begin
        $display("FAIL: unexpected samples %0d %0d %0d %0d",
                 captured[0], captured[1], captured[2], captured[3]);
        $finish;
    end

    if (!saw_frame_done) begin
        $display("FAIL: frame_done did not pulse");
        $finish;
    end

    $display("PASS: frame grabber captures ordered grayscale samples and pulses frame_done");
    $finish;
end

endmodule
