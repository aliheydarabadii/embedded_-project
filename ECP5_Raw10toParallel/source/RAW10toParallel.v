//   ==================================================================
//   >>>>>>>>>>>>>>>>>>>>>>> COPYRIGHT NOTICE <<<<<<<<<<<<<<<<<<<<<<<<<
//   ------------------------------------------------------------------
//   Copyright (c) 2017 by Lattice Semiconductor Corporation
//   ALL RIGHTS RESERVED 
//   ------------------------------------------------------------------
//
//   Permission:
//
//      Lattice SG Pte. Ltd. grants permission to use this code
//      pursuant to the terms of the Lattice Reference Design License Agreement. 
//
//
//   Disclaimer:
//
//      This VHDL or Verilog source code is intended as a design reference
//      which illustrates how these types of functions can be implemented.
//      It is the user's responsibility to verify their design for
//      consistency and functionality through the use of formal
//      verification methods.  Lattice provides no warranty
//      regarding the use or functionality of this code.
//
//   --------------------------------------------------------------------
//
//                  Lattice SG Pte. Ltd.
//                  101 Thomson Road, United Square #07-02 
//                  Singapore 307591
//
//                  web: http://www.latticesemi.com/
//                  email: techsupport@latticesemi.com
//
//   --------------------------------------------------------------------
//

module RAW10toParallel (
	input 			clk_i,
	input 			reset_n,
	
	// Raw10 input from Crosslink
	input        	CSI2_sens_clk ,
	input        	CSI2_sens_fv   ,
	input        	CSI2_sens_lv   ,   
	input [9:0]  	CSI2_sens_data ,
	
	// Parallel video to SiI1136
	output reg [11:0]    	pix_red,
	output reg [11:0]		pix_green,
	output reg [11:0]		pix_blue,
	output reg 				hsync,
	output reg 				vsync,
	output reg 				data_enable,
	output 					pixclk_out,
	
	// Sensor configuration
	inout  wire		scl,
	inout  wire		sda,
	inout  wire		scl2,
	inout  wire		sda2,
	
	// HDMI configuration
	inout 			HDMI_scl,
	inout 			HDMI_sda,
	
	output  wire 	reset_sensor,
	output  wire  	reset_crosslink,

	// blinking led
	output      	q
   );
   
//register input video
reg fv_i;
reg lv_i;
reg [9:0] data_i;

always @ (posedge CSI2_sens_clk or negedge reset_n)
	if(!reset_n) begin
		fv_i <= 0;
		lv_i <= 0;
		data_i <= 0;
	end
	else begin
   		fv_i <= CSI2_sens_fv;
		lv_i <= CSI2_sens_lv;
		data_i <= CSI2_sens_data;
	end
	
//register output video
wire [35:0] 	rgb;
wire [11:0]    	red_o, green_o, blue_o;
wire hsync_o;
wire vsync_o;
wire data_enable_o;

wire        fg_bram_we;
wire [9:0]  fg_bram_addr;
wire [11:0] fg_bram_wdata;
wire        frame_done;

wire [9:0]  cnn_bram_addr;
wire [11:0] cnn_bram_rdata;
wire        cnn_start;
wire [11:0] cnn_input_data;
wire        cnn_input_valid;
wire        cnn_input_read;
wire [11:0] cnn_output_data;
wire        cnn_output_write;
wire        cnn_done;
wire        detection_valid;
wire [11:0] detection_result;

// -------------------------------------------------------------------------
// CDC signals between video clock domain and CNN clock domain.
// -------------------------------------------------------------------------

// frame_done crosses from CSI2_sens_clk to clk_i using a toggle synchronizer.
reg       frame_done_toggle_csi;
reg [2:0] frame_done_toggle_sync_clk_i;
wire      frame_done_clk_i_pulse;

// detection result crosses from clk_i back to CSI2_sens_clk.
reg [11:0] detection_result_hold_clk_i;
reg        detection_toggle_clk_i;
reg [2:0]  detection_toggle_sync_csi;
reg        detection_valid_csi;
reg [11:0] detection_result_csi;

wire [11:0] overlay_red, overlay_green, overlay_blue;
wire        overlay_hsync;
wire        overlay_vsync;
wire        overlay_de;	
	
always @ (posedge CSI2_sens_clk or negedge reset_n)
	if(!reset_n) begin
		pix_red <= 0;
		pix_green <= 0;
		pix_blue <= 0;
		hsync <= 0;
		vsync <= 0;
		data_enable <= 0;			
	end
	else begin
		pix_red <= overlay_red;
		pix_green <= overlay_green;
		pix_blue <= overlay_blue;
		hsync <= overlay_hsync;
		vsync <= overlay_vsync;
		data_enable <= overlay_de;
	end
	
assign pixclk_out = ~CSI2_sens_clk;
assign red_o = rgb[35:24];
assign green_o = rgb[23:12];
assign blue_o = rgb[11:0]; 
	
image_pipe image_pipe_inst ( 
	.reset_n     (reset_n),

	.clk         	(CSI2_sens_clk),
	.frame_valid 	(fv_i),
	.line_valid  	(lv_i),
	.pixdata     	({data_i,2'b00}),

	.vsync       	(vsync_o),
	.hsync       	(hsync_o),
	.de          	(data_enable_o),
	.rgb_data		(rgb)
);

frame_grabber frame_grabber_inst (
	.clk			(CSI2_sens_clk),
	.reset_n		(reset_n),
	.red_in			(red_o),
	.green_in		(green_o),
	.blue_in		(blue_o),
	.vsync_in		(vsync_o),
	.de_in			(data_enable_o),
	.bram_we		(fg_bram_we),
	.bram_addr		(fg_bram_addr),
	.bram_wdata		(fg_bram_wdata),
	.frame_done		(frame_done)
);

input_bram input_bram_inst (
	.write_clk		(CSI2_sens_clk),
	.read_clk		(clk_i),

	.write_en		(fg_bram_we),
	.write_addr		(fg_bram_addr),
	.write_data		(fg_bram_wdata),

	.read_addr		(cnn_bram_addr),
	.read_data		(cnn_bram_rdata)
);

cnn_wrapper cnn_wrapper_inst (
	.clk			(clk_i),
	.reset_n		(reset_n),
	.start			(frame_done_clk_i_pulse),
	.bram_addr		(cnn_bram_addr),
	.bram_read_data		(cnn_bram_rdata),
	.cnn_start		(cnn_start),
	.cnn_input_data		(cnn_input_data),
	.cnn_input_valid	(cnn_input_valid),
	.cnn_input_read		(cnn_input_read),
	.cnn_output_data	(cnn_output_data),
	.cnn_output_write	(cnn_output_write),
	.detection_valid	(detection_valid),
	.detection_result	(detection_result)
);

cnn_top cnn_top_inst (
	.clock			(clk_i),
	.reset			(reset_n),
	.start_port		(cnn_start),
	.input_1_dout		(cnn_input_data),
	.input_1_empty_n	(cnn_input_valid),
	.layer10_out_full_n	(1'b1),
	.done_port		(cnn_done),
	.input_1_read		(cnn_input_read),
	.layer10_out_din	(cnn_output_data),
	.layer10_out_write	(cnn_output_write)
);

// -------------------------------------------------------------------------
// CDC: frame_done from CSI2_sens_clk domain to clk_i domain.
// frame_done is a one-cycle pulse in the fast video clock domain, so we
// convert it into a toggle and recover a one-cycle pulse in clk_i.
// -------------------------------------------------------------------------

always @(posedge CSI2_sens_clk or negedge reset_n) begin
	if (!reset_n) begin
		frame_done_toggle_csi <= 1'b0;
	end else if (frame_done) begin
		frame_done_toggle_csi <= ~frame_done_toggle_csi;
	end
end

always @(posedge clk_i or negedge reset_n) begin
	if (!reset_n) begin
		frame_done_toggle_sync_clk_i <= 3'b000;
	end else begin
		frame_done_toggle_sync_clk_i <= {
			frame_done_toggle_sync_clk_i[1:0],
			frame_done_toggle_csi
		};
	end
end

assign frame_done_clk_i_pulse =
	frame_done_toggle_sync_clk_i[2] ^ frame_done_toggle_sync_clk_i[1];


// -------------------------------------------------------------------------
// CDC: detection result from clk_i domain back to CSI2_sens_clk domain.
// detection_result_hold_clk_i is held stable until the next CNN result.
// The toggle tells the video domain when a new result is available.
// -------------------------------------------------------------------------

always @(posedge clk_i or negedge reset_n) begin
	if (!reset_n) begin
		detection_result_hold_clk_i <= 12'd0;
		detection_toggle_clk_i      <= 1'b0;
	end else if (detection_valid) begin
		detection_result_hold_clk_i <= detection_result;
		detection_toggle_clk_i      <= ~detection_toggle_clk_i;
	end
end

always @(posedge CSI2_sens_clk or negedge reset_n) begin
	if (!reset_n) begin
		detection_toggle_sync_csi <= 3'b000;
		detection_valid_csi       <= 1'b0;
		detection_result_csi      <= 12'd0;
	end else begin
		detection_toggle_sync_csi <= {
			detection_toggle_sync_csi[1:0],
			detection_toggle_clk_i
		};

		detection_valid_csi <=
			detection_toggle_sync_csi[2] ^ detection_toggle_sync_csi[1];

		if (detection_toggle_sync_csi[2] ^ detection_toggle_sync_csi[1]) begin
			detection_result_csi <= detection_result_hold_clk_i;
		end
	end
end

text_overlay text_overlay_inst (
	.clk			(CSI2_sens_clk),
	.reset_n		(reset_n),
	.r_in			(red_o),
	.g_in			(green_o),
	.b_in			(blue_o),
	.hsync_in		(hsync_o),
	.vsync_in		(vsync_o),
	.de_in			(data_enable_o),
	.detection_valid	(detection_valid_csi),
	.detection_result	(detection_result_csi),
	.r_out			(overlay_red),
	.g_out			(overlay_green),
	.b_out			(overlay_blue),
	.hsync_out		(overlay_hsync),
	.vsync_out		(overlay_vsync),
	.de_out			(overlay_de)
);

i2c_top i2c_inst (
	.clk_i 			(clk_i),
	.rst_n 			(reset_n),

	.scl			(scl),
	.sda			(sda),
	.scl2			(scl2),
	.sda2			(sda2),
	.reset_sensor	(reset_sensor),
	.reset_crosslink(reset_crosslink),
	.q				(q),
	.config_done	()
);

hdmi_i2c_top hdmi_i2c_top_inst(
	.rst_n			(reset_n),
	.clk			(clk_i),
	.scl			(HDMI_scl),
	.sda			(HDMI_sda),
	.config_done 	()
   );
   
endmodule
