#include "src/engine.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "src/dimensions.h"
#include "src/engine/vk.h"
#include "src/event_service.h"

namespace el::engine {
namespace {

const char* const kEngineName = "Elysian Engine";
constexpr uint8_t kEngineMajor = 0;
constexpr uint8_t kEngineMinor = 1;
constexpr uint8_t kEnginePatch = 0;

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
      std::for_each(std::begin(queues), std::end(queues), [&msg_buf](const auto& label) {
        msg_buf << "  " << label.pLabelName << std::endl;
      });
    }
    if (data->cmdBufLabelCount > 0) {
      msg_buf << "Command Buffers:" << std::endl;

      std::span cmdBufLabels(data->pCmdBufLabels, data->cmdBufLabelCount);
      std::for_each(std::begin(cmdBufLabels), std::end(cmdBufLabels), [&msg_buf](const auto& buf) {
        msg_buf << "  " << buf.pLabelName << std::endl;
      });
    }
    if (data->objectCount > 0) {
      msg_buf << "Objects:" << std::endl;

      std::span objects(data->pObjects, data->objectCount);
      std::for_each(std::begin(objects), std::end(objects), [&msg_buf](const auto& obj) {
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

auto check_device_extensions_supported(VkPhysicalDevice device) -> bool {
  uint32_t count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

  std::vector<VkExtensionProperties> exts(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, exts.data());

  std::unordered_set<std::string> required_exts(std::begin(kDeviceExtensions),
                                                std::end(kDeviceExtensions));
  std::for_each(
      std::begin(exts), std::end(exts), [&required_exts](const VkExtensionProperties prop) {
        required_exts.erase(static_cast<const char*>(prop.extensionName));
      });
  return required_exts.empty();
}

auto is_device_suitable(const DeviceConfig& config, VkPhysicalDevice device)
    -> bool {
  VkPhysicalDeviceProperties props = {};
  vkGetPhysicalDeviceProperties(device, &props);
  if (props.apiVersion < config.version().to_vk()) {
    return false;
  }
  if (!check_device_extensions_supported(device)) {
    return false;
  }

  return true;
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
  pick_physical_device(config);
  create_surface(*this);
  pick_logical_device();

  event_service_->add(
      el::EventType::kResized,
      [this](const el::Event* /*evt*/) -> void { this->set_resized(); });
}

auto Device::pick_logical_device() -> void {}

void Device::check_validation_available_if_needed() const {
  if (!enable_validation_) {
    return;
  }

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

  auto has_layer = [&layers](std::string_view name) noexcept {
    return std::any_of(std::begin(layers), std::end(layers), [name](const auto& prop) noexcept {
      std::string_view layer_name(static_cast<const char*>(prop.layerName));
      return name.compare(layer_name) == 0;
    });
  };

  if (!std::all_of(std::begin(kValidationLayers), std::end(kValidationLayers), has_layer)) {
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
    return is_device_suitable(config, device);
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

  if (enable_validation_) {
    instance_create_info.enabledLayerCount = kValidationLayers.size();
    instance_create_info.ppEnabledLayerNames = kValidationLayers.data();
    instance_create_info.pNext = &debug_create_info;
  }

  auto res = vkCreateInstance(&instance_create_info, nullptr, &instance_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(std::string("Failed to create vulkan instance: ")
                                 .append(to_string(res)));
  }

  setup_debug_handler_if_needed(&debug_create_info);
}

}  // namespace el::engine
