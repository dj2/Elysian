#include <iostream>

#include "src/engine.h"

auto main() -> int {

  el::engine::DeviceBuilder builder;
  auto device = builder.app_name("Elysian")
      .app_version(0, 1, 0)
      .enable_validation()
      .build();

  return 0;
}
