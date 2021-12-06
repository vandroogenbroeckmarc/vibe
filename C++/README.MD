# ViBe - C++

This repository contains a C++ implementation of the ViBe background subtraction algorithm. 

## Getting Started

The following instructions will help you install the required libraries

### Dependencies

- Boost
- OpenCV (only if you want to compile the example programs)

### Compilation

The compilation process has been tested on Linux Mint 17.3 and OS X El Capitan.

To compile the program, go into the 'build' directory and enter the following
commands:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
By default, the example programs are compiled. If you want to disable the
compilation of these programs, you must set to 'OFF' the
<code>libvibe++\_COMPILE_EXAMPLES</code> variable in the cache of CMake. As an alternative,
you can use the following commands instead of the previous ones:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -Dlibvibe++_COMPILE_EXAMPLES:BOOL=OFF ..
make
```

Note that on OS X, the program will not compile with clang++ (the default
compiler used by CMake). You can for instance use GCC as following (considering
that you installed, for example, GCC 5 with MacPorts):
```bash
cmake -DCMAKE_CXX_COMPILER=g++-mp-5 -DCMAKE_BUILD_TYPE=Release ..
make
```
or the Intel compiler:
```bash
cmake -DCMAKE_CXX_COMPILER=icpc -DCMAKE_BUILD_TYPE=Release ..
make
```
### Installation

To install the libvibe++ onto your system after the compilation step, you can
enter the following command (in superuser mode):

```bash
make install
```
or

```bash
sudo make install
```

The files will be installed into '/usr/local'.

### Compilation of a program using the libvibe++

To compile a program using the libvibe++, you must use the '-lvibe++' linkage
flag.

### Execution of a program using the libvibe++

If you encounter the following error:

<code>error while loading shared libraries: libvibe++.so: cannot open shared object
file: No such file or directory'</code>

check that <code>/usr/local/lib</code> is in your library path. If it is not, you can add
it temporarily by entering the following command:

```bash
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
```

## Authors

See the [AUTHORS](AUTHORS) file for details.


## License

Custom non-permissive.
This work is protected by several patents.

See the [LICENSE](LICENSE) file for details.
