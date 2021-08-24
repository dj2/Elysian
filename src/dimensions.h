#pragma once

#include <cstdint>
#include <functional>

namespace el {

struct Dimensions {
  uint32_t width;
  uint32_t height;
};

using DimensionsCallback = std::function<Dimensions()>;

}  // namespace el
