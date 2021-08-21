#pragma once

#include <array>
#include <cstdint>

#define EL_PAD(x) [[maybe_unused]] std::array<uint8_t, x> el_pad##__LINE__ = {}
