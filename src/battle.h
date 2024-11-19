#pragma once

#include "chrono.h"
#include "player.h"
#include "server/session-service.h"
#include "tick.h"
#include <algorithm>
#include <queue>

enum class Stop_cause : std::uint8_t { normal, escaping };

class Battle {
public:
  Battle(std::uint64_t id, std::array<player::Player *, 2> players,
         Session_service *session_service);

  void update(Duration delta);

  void stop(Stop_cause cause);

  [[nodiscard]] auto id() const -> std::uint64_t;

  [[nodiscard]] auto pending_events() -> std::queue<std::string> &;

  [[nodiscard]] auto ended() const -> bool;

private:
  std::size_t _id;
  std::queue<std::string> _pending_events;
  std::array<player::Player *, 2> _players; // First sender, second receiver
  Session_service *_session_service;
  bool _ended{};
  int _turn{};
  std::chrono::nanoseconds _time_since_last_update{};
};
