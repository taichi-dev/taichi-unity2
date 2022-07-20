#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#include <taichi/taichi_core.h>
#include <taichi/taichi_unity.h>

extern IUnityInterfaces* UNITY_INTERFACE;
extern IUnityGraphics* UNITY_GRAPHICS;

struct PluginInstance {
  PluginInstance();
  virtual ~PluginInstance();

  virtual TiRuntime import_native_runtime() const = 0;
  virtual TiMemory import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer_ptr) const = 0;
  virtual void wait_and_reset_event(TiRuntime runtime, TiEvent event) const = 0;
};


// -----------------------------------------------------------------------------

// Native plugin loader and unloader.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();
