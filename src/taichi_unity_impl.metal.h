#pragma once
#ifdef TI_WITH_METAL
#include <map>
#include <memory>
#include <queue>
#include "taichi_unity_impl.h"

struct PluginInstanceMetal : public PluginInstance {
  struct IUnityGraphicsMetal* unity_metal;

  PluginInstanceMetal();
  virtual ~PluginInstanceMetal() override final;

  virtual void render_thread_prelude() const override final;
  virtual TiRuntime import_native_runtime() const override final;
  virtual TiMemory import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer) const override final;
};

#endif // TI_WITH_METAL
