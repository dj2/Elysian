module;

#include <cstdint>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

export module engine;

namespace el {
namespace engine {

export enum class ErrorSeverity {
  kVerbose,
  kInfo,
  kWarning,
  kError,
};

export enum class ErrorType {
  kGeneral,
  kValidation,
  kPerformance,
};

export enum class DeviceError {
  kNone,
  kEngineNotAvailable,
  kValidationLayerNotAvailable,
  kNoSupportedGPUsFound,
  kInstanceCreateFailed,
  kDebugHandlerCreateFailed,
  kErrorExtensionNotPresent,
  kNoSuitableGPUFound,
};

struct PhysicalDevice {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceMemoryProperties memory_properties;
};

export class Device {
 public:
  ~Device();

 private:
  friend class DeviceBuilder;

  Device();

  VkDebugUtilsMessengerEXT debug_handler_;

  PhysicalDevice physical_device_;

  VkInstance instance_;
};

export class DeviceBuilder {
 public:
  DeviceBuilder();
  ~DeviceBuilder();

  DeviceBuilder* enable_validation();
  DeviceBuilder* app_name(std::string_view app_name);
  DeviceBuilder* app_version(uint32_t major, uint32_t minor, uint32_t patch);
  DeviceBuilder* device_extensions(std::vector<const char*> exts);
  // DeviceBuilder* error_callback(ErrorData* data);

  std::unique_ptr<Device> build();

  [[nodiscard]] DeviceError error() const { return error_; }

 protected:
  void set_error(DeviceError error) { error_ = error; }

  bool enable_validation_ = false;
  std::string_view app_name_;
  // ErrorData* error_data_ = nullptr;
  std::vector<std::string_view> device_extensions_;
  struct {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
  } version_;

  DeviceError error_ = DeviceError::kNone;

 private:
  [[nodiscard]] bool is_device_suitable(VkPhysicalDevice);
  [[nodiscard]] DeviceError check_validation_available_if_needed() const;
  VkApplicationInfo build_app_info();
  VkInstanceCreateInfo build_instance_create_info(VkApplicationInfo* app_info);
  VkDebugUtilsMessengerCreateInfoEXT build_debug_create_info();
  DeviceError setup_debug_handler_if_needed(
      Device* d,
      VkDebugUtilsMessengerCreateInfoEXT* debug_create_info);
  DeviceError pick_physical_device(Device* d);
};

}  // namespace engine
}  // namespace el
