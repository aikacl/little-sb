#pragma once

#include "player.h"
#include "server/session-service.h"
#include <algorithm>
#include <queue>

class Game {
public:
  Game(std::uint64_t const id, std::array<Player *, 2> players,
       Session_service *session_service)
      : _id{id}, _players{players}, _session_service{session_service},
        _last_tick{std::chrono::steady_clock::now()}
  {
  }

  void tick()
  {
    auto now{std::chrono::steady_clock::now()};
    if (now - _last_tick < std::chrono::milliseconds{500}) {
      return;
    }
    _last_tick = now;

    if (_ended) {
      return;
    }

    if (std::ranges::all_of(
            _players, [](auto &player) { return player->health() != 0; })) {
      auto const &attacker{_players.at(_turn % 2)};
      auto &target{_players.at((_turn % 2) ^ 1)};

      auto const drop{attacker->attack(*target)};

      Event health_drop{"health-drop"};
      health_drop.set_param("player", target->name());
      health_drop.set_param("drop", drop);
      _session_service->push_event(attacker->name(), health_drop);
      _session_service->push_event(target->name(), health_drop);

      ++_turn;
    }
    else {
      Event game_end{"game-end"};
      _session_service->push_event(_players[0]->name(), game_end);
      Event message("message");
      message.add_arg("game-end");
      _session_service->push_event(_players[0]->name(), message);
      _ended = true;
    }
  }

  [[nodiscard]] auto id() const -> std::size_t
  {
    return _id;
  }

  [[nodiscard]] auto pending_events() -> std::queue<std::string> &
  {
    return _pending_events;
  }

  [[nodiscard]] auto ended() const -> bool
  {
    return _ended;
  }

private:
  std::size_t _id;
  std::queue<std::string> _pending_events;
  std::array<Player *, 2> _players; // First sender, second receiver
  Session_service *_session_service;
  bool _ended{};
  int _turn{};
  decltype(std::chrono::steady_clock::now()) _last_tick;
};
