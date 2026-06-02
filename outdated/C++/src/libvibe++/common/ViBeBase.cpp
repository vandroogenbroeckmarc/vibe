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
#include <algorithm>
#include <cstddef>

#include "ViBeBase.h"

using namespace std;
using namespace ViBe;

namespace ViBe {
  inline ostream& operator<<(ostream& os, const ViBeBase& vibe) {
    vibe.print(os);
    return os;
  }
}

/* ========================================================================== *
 * ViBeBase                                                                   *
 * ========================================================================== */

// TODO Replace rand()
// TODO Add noise using generator?
ViBeBase::ViBeBase(
  int32_t height,
  int32_t width,
  int32_t channels,
  const uint8_t* buffer
) :
  height(height),
  width(width),
  numberOfSamples(DEFAULT_NUMBER_OF_SAMPLES),
  matchingThreshold(DEFAULT_MATCHING_THRESHOLD),
  matchingNumber(DEFAULT_MATCHING_NUMBER),
  updateFactor(DEFAULT_UPDATE_FACTOR),
  stride(width * channels),
  pixels(height * width),
  numValues(height * width * channels),
  historyImage(NULL),
  historyBuffer(NULL),
  lastHistoryImageSwapped(),
  jump(NULL),
  neighbor(NULL),
  position(NULL) {

  if (height <= 0)
    throw; // TODO Exception

  if (width <= 0)
    throw; // TODO Exception

  if (channels <= 0)
    throw; // TODO Exception

  if (buffer == NULL)
    throw; // TODO Exception

  const uint32_t COLUMNS = width * channels;

  /* Creates the historyImage structure. */
  historyImage = new uint8_t[NUMBER_OF_HISTORY_IMAGES * COLUMNS * height];

  for (uint32_t i = 0; i < NUMBER_OF_HISTORY_IMAGES; ++i) {
    for (int32_t index = COLUMNS * height - 1; index >= 0; --index)
      historyImage[i * COLUMNS * height + index] = buffer[index];
  }

  /* Now creates and fills the history buffer. */
  historyBuffer =
    new uint8_t[COLUMNS * height * (numberOfSamples - NUMBER_OF_HISTORY_IMAGES)];

  for (int32_t index = COLUMNS * height - 1; index >= 0; --index) {
    uint8_t value = buffer[index];

    for (uint32_t x = 0; x < numberOfSamples - NUMBER_OF_HISTORY_IMAGES; ++x) {
      historyBuffer[index * (numberOfSamples - NUMBER_OF_HISTORY_IMAGES) + x] =
        min(
          max(
            static_cast<int32_t>(value) + rand() % 20 - 10, // Add noise.
            static_cast<int32_t>(BACKGROUND)
          ),
          static_cast<int32_t>(FOREGROUND)
        );
    }
  }

  /* Fills the buffers with random values. */
  int32_t size = (width > height) ? 2 * width + 1 : 2 * height + 1;

  jump = new uint32_t[size];
  neighbor = new int32_t[size];
  position = new uint32_t[size];

  for (int32_t i = 0; i < size; ++i) {
    /* Values between 1 and 2 * updateFactor. */
    jump[i] = (rand() % (2 * updateFactor)) + 1;
    /* Values between { -width - 1, ... , width + 1 }. */
    neighbor[i] = ((rand() % 3) - 1) + ((rand() % 3) - 1) * width;
    /* Values between 0 and numberOfSamples - 1. */
    position[i] = rand() % numberOfSamples;
  }
}

/******************************************************************************/

ViBeBase::~ViBeBase() {
  delete[] historyImage;
  delete[] historyBuffer;
  delete[] jump;
  delete[] neighbor;
  delete[] position;
}

/******************************************************************************/

uint32_t ViBeBase::getNumberOfSamples() const {
  return numberOfSamples;
}

/******************************************************************************/

uint32_t ViBeBase::getMatchingThreshold() const {
  return matchingThreshold;
}

/******************************************************************************/

void ViBeBase::setMatchingThreshold(int32_t matchingThreshold) {
  if (matchingThreshold <= 0)
    throw; // TODO Exception;

  this->matchingThreshold = matchingThreshold;
}

/******************************************************************************/

uint32_t ViBeBase::getMatchingNumber() const {
  return matchingNumber;
}

/******************************************************************************/

void ViBeBase::setMatchingNumber(int32_t matchingNumber) {
  if (matchingNumber <= 0)
    throw; // TODO Exception;

  this->matchingNumber = matchingNumber;
}

/******************************************************************************/

uint32_t ViBeBase::getUpdateFactor() const {
  return updateFactor;
}

/******************************************************************************/

void ViBeBase::setUpdateFactor(int32_t updateFactor) {
  if (updateFactor <= 0)
    throw; // TODO Exception;

  this->updateFactor = updateFactor;

  /* We also need to change the values of the jump buffer ! */
  int32_t size = 2 * max(width, height) + 1;

  for (int32_t i = 0; i < size; ++i) {
    // 1 or values between 1 and 2 * updateFactor.
    jump[i] = (updateFactor == 1) ? 1 : (rand() % (2 * updateFactor)) + 1;
  }
}

/******************************************************************************/

void ViBeBase::print(ostream& os) const {
  os << " - Number of samples per pixel    : " << numberOfSamples   << endl;
  os << " - Number of matches needed       : " << matchingNumber    << endl;
  os << " - Matching threshold             : " << matchingThreshold << endl;
  os << " - Model update subsampling factor: " << updateFactor             ;
}
