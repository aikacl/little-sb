#include "player.h"
#include "random.h"

player_stuff::Player::Player(std::string name, int health,
                             std::pair<int, int> damage_range,
                             float critical_hit_rate, float critical_hit_buff,
                             int defense, int money)
    : _name{std::move(name)}, _health{health}, _damage_range{damage_range},
      _critical_hit_rate{critical_hit_rate},
      _critical_hit_buff{critical_hit_buff}, _defense{defense}, _money{money}
{
}

auto player_stuff::Player::attack(player_stuff::Player &target) const -> int
{
  return target.take_damage(damage_to(target));
}

auto player_stuff::Player::take_damage(int damage) -> int
{
  damage = std::min(damage, this->_health);
  this->_health -= damage;
  return damage;
}

void player_stuff::Player::cure(int delta)
{
  assert(delta >= 0);
  _health += delta;
}

auto player_stuff::Player::name() const -> std::string const &
{
  return _name;
}

auto player_stuff::Player::health() const -> int
{
  return _health;
}

auto player_stuff::Player::damage_to(player_stuff::Player const &target) const
    -> int
{
  auto const original{hit_one() - target._defense};
  return std::max(original, 0);
}

auto player_stuff::Player::damage_range() const -> std::pair<int, int>
{
  return _damage_range;
}

auto player_stuff::Player::generate_damage_from_range() const -> int
{
  return little_sb::random::uniform(_damage_range.first, _damage_range.second);
}

auto player_stuff::Player::hit_one() const -> int
{
  auto const damage{generate_damage_from_range()};
  auto const extra{little_sb::random::probability(_critical_hit_rate)
                       ? _critical_hit_buff
                       : 0.0F};
  auto const scale{1 + extra};
  auto const scaled_damage{static_cast<float>(damage) * scale};
  return static_cast<int>(scaled_damage);
}

auto player_stuff::Player::defense() const -> int
{
  return _defense;
}

void player_stuff::Player::cost_money(int cost)
{
  _money -= cost;
}

auto player_stuff::Player::money() const -> int
{
  return _money;
}
auto player_stuff::Player::critical_hit_rate() const -> float
{
  return _critical_hit_rate;
}
void player_stuff::Player::critical_hit_rate(float rate)
{
  _critical_hit_rate = rate;
}
void player_stuff::Builder::name(std::string name)
{
  _name = std::move(name);
}

void player_stuff::Builder::health(int health)
{
  _health = health;
}

void player_stuff::Builder::damage_range(std::pair<int, int> damage_range)
{
  _damage_range = damage_range;
}

void player_stuff::Builder::critical_hit_rate(float critical_hit_rate)
{
  _critical_hit_rate = critical_hit_rate;
}

void player_stuff::Builder::critical_hit_buff(float critical_hit_scale)
{
  _critical_hit_buff = critical_hit_scale;
}

void player_stuff::Builder::defense(int defense)
{
  _defense = defense;
}

void player_stuff::Builder::money(int money)
{
  _money = money;
}

auto player_stuff::Builder::build() const -> player_stuff::Player
{
  return player_stuff::Player{
      _name,    _health, _damage_range, _critical_hit_rate, _critical_hit_buff,
      _defense, _money};
}

player_stuff::Classic_builder::Classic_builder(std::string player_name)
{
  name(std::move(player_name));
  health(little_sb::random::uniform(20, 25));
  damage_range({3, little_sb::random::uniform(3, 7)});
  critical_hit_rate(little_sb::random::uniform(0.15, 0.2));
  critical_hit_buff(1.5);
  defense(little_sb::random::uniform(1, 5));
  money(20);
}
