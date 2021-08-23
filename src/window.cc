module;

#include "src/glfw3.h"

#include <string_view>
#include <vector>

export module window;

namespace el {

export class WindowConfig {
 public:
  auto set_title(std::string_view title) -> WindowConfig& {
    title_ = title;
    return *this;
  }

  auto set_dimensions(uint32_t width, uint32_t height) -> WindowConfig& {
    width_ = width;
    height_ = height;
    return *this;
  }

  [[nodiscard]] auto title() const -> std::string_view { return title_; }
  [[nodiscard]] auto width() const -> uint32_t { return width_; }
  [[nodiscard]] auto height() const -> uint32_t { return height_; }

 private:
  std::string_view title_;
  uint32_t width_ = 800;
  uint32_t height_ = 600;
};

export class Window {
 public:
  Window(const WindowConfig& config);
  ~Window();

  // Returned strings are owned by the window system and will be free'd.
  auto requiredEngineExtensions() -> std::vector<const char*>;

  auto shouldClose() const -> bool { return glfwWindowShouldClose(window_); }

  auto poll() -> void { glfwPollEvents(); }

 private:
  GLFWwindow* window_ = nullptr;
};

}  // namespace el
