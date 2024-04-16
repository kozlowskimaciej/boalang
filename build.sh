#!/usr/bin/bash

BUILD_TYPE=${1:-Debug}

cd build || exit
cmake .. \
-DCMAKE_TOOLCHAIN_FILE="${BUILD_TYPE}"/generators/conan_toolchain.cmake \
-DCMAKE_CXX_COMPILER=/usr/bin/clang++-14 \
-DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build .
