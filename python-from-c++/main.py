"""Command-line entry point for the Python port of libvibe++.

This script runs the ViBe background-subtraction algorithm on a video file
(for example an MPEG-1 .mpg file) and either displays the segmentation
result in real time or writes it to an output video / image sequence.

It is designed to work headlessly as well: when no display is available
(``--no-display``), the masks are written to disk.

Examples
--------
Display the segmentation of ``my_video.mpg`` using grayscale ViBe
(equivalent to main/ViBe_8UC1.cpp):

    python main.py my_video.mpg

Same but with the color (3-channel) variant
(equivalent to main/ViBe_8UC3.cpp):

    python main.py --color my_video.mpg

Headless run that saves the resulting masks to an output video file:

    python main.py --no-display --output out_masks.mp4 my_video.mpg

Save each mask as a separate PNG in a folder:

    python main.py --no-display --output masks/ my_video.mpg

Supported input formats
-----------------------
Anything OpenCV / FFmpeg can decode: .mpg, .mpeg, .mp4, .avi, .mov, .mkv ...
"""

from __future__ import annotations

import argparse
import math
import os
import re
import statistics
import sys
import time

import cv2
import numpy as np

from vibe import ViBeSequential


# ---------------------------------------------------------------------------
# Image-sequence capture (drop-in for cv2.VideoCapture)
# ---------------------------------------------------------------------------

# Matches a filename of the form  <radical><digits>.<jpg|jpeg|png>
# where <radical> is the longest leading non-digit-anchored prefix and
# <digits> is the *trailing* digit run before the extension.
#
# The non-greedy ``.*?`` ensures the digit group captures the maximal
# trailing digit run (e.g. "in0010" -> radical="in", digits="0010"),
# and digits are sorted by integer value so that 9 precedes 10 even
# when zero-padding widths differ.
_IMG_NAME_RE = re.compile(
    r"^(?P<radical>.*?)(?P<num>\d+)\.(?P<ext>jpg|jpeg|png)$",
    re.IGNORECASE,
)


class _ImageSequenceCapture:
    """A minimal drop-in replacement for :class:`cv2.VideoCapture` that reads
    image files from a directory in numeric order.

    Files matching ``<radical><digits>.<jpg|jpeg|png>`` are kept; the digit
    run is parsed as an integer so that, regardless of zero-padding width,
    ``in9.jpg``, ``in009.jpg``, ``in010.jpg`` and ``in0010.jpg`` all sort
    by their numeric value rather than lexicographically.

    Anything not matching the pattern is silently skipped.
    """

    def __init__(self, directory: str) -> None:
        self._files: list[str] = []
        self._idx: int = 0
        self._w: int = 0
        self._h: int = 0
        self._scan(directory)
        if self._files:
            sample = cv2.imread(self._files[0])
            if sample is not None:
                self._h, self._w = sample.shape[:2]

    def _scan(self, directory: str) -> None:
        if not os.path.isdir(directory):
            return
        entries: list[tuple[str, int, str]] = []
        for name in os.listdir(directory):
            m = _IMG_NAME_RE.match(name)
            if not m:
                continue
            entries.append((m["radical"], int(m["num"]), name))
        entries.sort(key=lambda t: (t[0], t[1]))
        self._files = [os.path.join(directory, n) for _, _, n in entries]

    # --- cv2.VideoCapture-shaped interface --------------------------------

    def isOpened(self) -> bool:                       # noqa: N802 (OpenCV API)
        return len(self._files) > 0

    def read(self):
        if self._idx >= len(self._files):
            return False, None
        path = self._files[self._idx]
        self._idx += 1
        frame = cv2.imread(path)
        return frame is not None, frame

    def get(self, prop: int) -> float:
        if prop == cv2.CAP_PROP_FRAME_WIDTH:
            return float(self._w)
        if prop == cv2.CAP_PROP_FRAME_HEIGHT:
            return float(self._h)
        if prop == cv2.CAP_PROP_FPS:
            # No native FPS for an image sequence; advertise 0.0 so callers
            # that *or*-fall-back to a default (e.g. ``fps or 25.0``) work.
            return 0.0
        if prop == cv2.CAP_PROP_FRAME_COUNT:
            return float(len(self._files))
        return 0.0

    def release(self) -> None:
        pass


def _open_input(path: str):
    """Return a frame source for *path*: either a real ``cv2.VideoCapture``
    if *path* is a video file, or an :class:`_ImageSequenceCapture` if it
    is a directory of JPEG/PNG frames.
    """
    if os.path.isdir(path):
        return _ImageSequenceCapture(path)
    return cv2.VideoCapture(path)


# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------

def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Run ViBe background subtraction on a video file "
                    "or on a directory of ordered JPEG/PNG frames.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "video",
        help="Path to the input. Either a video file (.mpg, .mp4, .avi, "
             "...) OR a directory of JPEG/PNG frames named "
             "<radical><digits>.<jpg|jpeg|png>. The digit run is sorted "
             "numerically, so 'in9.jpg' precedes 'in10.jpg' even when "
             "padding widths differ.",
    )
    parser.add_argument(
        "--color", action="store_true",
        help="Use the 3-channel color variant of ViBe "
             "(equivalent to main/ViBe_8UC3.cpp). "
             "Default is the 1-channel grayscale variant.",
    )
    parser.add_argument(
        "--no-display", action="store_true",
        help="Do not open windows; useful for headless/batch processing.",
    )
    parser.add_argument(
        "--output", "-o",
        default=None,
        help="Optional output path. If it ends in a video extension "
             "(.mp4, .avi, .mkv, ...) the masks are encoded into a video. "
             "If it ends with '/' or is an existing directory, each mask is "
             "saved as an individual PNG (mask_000000.png, ...).",
    )
    parser.add_argument(
        "--no-median", action="store_true",
        help="Skip the 3x3 median filter post-processing.",
    )
    parser.add_argument(
        "--benchmark", action="store_true",
        help="Pure-algorithm timing mode. Implies --no-display and "
             "--no-median; any --output is ignored. At the end of the run "
             "prints a detailed report: total wall time, FPS, and per-frame "
             "(segmentation + update) min, max, mean, median, stdev, p95, "
             "p99 in ms.",
    )
    parser.add_argument(
        "--samples", type=int, default=None,
        help="Override the number of samples per pixel (default 30). "
             "Changing this reallocates the model.",
    )
    parser.add_argument(
        "--threshold", type=int, default=None,
        help="Override the matching threshold (default 10).",
    )
    parser.add_argument(
        "--matches", type=int, default=None,
        help="Override the number of matches required (default 2).",
    )
    parser.add_argument(
        "--update-factor", type=int, default=None,
        help="Override the model subsampling factor (default 8).",
    )
    parser.add_argument(
        "--seed", type=int, default=None,
        help="Seed the NumPy RNG for reproducible outputs.",
    )
    parser.add_argument(
        "--max-frames", type=int, default=None,
        help="Stop after processing this many frames (for testing).",
    )
    return parser


# ---------------------------------------------------------------------------
# Output handling
# ---------------------------------------------------------------------------

_VIDEO_EXTS = {".mp4", ".avi", ".mkv", ".mov", ".mpg", ".mpeg", ".m4v"}


class _Sink:
    """Abstract writer: either a video encoder, a PNG folder, or nothing."""

    def __init__(self, output: str | None,
                 fps: float, width: int, height: int) -> None:
        self.kind = "none"
        self.writer = None
        self.folder = None
        self.frame_idx = 0

        if output is None:
            return

        # Folder mode: path ends with / or already exists as a directory.
        if output.endswith(os.sep) or output.endswith("/") \
                or os.path.isdir(output):
            os.makedirs(output, exist_ok=True)
            self.kind = "folder"
            self.folder = output
            return

        # Video mode: recognised video extension.
        ext = os.path.splitext(output)[1].lower()
        if ext in _VIDEO_EXTS:
            fourcc = cv2.VideoWriter_fourcc(*"mp4v") if ext == ".mp4" \
                else cv2.VideoWriter_fourcc(*"XVID")
            self.writer = cv2.VideoWriter(
                output, fourcc, fps if fps > 0 else 25.0,
                (width, height), isColor=False)
            if not self.writer.isOpened():
                # Fallback to a directory-of-PNGs if the encoder failed.
                print(f"[main.py] Could not open video writer for "
                      f"{output!r}; falling back to PNG folder.",
                      file=sys.stderr)
                self.writer = None
                fallback = os.path.splitext(output)[0] + "_masks"
                os.makedirs(fallback, exist_ok=True)
                self.kind = "folder"
                self.folder = fallback
                return
            self.kind = "video"
            return

        # Everything else -> treat it like a directory target.
        os.makedirs(output, exist_ok=True)
        self.kind = "folder"
        self.folder = output

    def write(self, mask: np.ndarray) -> None:
        if self.kind == "video" and self.writer is not None:
            self.writer.write(mask)
        elif self.kind == "folder" and self.folder is not None:
            path = os.path.join(self.folder, f"mask_{self.frame_idx:06d}.png")
            cv2.imwrite(path, mask)
        self.frame_idx += 1

    def release(self) -> None:
        if self.writer is not None:
            self.writer.release()


# ---------------------------------------------------------------------------
# Core loop
# ---------------------------------------------------------------------------

def run(args: argparse.Namespace) -> int:
    if args.seed is not None:
        np.random.seed(args.seed)

    # --benchmark implies: no display, no median, no output.
    if args.benchmark:
        args.no_display = True
        args.no_median = True
        if args.output:
            print("[main.py] warning: --benchmark disables disk output; "
                  f"ignoring --output {args.output!r}.", file=sys.stderr)
            args.output = None

    if not (os.path.isfile(args.video) or os.path.isdir(args.video)):
        print(f"Input not found (neither file nor directory): {args.video}",
              file=sys.stderr)
        return 1

    decoder = _open_input(args.video)
    if not decoder.isOpened():
        if os.path.isdir(args.video):
            print(f"No matching frames in directory: {args.video}\n"
                  "  (expected files like 'in0001.jpg', 'frame_010.png', ...)",
                  file=sys.stderr)
        else:
            print(f"Could not open video: {args.video}", file=sys.stderr)
        return 1

    fps = decoder.get(cv2.CAP_PROP_FPS) or 0.0
    width = int(decoder.get(cv2.CAP_PROP_FRAME_WIDTH) or 0)
    height = int(decoder.get(cv2.CAP_PROP_FRAME_HEIGHT) or 0)

    sink = _Sink(args.output, fps, width, height)

    vibe: ViBeSequential | None = None
    segmentation_map: np.ndarray | None = None
    frames = 0
    core_times_ms: list[float] = []
    t0 = time.perf_counter()

    while True:
        ok, frame = decoder.read()
        if not ok:
            break

        if args.color:
            working = frame                              # BGR 3-channel
        else:
            working = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        if vibe is None:
            h, w = working.shape[:2]
            vibe = ViBeSequential(h, w, working)

            # Apply any user overrides *after* construction (matching the
            # setter semantics of the C++ library).
            if args.samples is not None and \
                    args.samples != vibe.get_number_of_samples():
                # The C++ library doesn't expose a setter for number_of_samples
                # either; the only way to change it is to rebuild. We rebuild
                # in-place here so CLI users get the expected behaviour.
                vibe.number_of_samples = int(args.samples)
                # Rebuild history_buffer & position table with the new size.
                vibe.num_tests = vibe.number_of_samples - vibe.num_history
                buf_shape = (h, w, vibe.num_tests) + (
                    (vibe.channels,) if vibe.channels > 1 else ())
                noise = np.random.randint(-10, 10,
                                          size=buf_shape, dtype=np.int32)
                base = working.astype(np.int32)
                base_b = base[:, :, None] if vibe.channels == 1 \
                    else base[:, :, None, :]
                vibe.history_buffer = np.clip(
                    base_b + noise, 0, 255).astype(np.uint8)
                size = 2 * max(w, h) + 1
                vibe.position = np.random.randint(
                    0, vibe.number_of_samples, size=size).astype(np.uint32)

            if args.threshold is not None:
                vibe.set_matching_threshold(args.threshold)
            if args.matches is not None:
                vibe.set_matching_number(args.matches)
            if args.update_factor is not None:
                vibe.set_update_factor(args.update_factor)

            segmentation_map = np.empty((h, w), dtype=np.uint8)
            print("[main.py] ViBe parameters:")
            print(str(vibe))

        # Time only the core algorithm when --benchmark is active.
        _cs = time.perf_counter()
        vibe.segmentation(working, segmentation_map)
        vibe.update(working, segmentation_map)
        _ce = time.perf_counter()
        if args.benchmark:
            core_times_ms.append((_ce - _cs) * 1000.0)

        if not args.no_median:
            segmentation_map = cv2.medianBlur(segmentation_map, 3)

        sink.write(segmentation_map)

        if not args.no_display:
            cv2.imshow("Input video", working)
            cv2.imshow("Segmentation by ViBe", segmentation_map)
            if cv2.waitKey(1) & 0xFF == 27:  # ESC to quit
                break

        frames += 1
        if args.max_frames is not None and frames >= args.max_frames:
            break

    wall_s = max(time.perf_counter() - t0, 1e-9)
    print(f"[main.py] Processed {frames} frames in {wall_s:.2f}s "
          f"({frames / wall_s:.2f} FPS)")

    if args.benchmark:
        _print_benchmark_report(
            core_times_ms,
            wall_ms=wall_s * 1000.0,
            label=("ViBe (color / 3-channel)" if args.color
                   else "ViBe (grayscale / 1-channel)"),
        )

    sink.release()
    decoder.release()
    if not args.no_display:
        cv2.destroyAllWindows()
    return 0


# ---------------------------------------------------------------------------
# Benchmark stats
# ---------------------------------------------------------------------------

def _percentile(sorted_samples: list[float], p: float) -> float:
    """Nearest-rank percentile (matches the C++ implementation)."""
    if not sorted_samples:
        return 0.0
    n = len(sorted_samples)
    rank = max(1, math.ceil(p * n)) - 1
    return sorted_samples[min(rank, n - 1)]


def _print_benchmark_report(core_times_ms: list[float],
                            wall_ms: float,
                            label: str) -> None:
    n = len(core_times_ms)
    print()
    print("=" * 64)
    print(f"ViBe benchmark report  [{label}]")
    print("=" * 64)
    if n == 0:
        print("  (no frames processed)")
        print("=" * 64)
        return

    samples = sorted(core_times_ms)
    total = sum(samples)
    mean = total / n
    median = statistics.median(samples)
    stdev = statistics.stdev(samples) if n > 1 else 0.0
    p95 = _percentile(samples, 0.95)
    p99 = _percentile(samples, 0.99)
    fps_core = 1000.0 / mean if mean > 0 else 0.0
    fps_wall = 1000.0 * n / wall_ms if wall_ms > 0 else 0.0

    print(f"  Frames processed             : {n}")
    print(f"  Wall clock (end-to-end)      : {wall_ms:.3f} ms")
    print(f"  Core time (sum segm.+update) : {total:.3f} ms")
    print(f"  Throughput (wall clock)      : {fps_wall:.3f} fps")
    print(f"  Throughput (core time only)  : {fps_core:.3f} fps")
    print(f"  Per-frame core time [ms]:")
    print(f"     min                       : {samples[0]:.3f}")
    print(f"     max                       : {samples[-1]:.3f}")
    print(f"     mean                      : {mean:.3f}")
    print(f"     median                    : {median:.3f}")
    print(f"     stdev (sample)            : {stdev:.3f}")
    print(f"     p95                       : {p95:.3f}")
    print(f"     p99                       : {p99:.3f}")
    print("=" * 64)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)
    return run(args)


if __name__ == "__main__":
    sys.exit(main())
