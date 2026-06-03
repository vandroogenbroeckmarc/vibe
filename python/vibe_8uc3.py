"""Polychromatic (3-channel) example program for the Python port of libvibe++.

Python equivalent of ``main/ViBe_8UC3.cpp``. Reads a video sequence from the
path passed on the command line, runs the 3-channel variant of ViBe directly
on BGR frames, and displays the resulting segmentation map with a 3x3 median
filter as post-processing.

Usage
-----
    python vibe_8uc3.py <video_path>
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

        if vibe is None:
            h, w = frame.shape[:2]
            vibe = ViBeSequential(h, w, frame)
            segmentation_map = np.empty((h, w), dtype=np.uint8)

        vibe.segmentation(frame, segmentation_map)
        vibe.update(frame, segmentation_map)

        segmentation_map = cv2.medianBlur(segmentation_map, 3)

        cv2.imshow("Input video", frame)
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
