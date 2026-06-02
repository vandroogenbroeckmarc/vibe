"""
Python port of the ViBe background-subtraction algorithm (libvibe++).

Copyright - Benjamin Laugraud - 2016
Copyright - Marc Van Droogenbroeck - 2016

ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).

Permission to use ViBe without payment of fee is granted for nonprofit
educational and research purposes only.

This work may not be copied or reproduced in whole or in part for any purpose.
Copying, reproduction, or republishing for any purpose shall require a
license. Please contact the authors in such cases. All the code is provided
without any guarantee.

-----------------------------------------------------------------------------

This module is a self-contained Python port of the C++ implementation that
ships in the sibling `C++/` folder. The mapping is:

    libvibe++/common/ViBeBase.[h|cpp]         -> ViBeBase part of ViBeSequential
    libvibe++/common/ViBeTemplateBase.[h|t]   -> folded into ViBeSequential
    libvibe++/ViBe.[h|t]                      -> ViBeSequential
    libvibe++/distances/Manhattan.h           -> manhattan_match()
    libvibe++/metaprograms/DistanceL1.h       -> folded into manhattan_match()

The public API mirrors the C++ code:

    vibe = ViBeSequential(height, width, first_frame)
    vibe.segmentation(frame, segmentation_map)
    vibe.update(frame, segmentation_map)

`segmentation_map` is a HxW uint8 array filled with 0 (BACKGROUND) or
255 (FOREGROUND).

Only NumPy is required for the core algorithm; OpenCV is used by the CLI
front-end (see main.py) to decode video files.
"""

from __future__ import annotations

import numpy as np


# ---------------------------------------------------------------------------
# Distance: vectorised Manhattan / L1 match test
# ---------------------------------------------------------------------------

def manhattan_match(pixels_a: np.ndarray,
                    pixels_b: np.ndarray,
                    threshold: float,
                    channels: int) -> np.ndarray:
    """Return a boolean mask of pixels whose L1 distance is <= threshold.

    Mirrors ``ViBe::Manhattan<Channels>::distance`` (distances/Manhattan.h).
    The scaling factor comes from ``ManhattanFactor`` in the same header:

        * 1 channel : factor = 1.0
        * 3 channels: factor = 4.5

    For completeness, any other channel count falls back to ``channels * 1.5``.
    """
    if channels == 1:
        factor = 1.0
    elif channels == 3:
        factor = 4.5
    else:
        factor = float(channels) * 1.5

    a = pixels_a.astype(np.int32, copy=False)
    b = pixels_b.astype(np.int32, copy=False)

    if channels == 1:
        l1 = np.abs(a - b)
    else:
        l1 = np.sum(np.abs(a - b), axis=-1)

    return l1 <= (factor * threshold)


# ---------------------------------------------------------------------------
# ViBeSequential
# ---------------------------------------------------------------------------

class ViBeSequential:
    """Sequential (single-thread) Python port of ``ViBe::ViBeSequential``.

    Supports both 8UC1 (grayscale) and 8UC3 (color) inputs. The number of
    channels is inferred from the first frame passed to the constructor.
    """

    # --- public constants (identical to ViBeBase) --------------------------
    BACKGROUND = np.uint8(0)
    FOREGROUND = np.uint8(255)

    # --- default parameters (identical to ViBeBase) ------------------------
    DEFAULT_NUMBER_OF_SAMPLES = 30
    DEFAULT_MATCHING_THRESHOLD = 10
    DEFAULT_MATCHING_NUMBER = 2
    DEFAULT_UPDATE_FACTOR = 8
    NUMBER_OF_HISTORY_IMAGES = 2

    # ----------------------------------------------------------------------
    def __init__(self, height: int, width: int, buffer: np.ndarray) -> None:
        if height <= 0:
            raise ValueError("height must be > 0")
        if width <= 0:
            raise ValueError("width must be > 0")
        if buffer is None:
            raise ValueError("buffer must not be None")

        buffer = np.ascontiguousarray(buffer)
        if buffer.ndim == 2:
            channels = 1
        elif buffer.ndim == 3:
            channels = int(buffer.shape[2])
        else:
            raise ValueError("buffer must be a 2-D or 3-D ndarray")

        self.height = int(height)
        self.width = int(width)
        self.channels = channels

        # Parameters.
        self.number_of_samples = self.DEFAULT_NUMBER_OF_SAMPLES
        self.matching_threshold = self.DEFAULT_MATCHING_THRESHOLD
        self.matching_number = self.DEFAULT_MATCHING_NUMBER
        self.update_factor = self.DEFAULT_UPDATE_FACTOR

        # Common values.
        self.pixels = self.height * self.width
        self.num_history = self.NUMBER_OF_HISTORY_IMAGES
        self.num_tests = self.number_of_samples - self.num_history

        # history_image  : (N_H, H, W[, C])     - first NUMBER_OF_HISTORY_IMAGES
        #                                          samples, accessible as images.
        # history_buffer : (H, W, num_tests[, C]) - remaining samples, stored
        #                                            per pixel.
        shape_img = (self.num_history, self.height, self.width) + (
            (self.channels,) if self.channels > 1 else ())
        shape_buf = (self.height, self.width, self.num_tests) + (
            (self.channels,) if self.channels > 1 else ())

        self.history_image = np.empty(shape_img, dtype=np.uint8)
        for i in range(self.num_history):
            self.history_image[i] = buffer

        # Initial history buffer = first frame + uniform noise in [-10, +9],
        # clipped to [0, 255]. Matches the C++ ViBeBase constructor.
        noise = np.random.randint(-10, 10, size=shape_buf, dtype=np.int32)
        base = buffer.astype(np.int32)
        if self.channels == 1:
            base_b = base[:, :, None]           # (H, W, 1) broadcast over tests
        else:
            base_b = base[:, :, None, :]        # (H, W, 1, C)
        self.history_buffer = np.clip(base_b + noise, 0, 255).astype(np.uint8)

        self.last_history_image_swapped = 0

        # Random-sampling look-up tables used by update().
        size = 2 * max(self.width, self.height) + 1
        self.jump = np.random.randint(
            1, 2 * self.update_factor + 1, size=size).astype(np.uint32)
        nb = (np.random.randint(0, 3, size=size) - 1) \
            + (np.random.randint(0, 3, size=size) - 1) * self.width
        self.neighbor = nb.astype(np.int32)
        self.position = np.random.randint(
            0, self.number_of_samples, size=size).astype(np.uint32)

    # ----------------------------------------------------------------------
    # Getters / setters
    # ----------------------------------------------------------------------
    def get_number_of_samples(self) -> int:
        return self.number_of_samples

    def get_matching_threshold(self) -> int:
        return self.matching_threshold

    def set_matching_threshold(self, value: int) -> None:
        if value <= 0:
            raise ValueError("matching_threshold must be > 0")
        self.matching_threshold = int(value)

    def get_matching_number(self) -> int:
        return self.matching_number

    def set_matching_number(self, value: int) -> None:
        if value <= 0:
            raise ValueError("matching_number must be > 0")
        self.matching_number = int(value)

    def get_update_factor(self) -> int:
        return self.update_factor

    def set_update_factor(self, value: int) -> None:
        if value <= 0:
            raise ValueError("update_factor must be > 0")
        self.update_factor = int(value)
        size = 2 * max(self.width, self.height) + 1
        if self.update_factor == 1:
            self.jump = np.ones(size, dtype=np.uint32)
        else:
            self.jump = np.random.randint(
                1, 2 * self.update_factor + 1, size=size).astype(np.uint32)

    # ----------------------------------------------------------------------
    def __str__(self) -> str:
        return (
            f" - Number of samples per pixel    : {self.number_of_samples}\n"
            f" - Number of matches needed       : {self.matching_number}\n"
            f" - Matching threshold             : {self.matching_threshold}\n"
            f" - Model update subsampling factor: {self.update_factor}"
        )

    # ----------------------------------------------------------------------
    # Segmentation
    # ----------------------------------------------------------------------
    def segmentation(self,
                     buffer: np.ndarray,
                     segmentation_map: np.ndarray) -> None:
        """Build the foreground/background segmentation map for ``buffer``.

        Mirrors ``ViBeSequential::_CRTP_segmentation`` (ViBe.t).
        """
        if buffer is None or segmentation_map is None:
            raise ValueError("buffer and segmentation_map must not be None")

        H, W = self.height, self.width
        matching_number = self.matching_number
        threshold = self.matching_threshold
        channels = self.channels

        # Per-pixel "still needed" counter, initialised to matching_number - 1.
        remaining = np.full((H, W), matching_number - 1, dtype=np.int32)

        # --- First history image: a MISS bumps the counter back up --------
        # In C++: `if (!distance(...)) segMap[i] = matching_number;`
        match0 = manhattan_match(
            buffer, self.history_image[0], threshold, channels)
        remaining = np.where(match0, remaining, matching_number)

        # --- Remaining history images: a HIT decrements the counter -------
        for i in range(1, self.num_history):
            match_i = manhattan_match(
                buffer, self.history_image[i], threshold, channels)
            remaining = np.where(match_i, remaining - 1, remaining)

        # --- Rotate the swap index for the history-image swap -------------
        self.last_history_image_swapped = (
            (self.last_history_image_swapped + 1) % self.num_history)
        swap_idx = self.last_history_image_swapped

        # --- Search the history buffer for pixels that still need matches -
        if channels == 1:
            frame_b = buffer[:, :, None]                 # (H, W, 1)
        else:
            frame_b = buffer[:, :, None, :]              # (H, W, 1, C)

        matches = manhattan_match(
            frame_b, self.history_buffer, threshold, channels)
        # `matches` is (H, W, num_tests) bool.

        need_more = remaining > 0
        match_count = matches.sum(axis=2).astype(np.int32)

        achieved = need_more & (match_count >= remaining)
        remaining = np.where(
            achieved, remaining - match_count, remaining).astype(np.int32)

        if achieved.any():
            # argmax on a bool array returns the first True along axis=2.
            first_hit = np.argmax(matches, axis=2)
            ys, xs = np.nonzero(achieved)
            ks = first_hit[ys, xs]
            if channels == 1:
                tmp = self.history_image[swap_idx, ys, xs].copy()
                self.history_image[swap_idx, ys, xs] = \
                    self.history_buffer[ys, xs, ks]
                self.history_buffer[ys, xs, ks] = tmp
            else:
                tmp = self.history_image[swap_idx, ys, xs, :].copy()
                self.history_image[swap_idx, ys, xs, :] = \
                    self.history_buffer[ys, xs, ks, :]
                self.history_buffer[ys, xs, ks, :] = tmp

        # --- Produce output mask ------------------------------------------
        np.copyto(segmentation_map,
                  np.where(remaining > 0, self.FOREGROUND, self.BACKGROUND))

    # ----------------------------------------------------------------------
    # Update
    # ----------------------------------------------------------------------
    def update(self,
               buffer: np.ndarray,
               updating_mask: np.ndarray) -> None:
        """Randomly update the background model from ``buffer``.

        Mirrors ``ViBeSequential::_CRTP_update`` (ViBe.t). Pixels whose
        `updating_mask` value equals BACKGROUND (0) may be substituted into
        the model both at their own location and at a random 8-connected
        neighbour.
        """
        if buffer is None or updating_mask is None:
            raise ValueError("buffer and updating_mask must not be None")

        H, W = self.height, self.width
        BG = int(self.BACKGROUND)
        jump = self.jump
        neighbor = self.neighbor
        position = self.position

        # ---- interior rows (1 .. H-2) ------------------------------------
        for y in range(1, H - 1):
            shift = int(np.random.randint(0, W))
            indX = int(jump[shift])
            while indX < W - 1:
                if updating_mask[y, indX] == BG:
                    pos = int(position[shift])
                    nb_flat = int(neighbor[shift])
                    # Decompose flat neighbour offset (dx + dy * W).
                    ny, nx = y + (nb_flat // W), indX + (nb_flat % W)
                    self._substitute(buffer, y, indX, pos)
                    if 0 <= ny < H and 0 <= nx < W:
                        self._substitute(buffer, ny, nx, pos)
                shift += 1
                indX += int(jump[shift])

        # ---- first row ---------------------------------------------------
        self._update_border_row(buffer, updating_mask, 0)
        # ---- last row ----------------------------------------------------
        self._update_border_row(buffer, updating_mask, H - 1)
        # ---- first column ------------------------------------------------
        self._update_border_col(buffer, updating_mask, 0)
        # ---- last column -------------------------------------------------
        self._update_border_col(buffer, updating_mask, W - 1)

        # ---- pixel (0, 0): random update with probability 1/update_factor
        if np.random.randint(0, self.update_factor) == 0:
            if updating_mask[0, 0] == 0:
                pos = int(np.random.randint(0, self.number_of_samples))
                self._substitute(buffer, 0, 0, pos)

    # ----------------------------------------------------------------------
    # Helpers
    # ----------------------------------------------------------------------
    def _substitute(self, buffer: np.ndarray,
                    y: int, x: int, pos: int) -> None:
        """Write the pixel (y, x) of ``buffer`` into sample index ``pos``.

        Positions [0 .. NUMBER_OF_HISTORY_IMAGES - 1] map to history_image;
        the rest map to history_buffer.
        """
        if pos < self.num_history:
            self.history_image[pos, y, x] = buffer[y, x]
        else:
            k = pos - self.num_history
            if self.channels == 1:
                self.history_buffer[y, x, k] = buffer[y, x]
            else:
                self.history_buffer[y, x, k, :] = buffer[y, x, :]

    def _update_border_row(self, buffer, updating_mask, y) -> None:
        W = self.width
        shift = int(np.random.randint(0, W))
        indX = int(self.jump[shift])
        while indX <= W - 1:
            if updating_mask[y, indX] == self.BACKGROUND:
                self._substitute(buffer, y, indX, int(self.position[shift]))
            shift += 1
            indX += int(self.jump[shift])

    def _update_border_col(self, buffer, updating_mask, x) -> None:
        H = self.height
        shift = int(np.random.randint(0, H))
        indY = int(self.jump[shift])
        while indY <= H - 1:
            if updating_mask[indY, x] == self.BACKGROUND:
                self._substitute(buffer, indY, x, int(self.position[shift]))
            shift += 1
            indY += int(self.jump[shift])
