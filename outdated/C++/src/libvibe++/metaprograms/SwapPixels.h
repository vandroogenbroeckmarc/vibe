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
#ifndef _LIB_VIBE_XX_METAPROGRAMS_SWAP_PIXELS_H_
#define _LIB_VIBE_XX_METAPROGRAMS_SWAP_PIXELS_H_

#include "CopyPixel.h"
#include "../system/inline.h"
#include "../system/types.h"

namespace ViBe {
  namespace internals {
    /* ====================================================================== *
     * SwapPixels<Channels>                                                   *
     * ====================================================================== */

    template <int32_t Channels, typename Encoding = uint8_t>
    struct SwapPixels {
      STRONG_INLINE static void swap(
        Encoding* pixel1,
        Encoding* pixel2
      ) {
        Encoding tmp[Channels];
        CopyPixel<Channels, Encoding>::copy(&(tmp[0]), pixel1);
        CopyPixel<Channels, Encoding>::copy(pixel1, pixel2);
        CopyPixel<Channels, Encoding>::copy(pixel2, tmp);
      }
    };
  } /* _NS_internals_ */
} /* _NS_ViBe_ */

#endif /* _LIB_VIBE_XX_METAPROGRAMS_SWAP_PIXELS_H_ */
