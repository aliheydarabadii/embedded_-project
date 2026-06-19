# Embedded CNN Image Pipeline

This repository contains an FPGA image-processing pipeline for an ECP5 design and
the hls4ml/Bambu output used to integrate a small CNN into that pipeline.

The main hardware project is `ECP5_Raw10toParallel`. It receives RAW10 video,
converts it to an RGB video stream, captures a 32x32 grayscale frame window for
CNN inference, and overlays the detection result on the outgoing HDMI path.

## Repository Layout

- `ECP5_Raw10toParallel/` - Lattice Diamond project, Verilog RTL, constraints,
  simulation testbenches, generated implementation outputs, and bitstream files.
- `ECP5_Raw10toParallel/source/` - RAW10 unpacking, image pipe, debayering,
  color correction, gamma correction, I2C control, and top-level integration.
- `ECP5_Raw10toParallel/source/cnn_integration/` - BRAM capture, CNN FIFO
  wrapper, text overlay, and generated CNN top-level wrapper.
- `ECP5_Raw10toParallel/sim/` - Verilog testbenches for the CNN wrapper,
  frame grabber, overlay, and integrated CNN path.
- `hls_full_model_simple32/` - hls4ml/Bambu project for the 32x32 CNN,
  generated firmware, weights, memory files, HDL output, and build scripts.
- `generate.py` - Regenerates the hls4ml project from a Keras model and applies
  project-specific Bambu interface patches.
- `per_layer_precision_sweep_yolo.py` - Evaluates candidate fixed-point
  precisions against a YOLO-style image dataset.
- `CNN_TRAINING_readable.ipynb` - Training notebook for the CNN model.

## Hardware Flow

The Diamond project entry point is:

```sh
ECP5_Raw10toParallel/Raw10toParallel.ldf
```

The included Tcl helpers run against an absolute project path from the original
machine. If the repository is checked out elsewhere, update the path inside the
scripts before running them.

```sh
diamondc ECP5_Raw10toParallel/run_synthesis.tcl
diamondc ECP5_Raw10toParallel/run_full_bitstream.tcl
diamondc ECP5_Raw10toParallel/run_bitgen.tcl
```

The final bitstream artifact is also checked in at:

```sh
ECP5_Raw10toParallel/bitstream/Raw10toParallel.bit
```

## HLS/CNN Flow

The committed HLS output targets:

- Part: `LFE5UM85F8BG756C`
- Backend: `Bambu`
- Model input: `32x32x1`
- Top function: `myproject`
- Output project: `hls_full_model_simple32`

Regenerate the HLS project from a Keras model:

```sh
python3 generate.py --model path/to/model.keras --output-dir hls_full_model_simple32 --no-run-bambu
```

Run Bambu manually:

```sh
cd hls_full_model_simple32
bash build_bambu.sh
```

Build the software bridge shared library:

```sh
cd hls_full_model_simple32
bash build_lib.sh
```

## Simulation

The simulation directory includes standalone and integrated testbenches:

- `cnn_wrapper_tb.v` verifies the 1024-sample BRAM-to-CNN stream sequence and
  result latch behavior.
- `cnn_system_tb.v` connects the wrapper to the generated CNN top-level.
- `frame_grabber_tb.v` exercises frame capture into the CNN input BRAM.
- `text_overlay_tb.v` checks overlay timing and display behavior.

Use a Verilog simulator available in your environment and include the relevant
RTL files from `ECP5_Raw10toParallel/source` and
`ECP5_Raw10toParallel/source/cnn_integration`.

## Notes

- Several directories contain generated vendor and HLS artifacts. They are kept
  in the repository so the current hardware/HLS state can be reproduced without
  rerunning the full toolchain.
- The HLS regeneration scripts expect TensorFlow, hls4ml, Bambu, NumPy, Pillow,
  and Matplotlib to be installed in the active Python environment.
- The Diamond build scripts may need local path updates before they can run on a
  different workstation.
