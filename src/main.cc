#include <iostream>

#include "src/engine.h"

auto main() -> int {

  Device device(el::engine::DeviceBuilder()
    .set_app_name("Elysian")
    .set_app_version(0, 1, 0)
    .set_enable_validation());

  return 0;
}
