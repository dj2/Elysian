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

// Shift values for creating a Vulkan Version.
constexpr auto kVkVariantShift = 29;
constexpr auto kVkMajorShift = 22;
constexpr auto kVkMinorShift = 12;

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
  VkPhysicalDeviceFeatures features = {};
  EL_PAD(4);
  VkPhysicalDeviceProperties properties = {};
  VkPhysicalDeviceMemoryProperties memory_properties = {};
};

struct VersionInfo {
  uint32_t variant = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
  uint32_t major = 1;    // NOLINT(misc-non-private-member-variables-in-classes)
  uint32_t minor = 0;    // NOLINT(misc-non-private-member-variables-in-classes)
  uint32_t patch = 0;    // NOLINT(misc-non-private-member-variables-in-classes)

  [[nodiscard]] auto to_vk() const -> uint32_t {
    return variant << kVkVariantShift | major << kVkMajorShift |
           minor << kVkMinorShift | patch;
  }
};

class DeviceConfig {
 public:
  auto set_enable_validation() -> DeviceConfig& {
    enable_validation_ = true;
    return *this;
  }

  auto set_app_name(std::string_view app_name) -> DeviceConfig& {
    app_name_ = app_name;
    return *this;
  }

  auto set_app_version(uint32_t major, uint32_t minor, uint32_t patch)
      -> DeviceConfig& {
    version_ = {0, major, minor, patch};
    return *this;
  }

  auto set_device_extensions(const std::vector<const char*>& exts)
      -> DeviceConfig& {
    device_extensions_ = exts;
    return *this;
  }
  // auto error_callback(ErrorData* data) -> DeviceConfig& {
  //   error_data_ = data;
  //   return *this;
  // }

  [[nodiscard]] auto enable_validation() const -> bool {
    return enable_validation_;
  }
  [[nodiscard]] auto app_name() const -> std::string_view { return app_name_; }
  [[nodiscard]] auto device_extensions() const -> std::vector<const char*> {
    return device_extensions_;
  }
  [[nodiscard]] auto version() const -> VersionInfo { return version_; }

 private:
  std::string_view app_name_;
  // ErrorData* error_data_ = nullptr;
  std::vector<const char*> device_extensions_;
  VersionInfo version_;

  bool enable_validation_ = false;
  EL_PAD(3);
};

class Device {
 public:
  explicit Device(const DeviceConfig& config);

 private:
  // [[nodiscard]] auto is_device_suitable(VkPhysicalDevice device) const ->
  // bool;
  void check_validation_available_if_needed() const;
  [[nodiscard]] auto build_debug_create_info() const
      -> VkDebugUtilsMessengerCreateInfoEXT;
  void setup_debug_handler_if_needed(
      VkDebugUtilsMessengerCreateInfoEXT* debug_create_info);
  void pick_physical_device();
  void build_instance(const DeviceConfig& config);

  VkDebugUtilsMessengerEXT debug_handler_ = nullptr;

  PhysicalDevice physical_device_;

  VkInstance instance_ = {};
  bool enable_validation_;
};

}  // namespace el::engine
