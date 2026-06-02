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
#ifndef _LIB_VIBE_XX_MATH_ABSOLUTE_VALUE_H_
#define _LIB_VIBE_XX_MATH_ABSOLUTE_VALUE_H_

#include "../system/inline.h"

namespace ViBe {
  namespace internals {
    struct AbsoluteValue {
      // TODO static assert in the case where T is not a primitive value.
      template <typename T>
      STRONG_INLINE static T abs(T value) {
        return (value >= 0) ? value : -value;
      }
    };
  } /* _NS_internals_ */
} /* _NS_ViBe_ */

#endif /* _LIB_VIBE_XX_MATH_ABSOLUTE_VALUE_H_ */
