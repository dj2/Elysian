#include <iostream>

#include "src/engine.h"

auto main() -> int {
  el::engine::ErrorData err_data{
    .cb = [](const el::engine::Error& data) {
      std::cerr << data.message << std::endl;
    },
    .user_data = nullptr,
  };

  el::engine::Device device(el::engine::DeviceConfig()
    .set_app_name("Elysian")
    .set_app_version(0, 1, 0)
    .set_enable_validation()
    .set_error_data(&err_data));

  return 0;
}
