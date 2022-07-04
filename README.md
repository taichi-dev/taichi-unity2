# Taichi Unity Native Plugin

This repository provides extension APIs listed in `taichi/taichi_unity.h` in the [main Taichi repository](https://github.com/taichi-dev/taichi). To build this plugin you can either manually or with Visual Studio.

## Prerequisites

You need to set up environement variable `TAICHI_REPO_DIR` to the root directory of Taichi. For example, if you cloned Taichi with:

```sh
cd /path/to/respositories
git clone https://github.com/taichi-dev/taichi
```

Then you should set `TAICHI_REPO_DIR` to `/path/to/respositories/taichi`.

## Build with Visual Studio

We recommend you to build with Visual Studio for debugging.

1. Open this directory in Visual Studio 2019+ with CMake Integration installed;
2. Wait for auto-configuration to complete;
3. Choose `taichi_unity.dll (bin/taichi_unity.dll)` in the drop-down list to the right of the green play button (▶).
4. Pick up your built library in `out\build\x64-Debug\bin`.

Note that if you also want to debug with symbols of `taichi_c_api.dll`, you need to build Taichi with Visual Studio CMake Integration too for the best experience.

## Build with CMake

Run the following commands to build `taichi_unity` without Visual Studio.

```powershell
mkdir build
cd build
cmake .. -DTAICHI_C_API_INSTALL_DIR="$env:TAICHI_REPO_DIR/build"
cmake --build .
```

Then you will find `taichi_unity.dll` in `build/bin/Debug`.

## Integration to Unity

Copy `taichi_unity.dll` to `Assets/Plugins/x86_64` of your Unity project. Language bindings (function interface and marshalling codes) can be generated with `misc/generate_unity_language_binding.py` in the main Taichi repository.
