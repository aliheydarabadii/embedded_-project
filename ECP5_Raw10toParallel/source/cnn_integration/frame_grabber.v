`default_nettype none

// -----------------------------------------------------------------------------
// frame_grabber.v
//
// Captures one 32x32 grayscale frame from the post-ISP 12-bit RGB video stream
// for the hls4ml+Bambu CNN integration.
// Clock domain: CSI2_sens_clk, 148.5 MHz.
// Date: 2026-05-24
//
// The downsampler uses a fixed 60-column stride and 33-row stride. The row
// grid covers the first 32*33 = 1056 source rows, leaving the final 24 rows of
// a 1080-line frame unused.
// -----------------------------------------------------------------------------

module frame_grabber #(
    parameter integer SRC_WIDTH        = 1920,
    parameter integer SRC_HEIGHT       = 1080,
    parameter integer DST_WIDTH        = 32,
    parameter integer DST_HEIGHT       = 32,
    parameter integer H_STRIDE         = 60,
    parameter integer V_STRIDE         = 33,
    parameter integer VSYNC_ACTIVE_LOW = 0
) (
    input  wire        clk,
    input  wire        reset_n,
    input  wire [11:0] red_in,
    input  wire [11:0] green_in,
    input  wire [11:0] blue_in,
    input  wire        vsync_in,
    input  wire        de_in,
    output reg         bram_we,
    output reg  [9:0]  bram_addr,
    output reg  [11:0] bram_wdata,
    output reg         frame_done
);

localparam integer FRAME_PIXELS = DST_WIDTH * DST_HEIGHT;
localparam [7:0]  LUMA_R_COEFF = 8'd77;
localparam [7:0]  LUMA_G_COEFF = 8'd150;
localparam [7:0]  LUMA_B_COEFF = 8'd29;

reg        vsync_active_d;
reg        de_d;
reg        capture_active;
reg [11:0] x_count;
reg [10:0] y_count;
reg [11:0] next_sample_x;
reg [10:0] next_sample_y;
reg [4:0]  sample_col;
reg [4:0]  sample_row;
reg [10:0] write_count;

reg        sample_valid_s0;
reg [9:0]  sample_addr_s0;
reg        sample_last_s0;
reg [11:0] red_sample_s0;
reg [11:0] green_sample_s0;
reg [11:0] blue_sample_s0;

reg        sample_valid_s1;
reg [9:0]  sample_addr_s1;
reg        sample_last_s1;
reg [19:0] red_product_s1;
reg [19:0] green_product_s1;
reg [19:0] blue_product_s1;

reg        sample_valid_s2;
reg [9:0]  sample_addr_s2;
reg        sample_last_s2;
reg [19:0] luma_acc_s2;

wire vsync_active = (VSYNC_ACTIVE_LOW != 0) ? ~vsync_in : vsync_in;
wire vsync_start  = vsync_active & ~vsync_active_d;
wire de_rise      = de_in & ~de_d;
wire de_fall      = ~de_in & de_d;

wire [11:0] gray_12bit = luma_acc_s2[19:8];

// gray_12bit in [0, 4095], maps to fixed<12,4> in [0, 0.996] via >> 4.
wire [11:0] gray_q = {4'b0, gray_12bit[11:4]};

wire row_match = capture_active &&
                 (sample_row < DST_HEIGHT) &&
                 (y_count == next_sample_y);

wire col_match = row_match &&
                 (sample_col < DST_WIDTH) &&
                 (x_count == next_sample_x);

wire write_sample = de_in &&
                    col_match &&
                    (write_count < FRAME_PIXELS);

always @(posedge clk or negedge reset_n) begin
    if (!reset_n) begin
        vsync_active_d <= 1'b0;
        de_d <= 1'b0;
    end else begin
        vsync_active_d <= vsync_active;
        de_d <= de_in;
    end
end

always @(posedge clk or negedge reset_n) begin
    if (!reset_n) begin
        capture_active <= 1'b0;
        x_count <= 12'd0;
        y_count <= 11'd0;
        next_sample_x <= 12'd0;
        next_sample_y <= 11'd0;
        sample_col <= 5'd0;
	        sample_row <= 5'd0;
	        write_count <= 11'd0;
	        sample_valid_s0 <= 1'b0;
	        sample_addr_s0 <= 10'd0;
	        sample_last_s0 <= 1'b0;
	        red_sample_s0 <= 12'd0;
	        green_sample_s0 <= 12'd0;
	        blue_sample_s0 <= 12'd0;
	        sample_valid_s1 <= 1'b0;
	        sample_addr_s1 <= 10'd0;
	        sample_last_s1 <= 1'b0;
	        red_product_s1 <= 20'd0;
	        green_product_s1 <= 20'd0;
	        blue_product_s1 <= 20'd0;
	        sample_valid_s2 <= 1'b0;
	        sample_addr_s2 <= 10'd0;
	        sample_last_s2 <= 1'b0;
	        luma_acc_s2 <= 20'd0;
	        bram_we <= 1'b0;
	        bram_addr <= 10'd0;
	        bram_wdata <= 12'd0;
	        frame_done <= 1'b0;
	    end else begin
	        sample_valid_s0 <= write_sample;
	        sample_addr_s0 <= write_count[9:0];
	        sample_last_s0 <= (write_count == (FRAME_PIXELS - 1));
	        if (write_sample) begin
	            red_sample_s0 <= red_in;
	            green_sample_s0 <= green_in;
	            blue_sample_s0 <= blue_in;
	        end

	        sample_valid_s1 <= sample_valid_s0;
	        sample_addr_s1 <= sample_addr_s0;
	        sample_last_s1 <= sample_last_s0;
	        if (sample_valid_s0) begin
	            red_product_s1 <= LUMA_R_COEFF * red_sample_s0;
	            green_product_s1 <= LUMA_G_COEFF * green_sample_s0;
	            blue_product_s1 <= LUMA_B_COEFF * blue_sample_s0;
	        end

	        sample_valid_s2 <= sample_valid_s1;
	        sample_addr_s2 <= sample_addr_s1;
	        sample_last_s2 <= sample_last_s1;
	        if (sample_valid_s1)
	            luma_acc_s2 <= red_product_s1 + green_product_s1 + blue_product_s1;

	        bram_we <= sample_valid_s2;
	        bram_addr <= sample_addr_s2;
	        bram_wdata <= gray_q;
	        frame_done <= sample_valid_s2 && sample_last_s2;

	        if (vsync_start) begin
	            capture_active <= 1'b1;
	            x_count <= 12'd0;
            y_count <= 11'd0;
            next_sample_x <= 12'd0;
            next_sample_y <= 11'd0;
	            sample_col <= 5'd0;
	            sample_row <= 5'd0;
	            write_count <= 11'd0;
	            sample_valid_s0 <= 1'b0;
	            sample_valid_s1 <= 1'b0;
	            sample_valid_s2 <= 1'b0;
	            bram_we <= 1'b0;
	            frame_done <= 1'b0;
	            bram_addr <= 10'd0;
	        end else begin
            if (de_in) begin
                if (de_rise)
                    x_count <= 12'd1;
                else
                    x_count <= x_count + 12'd1;
            end else begin
                x_count <= 12'd0;
            end

	            if (write_sample) begin
	                if (write_count == (FRAME_PIXELS - 1)) begin
	                    write_count <= write_count + 11'd1;
	                    capture_active <= 1'b0;
	                end else begin
	                    write_count <= write_count + 11'd1;
	                end

                if (sample_col < (DST_WIDTH - 1)) begin
                    sample_col <= sample_col + 5'd1;
                    next_sample_x <= next_sample_x + H_STRIDE;
                end
            end

            if (de_fall) begin
                y_count <= y_count + 11'd1;
                sample_col <= 5'd0;
                next_sample_x <= 12'd0;

                if (row_match && (sample_row < DST_HEIGHT)) begin
                    sample_row <= sample_row + 5'd1;
                    next_sample_y <= next_sample_y + V_STRIDE;
                end
            end
        end
    end
end

endmodule

`default_nettype wire
