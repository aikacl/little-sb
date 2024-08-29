#include "player.h"

Player::Player() = default;

Player::Player(std::string name, int health, int damage, int defense, int money)
    : _name{std::move(name)}, _health{health}, _damage{damage},
      _defense{defense}, _money{money}
{
}

auto Player::attack(Player &target) const -> int
{
  return target.take_damage(damage_to(target));
}

auto Player::take_damage(int damage) -> int
{
  damage = std::min(damage, this->_health);
  this->_health -= damage;
  return damage;
}

void Player::cure(int delta)
{
  assert(delta >= 0);
  _health += delta;
}

auto Player::name() const -> std::string const &
{
  return _name;
}

auto Player::health() const -> int
{
  return _health;
}

auto Player::damage_to(Player const &target) const -> int
{
  return std::max(_damage - target._defense, 0);
}

auto Player::damage() const -> int
{
  return _damage;
}

auto Player::defense() const -> int
{
  return _defense;
}

void Player::cost_money(int cost)
{
  _money -= cost;
}

auto Player::money() const -> int
{
  return _money;
}
