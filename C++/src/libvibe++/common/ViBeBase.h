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
#ifndef _LIB_VIBE_XX_COMMON_VIBE_BASE_H_
#define _LIB_VIBE_XX_COMMON_VIBE_BASE_H_

#include <iostream>

#include "../system/types.h"

namespace ViBe {
  class ViBeBase {
      friend std::ostream& operator<<(std::ostream& os, const ViBeBase& vibe);

    public:

      static const uint8_t  BACKGROUND                 =                     0;
      static const uint8_t  FOREGROUND                 =                   255;

    protected:

      static const uint32_t DEFAULT_NUMBER_OF_SAMPLES  =                    20;
      static const uint32_t DEFAULT_MATCHING_THRESHOLD =                    20;
      static const uint32_t DEFAULT_MATCHING_NUMBER    =                     2;
      static const uint32_t DEFAULT_UPDATE_FACTOR      =                    16;
      static const uint32_t NUMBER_OF_HISTORY_IMAGES   =                     2;

    protected:

      /* Parameters. */
      uint32_t height;
      uint32_t width;
      uint32_t numberOfSamples;
      uint32_t matchingThreshold;
      uint32_t matchingNumber;
      uint32_t updateFactor;

      /* Common values. */
      uint32_t stride;
      uint32_t pixels;
      uint32_t numValues;

      /* Storage for the history. */
      uint8_t* historyImage;
      uint8_t* historyBuffer;
      uint32_t lastHistoryImageSwapped;

      /* Buffers with random values. */
      uint32_t* jump;
      int32_t* neighbor;
      uint32_t* position;

    protected:

      ViBeBase(
        int32_t height,
        int32_t width,
        int32_t channels,
        const uint8_t* buffer
      );

    public:

      virtual ~ViBeBase();

      uint32_t getNumberOfSamples() const;

      uint32_t getMatchingThreshold() const;

      void setMatchingThreshold(int32_t matchingThreshold);

      uint32_t getMatchingNumber() const;

      void setMatchingNumber(int32_t matchingNumber);

      uint32_t getUpdateFactor() const;

      void setUpdateFactor(int32_t updateFactor);

    public://TODO protected and debug operator overload.

      virtual void print(std::ostream& os = std::cout) const;
  };
}

#endif /* _LIB_VIBE_XX_COMMON_VIBE_BASE_H_ */
