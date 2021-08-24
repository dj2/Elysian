module;

#include <cstdint>
#include <functional>

export module dimensions;

namespace el {

export struct Dimensions {
  uint32_t width;
  uint32_t height;
};

export using DimensionsCallback = std::function<Dimensions()>;

}  // namespace el
