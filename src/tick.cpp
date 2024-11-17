#include "tick.h"

auto Tick::operator<=>(Tick const &other) const
{
  return _tick <=> other._tick;
}

auto Tick::operator++() -> Tick &
{
  ++_tick;
  return *this;
}

auto Tick::operator-(Tick const &other) const -> std::ptrdiff_t
{
  return static_cast<std::ptrdiff_t>(_tick) -
         static_cast<std::ptrdiff_t>(other._tick);
}
