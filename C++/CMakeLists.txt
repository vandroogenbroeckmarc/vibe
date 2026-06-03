# Copyright - Benjamin Laugraud - 2016
# Copyright - Marc Van Droogenbroeck - 2016
#
# ViBe was originally covered by a patent that is now in the public domain.
# See the accompanying LICENSE file for details.
#
# Modernization (2026):
#   - Raised cmake_minimum_required to 3.15 (target-based CMake floor).
#   - Dropped Boost dependency (now <cstdint> only).
#   - Moved compiler warnings to target properties with generator expressions
#     so MSVC builds do not fail on GCC-only flags.
#   - Delegates all target setup to the sub-directories.

cmake_minimum_required(VERSION 3.15)

project(
  libvibe++
  VERSION 2.0.0
  DESCRIPTION "ViBe: a universal background subtraction algorithm"
  HOMEPAGE_URL "http://www.telecom.ulg.ac.be/research/vibe"
  LANGUAGES CXX
)

# Honour the user's CMAKE_BUILD_TYPE. Default to Release for single-config
# generators so a bare `cmake -S . -B build` produces optimized binaries.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
               "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# BUILD_SHARED_LIBS controls whether libvibe++ is built as shared or static.
option(BUILD_SHARED_LIBS "Build libvibe++ as a shared library" ON)

# Optional compilation of the example programs.
option(LIBVIBEPP_COMPILE_EXAMPLES
       "Compile the example programs that depend on OpenCV" ON)

# Symbol visibility.
#
# The library's public classes (ViBeBase, ...) are not annotated with the
# VIBEXX_EXPORT macro produced by generate_export_header, so we keep the
# platform defaults: "default" visibility on ELF/Mach-O (everything
# exported) and WINDOWS_EXPORT_ALL_SYMBOLS=ON on Windows to mirror that
# behavior when producing a DLL. If the public API is ever explicitly
# annotated with VIBEXX_EXPORT, the two lines below can be re-enabled to
# hide everything else:
#   set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#   set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Put all artifacts in predictable per-build-type directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sub-projects.
add_subdirectory(src)

if(LIBVIBEPP_COMPILE_EXAMPLES)
  add_subdirectory(main)
endif()
