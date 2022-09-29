#include <cassert>
#include <vector>
#include <map>
#define VK_NO_PROTOTYPES 1
#include <vulkan/vulkan.h>
#include <taichi/taichi_vulkan.h>
#include "taichi_unity_impl.vulkan.h"
#include "Unity/IUnityGraphicsVulkan.h"

PluginInstanceVulkan::PluginInstanceVulkan(IUnityGraphicsVulkan* unity_vulkan) :
  unity_vulkan(unity_vulkan)
{
  UnityVulkanInstance unity_vulkan_instance = unity_vulkan->Instance();

  vkCmdResetEvent = PFN_vkCmdResetEvent(vkGetDeviceProcAddr(unity_vulkan_instance.device, "vkCmdResetEvent"));
  vkCmdWaitEvents = PFN_vkCmdWaitEvents(vkGetDeviceProcAddr(unity_vulkan_instance.device, "vkCmdWaitEvents"));
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
void PluginInstanceVulkan::wait_and_reset_event(TiRuntime runtime, TiEvent event) const {
  TiVulkanEventInteropInfo interop_info {};
  ti_export_vulkan_event(runtime, event, &interop_info);

  UnityVulkanRecordingState recording_state {};
  if (!unity_vulkan->CommandRecordingState(&recording_state, kUnityVulkanGraphicsQueueAccess_DontCare)) {
    return;
  }
  if (recording_state.commandBuffer != nullptr) {
    //assert(recording_state.commandBufferLevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkCmdWaitEvents(recording_state.commandBuffer, 1, &interop_info.event,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      0, nullptr, 0, nullptr, 0, nullptr);
    vkCmdResetEvent(recording_state.commandBuffer, interop_info.event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  }
}
