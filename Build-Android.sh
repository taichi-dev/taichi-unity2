#!/bin/bash

rm -rf build-android-aarch64
mkdir -p build-android-aarch64

pushd ./
cd build-android-aarch64

if [[ ! -v ANDROID_NDK_ROOT ]]; then
  ANDROID_NDK_ROOT="${ANDROID_NDK}"
fi

cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="./install" \
    -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_PLATFORM=android-26 \
    -G "Ninja" \
    -DTAICHI_C_API_INSTALL_DIR="${TAICHI_C_API_INSTALL_DIR}" \
    ..

cmake --build . -t taichi_unity
cmake --build . -t install

popd
