#pragma once

#include <optional>
#include <vector>

#include "src/engine/device.h"
#include "src/engine/vk.h"
#include "src/pad.h"

namespace el::engine {

class Swapchain {
 public:
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    EL_PAD(4);
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  static auto query_swap_chain_support(VkPhysicalDevice device,
                                       VkSurfaceKHR surface)
      -> std::optional<SwapChainSupportDetails>;

  explicit Swapchain(Device* device);
  Swapchain(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = delete;
  ~Swapchain();

  auto operator=(const Swapchain&) -> Swapchain& = delete;
  auto operator=(Swapchain&&) -> Swapchain& = delete;

 private:
  auto create_swapchain() -> void;
  auto create_image_views() -> void;

  Device* device_ = nullptr;

  VkSwapchainKHR swap_chain_{};
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;
  VkFormat image_format_{};
  VkExtent2D extent_{};
  EL_PAD(4);
};

}  // namespace el::engine
