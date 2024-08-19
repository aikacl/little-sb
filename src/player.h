#pragma once

#include "random.h"
#include <cstdint>
#include <string>

class Player {
public:
  explicit Player(std::string_view const name) : _name{name} {}

  [[nodiscard]] auto attack(Player &target) const -> std::uint32_t
  {
    auto const damage{std::min(target._health, damage_to(target))};
    target._health -= damage;
    return damage;
  }

  [[nodiscard]] auto name() const
  {
    return _name;
  }

  [[nodiscard]] auto health() const
  {
    return _health;
  }

private:
  [[nodiscard]] auto damage_to(Player const &_) const -> std::uint32_t
  {
    return little_sb::random::uniform(1, 10);
  }

  std::string _name;
  std::uint32_t _health{20};
};
