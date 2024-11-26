#include "player.h"
#include "random.h"

Player::Player(std::string name, int health, std::pair<int, int> damage_range,
               float critical_hit_rate, float critical_hit_buff, int defense,
               int money, float movement_volecity, float visual_range,
               glm::vec2 position)
    : _name{std::move(name)}, _health{health}, _damage_range{damage_range},
      _critical_hit_rate{critical_hit_rate},
      _critical_hit_buff{critical_hit_buff}, _defense{defense}, _money{money},
      _movement_velocity{movement_volecity}, _visual_range{visual_range},
      _position{position}
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

void Player::heal(int delta)
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

auto Player::damage_range() const -> std::pair<int, int>
{
  return _damage_range;
}

auto Player::generate_damage_from_range() const -> int
{
  return little_sb::random::uniform(_damage_range);
}

auto Player::hit_one() const -> int
{
  auto const damage{generate_damage_from_range()};
  auto const extra{little_sb::random::probability(_critical_hit_rate)
                       ? _critical_hit_buff
                       : 0.0F};
  auto const scale{1 + extra};
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
auto Player::Builder::name(std::string name) -> Player::Builder &
{
  _player->_name = std::move(name);
  return *this;
}

auto Player::Builder::health(int health) -> Player::Builder &
{
  _player->_health = health;
  return *this;
}

auto Player::Builder::damage_range(std::pair<int, int> damage_range)
    -> Player::Builder &
{
  _player->_damage_range = damage_range;
  return *this;
}

auto Player::Builder::critical_hit_rate(float critical_hit_rate)
    -> Player::Builder &
{
  _player->_critical_hit_rate = critical_hit_rate;
  return *this;
}

auto Player::Builder::critical_hit_buff(float critical_hit_scale)
    -> Player::Builder &
{
  _player->_critical_hit_buff = critical_hit_scale;
  return *this;
}

auto Player::Builder::defense(int defense) -> Player::Builder &
{
  _player->_defense = defense;
  return *this;
}

auto Player::Builder::money(int money) -> Player::Builder &
{
  _player->_money = money;
  return *this;
}

auto Player::Builder::build() -> std::unique_ptr<Player>
{
  return std::move(_player);
}

auto Player::Builder::movement_volecity(float movement_volecity)
    -> Player::Builder &
{
  _player->_movement_velocity = movement_volecity;
  return *this;
}

auto Player::Builder::visual_range(float visual_range) -> Player::Builder &
{
  _player->_visual_range = visual_range;
  return *this;
}
auto Player::Builder::position(Vec2 position) -> Player::Builder &
{
  _player->_position = position;
  return *this;
}
auto Player::position() const -> Vec2
{
  return _position;
}
auto Player::can_see(Player const &other) const -> bool
{
  // TODO(shelpam): Terrain effect
  return glm::distance(_position.dir, other._position.dir) <= _visual_range;
}
Player::Builder::Builder() : _player{std::make_unique<Player>()} {}
auto Vec2::x() const -> float
{
  return dir.x;
}

auto Vec2::y() const -> float
{
  return dir.y;
}
void Player::move_direction(Vec2 dir)
{
  _move_direction = dir;
}

void Player::do_move(Duration delta, Game_map const &map)
{
  _position.dir +=
      static_cast<float>(
          std::chrono::duration_cast<std::chrono::duration<float>>(delta)
              .count()) *
      _move_direction.dir;

  // Please revise the position based on the game map.
  // I directly check x and y compared by 0 here, because I know the game map
  // ranges between 0 and height, width.
  if (_position.x() < 0) {
    _position.dir.x = 0;
  }
  if (_position.x() >= map.height()) {
    _position.dir.x = map.height() - 1;
  }
  if (_position.y() < 0) {
    _position.dir.y = 0;
  }
  if (_position.y() >= map.width()) {
    _position.dir.y = map.width() - 1;
  }
}
