# ViBe — C

This directory contains a modernized C implementation of **ViBe**, a real-time
universal background subtraction algorithm for video sequences
(Barnich & Van Droogenbroeck, *IEEE TIP*, 2011).

<p align="center">
  <img src="../img/C_logo.png" height="150">
</p>

## Library components

- `vibe-background-sequential.h` — public interface
- `vibe-background-sequential.c` — implementation (C11)
- `your-main-file-sequential.c`  — template showing how to plug ViBe into
  your own code base (no OpenCV required)
- `main-opencv.cpp`              — OpenCV 4 demo reading a video file

## Dependencies

- A C11 compiler (GCC ≥ 4.9, Clang ≥ 3.1, Apple Clang, MSVC 2019+).
- A C++17 compiler for `main-opencv.cpp` only.
- OpenCV ≥ 4.5 — **required only** for `main-opencv.cpp`.

## Build — Strategy A: CMake + vcpkg (recommended, cross-platform)

```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh             # .\vcpkg\bootstrap-vcpkg.bat on Windows

cmake -S . -B build \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix install
```

## Build — Strategy B: Makefile (Linux/macOS, system OpenCV)

```bash
# Ubuntu / Debian
sudo apt install libopencv-dev pkg-config build-essential

# macOS
brew install opencv pkg-config

make            # build libvibe.a + main + main-opencv
make main       # C example only (no OpenCV needed)
make clean
sudo make install PREFIX=/usr/local
```

The Makefile auto-detects whether your system ships OpenCV under the
`opencv4` or the legacy `opencv` pkg-config module.

## Build — Strategy C: bare CMake (Linux/macOS, system OpenCV)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Disable the OpenCV example without removing OpenCV from the system:

```bash
cmake -S . -B build -DLIBVIBE_C_BUILD_EXAMPLE_OPENCV=OFF
```

## Running the examples

```bash
./build/main                              # template main (runs 100 no-op frames)
./build/main-opencv input/driveway-320x240.avi # OpenCV version
```

Press **ESC** or **Q** to close the OpenCV preview windows.

## Using libvibe from another CMake project

Once the library has been installed with `cmake --install`, any downstream
CMake project can consume it with:

```cmake
find_package(vibe CONFIG REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE vibe::vibe)
```

If `vibe` was installed to a non-standard prefix:

```bash
cmake -S . -B build -Dvibe_DIR=<prefix>/lib/cmake/vibe
```

## Authors

See the [AUTHORS](AUTHORS) file.

## License

The original ViBe patent (WO2009007198 / EP2015252 / US 8009918 B2 /
JP 4699564 B2) is now in the public domain. 
