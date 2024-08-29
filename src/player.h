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

  Player(std::string name, int health, std::pair<int, int> damage_range,
         int defense, int money);

  auto attack(Player &target) const -> int;
  auto take_damage(int damage) -> int;
  // delta can not be negative
  void cure(int delta);

  [[nodiscard]] auto name() const -> std::string const &;
  [[nodiscard]] auto health() const -> int;
  [[nodiscard]] auto damage_range() const -> std::pair<int, int>;

  void critical_hit_rate(float rate);
  [[nodiscard]] auto critical_hit_rate() const -> float;

  [[nodiscard]] auto hit_one() const -> int;
  [[nodiscard]] auto defense() const -> int;

  void cost_money(int cost);
  [[nodiscard]] auto money() const -> int;

private:
  [[nodiscard]] auto damage_to(Player const &target) const -> int;

  std::string _name;
  int _health;

  std::pair<int, int> _damage_range;
  float _critical_hit_rate{0.15};
  float _critical_hit_scale{1.5};

  int _defense;
  int _money;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage_range,
                                 _critical_hit_rate, _critical_hit_scale,
                                 _defense, _money)
};
