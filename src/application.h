#pragma once

#include "session.h"
#include "split-by.h"
#include "state.h"
#include <iostream>
#include <print>
#include <string>

class Application {
public:
  Application(std::string_view const host, std::uint16_t const port,
              std::string_view const player_name)
      : _subscribing_session{std::make_shared<Session>(connect(host, port))},
        _request_session{std::make_shared<Session>(connect(host, port))},
        _state{State::Greeting}, _player_name{player_name}
  {
    spdlog::set_level(spdlog::level::info);
    spdlog::trace("Call {}", std::source_location::current().function_name());

    run();
  }

private:
  auto connect(std::string_view const host,
               std::uint16_t const port) -> tcp::socket
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    tcp::socket socket{_io_context};
    auto const endpoints{
        tcp::resolver{_io_context}.resolve(host, std::to_string(port))};
    std::error_code ec;
    asio::connect(socket, endpoints, ec);
    handle_error(ec);
    return socket;
  }

  auto request(std::string_view const req) -> std::string
  {
    return _request_session->request<std::string>(
        Sb_packet{Sb_packet_sender{.type = Sb_packet_sender::Type::Client,
                                   .name{_player_name}},
                  Sb_packet_type::Message, req});
  }

  [[nodiscard]] auto should_stop() const -> bool
  {
    return _state == State::Should_stop;
  }

  void run()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    std::thread{[this]() {
      _subscribing_session->start([](Sb_packet const &packet) {
        spdlog::info("Server message: {}", packet.body.to_string());
        return true;
      });
    }}.detach();

    // The order of the following two lines can not be inversed.
    _subscribing_session->write(Sb_packet{
        Sb_packet_sender{Sb_packet_sender::Type::Client, _player_name},
        Sb_packet_type::Login, "Subscribe"});

    spdlog::info(
        "Connected to the server: {}",
        _request_session->request<std::string>(Sb_packet{
            Sb_packet_sender{Sb_packet_sender::Type::Client, _player_name},
            Sb_packet_type::Login, "Request"}));

    while (!should_stop()) {
      tick();
    }
  }

  void tick()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    switch (_state) {
    case State::Greeting:
      handle_greeting();
      break;
    case State::Starting:
      handle_starting();
      break;
    case State::Running:
      handle_running();
      break;
    case State::Ended:
      handle_ended();
      break;
    default: // Unreachable
      break;
    }
  }

  void handle_greeting()
  {
    spdlog::info("Type \"start\" to begin the game!");
    std::string choice;
    std::cin >> choice;
    std::cin.ignore();
    if (choice == "start") {
      start_new_game(_game_id);
    }
    else {
      spdlog::info("Unknown command '{}'", choice);
    }
  }

  void handle_starting()
  {
    /*std::println("Game started. The following is the health of players:");*/
    /*for (auto const &player : _players) {*/
    /*  std::println("{} has {} health.", player.name(), player.health());*/
    /*}*/
    /*std::println("");*/
    _state = State::Running;
  }

  void handle_running()
  {
    constexpr std::chrono::milliseconds waiting{300};
    std::this_thread::sleep_for(waiting);
    poll_events();
  }

  void poll_events()
  {
    for (;;) {
      auto const event{request(std::format("query event {}", _game_id))};

      if (event == "no") {
        break;
      }

      auto const parts{split_by(event)};
      spdlog::info("Event: {}", event);

      if (event == "ended") {
        _state = State::Ended;
        break;
      }
    }
  }

  void handle_ended()
  {
    spdlog::info("Continue? y/[n]");
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "y") {
      start_new_game(_game_id);
    }
    else {
      _state = State::Should_stop;
    }
  }

  void start_new_game(std::uint64_t &game_id)
  {
    std::vector<std::string> parts;

    for (;;) {
      spdlog::info("Players online: {}", request("list-players online"));
      spdlog::info("Who do you want battle with?");
      std::string choice;
      std::getline(std::cin, choice);
      auto const response{request(std::format("battle {}", choice))};
      parts = split_by(response);
      if (parts[0] == "ok") {
        break;
      }
      spdlog::warn("{}", response);
    }

    game_id = std::stoull(parts[1]);
    poll_events();
    _state = State::Starting;
  }

  asio::io_context _io_context;
  Session_ptr _subscribing_session;
  Session_ptr _request_session;
  State _state;
  std::string _player_name;
  std::uint64_t _game_id{};
};
