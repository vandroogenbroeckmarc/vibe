# Copyright - Benjamin Laugraud - 2016
# Copyright - Marc Van Droogenbroeck - 2016
#
# Modernization (2026):
#   - Single explicit find_package(OpenCV 4) call.
#   - Explicit add_executable for each example (BenlaugUtils no longer needed).
#   - C++17 selected via target_compile_features instead of raw -std= flags.
#   - Links to the modern vibe++::vibe++ imported target.

include(GNUInstallDirs)

find_package(OpenCV 4.5 REQUIRED
  COMPONENTS core highgui imgproc videoio imgcodecs
)

set(vibe++_examples
  ViBe_8UC1
  ViBe_8UC3
)

foreach(example IN LISTS vibe++_examples)
  add_executable(${example} ${example}.cpp)

  target_compile_features(${example} PRIVATE cxx_std_17)

  target_compile_options(${example} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
  )

  target_link_libraries(${example}
    PRIVATE
      vibe++::vibe++
      opencv_core
      opencv_highgui
      opencv_imgproc
      opencv_videoio
      opencv_imgcodecs
  )

  install(TARGETS ${example}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
