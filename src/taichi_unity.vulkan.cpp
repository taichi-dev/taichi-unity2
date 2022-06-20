#include <cassert>
#include <vector>
#include <map>
#include "taichi/taichi_unity.vulkan.h"
#include "taichi/taichi_vulkan.h"


PluginInstanceVulkan::PluginInstanceVulkan(IUnityGraphicsVulkan* unity_vulkan) :
  unity_vulkan(unity_vulkan) {}
PluginInstanceVulkan::~PluginInstanceVulkan() {
}

TiRuntime PluginInstanceVulkan::import_native_runtime() const {
  UnityVulkanInstance unity_vulkan_instance = unity_vulkan->Instance();

  TiVulkanRuntimeInteropInfo vrii {};
  vrii.api_version = VK_API_VERSION_1_0;
  vrii.instance = unity_vulkan_instance.instance;
  vrii.physical_device = unity_vulkan_instance.physicalDevice;
  vrii.device = unity_vulkan_instance.device;
  vrii.compute_queue = unity_vulkan_instance.graphicsQueue;
  vrii.compute_queue_family_index = unity_vulkan_instance.queueFamilyIndex;
  vrii.graphics_queue = unity_vulkan_instance.graphicsQueue;
  vrii.graphics_queue_family_index = unity_vulkan_instance.queueFamilyIndex;
  return ti_import_vulkan_runtime(&vrii);
}
TiMemory PluginInstanceVulkan::import_native_memory(TiRuntime runtime, void* native_buffer_ptr) const {
  UnityVulkanBuffer unity_vulkan_buffer;
  unity_vulkan->AccessBuffer(native_buffer_ptr, 0, 0, kUnityVulkanResourceAccess_ObserveOnly, &unity_vulkan_buffer);

  TiVulkanMemoryInteropInfo vmii {};
  vmii.buffer = unity_vulkan_buffer.buffer;
  vmii.size = unity_vulkan_buffer.sizeInBytes;
  vmii.usage = unity_vulkan_buffer.usage;
  return ti_import_vulkan_memory(runtime, &vmii);
}
