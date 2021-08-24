module;

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>

#include "src/vk.h"

import dimensions;

module engine;

namespace el::engine {
namespace {

const char* const kEngineName = "Elysian Engine";
constexpr uint8_t kEngineMajor = 0;
constexpr uint8_t kEngineMinor = 1;
constexpr uint8_t kEnginePatch = 0;

constexpr std::array<const char*, 1> kValidationLayers = {
    {"VK_LAYER_KHRONOS_validation"}};

auto objectTypeToName(VkObjectType type) -> std::string_view {
  switch (type) {
    case VK_OBJECT_TYPE_INSTANCE:
      return "instance";
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
      return "physical_device";
    case VK_OBJECT_TYPE_DEVICE:
      return "device";
    case VK_OBJECT_TYPE_QUEUE:
      return "queue";
    case VK_OBJECT_TYPE_SEMAPHORE:
      return "semaphore";
    case VK_OBJECT_TYPE_COMMAND_BUFFER:
      return "command_buffer";
    case VK_OBJECT_TYPE_FENCE:
      return "fence";
    case VK_OBJECT_TYPE_DEVICE_MEMORY:
      return "device_memory";
    case VK_OBJECT_TYPE_BUFFER:
      return "buffer";
    case VK_OBJECT_TYPE_IMAGE:
      return "image";
    case VK_OBJECT_TYPE_EVENT:
      return "event";
    case VK_OBJECT_TYPE_QUERY_POOL:
      return "query_pool";
    case VK_OBJECT_TYPE_BUFFER_VIEW:
      return "buffer_view";
    case VK_OBJECT_TYPE_IMAGE_VIEW:
      return "image_view";
    case VK_OBJECT_TYPE_SHADER_MODULE:
      return "shader_module";
    case VK_OBJECT_TYPE_PIPELINE_CACHE:
      return "pipeline_cache";
    case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
      return "pipeline_layout";
    case VK_OBJECT_TYPE_RENDER_PASS:
      return "render_pass";
    case VK_OBJECT_TYPE_PIPELINE:
      return "pipeline";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
      return "descriptor_set_layout";
    case VK_OBJECT_TYPE_SAMPLER:
      return "sampler";
    case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
      return "descriptor_pool";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET:
      return "descriptor_set";
    case VK_OBJECT_TYPE_FRAMEBUFFER:
      return "framebuffer";
    case VK_OBJECT_TYPE_COMMAND_POOL:
      return "command_pool";
    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
      return "sampler_ycbcr_conversion";
    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
      return "descriptor_update_template";
    case VK_OBJECT_TYPE_SURFACE_KHR:
      return "surface";
    case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
      return "swapchain";
    case VK_OBJECT_TYPE_DISPLAY_KHR:
      return "display";
    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
      return "display_mode";
    case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
      return "debug_report_callback";
    case VK_OBJECT_TYPE_CU_MODULE_NVX:
      return "cu_module";
    case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
      return "cu_function";
    case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
      return "debug_utils_messenger";
    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
      return "acceleration_structure";
    case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
      return "validation_cache";
    case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
      return "performance_configuration";
    case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
      return "deferred_operation";
    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
      return "indirect_commands_layout";
    case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT:
      return "private_data_slot";
    case VK_OBJECT_TYPE_UNKNOWN:
    case VK_OBJECT_TYPE_MAX_ENUM:
      return "unknown";
  }
  return "unknown";
}

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
      std::ranges::for_each(queues, [&msg_buf](const auto& label) {
        msg_buf << "  " << label.pLabelName << std::endl;
      });
    }
    if (data->cmdBufLabelCount > 0) {
      msg_buf << "Command Buffers:" << std::endl;

      std::span cmdBufLabels(data->pCmdBufLabels, data->cmdBufLabelCount);
      std::ranges::for_each(cmdBufLabels, [&msg_buf](const auto& buf) {
        msg_buf << "  " << buf.pLabelName << std::endl;
      });
    }
    if (data->objectCount > 0) {
      msg_buf << "Objects:" << std::endl;

      std::span objects(data->pObjects, data->objectCount);
      std::ranges::for_each(objects, [&msg_buf](const auto& obj) {
        msg_buf << "  " << objectTypeToName(obj.objectType);
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

}  // namespace

Device::Device(const DeviceConfig& config)
    : dimensions_cb_(config.dimensions_cb()),
      enable_validation_(config.enable_validation()) {
  assert(dimensions_cb_ != nullptr);

  build_instance(config);
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
    return std::ranges::any_of(layers, [name](const auto& prop) noexcept {
      return name.compare(prop.layerName) == 0;
    });
  };

  if (!std::ranges::all_of(kValidationLayers, has_layer)) {
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
    throw std::runtime_error("Failed to create debug handler");
  }
}

static auto is_device_suitable(VkPhysicalDevice /* unused */) -> bool {
  return true;
}

void Device::pick_physical_device() {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance_, &count, nullptr);
  if (count == 0) {
    throw std::runtime_error("No supported GPUs found");
  }

  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance_, &count, devices.data());

  auto is_suitable = [&](const auto& device) noexcept {
    return is_device_suitable(device);
  };
  auto iter = std::ranges::find_if(devices, is_suitable);
  if (iter == devices.end()) {
    throw std::runtime_error("No suitable GPUs found");
  }

  physical_device_.device = *iter;
}

void Device::build_instance(const DeviceConfig& config) {
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
    throw std::runtime_error("Failed to create vulkan instance");
  }

  setup_debug_handler_if_needed(&debug_create_info);
  pick_physical_device();
}

}  // namespace el::engine
