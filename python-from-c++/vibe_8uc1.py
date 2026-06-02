"""Monochromatic example program for the Python port of libvibe++.

Python equivalent of ``main/ViBe_8UC1.cpp``. Reads a video sequence from a
path passed on the command line, converts each frame to grayscale, runs
the single-channel variant of ViBe and displays the resulting segmentation
map with a 3x3 median filter as post-processing.

Usage
-----
    python vibe_8uc1.py <video_path>
"""

from __future__ import annotations

import sys

import cv2
import numpy as np

from vibe import ViBeSequential


def main(video_path: str) -> int:
    decoder = cv2.VideoCapture(video_path)
    if not decoder.isOpened():
        print(f"Could not open video: {video_path}", file=sys.stderr)
        return 1

    vibe: ViBeSequential | None = None
    segmentation_map: np.ndarray | None = None

    while True:
        ok, frame = decoder.read()
        if not ok:
            break

        bw_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        if vibe is None:
            h, w = bw_frame.shape
            vibe = ViBeSequential(h, w, bw_frame)
            segmentation_map = np.empty((h, w), dtype=np.uint8)

        vibe.segmentation(bw_frame, segmentation_map)
        vibe.update(bw_frame, segmentation_map)

        segmentation_map = cv2.medianBlur(segmentation_map, 3)

        cv2.imshow("Input video", bw_frame)
        cv2.imshow("Segmentation by ViBe", segmentation_map)
        if cv2.waitKey(1) & 0xFF == 27:  # ESC to quit
            break

    decoder.release()
    cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("A video file must be given as an argument to the program!",
              file=sys.stderr)
        sys.exit(1)
    sys.exit(main(sys.argv[1]))
