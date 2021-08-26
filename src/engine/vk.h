#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#include <vulkan/vulkan.h>
#pragma GCC diagnostic pop

#include <string_view>

auto to_string(VkResult result) -> std::string_view;
auto to_string(VkObjectType type) -> std::string_view;
