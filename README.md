# EyesThatBlink

EyesThatBlink is a webcam-based blink detection tool.

The current implementation is in Rust (`src/`, built with Cargo). A legacy C++ implementation is also present and can still be built with CMake.

## Features

- Detects blinks from webcam frames using OpenCV Haar cascades.
- Supports two blink detection modes:
  - `adaptive-ear` (default): adaptive openness thresholding.
  - `legacy`: treats missing eye detections as blinks.
- Sends desktop notifications when blink rate drops for long periods.

## Requirements

- Rust toolchain (edition 2024 project)
- OpenCV development libraries available on your system
- A working webcam
- Linux desktop notifications (used via `notify-rust`)

## Run (Rust implementation)

```bash
cargo run --release
```

Common options:

```bash
cargo run --release -- --draw
cargo run --release -- --blink-method adaptive-ear
cargo run --release -- --blink-method legacy
cargo run --release -- --debug
```

CLI flags:

- `-d, --debug` Increase debug verbosity (repeatable).
- `--draw` Show processed frames with face/eye rectangles.
- `-s, --score-threshold <float>` Detection score threshold.
- `-n, --nms-threshold <float>` NMS threshold.
- `-t, --tok-k <int>` Top-k value.
- `--blink-method <legacy|adaptive-ear>` Blink detection method.

## Build (legacy C++ implementation)

```bash
cmake -S . -B build
cmake --build build -j
./build/eyesthatblink
```

This path uses the C++ sources under `core/`, `ui/`, and related CMake targets.

## Project layout

- `src/`: Rust implementation
- `cascades/`: Haar cascade XMLs
- `core/`, `ui/`, `actions/`, `main.cpp`: legacy C++ implementation
- `data/`: desktop entry and app icon

## Packaging

- Rust package metadata for Debian is defined in `Cargo.toml` under `[package.metadata.deb]`.
- Additional packaging helpers: `package.sh`, `create_deb.sh`, `pkgs/PKGBUILD`.
