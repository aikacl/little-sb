#include "player.h"
#include "random.h"

player::Player::Player(std::string name, int health,
                       std::pair<int, int> damage_range,
                       float critical_hit_rate, float critical_hit_buff,
                       int defense, int money, double movement_volecity,
                       double visual_range, glm::vec2 position)
    : _name{std::move(name)}, _health{health}, _damage_range{damage_range},
      _critical_hit_rate{critical_hit_rate},
      _critical_hit_buff{critical_hit_buff}, _defense{defense}, _money{money},
      _movement_volecity{movement_volecity}, _visual_range{visual_range},
      _position{position}
{
}

auto player::Player::attack(player::Player &target) const -> int
{
  return target.take_damage(damage_to(target));
}

auto player::Player::take_damage(int damage) -> int
{
  damage = std::min(damage, this->_health);
  this->_health -= damage;
  return damage;
}

void player::Player::cure(int delta)
{
  assert(delta >= 0);
  _health += delta;
}

auto player::Player::name() const -> std::string const &
{
  return _name;
}

auto player::Player::health() const -> int
{
  return _health;
}

auto player::Player::damage_to(player::Player const &target) const -> int
{
  auto const original{hit_one() - target._defense};
  return std::max(original, 0);
}

auto player::Player::damage_range() const -> std::pair<int, int>
{
  return _damage_range;
}

auto player::Player::generate_damage_from_range() const -> int
{
  auto effect_on{[this](int value) {
    for (auto const &[_, v] : _damage_amplification) {
      value *= v;
    }
    for (auto const &[_, v] : _damage_addition) {
      value += v;
    }
    return value;
  }};
  return little_sb::random::uniform(effect_on(_damage_range.first),
                                    effect_on(_damage_range.second));
}

auto player::Player::hit_one() const -> int
{
  auto const damage{generate_damage_from_range()};
  auto const extra{little_sb::random::probability(_critical_hit_rate)
                       ? _critical_hit_buff
                       : 0.0F};
  auto const scale{1 + extra};
  auto const scaled_damage{static_cast<float>(damage) * scale};
  return static_cast<int>(scaled_damage);
}

auto player::Player::defense() const -> int
{
  return _defense;
}

void player::Player::cost_money(int cost)
{
  _money -= cost;
}

auto player::Player::money() const -> int
{
  return _money;
}
auto player::Player::critical_hit_rate() const -> float
{
  return _critical_hit_rate;
}
void player::Player::critical_hit_rate(float rate)
{
  _critical_hit_rate = rate;
}
void player::Builder::name(std::string name)
{
  _name = std::move(name);
}

void player::Builder::health(int health)
{
  _health = health;
}

void player::Builder::damage_range(std::pair<int, int> damage_range)
{
  _damage_range = damage_range;
}

void player::Builder::critical_hit_rate(float critical_hit_rate)
{
  _critical_hit_rate = critical_hit_rate;
}

void player::Builder::critical_hit_buff(float critical_hit_scale)
{
  _critical_hit_buff = critical_hit_scale;
}

void player::Builder::defense(int defense)
{
  _defense = defense;
}

void player::Builder::money(int money)
{
  _money = money;
}

auto player::Builder::build() const -> player::Player
{
  return player::Player{_name,
                        _health,
                        _damage_range,
                        _critical_hit_rate,
                        _critical_hit_buff,
                        _defense,
                        _money,
                        _movement_volecity,
                        _visual_range,
                        _position};
}

player::Classic_builder::Classic_builder(std::string player_name)
{
  name(std::move(player_name));
  health(little_sb::random::uniform(2000, 3000));
  {
    auto d1{little_sb::random::uniform(80, 100)};
    auto d2{little_sb::random::uniform(80, 100)};
    if (d1 > d2) {
      std::swap(d1, d2);
    }
    damage_range({d1, d2});
  }
  critical_hit_rate(little_sb::random::uniform(0.3, 0.5));
  critical_hit_buff(1.5);
  defense(little_sb::random::uniform(20, 30));
  money(20);
  movement_volecity(1);
  visual_range(10);
  position(
      {little_sb::random::uniform(0, 9), little_sb::random::uniform(0, 19)});
}

void player::Builder::movement_volecity(double movement_volecity)
{
  _movement_volecity = movement_volecity;
}

void player::Builder::visual_range(double visual_range)
{
  _visual_range = visual_range;
}
void player::Builder::position(glm::vec2 position)
{
  _position = position;
}
auto player::Player::position() const -> glm::vec2
{
  return _position;
}
auto player::Player::can_see(Player const &other) const -> bool
{
  // TODO(shelpam): Terrain effect
  return glm::distance(_position, other._position) <= _visual_range;
}
