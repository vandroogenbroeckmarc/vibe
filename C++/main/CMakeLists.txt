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
include(BenlaugUtils)

option(
  libvibe++_COMPILE_EXAMPLES
  "Compilation of the examples programs using the libvibe++ library"
  ON
)

if    (libvibe++_COMPILE_EXAMPLES)
  # C++ flags.
  if    (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
  else  (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif (CMAKE_COMPILER_IS_GNUCXX)

  # OpenCV.
  find_package(OpenCV REQUIRED)
  include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

  if    (${OpenCV_VERSION_MAJOR} VERSION_LESS 3)
    find_package(OpenCV REQUIRED core highgui imgproc)
  else  ()
    find_package(OpenCV REQUIRED core highgui imgproc videoio)
    add_definitions(-DOPENCV_3)
  endif (${OpenCV_VERSION_MAJOR} VERSION_LESS 3)

  file(
    GLOB
    main_src
    *.cpp
  )

  foreach    (EXE ${main_src})
    benlaug_executable_name(${EXE})

    add_executable(
      ${EXE_NAME}
      ${EXE}
    )

    target_link_libraries(
      ${EXE_NAME}
      vibe++_static
      ${OpenCV_LIBS}
      ${Boost_LIBRARIES}
    )
  endforeach (EXE ${main_src})
endif (libvibe++_COMPILE_EXAMPLES)
