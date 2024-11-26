#pragma once

#include "chrono.h"
#include "game-map.h"
#include "item/effect.h"
#include "json.h"
#include "uuid.h"
#include "value-modification.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

struct Vec2 {
  template <typename... Args> Vec2(Args... args) : dir{args...} {}
  [[nodiscard]] auto x() const -> float;
  [[nodiscard]] auto y() const -> float;

  glm::vec2 dir;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Vec2, dir.x, dir.y)
};

class Player;
using Damage_range = std::pair<int, int>;

class Player {
  friend class item::EnhancementEffect;

public:
  class Builder {
  public:
    Builder();

    auto name(std::string name) -> Builder &;
    auto health(int health) -> Builder &;
    auto damage_range(std::pair<int, int> damage_range) -> Builder &;
    auto critical_hit_rate(float critical_hit_rate) -> Builder &;
    auto critical_hit_buff(float critical_hit_scale) -> Builder &;
    auto defense(int defense) -> Builder &;
    auto money(int money) -> Builder &;
    auto movement_volecity(float movement_volecity) -> Builder &;
    auto visual_range(float visual_range) -> Builder &;
    auto position(Vec2 position) -> Builder &;

    auto build() -> std::unique_ptr<Player>;

  private:
    std::unique_ptr<Player> _player;
  };

  Player() = default; // Comforms json.

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
  void heal(int delta);

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

  [[nodiscard]] auto position() const -> Vec2;
  void do_move(Duration delta, Game_map const &map);
  void move_direction(Vec2 dir);

  [[nodiscard]] auto can_see(Player const &other) const -> bool;

private:
  Player(std::string name, int health, Damage_range damage_range,
         float critical_hit_rate, float critical_hit_buff, int defense,
         int money, float movement_volecity, float visual_range,
         glm::vec2 position);

  [[nodiscard]] auto damage_to(Player const &target) const -> int;

  std::string _name;
  int _health{};

  std::pair<int, int> _damage_range;

  float _critical_hit_rate{};
  float _critical_hit_buff{};

  int _defense{};
  int _money{};

  float _movement_velocity{};
  float _visual_range{};

  Vec2 _position{};
  Vec2 _move_direction{};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Player, _name, _health, _damage_range,
                                 _critical_hit_rate, _critical_hit_buff,
                                 _defense, _money, _movement_velocity,
                                 _visual_range, _position, _move_direction)
};
