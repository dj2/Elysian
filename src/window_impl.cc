module;

#include "src/glfw3.h"

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

module window;

namespace el {

Window::Window(const WindowConfig& config) {
  glfwSetErrorCallback([](int error, const char* desc) {
    std::cerr << "Err: " << error << ": " << desc << std::endl;
  });

  if (!glfwInit()) {
    std::runtime_error("GLFW initialization failed.");
  }
  if (!glfwVulkanSupported()) {
    std::runtime_error("GLFW vulkan support missing.");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window_ = glfwCreateWindow(static_cast<int>(config.width()),
                             static_cast<int>(config.height()),
                             config.title().data(), nullptr, nullptr);
  if (!window_) {
    std::runtime_error("GLFW window creation failed.");
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(
      window_, [](GLFWwindow* /*window*/, int /*width*/, int /*height*/) {});
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

auto Window::requiredEngineExtensions() -> std::vector<const char*> {
  uint32_t glfw_ext_count = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
  if (glfw_exts == nullptr) {
    throw std::runtime_error("GLFW error retrieving instance extensions");
  }

  return std::vector<const char*>(glfw_exts, glfw_exts + glfw_ext_count);
}

}  // namespace el
