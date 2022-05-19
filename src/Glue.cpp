#include <cassert>
#include "Glue/Glue.h"
#include "Glue/GlueVulkan.h"



IUnityInterfaces* UNITY_INTERFACES;
IUnityGraphics* UNITY_GRAPHICS;
UnityGfxRenderer CURRENT_RENDERER = kUnityGfxRendererNull;



void UNITY_INTERFACE_API OnGfxDeviceEvent(UnityGfxDeviceEventType eventType) {
  switch (eventType) {
  case kUnityGfxDeviceEventInitialize:
  {
    extern void InitializeGfxDevice();
    InitializeGfxDevice();
    break;
  }
  case kUnityGfxDeviceEventShutdown:
  {
    extern void ShutdownGfxDevice();
    ShutdownGfxDevice();
    break;
  }
  }
}

void LoadPlugin(IUnityInterfaces* interfaces) {
  UNITY_INTERFACES = interfaces;
  UNITY_GRAPHICS = UNITY_INTERFACES->Get<IUnityGraphics>();
  UNITY_GRAPHICS->RegisterDeviceEventCallback(OnGfxDeviceEvent);

  OnGfxDeviceEvent(kUnityGfxDeviceEventInitialize);
}
void UnloadPlugin() {
  UNITY_GRAPHICS->UnregisterDeviceEventCallback(OnGfxDeviceEvent);
  UNITY_GRAPHICS = nullptr;
  UNITY_INTERFACES = nullptr;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces * interfaces) {
  LoadPlugin(interfaces);
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload() {
  UnloadPlugin();
}



void InitializeGfxDevice() {
  CURRENT_RENDERER = UNITY_GRAPHICS->GetRenderer();
  switch (CURRENT_RENDERER) {
  case kUnityGfxRendererVulkan: InitializeGfxDeviceVulkan(); break;
  //default: assert(false);
  }
}
void UNITY_INTERFACE_API HandleGfxEvent(int32_t event_id) {
  switch (CURRENT_RENDERER) {
  case kUnityGfxRendererVulkan: PluginEventRegistryVulkan::get_inst().event_handlers.at(event_id)->handle(); break;
  //default: assert(false);
  }
}
void ShutdownGfxDevice() {
  switch (CURRENT_RENDERER) {
  case kUnityGfxRendererVulkan: ShutdownGfxDeviceVulkan(); break;
  //default: assert(false);
  }
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungGetPluginGfxEventHandlers() {
  return HandleGfxEvent;
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgI32(int32_t event_id, uint32_t binding, int32_t value) {
  PluginEventArg arg {};
  arg.ty = L_EVENT_ARG_TYPE_I32;
  arg.arg_i32 = value;
  PluginEventRegistryVulkan::get_inst().event_handlers.at(event_id)->bind_arg(binding, std::move(arg));
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgF32(int32_t event_id, uint32_t binding, float value) {
  PluginEventArg arg {};
  arg.ty = L_EVENT_ARG_TYPE_F32;
  arg.arg_f32 = value;
  PluginEventRegistryVulkan::get_inst().event_handlers.at(event_id)->bind_arg(binding, std::move(arg));
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgBufferPtr(int32_t event_id, uint32_t binding, void* native_buf) {
  PluginEventArg arg {};
  arg.ty = L_EVENT_ARG_TYPE_NATIVE_BUFFER;
  arg.arg_native_buf = native_buf;
  PluginEventRegistryVulkan::get_inst().event_handlers.at(event_id)->bind_arg(binding, std::move(arg));
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgTexturePtr(int32_t event_id, uint32_t binding, void* native_tex) {
  PluginEventArg arg {};
  arg.ty = L_EVENT_ARG_TYPE_NATIVE_TEXTURE;
  arg.arg_native_tex = native_tex;
  PluginEventRegistryVulkan::get_inst().event_handlers.at(event_id)->bind_arg(binding, std::move(arg));
}

PluginEventHandler::PluginEventHandler(UnityGfxRenderer renderer, uint32_t narg) : renderer(renderer) {
  args.resize(narg);
}
void PluginEventHandler::bind_arg(uint32_t binding, PluginEventArg&& arg) {
  args.at(binding) = std::forward<PluginEventArg>(arg);
}
int32_t PluginEventHandler::get_arg_i32(uint32_t binding) const {
  const PluginEventArg& arg = args.at(binding);
  assert(arg.ty == L_EVENT_ARG_TYPE_I32);
  return arg.arg_i32;
}
float PluginEventHandler::get_arg_f32(uint32_t binding) const {
  const PluginEventArg& arg = args.at(binding);
  assert(arg.ty == L_EVENT_ARG_TYPE_F32);
  return arg.arg_f32;
}
void* PluginEventHandler::get_arg_native_buf(uint32_t binding) const {
  const PluginEventArg& arg = args.at(binding);
  assert(arg.ty == L_EVENT_ARG_TYPE_NATIVE_BUFFER);
  return arg.arg_native_buf;
}
void* PluginEventHandler::get_arg_native_tex(uint32_t binding) const {
  const PluginEventArg& arg = args.at(binding);
  assert(arg.ty == L_EVENT_ARG_TYPE_NATIVE_TEXTURE);
  return arg.arg_native_tex;
}



int32_t LAST_ERROR = 0;
extern "C" int32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungGetLastError() {
  return LAST_ERROR;
}
