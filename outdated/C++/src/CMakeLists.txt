# Copyright - Benjamin Laugraud <blaugraud@ulg.ac.be> - 2016
# Copyright - Marc Van Droogenbroeck <m.vandroogenbroeck@ulg.ac.be> - 2016
#
# ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).
#
# Permission to use ViBe without payment of fee is granted for nonprofit
# educational and research purposes only.
#
# This work may not be copied or reproduced in whole or in part for any purpose.
#
# Copying, reproduction, or republishing for any purpose shall require a
# license. Please contact the authors in such cases. All the code is provided
# without any guarantee.

file(
  GLOB_RECURSE
  vibe++_src
  libvibe++/*.cpp
)

file(
  GLOB_RECURSE
  vibe++_include
  libvibe++/*.h
  libvibe++/*.t
)

add_library(
  vibe++
  SHARED
  ${vibe++_src}
  ${vibe++_include}
)

add_library(
  vibe++_static
  STATIC
  ${vibe++_src}
  ${vibe++_include}
)

set_target_properties(
  vibe++_static
  PROPERTIES
  OUTPUT_NAME
  vibe++
)

install(
  TARGETS vibe++
  LIBRARY NAMELINK_SKIP
  DESTINATION lib
  CONFIGURATIONS Release
  COMPONENT Runtime
)

install(
  TARGETS vibe++_static
  DESTINATION lib
  CONFIGURATIONS Release
  COMPONENT Development
)

install(
  TARGETS vibe++
  LIBRARY NAMELINK_ONLY
  DESTINATION lib
  CONFIGURATIONS Release
  COMPONENT Development
)

install(
  DIRECTORY libvibe++
  DESTINATION include
  FILES_MATCHING PATTERN "*.h" PATTERN "*.t"
)