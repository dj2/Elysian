#include "src/engine.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "src/dimensions.h"
#include "src/engine/vk.h"
#include "src/event_service.h"
#include "src/pad.h"

namespace el::engine {
namespace {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  EL_PAD(4);
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> compute_family;
  std::optional<uint32_t> transfer_family;
  std::optional<uint32_t> present_family;
};

const char* const kEngineName = "Elysian Engine";
constexpr uint8_t kEngineMajor = 0;
constexpr uint8_t kEngineMinor = 1;
constexpr uint8_t kEnginePatch = 0;

constexpr uint32_t kQueueFamilyBits =
    VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

constexpr std::array<const char*, 1> kValidationLayers = {
    {"VK_LAYER_KHRONOS_validation"}};

constexpr std::array<const char*, 1> kDeviceExtensions = {
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME}};

auto debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                    VkDebugUtilsMessageTypeFlagsEXT type,
                    const VkDebugUtilsMessengerCallbackDataEXT* data,
                    void* user_data) -> VkBool32 {
  const auto* err_data = static_cast<ErrorData*>(user_data);

  if (err_data != nullptr && err_data->cb) {
    ErrorSeverity sev = ErrorSeverity::kError;
    if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
      sev = ErrorSeverity::kWarning;
    } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
      sev = ErrorSeverity::kInfo;
    } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) !=
               0) {
      sev = ErrorSeverity::kVerbose;
    }

    ErrorType error_type = ErrorType::kGeneral;
    if ((type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
      error_type = ErrorType::kPerformance;
    } else if ((type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
      error_type = ErrorType::kValidation;
    }

    std::stringstream msg_buf;
    msg_buf << "Err: " << data->pMessage << std::endl;
    if (data->pMessageIdName != nullptr) {
      msg_buf << "MessageId (" << data->messageIdNumber
              << "): " << data->pMessageIdName << std::endl;
    }
    if (data->queueLabelCount > 0) {
      msg_buf << "Queues:" << std::endl;

      std::span queues(data->pQueueLabels, data->queueLabelCount);
      std::for_each(std::begin(queues), std::end(queues),
                    [&msg_buf](const auto& label) {
                      msg_buf << "  " << label.pLabelName << std::endl;
                    });
    }
    if (data->cmdBufLabelCount > 0) {
      msg_buf << "Command Buffers:" << std::endl;

      std::span cmdBufLabels(data->pCmdBufLabels, data->cmdBufLabelCount);
      std::for_each(std::begin(cmdBufLabels), std::end(cmdBufLabels),
                    [&msg_buf](const auto& buf) {
                      msg_buf << "  " << buf.pLabelName << std::endl;
                    });
    }
    if (data->objectCount > 0) {
      msg_buf << "Objects:" << std::endl;

      std::span objects(data->pObjects, data->objectCount);
      std::for_each(std::begin(objects), std::end(objects),
                    [&msg_buf](const auto& obj) {
                      msg_buf << "  " << to_string(obj.objectType);
                      msg_buf << "(0x" << std::hex << obj.objectHandle << ")";
                      if (obj.pObjectName) {
                        msg_buf << " " << obj.pObjectName;
                      }
                      msg_buf << std::endl;
                    });
    }

    err_data->cb({
        .severity = sev,
        .type = error_type,
        .message = msg_buf.str(),
        .user_data = err_data->user_data,
    });
  }
  return VK_FALSE;
}

auto build_app_info(const DeviceConfig& config) -> VkApplicationInfo {
  return {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = config.app_name().data(),
      .applicationVersion = config.version().to_vk(),
      .pEngineName = kEngineName,
      .engineVersion =
          VersionInfo{0, kEngineMajor, kEngineMinor, kEnginePatch}.to_vk(),
      .apiVersion = VersionInfo{0, 1, 2, 0}.to_vk(),
  };
}

auto build_instance_create_info(VkApplicationInfo* app_info,
                                const std::vector<const char*>& exts)
    -> VkInstanceCreateInfo {
  return {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = app_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = uint32_t(exts.size()),
      .ppEnabledExtensionNames = exts.data(),
  };
}

auto find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
    -> std::optional<QueueFamilyIndices> {
  QueueFamilyIndices indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families.data());

  uint32_t i = 0;
  std::vector<uint32_t> scores(queue_family_count);
  auto check_queue = [&](const VkQueueFamilyProperties props) {
    scores[i] = props.queueFlags & kQueueFamilyBits;
    if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U &&
        (!indices.graphics_family.has_value() ||
         scores[i] < scores[indices.graphics_family.value()])) {
      indices.graphics_family = i;
    }
    if ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0U &&
        (!indices.compute_family.has_value() ||
         scores[i] < scores[indices.compute_family.value()])) {
      indices.compute_family = i;
    }
    if ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0U &&
        (!indices.transfer_family.has_value() ||
         scores[i] < scores[indices.transfer_family.value()])) {
      indices.transfer_family = i;
    }

    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support != VK_FALSE) {
      indices.present_family = i;
    }

    if (indices.graphics_family.has_value() &&
        indices.compute_family.has_value() &&
        indices.transfer_family.has_value() &&
        indices.present_family.has_value()) {
      return true;
    }

    i += 1;
    return false;
  };

  if (std::any_of(std::begin(queue_families), std::end(queue_families),
                  check_queue)) {
    return {indices};
  }
  return {};
}

auto query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
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

auto device_extensions(VkPhysicalDevice device)
    -> std::optional<std::vector<const char*>> {
  uint32_t count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

  std::vector<VkExtensionProperties> exts(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, exts.data());

  std::unordered_set<std::string> required_exts(std::begin(kDeviceExtensions),
                                                std::end(kDeviceExtensions));
  std::for_each(
      std::begin(exts), std::end(exts),
      [&required_exts](const VkExtensionProperties prop) {
        required_exts.erase(static_cast<const char*>(prop.extensionName));
      });
  if (!required_exts.empty()) {
    return {};
  }

  std::vector<const char*> ret(std::begin(kDeviceExtensions),
                               std::end(kDeviceExtensions));
  if (std::any_of(std::begin(exts), std::end(exts),
                  [](const VkExtensionProperties prop) {
                    return std::string(
                               static_cast<const char*>(prop.extensionName)) ==
                           "VK_KHR_portability_subset";
                  })) {
    ret.push_back("VK_KHR_portability_subset");
  }
  return ret;
}

auto is_device_suitable(const DeviceConfig& config,
                        VkPhysicalDevice device,
                        VkSurfaceKHR surface) -> bool {
  VkPhysicalDeviceProperties props = {};
  vkGetPhysicalDeviceProperties(device, &props);

  return props.apiVersion >= config.version().to_vk() &&
         device_extensions(device).has_value() &&
         query_swap_chain_support(device, surface) &&
         find_queue_families(device, surface);
}

}  // namespace

Device::Device(const DeviceConfig& config)
    : dimensions_cb_(config.dimensions_cb()),
      event_service_(config.event_service()),
      enable_validation_(config.enable_validation()) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  assert(dimensions_cb_);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  assert(event_service_ != nullptr);

  auto create_surface = config.surface_cb();

  create_instance(config);
  create_surface(*this);
  pick_physical_device(config);
  create_logical_device();
  create_command_pools();

  event_service_->add(
      el::EventType::kResized,
      [this](const el::Event* /*evt*/) -> void { this->set_resized(); });
}

Device::~Device() {
  vkDestroyCommandPool(device_, compute_cmd_pool_, nullptr);
  vkDestroyCommandPool(device_, transfer_cmd_pool_, nullptr);
  vkDestroyCommandPool(device_, graphics_cmd_pool_, nullptr);

  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);

  auto vkDestroyDebugUtilsMessengerEXT =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));
  if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
    vkDestroyDebugUtilsMessengerEXT(instance_, debug_handler_, nullptr);
  }

  vkDestroyInstance(instance_, nullptr);
}

auto Device::create_logical_device() -> void {
  auto indices = find_queue_families(physical_device_.device, surface_);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::unordered_set<uint32_t> unique_queue_families = {
      indices->graphics_family.value(), indices->compute_family.value(),
      indices->transfer_family.value(), indices->present_family.value()};
  float priority = 1.0F;
  std::for_each(std::begin(unique_queue_families),
                std::end(unique_queue_families), [&](uint32_t idx) {
                  queue_create_infos.push_back(
                      {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                       .queueFamilyIndex = idx,
                       .queueCount = 1,
                       .pQueuePriorities = &priority});
                });

  auto dev_exts = device_extensions(physical_device_.device).value();
  VkPhysicalDeviceFeatures device_features{};
  VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = uint32_t(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = uint32_t(dev_exts.size()),
      .ppEnabledExtensionNames = dev_exts.data(),
      .pEnabledFeatures = &device_features,
  };

  auto res =
      vkCreateDevice(physical_device_.device, &create_info, nullptr, &device_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create device: ").append(to_string(res)));
  }

  vkGetDeviceQueue(device_, indices->graphics_family.value(), 0,
                   &graphics_queue_);
  vkGetDeviceQueue(device_, indices->compute_family.value(), 0,
                   &compute_queue_);
  vkGetDeviceQueue(device_, indices->transfer_family.value(), 0,
                   &transfer_queue_);
  vkGetDeviceQueue(device_, indices->present_family.value(), 0,
                   &present_queue_);
}

auto Device::create_command_pools() -> void {
  auto indices = find_queue_families(physical_device_.device, surface_);

  struct PoolInfo {
    uint32_t index = 0;
    EL_PAD(4);
    VkCommandPool* pool = nullptr;
  };
  std::array<PoolInfo, 3> pools = {
      PoolInfo{
          .index = indices->graphics_family.value(),
          .pool = &graphics_cmd_pool_,
      },
      PoolInfo{
          .index = indices->transfer_family.value(),
          .pool = &transfer_cmd_pool_,
      },
      PoolInfo{
          .index = indices->compute_family.value(),
          .pool = &compute_cmd_pool_,
      },
  };

  auto cmd_pool_creator = [device = device_](const PoolInfo& info) {
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = info.index,
    };

    auto res = vkCreateCommandPool(device, &create_info, nullptr, info.pool);
    if (res != VK_SUCCESS) {
      throw std::runtime_error(std::string("failed to create command pool: ")
                                   .append(to_string(res)));
    }
  };

  std::for_each(std::begin(pools), std::end(pools), cmd_pool_creator);
}

void Device::check_validation_available_if_needed() const {
  if (!enable_validation_) {
    return;
  }

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

  auto has_layer = [&layers](std::string_view name) noexcept {
    return std::any_of(
        std::begin(layers), std::end(layers),
        [name](const auto& prop) noexcept {
          std::string_view layer_name(static_cast<const char*>(prop.layerName));
          return name.compare(layer_name) == 0;
        });
  };

  if (!std::all_of(std::begin(kValidationLayers), std::end(kValidationLayers),
                   has_layer)) {
    throw std::runtime_error("Validation layer not available");
  }
}

auto Device::build_debug_create_info(const DeviceConfig& config) const
    -> VkDebugUtilsMessengerCreateInfoEXT {
  if (!enable_validation_) {
    return {};
  }

  return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData = static_cast<void*>(config.error_data()),
  };
}

void Device::setup_debug_handler_if_needed(
    VkDebugUtilsMessengerCreateInfoEXT* debug_create_info) {
  if (!enable_validation_) {
    return;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));
  if (func == nullptr) {
    throw std::runtime_error("DebugUtilsMessengerEXT not available");
  }

  auto res = func(instance_, debug_create_info, nullptr, &debug_handler_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create debug handler: ").append(to_string(res)));
  }
}

void Device::pick_physical_device(const DeviceConfig& config) {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance_, &count, nullptr);
  if (count == 0) {
    throw std::runtime_error("No supported GPUs found");
  }

  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance_, &count, devices.data());

  auto is_suitable = [&](const auto& device) noexcept {
    return is_device_suitable(config, device, surface_);
  };
  auto iter = std::find_if(std::begin(devices), std::end(devices), is_suitable);
  if (iter == devices.end()) {
    throw std::runtime_error("No suitable GPUs found");
  }

  physical_device_.device = *iter;
}

void Device::create_instance(const DeviceConfig& config) {
  check_validation_available_if_needed();

  auto exts = config.device_extensions();
  if (enable_validation_) {
    exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  auto app_info = build_app_info(config);
  auto instance_create_info = build_instance_create_info(&app_info, exts);
  auto debug_create_info = build_debug_create_info(config);

  std::vector<VkValidationFeatureEnableEXT> enabled_validation_features;
  std::vector<VkValidationFeatureDisableEXT> disabled_validation_features;
  VkValidationFeaturesEXT validation_features = {};
  if (enable_validation_) {
    debug_create_info.pNext = instance_create_info.pNext;

    instance_create_info.enabledLayerCount = kValidationLayers.size();
    instance_create_info.ppEnabledLayerNames = kValidationLayers.data();
    instance_create_info.pNext = &debug_create_info;

    enabled_validation_features = {
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    disabled_validation_features = {VK_VALIDATION_FEATURE_DISABLE_ALL_EXT};

    validation_features = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .enabledValidationFeatureCount =
            uint32_t(enabled_validation_features.size()),
        .pEnabledValidationFeatures = enabled_validation_features.data(),
        .disabledValidationFeatureCount =
            uint32_t(disabled_validation_features.size()),
        .pDisabledValidationFeatures = disabled_validation_features.data(),
    };

    validation_features.pNext = instance_create_info.pNext;
    instance_create_info.pNext = &validation_features;
  }

  auto res = vkCreateInstance(&instance_create_info, nullptr, &instance_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(std::string("Failed to create vulkan instance: ")
                                 .append(to_string(res)));
  }

  setup_debug_handler_if_needed(&debug_create_info);
}

}  // namespace el::engine
