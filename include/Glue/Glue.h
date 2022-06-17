#pragma once
#include <cstdint>
#include <vector>
#include "PlatformBase.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#include "taichi/taichi_core.h"

extern IUnityInterfaces* UNITY_INTERFACE;
extern IUnityGraphics* UNITY_GRAPHICS;

struct PluginInstance {
  PluginInstance();
  virtual ~PluginInstance();

  virtual void record_submit(TiRuntime runtime) = 0;
  virtual void apply_submit() = 0;
  virtual TiRuntime import_native_runtime() const = 0;
  virtual TiMemory import_native_memory(TiRuntime runtime, void* native_buffer_ptr) const = 0;
};
extern PluginInstance* INSTANCE;

// -----------------------------------------------------------------------------

// Native plugin loader and unloader.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

// Unity graphics event invocation can run our code in the rendering thread
// Note that submitting commands simultaneously to a same queue is not allowed
// by any graphics API.
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_submit_in_render_thread_unity(TiRuntime runtime);
extern "C" TiRuntime UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_runtime_unity();
extern "C" TiMemory UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_memory_unity(TiRuntime runtime, void* native_buffer_ptr);
