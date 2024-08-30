#pragma once

#include "json.h"
#include <memory>
#include <string>

namespace player_stuff {
class Player;
using Player_ptr = std::shared_ptr<Player>;

class Player {
  friend class Builder;

public:
  Player() = default;

  // Don't use this because it is here for json creation. Player();
  auto attack(Player &target) const -> int;
  auto take_damage(int damage) -> int;
  // delta can not be negative
  void cure(int delta);

  [[nodiscard]] auto name() const -> std::string const &;
  [[nodiscard]] auto health() const -> int;
  [[nodiscard]] auto damage_range() const -> std::pair<int, int>;
  [[nodiscard]] auto generate_damage_from_range() const -> int;

  void critical_hit_rate(float rate);
  [[nodiscard]] auto critical_hit_rate() const -> float;

  [[nodiscard]] auto hit_one() const -> int;
  [[nodiscard]] auto defense() const -> int;

  void cost_money(int cost);
  [[nodiscard]] auto money() const -> int;

private:
  Player(std::string name, int health, std::pair<int, int> damage_range,
         float critical_hit_rate, float critical_hit_buff, int defense,
         int money);

  [[nodiscard]] auto damage_to(Player const &target) const -> int;

  std::string _name;
  int _health;

  std::pair<int, int> _damage_range;
  float _critical_hit_rate;
  float _critical_hit_buff;

  int _defense;
  int _money;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage_range,
                                 _critical_hit_rate, _critical_hit_buff,
                                 _defense, _money)
};

class Builder {
public:
  void name(std::string name);
  void health(int health);
  void damage_range(std::pair<int, int> damage_range);
  void critical_hit_rate(float critical_hit_rate);
  void critical_hit_buff(float critical_hit_scale);
  void defense(int defense);
  void money(int money);

  [[nodiscard]] auto build() const -> Player;

private:
  std::string _name;
  int _health;

  std::pair<int, int> _damage_range;
  float _critical_hit_rate;
  float _critical_hit_buff;

  int _defense;
  int _money;
};

class Classic_builder : public Builder {
public:
  Classic_builder(std::string player_name);
};

} // namespace player_stuff
