#include "player.h"

Player::Player() = default;

Player::Player(std::string name, int health, int damage, int defense)
    : _name{std::move(name)}, _health{health}, _damage{damage},
      _defense{defense}
{
}

auto Player::attack(Player &target) const -> int
{
  return target.take_damage(damage_to(target));
}

auto Player::take_damage(std::int32_t damage) -> int
{
  damage = std::min(damage, this->_health);
  this->_health -= damage;
  return damage;
}

auto Player::name() const -> std::string const &
{
  return _name;
}

auto Player::health() const -> std::int32_t
{
  return _health;
}

auto Player::damage_to(Player const &target) const -> int
{
  return std::max(_damage - target._defense, 1);
}

auto Player::damage() const -> int
{
  return _damage;
}

auto Player::defense() const -> int
{
  return _defense;
}
