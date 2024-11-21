#pragma once

#include "chrono.h"

struct Message {
  auto operator<=>(Message other) const -> std::strong_ordering;
  Duration expiring_time;
  std::string content;
};
