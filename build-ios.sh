#!/bin/bash

if [[ -z "tmp/ios.toolchain.cmake" ]]; then
    if [[ -z "tmp" ]]; then
        mkdir tmp
    fi
    wget https://raw.githubusercontent.com/leetal/ios-cmake/master/ios.toolchain.cmake -o tmp/ios.toolchain.cmake
fi

mkdir build-ios-arm64
pushd build-ios-arm64
cmake -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE="tmp/ios.toolchain.cmake" \
    -DPLATFORM=OS64 \
    -DCMAKE_INSTALL_PREFIX="./install" \
    -G "Xcode" \
    -DTAICHI_C_API_INSTALL_DIR="$TAICHI_C_API_INSTALL_DIR" \
    -DTAICHI_UNITY_BUILD_STATIC=ON \
    ..
cmake --build . -t taichi_unity
cmake --build . -t install
popd
