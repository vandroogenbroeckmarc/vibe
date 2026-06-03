# Copyright - Benjamin Laugraud - 2016
# Copyright - Marc Van Droogenbroeck - 2016
#
# Modernization (2026):
#   - Explicit source list (no GLOB).
#   - Single target using BUILD_SHARED_LIBS.
#   - target_include_directories with BUILD/INSTALL interface split.
#   - target_compile_features(cxx_std_17) replaces manual -std= flags.
#   - generate_export_header for portable DLL-export on Windows.
#   - install(EXPORT) + vibe++Config.cmake.in for a proper CMake package.

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

set(vibe++_sources
  libvibe++/common/ViBeBase.cpp
)

set(vibe++_headers
  libvibe++/ViBe.h
  libvibe++/ViBe.t
  libvibe++/common/ViBeBase.h
  libvibe++/common/ViBeTemplateBase.h
  libvibe++/common/ViBeTemplateBase.t
  libvibe++/distances/Manhattan.h
  libvibe++/math/AbsoluteValue.h
  libvibe++/metaprograms/CopyPixel.h
  libvibe++/metaprograms/DistanceL1.h
  libvibe++/metaprograms/SwapPixels.h
  libvibe++/system/inline.h
  libvibe++/system/types.h
)

add_library(vibe++ ${vibe++_sources} ${vibe++_headers})
add_library(vibe++::vibe++ ALIAS vibe++)

# The export header is generated into the build tree and installed beside
# the other headers.
generate_export_header(vibe++
  BASE_NAME VIBEXX
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/libvibe++/system/export.h
)

target_include_directories(vibe++
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(vibe++ PUBLIC cxx_std_17)

# Portable warning configuration.
target_compile_options(vibe++ PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
)

set_target_properties(vibe++ PROPERTIES
  VERSION   ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  OUTPUT_NAME vibe++
  EXPORT_NAME vibe++
  # The public classes (ViBeBase, ...) are not annotated with VIBEXX_EXPORT,
  # so we rely on the platform defaults plus this flag on Windows: CMake
  # then emits a .def file that exports every non-inline symbol from the
  # library, matching the default ELF/Mach-O behavior.
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# -- Installation --------------------------------------------------------------

install(
  TARGETS vibe++
  EXPORT  vibe++Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY libvibe++
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.t"
)

# Install the generated export header.
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libvibe++/system/export.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libvibe++/system
)

# CMake package config.
set(vibe++_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/vibe++)

install(
  EXPORT vibe++Targets
  FILE vibe++Targets.cmake
  NAMESPACE vibe++::
  DESTINATION ${vibe++_CMAKE_CONFIG_DIR}
)

configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/cmake/vibe++Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/vibe++Config.cmake
  INSTALL_DESTINATION ${vibe++_CMAKE_CONFIG_DIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/vibe++ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vibe++Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vibe++ConfigVersion.cmake
  DESTINATION ${vibe++_CMAKE_CONFIG_DIR}
)
