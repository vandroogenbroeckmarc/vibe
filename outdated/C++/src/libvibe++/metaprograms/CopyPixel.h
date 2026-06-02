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
#ifndef _LIB_VIBE_XX_METAPROGRAMS_COPY_PIXEL_H_
#define _LIB_VIBE_XX_METAPROGRAMS_COPY_PIXEL_H_

#include "../system/inline.h"
#include "../system/types.h"

namespace ViBe {
  namespace internals {
    /* ====================================================================== *
     * CopyPixel<Channels>                                                    *
     * ====================================================================== */

    template <int32_t Channels, typename Encoding = uint8_t>
    struct CopyPixel {
      STRONG_INLINE static void copy(
        Encoding* destination,
        const Encoding* source
      ) {
        *destination = *source;
        CopyPixel<Channels - 1, Encoding>::copy(destination + 1, source + 1);
      }
    };

    /* ====================================================================== *
     * CopyPixel<1>                                                           *
     * ====================================================================== */

    template <typename Encoding>
    struct CopyPixel<1, Encoding> {
      STRONG_INLINE static void copy(
        Encoding* destination,
        const Encoding* source
      ) {
        *destination = *source;
      }
    };
  } /* _NS_internals_ */
} /* _NS_ViBe_ */

#endif /* _LIB_VIBE_XX_METAPROGRAMS_COPY_PIXEL_H_ */
