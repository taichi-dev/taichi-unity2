#pragma once
#include <map>
#include <memory>
#include "Glue/Glue.h"
#define VK_NO_PROTOTYPES
#include "Unity/IUnityGraphicsVulkan.h"

static_assert(SUPPORT_VULKAN);



extern IUnityGraphicsVulkan* UNITY_GRAPHICS_VULKAN;
extern void InitializeGfxDeviceVulkan();
extern void ShutdownGfxDeviceVulkan();



struct PluginEventHandlerVulkan : public PluginEventHandler {
  PluginEventHandlerVulkan(uint32_t narg);

  // Configure event.
  virtual UnityVulkanPluginEventConfig cfg() = 0;

  UnityVulkanBuffer get_arg_buf(uint32_t binding) const;
  UnityVulkanImage get_arg_tex(uint32_t binding) const;
};

struct PluginEventRegistryVulkan {
  std::map<int32_t, std::unique_ptr<PluginEventHandlerVulkan>> event_handlers;

  static PluginEventRegistryVulkan& get_inst() {
    static std::unique_ptr<PluginEventRegistryVulkan> INST {};
    if (!INST) {
      INST = std::unique_ptr<PluginEventRegistryVulkan>(new PluginEventRegistryVulkan);
    }
    return *INST;
  }

  static void reg(
    int32_t event_id,
    std::unique_ptr<PluginEventHandlerVulkan>&& event_handler
  ) {
    get_inst()
      .event_handlers
      .emplace(std::make_pair(event_id, std::forward<std::unique_ptr<PluginEventHandlerVulkan>>(event_handler)));
  }

private:
  PluginEventRegistryVulkan() {}
};
