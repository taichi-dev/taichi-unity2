#include <cassert>
#include "taichi/taichi_unity.h"
#include "taichi/taichi_unity.vulkan.h"

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
  std::vector<void*> pending_native_buffer_imports_;
  std::vector<std::unique_ptr<RenderThreadTask>> pending_tasks;

  // Added by RENDER THREAD; removed by GAME THREAD.
  std::map<void*, TiMemory> imported_native_buffers_;
};
std::unique_ptr<TaichiUnityRuntimeState> RUNTIME_STATE;

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
    ti_launch_kernel(runtime_, kernel_, args_.size(), args_.data());
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
    ti_launch_compute_graph(runtime_, compute_graph_, args_.size(), args_.data());
  }

};

struct RenderThreadCopyMemoryToNativeTask : public RenderThreadTask {
  TiRuntime runtime_;
  TiMemorySlice src_;
  void* native_buffer_ptr_;
  uint64_t native_buffer_offset_;
  uint64_t native_buffer_size_;

  RenderThreadCopyMemoryToNativeTask(
    TiRuntime runtime,
    const TiMemorySlice& memory_slice,
    void* native_buffer_ptr,
    uint64_t native_buffer_offset,
    uint64_t native_buffer_size
  ) :
    runtime_(runtime),
    src_(memory_slice),
    native_buffer_ptr_(native_buffer_ptr),
    native_buffer_offset_(native_buffer_offset),
    native_buffer_size_(native_buffer_size) {}

  virtual void run_in_render_thread() override final {
    TiMemorySlice dst {};
    dst.memory = INSTANCE->import_native_memory(runtime_, native_buffer_ptr_);
    dst.offset = native_buffer_offset_;
    dst.size = native_buffer_size_;
    ti_copy_memory(runtime_, &dst, &src_);
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
    case kUnityGfxRendererVulkan:
    {
      IUnityGraphicsVulkan* unity_graphics_vulkan = UNITY_INTERFACES->Get<IUnityGraphicsVulkan>();
      INSTANCE = std::unique_ptr<PluginInstance>(new PluginInstanceVulkan(unity_graphics_vulkan));
      break;
    }
    //default: assert(false);
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



extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * interfaces) {
  LoadPlugin(interfaces);
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload() {
  UnloadPlugin();
}



void UNITY_INTERFACE_API tix_render_thread_main(int32_t event_id) {
  if (INSTANCE == nullptr || RUNTIME_STATE == nullptr) { return; }
  TiRuntime runtime = RUNTIME_STATE->runtime;

  std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);

  for (void* native_buffer_ptr : RUNTIME_STATE->pending_native_buffer_imports_) {
    TiMemory memory = INSTANCE->import_native_memory(runtime, native_buffer_ptr);
    RUNTIME_STATE->imported_native_buffers_.emplace(std::make_pair(native_buffer_ptr, memory));
  }
  RUNTIME_STATE->pending_native_buffer_imports_.clear();

  for (const auto& pending_task : RUNTIME_STATE->pending_tasks) {
    pending_task->run_in_render_thread();
  }
  RUNTIME_STATE->pending_tasks.clear();

  ti_submit(runtime);
  ti_wait(runtime);
}



extern "C" TiRuntime UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_runtime_unity() {
  assert(INSTANCE != nullptr);
  TiRuntime runtime = INSTANCE->import_native_runtime();
  RUNTIME_STATE = std::make_unique<TaichiUnityRuntimeState>();
  RUNTIME_STATE->runtime = runtime;
  return runtime;
}

extern "C" TiMemory UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_memory_async_unity(TiRuntime runtime, void* native_buffer_ptr) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime);
  std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);
  auto it = RUNTIME_STATE->imported_native_buffers_.find(native_buffer_ptr);
  if (it != RUNTIME_STATE->imported_native_buffers_.end()) {
    TiMemory memory = it->second;
    RUNTIME_STATE->imported_native_buffers_.erase(it);
    return memory;
  } else {
    RUNTIME_STATE->pending_native_buffer_imports_.emplace_back(native_buffer_ptr);
    return TI_NULL_HANDLE;
  }

}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_launch_kernel_async_unity(
  TiRuntime runtime,
  TiKernel kernel,
  uint32_t arg_count,
  const TiArgument* args
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime);
  std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);
  RUNTIME_STATE->pending_tasks.emplace_back(new RenderThreadLaunchKenelTask(runtime, kernel, arg_count, args));
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_launch_compute_graph_async_unity(
  TiRuntime runtime,
  TiComputeGraph compute_graph,
  uint32_t arg_count,
  const TiNamedArgument* args
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime);
  std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);
  RUNTIME_STATE->pending_tasks.emplace_back(new RenderThreadLaunchComputeGraphTask(runtime, compute_graph, arg_count, args));
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_copy_memory_to_native_async_unity(
  TiRuntime runtime,
  const TiMemorySlice* memory,
  void* native_buffer_ptr,
  uint64_t native_buffer_offset,
  uint64_t native_buffer_size
) {
  assert(RUNTIME_STATE != nullptr && runtime == RUNTIME_STATE->runtime && native_buffer_ptr != nullptr);
  std::lock_guard<std::mutex> guard(RUNTIME_STATE->mutex);
  RUNTIME_STATE->pending_tasks.emplace_back(new RenderThreadCopyMemoryToNativeTask(runtime, *memory, native_buffer_ptr, native_buffer_offset, native_buffer_size));
}


extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_submit_in_render_thread_unity(TiRuntime runtime) {
  return tix_render_thread_main;
}
