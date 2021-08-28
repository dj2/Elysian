#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "src/engine/device.h"
#include "src/pad.h"

namespace el::engine {
namespace shader {

enum class Type {
  kVertex,
  kFragment,
  kCompute,
};

}  // namespace shader

class ShaderConfig {
 public:
  explicit ShaderConfig(Device* device) : device_(device) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    assert(device);
  }

  auto set_data(std::vector<uint32_t>&& data) -> ShaderConfig& {
    data_ = std::move(data);
    return *this;
  }

  auto set_type(shader::Type type) -> ShaderConfig& {
    type_ = type;
    return *this;
  }

  auto set_entrypoint_name(std::string_view name) {
    entrypoint_name_ = name;
    return *this;
  }

  [[nodiscard]] auto device() const -> Device* { return device_; }

  [[nodiscard]] auto data() const -> const std::vector<uint32_t>& {
    return data_;
  }

  [[nodiscard]] auto type() const -> shader::Type { return type_; }

  [[nodiscard]] auto entrypoint_name() const -> std::string_view {
    return entrypoint_name_;
  }

 private:
  Device* device_ = nullptr;
  std::string entrypoint_name_ = "main";
  std::vector<uint32_t> data_;
  shader::Type type_ = shader::Type::kVertex;
  EL_PAD(4);
};

class Shader {
 public:
  explicit Shader(const ShaderConfig& config);
  Shader(const Shader&) = delete;
  Shader(const Shader&&) = delete;
  ~Shader();

  auto operator=(const Shader&) -> Shader& = delete;
  auto operator=(const Shader&&) -> Shader& = delete;

  auto create_info() -> VkPipelineShaderStageCreateInfo { return stage_info_; }

 private:
  VkShaderModule module_{};
  VkPipelineShaderStageCreateInfo stage_info_{};
  shader::Type type_;
  EL_PAD(4);
};

}  // namespace el::engine
