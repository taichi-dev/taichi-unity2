rm -rf build-linux
mkdir -p build-linux

pushd ./
cd build-linux

cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="./install" \
    -DTAICHI_C_API_INSTALL_DIR="${TAICHI_C_API_INSTALL_DIR}" \
    ..

cmake --build . -t taichi_unity

popd
