/* Copyright - Benjamin Laugraud <blaugraud@ulg.ac.be> - 2016
 * Copyright - Marc Van Droogenbroeck <m.vandroogenbroeck@ulg.ac.be> - 2016
 *
 * ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).
 *
 * Permission to use ViBe without payment of fee is granted for nonprofit
 * educational and research purposes only.
 *
 * This work may not be copied or reproduced in whole or in part for any
 * purpose.
 *
 * Copying, reproduction, or republishing for any purpose shall require a
 * license. Please contact the authors in such cases. All the code is provided
 * without any guarantee.
 */
#ifdef _LIB_VIBE_XX_VIBE_H_

/* ========================================================================== *
 * ViBeSequential<Channels, Distance>                                         *
 * ========================================================================== */

template <int32_t Channels, class Distance>
ViBeSequential<Channels, Distance>::ViBeSequential(
  int32_t height,
  int32_t width,
  const uint8_t* buffer
) : Base(
    height,
    width,
    Channels,
    buffer
  ) {}

/******************************************************************************/

template <int32_t Channels, class Distance>
void ViBeSequential<Channels, Distance>::_CRTP_segmentation(
  const uint8_t* buffer,
  uint8_t* segmentationMap
) {
#ifndef    NDEBUG
  if (buffer == NULL)
    throw; // TODO Exception

  if (segmentationMap == NULL)
    throw; // TODO Exception
#endif  /* NDEBUG */

  /* Even though those variables/contents are redundant with the variables of
   * ViBeBase, they avoid additional dereference instructions.
   */
  static const uint32_t NUMBER_OF_HISTORY_IMAGES =
    this->NUMBER_OF_HISTORY_IMAGES;

  static const uint8_t FOREGROUND = this->FOREGROUND;

  uint32_t pixels = this->pixels;
  uint32_t numValues = this->numValues;
  uint32_t matchingNumber = this->matchingNumber;
  uint32_t matchingThreshold = this->matchingThreshold;

  uint8_t* historyImage = this->historyImage;
  uint8_t* historyBuffer = this->historyBuffer;

  /* Initialize segmentation map. */
  memset(segmentationMap, matchingNumber - 1, pixels);

  /* First history Image structure. */
  for (int32_t index = pixels - 1; index >= 0; --index) {
    if (
      !Distance::distance(
        buffer + (Channels * index),
        historyImage + (Channels * index),
        matchingThreshold
      )
    )
      segmentationMap[index] = matchingNumber;
  }

  /* Next historyImages. */
  for (uint32_t i = 1; i < NUMBER_OF_HISTORY_IMAGES; ++i) {
    uint8_t* pels = historyImage + i * numValues;

    for (int32_t index = pixels - 1; index >= 0; --index) {
      if (
        Distance::distance(
          buffer + (Channels * index),
          pels + (Channels * index),
          matchingThreshold
        )
      )
        --segmentationMap[index];
    }
  }

  /* For swapping. */
  this->lastHistoryImageSwapped =
    (this->lastHistoryImageSwapped + 1) % NUMBER_OF_HISTORY_IMAGES;

  uint8_t* swappingImageBuffer =
    historyImage + (this->lastHistoryImageSwapped) * numValues;

  /* Now, we move in the buffer and leave the historyImages. */
  int32_t numberOfTests = (this->numberOfSamples - NUMBER_OF_HISTORY_IMAGES);

  for (int32_t index = pixels - 1; index >= 0; --index) {
    if (segmentationMap[index] > 0) {
      /* We need to check the full border and swap values with the first or
       * second historyImage. We still need to find a match before we can stop
       * our search.
       */
      uint32_t indexHistoryBuffer = (Channels * index) * numberOfTests;
      uint8_t currentValue[Channels];

      internals::CopyPixel<Channels>::copy(
        &(currentValue[0]),
        buffer + (Channels * index)
      );

      for (int i = numberOfTests; i > 0; --i, indexHistoryBuffer += Channels) {
        if (
          Distance::distance(
            &(currentValue[0]),
            historyBuffer + indexHistoryBuffer,
            matchingThreshold
          )
        ) {
          --segmentationMap[index];

          /* Swaping: Putting found value in history image buffer. */
          internals::SwapPixels<Channels>::swap(
            swappingImageBuffer + (Channels * index),
            historyBuffer + indexHistoryBuffer
          );

          /* Exit inner loop. */
          if (segmentationMap[index] <= 0)
            break;
        }
      }
    }
  }

  /* Produces the output. Note that this step is application-dependent. */
  for (
    uint8_t* mask = segmentationMap;
    mask < segmentationMap + pixels;
    ++mask
  ) {
    if (*mask > 0)
      *mask = FOREGROUND;
  }
}

/******************************************************************************/

template <int32_t Channels, class Distance>
void ViBeSequential<Channels, Distance>::_CRTP_update(
  const uint8_t* buffer,
  uint8_t* updatingMask
) {
#ifndef    NDEBUG
  if (buffer == NULL)
    throw; // TODO Exception

  if (updatingMask == NULL)
    throw; // TODO Exception
#endif  /* NDEBUG */

  /* Some variables. */
  static const uint32_t NUMBER_OF_HISTORY_IMAGES =
    this->NUMBER_OF_HISTORY_IMAGES;

  static const uint8_t BACKGROUND = this->BACKGROUND;

  uint32_t height = this->height;
  uint32_t width = this->width;
  uint32_t numValues = this->numValues;

  uint8_t* historyImage = this->historyImage;
  uint8_t* historyBuffer = this->historyBuffer;

  /* Some utility variable. */
  int numberOfTests = (this->numberOfSamples - NUMBER_OF_HISTORY_IMAGES);

  uint32_t* jump = this->jump;
  int32_t* neighbor = this->neighbor;
  uint32_t* position = this->position;

  /* All the frame, except the border. */
  uint32_t shift, indX, indY;
  uint32_t x, y;

  for (y = 1; y < height - 1; ++y) {
    shift = rand() % width;
    indX = jump[shift]; // index_jump should never be zero (> 1).

    while (indX < width - 1) {
      int index = indX + y * width;

      if (updatingMask[index] == BACKGROUND) {
        /* In-place substitution. */
        uint8_t currentValue[Channels];

        internals::CopyPixel<Channels>::copy(
          &(currentValue[0]),
          buffer + (Channels * index)
        );

        int indexNeighbor = Channels * (index + neighbor[shift]);

        if (position[shift] < NUMBER_OF_HISTORY_IMAGES) {
          internals::CopyPixel<Channels>::copy(
            historyImage + (Channels * index + position[shift] * numValues),
            &(currentValue[0])
          );

          internals::CopyPixel<Channels>::copy(
            historyImage + (indexNeighbor + position[shift] * numValues),
            &(currentValue[0])
          );
        }
        else {
          int pos = position[shift] - NUMBER_OF_HISTORY_IMAGES;

          internals::CopyPixel<Channels>::copy(
            historyBuffer + (
              (Channels * index) * numberOfTests + Channels * pos
            ),
            &(currentValue[0])
          );

          internals::CopyPixel<Channels>::copy(
            historyBuffer + (indexNeighbor * numberOfTests + Channels * pos),
            &(currentValue[0])
          );
        }
      }

      ++shift;
      indX += jump[shift];
    }
  }

  /* First row. */
  y = 0;
  shift = rand() % width;
  indX = jump[shift]; // index_jump should never be zero (> 1).

  while (indX <= width - 1) {
    int index = indX + y * width;

    if (updatingMask[index] == BACKGROUND) {
      if (position[shift] < NUMBER_OF_HISTORY_IMAGES) {
        internals::CopyPixel<Channels>::copy(
          historyImage + (Channels * index + position[shift] * numValues),
          buffer + (Channels * index)
        );
      }
      else {
        int pos = position[shift] - NUMBER_OF_HISTORY_IMAGES;

        internals::CopyPixel<Channels>::copy(
          historyBuffer + ((Channels * index) * numberOfTests + Channels * pos),
          buffer + (Channels * index)
        );
      }
    }

    ++shift;
    indX += jump[shift];
  }

  /* Last row. */
  y = height - 1;
  shift = rand() % width;
  indX = jump[shift]; // index_jump should never be zero (> 1).

  while (indX <= width - 1) {
    int index = indX + y * width;

    if (updatingMask[index] == BACKGROUND) {
      if (position[shift] < NUMBER_OF_HISTORY_IMAGES) {
        internals::CopyPixel<Channels>::copy(
          historyImage + (Channels * index + position[shift] * numValues),
          buffer + (Channels * index)
        );
      }
      else {
        int pos = position[shift] - NUMBER_OF_HISTORY_IMAGES;

        internals::CopyPixel<Channels>::copy(
          historyBuffer + ((Channels * index) * numberOfTests + Channels * pos),
          buffer + (Channels * index)
        );
      }
    }

    ++shift;
    indX += jump[shift];
  }

  /* First column. */
  x = 0;
  shift = rand() % height;
  indY = jump[shift]; // index_jump should never be zero (> 1).

  while (indY <= height - 1) {
    int index = x + indY * width;

    if (updatingMask[index] == BACKGROUND) {
      if (position[shift] < NUMBER_OF_HISTORY_IMAGES) {
        internals::CopyPixel<Channels>::copy(
          historyImage + (Channels * index + position[shift] * numValues),
          buffer + (Channels * index)
        );
      }
      else {
        int pos = position[shift] - NUMBER_OF_HISTORY_IMAGES;

        internals::CopyPixel<Channels>::copy(
          historyBuffer + ((Channels * index) * numberOfTests + Channels * pos),
          buffer + (Channels * index)
        );
      }
    }

    ++shift;
    indY += jump[shift];
  }

  /* Last column. */
  x = width - 1;
  shift = rand() % height;
  indY = jump[shift]; // index_jump should never be zero (> 1).

  while (indY <= height - 1) {
    int index = x + indY * width;

    if (updatingMask[index] == BACKGROUND) {
      if (position[shift] < NUMBER_OF_HISTORY_IMAGES) {
        internals::CopyPixel<Channels>::copy(
          historyImage + (Channels * index + position[shift] * numValues),
          buffer + (Channels * index)
        );
      }
      else {
        int pos = position[shift] - NUMBER_OF_HISTORY_IMAGES;

        internals::CopyPixel<Channels>::copy(
          historyBuffer + ((Channels * index) * numberOfTests + Channels * pos),
          buffer + (Channels * index)
        );
      }
    }

    ++shift;
    indY += jump[shift];
  }

  /* The first pixel! */
  if (rand() % this->updateFactor == 0) {
    if (updatingMask[0] == 0) {
      uint32_t position = rand() % this->numberOfSamples;

      if (position < NUMBER_OF_HISTORY_IMAGES) {
        internals::CopyPixel<Channels>::copy(
          historyImage + (position * numValues),
          buffer
        );
      }
      else {
        int pos = position - NUMBER_OF_HISTORY_IMAGES;

        internals::CopyPixel<Channels>::copy(
          historyBuffer + (Channels * pos),
          buffer
        );
      }
    }
  }
}

#endif /* _LIB_VIBE_XX_VIBE_H_ */
