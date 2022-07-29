if (-not(Test-Path "build-android-aarch64")) {
    New-Item "build-android-aarch64" -ItemType Directory
}

Push-Location "build-android-aarch64"
cmake -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_TOOLCHAIN_FILE="$env:ANDROID_NDK/build/cmake/android.toolchain.cmake" `
    -DANDROID_ABI="arm64-v8a" `
    -DANDROID_PLATFORM=android-26 `
    -G "Ninja" `
    -DTAICHI_C_API_INSTALL_DIR="$env:TAICHI_REPO_DIR/build-android-aarch64/install/c_api" `
    ..
cmake --build . -t taichi_unity
Pop-Location
