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
#ifdef _LIB_VIBE_XX_COMMON_VIBE_TEMPLATE_BASE_H_

/* ========================================================================== *
 * ViBeTemplateBase                                                           *
 * ========================================================================== */

template <class Derived>
ViBeTemplateBase<Derived>::ViBeTemplateBase(
  int32_t height,
  int32_t width,
  int32_t channels,
  const uint8_t* buffer
) :
  Base(
    height,
    width,
    channels,
    buffer
  ) {}

/******************************************************************************/

template <class Derived>
inline void ViBeTemplateBase<Derived>::segmentation(
  const uint8_t* buffer,
  uint8_t* segmentationMap
) {
  static_cast<Derived*>(this)->_CRTP_segmentation(buffer, segmentationMap);
}

/******************************************************************************/

template <class Derived>
inline void ViBeTemplateBase<Derived>::update(
  const uint8_t* buffer,
  uint8_t* updatingMask
) {
  static_cast<Derived*>(this)->_CRTP_update(buffer, updatingMask);
}

#endif /* _LIB_VIBE_XX_COMMON_VIBE_TEMPLATE_BASE_H_ */
