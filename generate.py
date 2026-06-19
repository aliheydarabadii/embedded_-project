import argparse
import os
import re
import shutil
import subprocess
from pathlib import Path

import tensorflow as tf
import hls4ml


# ─────────────────────────────────────────────
# CONFIG — 32×32 INPUT + FIRST AVGPOOL MODEL
# ─────────────────────────────────────────────
PART = "LFE5UM85F8BG756C"
BACKEND = "bambu"
PROJECT_NAME = "myproject"

EXPECTED_INPUT_SHAPE = (None, 32, 32, 1)


# ─────────────────────────────────────────────
# SELECTED PER-LAYER PRECISIONS
# Model:
# input_1
# average_pooling2d
# conv2d
# conv2d_relu
# conv2d_1
# conv2d_1_relu
# average_pooling2d_1
# flatten
# dense
# dense_sigmoid
# ─────────────────────────────────────────────
SELECTED_LAYER_PRECISIONS = {
    "input_1": "fixed<12,4>",

    "average_pooling2d": "fixed<12,4>",

    "conv2d": "fixed<16,5>",
    "conv2d_relu": "fixed<14,4>",

    "conv2d_1": "fixed<16,5>",
    "conv2d_1_relu": "fixed<12,4>",

    "average_pooling2d_1": "fixed<20,6>",

    "flatten": "fixed<12,4>",

    "dense": "fixed<14,4>",
    "dense_sigmoid": "fixed<12,4>",
}


# ─────────────────────────────────────────────
# FALLBACK MODEL
# ─────────────────────────────────────────────
def build_simple32_pooled_12_model():
    """
    This must match the trained model exactly.

    Input 32x32x1
    AveragePooling2D(2x2)     -> 16x16x1
    Conv2D(1, stride=2)       -> 8x8x1
    Conv2D(2, stride=2)       -> 4x4x2
    AveragePooling2D(4x4)     -> 1x1x2
    Flatten                   -> 2
    Dense(1, sigmoid)         -> 1
    """
    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(32, 32, 1)),

        tf.keras.layers.AveragePooling2D(pool_size=(2, 2)),

        tf.keras.layers.Conv2D(
            filters=1,
            kernel_size=3,
            strides=2,
            padding="same",
            activation="relu",
        ),

        tf.keras.layers.Conv2D(
            filters=2,
            kernel_size=3,
            strides=2,
            padding="same",
            activation="relu",
        ),

        tf.keras.layers.AveragePooling2D(pool_size=(4, 4)),
        tf.keras.layers.Flatten(),

        tf.keras.layers.Dense(1, activation="sigmoid"),
    ])

    return model


def load_model_compatible(model_path):
    """
    Try normal load_model first.
    If it fails because of Keras version mismatch,
    rebuild the exact architecture and load weights.
    """
    try:
        print("[INFO] Trying tf.keras.models.load_model...")
        model = tf.keras.models.load_model(model_path, compile=False)
        print("[OK] Loaded full Keras model.")
        return model

    except Exception as e:
        print("\n[WARN] Full model loading failed.")
        print("[WARN] Rebuilding 32x32 pooled model and loading weights.")
        print(f"[WARN] Original error: {e}\n")

        model = build_simple32_pooled_12_model()
        model.load_weights(model_path)

        print("[OK] Loaded weights into fallback model.")

        return model


# ─────────────────────────────────────────────
# HLS CONFIG
# ─────────────────────────────────────────────
def set_layer_precision(config, layer_name, precision):
    layer_cfg = config.get("LayerName", {}).get(layer_name)

    if layer_cfg is None:
        print(f"[WARN] Layer not found in hls4ml config: {layer_name}")
        return

    precision_cfg = layer_cfg.get("Precision")

    if precision_cfg is None:
        print(f"[WARN] No Precision field for layer: {layer_name}")
        return

    if isinstance(precision_cfg, dict):
        for key in list(precision_cfg.keys()):
            old = precision_cfg[key]
            precision_cfg[key] = precision
            print(f"[OK] {layer_name}.{key}: {old} -> {precision}")
    else:
        old = layer_cfg["Precision"]
        layer_cfg["Precision"] = precision
        print(f"[OK] {layer_name}: {old} -> {precision}")


def make_hls_config(model, precision, reuse_factor, io_type, use_selected_precisions=True):
    config = hls4ml.utils.config_from_keras_model(
        model,
        granularity="name",
        default_precision=precision,
        backend=BACKEND,
    )

    config["Model"]["Precision"] = precision
    config["Model"]["ReuseFactor"] = reuse_factor
    config["Model"]["Strategy"] = "Resource"
    config["Model"]["IOType"] = io_type

    if use_selected_precisions:
        print("\n[INFO] Applying selected per-layer precisions:")
        for layer_name, layer_precision in SELECTED_LAYER_PRECISIONS.items():
            set_layer_precision(config, layer_name, layer_precision)
    else:
        print("\n[INFO] Using global precision only.")

    print("\n[INFO] Final hls4ml layer precisions:")
    for lname, lcfg in config.get("LayerName", {}).items():
        print(f"{lname}: {lcfg.get('Precision', None)}")

    return config


# ─────────────────────────────────────────────
# PATCHES
# ─────────────────────────────────────────────
def patch_interface_pragmas(cpp_path):
    cpp_path = Path(cpp_path)

    if not cpp_path.exists():
        raise FileNotFoundError(f"Generated C++ file not found: {cpp_path}")

    text = cpp_path.read_text(encoding="utf-8")

    combined_pattern = re.compile(
        r"(?m)^([ \t]*)#pragma\s+HLS\s+interface\s+mode=(valid|fifo)\s+port="
        r"([A-Za-z_][A-Za-z0-9_]*(?:\s*,\s*[A-Za-z_][A-Za-z0-9_]*)+)\s*$"
    )

    def replace_combined(match):
        indent = match.group(1)
        ports = [p.strip() for p in match.group(3).split(",")]
        return "\n".join(
            f"{indent}#pragma HLS interface mode=fifo port={port}"
            for port in ports
        )

    text = combined_pattern.sub(replace_combined, text)

    text = re.sub(
        r"(?m)^([ \t]*)#pragma\s+HLS\s+interface\s+mode=valid\s+port=([A-Za-z_][A-Za-z0-9_]*)\s*$",
        r"\1#pragma HLS interface mode=fifo port=\2",
        text,
    )

    cpp_path.write_text(text, encoding="utf-8")

    if "mode=valid" in text:
        raise RuntimeError("Bambu pragma patch failed: mode=valid still exists.")

    print("[OK] Patched HLS interface pragmas.")


def patch_bambu_register_allocation(build_script):
    """
    Force COLORING register allocation to avoid Bambu hanging in WeightedCliqueRegisterBinding.
    """
    build_script = Path(build_script)

    if not build_script.exists():
        print(f"[WARN] Build script not found: {build_script}")
        return

    text = build_script.read_text()

    if "--register-allocation=COLORING" in text:
        print("[OK] Bambu register allocation already COLORING.")
        return

    original_text = text

    patterns = [
        (
            r"(?m)^(\s*)(bambu)(\s+)",
            r"\1\2 --register-allocation=COLORING\3",
        ),
        (
            r"(?m)^(\s*)(\$BAMBU)(\s+)",
            r"\1\2 --register-allocation=COLORING\3",
        ),
        (
            r"(?m)^(\s*)(\$\{BAMBU\})(\s+)",
            r"\1\2 --register-allocation=COLORING\3",
        ),
    ]

    for pattern, repl in patterns:
        text = re.sub(pattern, repl, text)

    if text == original_text:
        print("[WARN] Could not patch register allocation automatically.")
        print("      Manually add --register-allocation=COLORING to build_bambu.sh")
    else:
        build_script.write_text(text)
        print("[OK] Added --register-allocation=COLORING to build_bambu.sh")


def verify_generated_cpp_is_32x32(cpp_path):
    cpp_path = Path(cpp_path)

    if not cpp_path.exists():
        raise FileNotFoundError(f"Generated C++ file not found: {cpp_path}")

    text = cpp_path.read_text(encoding="utf-8")

    if "input_t input_1[16*16*1]" in text:
        raise RuntimeError("Generated C++ is 16x16, but expected 32x32.")

    if "input_t input_1[32*32*1]" in text:
        print("[OK] Generated C++ input shape is 32x32.")
    else:
        print("[WARN] Could not find explicit input_1[32*32*1]. Check manually.")


def patch_build_script_compiler(build_script):
    """
    Optional compiler fallback patch.
    Useful when hls4ml generated scripts default to g++ but clang++-16 exists.
    """
    build_script = Path(build_script)

    if not build_script.exists():
        return

    candidate_paths = [
        "/tmp/bambu_appimage/squashfs-root/usr/compilers/clang-16/bin/clang++-16",
        "/usr/bin/clang++-16",
    ]

    compiler = None
    for p in candidate_paths:
        if Path(p).exists():
            compiler = p
            break

    if compiler is None:
        return

    text = build_script.read_text()

    if 'FALLBACK_CC="g++"' in text:
        text = text.replace('FALLBACK_CC="g++"', f'FALLBACK_CC="{compiler}"')
        build_script.write_text(text)
        print(f"[OK] Patched fallback compiler to: {compiler}")


# ─────────────────────────────────────────────
# RUN BAMBU
# ─────────────────────────────────────────────
def run_bambu(output_dir, timeout):
    output_dir = Path(output_dir)
    build_script = output_dir / "build_bambu.sh"
    log_path = output_dir / "bambu_build.log"

    if not build_script.exists():
        raise FileNotFoundError(f"Missing build script: {build_script}")

    print("\n[INFO] Running Bambu...")
    print(f"[INFO] Log: {log_path}")

    with open(log_path, "w", encoding="utf-8") as log:
        log.write("Command: bash build_bambu.sh\n")
        log.write(f"Working directory: {output_dir}\n\n")
        log.flush()

        try:
            proc = subprocess.run(
                ["bash", "build_bambu.sh"],
                cwd=str(output_dir),
                stdout=log,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=timeout,
            )

            if proc.returncode == 0:
                print("[PASS] Bambu finished successfully.")
                return True

            print(f"[FAIL] Bambu failed with return code {proc.returncode}.")
            return False

        except subprocess.TimeoutExpired:
            print("[TIMEOUT] Bambu timed out.")
            return False


# ─────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("--model", required=True)
    parser.add_argument("--output-dir", default="hls_full_model_simple32")
    parser.add_argument("--precision", default="fixed<20,6>")
    parser.add_argument("--reuse-factor", type=int, default=9)
    parser.add_argument(
        "--io-type",
        default="io_parallel",
        choices=["io_parallel", "io_stream", "io_serial"],
    )
    parser.add_argument("--timeout", type=int, default=3600)
    parser.add_argument("--no-run-bambu", action="store_true")
    parser.add_argument("--no-selected-precisions", action="store_true")

    args = parser.parse_args()

    output_dir = Path(args.output_dir)

    if output_dir.exists():
        shutil.rmtree(output_dir)

    print("Available backends:", hls4ml.backends.get_available_backends())
    print(f"Model      : {args.model}")
    print(f"Output dir : {output_dir}")
    print(f"Precision  : {args.precision}")
    print(f"ReuseFactor: {args.reuse_factor}")
    print(f"IOType     : {args.io_type}")

    model = load_model_compatible(args.model)

    print("\nKeras model summary:")
    model.summary()

    print("\nModel input shape:", model.input_shape)

    if model.input_shape != EXPECTED_INPUT_SHAPE:
        raise RuntimeError(
            f"Wrong model input shape. Expected {EXPECTED_INPUT_SHAPE}, got {model.input_shape}"
        )

    config = make_hls_config(
        model=model,
        precision=args.precision,
        reuse_factor=args.reuse_factor,
        io_type=args.io_type,
        use_selected_precisions=not args.no_selected_precisions,
    )

    hls_model = hls4ml.converters.convert_from_keras_model(
        model,
        hls_config=config,
        output_dir=str(output_dir),
        project_name=PROJECT_NAME,
        backend=BACKEND,
        part=PART,
        io_type=args.io_type,
    )

    print("\n[INFO] Writing hls4ml project...")
    hls_model.write()

    print("[INFO] Generating firmware project files...")
    hls_model.build(
        reset=False,
        csim=False,
        synth=False,
    )

    cpp_path = output_dir / "firmware" / f"{PROJECT_NAME}.cpp"

    verify_generated_cpp_is_32x32(cpp_path)
    patch_interface_pragmas(cpp_path)

    patch_build_script_compiler(output_dir / "build_bambu.sh")
    patch_build_script_compiler(output_dir / "build_lib.sh")


    print("\n[OK] Generated project.")
    print(f"Firmware : {output_dir / 'firmware'}")
    print(f"C++ file : {cpp_path}")

    print("\nCheck generated shape:")
    print(f'grep -n "void myproject" -A5 {cpp_path}')

    print("\nCheck weights:")
    print(f'grep -n "load_weights_from_txt" {cpp_path}')

    print("\nCheck Bambu flag:")
    print(f'grep -n "register-allocation\\|bambu" {output_dir / "build_bambu.sh"}')

    if args.no_run_bambu:
        print("\n[INFO] Skipping Bambu.")
        print(f"Run manually with: cd {output_dir} && bash build_bambu.sh")
        return

    ok = run_bambu(output_dir, timeout=args.timeout)

    if ok:
        print("\n[DONE] Bambu build finished.")
    else:
        print("\n[NOT DONE] Check log:")
        print(output_dir / "bambu_build.log")


if __name__ == "__main__":
    main()