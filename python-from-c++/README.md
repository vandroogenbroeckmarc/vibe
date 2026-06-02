# ViBe - Self-contained Python port of libvibe++

This directory is a standalone Python port of the C++ `libvibe++` library
sitting in the sibling `../C++` folder. It implements the ViBe
background-subtraction algorithm described in

>  O. Barnich, M. Van Droogenbroeck.
>  "ViBe: A universal background subtraction algorithm for video sequences."
>  IEEE Transactions on Image Processing, 20(6):1709-1724, 2011.

This port mirrors the modernized C++ library class-by-class and is meant as a lightweight, dependency-light reference implementation.

## File layout

    libvibe++/ (C++)                       |  python-from-c++/ (Python)
    -------------------------------------- | --------------------------------
    ViBe.h + ViBe.t + common/ViBeBase.*    |  vibe.py  (class ViBeSequential)
    distances/Manhattan.h                  |  vibe.py  (manhattan_match)
    main/ViBe_8UC1.cpp                     |  vibe_8uc1.py
    main/ViBe_8UC3.cpp                     |  vibe_8uc3.py
    -                                      |  main.py  (CLI front-end, .mpg)

## Requirements

    python >= 3.8
    numpy
    opencv-python

Install with:

    pip install -r requirements.txt

## Quick demo via Make

A bundled `input/driveway-320x240.avi` clip and a thin `Makefile` let
anyone take the port for a spin in one line:

    make demo             # interactive color preview (--no-median --color)
    make demo-gray        # same clip, grayscale (--no-median)
    make demo-benchmark   # pure-algorithm timing report

Run `make help` for the full list of targets. `make clean` removes Python
`__pycache__/` directories.

## CLI usage

The `main.py` script is a ready-made command-line front-end that accepts
**two kinds of input**:

1. **A video file** — MPEG (`.mpg`, `.mpeg`) as well as every other format
   OpenCV/FFmpeg can decode (`.mp4`, `.avi`, `.mkv`, `.mov`, ...).
2. **A directory of image frames** — JPEG (`.jpg`, `.jpeg`) or PNG (`.png`)
   files named `<radical><digits>.<ext>`. The digit run is parsed as an
   integer and sorted numerically, so `in9.jpg` precedes `in10.jpg` even
   when the zero-padding widths differ (e.g. `in009.jpg` and `in0010.jpg`
   are still ordered 9 → 10). Non-matching files are silently skipped.

The bundled example sequence `../input/highway-jpg/` (1700 frames named
`in000001.jpg ... in001700.jpg`) exercises mode 2:

    python3 main.py --no-median ../input/highway-jpg
    python3 main.py --benchmark --max-frames 500 --seed 42 \
        ../input/highway-jpg

Display the grayscale segmentation in real time (equivalent to
`main/ViBe_8UC1.cpp`):

    python main.py path/to/video.mpg

Run the color (3-channel) variant (equivalent to `main/ViBe_8UC3.cpp`):

    python main.py --color path/to/video.mpg

Headless run that saves the masks to a video file:

    python main.py --no-display --output masks.mp4 path/to/video.mpg

Save each mask as a separate PNG in a folder:

    python main.py --no-display --output out_masks/ path/to/video.mpg

### Benchmarking

`--benchmark` is the pure-algorithm timing mode. It implies `--no-display`
and `--no-median` and discards any `--output` so the measurement reflects
the ViBe computation itself. The timer wraps only
`ViBeSequential.segmentation()` + `update()`; decoding and post-processing
are excluded. At the end of the run `main.py` prints a detailed report:

    ================================================================
    ViBe benchmark report  [ViBe (grayscale / 1-channel)]
    ================================================================
      Frames processed             : 500
      Wall clock (end-to-end)      : 1640.320 ms
      Core time (sum segm.+update) : 1412.760 ms
      Throughput (wall clock)      : 304.822 fps
      Throughput (core time only)  : 353.794 fps
      Per-frame core time [ms]:
         min                       : 2.610
         max                       : 4.210
         mean                      : 2.826
         median                    : 2.774
         stdev (sample)            : 0.192
         p95                       : 3.060
         p99                       : 3.910
    ================================================================

Typical use:

    python main.py --benchmark --max-frames 500 --seed 42 path/to/video.mpg
    python main.py --benchmark --color --max-frames 500 --seed 42 path/to/video.mpg

Full option list:

    python main.py --help

## Programmatic usage

```python
import cv2
import numpy as np
from vibe import ViBeSequential

cap = cv2.VideoCapture("my_video.mpg")
ok, frame = cap.read()
bw = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

vibe = ViBeSequential(bw.shape[0], bw.shape[1], bw)
mask = np.empty_like(bw)

while True:
    ok, frame = cap.read()
    if not ok:
        break
    bw = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    vibe.segmentation(bw, mask)
    vibe.update(bw, mask)
    cv2.imshow("mask", mask)
    if cv2.waitKey(1) & 0xFF == 27:
        break
```

## Default parameters (identical to the C++ defaults)

    number_of_samples        = 30
    matching_threshold       = 10
    matching_number          = 2
    update_factor            = 8
    number_of_history_images = 2

Each of these can be overridden from the CLI (`--samples`, `--threshold`,
`--matches`, `--update-factor`) or via the setter methods on
`ViBeSequential`.

## Reproducibility

Pass `--seed N` on the command line (or call `numpy.random.seed(N)` before
constructing `ViBeSequential`) to get deterministic runs.

## License

ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).
Permission to use ViBe without payment of fee is granted for nonprofit
educational and research purposes only. See the `LICENSE` files in the
sibling `C++/` and `Python/` directories for the full terms.
