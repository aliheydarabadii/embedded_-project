`timescale 1ns / 1ps

module text_overlay_tb;

reg         clk;
reg         reset_n;
reg  [11:0] r_in;
reg  [11:0] g_in;
reg  [11:0] b_in;
reg         hsync_in;
reg         vsync_in;
reg         de_in;
reg         detection_valid;
reg  [11:0] detection_result;

wire [11:0] r_out;
wire [11:0] g_out;
wire [11:0] b_out;
wire        hsync_out;
wire        vsync_out;
wire        de_out;

integer white_pixels;
integer green_pixels;
integer i;

text_overlay dut (
    .clk              (clk),
    .reset_n          (reset_n),
    .r_in             (r_in),
    .g_in             (g_in),
    .b_in             (b_in),
    .hsync_in         (hsync_in),
    .vsync_in         (vsync_in),
    .de_in            (de_in),
    .detection_valid  (detection_valid),
    .detection_result (detection_result),
    .r_out            (r_out),
    .g_out            (g_out),
    .b_out            (b_out),
    .hsync_out        (hsync_out),
    .vsync_out        (vsync_out),
    .de_out           (de_out)
);

always #5 clk = ~clk;

always @(posedge clk) begin
    if (de_out && (r_out == 12'hfff) &&
                  (g_out == 12'hfff) &&
                  (b_out == 12'hfff))
        white_pixels = white_pixels + 1;

    if (de_out && (r_out == 12'h000) &&
                  (g_out == 12'hfff) &&
                  (b_out == 12'h000))
        green_pixels = green_pixels + 1;
end

task send_detection;
    input [11:0] score;
    begin
        @(negedge clk);
        detection_result = score;
        detection_valid = 1'b1;
        @(negedge clk);
        detection_valid = 1'b0;
        @(negedge clk);
    end
endtask

task send_line;
    begin
        @(negedge clk);
        de_in = 1'b1;
        for (i = 0; i < 96; i = i + 1)
            @(negedge clk);
        de_in = 1'b0;
        repeat (4) @(negedge clk);
    end
endtask

task send_frame_prefix;
    integer line;
    begin
        @(negedge clk);
        vsync_in = 1'b1;
        @(negedge clk);
        vsync_in = 1'b0;

        for (line = 0; line < 24; line = line + 1)
            send_line();

        repeat (4) @(negedge clk);
    end
endtask

initial begin
    clk = 1'b0;
    reset_n = 1'b0;
    r_in = 12'h000;
    g_in = 12'h000;
    b_in = 12'h000;
    hsync_in = 1'b0;
    vsync_in = 1'b0;
    de_in = 1'b0;
    detection_valid = 1'b0;
    detection_result = 12'h000;
    white_pixels = 0;
    green_pixels = 0;

    repeat (3) @(negedge clk);
    reset_n = 1'b1;

    send_frame_prefix();
    if (white_pixels == 0) begin
        $display("FAIL: initial NOT DETECTED overlay did not render");
        $finish;
    end

    send_detection(12'h080);
    send_detection(12'h080);
    send_detection(12'h080);
    if (dut.display_state !== 1'b0) begin
        $display("FAIL: overlay switched at or above the 50 percent threshold");
        $finish;
    end

    send_detection(12'h07f);
    send_detection(12'h07f);
    if (dut.display_state !== 1'b0) begin
        $display("FAIL: overlay switched before three below-threshold events");
        $finish;
    end

    send_detection(12'h07f);
    if (dut.display_state !== 1'b1) begin
        $display("FAIL: overlay did not switch after three below-threshold events");
        $finish;
    end

    white_pixels = 0;
    green_pixels = 0;
    send_frame_prefix();
    if (green_pixels == 0) begin
        $display("FAIL: DETECTED overlay did not render");
        $finish;
    end

    $display("PASS: text overlay renders both states and applies hysteresis");
    $finish;
end

endmodule
