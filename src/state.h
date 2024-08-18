#pragma once

#include <cstdint>

enum class State : std::uint8_t {
  greeting,
  starting,
  running,
  ended,
  should_terminate,
};
