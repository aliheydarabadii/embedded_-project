`default_nettype none

// -----------------------------------------------------------------------------
// input_bram.v
//
// 1024 x 12-bit frame buffer between frame_grabber and cnn_wrapper.
//
// Write side:
//   CSI2_sens_clk domain, written by frame_grabber.
//
// Read side:
//   clk_i domain, read by cnn_wrapper / cnn_top.
//
// This separates the fast video clock from the slower CNN clock.
// -----------------------------------------------------------------------------

module input_bram (
    input  wire        write_clk,
    input  wire        read_clk,

    input  wire        write_en,
    input  wire [9:0]  write_addr,
    input  wire [11:0] write_data,

    input  wire [9:0]  read_addr,
    output reg  [11:0] read_data
);

    (* syn_ramstyle = "block_ram" *) reg [11:0] ram [0:1023];

    always @(posedge write_clk) begin
        if (write_en) begin
            ram[write_addr] <= write_data;
        end
    end

    // One-cycle synchronous read in the CNN clock domain.
    always @(posedge read_clk) begin
        read_data <= ram[read_addr];
    end

endmodule

`default_nettype wire
