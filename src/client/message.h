#pragma once

#include "chrono.h"

struct Message {
  auto operator<=>(Message other) const -> std::strong_ordering;
  std::chrono::time_point<std::chrono::system_clock> created_time;
  Duration expiring_time;
  std::string content;
};
