#include <iostream>

#include "src/dimensions.h"
#include "src/engine.h"
#include "src/event_service.h"
#include "src/window.h"

constexpr uint32_t kDefaultWidth = 1024;
constexpr uint32_t kDefaultHeight = 768;

auto main() -> int {
  el::EventService event_service;

  el::Window window(
      el::WindowConfig()
          .set_title("Elysian")
          .set_dimensions({.width = kDefaultWidth, .height = kDefaultHeight})
          .set_event_service(&event_service));

  el::engine::ErrorData err_data{
      .cb =
          [](const el::engine::Error& data) {
            std::cerr << data.message << std::endl;
          },
      .user_data = nullptr,
  };

  el::engine::Device device(
      el::engine::DeviceConfig()
          .set_app_name("Elysian")
          .set_app_version(0, 1, 0)
          .set_enable_validation()
          .set_error_data(&err_data)
          .set_device_extensions(el::Window::required_engine_extensions())
          .set_event_service(&event_service)
          .set_dimensions_cb(
              [&window]() -> el::Dimensions { return window.dimensions(); })
          .set_surface_cb(
              [&window](el::engine::Device& d) { window.create_surface(d); }));

  while (!window.shouldClose()) {
    el::Window::Poll();
  }

  return 0;
}
