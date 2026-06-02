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

# CMake parameter(s).
cmake_minimum_required(VERSION 2.8)
include(CMakeDependentOption)

# Disable RPATH support for OS X.
if    (${CMAKE_MAJOR_VERSION} EQUAL 3 AND APPLE)
  cmake_policy(SET CMP0042 OLD)
endif (${CMAKE_MAJOR_VERSION} EQUAL 3 AND APPLE)

# Project name.
project(libvibe++)

# Custom CMake modules path.
list(APPEND CMAKE_MODULE_PATH ${libvibe++_SOURCE_DIR}/cmake)

# C++ flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g")

# Boost.
find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Include src directory.
include_directories(src)

# Recursion baby.
add_subdirectory(src)
add_subdirectory(main)
