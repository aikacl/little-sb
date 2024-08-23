#pragma once

#include "sb-packet.h"
#include "session.h"
#include <string>
#include <string_view>
#include <utility>

class Application {
  enum class State : std::uint8_t {
    greeting,
    starting,
    running,
    ended,
    should_stop,
  };

public:
  Application(std::string_view host, std::uint16_t port,
              std::string_view player_name);
  void run();

private:
  [[nodiscard]] auto should_stop() const -> bool;
  [[nodiscard]] auto connect(std::string_view host,
                             std::uint16_t port) -> tcp::socket;
  void write(std::string message);
  void tick();
  void handle_greeting();
  void handle_starting();
  void handle_running();
  void poll_events();
  void handle_ended();
  void start_new_game(std::uint64_t &game_id);

  template <typename Result_type>
  auto request(std::string request) -> Result_type
  {
    auto const packet{_requesting_session->request(
        Sb_packet{Sb_packet_sender{_player_name, _player_name},
                  json(std::move(request)).dump()})};
    return json::parse(packet.payload).get<Result_type>();
  }

  asio::io_context _io_context;
  Session_ptr _subscribing_session;
  Session_ptr _requesting_session;
  State _state{State::greeting};
  std::string _player_name;
  std::uint64_t _game_id{};
};
