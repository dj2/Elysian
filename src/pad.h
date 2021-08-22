#pragma once

#include <array>
#include <cstdint>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LINE std::source_location::current().line
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EL_PAD(x) std::array<uint8_t, x> el_pad_##LINE = {}
