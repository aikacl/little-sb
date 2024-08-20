#pragma once

#include <cstdint>

enum class State : std::uint8_t {
  Greeting,
  Starting,
  Running,
  Ended,
  Should_stop,
};
