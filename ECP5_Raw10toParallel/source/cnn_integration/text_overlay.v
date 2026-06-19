`default_nettype none

// -----------------------------------------------------------------------------
// text_overlay.v
//
// Draws a top-left "DETECTED" / "NOT DETECTED" overlay on the post-ISP video
// stream using the hls4ml+Bambu CNN result with 3-frame hysteresis.
// Clock domain: CSI2_sens_clk, 148.5 MHz.
// Date: 2026-05-24
// Generated for hls4ml+Bambu CNN integration.
//
// Pixel path has TWO registered pipeline stages (was one). The extra stage
// breaks the long combinational chain:
//   x_count -> comparisons -> char_index -> overlay_char() [Stage 1 capture]
//   glyph_char -> glyph_row() -> glyph_pixel -> r/g/b_out [Stage 2 output]
//
// All sync outputs (hsync_out, vsync_out, de_out) are delayed by the same
// 2-cycle latency so the video stream remains aligned.
//
// Hysteresis updates only on the detection_valid event:
//   raw_det == display_state  -> clear pending count
//   raw_det == pending_state  -> advance pending count
//   otherwise                 -> start a new pending state
// -----------------------------------------------------------------------------

module text_overlay #(
    parameter integer VSYNC_ACTIVE_LOW = 0
) (
    input  wire        clk,
    input  wire        reset_n,

    input  wire [11:0] r_in,
    input  wire [11:0] g_in,
    input  wire [11:0] b_in,
    input  wire        hsync_in,
    input  wire        vsync_in,
    input  wire        de_in,

    input  wire        detection_valid,
    input  wire [11:0] detection_result,

    output reg  [11:0] r_out,
    output reg  [11:0] g_out,
    output reg  [11:0] b_out,
    output reg         hsync_out,
    output reg         vsync_out,
    output reg         de_out
);

// Q4.8 score: 0x080 represents 50%; scores below this display DETECTED.
localparam signed [11:0] DETECT_THRESHOLD = 12'sh080;

localparam [11:0] BG_X      = 12'd8;
localparam [10:0] BG_Y      = 11'd8;
localparam [11:0] BG_RIGHT  = 12'd408;
localparam [10:0] BG_BOTTOM = 11'd56;

localparam [11:0] TEXT_X             = 12'd16;
localparam [10:0] TEXT_Y             = 11'd16;
localparam [10:0] TEXT_BOTTOM        = 11'd48;
localparam [11:0] DETECTED_RIGHT     = 12'd272;
localparam [11:0] NOT_DETECTED_RIGHT = 12'd400;

// -------------------------------------------------------------------------
// Stage-0 state: coordinate counters, detection hysteresis
// -------------------------------------------------------------------------

reg        de_d;
reg        vsync_active_d;
reg        detection_valid_d;
reg [11:0] x_count;
reg [10:0] y_count;

reg        display_state;
reg        pending_state;
reg [1:0]  stable_count;
reg signed [11:0] detection_result_latched;

wire vsync_active = (VSYNC_ACTIVE_LOW != 0) ? ~vsync_in : vsync_in;
wire vsync_start  = vsync_active & ~vsync_active_d;
wire de_rise      = de_in & ~de_d;
wire de_fall      = ~de_in & de_d;

wire detection_event = detection_valid & ~detection_valid_d;
wire raw_det_now = ($signed(detection_result) < DETECT_THRESHOLD);

// -------------------------------------------------------------------------
// Stage-0 combinational: pixel position, region checks, char lookup
// These feed the Stage-1 pipeline registers.
// -------------------------------------------------------------------------

wire [11:0] pixel_x = x_count;
wire [10:0] pixel_y = y_count;

wire [11:0] text_right = display_state ? DETECTED_RIGHT : NOT_DETECTED_RIGHT;

wire in_background = de_in &&
                     (pixel_x >= BG_X) &&
                     (pixel_x < BG_RIGHT) &&
                     (pixel_y >= BG_Y) &&
                     (pixel_y < BG_BOTTOM);

wire in_text_box = de_in &&
                   (pixel_x >= TEXT_X) &&
                   (pixel_x < text_right) &&
                   (pixel_y >= TEXT_Y) &&
                   (pixel_y < TEXT_BOTTOM);

wire [11:0] rel_x     = pixel_x - TEXT_X;
wire [10:0] rel_y     = pixel_y - TEXT_Y;
wire [3:0]  char_index = rel_x[8:5];
wire [2:0]  glyph_x   = rel_x[4:2];
wire [2:0]  glyph_y   = rel_y[4:2];
wire [7:0]  glyph_char = overlay_char(display_state, char_index);

// -------------------------------------------------------------------------
// Stage-1 pipeline registers
// Captures: char lookup result, glyph coords, region flags, RGB, sync
// -------------------------------------------------------------------------

reg [7:0]  glyph_char_r1;
reg [2:0]  glyph_x_r1;
reg [2:0]  glyph_y_r1;
reg        in_bg_r1;
reg        in_tb_r1;
reg        display_r1;
reg [11:0] r_r1;
reg [11:0] g_r1;
reg [11:0] b_r1;
reg        hsync_r1;
reg        vsync_r1;
reg        de_r1;

// -------------------------------------------------------------------------
// Stage-1 combinational (feeds Stage-2 output registers)
// -------------------------------------------------------------------------

wire [7:0] glyph_bits   = glyph_row(glyph_char_r1, glyph_y_r1);
wire       glyph_pixel  = in_tb_r1 & glyph_bits[3'd7 - glyph_x_r1];

// -------------------------------------------------------------------------
// Sequential logic
// -------------------------------------------------------------------------

always @(posedge clk or negedge reset_n) begin
    if (!reset_n) begin
        // Stage-0 state
        de_d                   <= 1'b0;
        vsync_active_d         <= 1'b0;
        detection_valid_d      <= 1'b0;
        x_count                <= 12'd0;
        y_count                <= 11'd0;
        display_state          <= 1'b0;
        pending_state          <= 1'b0;
        stable_count           <= 2'd0;
        detection_result_latched <= 12'sd0;

        // Stage-1 pipeline registers
        glyph_char_r1  <= 8'd0;
        glyph_x_r1     <= 3'd0;
        glyph_y_r1     <= 3'd0;
        in_bg_r1       <= 1'b0;
        in_tb_r1       <= 1'b0;
        display_r1     <= 1'b0;
        r_r1           <= 12'd0;
        g_r1           <= 12'd0;
        b_r1           <= 12'd0;
        hsync_r1       <= 1'b0;
        vsync_r1       <= 1'b0;
        de_r1          <= 1'b0;

        // Stage-2 output registers
        r_out     <= 12'd0;
        g_out     <= 12'd0;
        b_out     <= 12'd0;
        hsync_out <= 1'b0;
        vsync_out <= 1'b0;
        de_out    <= 1'b0;
    end else begin

        // ---- Stage-0: counters and hysteresis ----

        de_d              <= de_in;
        vsync_active_d    <= vsync_active;
        detection_valid_d <= detection_valid;

        if (detection_event) begin
            detection_result_latched <= $signed(detection_result);

            if (raw_det_now == display_state) begin
                pending_state <= display_state;
                stable_count  <= 2'd0;
            end else if (raw_det_now == pending_state) begin
                if (stable_count >= 2'd2) begin
                    display_state <= raw_det_now;
                    stable_count  <= 2'd0;
                end else begin
                    stable_count <= stable_count + 2'd1;
                end
            end else begin
                pending_state <= raw_det_now;
                stable_count  <= 2'd1;
            end
        end

        if (vsync_start) begin
            x_count <= 12'd0;
            y_count <= 11'd0;
        end else begin
            if (de_in) begin
                if (de_rise)
                    x_count <= 12'd1;
                else
                    x_count <= x_count + 12'd1;
            end else begin
                x_count <= 12'd0;
            end

            if (de_fall)
                y_count <= y_count + 11'd1;
        end

        // ---- Stage-1 capture: char lookup, region flags, RGB, sync ----
        // (combinational inputs: in_background, in_text_box, glyph_char,
        //  glyph_x, glyph_y — all driven from x_count/y_count this cycle)

        glyph_char_r1 <= glyph_char;
        glyph_x_r1    <= glyph_x;
        glyph_y_r1    <= glyph_y;
        in_bg_r1      <= in_background;
        in_tb_r1      <= in_text_box;
        display_r1    <= display_state;
        r_r1          <= r_in;
        g_r1          <= g_in;
        b_r1          <= b_in;
        hsync_r1      <= hsync_in;
        vsync_r1      <= vsync_in;
        de_r1         <= de_in;

        // ---- Stage-2 output: glyph_row lookup, final pixel mux ----
        // (combinational inputs: glyph_char_r1, glyph_y_r1, glyph_x_r1,
        //  in_tb_r1, in_bg_r1, display_r1, r_r1/g_r1/b_r1)

        hsync_out <= hsync_r1;
        vsync_out <= vsync_r1;
        de_out    <= de_r1;

        if (glyph_pixel) begin
            if (display_r1) begin
                r_out <= 12'h000;
                g_out <= 12'hfff;
                b_out <= 12'h000;
            end else begin
                r_out <= 12'hfff;
                g_out <= 12'hfff;
                b_out <= 12'hfff;
            end
        end else if (in_bg_r1) begin
            r_out <= 12'h000;
            g_out <= 12'h000;
            b_out <= 12'h000;
        end else begin
            r_out <= r_r1;
            g_out <= g_r1;
            b_out <= b_r1;
        end

    end
end

// -------------------------------------------------------------------------
// Font functions (pure combinational)
// -------------------------------------------------------------------------

function [7:0] overlay_char;
    input detected;
    input [3:0] index;
    begin
        if (detected) begin
            case (index)
                4'd0: overlay_char = "D";
                4'd1: overlay_char = "E";
                4'd2: overlay_char = "T";
                4'd3: overlay_char = "E";
                4'd4: overlay_char = "C";
                4'd5: overlay_char = "T";
                4'd6: overlay_char = "E";
                4'd7: overlay_char = "D";
                default: overlay_char = " ";
            endcase
        end else begin
            case (index)
                4'd0: overlay_char = "N";
                4'd1: overlay_char = "O";
                4'd2: overlay_char = "T";
                4'd3: overlay_char = " ";
                4'd4: overlay_char = "D";
                4'd5: overlay_char = "E";
                4'd6: overlay_char = "T";
                4'd7: overlay_char = "E";
                4'd8: overlay_char = "C";
                4'd9: overlay_char = "T";
                4'd10: overlay_char = "E";
                4'd11: overlay_char = "D";
                default: overlay_char = " ";
            endcase
        end
    end
endfunction

function [7:0] glyph_row;
    input [7:0] char_code;
    input [2:0] row;
    begin
        case (char_code)
            "C": begin
                case (row)
                    3'd0: glyph_row = 8'b00111100;
                    3'd1: glyph_row = 8'b01100110;
                    3'd2: glyph_row = 8'b11000000;
                    3'd3: glyph_row = 8'b11000000;
                    3'd4: glyph_row = 8'b11000000;
                    3'd5: glyph_row = 8'b11000000;
                    3'd6: glyph_row = 8'b01100110;
                    default: glyph_row = 8'b00111100;
                endcase
            end

            "D": begin
                case (row)
                    3'd0: glyph_row = 8'b11111000;
                    3'd1: glyph_row = 8'b11001100;
                    3'd2: glyph_row = 8'b11000110;
                    3'd3: glyph_row = 8'b11000110;
                    3'd4: glyph_row = 8'b11000110;
                    3'd5: glyph_row = 8'b11000110;
                    3'd6: glyph_row = 8'b11001100;
                    default: glyph_row = 8'b11111000;
                endcase
            end

            "E": begin
                case (row)
                    3'd0: glyph_row = 8'b11111110;
                    3'd1: glyph_row = 8'b11000000;
                    3'd2: glyph_row = 8'b11000000;
                    3'd3: glyph_row = 8'b11111000;
                    3'd4: glyph_row = 8'b11000000;
                    3'd5: glyph_row = 8'b11000000;
                    3'd6: glyph_row = 8'b11000000;
                    default: glyph_row = 8'b11111110;
                endcase
            end

            "N": begin
                case (row)
                    3'd0: glyph_row = 8'b11000110;
                    3'd1: glyph_row = 8'b11100110;
                    3'd2: glyph_row = 8'b11110110;
                    3'd3: glyph_row = 8'b11011110;
                    3'd4: glyph_row = 8'b11001110;
                    3'd5: glyph_row = 8'b11000110;
                    3'd6: glyph_row = 8'b11000110;
                    default: glyph_row = 8'b11000110;
                endcase
            end

            "O": begin
                case (row)
                    3'd0: glyph_row = 8'b00111100;
                    3'd1: glyph_row = 8'b01100110;
                    3'd2: glyph_row = 8'b11000011;
                    3'd3: glyph_row = 8'b11000011;
                    3'd4: glyph_row = 8'b11000011;
                    3'd5: glyph_row = 8'b11000011;
                    3'd6: glyph_row = 8'b01100110;
                    default: glyph_row = 8'b00111100;
                endcase
            end

            "T": begin
                case (row)
                    3'd0: glyph_row = 8'b11111111;
                    3'd1: glyph_row = 8'b00011000;
                    3'd2: glyph_row = 8'b00011000;
                    3'd3: glyph_row = 8'b00011000;
                    3'd4: glyph_row = 8'b00011000;
                    3'd5: glyph_row = 8'b00011000;
                    3'd6: glyph_row = 8'b00011000;
                    default: glyph_row = 8'b00011000;
                endcase
            end

            default: begin
                glyph_row = 8'b00000000;
            end
        endcase
    end
endfunction

endmodule

`default_nettype wire
