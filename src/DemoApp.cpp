#include <fstream>
#include "Glue/GlueVulkan.h"

struct DemoEventHandlerVulkan : public PluginEventHandlerVulkan {
  float counter;

  DemoEventHandlerVulkan() : PluginEventHandlerVulkan(1), counter(0.0f) {}

  virtual UnityVulkanPluginEventConfig cfg() override final {
    UnityVulkanPluginEventConfig out;
    out.flags = kUnityVulkanEventConfigFlag_EnsurePreviousFrameSubmission;
    out.graphicsQueueAccess = kUnityVulkanGraphicsQueueAccess_DontCare;
    out.renderPassPrecondition = kUnityVulkanRenderPass_DontCare;
    return out;
  }
  virtual void handle() override final {
    const char* value = get_arg_str(0);
    counter += std::atof(value);
    LAST_ERROR = counter;
  }
};
int L_DUMMY_DemoEventHandlerVulkan =
  (PluginEventRegistryVulkan::reg(1, std::make_unique<DemoEventHandlerVulkan>()), 1);
