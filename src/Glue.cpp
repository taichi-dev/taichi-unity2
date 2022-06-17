#include <cassert>
#include "Glue/Glue.h"
#include "Glue/GlueVulkan.h"

IUnityInterfaces* UNITY_INTERFACES;
IUnityGraphics* UNITY_GRAPHICS;
PluginInstance* INSTANCE;

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
      INSTANCE = new PluginInstanceVulkan(unity_graphics_vulkan);
      break;
    }
    //default: assert(false);
    }
    break;
  }
  case kUnityGfxDeviceEventShutdown:
  {
    delete INSTANCE;
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

void UNITY_INTERFACE_API tix_submit_in_render_thread_unity_impl(int32_t event_id) {
  INSTANCE->apply_submit();
}
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_submit_in_render_thread_unity(TiRuntime runtime) {
  INSTANCE->record_submit(runtime);
  return tix_submit_in_render_thread_unity_impl;
}
extern "C" TiRuntime UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_runtime_unity() {
  return INSTANCE->import_native_runtime();
}
extern "C" TiMemory UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API tix_import_native_memory_unity(TiRuntime runtime, void* native_buffer_ptr) {
  return INSTANCE->import_native_memory(runtime, native_buffer_ptr);
}
