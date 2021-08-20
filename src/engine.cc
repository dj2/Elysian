module;

#include <vulkan/vulkan.h>
#include <cstdint>

export module engine;

namespace el {
namespace engine {

export class Device {
 public:
  Device() = default;
  ~Device() = default;

  uint32_t ver() const { return 42; }
};

}  // namespace engine
}  // namespace el
