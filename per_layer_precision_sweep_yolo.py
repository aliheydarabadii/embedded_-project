#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
from glob import glob
from pathlib import Path
from typing import Any

os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "2")

import numpy as np
import tensorflow as tf
from PIL import Image

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


# ─────────────────────────────────────────────
# CONFIG
# ─────────────────────────────────────────────

SCRIPT_DIR = Path(__file__).resolve().parent

MODEL_CANDIDATES = [
    SCRIPT_DIR / "new_model_16.h5",
    SCRIPT_DIR / "new_model_16.keras",
    SCRIPT_DIR / "new_model_32.h5",
    SCRIPT_DIR / "new_model_32.keras",
]

DEFAULT_DATA_DIR = SCRIPT_DIR / "dataset"

DEFAULT_OUTPUT_ROOT = Path("/tmp/hls4ml_new_model_check")
DEFAULT_CSV = SCRIPT_DIR / "new_model_predictions.csv"
DEFAULT_TRIALS_CSV = SCRIPT_DIR / "new_model_precision_trials.csv"
DEFAULT_SELECTION_JSON = SCRIPT_DIR / "new_model_precision_selection.json"

PART = "LFE5UM85F8BG756C"
BACKEND = "bambu"
PROJECT_NAME = "myproject"

DEFAULT_LOAD_SIZE = 64
DEFAULT_THRESHOLD = 0.50
DEFAULT_REUSE_FACTOR = 9

DEFAULT_BASE_PRECISION = "fixed<20,6>"
DEFAULT_SAFE_PRECISION = "fixed<28,10>"

DEFAULT_CANDIDATES = [
    "fixed<12,4>",
    "fixed<14,4>",
    "fixed<16,5>",
    "fixed<18,5>",
    "fixed<20,6>",
    "fixed<22,8>",
    "fixed<24,10>",
    "fixed<26,12>",
    "fixed<28,10>",
]

IMAGE_EXTS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}


# ─────────────────────────────────────────────
# MODEL LOADING
# ─────────────────────────────────────────────

def find_model_path(user_model_path: Path | None) -> Path:
    if user_model_path is not None:
        if not user_model_path.exists():
            raise FileNotFoundError(f"Model not found: {user_model_path}")
        return user_model_path

    for path in MODEL_CANDIDATES:
        if path.exists():
            return path

    raise FileNotFoundError(
        "Could not find model file. Checked:\n"
        + "\n".join(str(p) for p in MODEL_CANDIDATES)
    )


def build_simple32_pooled_12_model() -> tf.keras.Model:
    """
    Exact fallback architecture for the current model.

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


def load_keras_model_compatible(model_path: Path) -> tf.keras.Model:
    """
    Try to load the full model first.
    If Keras version incompatibility happens, rebuild the exact fallback model and load weights.
    """
    print(f"[INFO] Loading model: {model_path}")

    try:
        model = tf.keras.models.load_model(model_path, compile=False)
        print("[OK] Loaded full Keras model.")

    except Exception as e:
        print("\n[WARN] Full model loading failed.")
        print("[WARN] Rebuilding exact simple32 pooled model and loading weights.")
        print(f"[WARN] Original error: {repr(e)}\n")

        model = build_simple32_pooled_12_model()
        model.load_weights(model_path)

        compat_path = Path(f"/tmp/{model_path.stem}_simple32_pooled_tf_compatible.h5")
        model.save(compat_path)
        print(f"[OK] Saved compatible copy to: {compat_path}")

    model.summary()

    input_shape = model.input_shape
    print("\nModel input shape:", input_shape)

    if input_shape != (None, 32, 32, 1):
        raise RuntimeError(
            f"Wrong model input shape. Expected (None, 32, 32, 1), got {input_shape}."
        )

    return model


def get_model_hw(model: tf.keras.Model) -> tuple[int, int]:
    return int(model.input_shape[1]), int(model.input_shape[2])


# ─────────────────────────────────────────────
# PREPROCESSING
# ─────────────────────────────────────────────

def preprocess_image(path: Path, input_hw: tuple[int, int], load_size: int) -> np.ndarray:
    """
    RGB image
    -> resize to load_size x load_size
    -> normalize to [0, 1]
    -> BT.601 grayscale:
       gray = (77R + 150G + 29B) / 256
    -> resize to model input size
    -> shape: (H, W, 1)
    """
    img = Image.open(path).convert("RGB").resize((load_size, load_size), Image.BILINEAR)
    arr = np.asarray(img, dtype=np.float32) / 255.0

    r = arr[:, :, 0:1]
    g = arr[:, :, 1:2]
    b = arr[:, :, 2:3]

    gray = (
        0.30078125 * r +
        0.58593750 * g +
        0.11328125 * b
    )

    gray_tf = tf.convert_to_tensor(gray, dtype=tf.float32)
    gray_resized = tf.image.resize(gray_tf, input_hw, method="bilinear").numpy()

    return gray_resized.reshape(input_hw[0], input_hw[1], 1).astype(np.float32)


def debug_preprocess_image(
    path: Path,
    input_hw: tuple[int, int],
    load_size: int,
    out_path: Path,
) -> None:
    img_original = Image.open(path).convert("RGB")
    img_load = img_original.resize((load_size, load_size), Image.BILINEAR)

    arr = np.asarray(img_load, dtype=np.float32) / 255.0

    gray = (
        0.30078125 * arr[:, :, 0] +
        0.58593750 * arr[:, :, 1] +
        0.11328125 * arr[:, :, 2]
    )

    x = preprocess_image(path, input_hw=input_hw, load_size=load_size)

    plt.figure(figsize=(10, 4))

    plt.subplot(1, 3, 1)
    plt.imshow(img_original)
    plt.title("Original")
    plt.axis("off")

    plt.subplot(1, 3, 2)
    plt.imshow(gray, cmap="gray", vmin=0, vmax=1)
    plt.title(f"{load_size}x{load_size} grayscale")
    plt.axis("off")

    plt.subplot(1, 3, 3)
    plt.imshow(x.squeeze(), cmap="gray", vmin=0, vmax=1)
    plt.title(f"{input_hw[0]}x{input_hw[1]} model input")
    plt.axis("off")

    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=160)
    plt.close()


# ─────────────────────────────────────────────
# DATASET LOADING
# ─────────────────────────────────────────────

def yolo_label_to_binary(label_path: Path) -> int:
    if not label_path.exists():
        raise FileNotFoundError(f"Missing label file: {label_path}")

    text = label_path.read_text().strip()
    return 1 if text else 0


def find_folder_dataset_root(
    data_dir: Path,
    split: str | None,
    class_names: list[str],
) -> Path:
    candidates = []

    if split:
        candidates.append(data_dir / split)

    candidates.append(data_dir)

    for root in candidates:
        if all((root / cls).is_dir() for cls in class_names):
            return root

    raise FileNotFoundError(
        "Could not find folder dataset with class subfolders.\n"
        f"Expected: {class_names}\n"
        f"Checked: {[str(c) for c in candidates]}"
    )


def load_folder_dataset(
    data_dir: Path,
    split: str | None,
    class_names: list[str],
    input_hw: tuple[int, int],
    load_size: int,
    max_images: int | None,
):
    root = find_folder_dataset_root(data_dir, split, class_names)

    x_list = []
    y_list = []
    paths = []

    for label, cls in enumerate(class_names):
        folder = root / cls

        img_paths = [
            p for p in sorted(folder.iterdir())
            if p.is_file() and p.suffix.lower() in IMAGE_EXTS
        ]

        for img_path in img_paths:
            x_list.append(preprocess_image(img_path, input_hw=input_hw, load_size=load_size))
            y_list.append(label)
            paths.append(str(img_path))

            if max_images is not None and len(paths) >= max_images:
                break

        if max_images is not None and len(paths) >= max_images:
            break

    if not x_list:
        raise RuntimeError(f"No images found in folder dataset root: {root}")

    x_test = np.ascontiguousarray(np.stack(x_list, axis=0), dtype=np.float32)
    y_true = np.asarray(y_list, dtype=np.int32)

    print(f"\nLoaded folder dataset: {root}")
    print(f"Images       : {len(y_true)}")
    print(f"Non-person   : {int(np.sum(y_true == 0))}")
    print(f"Person       : {int(np.sum(y_true == 1))}")
    print(f"Input tensor : {x_test.shape}, {x_test.dtype}")
    print(f"Pixel range  : {x_test.min():.6f} -> {x_test.max():.6f}")

    return x_test, y_true, paths


def load_yolo_split(
    data_dir: Path,
    split: str,
    input_hw: tuple[int, int],
    load_size: int,
    max_images: int | None,
):
    image_dir = data_dir / "images" / split
    label_dir = data_dir / "labels" / split

    if not image_dir.exists():
        raise FileNotFoundError(f"Missing image directory: {image_dir}")

    if not label_dir.exists():
        raise FileNotFoundError(f"Missing label directory: {label_dir}")

    image_paths = [
        p for p in sorted(image_dir.iterdir())
        if p.is_file() and p.suffix.lower() in IMAGE_EXTS
    ]

    if max_images is not None:
        image_paths = image_paths[:max_images]

    x_list = []
    y_list = []
    paths = []

    for img_path in image_paths:
        label_path = label_dir / f"{img_path.stem}.txt"

        x_list.append(preprocess_image(img_path, input_hw=input_hw, load_size=load_size))
        y_list.append(yolo_label_to_binary(label_path))
        paths.append(str(img_path))

    if not x_list:
        raise RuntimeError(f"No images found in {image_dir}")

    x_test = np.ascontiguousarray(np.stack(x_list, axis=0), dtype=np.float32)
    y_true = np.asarray(y_list, dtype=np.int32)

    print(f"\nLoaded YOLO split: {split}")
    print(f"Images       : {len(y_true)}")
    print(f"Non-person   : {int(np.sum(y_true == 0))}")
    print(f"Person       : {int(np.sum(y_true == 1))}")
    print(f"Input tensor : {x_test.shape}, {x_test.dtype}")
    print(f"Pixel range  : {x_test.min():.6f} -> {x_test.max():.6f}")

    return x_test, y_true, paths


def detect_dataset_format(data_dir: Path, split: str, class_names: list[str]) -> str:
    try:
        _ = find_folder_dataset_root(data_dir, split, class_names)
        return "folder"
    except FileNotFoundError:
        pass

    if (data_dir / "images" / split).is_dir() and (data_dir / "labels" / split).is_dir():
        return "yolo"

    raise FileNotFoundError(
        f"Could not auto-detect dataset format under: {data_dir}\n"
        "Expected either class folders or YOLO images/labels folders."
    )


def load_dataset(
    data_dir: Path,
    dataset_format: str,
    split: str,
    class_names: list[str],
    input_hw: tuple[int, int],
    load_size: int,
    max_images: int | None,
):
    if dataset_format == "auto":
        dataset_format = detect_dataset_format(data_dir, split, class_names)
        print(f"[INFO] Auto-detected dataset format: {dataset_format}")

    if dataset_format == "folder":
        return load_folder_dataset(
            data_dir=data_dir,
            split=split,
            class_names=class_names,
            input_hw=input_hw,
            load_size=load_size,
            max_images=max_images,
        )

    if dataset_format == "yolo":
        return load_yolo_split(
            data_dir=data_dir,
            split=split,
            input_hw=input_hw,
            load_size=load_size,
            max_images=max_images,
        )

    raise ValueError(f"Unknown dataset format: {dataset_format}")


def load_test_images(
    image_paths: list[Path],
    image_dir: Path | None,
    input_hw: tuple[int, int],
    load_size: int,
):
    all_paths = list(image_paths)

    if image_dir is not None:
        if not image_dir.is_dir():
            raise FileNotFoundError(f"Image directory not found: {image_dir}")

        for p in sorted(image_dir.iterdir()):
            if p.is_file() and p.suffix.lower() in IMAGE_EXTS:
                all_paths.append(p)

    if not all_paths:
        raise ValueError("No test images were provided.")

    x_list = []
    paths = []

    for img_path in all_paths:
        if not img_path.exists():
            raise FileNotFoundError(f"Image not found: {img_path}")

        x_list.append(preprocess_image(img_path, input_hw=input_hw, load_size=load_size))
        paths.append(str(img_path))

    x = np.ascontiguousarray(np.stack(x_list, axis=0), dtype=np.float32)

    print(f"\nLoaded test images: {len(paths)}")
    print(f"Input tensor : {x.shape}, {x.dtype}")
    print(f"Pixel range  : {x.min():.6f} -> {x.max():.6f}")

    return x, None, paths


# ─────────────────────────────────────────────
# METRICS
# ─────────────────────────────────────────────

def labels_from_scores(scores: np.ndarray, threshold: float) -> np.ndarray:
    return (scores >= threshold).astype(np.int32)


def classification_metrics(
    scores: np.ndarray,
    y_true: np.ndarray,
    threshold: float,
) -> dict[str, Any]:
    labels = labels_from_scores(scores, threshold)

    tp = int(np.sum((y_true == 1) & (labels == 1)))
    tn = int(np.sum((y_true == 0) & (labels == 0)))
    fp = int(np.sum((y_true == 0) & (labels == 1)))
    fn = int(np.sum((y_true == 1) & (labels == 0)))

    accuracy = (tp + tn) / max(1, tp + tn + fp + fn)
    precision = tp / max(1, tp + fp)
    recall = tp / max(1, tp + fn)
    f1 = 2 * precision * recall / max(1e-8, precision + recall)

    non_person_precision = tn / max(1, tn + fn)
    non_person_recall = tn / max(1, tn + fp)
    non_person_f1 = (
        2 * non_person_precision * non_person_recall /
        max(1e-8, non_person_precision + non_person_recall)
    )

    return {
        "accuracy": float(accuracy),
        "precision": float(precision),
        "recall": float(recall),
        "f1": float(f1),
        "non_person_precision": float(non_person_precision),
        "non_person_recall": float(non_person_recall),
        "non_person_f1": float(non_person_f1),
        "tp": tp,
        "tn": tn,
        "fp": fp,
        "fn": fn,
    }


def print_confusion_and_report(
    scores: np.ndarray,
    y_true: np.ndarray,
    threshold: float,
    class_names: list[str],
) -> None:
    labels = labels_from_scores(scores, threshold)
    metrics = classification_metrics(scores, y_true, threshold)

    print("\nConfusion Matrix:")
    print(np.array([
        [metrics["tn"], metrics["fp"]],
        [metrics["fn"], metrics["tp"]],
    ]))

    print("\nBasic report:")
    print(
        f"{class_names[0]:>12s}  "
        f"precision={metrics['non_person_precision']:.4f}  "
        f"recall={metrics['non_person_recall']:.4f}  "
        f"f1={metrics['non_person_f1']:.4f}"
    )
    print(
        f"{class_names[1]:>12s}  "
        f"precision={metrics['precision']:.4f}  "
        f"recall={metrics['recall']:.4f}  "
        f"f1={metrics['f1']:.4f}"
    )
    print(f"\naccuracy={metrics['accuracy']:.4f}")

    try:
        from sklearn.metrics import classification_report
        print("\nClassification Report:")
        print(classification_report(
            y_true,
            labels,
            target_names=class_names,
            zero_division=0,
        ))
    except Exception:
        pass


def compare_metrics(
    keras_pred: np.ndarray,
    hls_pred: np.ndarray,
    y_true: np.ndarray,
    threshold: float,
) -> dict[str, Any]:
    keras_labels = labels_from_scores(keras_pred, threshold)
    hls_labels = labels_from_scores(hls_pred, threshold)

    return {
        "agreement": float(np.mean(keras_labels == hls_labels)),
        "mae": float(np.mean(np.abs(keras_pred - hls_pred))),
        "max_error": float(np.max(np.abs(keras_pred - hls_pred))),
        "num_disagreements": int(np.sum(keras_labels != hls_labels)),
        "keras": classification_metrics(keras_pred, y_true, threshold),
        "hls4ml": classification_metrics(hls_pred, y_true, threshold),
    }


def print_comparison_summary(name: str, metrics: dict[str, Any]) -> None:
    print(f"\n{name}")
    print("-" * len(name))
    print(f"agreement        : {metrics['agreement']:.4f}")
    print(f"MAE              : {metrics['mae']:.6f}")
    print(f"max error        : {metrics['max_error']:.6f}")
    print(f"disagreements    : {metrics['num_disagreements']}")

    k = metrics["keras"]
    h = metrics["hls4ml"]

    print(
        f"Keras  acc/prec/rec/f1: "
        f"{k['accuracy']:.4f} / {k['precision']:.4f} / {k['recall']:.4f} / {k['f1']:.4f} "
        f"TP={k['tp']} TN={k['tn']} FP={k['fp']} FN={k['fn']}"
    )

    print(
        f"hls4ml acc/prec/rec/f1: "
        f"{h['accuracy']:.4f} / {h['precision']:.4f} / {h['recall']:.4f} / {h['f1']:.4f} "
        f"TP={h['tp']} TN={h['tn']} FP={h['fp']} FN={h['fn']}"
    )


def print_single_predictions(
    paths: list[str],
    scores: np.ndarray,
    threshold: float,
    max_print: int = 30,
) -> None:
    print("\nPredictions sample:")

    n = min(len(paths), max_print)

    for path, score in zip(paths[:n], scores[:n]):
        label = "PERSON" if score >= threshold else "NO PERSON"
        print(f"  {Path(path).name}: P(person)={score:.4f} -> {label}")

    if len(paths) > max_print:
        print(f"  ... printed {max_print}/{len(paths)} predictions")


def threshold_sweep(
    scores: np.ndarray,
    y_true: np.ndarray,
    thresholds: list[float],
) -> None:
    print("\nThreshold sweep:")
    print("threshold   acc      precision  recall   f1      TP   TN   FP   FN")

    for t in thresholds:
        m = classification_metrics(scores, y_true, t)
        print(
            f"{t:8.2f}   "
            f"{m['accuracy']:.4f}   "
            f"{m['precision']:.4f}     "
            f"{m['recall']:.4f}  "
            f"{m['f1']:.4f}  "
            f"{m['tp']:4d} {m['tn']:4d} {m['fp']:4d} {m['fn']:4d}"
        )


# ─────────────────────────────────────────────
# MISCLASSIFIED IMAGES
# ─────────────────────────────────────────────

def save_grid(
    indices: np.ndarray,
    paths: list[str],
    scores: np.ndarray,
    y_true: np.ndarray,
    y_pred: np.ndarray,
    out_path: Path,
    title: str,
    input_hw: tuple[int, int],
    load_size: int,
    max_images: int,
    show_preprocessed: bool,
) -> None:
    if len(indices) == 0:
        print(f"[INFO] No images for: {title}")
        return

    out_path.parent.mkdir(parents=True, exist_ok=True)

    n = min(len(indices), max_images)
    cols = 4
    rows = int(np.ceil(n / cols))

    plt.figure(figsize=(cols * 3.5, rows * 3.5))

    for i, idx in enumerate(indices[:n]):
        path = Path(paths[idx])

        plt.subplot(rows, cols, i + 1)

        if show_preprocessed:
            x = preprocess_image(path, input_hw=input_hw, load_size=load_size)
            plt.imshow(x.squeeze(), cmap="gray", vmin=0, vmax=1)
        else:
            img = Image.open(path).convert("RGB")
            plt.imshow(img)

        true_label = "person" if y_true[idx] == 1 else "non_person"
        pred_label = "person" if y_pred[idx] == 1 else "non_person"

        plt.title(
            f"{path.name}\nT:{true_label} P:{pred_label}\nscore={scores[idx]:.3f}",
            fontsize=8,
        )
        plt.axis("off")

    plt.suptitle(title, fontsize=14)
    plt.tight_layout()
    plt.savefig(out_path, dpi=160)
    plt.close()

    print(f"[OK] Saved: {out_path}")


def copy_misclassified_images(
    indices: np.ndarray,
    paths: list[str],
    scores: np.ndarray,
    y_true: np.ndarray,
    y_pred: np.ndarray,
    out_dir: Path,
) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)

    for idx in indices:
        src = Path(paths[idx])
        true_label = "person" if y_true[idx] == 1 else "non_person"
        pred_label = "person" if y_pred[idx] == 1 else "non_person"

        dst_name = (
            f"true-{true_label}__pred-{pred_label}__"
            f"score-{scores[idx]:.4f}__{src.name}"
        )

        shutil.copy2(src, out_dir / dst_name)


def save_misclassified_outputs(
    paths: list[str],
    scores: np.ndarray,
    y_true: np.ndarray,
    threshold: float,
    output_root: Path,
    input_hw: tuple[int, int],
    load_size: int,
    max_images: int,
) -> None:
    y_pred = labels_from_scores(scores, threshold)

    false_positives = np.where((y_true == 0) & (y_pred == 1))[0]
    false_negatives = np.where((y_true == 1) & (y_pred == 0))[0]
    all_mistakes = np.where(y_true != y_pred)[0]

    print("\nMisclassified images:")
    print(f"  False positives: {len(false_positives)}")
    print(f"  False negatives: {len(false_negatives)}")
    print(f"  Total mistakes : {len(all_mistakes)}")

    mis_dir = output_root / "misclassified"

    copy_misclassified_images(false_positives, paths, scores, y_true, y_pred, mis_dir / "false_positives")
    copy_misclassified_images(false_negatives, paths, scores, y_true, y_pred, mis_dir / "false_negatives")

    save_grid(
        false_positives,
        paths,
        scores,
        y_true,
        y_pred,
        out_path=mis_dir / "false_positives_original.png",
        title="False Positives: non_person predicted as person",
        input_hw=input_hw,
        load_size=load_size,
        max_images=max_images,
        show_preprocessed=False,
    )

    save_grid(
        false_negatives,
        paths,
        scores,
        y_true,
        y_pred,
        out_path=mis_dir / "false_negatives_original.png",
        title="False Negatives: person predicted as non_person",
        input_hw=input_hw,
        load_size=load_size,
        max_images=max_images,
        show_preprocessed=False,
    )

    save_grid(
        all_mistakes,
        paths,
        scores,
        y_true,
        y_pred,
        out_path=mis_dir / "all_mistakes_preprocessed.png",
        title="All Misclassified Images: model input view",
        input_hw=input_hw,
        load_size=load_size,
        max_images=max_images,
        show_preprocessed=True,
    )


# ─────────────────────────────────────────────
# HLS4ML PATCHING
# ─────────────────────────────────────────────

def find_cpp16_compiler() -> str | None:
    candidates = [
        "/tmp/bambu_appimage/squashfs-root/usr/compilers/clang-16/bin/clang++-16",
        *glob("/tmp/.mount_bambu*/usr/bin/clang++-16"),
    ]

    for candidate in candidates:
        if Path(candidate).is_file() and os.access(candidate, os.X_OK):
            return candidate

    for candidate in ("clang++-16", "g++-13", "g++"):
        resolved = shutil.which(candidate)
        if resolved:
            return resolved

    return None


def patch_build_lib(build_lib: Path) -> None:
    if not build_lib.exists():
        return

    compiler = find_cpp16_compiler()

    if compiler is None:
        print("[WARN] Could not find clang++-16/g++ compiler.")
        return

    text = build_lib.read_text()

    text = text.replace(
        'FALLBACK_CC="g++"',
        f'FALLBACK_CC="{compiler}"',
    )

    old_block = """if [ -n "$MOUNT_DIR" ] && [ -x "$MOUNT_DIR/usr/bin/clang++-16" ]; then
    CC="$MOUNT_DIR/usr/bin/clang++-16"
else
    echo "Bambu AppImage not detected. Using fallback compiler."
    CC="$FALLBACK_CC"
fi
"""

    new_block = """if [ -n "${CXX:-}" ]; then
    CC="$CXX"
elif [ -n "$MOUNT_DIR" ] && [ -x "$MOUNT_DIR/usr/bin/clang++-16" ]; then
    CC="$MOUNT_DIR/usr/bin/clang++-16"
elif [ -x "/tmp/bambu_appimage/squashfs-root/usr/compilers/clang-16/bin/clang++-16" ]; then
    CC="/tmp/bambu_appimage/squashfs-root/usr/compilers/clang-16/bin/clang++-16"
else
    echo "Bambu AppImage not detected. Using fallback compiler."
    CC="$FALLBACK_CC"
fi
"""

    text = text.replace(old_block, new_block)
    build_lib.write_text(text)


def patch_bambu_stream_pragma() -> None:
    try:
        from hls4ml.writer.bambu_writer import BambuWriter
    except Exception:
        return

    if getattr(BambuWriter, "_stream_depth_default_patch", False):
        return

    original = BambuWriter._make_array_pragma

    @staticmethod
    def patched_make_array_pragma(variable):
        if variable.pragma == "stream":
            depth = variable.size() if hasattr(variable, "size") else 0
            return f"//#pragma HLS STREAM variable={variable.name} depth={depth}"
        return original(variable)

    BambuWriter._make_array_pragma = patched_make_array_pragma
    BambuWriter._stream_depth_default_patch = True


def patch_bambu_interface_pragmas(cpp_path: Path) -> None:
    if not cpp_path.exists():
        return

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


# ─────────────────────────────────────────────
# HLS CONFIG HELPERS
# ─────────────────────────────────────────────

def make_base_hls_config(
    model: tf.keras.Model,
    base_precision: str,
    reuse_factor: int,
    io_type: str,
):
    import hls4ml

    config = hls4ml.utils.config_from_keras_model(
        model,
        granularity="name",
        default_precision=base_precision,
        backend=BACKEND,
    )

    config["Model"]["Precision"] = base_precision
    config["Model"]["ReuseFactor"] = reuse_factor
    config["Model"]["Strategy"] = "Resource"
    config["Model"]["IOType"] = io_type

    return config


def get_layer_precision_dict(
    config: dict[str, Any],
    layer_name: str,
) -> dict[str, str] | None:
    layer_cfg = config.get("LayerName", {}).get(layer_name)

    if layer_cfg is None:
        return None

    precision_cfg = layer_cfg.get("Precision")

    if isinstance(precision_cfg, dict):
        return precision_cfg

    return None


def get_precision_layer_names(config: dict[str, Any]) -> list[str]:
    names = []

    for lname, lcfg in config.get("LayerName", {}).items():
        if isinstance(lcfg.get("Precision"), dict):
            names.append(lname)

    return names


def apply_precision_selection(
    config: dict[str, Any],
    selected_precisions: dict[str, str],
    keys_mode: str,
    only_keys: list[str],
) -> None:
    for lname, precision in selected_precisions.items():
        precision_cfg = get_layer_precision_dict(config, lname)

        if precision_cfg is None:
            continue

        if keys_mode == "all":
            keys = list(precision_cfg.keys())
        else:
            keys = [k for k in only_keys if k in precision_cfg]

        for key in keys:
            precision_cfg[key] = precision


def print_layer_precision_config(config: dict[str, Any]) -> None:
    print("\nLayer precision config:")

    for lname, lcfg in config.get("LayerName", {}).items():
        precision_cfg = lcfg.get("Precision", {})
        print(f"  {lname:30s} {precision_cfg}")


# ─────────────────────────────────────────────
# HLS MODEL BUILD/PREDICT
# ─────────────────────────────────────────────

def make_hls_model_with_selection(
    model: tf.keras.Model,
    output_dir: Path,
    base_precision: str,
    selected_precisions: dict[str, str],
    reuse_factor: int,
    io_type: str,
    keys_mode: str,
    only_keys: list[str],
    clean: bool,
    print_config: bool = False,
):
    import hls4ml

    if clean and output_dir.exists():
        shutil.rmtree(output_dir)

    config = make_base_hls_config(
        model=model,
        base_precision=base_precision,
        reuse_factor=reuse_factor,
        io_type=io_type,
    )

    apply_precision_selection(
        config=config,
        selected_precisions=selected_precisions,
        keys_mode=keys_mode,
        only_keys=only_keys,
    )

    if print_config:
        print_layer_precision_config(config)

    hls_model = hls4ml.converters.convert_from_keras_model(
        model,
        hls_config=config,
        output_dir=str(output_dir),
        project_name=PROJECT_NAME,
        backend=BACKEND,
        part=PART,
        io_type=io_type,
    )

    patch_bambu_stream_pragma()

    hls_model.write()
    patch_build_lib(output_dir / "build_lib.sh")
    patch_bambu_interface_pragmas(output_dir / "firmware" / f"{PROJECT_NAME}.cpp")

    original_write = hls_model.write
    hls_model.write = lambda: None

    try:
        hls_model.compile()
    finally:
        hls_model.write = original_write

    return hls_model, config


def run_hls_prediction(
    model: tf.keras.Model,
    x_test: np.ndarray,
    output_dir: Path,
    base_precision: str,
    selected_precisions: dict[str, str],
    reuse_factor: int,
    io_type: str,
    keys_mode: str,
    only_keys: list[str],
    clean: bool,
    print_config: bool = False,
) -> tuple[np.ndarray, dict[str, Any]]:
    hls_model, config = make_hls_model_with_selection(
        model=model,
        output_dir=output_dir,
        base_precision=base_precision,
        selected_precisions=selected_precisions,
        reuse_factor=reuse_factor,
        io_type=io_type,
        keys_mode=keys_mode,
        only_keys=only_keys,
        clean=clean,
        print_config=print_config,
    )

    hls_pred = np.asarray(
        hls_model.predict(np.ascontiguousarray(x_test))
    ).reshape(-1)

    return hls_pred, config


# ─────────────────────────────────────────────
# PRECISION SEARCH
# ─────────────────────────────────────────────

def passes_acceptance(
    metrics: dict[str, Any],
    min_agreement: float,
    max_mae: float,
    max_error: float,
) -> bool:
    return (
        metrics["agreement"] >= min_agreement
        and metrics["mae"] <= max_mae
        and metrics["max_error"] <= max_error
    )


def sanitize_name(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]+", "_", name)


def greedy_layer_precision_search(
    model: tf.keras.Model,
    x_test: np.ndarray,
    y_true: np.ndarray,
    keras_pred: np.ndarray,
    layer_names: list[str],
    output_root: Path,
    base_precision: str,
    safe_precision: str,
    candidates: list[str],
    reuse_factor: int,
    io_type: str,
    threshold: float,
    min_agreement: float,
    max_mae: float,
    max_error: float,
    keys_mode: str,
    only_keys: list[str],
    continue_on_fail: bool,
) -> tuple[dict[str, str], list[dict[str, Any]]]:
    selected = {lname: safe_precision for lname in layer_names}
    all_trials: list[dict[str, Any]] = []

    print("\nInitial selected precision:")
    for lname in layer_names:
        print(f"  {lname}: {selected[lname]}")

    for layer_index, lname in enumerate(layer_names):
        print("\n" + "=" * 100)
        print(f"Tuning layer {layer_index + 1}/{len(layer_names)}: {lname}")
        print("=" * 100)

        best_for_layer = None

        for candidate in candidates:
            trial_selection = dict(selected)
            trial_selection[lname] = candidate

            tag = (
                f"{layer_index:02d}_{sanitize_name(lname)}_"
                f"{candidate.replace('<','').replace('>','').replace(',','_')}"
            )
            output_dir = output_root / tag

            print(f"\n[TRIAL] layer={lname} candidate={candidate}")
            print(f"[TRIAL] output_dir={output_dir}")

            try:
                hls_pred, _config = run_hls_prediction(
                    model=model,
                    x_test=x_test,
                    output_dir=output_dir,
                    base_precision=base_precision,
                    selected_precisions=trial_selection,
                    reuse_factor=reuse_factor,
                    io_type=io_type,
                    keys_mode=keys_mode,
                    only_keys=only_keys,
                    clean=True,
                    print_config=False,
                )

                metrics = compare_metrics(
                    keras_pred=keras_pred,
                    hls_pred=hls_pred,
                    y_true=y_true,
                    threshold=threshold,
                )

                ok = passes_acceptance(
                    metrics=metrics,
                    min_agreement=min_agreement,
                    max_mae=max_mae,
                    max_error=max_error,
                )

                print_comparison_summary(
                    f"Result for {lname} = {candidate}  [{'PASS' if ok else 'FAIL'}]",
                    metrics,
                )

                trial_row = {
                    "layer_index": layer_index,
                    "layer_name": lname,
                    "candidate": candidate,
                    "status": "PASS" if ok else "FAIL",
                    "agreement": metrics["agreement"],
                    "mae": metrics["mae"],
                    "max_error": metrics["max_error"],
                    "num_disagreements": metrics["num_disagreements"],
                    "keras_accuracy": metrics["keras"]["accuracy"],
                    "keras_precision": metrics["keras"]["precision"],
                    "keras_recall": metrics["keras"]["recall"],
                    "keras_f1": metrics["keras"]["f1"],
                    "hls_accuracy": metrics["hls4ml"]["accuracy"],
                    "hls_precision": metrics["hls4ml"]["precision"],
                    "hls_recall": metrics["hls4ml"]["recall"],
                    "hls_f1": metrics["hls4ml"]["f1"],
                    "output_dir": str(output_dir),
                }

                all_trials.append(trial_row)

                if ok:
                    best_for_layer = candidate
                    selected[lname] = candidate
                    print(f"[SELECT] {lname} -> {candidate}")
                    break

            except Exception as e:
                print(f"[ERROR] Trial failed for {lname}={candidate}: {repr(e)}")

                all_trials.append({
                    "layer_index": layer_index,
                    "layer_name": lname,
                    "candidate": candidate,
                    "status": "ERROR",
                    "agreement": "",
                    "mae": "",
                    "max_error": "",
                    "num_disagreements": "",
                    "keras_accuracy": "",
                    "keras_precision": "",
                    "keras_recall": "",
                    "keras_f1": "",
                    "hls_accuracy": "",
                    "hls_precision": "",
                    "hls_recall": "",
                    "hls_f1": "",
                    "output_dir": str(output_root / f"{layer_index:02d}_{sanitize_name(lname)}_ERROR"),
                    "error": repr(e),
                })

                if not continue_on_fail:
                    raise

        if best_for_layer is None:
            print(f"[WARN] No candidate passed for layer {lname}. Keeping safe precision: {safe_precision}")
            selected[lname] = safe_precision

    return selected, all_trials


# ─────────────────────────────────────────────
# OUTPUT FILES
# ─────────────────────────────────────────────

def write_rows_csv(path: Path, rows: list[dict[str, Any]]) -> None:
    if not rows:
        return

    path.parent.mkdir(parents=True, exist_ok=True)

    fieldnames = sorted(set().union(*(row.keys() for row in rows)))

    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

        for row in rows:
            writer.writerow(row)


def write_prediction_csv(
    csv_path: Path,
    paths: list[str],
    y_true: np.ndarray | None,
    keras_pred: np.ndarray,
    threshold: float,
    hls_pred: np.ndarray | None = None,
) -> None:
    csv_path.parent.mkdir(parents=True, exist_ok=True)

    keras_labels = labels_from_scores(keras_pred, threshold)

    if hls_pred is not None:
        hls_labels = labels_from_scores(hls_pred, threshold)
    else:
        hls_labels = None

    with csv_path.open("w", newline="") as f:
        fieldnames = [
            "path",
            "true",
            "keras_score",
            "keras_label",
        ]

        if hls_pred is not None:
            fieldnames.extend([
                "hls4ml_score",
                "hls4ml_label",
                "abs_error",
                "disagree",
            ])

        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

        for i, path in enumerate(paths):
            row = {
                "path": path,
                "true": "" if y_true is None else int(y_true[i]),
                "keras_score": float(keras_pred[i]),
                "keras_label": int(keras_labels[i]),
            }

            if hls_pred is not None:
                row.update({
                    "hls4ml_score": float(hls_pred[i]),
                    "hls4ml_label": int(hls_labels[i]),
                    "abs_error": float(abs(keras_pred[i] - hls_pred[i])),
                    "disagree": int(keras_labels[i] != hls_labels[i]),
                })

            writer.writerow(row)


def save_selection_json(
    path: Path,
    selected_precisions: dict[str, str],
    base_precision: str,
    safe_precision: str,
    candidates: list[str],
    keys_mode: str,
    only_keys: list[str],
    final_metrics: dict[str, Any],
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)

    payload = {
        "base_precision": base_precision,
        "safe_precision": safe_precision,
        "candidates": candidates,
        "keys_mode": keys_mode,
        "only_keys": only_keys,
        "selected_precisions": selected_precisions,
        "final_metrics": final_metrics,
    }

    path.write_text(json.dumps(payload, indent=2))


def load_selection_json(path: Path) -> dict[str, str]:
    payload = json.loads(path.read_text())
    return payload["selected_precisions"]


# ─────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("--model", type=Path, default=None)

    parser.add_argument("--data-dir", type=Path, default=DEFAULT_DATA_DIR)
    parser.add_argument("--dataset-format", default="auto", choices=["auto", "folder", "yolo"])
    parser.add_argument("--split", default="test")
    parser.add_argument("--class-names", nargs=2, default=["non_person", "person"])

    parser.add_argument("--test-image", type=Path, nargs="*", default=[])
    parser.add_argument("--test-image-dir", type=Path, default=None)

    parser.add_argument("--load-size", type=int, default=DEFAULT_LOAD_SIZE)
    parser.add_argument("--max-images", type=int, default=-1)
    parser.add_argument("--threshold", type=float, default=DEFAULT_THRESHOLD)

    parser.add_argument("--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT)
    parser.add_argument("--prediction-csv", type=Path, default=DEFAULT_CSV)
    parser.add_argument("--trials-csv", type=Path, default=DEFAULT_TRIALS_CSV)
    parser.add_argument("--selection-json", type=Path, default=DEFAULT_SELECTION_JSON)

    parser.add_argument("--keras-only", action="store_true")
    parser.add_argument("--save-misclassified", action="store_true", default=True)
    parser.add_argument("--max-grid-images", type=int, default=40)
    parser.add_argument("--threshold-sweep", action="store_true")

    parser.add_argument("--base-precision", default=DEFAULT_BASE_PRECISION)
    parser.add_argument("--safe-precision", default=DEFAULT_SAFE_PRECISION)
    parser.add_argument("--candidates", nargs="+", default=DEFAULT_CANDIDATES)

    parser.add_argument("--reuse-factor", type=int, default=DEFAULT_REUSE_FACTOR)
    parser.add_argument(
        "--io-type",
        default="io_parallel",
        choices=["io_parallel", "io_serial", "io_stream"],
    )

    parser.add_argument("--min-agreement", type=float, default=0.97)
    parser.add_argument("--max-mae", type=float, default=0.01)
    parser.add_argument("--max-error", type=float, default=0.02)

    parser.add_argument(
        "--keys-mode",
        choices=["all", "selected"],
        default="all",
    )

    parser.add_argument(
        "--precision-keys",
        nargs="+",
        default=["result", "accum", "weight", "bias"],
    )

    parser.add_argument("--list-layers", action="store_true")
    parser.add_argument("--layer-names", nargs="*", default=None)
    parser.add_argument("--continue-on-fail", action="store_true")

    parser.add_argument("--load-selection", type=Path, default=None)
    parser.add_argument("--no-clean-root", action="store_true")
    parser.add_argument("--print-final-config", action="store_true")

    args = parser.parse_args()

    max_images = None if args.max_images < 0 else args.max_images

    args.output_root.mkdir(parents=True, exist_ok=True)

    model_path = find_model_path(args.model)
    model = load_keras_model_compatible(model_path)
    input_hw = get_model_hw(model)

    print(f"\n[INFO] Model input HxW: {input_hw}")
    print(f"[INFO] Grayscale load size: {args.load_size}")
    print(f"[INFO] Threshold: {args.threshold}")

    if args.test_image or args.test_image_dir is not None:
        x_test, y_true, paths = load_test_images(
            image_paths=args.test_image,
            image_dir=args.test_image_dir,
            input_hw=input_hw,
            load_size=args.load_size,
        )

        for p in paths:
            debug_path = args.output_root / "debug_preprocess" / f"{Path(p).stem}_debug.png"
            debug_preprocess_image(
                Path(p),
                input_hw=input_hw,
                load_size=args.load_size,
                out_path=debug_path,
            )
            print(f"[OK] Saved preprocessing debug image: {debug_path}")

    else:
        x_test, y_true, paths = load_dataset(
            data_dir=args.data_dir,
            dataset_format=args.dataset_format,
            split=args.split,
            class_names=args.class_names,
            input_hw=input_hw,
            load_size=args.load_size,
            max_images=max_images,
        )

    print("\n[INFO] Running Keras prediction...")
    keras_pred = model.predict(x_test, verbose=0).reshape(-1)

    print_single_predictions(paths, keras_pred, threshold=args.threshold)

    if y_true is not None:
        print_confusion_and_report(
            scores=keras_pred,
            y_true=y_true,
            threshold=args.threshold,
            class_names=args.class_names,
        )

        if args.threshold_sweep:
            threshold_sweep(
                scores=keras_pred,
                y_true=y_true,
                thresholds=[0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60],
            )

        if args.save_misclassified:
            save_misclassified_outputs(
                paths=paths,
                scores=keras_pred,
                y_true=y_true,
                threshold=args.threshold,
                output_root=args.output_root,
                input_hw=input_hw,
                load_size=args.load_size,
                max_images=args.max_grid_images,
            )

    write_prediction_csv(
        csv_path=args.prediction_csv,
        paths=paths,
        y_true=y_true,
        keras_pred=keras_pred,
        threshold=args.threshold,
        hls_pred=None,
    )

    print(f"\n[OK] Saved Keras prediction CSV: {args.prediction_csv}")

    if args.keras_only:
        print("\n[INFO] --keras-only was set. Stopping before hls4ml/Bambu.")
        return

    if y_true is None:
        raise RuntimeError(
            "hls4ml precision sweep needs labeled data. "
            "Use a folder or YOLO dataset, not only --test-image."
        )

    import hls4ml

    print("\nAvailable hls4ml backends:", hls4ml.backends.get_available_backends())

    probe_config = make_base_hls_config(
        model=model,
        base_precision=args.base_precision,
        reuse_factor=args.reuse_factor,
        io_type=args.io_type,
    )

    layer_names = get_precision_layer_names(probe_config)

    print("\nDiscovered hls4ml layer names with Precision dict:")
    for lname in layer_names:
        print(f"  {lname}: {get_layer_precision_dict(probe_config, lname)}")

    if args.list_layers:
        return

    if args.layer_names:
        requested = set(args.layer_names)
        layer_names = [lname for lname in layer_names if lname in requested]
        missing = sorted(requested - set(layer_names))

        if missing:
            raise ValueError(f"Requested layer names not found in config: {missing}")

    if not args.no_clean_root and args.output_root.exists():
        shutil.rmtree(args.output_root)

    args.output_root.mkdir(parents=True, exist_ok=True)

    if args.load_selection is not None:
        print(f"\n[INFO] Loading existing precision selection from {args.load_selection}")
        selected_precisions = load_selection_json(args.load_selection)
        trials = []

    else:
        print("\nPrecision search settings:")
        print(f"  base precision : {args.base_precision}")
        print(f"  safe precision : {args.safe_precision}")
        print(f"  candidates     : {args.candidates}")
        print(f"  threshold      : {args.threshold}")
        print(
            f"  acceptance     : agreement>={args.min_agreement}, "
            f"MAE<={args.max_mae}, max_error<={args.max_error}"
        )

        selected_precisions, trials = greedy_layer_precision_search(
            model=model,
            x_test=x_test,
            y_true=y_true,
            keras_pred=keras_pred,
            layer_names=layer_names,
            output_root=args.output_root,
            base_precision=args.base_precision,
            safe_precision=args.safe_precision,
            candidates=args.candidates,
            reuse_factor=args.reuse_factor,
            io_type=args.io_type,
            threshold=args.threshold,
            min_agreement=args.min_agreement,
            max_mae=args.max_mae,
            max_error=args.max_error,
            keys_mode=args.keys_mode,
            only_keys=args.precision_keys,
            continue_on_fail=args.continue_on_fail,
        )

    print("\n" + "=" * 100)
    print("FINAL SELECTED PRECISIONS")
    print("=" * 100)

    for lname, precision in selected_precisions.items():
        print(f"{lname:30s} -> {precision}")

    print("\n[INFO] Running final check with selected per-layer precisions...")

    final_hls_pred, final_config = run_hls_prediction(
        model=model,
        x_test=x_test,
        output_dir=args.output_root / "FINAL_SELECTED",
        base_precision=args.base_precision,
        selected_precisions=selected_precisions,
        reuse_factor=args.reuse_factor,
        io_type=args.io_type,
        keys_mode=args.keys_mode,
        only_keys=args.precision_keys,
        clean=True,
        print_config=args.print_final_config,
    )

    final_metrics = compare_metrics(
        keras_pred=keras_pred,
        hls_pred=final_hls_pred,
        y_true=y_true,
        threshold=args.threshold,
    )

    print_comparison_summary("FINAL SELECTED CONFIG", final_metrics)

    write_prediction_csv(
        csv_path=args.prediction_csv,
        paths=paths,
        y_true=y_true,
        keras_pred=keras_pred,
        threshold=args.threshold,
        hls_pred=final_hls_pred,
    )

    write_rows_csv(args.trials_csv, trials)

    save_selection_json(
        path=args.selection_json,
        selected_precisions=selected_precisions,
        base_precision=args.base_precision,
        safe_precision=args.safe_precision,
        candidates=args.candidates,
        keys_mode=args.keys_mode,
        only_keys=args.precision_keys,
        final_metrics=final_metrics,
    )

    print("\nSaved:")
    print(f"  Prediction CSV : {args.prediction_csv}")
    print(f"  Trials CSV     : {args.trials_csv}")
    print(f"  Selection JSON : {args.selection_json}")
    print(f"  HLS projects   : {args.output_root}")


if __name__ == "__main__":
    main()