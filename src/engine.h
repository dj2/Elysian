#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "src/dimensions.h"
#include "src/event_service.h"
#include "src/pad.h"
#include "src/vk.h"

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

struct Error {
  ErrorSeverity severity;
  ErrorType type;
  std::string_view message;
  void* user_data = nullptr;
};

using ErrorCallback = std::function<void(const Error& data)>;
struct ErrorData {
  ErrorCallback cb;
  void* user_data = nullptr;
};

struct PhysicalDevice {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures features = {};
  EL_PAD(4);
  VkPhysicalDeviceProperties properties = {};
  VkPhysicalDeviceMemoryProperties memory_properties = {};
};

class VersionInfo {
 public:
  VersionInfo() = default;
  VersionInfo(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch)
      : variant_(variant), major_(major), minor_(minor), patch_(patch) {}
  VersionInfo(uint32_t major, uint32_t minor, uint32_t patch)
      : VersionInfo(0, major, minor, patch) {}

  [[nodiscard]] auto to_vk() const -> uint32_t {
    return variant_ << kVkVariantShift | major_ << kVkMajorShift |
           minor_ << kVkMinorShift | patch_;
  }

 private:
  uint32_t variant_ = 0;
  uint32_t major_ = 1;
  uint32_t minor_ = 0;
  uint32_t patch_ = 0;
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

  auto set_error_data(ErrorData* data) -> DeviceConfig& {
    error_data_ = data;
    return *this;
  }

  auto set_dimensions_cb(DimensionsCallback cb) -> DeviceConfig& {
    dimensions_cb_ = std::move(cb);
    return *this;
  }

  auto set_event_service(EventService* event_service) -> DeviceConfig& {
    event_service_ = event_service;
    return *this;
  }

  [[nodiscard]] auto enable_validation() const -> bool {
    return enable_validation_;
  }
  [[nodiscard]] auto app_name() const -> std::string_view { return app_name_; }
  [[nodiscard]] auto device_extensions() const -> std::vector<const char*> {
    return device_extensions_;
  }
  [[nodiscard]] auto version() const -> VersionInfo { return version_; }
  [[nodiscard]] auto error_data() const -> ErrorData* { return error_data_; }
  [[nodiscard]] auto dimensions_cb() const -> DimensionsCallback {
    return dimensions_cb_;
  }
  [[nodiscard]] auto event_service() const -> EventService* {
    return event_service_;
  }

 private:
  std::string_view app_name_;
  ErrorData* error_data_ = nullptr;
  std::vector<const char*> device_extensions_ = {};
  VersionInfo version_ = {};
  DimensionsCallback dimensions_cb_;
  EventService* event_service_ = nullptr;

  bool enable_validation_ = false;
  EL_PAD(7);
};

class Device {
 public:
  explicit Device(const DeviceConfig& config);

  auto set_resized() -> void { framebuffer_resized_ = true; }

 private:
  // [[nodiscard]] auto is_device_suitable(VkPhysicalDevice device) const ->
  // bool;
  void check_validation_available_if_needed() const;
  [[nodiscard]] auto build_debug_create_info(const DeviceConfig& config) const
      -> VkDebugUtilsMessengerCreateInfoEXT;
  void setup_debug_handler_if_needed(
      VkDebugUtilsMessengerCreateInfoEXT* debug_create_info);
  void pick_physical_device();
  void build_instance(const DeviceConfig& config);

  DimensionsCallback dimensions_cb_ = nullptr;
  EventService* event_service_ = nullptr;

  VkDebugUtilsMessengerEXT debug_handler_ = nullptr;

  PhysicalDevice physical_device_ = {};

  VkInstance instance_ = {};
  bool enable_validation_ = false;
  bool framebuffer_resized_ = false;

  EL_PAD(6);
};

}  // namespace el::engine
