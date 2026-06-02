/* Copyright - Benjamin Laugraud - 2016
 * Copyright - Marc Van Droogenbroeck - 2016
 *
 * ViBe was originally covered by a patent that is now in the public domain.
 * See the accompanying LICENSE file for details.
 *
 * Modernization (2026): dropped the Boost.cstdint dependency in favor of
 * the standard C++11 <cstdint> header.
 */
#ifndef _LIB_VIBE_XX_SYSTEM_TYPES_H_
#define _LIB_VIBE_XX_SYSTEM_TYPES_H_

#include <cstdint>

namespace ViBe {
  typedef std::int8_t                                                   int8_t;
  typedef std::int32_t                                                 int32_t;

  typedef std::uint8_t                                                 uint8_t;
  typedef std::uint32_t                                               uint32_t;
}

#endif /* _LIB_VIBE_XX_SYSTEM_TYPES_H_ */
