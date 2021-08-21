#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <vulkan/vulkan.h>
#pragma clang diagnostic pop

#include "src/pad.h"

namespace el::engine {

enum class ErrorSeverity {
  kVerbose,
  kInfo,
  kWarning,
  kError,
};

enum class ErrorType {
  kGeneral,
  kValidation,
  kPerformance,
};

struct PhysicalDevice {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures features;
  EL_PAD(4);
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memory_properties;
};

class Device {
 public:
  ~Device() = default;

 private:
  friend class DeviceBuilder;

  Device();

  VkDebugUtilsMessengerEXT debug_handler_;

  PhysicalDevice physical_device_;

  VkInstance instance_;
};

class DeviceBuilder {
 public:
  DeviceBuilder();
  ~DeviceBuilder();

  auto enable_validation() -> DeviceBuilder&;
  auto app_name(std::string_view app_name) -> DeviceBuilder&;
  auto app_version(uint32_t major, uint32_t minor, uint32_t patch)
      -> DeviceBuilder&;
  auto device_extensions(const std::vector<const char*>& exts)
      -> DeviceBuilder&;
  // auto error_callback(ErrorData* data) -> DeviceBuilder&;

  auto build() -> std::unique_ptr<Device>;

 private:
  // [[nodiscard]] auto is_device_suitable(VkPhysicalDevice device) const ->
  // bool;
  void check_validation_available_if_needed() const;
  auto build_app_info() -> VkApplicationInfo;
  auto build_instance_create_info(VkApplicationInfo* app_info)
      -> VkInstanceCreateInfo;
  [[nodiscard]] auto build_debug_create_info() const
      -> VkDebugUtilsMessengerCreateInfoEXT;
  void setup_debug_handler_if_needed(
      Device* d,
      VkDebugUtilsMessengerCreateInfoEXT* debug_create_info) const;
  void pick_physical_device(Device* d);

  std::string_view app_name_;
  // ErrorData* error_data_ = nullptr;
  std::vector<const char*> device_extensions_;
  struct {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
  } version_;

  bool enable_validation_ = false;
  EL_PAD(3);
};

}  // namespace el::engine
