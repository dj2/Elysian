#pragma once

#include <cstdint>

namespace el::engine {

// Shift values for creating a Vulkan Version.
constexpr auto kVkVariantShift = 29;
constexpr auto kVkMajorShift = 22;
constexpr auto kVkMinorShift = 12;

class VersionInfo {
 public:
  VersionInfo() = default;
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  VersionInfo(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch)
      : variant_(variant), major_(major), minor_(minor), patch_(patch) {}
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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

}  // namespace el::engine
