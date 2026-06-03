# ViBe — C++

This repository contains a modernized C++ implementation of **ViBe**, a
real-time universal background subtraction algorithm for video sequences
(Barnich & Van Droogenbroeck, *IEEE Transactions on Image Processing*, 2011).

<p align="center">
  <img src="../img/C++_logo.png" height="150">
</p>

## Dependencies

- A C++17 compiler (GCC ≥ 7, Clang ≥ 5, Apple Clang ≥ 10, MSVC 2017+).
- CMake ≥ 3.15.
- OpenCV ≥ 4.5 — **required only** to build the example programs.
- Boost is **no longer required** (the library now uses the standard
  `<cstdint>` header).

## Quick start — Strategy A (recommended, cross-platform via vcpkg)

Works identically on Linux, macOS and Windows.

```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh          # .\vcpkg\bootstrap-vcpkg.bat on Windows

cmake -S . -B build \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

vcpkg picks up the `vcpkg.json` manifest shipped at the repository root and
installs OpenCV automatically.

## Quick start — Strategy B (system-provided OpenCV)

Install OpenCV and a C++17 toolchain with your package manager:

| Platform          | One-liner                                                        |
|-------------------|------------------------------------------------------------------|
| Ubuntu / Debian   | `sudo apt install libopencv-dev cmake ninja-build g++`           |
| Fedora            | `sudo dnf install opencv-devel cmake ninja-build gcc-c++`        |
| macOS (Homebrew)  | `brew install opencv cmake ninja`                                |
| Windows           | Strategy A is recommended; Chocolatey: `choco install opencv cmake ninja` |

Then:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Disable the examples (header-only-ish library build, no OpenCV required) with
`-DLIBVIBEPP_COMPILE_EXAMPLES=OFF`.

## Installation

```bash
cmake --install build --prefix <install-prefix>
```

On Linux/macOS, installing into `/usr/local` requires `sudo`.
On Windows, point the prefix at e.g. `C:\Program Files\libvibe++`.

## Using libvibe++ from another CMake project

```cmake
find_package(vibe++ CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE vibe++::vibe++)
```

If `vibe++` was installed to a non-standard prefix, point CMake at it:

```bash
cmake -S . -B build -Dvibe++_DIR=<prefix>/lib/cmake/vibe++
```

## Quick demo via Make

A bundled `input/driveway-320x240.avi` clip and a thin `Makefile` wrapper
around the CMake build let anyone take the library for a spin in one line:

```bash
make demo             # build + run ViBe_8UC1 on the clip with --no-median
make demo-color       # same clip through ViBe_8UC3 (--no-median)
make demo-benchmark   # pure-algorithm timing report (--benchmark)
```

Run `make help` for the full list of targets. `make clean` removes the
`build/` directory.

## Running the examples

The two example programs share a common command-line interface that is
one-for-one compatible with the Python port (`python-from-c++/main.py`):

```
ViBe_8UC1 | ViBe_8UC3
  [-h] [--color] [--no-display] [--output OUTPUT] [--no-median] [--benchmark]
  [--samples SAMPLES] [--threshold THRESHOLD] [--matches MATCHES]
  [--update-factor UPDATE_FACTOR] [--seed SEED] [--max-frames MAX_FRAMES]
  VIDEO
```

Minimal interactive use:

```bash
./build/bin/ViBe_8UC1 path/to/video.mp4     # grayscale (1-channel) variant
./build/bin/ViBe_8UC3 path/to/video.mp4     # color     (3-channel) variant
```

Press **ESC** or **Q** to close the preview windows cleanly.

### Input modes: video file or directory of frames

The positional `VIDEO` argument accepts **two kinds of input**:

1. **A video file** — anything FFmpeg/OpenCV can decode (`.mp4`, `.avi`,
   `.mov`, `.mkv`, ...).
2. **A directory of image frames** — JPEG (`.jpg`, `.jpeg`) or PNG (`.png`)
   files named `<radical><digits>.<ext>`. The digit run is parsed as an
   integer and sorted numerically, so `in9.jpg` precedes `in10.jpg` even
   when zero-padding widths differ (e.g. `in009.jpg` and `in0010.jpg` are
   still ordered 9 → 10). Files that do not match the pattern are silently
   skipped. The match is case-insensitive on the extension.

The bundled example sequence `../input/highway-jpg/` (1700 frames named
`in000001.jpg ... in001700.jpg`) exercises mode 2:

```bash
./build/bin/ViBe_8UC1 --no-median ../input/highway-jpg
./build/bin/ViBe_8UC1 --benchmark --max-frames 500 --seed 42 \
    ../input/highway-jpg
```

### Headless processing (no display, write masks to disk)

Write masks to an encoded video:

```bash
./build/bin/ViBe_8UC1 --no-display --output out_masks.mp4 input.mp4
```

Write masks as individual PNGs (path ending with `/` or an existing
directory triggers folder mode):

```bash
./build/bin/ViBe_8UC3 --no-display --output masks/ input.mp4
```

### Tuning ViBe parameters

`--threshold`, `--matches` and `--update-factor` map directly onto the
`setMatchingThreshold()`, `setMatchingNumber()` and `setUpdateFactor()`
methods of the library:

```bash
./build/bin/ViBe_8UC1 --threshold 25 --matches 3 --update-factor 8 \
                      --seed 42 --max-frames 500 input.mp4
```

Notes:
* `--color` is accepted for CLI compatibility with the Python driver but
  is a no-op here — the binary itself (`ViBe_8UC1` vs `ViBe_8UC3`) is the
  channel selection.
* `--samples` is accepted but not honored: `libvibe++` hard-codes
  `DEFAULT_NUMBER_OF_SAMPLES = 20` as a `static const` in
  `ViBeBase.h`, with no public setter and no runtime reallocation of the
  sample buffer. The CLI emits a warning when the flag is used.
* `--seed` defaults to `-1`, meaning the RNG is seeded from `time(nullptr)`.
* `--max-frames` is useful for timing or CI runs on a truncated sequence.

### Benchmarking

`--benchmark` is the pure-algorithm timing mode. It implies `--no-display`
and `--no-median` and discards any `--output` so the measurement reflects
the ViBe computation itself. The timer wraps only `segmentation() +
update()`; decoding and any post-processing are excluded. At the end of
the run the program prints:

```
================================================================
ViBe benchmark report  [ViBe_8UC1 / 1-channel / Manhattan<1>]
================================================================
  Frames processed             : 500
  Wall clock (end-to-end)      : 612.140 ms
  Core time (sum segm.+update) : 431.005 ms
  Throughput (wall clock)      : 816.776 fps
  Throughput (core time only)  : 1160.017 fps
  Per-frame core time [ms]:
     min                       : 0.701
     max                       : 1.844
     mean                      : 0.862
     median                    : 0.852
     stdev (sample)            : 0.091
     p95                       : 0.981
     p99                       : 1.214
================================================================
```

Typical use:

```bash
./build/bin/ViBe_8UC1 --benchmark --max-frames 500 --seed 42 input.mp4
./build/bin/ViBe_8UC3 --benchmark --max-frames 500 --seed 42 input.mp4
```

Full usage is always available with:

```bash
./build/bin/ViBe_8UC1 -h
```

## Authors

See the [AUTHORS](AUTHORS) file for details.

## License

The original ViBe patent (WO2009007198 / EP2015252 / US 8009918 B2 /
JP 4699564 B2) is now in the public domain.
See the [LICENSE](LICENSE) file for the source code license.

## Citation

```bibtex
@article{Barnich2011ViBe,
  title   = {{ViBe}: A universal background subtraction algorithm for video sequences},
  author  = {Barnich, Olivier and {Van Droogenbroeck}, Marc},
  journal = {IEEE Transactions on Image Processing},
  year    = {2011},
  volume  = {20},
  number  = {6},
  pages   = {1709--1724},
  month   = {June},
  doi     = {10.1109/TIP.2010.2101613}
}
```
