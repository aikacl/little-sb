#pragma once

#include "json.h"
#include <cstdint>
#include <memory>
#include <string>

class Player;
using Player_ptr = std::shared_ptr<Player>;

class Player {
public:
  // Don't use this. This is for json creation.
  Player();

  Player(std::string name, int health, int damage, int defense);

  auto attack(Player &target) const -> int;
  auto take_damage(std::int32_t damage) -> int;

  [[nodiscard]] auto name() const -> std::string const &;
  [[nodiscard]] auto health() const -> int;
  [[nodiscard]] auto damage() const -> int;
  [[nodiscard]] auto defense() const -> int;

private:
  [[nodiscard]] auto damage_to(Player const &target) const -> std::int32_t;

  std::string _name;
  std::int32_t _health;
  std::int32_t _damage;
  std::int32_t _defense;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage, _defense)
};
