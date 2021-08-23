module;

#include "src/glfw3.h"

#include <string_view>
#include <vector>

export module window;

namespace el {

export struct Dimensions {
  uint32_t width;
  uint32_t height;
};

constexpr uint32_t kDefaultWidth = 800;
constexpr uint32_t kDefaultHeight = 600;

export class WindowConfig {
 public:
  auto set_title(std::string_view title) -> WindowConfig& {
    title_ = title;
    return *this;
  }

  auto set_dimensions(Dimensions dims) -> WindowConfig& {
    dimensions_ = dims;
    return *this;
  }

  [[nodiscard]] auto title() const -> std::string_view { return title_; }
  [[nodiscard]] auto width() const -> uint32_t { return dimensions_.width; }
  [[nodiscard]] auto height() const -> uint32_t { return dimensions_.height; }

 private:
  std::string_view title_;
  Dimensions dimensions_ = {.width = kDefaultWidth, .height = kDefaultHeight};
};

export class Window {
 public:
  explicit Window(const WindowConfig& config);
  ~Window();

  // Returned strings are owned by the window system and will be free'd.
  auto requiredEngineExtensions() -> std::vector<const char*>;

  [[nodiscard]] auto shouldClose() const -> bool {
    return glfwWindowShouldClose(window_) != 0;
  }

  auto static Poll() -> void { glfwPollEvents(); }

 private:
  GLFWwindow* window_ = nullptr;
};

}  // namespace el
