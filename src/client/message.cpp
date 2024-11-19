#include "message.h"

auto Message::operator<=>(Message other) const -> std::strong_ordering
{
  return expiring_time <=> other.expiring_time;
}
