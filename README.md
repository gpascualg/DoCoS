# Build

Using CMake:

    mkdir build
    cd build
    cmake ..
    make

Executable will be found at

    build/src/docos


## Tested with

Builds with 11.0 and libc++. Should work with any compile as long as it has support for

* Coroutines TS
* Conceps TS
* C++17 major features (notably *constexpr*, *fold expressions*, *variant*)

Might require tweaking *src/CMakeLists.txt* to add flags when not using the above combination.
