#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "PlatformBase.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"
#include "taichi/taichi_core.h"

extern IUnityInterfaces* UNITY_INTERFACE;
extern IUnityGraphics* UNITY_GRAPHICS;

struct PluginInstance {
  PluginInstance();
  virtual ~PluginInstance();

  virtual TiRuntime import_native_runtime() const = 0;

  virtual TiMemory import_native_memory(TiRuntime runtime, void* native_buffer_ptr) const = 0;
};


// -----------------------------------------------------------------------------

// Native plugin loader and unloader.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

// Import unity runtime. The arch of runtime depends on what Unity provided.
extern "C" TiRuntime UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_runtime_unity();

// Import a managed buffer created by Unity. `native_buffer_ptr` comes from
// `GetNativeBufferPtr`. The native buffer will be imported in the RENDER
// THREAD so your first call from C# scripts in the GAME THREAD might
// receive a `TI_NULL_HANDLE`. You will receive a handle to the imported
// memory after rendering the current frame.
//
// WARNING: Once you receive a valid `TiMemory` handle from this API, you SHOULD
// NOT call this again with the same `native_buffer_ptr`. Otherwise, duplicated
// tracking records will be created and your program MAY suffer from a
// constantly expanding memory consumption.
extern "C" TiMemory UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_memory_async_unity(TiRuntime runtime, void* native_buffer_ptr);

// Same as `ti_launch_kernel` and `ti_launch_compute_graph` but `TiMemory` MUST
// come from `tix_import_native_memory_unity`.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_launch_kernel_async_unity(
  TiRuntime runtime,
  TiKernel kernel,
  uint32_t arg_count,
  const TiArgument* args
);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_launch_compute_graph_async_unity(
  TiRuntime runtime,
  TiComputeGraph compute_graph,
  uint32_t arg_count,
  const TiNamedArgument* args
);

// Unity graphics event invocation can run our code in the rendering thread
// Note that submitting commands simultaneously to a same queue is not allowed
// by any graphics API.
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_submit_in_render_thread_unity(TiRuntime runtime);
