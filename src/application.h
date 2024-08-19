#pragma once

#include "player.h"
#include "session.h"
#include "state.h"
#include <iostream>
#include <print>
#include <string>

class Application {
public:
  Application(Session *client) : _session{client} {}

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
    _state = State::Starting;
    initialize_players();
  }

  void initialize_players()
  {
    _players.assign({Player{"You"}, Player{"The enemy"}});
  }

  void tick()
  {
    if (_state == State::Greeting) {
      auto const response{_session->request<std::string>("Login")};
      std::println("Connected to the server: {}", response);

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
    else if (_state == State::Starting) {
      std::println("Game started. The following is the health of players:");
      for (auto const &player : _players) {
        std::println("{} has {} health.", player.name(), player.health());
      }
      std::println("");
      _state = State::Running;
    }
    else if (_state == State::Running) {
      /*std::this_thread::sleep_for(std::chrono::milliseconds{3000});*/
      /*auto const &attacker{_players[_acting_player]};*/
      /*auto &receiver{_players[_acting_player ^ 1]};*/
      /*auto const damage{attacker.attack(receiver)};*/
      /*std::println("{} gives {} damage to {}!", attacker.name(), damage,*/
      /*             receiver.name());*/
      /**/
      /*if (receiver.health() == 0) {*/
      /*  std::println("{} has died. Game over!", receiver.name());*/
      /*  _state = State::Ended;*/
      /*}*/
      /*else {*/
      /*  std::println("Now you have {} health remaining.",
       * _players[0].health());*/
      /*  std::println("And the enemy has {} health remaining.",*/
      /*               _players[1].health());*/
      /*  std::println("");*/
      /*}*/

      /*_acting_player ^= 1;*/
    }
    else if (_state == State::Ended) {
      std::print("Continue? y/[n]: ");
      std::string choice;
      std::getline(std::cin, choice);
      if (choice == "y") {
        start_new_game();
      }
      else {
        _state = State::Should_terminate;
      }
    }
    else if (_state == State::Should_terminate) {
      if (auto const response{_session->request<std::string>("Logout")};
          response !=
          std::format("Ok, {} logged out.", _session->player_name())) {
        std::println("Failed to logout from the server, '{}' received",
                     response);
      }
    }
  }

  [[nodiscard]] auto should_terminate() const -> bool
  {
    return _state == State::Should_terminate;
  }

  Session *_session;
  State _state{State::Greeting};
  std::string player_name;
  std::vector<Player> _players;
};
