#pragma once

#include "random.h"
#include <cstdint>
#include <string>

class Player {
public:
	explicit Player(std::string_view const name) : _name{ name } {}

	[[nodiscard]] auto attack(Player& target) const -> std::uint32_t
	{
		return target.take_damage(damage_to(target));
	}

	[[nodiscard]] auto name() const
	{
		return _name;
	}

	[[nodiscard]] auto health() const
	{
		return _health;
	}

	auto take_damage(std::uint32_t  damage) -> std::uint32_t {
		damage = std::min(damage, this->_health);
		this->_health -= damage;
		return damage;
	}

private:
	[[nodiscard]] auto damage_to(Player const& _) const -> std::uint32_t
	{
		return little_sb::random::uniform(1, 10);
	}

	std::string _name;
	std::uint32_t _health{ 20 };
};
