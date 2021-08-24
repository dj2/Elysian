#include <iostream>

import dimensions;
import engine;
import event_service;
import window;

auto main() -> int {
  el::Window window(el::WindowConfig().set_title("Elysian").set_dimensions(
      {.width = 1024, .height = 768}));

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
          .set_device_extensions(window.required_engine_extensions())
          .set_dimensions_cb(
              [&window]() -> el::Dimensions { return window.dimensions(); }));

  window.add_event_listener(
      el::EventType::kResized,
      [&device](const el::Event* /*evt*/) -> void { device.set_resized(); });

  while (!window.shouldClose()) {
    el::Window::Poll();
  }

  return 0;
}
