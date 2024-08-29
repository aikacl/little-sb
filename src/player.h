#pragma once

#include "json.h"
#include <memory>
#include <string>

class Player;
using Player_ptr = std::shared_ptr<Player>;

class Player {
public:
  // Don't use this. This is for json creation.
  Player();

  Player(std::string name, int health, int damage, int defense, int money);

  auto attack(Player &target) const -> int;
  auto take_damage(int damage) -> int;
  // delta can not be negative
  void cure(int delta);

  [[nodiscard]] auto name() const -> std::string const &;
  [[nodiscard]] auto health() const -> int;
  [[nodiscard]] auto damage() const -> int;
  [[nodiscard]] auto defense() const -> int;

  void cost_money(int cost);
  [[nodiscard]] auto money() const -> int;

private:
  [[nodiscard]] auto damage_to(Player const &target) const -> int;

  std::string _name;
  int _health;
  int _damage;
  int _defense;
  int _money;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage, _defense,
                                 _money)
};
