#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#ifdef TI_WITH_VULKAN
#include <vulkan/vulkan.h>
#endif // TI_WITH_VULKAN
#include <taichi/taichi.h>
#include <taichi/taichi_unity.h>

extern IUnityInterfaces* UNITY_INTERFACES;
extern IUnityGraphics* UNITY_GRAPHICS;

struct PluginInstance {
  PluginInstance();
  virtual ~PluginInstance();

  virtual void render_thread_prelude() const {}
  virtual TiRuntime import_native_runtime() const = 0;
  virtual TiMemory import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer_ptr) const = 0;
};


// -----------------------------------------------------------------------------

// Native plugin loader and unloader.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();
