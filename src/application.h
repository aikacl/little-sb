#pragma once

#include "session.h"
#include "split-by.h"
#include "state.h"
#include <iostream>
#include <print>
#include <string>

class Application {
public:
  Application(std::string_view const hostname, std::uint16_t const port,
              std::string_view const player_name)
      : _session{_io_context,
                 tcp::resolver{_io_context}.resolve(hostname,
                                                    std::to_string(port)),
                 player_name},
        _player_name{player_name}
  {
    run();
  }

private:
  void run()
  {
    while (!should_terminate()) {
      tick();
    }
  }

  void tick()
  {
    if (_state == State::Greeting) {
      auto const response{_session.request<std::string>("Login")};
      std::println("Connected to the server: {}", response);

      std::println("Type \"start\" to begin the game!");
      std::string choice;
      std::cin >> choice;
      std::cin.ignore();
      if (choice == "start") {
        start_new_game(_game_id);
      }
      else {
        std::println("Unknown command '{}'", choice);
      }
    }
    else if (_state == State::Starting) {
      /*std::println("Game started. The following is the health of players:");*/
      /*for (auto const &player : _players) {*/
      /*  std::println("{} has {} health.", player.name(), player.health());*/
      /*}*/
      /*std::println("");*/
      _state = State::Running;
    }
    else if (_state == State::Running) {
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      poll_events();
    }
    else if (_state == State::Ended) {
      std::print("Continue? y/[n]: ");
      std::string choice;
      std::getline(std::cin, choice);
      if (choice == "y") {
        start_new_game(_game_id);
      }
      else {
        _state = State::Should_terminate;
      }
    }
    else if (_state == State::Should_terminate) {
      if (auto const response{_session.request<std::string>("Logout")};
          response !=
          std::format("Ok, {} logged out.", _session.player_name())) {
        std::println("Failed to logout from the server, '{}' received",
                     response);
      }
    }
  }

  void start_new_game(std::uint64_t &game_id)
  {
    std::vector<std::string> parts;

    for (;;) {
      std::println("Players: {}",
                   _session.request<std::string>("list-players"));
      std::println("Please input the component:");
      std::string choice;
      std::getline(std::cin, choice);
      auto const response{
          _session.request<std::string>(std::format("battle {}", choice))};
      parts = split_by(response);
      if (parts[0] == "ok") {
        break;
      }
      std::println("{}", response);
    }

    game_id = std::stoull(parts[1]);
    poll_events();
    _state = State::Starting;
  }

  void poll_events()
  {
    for (;;) {
      auto const event{_session.request<std::string>(
          std::format("query event {}", _game_id))};

      if (event == "no") {
        break;
      }

      auto const parts{split_by(event)};
      std::println("{}", event);

      if (event == "ended") {
        /*std::println("[debug] Triggering game ended");*/
        _state = State::Ended;
        break;
      }
    }
  }

  [[nodiscard]] auto should_terminate() const -> bool
  {
    return _state == State::Should_terminate;
  }

  asio::io_context _io_context;
  Session _session;
  State _state{State::Greeting};
  std::string _player_name;
  std::uint64_t _game_id{};
};
