`default_nettype none

// -----------------------------------------------------------------------------
// cnn_wrapper.v
//
// Feeds one 1024-pixel fixed<12,4> frame from input_bram into the hls4ml+Bambu
// CNN clean FIFO interface and latches the 12-bit sigmoid result.
// Clock domain: clk_i.
// Date: 2026-05-24
// Generated for hls4ml+Bambu CNN integration.
//
// FSM:
//   IDLE -> PRIME0 -> PRIME1 -> STREAM -> COLLECT -> IDLE
//
// PRIME0/PRIME1 account for input_bram's 1-cycle synchronous read latency.
// STREAM uses a held current-pixel register plus the BRAM read output as the
// prefetched next pixel. The visible input_1_dout only advances when the CNN
// asserts input_1_read.
// -----------------------------------------------------------------------------

module cnn_wrapper (
    input  wire        clk,
    input  wire        reset_n,
    input  wire        start,

    output reg  [9:0]  bram_addr,
    input  wire [11:0] bram_read_data,

    output reg         cnn_start,
    output wire [11:0] cnn_input_data,
    output reg         cnn_input_valid,
    input  wire        cnn_input_read,

    input  wire [11:0] cnn_output_data,
    input  wire        cnn_output_write,

    output reg         detection_valid,
    output reg  [11:0] detection_result
);

localparam [2:0] S_IDLE    = 3'd0;
localparam [2:0] S_PRIME0  = 3'd1;
localparam [2:0] S_PRIME1  = 3'd2;
localparam [2:0] S_STREAM  = 3'd3;
localparam [2:0] S_COLLECT = 3'd4;

reg [2:0]  state;
reg [11:0] held_data;
reg [9:0]  read_count;

assign cnn_input_data = held_data;

always @(posedge clk or negedge reset_n) begin
    if (!reset_n) begin
        state <= S_IDLE;
        bram_addr <= 10'd0;
        held_data <= 12'd0;
        read_count <= 10'd0;
        cnn_start <= 1'b0;
        cnn_input_valid <= 1'b0;
        detection_valid <= 1'b0;
        detection_result <= 12'd0;
    end else begin
        cnn_start <= 1'b0;
        detection_valid <= 1'b0;

        case (state)
            S_IDLE: begin
                bram_addr <= 10'd0;
                read_count <= 10'd0;
                cnn_input_valid <= 1'b0;

                if (start)
                    state <= S_PRIME0;
            end

            S_PRIME0: begin
                held_data <= bram_read_data;
                bram_addr <= 10'd1;
                cnn_input_valid <= 1'b0;
                read_count <= 10'd0;
                state <= S_PRIME1;
            end

            S_PRIME1: begin
                bram_addr <= 10'd2;
                cnn_input_valid <= 1'b1;
                cnn_start <= 1'b1;
                state <= S_STREAM;
            end

            S_STREAM: begin
                cnn_input_valid <= 1'b1;

                if (cnn_input_read) begin
                    read_count <= read_count + 10'd1;

                    if (read_count == 10'd1023) begin
                        cnn_input_valid <= 1'b0;
                        bram_addr <= 10'd0;
                        state <= S_COLLECT;
                    end else begin
                        held_data <= bram_read_data;

                        if (bram_addr != 10'd1023)
                            bram_addr <= bram_addr + 10'd1;
                        else
                            bram_addr <= 10'd1023;
                    end
                end
            end

            S_COLLECT: begin
                cnn_input_valid <= 1'b0;
                bram_addr <= 10'd0;

                if (cnn_output_write) begin
                    detection_result <= cnn_output_data;
                    detection_valid <= 1'b1;
                    state <= S_IDLE;
                end
            end

            default: begin
                state <= S_IDLE;
            end
        endcase
    end
end

endmodule

`default_nettype wire
