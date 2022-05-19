#include <cassert>
#include <vector>
#include <map>
#include "Glue/GlueVulkan.h"



IUnityGraphicsVulkan* UNITY_GRAPHICS_VULKAN;



PluginEventHandlerVulkan::PluginEventHandlerVulkan(uint32_t narg) :
  PluginEventHandler(kUnityGfxRendererVulkan, narg) {}

UnityVulkanBuffer PluginEventHandlerVulkan::get_arg_buf(uint32_t binding) const {
  UnityVulkanBuffer out;
  UNITY_GRAPHICS_VULKAN->AccessBuffer(get_arg_native_buf(binding), 0, 0, kUnityVulkanResourceAccess_ObserveOnly, &out);
  return out;
}
UnityVulkanImage PluginEventHandlerVulkan::get_arg_tex(uint32_t binding) const {
  UnityVulkanImage out;
  UNITY_GRAPHICS_VULKAN->AccessTexture(get_arg_native_tex(binding), nullptr, VK_IMAGE_LAYOUT_UNDEFINED, 0, 0, kUnityVulkanResourceAccess_ObserveOnly, &out);
  return out;
}

void InitializeGfxDeviceVulkan() {
  UNITY_GRAPHICS_VULKAN = UNITY_INTERFACES->Get<IUnityGraphicsVulkan>();
  const PluginEventRegistryVulkan& reg = PluginEventRegistryVulkan::get_inst();
  for (const auto& pair : reg.event_handlers) {
    UnityVulkanPluginEventConfig cfg = pair.second->cfg();
    UNITY_GRAPHICS_VULKAN->ConfigureEvent(pair.first, &cfg);
  }
}
void ShutdownGfxDeviceVulkan() {
  UNITY_GRAPHICS_VULKAN = nullptr;
}
