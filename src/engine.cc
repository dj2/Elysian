#include <algorithm>
#include <cstdint>
#include <iostream>
#include <span>
#include <sstream>
#include <stdexcept>

#include "src/engine.h"

namespace el::engine {
namespace {

#define TO_VK_VERSION(variant, major, minor, patch) \
  (((variant) << 29) | ((major) << 22) | ((minor) << 12) | ((patch)))

#define ARRAY_SIZE(ary) (sizeof(ary) / sizeof((ary)[0]))

const char* kEngineName = "Elysian Engine";
constexpr uint8_t kEngineMajor = 0;
constexpr uint8_t kEngineMinor = 1;
constexpr uint8_t kEnginePatch = 0;

std::array<const char*, 1> kValidationLayers = {
    {"VK_LAYER_KHRONOS_validation"}};
uint32_t kValidationLayerCount = ARRAY_SIZE(kValidationLayers);

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
                    void* /*user_data*/) -> VkBool32 {
  // ErrorData* err_data = static_cast<ErrorData*>(user_data);

  // if (err_data && err_data->cb) {
  ErrorSeverity sev = ErrorSeverity::kError;
  if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
    sev = ErrorSeverity::kWarning;
  } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
    sev = ErrorSeverity::kInfo;
  } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) !=
             0) {
    sev = ErrorSeverity::kVerbose;
  }

  ErrorType ty = ErrorType::kGeneral;
  if ((type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
    ty = ErrorType::kPerformance;
  } else if ((type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
    ty = ErrorType::kValidation;
  }

  std::stringstream msg_buf;
  msg_buf << "Err: " << data->pMessage << std::endl;
  if (data->pMessageIdName != nullptr) {
    msg_buf << "MessageId (" << data->messageIdNumber
            << "): " << data->pMessageIdName << std::endl;
  }
  if (data->queueLabelCount > 0) {
    msg_buf << "Queues:" << std::endl;

    std::ranges::for_each({data->pQueueLabels, data->queueLabelCount},
                          [&msg_buf](const auto& label) {
                            msg_buf << "  " << label.pLabelName << std::endl;
                          });
  }
  if (data->cmdBufLabelCount > 0) {
    msg_buf << "Command Buffers:" << std::endl;

    std::ranges::for_each({data->pCmdBufLabels, data->cmdBufLabelCount},
                          [&msg_buf](const auto& buf) {
                            msg_buf << "  " << buf.pLabelName << std::endl;
                          });
  }
  if (data->objectCount > 0) {
    msg_buf << "Objects:" << std::endl;

    std::ranges::for_each(
        {data->pObjects, data->objectCount}, [&msg_buf](const auto& obj) {
          msg_buf << "  " << objectTypeToName(obj.objectType);
          msg_buf << "(0x" << std::hex << obj.objectHandle << ")";
          if (obj.pObjectName) {
            msg_buf << " " << obj.pObjectName;
          }
          msg_buf << std::endl;
        });
  }

  std::cout << "msg: " << msg_buf.str() << std::endl;
  // err_data->cb({
  //   .severity = sev,
  //   .type = ty,
  //   .message = msg_buf.str().c_str(),
  //   .data = err_data->user_data
  // });
  //}
  return VK_FALSE;
}

}  // namespace

Device::Device() = default;

DeviceBuilder::DeviceBuilder() = default;

DeviceBuilder::~DeviceBuilder() = default;

auto DeviceBuilder::enable_validation() -> DeviceBuilder& {
  enable_validation_ = true;
  return *this;
}

auto DeviceBuilder::app_name(std::string_view app_name) -> DeviceBuilder& {
  app_name_ = app_name;
  return *this;
}

auto DeviceBuilder::app_version(uint32_t major, uint32_t minor, uint32_t patch)
    -> DeviceBuilder& {
  version_.major = major;
  version_.minor = minor;
  version_.patch = patch;
  return *this;
}

auto DeviceBuilder::device_extensions(const std::vector<const char*>& exts)
    -> DeviceBuilder& {
  device_extensions_ = exts;
  return *this;
}

// auto DeviceBuilder::error_callback(ErrorData* data) -> DeviceBuilder& {
//   error_data_ = data;
//   return *this;
// }

void DeviceBuilder::check_validation_available_if_needed() const {
  if (!enable_validation_) {
    return;
  }

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

  auto has_layer = [&layers](std::string_view name) {
    return std::ranges::any_of(layers, [name](const auto& prop) {
      return name.compare(prop.layerName) == 0;
    });
  };

  if (!std::ranges::all_of(kValidationLayers, has_layer)) {
    throw std::runtime_error("Validation layer not available");
  }
}

auto DeviceBuilder::build_app_info() -> VkApplicationInfo {
  return {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = app_name_.data(),
      .applicationVersion =
          TO_VK_VERSION(0, version_.major, version_.minor, version_.patch),
      .pEngineName = kEngineName,
      .engineVersion =
          TO_VK_VERSION(0, kEngineMajor, kEngineMinor, kEnginePatch),
      .apiVersion = TO_VK_VERSION(0, 1, 2, 0),
  };
}

auto DeviceBuilder::build_instance_create_info(VkApplicationInfo* app_info)
    -> VkInstanceCreateInfo {
  return {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = app_info,
      .enabledLayerCount = 0,
      .enabledExtensionCount = uint32_t(device_extensions_.size()),
      .ppEnabledExtensionNames = device_extensions_.data(),
  };
}

auto DeviceBuilder::build_debug_create_info() const
    -> VkDebugUtilsMessengerCreateInfoEXT {
  if (!enable_validation_) {
    return {};
  }

  return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      //.pUserData = error_data_,
  };
}

void DeviceBuilder::setup_debug_handler_if_needed(
    Device* d,
    VkDebugUtilsMessengerCreateInfoEXT* debug_create_info) const {
  if (!enable_validation_) {
    return;
  }

  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(d->instance_, "vkCreateDebugUtilsMessengerEXT"));
  if (func == nullptr) {
    throw std::runtime_error("DebugUtilsMessengerEXT not available");
  }

  auto res = func(d->instance_, debug_create_info, nullptr, &d->debug_handler_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create debug handler");
  }
}

static auto is_device_suitable(VkPhysicalDevice /* unused */) -> bool {
  return true;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void DeviceBuilder::pick_physical_device(Device* d) {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(d->instance_, &count, nullptr);
  if (count == 0) {
    throw std::runtime_error("No supported GPUs found");
  }

  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(d->instance_, &count, devices.data());

  auto is_suitable = [&](const auto& device) {
    return is_device_suitable(device);
  };
  auto it = std::ranges::find_if(devices, is_suitable);
  if (it == devices.end()) {
    throw std::runtime_error("No suitable GPUs found");
  }

  d->physical_device_.device = *it;
}

auto DeviceBuilder::build() -> std::unique_ptr<Device> {
  check_validation_available_if_needed();

  if (enable_validation_) {
    device_extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  auto app_info = build_app_info();
  auto instance_create_info = build_instance_create_info(&app_info);
  auto debug_create_info = build_debug_create_info();

  if (enable_validation_) {
    instance_create_info.enabledLayerCount = kValidationLayerCount;
    instance_create_info.ppEnabledLayerNames = kValidationLayers.data();
    instance_create_info.pNext = &debug_create_info;
  }

  // Using new because of private constructor.
  auto ptr = std::unique_ptr<Device>(new Device());
  Device* d = ptr.get();

  auto res = vkCreateInstance(&instance_create_info, nullptr, &d->instance_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vulkan instance");
  }

  setup_debug_handler_if_needed(d, &debug_create_info);
  pick_physical_device(d);

  return ptr;
}

}  // namespace el::engine
