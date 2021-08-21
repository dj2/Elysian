#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#include <vulkan/vulkan.h>
#pragma clang diagnostic pop

#include "src/pad.h"

namespace el {
namespace engine {

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
  ~Device();

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

  DeviceBuilder& enable_validation();
  DeviceBuilder& app_name(std::string_view app_name);
  DeviceBuilder& app_version(uint32_t major, uint32_t minor, uint32_t patch);
  DeviceBuilder& device_extensions(std::vector<const char*> exts);
  // DeviceBuilder& error_callback(ErrorData* data);

  std::unique_ptr<Device> build();

 protected:
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

 private:
  [[nodiscard]] bool is_device_suitable(VkPhysicalDevice);
  void check_validation_available_if_needed();
  VkApplicationInfo build_app_info();
  VkInstanceCreateInfo build_instance_create_info(VkApplicationInfo* app_info);
  VkDebugUtilsMessengerCreateInfoEXT build_debug_create_info();
  void setup_debug_handler_if_needed(
      Device* d,
      VkDebugUtilsMessengerCreateInfoEXT* debug_create_info);
  void pick_physical_device(Device* d);
};

}  // namespace engine
}  // namespace el
