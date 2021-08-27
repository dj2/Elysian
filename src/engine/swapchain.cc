#include "src/engine/swapchain.h"

#include <algorithm>

namespace el::engine {
namespace {

auto choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available) -> VkSurfaceFormatKHR {
  for (const auto& format : available) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return available[0];
}

auto choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available)
    -> VkPresentModeKHR {
  for (const auto& mode : available) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

auto choose_swap_extent(const VkSurfaceCapabilitiesKHR& caps,
                        uint32_t width,
                        uint32_t height) -> VkExtent2D {
  if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return caps.currentExtent;
  }

  return {std::max(caps.minImageExtent.width,
                   std::min(caps.maxImageExtent.width, width)),
          std::max(caps.minImageExtent.height,
                   std::min(caps.maxImageExtent.height, height))};
}

}  // namespace

// static
auto Swapchain::query_swap_chain_support(VkPhysicalDevice device,
                                         VkSurfaceKHR surface)
    -> std::optional<SwapChainSupportDetails> {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_mode_count, details.present_modes.data());
  }
  if (!details.formats.empty() && !details.present_modes.empty()) {
    return {details};
  }
  return {};
}

Swapchain::Swapchain(Device* device) : device_(device) {
  create_swapchain();
  create_image_views();
}

Swapchain::~Swapchain() {
  vkDeviceWaitIdle(device_->device());

  std::for_each(std::begin(image_views_), std::end(image_views_),
                [device = device_->device()](VkImageView view) {
                  vkDestroyImageView(device, view, nullptr);
                });

  vkDestroySwapchainKHR(device_->device(), swap_chain_, nullptr);
}

auto Swapchain::create_swapchain() -> void {
  auto support = Swapchain::query_swap_chain_support(device_->physical_device(),
                                                     device_->surface());
  if (!support.has_value()) {
    throw std::runtime_error(
        "Unable to retrieve swap chain support information");
  }

  auto dimensions = device_->dimensions();
  auto fmt = choose_swap_surface_format(support->formats);
  auto mode = choose_swap_present_mode(support->present_modes);
  auto extent = choose_swap_extent(support->capabilities, dimensions.width,
                                   dimensions.height);
  auto img_count = support->capabilities.minImageCount + 1;

  if (support->capabilities.maxImageCount > 0) {
    img_count = std::min(img_count, support->capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = device_->surface(),
      .minImageCount = img_count,
      .imageFormat = fmt.format,
      .imageColorSpace = fmt.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = support->capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = mode,
      .clipped = VK_TRUE,
      .oldSwapchain = nullptr};

  auto indices = device_->find_queue_families();
  std::array<uint32_t, 2> family_indices = {indices.graphics_family.value(),
                                            indices.present_family.value()};
  if (indices.graphics_family != indices.present_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = uint32_t(family_indices.size());
    create_info.pQueueFamilyIndices = family_indices.data();
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  auto res = vkCreateSwapchainKHR(device_->device(), &create_info, nullptr,
                                  &swap_chain_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create swap chain: ").append(to_string(res)));
  }

  vkGetSwapchainImagesKHR(device_->device(), swap_chain_, &img_count, nullptr);
  images_.resize(img_count);
  vkGetSwapchainImagesKHR(device_->device(), swap_chain_, &img_count,
                          images_.data());

  image_format_ = fmt.format;
  extent_ = extent;
}

auto Swapchain::create_image_views() -> void {
  image_views_.resize(images_.size());

  auto view_creator = [this](const VkImage& image) {
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = image_format_,
        .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    VkImageView view{};
    auto res =
        vkCreateImageView(device_->device(), &create_info, nullptr, &view);
    if (res != VK_SUCCESS) {
      throw std::runtime_error(
          std::string("failed to create image view: ").append(to_string(res)));
    }
    return view;
  };

  std::transform(std::begin(images_), std::end(images_),
                 std::begin(image_views_), view_creator);
}

}  // namespace el::engine
