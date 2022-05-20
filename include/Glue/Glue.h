#pragma once
#include <cstdint>
#include <vector>
#include "PlatformBase.h"
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"



extern IUnityInterfaces* UNITY_INTERFACES;
extern IUnityGraphics* UNITY_GRAPHICS;
// There is no setter function for this error code so you simply write to it.
extern int32_t LAST_ERROR;



// Native plugin loader and unloader.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();
// Interface for unity C# script to interact with this native plugin.
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungGetPluginGfxEventHandlers();
// Bind a native rendering resource handle (buffer or texture) before invocation of an event handler.
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgI32(int32_t event_id, uint32_t binding, int32_t value);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgF32(int32_t event_id, uint32_t binding, float value);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgString(int32_t event_id, uint32_t binding, const char* str);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgBufferPtr(int32_t event_id, uint32_t binding, void* native_buf);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungBindGfxEventArgTexturePtr(int32_t event_id, uint32_t binding, void* native_tex);
// Report error code for debugging.
extern "C" int32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API ungGetLastError();



enum PluginEventArgType {
  L_EVENT_ARG_TYPE_I32,
  L_EVENT_ARG_TYPE_F32,
  L_EVENT_ARG_TYPE_STRING,
  L_EVENT_ARG_TYPE_NATIVE_BUFFER,
  L_EVENT_ARG_TYPE_NATIVE_TEXTURE,
};
struct PluginEventArg {
  PluginEventArgType ty;
  union {
    int32_t arg_i32;
    float arg_f32;
    const char* arg_str;
    void* arg_native_buf;
    void* arg_native_tex;
  };
};
struct PluginEventHandler {
  const UnityGfxRenderer renderer;

  PluginEventHandler(UnityGfxRenderer renderer, uint32_t narg);

  // Handle event when Unity renders a frame.
  virtual void handle() = 0;

  // Argument accessors.
  void bind_arg(uint32_t binding, PluginEventArg&& arg);
  int32_t get_arg_i32(uint32_t binding) const;
  float get_arg_f32(uint32_t binding) const;
  const char* get_arg_str(uint32_t binding) const;
  void* get_arg_native_buf(uint32_t binding) const;
  void* get_arg_native_tex(uint32_t binding) const;

private:
  std::vector<PluginEventArg> args;
};
