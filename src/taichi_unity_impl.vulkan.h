#pragma once
#include <map>
#include <memory>
#include <queue>
#include "taichi_unity_impl.h"
#include "Unity/IUnityGraphicsVulkan.h"

struct PluginInstanceVulkan : public PluginInstance {
  IUnityGraphicsVulkan* unity_vulkan;
  PFN_vkCmdResetEvent vkCmdResetEvent;
  PFN_vkCmdWaitEvents vkCmdWaitEvents;
  std::queue<TiRuntime> submit_args;

  PluginInstanceVulkan(IUnityGraphicsVulkan* unity_vulkan);
  virtual ~PluginInstanceVulkan() override final;

  virtual TiRuntime import_native_runtime() const override final;
  virtual TiMemory import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer) const override final;
  virtual void wait_and_reset_event(TiRuntime runtime, TiEvent event) const override final;
};
