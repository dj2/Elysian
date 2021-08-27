#include "src/engine/shader.h"

#include <cassert>

#include "src/engine/vk.h"

namespace el::engine {
namespace {

auto type_to_vk(shader::Type type) -> VkShaderStageFlagBits {
  switch (type) {
    case shader::Type::kVertex:
      return VK_SHADER_STAGE_VERTEX_BIT;
    case shader::Type::kFragment:
      return VK_SHADER_STAGE_FRAGMENT_BIT;
    case shader::Type::kCompute:
      return VK_SHADER_STAGE_COMPUTE_BIT;
  }
}

}  // namespace

Shader::Shader(const ShaderConfig& config) : type_(config.type()) {
  VkShaderModuleCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize =
          uint32_t(config.data().size() * sizeof(uint32_t)),  // size in bytes
      .pCode = config.data().data(),
  };

  auto res = vkCreateShaderModule(config.device()->device(), &create_info,
                                  nullptr, &module_);
  if (res != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Failed to create shader module: ").append(to_string(res)));
  }

  stage_info_ = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = type_to_vk(config.type()),
      .module = module_,
      .pName = config.entrypoint_name().data(),
  };
}

}  // namespace el::engine
