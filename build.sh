cd build || exit
cmake .. \
-DCMAKE_TOOLCHAIN_FILE=Debug/generators/conan_toolchain.cmake \
-DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
-DCMAKE_BUILD_TYPE=Debug
cmake --build .