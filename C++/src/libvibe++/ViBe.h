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
#ifndef _LIB_VIBE_XX_VIBE_H_
#define _LIB_VIBE_XX_VIBE_H_

#include <cstring>

#include "common/ViBeTemplateBase.h"
#include "metaprograms/CopyPixel.h"
#include "metaprograms/SwapPixels.h"

namespace ViBe {
  template <int32_t Channels, class Distance>
  class ViBeSequential : public ViBeTemplateBase<ViBeSequential<Channels, Distance> > {
    protected:

      typedef ViBeTemplateBase<ViBeSequential<Channels, Distance> >       Base;

    public:

      ViBeSequential(
        int32_t height,
        int32_t width,
        const uint8_t* buffer
      );

      virtual ~ViBeSequential() {}

      void _CRTP_segmentation(
        const uint8_t* buffer,
        uint8_t* segmentationMap
      );

      void _CRTP_update(
        const uint8_t* buffer,
        uint8_t* updatingMask
      );
  };

#include "ViBe.t"
} /* _NS_ViBe_ */

#endif /* _LIB_VIBE_XX_VIBE_H_ */
