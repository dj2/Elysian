#include "src/window.h"

#include <cassert>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "src/dimensions.h"
#include "src/engine/vk.h"
#include "src/event_service.h"
#include "src/glfw3.h"

namespace el {

Window::Window(const WindowConfig& config)
    : event_service_(config.event_service()) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  assert(event_service_ != nullptr);

  glfwSetErrorCallback([](int error, const char* desc) {
    std::cerr << "Err: " << error << ": " << desc << std::endl;
  });

  if (glfwInit() == 0) {
    throw std::runtime_error("GLFW initialization failed.");
  }
  if (glfwVulkanSupported() == 0) {
    throw std::runtime_error("GLFW vulkan support missing.");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window_ = glfwCreateWindow(static_cast<int>(config.width()),
                             static_cast<int>(config.height()),
                             config.title().data(), nullptr, nullptr);
  if (window_ == nullptr) {
    throw std::runtime_error("GLFW window creation failed.");
  }

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(
      window_, [](GLFWwindow* win, int /*width*/, int /*height*/) {
        auto* t = static_cast<Window*>(glfwGetWindowUserPointer(win));
        ResizeEvent evt;
        t->event_service_->emit(EventType::kResized, &evt);
      });
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

// static
auto Window::required_engine_extensions() -> std::vector<const char*> {
  uint32_t glfw_ext_count = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
  if (glfw_exts == nullptr) {
    throw std::runtime_error("GLFW error retrieving instance extensions");
  }

  std::span exts(glfw_exts, glfw_ext_count);
  return {std::begin(exts), std::end(exts)};
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

auto Window::create_surface(engine::Device& device) -> void {
  device.create_surface([&](VkInstance instance) -> VkSurfaceKHR {
    VkSurfaceKHR surface = {};

    auto res =
        glfwCreateWindowSurface(instance, this->window_, nullptr, &surface);
    if (res != VK_SUCCESS) {
      throw std::runtime_error(
          std::string("glfwCreateWindowSurface: ").append(to_string(res)));
    }
    return surface;
  });
}

}  // namespace el
