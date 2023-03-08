#ifdef TI_WITH_VULKAN
#include <cassert>
#include <vector>
#include <map>
#include "taichi_unity_impl.vulkan.h"
#include "Unity/IUnityGraphicsVulkan.h"

PluginInstanceVulkan::PluginInstanceVulkan() :
  unity_vulkan(UNITY_INTERFACES->Get<IUnityGraphicsVulkan>())
{
  UnityVulkanPluginEventConfig event_cfg {};
  event_cfg.flags =
    kUnityVulkanEventConfigFlag_EnsurePreviousFrameSubmission |
    kUnityVulkanEventConfigFlag_FlushCommandBuffers |
    kUnityVulkanEventConfigFlag_SyncWorkerThreads |
    kUnityVulkanEventConfigFlag_ModifiesCommandBuffersState;
  event_cfg.graphicsQueueAccess = kUnityVulkanGraphicsQueueAccess_Allow;
  event_cfg.renderPassPrecondition = kUnityVulkanRenderPass_EnsureOutside;
  unity_vulkan->ConfigureEvent(0, &event_cfg);

  UnityVulkanInstance unity_vulkan_instance = unity_vulkan->Instance();
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_ = unity_vulkan_instance.getInstanceProcAddr;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr_ = PFN_vkGetDeviceProcAddr(vkGetInstanceProcAddr_(unity_vulkan_instance.instance, "vkGetDeviceProcAddr"));
  // (penguinliong) Get other API pointers here, if ever needed.
}
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
TiMemory PluginInstanceVulkan::import_native_memory(TiRuntime runtime, TixNativeBufferUnity native_buffer) const {
  UnityVulkanBuffer unity_vulkan_buffer;
  unity_vulkan->AccessBuffer((void*)native_buffer, 0, 0, kUnityVulkanResourceAccess_ObserveOnly, &unity_vulkan_buffer);

  TiVulkanMemoryInteropInfo vmii {};
  vmii.buffer = unity_vulkan_buffer.buffer;
  vmii.size = unity_vulkan_buffer.sizeInBytes;
  vmii.usage = unity_vulkan_buffer.usage;
  return ti_import_vulkan_memory(runtime, &vmii);
}

#endif // TI_WITH_VULKAN
