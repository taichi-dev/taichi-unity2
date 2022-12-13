if (-not(Test-Path "build-windows")) {
    New-Item "build-windows" -ItemType Directory
}

Push-Location "build-windows"
cmake -DCMAKE_BUILD_TYPE=Release `
    -DTAICHI_C_API_INSTALL_DIR="$env:TAICHI_C_API_INSTALL_DIR" `
    ..
cmake --build . -t taichi_unity
Pop-Location
