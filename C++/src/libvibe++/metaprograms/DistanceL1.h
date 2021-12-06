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
#ifndef _LIB_VIBE_XX_METAPROGRAMS_DISTANCE_L1_H_
#define _LIB_VIBE_XX_METAPROGRAMS_DISTANCE_L1_H_

#include "../math/AbsoluteValue.h"
#include "../system/inline.h"
#include "../system/types.h"

namespace ViBe {
  namespace internals {
    /* ====================================================================== *
     * DistanceL1<Channels>                                                   *
     * ====================================================================== */

    template <int32_t Channels, typename Encoding = uint8_t>
    struct DistanceL1 {
      STRONG_INLINE static uint32_t add(
        const Encoding* pixel1,
        const Encoding* pixel2
      ) {
        return (
          AbsoluteValue::abs(*pixel1 - *pixel2) +
          DistanceL1<Channels - 1>::add(pixel1 + 1, pixel2 + 1)
        );
      }
    };

    /* ====================================================================== *
     * DistanceL1<1>                                                          *
     * ====================================================================== */

    template <typename Encoding>
    struct DistanceL1<1, Encoding> {
      STRONG_INLINE static uint32_t add(
        const Encoding* pixel1,
        const Encoding* pixel2
      ) {
        return AbsoluteValue::abs(*pixel1 - *pixel2);
      }
    };
  } /* _NS_internals_ */
} /* _NS_ViBe_ */

#endif /* _LIB_VIBE_XX_METAPROGRAMS_DISTANCE_L1_H_ */
