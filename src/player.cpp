#include "player.h"
#include "random.h"

Player::Player() = default;

Player::Player(std::string name, int health, std::pair<int, int> damage_range,
               int defense, int money)
    : _name{std::move(name)}, _health{health}, _damage_range{damage_range},
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
  auto const original{hit_one() - target._defense};
  return std::max(original, 0);
}

[[nodiscard]] auto Player::damage_range() const -> std::pair<int, int>
{
  return _damage_range;
}

auto Player::hit_one() const -> int
{
  auto const damage{
      little_sb::random::uniform(_damage_range.first, _damage_range.second)};
  auto const scale{(1 + (little_sb::random::probability(_critical_hit_rate)
                             ? _critical_hit_scale
                             : 0))};
  auto const scaled_damage{static_cast<float>(damage) * scale};
  return static_cast<int>(scaled_damage);
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
auto Player::critical_hit_rate() const -> float
{
  return _critical_hit_rate;
}
void Player::critical_hit_rate(float rate)
{
  _critical_hit_rate = rate;
}
