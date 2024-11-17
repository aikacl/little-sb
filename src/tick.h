#pragma once
#include <compare>
#include <cstdint>

class Tick {
public:
  auto operator<=>(Tick const &other) const;
  auto operator++() -> Tick &;
  auto operator-(Tick const &other) const -> std::ptrdiff_t;

private:
  std::size_t _tick{};
};
