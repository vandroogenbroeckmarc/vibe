# ViBe (C implementation) — modernized CMake build (2026).
#
# This CMakeLists.txt replaces the plain Makefile on Windows (MSVC) and
# offers a single cross-platform build path. The Makefile is kept in place
# for users who prefer the traditional Unix workflow.
#
# Example usage:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   cmake --install build --prefix <install-prefix>

cmake_minimum_required(VERSION 3.15)

project(libvibe-c
  VERSION 2.0.0
  DESCRIPTION "ViBe: a universal background subtraction algorithm (C implementation)"
  HOMEPAGE_URL "http://www.telecom.ulg.ac.be/research/vibe"
  LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_SHARED_LIBS             "Build libvibe as a shared library" OFF)
option(LIBVIBE_C_BUILD_EXAMPLE_PLAIN "Build the pure-C example program (main)"       ON)
option(LIBVIBE_C_BUILD_EXAMPLE_OPENCV "Build the OpenCV-based example (main-opencv)" ON)

# ---- Library ----------------------------------------------------------------

add_library(vibe
  vibe-background-sequential.c
  vibe-background-sequential.h
)
add_library(vibe::vibe ALIAS vibe)

target_include_directories(vibe
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(vibe PUBLIC c_std_11)

target_compile_options(vibe PRIVATE
  $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
  $<$<C_COMPILER_ID:MSVC>:/W3>
)

set_target_properties(vibe PROPERTIES
  VERSION   ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  OUTPUT_NAME vibe
  EXPORT_NAME vibe
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# ---- Examples ---------------------------------------------------------------

if(LIBVIBE_C_BUILD_EXAMPLE_PLAIN)
  add_executable(main your-main-file-sequential.c)
  target_link_libraries(main PRIVATE vibe::vibe)
  target_compile_features(main PRIVATE c_std_11)
  install(TARGETS main RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(LIBVIBE_C_BUILD_EXAMPLE_OPENCV)
  find_package(OpenCV 4.5 QUIET COMPONENTS core highgui imgproc videoio)
  if(OpenCV_FOUND)
    add_executable(main-opencv main-opencv.cpp)
    target_link_libraries(main-opencv PRIVATE
      vibe::vibe
      opencv_core opencv_highgui opencv_imgproc opencv_videoio
    )
    target_compile_features(main-opencv PRIVATE cxx_std_17)
    install(TARGETS main-opencv RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  else()
    message(STATUS
      "OpenCV 4.5+ not found — skipping main-opencv. "
      "Disable this warning with -DLIBVIBE_C_BUILD_EXAMPLE_OPENCV=OFF.")
  endif()
endif()

# ---- Install + package config ----------------------------------------------

install(
  TARGETS vibe
  EXPORT  vibeTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES vibe-background-sequential.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(vibe_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/vibe)

install(
  EXPORT vibeTargets
  FILE   vibeTargets.cmake
  NAMESPACE vibe::
  DESTINATION ${vibe_CMAKE_CONFIG_DIR}
)

configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/cmake/vibeConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/vibeConfig.cmake
  INSTALL_DESTINATION ${vibe_CMAKE_CONFIG_DIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/vibeConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vibeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vibeConfigVersion.cmake
  DESTINATION ${vibe_CMAKE_CONFIG_DIR}
)
