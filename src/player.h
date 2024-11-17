#pragma once

#include "item/effect.h"
#include "json.h"
#include "uuid.h"
#include "value-modification.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace player {
class Player;
using Player_ptr = std::shared_ptr<Player>;
using Damage_range = std::pair<int, int>;

class Player {
  friend class Builder;
  friend class item::EnhancementEffect;

public:
  Player() = default;

  void add_effect(std::shared_ptr<item::Effect> const &e)
  {
    e->perform(*this);
  }

  void remove_effect(std::shared_ptr<item::Effect> const &e)
  {
    e->deperform(*this);
  }

  // Don't use this because it is here for json creation. Player();
  auto attack(Player &target) const -> int;
  auto take_damage(int damage) -> int;
  // delta can not be negative
  void cure(int delta);

  [[nodiscard]] auto name() const -> std::string const &;
  [[nodiscard]] auto health() const -> int;
  [[nodiscard]] auto damage_range() const -> Damage_range;
  [[nodiscard]] auto generate_damage_from_range() const -> int;

  void critical_hit_rate(float rate);
  [[nodiscard]] auto critical_hit_rate() const -> float;

  [[nodiscard]] auto hit_one() const -> int;
  [[nodiscard]] auto defense() const -> int;

  void cost_money(int cost);
  [[nodiscard]] auto money() const -> int;

  [[nodiscard]] auto position() const -> glm::vec2;

  [[nodiscard]] auto can_see(Player const &other) const -> bool;

private:
  Player(std::string name, int health, Damage_range damage_range,
         float critical_hit_rate, float critical_hit_buff, int defense,
         int money, double movement_volecity, double visual_range,
         glm::vec2 position);

  [[nodiscard]] auto damage_to(Player const &target) const -> int;

  std::string _name;
  int _health;

  std::pair<int, int> _damage_range;
  std::map<std::shared_ptr<item::Effect>, double> _damage_amplification;
  std::map<std::shared_ptr<item::Effect>, int> _damage_addition;

  float _critical_hit_rate;
  float _critical_hit_buff;

  int _defense;
  int _money;

  double _movement_volecity;
  double _visual_range;

  glm::vec2 _position;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage_range,
                                 _critical_hit_rate, _critical_hit_buff,
                                 _defense, _money, _movement_volecity,
                                 _visual_range, _position.x, _position.y)
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
  void movement_volecity(double movement_volecity);
  void visual_range(double visual_range);
  void position(glm::vec2 position);

  [[nodiscard]] auto build() const -> Player;

private:
  std::string _name;
  int _health;

  std::pair<int, int> _damage_range;
  float _critical_hit_rate;
  float _critical_hit_buff;

  int _defense;
  int _money;

  double _movement_volecity;
  double _visual_range;

  glm::vec2 _position;
};

class Classic_builder : public Builder {
public:
  explicit Classic_builder(std::string player_name);
};

} // namespace player
