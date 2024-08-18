#pragma once

#include "random.h"
#include <cstdint>
#include <string>

class Player {
public:
  Player(std::string name) : _name(std::move(name)) {}

  [[nodiscard]] auto attack(Player &target) const -> std::uint32_t
  {
    auto const damage{damage_to(target)};
    target._health -= std::min(target._health, damage);
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
  [[nodiscard]] static auto damage_to(Player const &_) -> std::uint32_t
  {
    return little_sb::random::uniform(1, 10);
  }

  std::string _name;
  std::uint32_t _health{20};
};
