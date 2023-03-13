#include <cassert>
#include <cstring>

#include "taichi_unity_impl.h"
#include "taichi_unity_impl.vulkan.h"
#include "taichi_unity_impl.metal.h"

IUnityInterfaces* UNITY_INTERFACES;
IUnityGraphics* UNITY_GRAPHICS;
std::unique_ptr<PluginInstance> INSTANCE;





struct RenderThreadTask {
  RenderThreadTask() {}
  virtual ~RenderThreadTask() {}

  virtual void run_in_render_thread() = 0;
};

struct TaichiUnityRuntimeState {
  std::mutex mutex;

  TiRuntime runtime;

  // Added by GAME THREAD; removed by RENDER THREAD.
  std::vector<TixNativeBufferUnity> pending_native_buffer_imports_;
  std::vector<std::unique_ptr<RenderThreadTask>> pending_tasks_;

  // Added by RENDER THREAD; removed by GAME THREAD.
  std::map<TixNativeBufferUnity, TiMemory> imported_native_buffers_;

  void enqueue_task(RenderThreadTask* task) {
    std::lock_guard<std::mutex> guard(mutex);
    pending_tasks_.emplace_back(std::unique_ptr<RenderThreadTask>(task));
  }
};
std::unique_ptr<TaichiUnityRuntimeState> RUNTIME_STATE;

struct RenderThreadUserTask : public RenderThreadTask {
  void* user_data_;
  TixAsyncTaskUnity async_task_;

  RenderThreadUserTask(void* user_data, TixAsyncTaskUnity async_task) :
      user_data_(user_data), async_task_(async_task) {}

  virtual void run_in_render_thread() override final {
    async_task_(user_data_);
  }
};

struct RenderThreadLaunchKenelTask : public RenderThreadTask {
  TiRuntime runtime_;
  TiKernel kernel_;
  std::vector<TiArgument> args_;

  RenderThreadLaunchKenelTask(
    TiRuntime runtime,
    TiKernel kernel,
    uint32_t arg_count,
    const TiArgument* args
  ) : runtime_(runtime), kernel_(kernel), args_() {
    args_.resize(arg_count);
    std::memcpy(args_.data(), args, arg_count * sizeof(TiArgument));
  }

  virtual void run_in_render_thread() override final {
    ti_launch_kernel(runtime_, kernel_, (uint32_t)args_.size(), args_.data());
  }
};

struct RenderThreadLaunchComputeGraphTask : public RenderThreadTask {
  TiRuntime runtime_;
  TiComputeGraph compute_graph_;
  std::vector<TiNamedArgument> args_;
  std::vector<std::string> arg_names_;

  RenderThreadLaunchComputeGraphTask(
    TiRuntime runtime,
    TiComputeGraph compute_graph,
    uint32_t arg_count,
    const TiNamedArgument* args
  ) : runtime_(runtime), compute_graph_(compute_graph), args_() {
    arg_names_.reserve(arg_count);
    args_.resize(arg_count);
    std::memcpy(args_.data(), args, arg_count * sizeof(TiNamedArgument));

    for (auto& arg : args_) {
      arg_names_.emplace_back(arg.name);
      arg.name = arg_names_.back().c_str();
    }
  }

  virtual void run_in_render_thread() override final {
    ti_launch_compute_graph(runtime_, compute_graph_, (uint32_t)args_.size(), args_.data());
  }

};

struct RenderThreadCopyMemoryToNativeTask : public RenderThreadTask {
  TiRuntime runtime_;
  TiMemorySlice src_;
  TixNativeBufferUnity dst_;
  uint64_t dst_offset_;

  RenderThreadCopyMemoryToNativeTask(
    TiRuntime runtime,
    const TiMemorySlice& src,
    TixNativeBufferUnity dst,
    uint64_t dst_offset
  ) :
    runtime_(runtime),
    src_(src),
    dst_(dst),
    dst_offset_(dst_offset) {}

  virtual void run_in_render_thread() override final {
    TiMemorySlice dst {};
    dst.memory = INSTANCE->import_native_memory(runtime_, dst_);
    dst.offset = dst_offset_;
    dst.size = src_.size;
    ti_copy_memory_device_to_device(runtime_, &dst, &src_);
    ti_free_memory(runtime_, dst.memory);
  }

};



PluginInstance::PluginInstance() {}
PluginInstance::~PluginInstance() {}

void UNITY_INTERFACE_API OnGfxDeviceEvent(UnityGfxDeviceEventType eventType) {
  switch (eventType) {
  case kUnityGfxDeviceEventInitialize:
  {
    UnityGfxRenderer renderer = UNITY_GRAPHICS->GetRenderer();
    switch (renderer) {
#ifdef TI_WITH_VULKAN
    case kUnityGfxRendererVulkan:
    {
      INSTANCE = std::unique_ptr<PluginInstance>(new PluginInstanceVulkan());
      break;
    }
#endif // TI_WITH_VULKAN
#ifdef TI_WITH_METAL
    case kUnityGfxRendererMetal:
    {
      INSTANCE = std::unique_ptr<PluginInstance>(new PluginInstanceMetal());
      break;
    }
#endif // TI_WITH_METAL
    default: assert(false);
    }
    break;
  }
  case kUnityGfxDeviceEventShutdown:
  {
    INSTANCE = nullptr;
    break;
  }
  }
}

void LoadPlugin(IUnityInterfaces* interfaces) {
  UNITY_INTERFACES = interfaces;
  UNITY_GRAPHICS = interfaces->Get<IUnityGraphics>();

  UNITY_GRAPHICS->RegisterDeviceEventCallback(OnGfxDeviceEvent);
  OnGfxDeviceEvent(kUnityGfxDeviceEventInitialize);
}
void UnloadPlugin() {
  UNITY_GRAPHICS->UnregisterDeviceEventCallback(OnGfxDeviceEvent);
  UNITY_GRAPHICS = nullptr;
  UNITY_INTERFACES = nullptr;
}



// -----------------------------------------------------------------------------

extern "C" {


void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * interfaces) {
  LoadPlugin(interfaces);
}
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload() {
  UnloadPlugin();
}



void UNITY_INTERFACE_API tix_render_thread_main(int32_t event_id) {
  if (INSTANCE == nullptr || RUNTIME_STATE == nullptr) { return; }

  INSTANCE->render_thread_prelude();

  TiRuntime runtime = RUNTIME_STATE->runtime;
  std::vector<TixNativeBufferUnity> pending_native_buffer_imports;
  std::vector<std::unique_ptr<RenderThreadTask>> pending_tasks;

  {
    std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);
    pending_native_buffer_imports = std::move(RUNTIME_STATE->pending_native_buffer_imports_);
    pending_tasks = std::move(RUNTIME_STATE->pending_tasks_);
  }

  for (TixNativeBufferUnity native_buffer : pending_native_buffer_imports) {
    TiMemory memory = INSTANCE->import_native_memory(runtime, native_buffer);
    RUNTIME_STATE->imported_native_buffers_.emplace(std::make_pair(native_buffer, memory));
  }

  for (const auto& pending_task : pending_tasks) {
    pending_task->run_in_render_thread();
  }

  // Force synchronize to ensure everything is done when we give control back to
  // Unity.
  ti_wait(runtime);
}



// Import unity runtime. The arch of runtime depends on what Unity provided.
TI_DLL_EXPORT TiRuntime TI_API_CALL tix_import_native_runtime_unity() {
  assert(INSTANCE != nullptr);
  TiRuntime runtime = INSTANCE->import_native_runtime();
  RUNTIME_STATE = std::make_unique<TaichiUnityRuntimeState>();
  RUNTIME_STATE->runtime = runtime;
  return runtime;
}

TI_DLL_EXPORT void TI_API_CALL
tix_enqueue_task_async_unity(void *user_data, TixAsyncTaskUnity async_task) {
  RUNTIME_STATE->enqueue_task(new RenderThreadUserTask(user_data, async_task));
}

// Same as `ti_launch_kernel` and `ti_launch_compute_graph` but `TiMemory` MUST
// come from `tix_import_native_memory_unity`.
TI_DLL_EXPORT void TI_API_CALL tix_launch_kernel_async_unity(
  TiRuntime runtime,
  TiKernel kernel,
  uint32_t arg_count,
  const TiArgument* args
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime);
  RUNTIME_STATE->enqueue_task(new RenderThreadLaunchKenelTask(runtime, kernel, arg_count, args));
}
TI_DLL_EXPORT void TI_API_CALL tix_launch_compute_graph_async_unity(
  TiRuntime runtime,
  TiComputeGraph compute_graph,
  uint32_t arg_count,
  const TiNamedArgument* args
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime);
  RUNTIME_STATE->enqueue_task(new RenderThreadLaunchComputeGraphTask(runtime, compute_graph, arg_count, args));
}

TI_DLL_EXPORT void TI_API_CALL tix_copy_memory_to_native_buffer_async_unity(
  TiRuntime runtime,
  TixNativeBufferUnity dst,
  uint64_t dst_offset,
  const TiMemorySlice* src
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime && dst != nullptr && src != nullptr);
  RUNTIME_STATE->enqueue_task(new RenderThreadCopyMemoryToNativeTask(runtime, *src, dst, dst_offset));
}

TI_DLL_EXPORT void TI_API_CALL tix_copy_memory_device_to_host_unity(
  TiRuntime runtime,
  void* dst,
  uint64_t dst_offset,
  const TiMemorySlice* src
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime && dst != nullptr && src != nullptr);
  void* mapped = ti_map_memory(runtime, src->memory);
  std::memcpy((uint8_t*)dst + dst_offset, mapped, src->size);
  ti_unmap_memory(runtime, src->memory);
}

TI_DLL_EXPORT void TI_API_CALL tix_copy_memory_host_to_device_unity(
  TiRuntime runtime,
  const TiMemorySlice* dst,
  const void* src,
  uint64_t src_offset
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime && dst != nullptr && src != nullptr);
  void* mapped = ti_map_memory(runtime, dst->memory);
  std::memcpy(mapped, (const uint8_t*)src + src_offset, dst->size);
  ti_unmap_memory(runtime, dst->memory);
}


// Unity graphics event invocation can run our code in the rendering thread
// Note that submitting commands simultaneously to a same queue is not allowed
// by any graphics API.
 TI_DLL_EXPORT void* TI_API_CALL tix_submit_async_unity(TiRuntime runtime) {
  return (void*)(&tix_render_thread_main);
}

} // extern "C"
