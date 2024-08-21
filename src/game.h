#pragma once

#include "player.h"
#include <algorithm>
#include <format>
#include <queue>

class Game {
public:
  Game(std::uint64_t const id, std::array<Player, 2> players)
      : _id{id}, _players{std::move(players)}
  {
  }

  void tick()
  {
    if (std::ranges::all_of(
            _players, [](auto &player) { return player.health() != 0; })) {
      auto const &attacker{_players.at(_turn % 2)};
      auto &target{_players.at((_turn % 2) ^ 1)};

      attacker.attack(target);

      _pending_events.push(
          std::format("{} {}", target.name(), target.health()));

      ++_turn;
    }
    else {
      _is_ended = true;
    }
  }

  [[nodiscard]] auto id() const -> std::uint64_t
  {
    return _id;
  }

  [[nodiscard]] auto pending_events() -> std::queue<std::string> &
  {
    return _pending_events;
  }

  [[nodiscard]] auto is_ended() const -> bool
  {
    return _is_ended;
  }

private:
  std::uint64_t _id;
  std::queue<std::string> _pending_events;
  std::array<Player, 2> _players;
  bool _is_ended{};
  int _turn{};
};
