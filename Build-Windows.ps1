if(Test-Path "build-windows") {
    Remove-Item -Recurse -Force "build-windows"
}
if (-not(Test-Path "build-windows")) {
    New-Item "build-windows" -ItemType Directory
}

Push-Location "build-windows"
cmake -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="./install" `
    -DTAICHI_C_API_INSTALL_DIR="$env:TAICHI_C_API_INSTALL_DIR" `
    ..
cmake --build . --config Release -t taichi_unity
cmake --build . --config Release -t install
Pop-Location
