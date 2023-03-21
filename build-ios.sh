#!/bin/bash

if [[ -z "tmp/ios.toolchain.cmake" ]]; then
    if [[ -z "tmp" ]]; then
        mkdir tmp
    fi
    wget https://raw.githubusercontent.com/leetal/ios-cmake/master/ios.toolchain.cmake -o tmp/ios.toolchain.cmake
fi

echo Using Taichi Runtime at $TAICHI_C_API_INSTALL_DIR

rm -rf build-ios-arm64
mkdir build-ios-arm64
pushd build-ios-arm64
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="tmp/ios.toolchain.cmake" \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED="NO" \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED="NO" \
    -DENABLE_BITCODE=ON \
    -DENABLE_ARC=OFF \
    -DDEPLOYMENT_TARGET=13.0 \
    -DPLATFORM=OS64 \
    -DCMAKE_INSTALL_PREFIX="./install" \
    -G "Xcode" \
    -DTAICHI_C_API_INSTALL_DIR="$TAICHI_C_API_INSTALL_DIR" \
    -DTAICHI_UNITY_BUILD_STATIC=ON \
    ..
cmake --build . -t taichi_unity
cmake --build . -t install
popd
