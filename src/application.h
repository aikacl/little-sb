#pragma once

#include "player.h"
#include "state.h"
#include <iostream>
#include <print>
#include <string>
#include <thread>

class Application {
public:
  [[nodiscard]] auto run() -> int
  {
    while (!should_terminate()) {
      tick();
    }
    return 0;
  }

private:
  void start_new_game()
  {
    _state = State::starting;
    initialize_players();
  }

  void initialize_players()
  {
    _players.assign({{"You"}, {"The enemy"}});
  }

  void tick()
  {
    if (_state == State::greeting) {
      std::println("Type \"start\" to begin the game!");
      std::string choice;
      std::cin >> choice;
      std::cin.ignore();
      if (choice == "start") {
        start_new_game();
      }
      else {
        std::println("Unknown command '{}'", choice);
      }
    }
    else if (_state == State::starting) {
      std::println("Game started. The following is the health of players:");
      for (auto const &player : _players) {
        std::println("{} has {} health.", player.name(), player.health());
      }
      std::println("");
      _state = State::running;
    }
    else if (_state == State::running) {
      std::this_thread::sleep_for(std::chrono::milliseconds{3000});
      auto const &attacker{_players[_acting_player]};
      auto &receiver{_players[_acting_player ^ 1]};
      auto const damage{attacker.attack(receiver)};
      std::println("{} gives {} damage to {}!", attacker.name(), damage,
                   receiver.name());

      if (receiver.health() == 0) {
        std::println("{} has died. Game over!", receiver.name());
        _state = State::ended;
      }
      else {
        std::println("Now you have {} health remaining.", _players[0].health());
        std::println("And the enemy has {} health remaining.",
                     _players[1].health());
        std::println("");
      }

      _acting_player ^= 1;
    }
    else if (_state == State::ended) {
      std::print("Continue? y/[n]: ");
      std::string choice;
      std::getline(std::cin, choice);
      if (choice == "y") {
        start_new_game();
      }
      else {
        _state = State::should_terminate;
      }
    }
    else if (_state == State::should_terminate) {
      // Do NOTHING.
    }
  }

  [[nodiscard]] auto should_terminate() const -> bool
  {
    return _state == State::should_terminate;
  }

  State _state{State::greeting};
  int _acting_player{};
  std::vector<Player> _players;
};
