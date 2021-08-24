#pragma once

#include <string_view>
#include <vector>

#include "src/dimensions.h"
#include "src/event_service.h"
#include "src/glfw3.h"

namespace el {

constexpr uint32_t kDefaultWidth = 800;
constexpr uint32_t kDefaultHeight = 600;

class WindowConfig {
 public:
  auto set_title(std::string_view title) -> WindowConfig& {
    title_ = title;
    return *this;
  }

  auto set_dimensions(Dimensions dims) -> WindowConfig& {
    dimensions_ = dims;
    return *this;
  }

  auto set_event_service(EventService* event_service) -> WindowConfig& {
    event_service_ = event_service;
    return *this;
  }

  [[nodiscard]] auto title() const -> std::string_view { return title_; }
  [[nodiscard]] auto width() const -> uint32_t { return dimensions_.width; }
  [[nodiscard]] auto height() const -> uint32_t { return dimensions_.height; }
  [[nodiscard]] auto event_service() const -> EventService* {
    return event_service_;
  }

 private:
  std::string_view title_;
  Dimensions dimensions_ = {.width = kDefaultWidth, .height = kDefaultHeight};
  EventService* event_service_ = nullptr;
};

class Window {
 public:
  explicit Window(const WindowConfig& config);
  ~Window();

  // Returned strings are owned by the window system and will be free'd.
  auto required_engine_extensions() -> std::vector<const char*>;

  [[nodiscard]] auto shouldClose() const -> bool {
    return glfwWindowShouldClose(window_) != 0;
  }

  [[nodiscard]] auto dimensions() const -> Dimensions;

  auto static Poll() -> void { glfwPollEvents(); }

 private:
  GLFWwindow* window_ = nullptr;
  EventService* event_service_;
};

}  // namespace el
