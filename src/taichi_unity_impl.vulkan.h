#pragma once
#ifdef TI_WITH_VULKAN
#include <map>
#include <memory>
#include <queue>
#include "taichi_unity_impl.h"

struct PluginInstanceVulkan : public PluginInstance {
  struct IUnityGraphicsVulkan* unity_vulkan;

  PluginInstanceVulkan();
  virtual ~PluginInstanceVulkan() override final;

  virtual TiRuntime import_native_runtime() const override final;
  virtual TiMemory import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer) const override final;
};

#endif // TI_WITH_VULKAN
