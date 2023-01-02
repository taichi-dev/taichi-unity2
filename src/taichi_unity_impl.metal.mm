#ifdef TI_WITH_METAL
#include <cassert>
#include <cstdio>
#include <vector>
#include <map>
#include "taichi_unity_impl.metal.h"
#include "Unity/IUnityGraphicsMetal.h"
#import <Metal/Metal.h>

PluginInstanceMetal::PluginInstanceMetal() :
  unity_metal(UNITY_INTERFACES->Get<IUnityGraphicsMetal>()) {
}
PluginInstanceMetal::~PluginInstanceMetal() {
}

void PluginInstanceMetal::render_thread_prelude() const {
  unity_metal->EndCurrentCommandEncoder();
}
TiRuntime PluginInstanceMetal::import_native_runtime() const {
  NSBundle *bundle = unity_metal->MetalBundle();
  id<MTLDevice> mtl_device = unity_metal->MetalDevice();

  TiMetalRuntimeInteropInfoExt mrii {};
  mrii.bundle = (__bridge TiNsBundleExt)bundle;
  mrii.device = (__bridge TiMtlDeviceExt)mtl_device;
  return ti_import_metal_runtime_ext(&mrii);
}
TiMemory PluginInstanceMetal::import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer) const {
  id<MTLBuffer> mtl_buffer = (__bridge id<MTLBuffer>)native_buffer;

  TiMetalMemoryInteropInfoExt vmii {};
  vmii.buffer = (__bridge TiMtlBufferExt)mtl_buffer;
  vmii.size = mtl_buffer.length;
  return ti_import_metal_memory_ext(runtime, &vmii);
}

#endif // TI_WITH_METAL
