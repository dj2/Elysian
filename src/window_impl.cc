module;

#include "src/glfw3.h"

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

import dimensions;
import event_service;

module window;

namespace el {

Window::Window(const WindowConfig& config) {
  glfwSetErrorCallback([](int error, const char* desc) {
    std::cerr << "Err: " << error << ": " << desc << std::endl;
  });

  if (!glfwInit()) {
    throw std::runtime_error("GLFW initialization failed.");
  }
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("GLFW vulkan support missing.");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window_ = glfwCreateWindow(static_cast<int>(config.width()),
                             static_cast<int>(config.height()),
                             config.title().data(), nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error("GLFW window creation failed.");
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* win, int, int) {
    auto* t = static_cast<Window*>(glfwGetWindowUserPointer(win));
    ResizeEvent evt;
    t->event_service_.emit(EventType::kResized, &evt);
  });
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

auto Window::required_engine_extensions() -> std::vector<const char*> {
  uint32_t glfw_ext_count = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
  if (glfw_exts == nullptr) {
    throw std::runtime_error("GLFW error retrieving instance extensions");
  }

  return std::vector<const char*>(glfw_exts, glfw_exts + glfw_ext_count);
}

auto Window::dimensions() const -> Dimensions {
  int w = 0;
  int h = 0;
  glfwGetFramebufferSize(window_, &w, &h);

  return {
      .width = static_cast<uint32_t>(w),
      .height = static_cast<uint32_t>(h),
  };
}

}  // namespace el
